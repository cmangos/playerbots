
#include "playerbot/playerbot.h"
#include "ChooseRpgTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/PossibleRpgTargetsValue.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "GuildCreateActions.h"
#include "RpgSubActions.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/strategy/values/PositionValue.h"

using namespace ai;

bool ChooseRpgTargetAction::HasSameTarget(ObjectGuid guid, uint32 max, std::list<ObjectGuid>& nearGuids)
{
    if (ai->HasRealPlayerMaster())
        return 0;

    uint32 num = 0;

    for (auto& i : nearGuids)
    {
        Player* player = sObjectMgr.GetPlayer(i);

        if (!player)
            continue;

        PlayerbotAI* ai = player->GetPlayerbotAI();

        if (!ai)
            continue;

        if (!ai->AllowActivity(GRIND_ACTIVITY))
            continue;

        if (PAI_VALUE(GuidPosition,"rpg target") != guid)
            continue;

        num++;

        if (num >= max)
            break;
    }

    return num;
}

float ChooseRpgTargetAction::getMaxRelevance(GuidPosition guidP)
{
    GuidPosition currentRpgTarget = AI_VALUE(GuidPosition, "rpg target");
    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    Strategy* rpgStrategy;

    std::list<TriggerNode*> triggerNodes;

    float maxRelevance = 0.0f;

    for (auto& strategy : ai->GetAiObjectContext()->GetSupportedStrategies())
    {
        if (strategy.find("rpg") == std::string::npos)
            continue;

        if (!ai->HasStrategy(strategy, BotState::BOT_STATE_NON_COMBAT))
            continue;

        rpgStrategy = ai->GetAiObjectContext()->GetStrategy(strategy);

        rpgStrategy->InitTriggers(triggerNodes, BotState::BOT_STATE_NON_COMBAT);

        for (auto& triggerNode : triggerNodes)
        {
            Trigger* trigger = context->GetTrigger(triggerNode->getName());

            if (trigger)
            {
                triggerNode->setTrigger(trigger);

                if (triggerNode->getFirstRelevance() < maxRelevance || triggerNode->getFirstRelevance() > 2.0f)
                    continue;

                Trigger* trigger = triggerNode->getTrigger();

                if (!trigger->IsActive())
                    continue;

                NextAction** nextActions = triggerNode->getHandlers();

                bool isRpg = false;

                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                {
                    NextAction* nextAction = nextActions[i];

                    Action* action = ai->GetAiObjectContext()->GetAction(nextAction->getName());

                    if (dynamic_cast<RpgEnabled*>(action))
                        isRpg = true;
                }
                NextAction::destroy(nextActions);

                if (isRpg)
                {
                    maxRelevance = triggerNode->getFirstRelevance();
                    rgpActionReason[guidP] = triggerNode->getName();
                }
            }
        }

        for (std::list<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
        {
            TriggerNode* trigger = *i;
            delete trigger;
        }

        triggerNodes.clear();
    }

    SET_AI_VALUE(GuidPosition,"rpg target", currentRpgTarget);

    if (!maxRelevance)
        return 0.0;

    return floor((maxRelevance - 1.0) * 1000.0f);
}

bool ChooseRpgTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    GuidPosition masterRpgTarget;
    if (requester && requester->GetPlayerbotAI() && requester->GetMapId() == bot->GetMapId() && !requester->IsBeingTeleported())
    {
        
    }
    else
    {
        requester = nullptr;
    }

    std::unordered_map<ObjectGuid, float> targets;
    std::vector<ObjectGuid> targetList;

    std::list<ObjectGuid> possibleTargets = AI_VALUE(std::list<ObjectGuid>, "possible rpg targets");
    std::list<ObjectGuid> possibleObjects = bot->GetMap()->IsDungeon() ? AI_VALUE(std::list<ObjectGuid>, "nearest game objects") : AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los"); // skip not in LOS objects in dungeons
    std::list<ObjectGuid> possiblePlayers = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");
    std::set<ObjectGuid>& ignoreList = AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");

    for (auto target : possibleTargets)
        targets[target] = 0.0f;

    for (auto target : possibleObjects)
        targets[target] = 0.0f;

    for (uint32 i = 0; i < 5 && possiblePlayers.size(); i++)
    {
        auto p = possiblePlayers.begin();
        std::advance(p, urand(0, possiblePlayers.size() - 1));
        if (ignoreList.find(*p) != ignoreList.end())
            continue;

        targets[*p] = 0.0f;
        possiblePlayers.erase(p);
    }

    if (targets.empty())
    {
        return false;
    }

    if (urand(0, 9))
    {
        for (auto target : ignoreList)
            targets.erase(target);
    }

    SET_AI_VALUE(std::string, "next rpg action", this->getName());

    bool hasGoodRelevance = false;

    rgpActionReason.clear();

    for (auto& target : targets)
        targetList.push_back(target.first);

    std::shuffle(targetList.begin(), targetList.end(), *GetRandomGenerator());

    //Update tradeskill items so we can use lazy in trigger check.
    if(ai->HasStrategy("rpg craft", BotState::BOT_STATE_NON_COMBAT))
    {
        AI_VALUE2(std::list<uint32>, "inventory item ids", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_SKILL));
    }

    context->ClearExpiredValues("can free move",10); //Clean up old free move to.

    uint16 checked = 0, maxCheck = 50;

    if (ai->HasRealPlayerMaster())
        maxCheck = 500;

    for (auto& guid :targetList)
    {
        GuidPosition guidP(guid, bot->GetMapId());

        if (!guidP)
            continue;

        if (guidP.GetWorldObject() && !AI_VALUE2(bool, "can free move to", guidP.to_string()))
            continue;

        if (guidP.IsGameObject())
        {
            GameObject* go = guidP.GetGameObject();
            if (!go || !sServerFacade.isSpawned(go)
                || go->IsInUse()
                || go->GetGoState() != GO_STATE_READY)
                continue;

            // skip objects too high
            if (fabs(go->GetPositionZ() - bot->GetPositionZ()) > 20.f)
                continue;
        }
        else if (guidP.IsPlayer())
        {
            Player* player = guidP.GetPlayer();

            if (!player)
                continue;

            if (player == bot)
                continue;

            if (player->GetPlayerbotAI())
            {
                GuidPosition guidPP = PAI_VALUE(GuidPosition, "rpg target");

                if (guidPP.IsPlayer())
                {
                    continue;
                }

                if (bot->GetGroup() && player->GetGroup() == bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid())) //If leader starts following members that are on follow they keep moving around.
                    continue;
            }
        }

        if (possiblePlayers.size() < 200 && HasSameTarget(guidP, urand(5, 15), possiblePlayers))
            continue;

        float relevance = getMaxRelevance(guidP);

        if (guidP.GetEntry() == travelTarget->getEntry())
            relevance *= 1.5f;

        if (!hasGoodRelevance || relevance > 1)
            targets[guidP] = relevance;

        if (targets[guidP] > 1)
        {
            hasGoodRelevance = true;
            uint32 mod = guidP.fDist(bot);
            if (mod > 60 + targets[guidP])
                targets[guidP] = 2;
            else
                targets[guidP] += 60 - mod;
        }

        checked++;

        if (checked >= 50) //Some limit on stuff to check.
            break;
    }

    SET_AI_VALUE(std::string, "next rpg action", "");

    for (auto it = begin(targets); it != end(targets);)
    {
        if (it->second == 0) //Remove empty targets.
            it = targets.erase(it);
        else if (hasGoodRelevance && it->second <= 1.0) //Remove useless targets if there's any good ones
            it = targets.erase(it);
        else if (!hasGoodRelevance && requester && requester != bot && (!masterRpgTarget || it->first != masterRpgTarget)) //Remove useless targets if it's not masters target.
            it = targets.erase(it);
        else
            ++it;
    }

    if (targets.empty())
    {
        if (ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT))
        {
            std::ostringstream out;
            out << "found: no targets, " << checked << " checked.";
            ai->TellPlayerNoFacing(requester, out);
        }
        sLog.outDetail("%s can't choose RPG target: all %zu are not available", bot->GetName(), possibleTargets.size());
        RESET_AI_VALUE(std::set<ObjectGuid>&,"ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    if (ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT))
    {
        std::vector<std::pair<ObjectGuid, uint32>> sortedTargets(targets.begin(), targets.end());

        std::sort(sortedTargets.begin(), sortedTargets.end(), [](std::pair<ObjectGuid, uint32>i, std::pair<ObjectGuid, uint32> j) {return i.second > j.second; });

        ai->TellPlayerNoFacing(requester, "------" + std::to_string(targets.size()) + "------");

        uint32 checked = 0;

        for (auto target : sortedTargets)
        {
            GuidPosition guidP(target.first, bot->GetMapId());

            if (!guidP.GetWorldObject())
                continue;

            std::ostringstream out;
            out << chat->formatWorldobject(guidP.GetWorldObject());

            out << " " << rgpActionReason[guidP] << " " << target.second;

            ai->TellPlayerNoFacing(requester, out);

            checked++;

            if (checked >= 10)
            {
                std::ostringstream out;
                out << "and " << (sortedTargets.size()-checked) << " more...";
                ai->TellPlayerNoFacing(requester, out);
                break;
            }
        }
    }

    std::vector<ObjectGuid> guidps;
    std::vector<int> relevances;

    for (auto& target : targets)
    {
        guidps.push_back(target.first);
        relevances.push_back(target.second);

        if (target.second == 1)
            ignoreList.insert(target.first);
    }

    std::mt19937 gen(time(0));

    sTravelMgr.weighted_shuffle(guidps.begin(), guidps.end(), relevances.begin(), relevances.end(), gen);

    GuidPosition guidP(guidps.front(),bot->GetMapId());

    if (!guidP)
    {
        RESET_AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    if ((ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT)) && guidP.GetWorldObject())
    {
        std::ostringstream out;
        out << "found: ";
        out << chat->formatWorldobject(guidP.GetWorldObject());

        out << " " << rgpActionReason[guidP] << " " << targets[guidP];

        ai->TellPlayerNoFacing(requester, out);
    }

    SET_AI_VALUE(GuidPosition, "rpg target", guidP);
    SET_AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target", ignoreList);

    return true;
}

