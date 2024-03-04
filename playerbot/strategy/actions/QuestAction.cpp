
#include "playerbot/playerbot.h"
#include "QuestAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool QuestAction::Execute(Event& event)
{
    ObjectGuid guid = event.getObject();

    Player* master = GetMaster();

    if (!guid)
    {
        if (!master)
        {
            guid = bot->GetSelectionGuid();
        }
        else
        {
            guid = master->GetSelectionGuid();
        }
    }

    if (guid)
    {
        return ProcessQuests(guid);
    }

    bool result = false;
    list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
    for (list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
            result |= ProcessQuests(unit);
    }
    list<ObjectGuid> gos = AI_VALUE(list<ObjectGuid>, "nearest game objects");
    for (list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (go && bot->GetDistance(go) <= INTERACTION_DISTANCE)
            result |= ProcessQuests(go);
    }

    return result;
}

bool QuestAction::CompleteQuest(Player* player, uint32 entry)
{
    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        return false;
    }

    // Add quest items for quests that require items
    for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->ReqItemId[x];
        uint32 count = pQuest->ReqItemCount[x];
        if (!id || !count)
        {
            continue;
        }

        uint32 curItemCount = player->GetItemCount(id, true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, id, true);
            player->SendNewItem(item, count - curItemCount, true, false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = pQuest->ReqCreatureOrGOId[i];
        uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

        if (uint32 spell_id = pQuest->ReqSpell[i])
        {
            for (uint16 z = 0; z < creaturecount; ++z)
            {
                player->CastedCreatureOrGO(creature, ObjectGuid(), spell_id);
            }
        }
        else if (creature > 0)
        {
            if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature))
                for (uint16 z = 0; z < creaturecount; ++z)
                {
                    player->KilledMonster(cInfo, nullptr);
                }
        }
        else if (creature < 0)
        {
            for (uint16 z = 0; z < creaturecount; ++z)
            {
                player->CastedCreatureOrGO(-creature, ObjectGuid(), 0);
            }
        }
    }

    // If the quest requires reputation to complete
    if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
        if (curRep < repValue)
        {
#ifdef MANGOSBOT_ONE
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry<FactionEntry>(repFaction))
#else
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
#endif
            {
                player->GetReputationMgr().SetReputation(factionEntry, repValue);
            }
        }
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if (ReqOrRewMoney < 0)
    {
        player->ModifyMoney(-ReqOrRewMoney);
    }

    player->CompleteQuest(entry);
    return true;
}

bool QuestAction::ProcessQuests(ObjectGuid questGiver)
{
    GameObject *gameObject = ai->GetGameObject(questGiver);
    if (gameObject && gameObject->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
    {
        return ProcessQuests(gameObject);
    }

    Creature* creature = ai->GetCreature(questGiver);
    if (creature)
    {
        return ProcessQuests(creature);
    }

    return false;
}

bool QuestAction::ProcessQuests(WorldObject* questGiver)
{
    ObjectGuid guid = questGiver->GetObjectGuid();

    if (sServerFacade.GetDistance2d(bot, questGiver) > INTERACTION_DISTANCE && !sPlayerbotAIConfig.syncQuestWithPlayer)
    {
        Player* master = ai->GetMaster();
        if (!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(master, BOT_TEXT("quest_error_talk"));

        return false;
    }

    if (!sServerFacade.IsInFront(bot, questGiver, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
    {
        sServerFacade.SetFacingTo(bot, questGiver);
    }

    bot->SetSelectionGuid(guid);
    bot->PrepareQuestMenu(guid);
    QuestMenu& questMenu = bot->GetPlayerMenu()->GetQuestMenu();

    bool hasAccept = false;
    for (uint32 i = 0; i < questMenu.MenuItemCount(); ++i)
    {
        QuestMenuItem const& menuItem = questMenu.GetItem(i);
        uint32 questID = menuItem.m_qId;
        Quest const* quest = sObjectMgr.GetQuestTemplate(questID);
        if (!quest)
        {
            continue;
        }

        hasAccept |= ProcessQuest(GetMaster(), quest, questGiver);
    }

    return hasAccept;
}

bool QuestAction::AcceptQuest(Player* requester, Quest const* quest, uint64 questGiver)
{
    bool success = false;
    const uint32 questId = quest->GetQuestId();

    string outputMessage;
    map<string, string> args;
    args["%quest"] = chat->formatQuest(quest);
    
    if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
    {
        outputMessage = BOT_TEXT2("quest_error_completed", args);
    }
    else if (! bot->CanTakeQuest(quest, false))
    {
        if (! bot->SatisfyQuestStatus(quest, false))
        {
            outputMessage = BOT_TEXT2("quest_error_have_quest", args);
        }
        else if (!ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        {
            outputMessage = BOT_TEXT2("quest_error_cant_take", args);
        }
    }
    else if (! bot->SatisfyQuestLog(false))
    {
        outputMessage = BOT_TEXT2("quest_error_log_full", args);
    }
    else if (! bot->CanAddQuest(quest, false))
    {
        outputMessage = BOT_TEXT2("quest_error_bag_full", args);
    }
    else
    {
        WorldPacket p(CMSG_QUESTGIVER_ACCEPT_QUEST);
        uint32 unk1 = 0;
        p << questGiver << questId << unk1;
        p.rpos(0);
        bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(p);

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_NONE && sPlayerbotAIConfig.syncQuestWithPlayer)
        {
            Object* pObject = bot->GetObjectByTypeMask((ObjectGuid)questGiver, TYPEMASK_CREATURE_GAMEOBJECT_PLAYER_OR_ITEM);
            bot->AddQuest(quest, pObject);
        }

        if (bot->GetQuestStatus(questId) != QUEST_STATUS_NONE && bot->GetQuestStatus(questId) != QUEST_STATUS_AVAILABLE)
        {
            sPlayerbotAIConfig.logEvent(ai, "AcceptQuestAction", quest->GetTitle(), to_string(quest->GetQuestId()));
            outputMessage = BOT_TEXT2("quest_accepted", args);
            success = true;
        }
    }

    if (success || !ai->GetMaster() || sServerFacade.GetDistance2d(bot, ai->GetMaster()) < sPlayerbotAIConfig.reactDistance || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayer(requester, outputMessage, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    return success;
}

bool QuestObjectiveCompletedAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket p(event.getPacket());
    p.rpos(0);

    uint32 entry, questId, available, required;
    ObjectGuid guid;
    p >> questId >> entry >> available >> required >> guid;

    if (entry & 0x80000000)
    {
        entry &= 0x7FFFFFFF;
        GameObjectInfo const* info = sObjectMgr.GetGameObjectInfo(entry);
        if (info)
        {
            ai->TellPlayer(requester, chat->formatQuestObjective(info->name, available, required), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
    }
    else
    {
        CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(entry);
        if (info)
        {
            ai->TellPlayer(requester, chat->formatQuestObjective(info->Name, available, required), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
    }

    Quest const* qInfo = sObjectMgr.GetQuestTemplate(questId);
    sPlayerbotAIConfig.logEvent(ai, "QuestObjectiveCompletedAction", qInfo->GetTitle(), to_string((float)available / (float)required));
    return false;
}
