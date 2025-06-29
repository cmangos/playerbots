#include "PlayerbotAI.h"
#include "PlayerbotRpcClient.h"
#include "PlayerbotRpcClientMgr.h" // Added for the manager
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "Entities/Player.h"
#include "Entities/Unit.h"
#include "QuestDef.h"
#include "ObjectGuid.h"
#include "rpc/playerbot_rpc.pb.h"
#include "Logging/Log.h"
#include "strategy/AiObjectContext.h"
#include "strategy/values/ValueContext.h"

// RPC related methods
void PlayerbotAI::InitRpcClient()
{
    // The PlayerbotRpcClientMgr should be initialized globally once,
    // for example, when the PlayerbotAIConfig is loaded or the server starts.
    // We ensure it's initialized here if needed, but this might log repeatedly if called per bot
    // without a proper global initialization point.
    if (!sPlayerbotRpcClientMgr.IsInitialized() && !sPlayerbotAIConfig.actionServerAddress.empty()) {
        sPlayerbotRpcClientMgr.Initialize(sPlayerbotAIConfig.actionServerAddress);
    }

    if (sPlayerbotRpcClientMgr.IsInitialized())
    {
        playerbot_rpc::ActionService::StubInterface* stub = sPlayerbotRpcClientMgr.GetClientStub();
        if (stub)
        {
            try
            {
                // Note: rpcClient is std::unique_ptr<PlayerbotRpcClient>
                rpcClient = std::make_unique<PlayerbotRpcClient>(this, stub);
                if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
                    std::ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                    out << GetBot()->GetName() << ",INFO,PlayerbotAI,InitRpcClient: PlayerbotRpcClient wrapper created using shared stub for bot.";
                    sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
                }
            }
            catch (const std::exception& e)
            {
                 if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
                    std::ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                    out << GetBot()->GetName() << ",ERROR,PlayerbotAI,InitRpcClient: Exception while creating PlayerbotRpcClient wrapper: " << e.what();
                    sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
                }
                rpcClient.reset();
            }
        }
        else
        {
            if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
                std::ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << GetBot()->GetName() << ",ERROR,PlayerbotAI,InitRpcClient: Failed to get client stub from PlayerbotRpcClientMgr for bot.";
                sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
            }
            rpcClient.reset();
        }
    }
    else
    {
        if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
            out << GetBot()->GetName() << ",INFO,PlayerbotAI,InitRpcClient: PlayerbotRpcClientMgr not initialized. RPC client not created for bot.";
            sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
        }
        rpcClient.reset();
    }
}

bool PlayerbotAI::ShouldUseRpcForDecision()
{
    return rpcClient != nullptr && sPlayerbotAIConfig.enableActionServerDecisionMaking;
}

void PlayerbotAI::PopulateBotStateSnapshot(playerbot_rpc::BotStateSnapshot* snapshot)
{
    if (!snapshot || !bot) return;

    snapshot->set_bot_guid(bot->GetObjectGuid().GetRawValue());

    snapshot->set_bot_class(static_cast<playerbot_rpc::BotClass>(bot->getClass()));
    snapshot->set_bot_race(static_cast<playerbot_rpc::BotRace>(bot->getRace()));

    playerbot_rpc::Position* current_pos = snapshot->mutable_current_position();
    current_pos->set_x(bot->GetPositionX());
    current_pos->set_y(bot->GetPositionY());
    current_pos->set_z(bot->GetPositionZ());
    current_pos->set_orientation(bot->GetOrientation());
    current_pos->set_map_id(bot->GetMapId());

    playerbot_rpc::BotStats* stats = snapshot->mutable_current_stats();
    stats->set_health(bot->GetHealth());
    stats->set_max_health(bot->GetMaxHealth());
    stats->set_mana(bot->GetPower(POWER_MANA));
    stats->set_max_mana(bot->GetMaxPower(POWER_MANA));
    stats->set_level(bot->GetLevel());

    snapshot->set_is_in_combat(sServerFacade.IsInCombat(bot));

    if (Unit* currentTarget = GetAiObjectContext()->GetValue<Unit*>("current target")->Get()) {
        playerbot_rpc::Entity* entity_proto = snapshot->add_nearby_entities();
        entity_proto->set_guid(currentTarget->GetObjectGuid().GetRawValue());
        entity_proto->set_name(currentTarget->GetName());
        entity_proto->mutable_position()->set_x(currentTarget->GetPositionX());
        entity_proto->mutable_position()->set_y(currentTarget->GetPositionY());
        entity_proto->mutable_position()->set_z(currentTarget->GetPositionZ());
        entity_proto->mutable_position()->set_map_id(currentTarget->GetMapId());
        entity_proto->set_is_hostile(sServerFacade.IsHostileTo(bot, currentTarget));
        entity_proto->set_health_percent(GetHealthPercent(*currentTarget));
    }

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot) {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId) continue;

        const Quest* qTemplate = sObjectMgr.GetQuestTemplate(questId);
        if (!qTemplate) continue;

        playerbot_rpc::Quest* quest_proto = snapshot->add_active_quests();
        quest_proto->set_quest_id(qTemplate->GetQuestId());
        quest_proto->set_title(qTemplate->GetTitle());
        quest_proto->set_level(qTemplate->GetQuestLevel());
        quest_proto->set_is_active(true);
        quest_proto->set_completed(bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE);

        QuestStatusData qStatus = bot->getQuestStatusMap()[questId];
        for (int i = 0; i < QUEST_OBJECTIVES_COUNT; ++i) {
            if (qTemplate->ReqCreatureOrGOId[i] != 0 || qTemplate->ReqItemId[i] != 0) {
                 playerbot_rpc::QuestObjective* obj_proto = quest_proto->add_objectives();
                 obj_proto->set_description(qTemplate->ObjectiveText[i]);

                if (qTemplate->ReqCreatureOrGOId[i] != 0) {
                    obj_proto->set_current_count(qStatus.m_creatureOrGOcount[i]);
                    obj_proto->set_required_count(qTemplate->ReqCreatureOrGOCount[i]);
                    obj_proto->set_completed(qStatus.m_creatureOrGOcount[i] >= qTemplate->ReqCreatureOrGOCount[i]);
                } else if (qTemplate->ReqItemId[i] != 0) {
                    obj_proto->set_current_count(qStatus.m_itemcount[i]);
                    obj_proto->set_required_count(qTemplate->ReqItemCount[i]);
                    obj_proto->set_completed(qStatus.m_itemcount[i] >= qTemplate->ReqItemCount[i]);
                }
            }
        }
    }

    if (engines[(uint8)BotState::BOT_STATE_NON_COMBAT]) {
        std::list<std::string_view> strats = engines[(uint8)BotState::BOT_STATE_NON_COMBAT]->GetStrategies();
        if (!strats.empty()) snapshot->set_current_strategy_non_combat(std::string(strats.front()));
    }
    if (engines[(uint8)BotState::BOT_STATE_COMBAT]) {
         std::list<std::string_view> strats = engines[(uint8)BotState::BOT_STATE_COMBAT]->GetStrategies();
        if (!strats.empty()) snapshot->set_current_strategy_combat(std::string(strats.front()));
    }

    if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << GetBot()->GetName() << ",DEBUG,PlayerbotAI,PopulateBotStateSnapshot: Snapshot populated for bot.";
        sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
    }
}