bool ChooseRpgTargetAction::isUseful()
{
    if (!ai->AllowActivity(RPG_ACTIVITY))
        return false;

    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    if (guidP && guidP.distance(bot) < sPlayerbotAIConfig.reactDistance * 2)
        return false;

    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    if (travelTarget->isTraveling() && AI_VALUE2(bool, "can free move to", *travelTarget->getPosition()))
        return false;

    if (AI_VALUE(std::list<ObjectGuid>, "possible rpg targets").empty())
        return false;

    //Not stay, not guard, not combat, not trading and group ready.
    if (!AI_VALUE(bool, "can move around"))
        return false;

    return true;
}

/*
bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldObject* target)
{
    if (!target)
        return false;

    WorldLocation location;
    target->GetPosition(location);
    return isFollowValid(bot, location);
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldPosition pos)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    Player* master = ai->GetGroupMaster();
    Player* realMaster = ai->GetMaster();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!master || bot == master || master->IsBeingTeleported() || master->GetMapId() != bot->GetMapId())
        return true;

    float distance;

    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    bool freeMove = false;
    
    //Set distance relative to focus position.
    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
        distance = sqrt(master->GetDistance2d(pos.getX(), pos.getY(), DIST_CALC_NONE));
    else if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) && posMap["stay"].isSet())
        distance = sqrt(pos.sqDistance2d(posMap["stay"].Get()));
    else if (ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT) && posMap["guard"].isSet())
        distance = sqrt(pos.sqDistance2d(posMap["guard"].Get()));
    else
    {
        distance = sqrt(pos.sqDistance2d(bot));
        freeMove = true;
    }


    //Check if bot is in dungeon with master.
    bool inDungeon = false;
    if (master->IsInWorld() && master->GetMap()->IsDungeon())
    {
        if (bot->GetMapId() == master->GetMapId())
            inDungeon = true;
    }

    //Restrict distance in combat and in dungeons.
    if ((inDungeon || master->IsInCombat()) && distance > 5.0f)
        return false;

    //With a bot master bots have more freedom.
    if (!ai->HasActivePlayerMaster())
    {
        Player* player = master;
        if (PAI_VALUE(WorldPosition, "last long move").distance(pos) < sPlayerbotAIConfig.reactDistance)
            return true;

        if (!master->IsInCombat() && distance < sPlayerbotAIConfig.reactDistance * 0.75f)
            return true;

        if (distance < sPlayerbotAIConfig.reactDistance * 0.25f)
            return true;

        return false;
    }

    //Increase distance as master is standing still.
    float maxDist = INTERACTION_DISTANCE;

    if (freeMove || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT)) //Free and guard start with a base 20y range.
        maxDist += sPlayerbotAIConfig.lootDistance;

    if (WorldPosition(bot).fDist(master) < sPlayerbotAIConfig.reactDistance)
    {
        uint32 lastMasterMove = MEM_AI_VALUE(WorldPosition, "master position")->LastChangeDelay();

        if (lastMasterMove > 30.0f) //After 30 seconds increase the range by 1y each second.
            maxDist += (lastMasterMove - 30);

        if (maxDist > sPlayerbotAIConfig.reactDistance)
            if (freeMove)
                return true;
            else
                maxDist = sPlayerbotAIConfig.reactDistance;
    }
    else if (freeMove)
        return true;
    else
        maxDist = sPlayerbotAIConfig.reactDistance;

    if (distance < maxDist)
        return true;

    return false;
}
*/