bool PlayerbotAI::ProcessRpcDecision()
{
    if (!ShouldUseRpcForDecision()) return false;

    playerbot_rpc::BotStateSnapshot snapshot;
    PopulateBotStateSnapshot(&snapshot);

    playerbot_rpc::MacroDecisionResponse response;
    if (rpcClient && rpcClient->GetMacroDecision(snapshot, &response))
    {
        if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
            out << GetBot()->GetName() << ",INFO,PlayerbotAI,ProcessRpcDecision: Received decision type " << response.decision_type()
                << ". Justification: " << response.justification();
            sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
        }

        if (sPlayerbotAIConfig.enableDebugWhispers && master && bot) {
            std::string decision_text = "RPC Decision (" + bot->GetName() + "): " + std::to_string(response.decision_type()) + " - " + response.justification();
            TellPlayer(master, decision_text , PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, true);
        }

        switch (response.decision_type())
        {
            case playerbot_rpc::MacroDecisionResponse::SET_STRATEGY:
                if (!response.strategy_name().empty())
                {
                    BotState stateToApply = sServerFacade.IsInCombat(bot) ? BotState::BOT_STATE_COMBAT : BotState::BOT_STATE_NON_COMBAT;
                    ChangeStrategy(response.strategy_name(), stateToApply);
                    ResetAIInternalUpdateDelay();
                    return true;
                }
                break;
            case playerbot_rpc::MacroDecisionResponse::EXECUTE_COMMAND:
                if (!response.command_string().empty())
                {
                    HandleCommand(CHAT_MSG_WHISPER , response.command_string(), *master);
                    ResetAIInternalUpdateDelay();
                    return true;
                }
                break;
            case playerbot_rpc::MacroDecisionResponse::TRAVEL_TO:
                if (response.has_travel_position()) {
                    const auto& pos = response.travel_position();
                    std::ostringstream travelCmd;
                    travelCmd << "travel " << pos.x() << " " << pos.y() << " " << pos.z();
                    // Map ID handling for travel needs to be robust in the actual "travel" action.
                    // For now, we assume current map if map_id in response is 0 or matches current.
                    // If map_id is different, the travel action needs to handle cross-map travel (e.g. using teleporter or flight paths).
                    HandleCommand(CHAT_MSG_WHISPER , travelCmd.str(), *master);
                    ResetAIInternalUpdateDelay();
                    return true;
                }
                break;
            case playerbot_rpc::MacroDecisionResponse::TARGET_ENTITY:
                if (response.target_guid() != 0)
                {
                    ObjectGuid newTargetGuid(response.target_guid());
                    Unit* newTarget = GetUnit(newTargetGuid);
                    if (newTarget) {
                        bot->SetSelectionGuid(newTargetGuid);
                        GetAiObjectContext()->GetValue<Unit*>("current target")->Set(newTarget);
                        ResetAIInternalUpdateDelay();
                    }
                    return true;
                }
                break;
            case playerbot_rpc::MacroDecisionResponse::CONTINUE_CURRENT:
            case playerbot_rpc::MacroDecisionResponse::DECIDE_UNKNOWN:
            default:
                return false;
        }
    }
    else if (rpcClient)
    {
        if (bot && master && sPlayerbotAIConfig.hasLog("bot_rpc_client.csv")) {
             std::ostringstream out;
             out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
             out << GetBot()->GetName() << ",ERROR,PlayerbotAI,ProcessRpcDecision: GetMacroDecision RPC call failed to " << sPlayerbotAIConfig.actionServerAddress;
             sPlayerbotAIConfig.log("bot_rpc_client.csv", out.str().c_str());
        }
    }
    return false;
}

void PlayerbotAI::StartRpcService() {
    sLog.outBasic("PlayerbotAI %s: StartRpcService stub called. GameControlService server not implemented in this step.", bot ? bot->GetName() : "UNKNOWN_BOT");
}

void PlayerbotAI::StopRpcService() {
    sLog.outBasic("PlayerbotAI %s: StopRpcService stub called.", bot ? bot->GetName() : "UNKNOWN_BOT");
}
