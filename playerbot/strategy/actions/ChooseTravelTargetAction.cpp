
#include "playerbot/playerbot.h"
#include "playerbot/LootObjectStack.h"
#include "ChooseTravelTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/TravelValues.h"
#include <iomanip>

using namespace ai;

bool ChooseTravelTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    //Get the current travel target. This target is no longer active.
    TravelTarget * oldTarget = AI_VALUE(TravelTarget *,"travel target");

    //Select a new target to travel to. 
    TravelTarget newTarget = TravelTarget(ai);   
    if (!oldTarget->IsForced() || oldTarget->GetStatus() == TravelStatus::TRAVEL_STATUS_EXPIRED)
        getNewTarget(requester, &newTarget, oldTarget);
    else
        newTarget.CopyTarget(oldTarget);

    //If the new target is not active we failed.
    if (!newTarget.IsActive() && !newTarget.IsForced())
       return false;    

    setNewTarget(requester, &newTarget, oldTarget);

    return true;
}

//Select a new travel target.
//Currently this selects mostly based on priority (current quest > new quest).
//This works fine because destinations can be full (max 15 bots per quest giver, max 1 bot per quest mob).
//
//Eventually we want to rewrite this to be more intelligent.
void ChooseTravelTargetAction::getNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget)
{
    info = PlayerTravelInfo(bot);

    bool foundTarget = false;
    focusQuestTravelList focusList = AI_VALUE(focusQuestTravelList, "focus travel target");

    foundTarget = SetGroupTarget(requester, newTarget);                                 //Join groups members

    //Empty bags/repair
    if (!foundTarget && urand(1, 100) > 10)                                  //90% chance
    {
        bool shouldRpg = false;
        if (AI_VALUE2(bool, "group or", "should sell,can sell,following party,near leader")) //One of party members wants to sell items (full bags).
        {
            ai->TellDebug(requester, "Group needs to sell items (full bags)", "debug travel");
            shouldRpg = true;
        }
        else if (AI_VALUE2(bool, "group or", "should repair,can repair,following party,near leader")) //One of party members wants to repair.
        {
            ai->TellDebug(requester, "Group needs to repair", "debug travel");
            shouldRpg = true;
        }
        else if (AI_VALUE2(bool, "group or", "should ah sell,can ah sell,following party,near leader") && bot->GetLevel() > 5) //One of party members wants to sell items to AH (full bags).
        {
            ai->TellDebug(requester, "Group needs to ah items (full bags)", "debug travel");
            shouldRpg = true;
        }
        else if (!shouldRpg && ai->HasStrategy("free", BotState::BOT_STATE_NON_COMBAT))
        {
            if (AI_VALUE(bool, "should sell") && AI_VALUE(bool, "can sell")) //Bot wants to sell (full bags).
            {
                ai->TellDebug(requester, "Bot needs to sell/ah items (full bags)", "debug travel");
                shouldRpg = true;                
            }
            if (AI_VALUE(bool, "should ah sell") && AI_VALUE(bool, "can ah sell")) //Bot wants to ah sell (repariable item that it wants to ah).
            {
                ai->TellDebug(requester, "Bot needs to sell/ah items (full bags)", "debug travel");
                shouldRpg = true;
            }
            else if (AI_VALUE(bool, "should repair") && AI_VALUE(bool, "can repair")) //Bot wants to repair.
            {
                ai->TellDebug(requester, "Bot needs to repair", "debug travel");
                shouldRpg = true;
            }
        }
                     
        if (shouldRpg)
        {
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetRpgTarget1", &context->performanceStack);
            foundTarget = SetRpgTarget(requester, newTarget);                           //Go to town to sell items or repair
        }
    }

    WorldPosition botPos(bot);

    //Rpg in city
    if (focusList.empty() && !foundTarget && urand(1, 100) > 90 && bot->GetLevel() > 5 && botPos.isOverworld())           //10% chance if not currenlty in dungeon.
    {
        ai->TellDebug(requester, "Random rpg in city", "debug travel");
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetNpcFlagTarget2", &context->performanceStack);
        foundTarget = SetNpcFlagTarget(requester, newTarget, { UNIT_NPC_FLAG_BANKER,UNIT_NPC_FLAG_BATTLEMASTER,UNIT_NPC_FLAG_AUCTIONEER });
    }

    if (!foundTarget && !focusList.empty()) 
    {
        ai->TellDebug(requester, "Do focus questing", "debug travel");
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetFocusQuestTarget", &context->performanceStack);
        foundTarget = SetQuestTarget(requester, newTarget, true, true, true);    //Do any nearby    

        if (!foundTarget)
        {
            ai->TellDebug(requester, "No focus quest found", "debug travel");
            SetNullTarget(newTarget);
        }

        return;
    }

    // PvP activities
    bool pvpActivate = false;
    if (pvpActivate && !foundTarget && urand(0, 4) && bot->GetLevel() > 50)
    {
        ai->TellDebug(requester, "Pvp in Tarren Mill", "debug travel");
        WorldPosition pos = WorldPosition(bot);
        TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();

        TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, "Tarren Mill");
        if (dest)
        {
            std::vector<WorldPosition*> points = dest->NextPoint(pos);

            if (!points.empty())
            {
                target->SetTarget(dest, points.front());
                target->SetForced(true);

                std::ostringstream out; out << "Traveling to " << dest->GetTitle();
                ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                foundTarget = true;
            }
        }
    }

    //Grind for money (if not in dungeon)
    if (!foundTarget && AI_VALUE(bool, "should get money") && botPos.isOverworld())
    {
        //Empty mail for money
        if (AI_VALUE(bool, "can get mail"))
        {
            ai->TellDebug(requester, "Get mail for money", "debug travel");
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetGoTarget1", &context->performanceStack);
            foundTarget = SetGOTypeTarget(requester, newTarget, GAMEOBJECT_TYPE_MAILBOX,"",false);  //Find a mailbox
        }

        if (!foundTarget)
        {
            if (urand(1, 100) > 10) //90% Focus on active quests for money.
            {
                {
                    ai->TellDebug(requester, "Turn in quests for money", "debug travel");
                    auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetQuestTarget1", &context->performanceStack);
                    foundTarget = SetQuestTarget(requester, newTarget, false, true, true);           //Turn in quests for money.
                }

                if (!foundTarget)
                {
                    ai->TellDebug(requester, "Start quests for money", "debug travel");
                    auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetQuestTarget2", &context->performanceStack);
                    foundTarget = SetQuestTarget(requester, newTarget, true, false, false);      //Find new (low) level quests
                }
            }

            if (!foundTarget)
            {
                ai->TellDebug(requester, "Grind mobs for money", "debug travel");
                auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetGrindTarget1", &context->performanceStack);
                foundTarget = SetGrindTarget(requester, newTarget);                               //Go grind mobs for money    
            }
        }
    }


    //Continue current target.
    if (!foundTarget && (!botPos.isOverworld() || urand(1, 100) > 10))        //90% chance or currently in dungeon.
    {
        ai->TellDebug(requester, "Continue previous target", "debug travel");
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetCurrentTarget", &context->performanceStack);
        foundTarget = SetCurrentTarget(requester, newTarget, oldTarget);             //Extend current target.
    }

    //Get mail
    if (!foundTarget && urand(1, 100) > 70)                                  //30% chance
    {
        if (AI_VALUE(bool, "can get mail"))
        {
            ai->TellDebug(requester, "Get mail", "debug travel");
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetGoTarget2", &context->performanceStack);
            foundTarget = SetGOTypeTarget(requester, newTarget, GAMEOBJECT_TYPE_MAILBOX, "", false);  //Find a mailbox
        }
    }

    //Dungeon in group.
    if (!foundTarget && (!botPos.isOverworld() || urand(1, 100) > 50))      //50% chance or currently in instance.
        if (AI_VALUE(bool, "can fight boss"))
        {
            ai->TellDebug(requester, "Fight boss for loot", "debug travel");
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetBossTarget", &context->performanceStack);
            foundTarget = SetBossTarget(requester, newTarget);                         //Go fight a (dungeon boss)
        }

    //Do quests (start, do, end)
    if (!foundTarget && urand(1, 100) > 5)                                 //95% chance
    {
        ai->TellDebug(requester, "Do questing", "debug travel");
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetQuestTarget", &context->performanceStack);
        foundTarget = SetQuestTarget(requester, newTarget, true, true, true);    //Do any nearby           
    }

    //Explore a nearby unexplored area.
    if (!foundTarget && ai->HasStrategy("explore", BotState::BOT_STATE_NON_COMBAT) && urand(1, 100) > 90)  //10% chance Explore a unexplored sub-zone.
    {
        ai->TellDebug(requester, "Explore unexplored areas", "debug travel");
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetExploreTarget", &context->performanceStack);
        foundTarget = SetExploreTarget(requester, newTarget);
    }

    //Just hang with an npc
    if (!foundTarget && urand(1, 100) > 50)                                 //50% chance
    {
        {
            ai->TellDebug(requester, "Rpg with random npcs", "debug travel");
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetRpgTarget2", &context->performanceStack);
            foundTarget = SetRpgTarget(requester, newTarget);
            if (foundTarget)
                newTarget->SetForced(true);
        }
    }

    if (!foundTarget)
    {
        ai->TellDebug(requester, "Grind random mobs", "debug travel");
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetGrindTarget2", &context->performanceStack);
        foundTarget = SetGrindTarget(requester, newTarget);
    }

    if (!foundTarget)
    {
        ai->TellDebug(requester, "Stop traveling", "debug travel");
        SetNullTarget(newTarget);                                           //Idle a bit.
    }
}

void ChooseTravelTargetAction::setNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget)
{
    if (oldTarget->IsForced() && (oldTarget->GetStatus() == TravelStatus::TRAVEL_STATUS_COOLDOWN || oldTarget->GetStatus() == TravelStatus::TRAVEL_STATUS_EXPIRED) && ai->HasStrategy("travel once", BotState::BOT_STATE_NON_COMBAT))
    {
        ai->ChangeStrategy("-travel once", BotState::BOT_STATE_NON_COMBAT);
        ai->TellPlayerNoFacing(requester, "Arrived at " + oldTarget->GetDestination()->GetTitle());
        SetNullTarget(newTarget);
    }

    if(AI_VALUE2(bool, "can free move to", newTarget->GetPosStr()))
        ReportTravelTarget(requester, newTarget, oldTarget);

    //If we are heading to a creature/npc clear it from the ignore list. 
    if (oldTarget && oldTarget == newTarget && newTarget->GetEntry())
    {
        std::set<ObjectGuid>& ignoreList = context->GetValue<std::set<ObjectGuid>&>("ignore rpg target")->Get();

        for (auto& i : ignoreList)
        {
            if (i.GetEntry() == newTarget->GetEntry())
            {
                ignoreList.erase(i);
            }
        }

        context->GetValue<std::set<ObjectGuid>&>("ignore rpg target")->Set(ignoreList);
    }

    //Actually apply the new target to the travel target used by the bot.
    oldTarget->CopyTarget(newTarget);

    //If we are idling but have a master. Idle only 10 seconds.
    if (ai->GetMaster() && oldTarget->IsActive() && typeid(*oldTarget->GetDestination()) == typeid(NullTravelDestination))
        oldTarget->SetExpireIn(10 * IN_MILLISECONDS);
    else if (oldTarget->IsForced()) //Make sure travel goes into cooldown after getting to the destination.
        oldTarget->SetExpireIn(HOUR * IN_MILLISECONDS);

    //Clear rpg and attack/grind target. We want to travel, not hang around some more.
    RESET_AI_VALUE(GuidPosition,"rpg target");
    RESET_AI_VALUE(ObjectGuid,"attack target");
};

//Tell the master what travel target we are moving towards.
//This should at some point be rewritten to be denser or perhaps logic moved to ->getTitle()
void ChooseTravelTargetAction::ReportTravelTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget)
{
    TravelDestination* destination = newTarget->GetDestination();

    TravelDestination* oldDestination = oldTarget->GetDestination();

    std::ostringstream out;

    if (newTarget->IsForced())
        out << "(Forced) ";

    if (typeid(*destination) == typeid(QuestRelationTravelDestination) || typeid(*destination) == typeid(QuestObjectiveTravelDestination))
    {
        QuestTravelDestination* QuestDestination = (QuestTravelDestination*)destination;
        WorldPosition botLocation(bot);
        CreatureInfo const* cInfo = NULL;
        GameObjectInfo const* gInfo = NULL;

        if (destination->GetEntry() > 0)
            cInfo = ObjectMgr::GetCreatureTemplate(destination->GetEntry());
        else
            gInfo = ObjectMgr::GetGameObjectInfo(destination->GetEntry() * -1);

        std::string Sub;

        if (newTarget->IsGroupCopy())
            out << "Following group ";
        else if(oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->GetDestination()->DistanceTo(botLocation)) << "y";

        out << " for " << QuestDestination->QuestTravelDestination::GetTitle();

        out << " to " << QuestDestination->GetTitle();
    }
    else if (typeid(*destination) == typeid(RpgTravelDestination))
    {
        RpgTravelDestination* RpgDestination = (RpgTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->IsGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->GetDestination()->DistanceTo(botLocation)) << "y";

        out << " for ";

        if (RpgDestination->GetEntry() > 0)
        {
            CreatureInfo const* cInfo = RpgDestination->GetCreatureInfo();

            if (cInfo)
            {
                if ((cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR ) && AI_VALUE2(bool, "group or", "should sell,can sell"))
                    out << "selling items";
                else if ((cInfo->NpcFlags & UNIT_NPC_FLAG_REPAIR) && AI_VALUE2(bool, "group or", "should repair,can repair"))
                    out << "repairing";
                else if ((cInfo->NpcFlags & UNIT_NPC_FLAG_AUCTIONEER) && AI_VALUE2(bool, "group or", "should ah sell,can ah sell"))
                    out << "posting items on the auctionhouse";
                else
                    out << "rpg";
            }
            else
                out << "rpg";
        }
        else
        {
            GameObjectInfo const* gInfo = RpgDestination->GetGoInfo();

            if (gInfo)
            {
                if (gInfo->type == GAMEOBJECT_TYPE_MAILBOX && AI_VALUE(bool, "can get mail"))
                    out << "getting mail";
                else
                    out << "rpg";
            }
            else
                out << "rpg";
        }

        out << " to " << RpgDestination->GetTitle();        
    }
    else if (typeid(*destination) == typeid(ExploreTravelDestination))
    {
        ExploreTravelDestination* ExploreDestination = (ExploreTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->IsGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->GetDestination()->DistanceTo(botLocation)) << "y";

        out << " for exploration";

        out << " to " << ExploreDestination->GetTitle();
    }
    else if (typeid(*destination) == typeid(GrindTravelDestination))
    {
        GrindTravelDestination* GrindDestination = (GrindTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->IsGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->GetDestination()->DistanceTo(botLocation)) << "y";

        out << " for grinding money";

        out << " to " << GrindDestination->GetTitle();
    }
    else if (typeid(*destination) == typeid(BossTravelDestination))
    {
        BossTravelDestination* BossDestination = (BossTravelDestination*)destination;

        WorldPosition botLocation(bot);

        if (newTarget->IsGroupCopy())
            out << "Following group ";
        else if (oldDestination && oldDestination == destination)
            out << "Continuing ";
        else
            out << "Traveling ";

        out << round(newTarget->GetDestination()->DistanceTo(botLocation)) << "y";

        out << " for good loot";

        out << " to " << BossDestination->GetTitle();
    }
    else if (typeid(*destination) == typeid(NullTravelDestination))
    {
        if (!oldTarget->GetDestination() || typeid(oldTarget->GetDestination()) != typeid(NullTravelDestination))
        {
            out.clear();
            out << "No where to travel. Idling a bit.";
        }
    }

    if (out.str().empty())
        return;

    ai->TellPlayerNoFacing(requester, out,PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    std::string message = out.str().c_str();

    if (sPlayerbotAIConfig.hasLog("travel_map.csv"))
    {
        WorldPosition botPos(bot);
        WorldPosition destPos = *newTarget->GetPosition();

        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        out << std::fixed << std::setprecision(2);

        out << std::to_string(bot->getRace()) << ",";
        out << std::to_string(bot->getClass()) << ",";
        float subLevel = ai->GetLevelFloat();

        out << subLevel << ",";

        if (!destPos)
            destPos = botPos;

        botPos.printWKT({ botPos,destPos }, out, 1);

        if (typeid(*destination) == typeid(NullTravelDestination))
            out << "0,";
        else
            out << round(newTarget->GetDestination()->DistanceTo(botPos)) << ",";

        out << "1," << "\"" << destination->GetTitle() << "\",\"" << message << "\"";

        if (typeid(*destination) == typeid(NullTravelDestination))
            out << ",none";
        else if (typeid(*destination) == typeid(QuestTravelDestination))
            out << ",quest";
        else if (typeid(*destination) == typeid(QuestRelationTravelDestination))
            out << ",questgiver";
        else if (typeid(*destination) == typeid(QuestObjectiveTravelDestination))
            out << ",objective";
        else  if (typeid(*destination) == typeid(RpgTravelDestination))
        {
            RpgTravelDestination* RpgDestination = (RpgTravelDestination*)destination;
            if (RpgDestination->GetEntry() > 0)
            {
                CreatureInfo const* cInfo = RpgDestination->GetCreatureInfo();

                if (cInfo)
                {
                    if ((cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR) && AI_VALUE2(bool, "group or", "should sell,can sell"))
                        out << ",sell";
                    else if ((cInfo->NpcFlags & UNIT_NPC_FLAG_REPAIR) && AI_VALUE2(bool, "group or", "should repair,can repair"))
                        out << ",repair";
                    else if ((cInfo->NpcFlags & UNIT_NPC_FLAG_AUCTIONEER) && AI_VALUE2(bool, "group or", "should ah sell,can ah sell"))
                        out << ",ah";
                    else
                        out << ",rpg";
                }
                else
                    out << ",rpg";
            }
            else
            {
                GameObjectInfo const* gInfo = RpgDestination->GetGoInfo();

                if (gInfo)
                {
                    if (gInfo->type == GAMEOBJECT_TYPE_MAILBOX && AI_VALUE(bool, "can get mail"))
                        out << ",mail";
                    else
                        out << ",rpg";
                }
                else
                    out << ",rpg";
            }
        }
        else if (typeid(*destination) == typeid(ExploreTravelDestination))
            out << ",explore";
        else if (typeid(*destination) == typeid(GrindTravelDestination))
            out << ",grind";
        else if (typeid(*destination) == typeid(BossTravelDestination))
            out << ",boss";
        else
            out << ",unknown";

        sPlayerbotAIConfig.log("travel_map.csv", out.str().c_str());

        WorldPosition lastPos = AI_VALUE2(WorldPosition, "custom position", "last choose travel");

        if (lastPos)
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
            out << bot->GetName() << ",";
            out << std::fixed << std::setprecision(2);

            out << std::to_string(bot->getRace()) << ",";
            out << std::to_string(bot->getClass()) << ",";
            float subLevel = ai->GetLevelFloat();

            out << subLevel << ",";

            WorldPosition lastPos = AI_VALUE2(WorldPosition, "custom position", "last choose travel");

            botPos.printWKT({ lastPos, botPos }, out, 1);

            if (typeid(*destination) == typeid(NullTravelDestination))
                out << "0,";
            else
                out << round(newTarget->GetDestination()->DistanceTo(botPos)) << ",";

            out << "0," << "\"" << destination->GetTitle() << "\",\""<< message << "\"";

            sPlayerbotAIConfig.log("travel_map.csv", out.str().c_str());
        }

        SET_AI_VALUE2(WorldPosition, "custom position", "last choose travel", botPos);
    }
}

//Select only those points that are in sight distance or failing that a multiplication of the sight distance.
std::vector<WorldPosition*> ChooseTravelTargetAction::getLogicalPoints(Player* requester, std::vector<WorldPosition*>& travelPoints)
{
    auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "getLogicalPoints", &context->performanceStack);
    std::vector<WorldPosition*> retvec;

    static std::vector<float> distanceLimits = { sPlayerbotAIConfig.sightDistance, 4 * sPlayerbotAIConfig.sightDistance, 10 * sPlayerbotAIConfig.sightDistance, 20 * sPlayerbotAIConfig.sightDistance, 50 * sPlayerbotAIConfig.sightDistance, 100 * sPlayerbotAIConfig.sightDistance, 10000 * sPlayerbotAIConfig.sightDistance };

    std::vector<std::vector<WorldPosition*>> partitions;

    for (uint8 l = 0; l < distanceLimits.size(); l++)
        partitions.push_back({});

    WorldPosition centerLocation;

    int32 botLevel = (int)bot->GetLevel();

    bool canFightElite = AI_VALUE(bool, "can fight elite");

    if (AI_VALUE(bool, "can fight boss"))
        botLevel += 5;
    else if (canFightElite)
        botLevel += 2;
    else if (!AI_VALUE(bool, "can fight equal"))
    {
        botLevel -= (2 + AI_VALUE(uint32, "death count"));
    }

    if (botLevel < 6)
        botLevel = 6;

    if (requester)
        centerLocation = WorldPosition(requester);
    else
        centerLocation = WorldPosition(bot);

    {
       auto pmo1 = sPerformanceMonitor.start(PERF_MON_VALUE, "Shuffle", &context->performanceStack);
       if (travelPoints.size() > 50)
          std::shuffle(travelPoints.begin(), travelPoints.end(), *GetRandomGenerator());
    }

    uint8 checked = 0;

    //Loop over all points
    for (auto pos : travelPoints)
    {
        if (pos->getMapId() == bot->GetMapId())
        {
            auto pmo1 = sPerformanceMonitor.start(PERF_MON_VALUE, "AreaLevel", &context->performanceStack);

            int32 areaLevel = pos->getAreaLevel();

            if (!pos->isOverworld() && !canFightElite)
                areaLevel += 10;

            if (!areaLevel || botLevel < areaLevel) //Skip points that are in a area that is too high level.
                continue;
        }

        GuidPosition* guidP = dynamic_cast<GuidPosition*>(pos);

        auto pmo2 = sPerformanceMonitor.start(PERF_MON_VALUE, "IsEventUnspawned", &context->performanceStack);
        if (guidP && guidP->IsEventUnspawned()) //Skip points that are not spawned due to events.
        {
            continue;
        }
        pmo2.reset();

        auto pmo3 = sPerformanceMonitor.start(PERF_MON_VALUE, "distancePartition", &context->performanceStack);
        centerLocation.distancePartition(distanceLimits, pos, partitions); //Partition point in correct distance bracket.
        pmo3.reset();

        if (checked++ > 50)
            break;
    }

    pmo.reset();

    for (uint8 l = 0; l < distanceLimits.size(); l++)
    {
        if (partitions[l].empty() || !urand(0, 10)) //Return the first non-empty bracket with 10% chance to skip a higher bracket.
        {
            if(!partitions[l].empty())
            ai->TellDebug(requester, "Skipping " + std::to_string(partitions[l].size()) + " points at range " + std::to_string(uint32(distanceLimits[l])), "debug travel");
            continue;
        }

        ai->TellDebug(requester, "Selecting " + std::to_string(partitions[l].size()) + " points at range " + std::to_string(uint32(distanceLimits[l])), "debug travel");
        return partitions[l];
    }

    if (requester && centerLocation.fDist(bot) > 500.0f) //Try again with bot as center.
    {
        ai->TellDebug(requester, "No points near " + std::string(requester->GetName()) + " trying near myself.", "debug travel");
        return getLogicalPoints(nullptr, travelPoints);
    }

    return partitions.back();
}

//Sets the target to the best destination.
bool ChooseTravelTargetAction::SetBestTarget(Player* requester, TravelTarget* target, std::vector<TravelDestination*>& TravelDestinations)
{
    if (TravelDestinations.empty())
        return false;

    WorldPosition botLocation(bot);

    std::vector<WorldPosition*> travelPoints;

    //Select all points from the selected destinations
    for (auto& activeTarget : TravelDestinations)
    {
        if (!activeTarget->IsActive(bot, info))
            continue;

        std::vector<WorldPosition*> points = activeTarget->GetPoints();
        for (WorldPosition* point : points)
        {
            if (point && point->isValid())
            {
                travelPoints.push_back(point);
            }
        }
    }

    ai->TellDebug(requester, std::to_string(travelPoints.size()) + " points total.", "debug travel");

    if (travelPoints.empty()) //No targets or no points.
        return false;

    if (TravelDestinations.size() == 1 && travelPoints.size() == 1)
    {
        target->SetTarget(TravelDestinations.front(), travelPoints.front());
        return target->IsActive();
    }

    travelPoints = getLogicalPoints(requester, travelPoints);

    if (travelPoints.empty())
        return false;

    travelPoints = botLocation.GetNextPoint(travelPoints); //Pick a good point.

    //Pick the best destination and point (random shuffle).

    for (auto destination : TravelDestinations) //Pick the destination that has this point.
        if (destination->HasPoint(travelPoints.front()))
        {
            TravelDestinations.front() = destination;
            break;
        }

    target->SetTarget(TravelDestinations.front(), travelPoints.front());

    ai->TellDebug(requester, "Point at " + std::to_string(uint32(target->Distance(bot))) + "y selected.", "debug travel");

    return target->IsActive();
}

bool ChooseTravelTargetAction::SetGroupTarget(Player* requester, TravelTarget* target)
{
    std::vector<TravelDestination*> activeDestinations;
    std::vector<WorldPosition*> activePoints;

    std::list<ObjectGuid> groupPlayers;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (ref->getSource() != bot)
        {
            if (ref->getSubGroup() != bot->GetSubGroup())
            {
                groupPlayers.push_back(ref->getSource()->GetObjectGuid());
            }
            else
            {
                groupPlayers.push_front(ref->getSource()->GetObjectGuid());
            }
        }
    }

    //Find targets of the group.
    for (auto& member : groupPlayers)
    {
        Player* player = sObjectMgr.GetPlayer(member);

        if (!player)
            continue;

        if (!player->GetPlayerbotAI())
            continue;

        if (!player->GetPlayerbotAI()->GetAiObjectContext())
            continue;

        TravelTarget* groupTarget = player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

        if (groupTarget->IsGroupCopy())
            continue;

        if (!groupTarget->IsActive())
            continue;

        if (!groupTarget->GetDestination()->IsActive(bot, info) || typeid(*groupTarget->GetDestination()) == typeid(RpgTravelDestination))
            continue;

        activeDestinations.push_back(groupTarget->GetDestination());
        activePoints.push_back(groupTarget->GetPosition());
    }

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(activeDestinations.size()) + " group targets found.");

    bool hasTarget = SetBestTarget(requester, target, activeDestinations);

    if (hasTarget)
        target->SetGroupCopy();

    return hasTarget;
}

bool ChooseTravelTargetAction::SetCurrentTarget(Player* requester, TravelTarget* target, TravelTarget* oldTarget)
{
    TravelDestination* oldDestination = oldTarget->GetDestination();

    if (oldTarget->IsMaxRetry(false))
        return false;

    if (!oldDestination) //Does this target have a destination?
        return false;

    if (!oldDestination->IsActive(bot, info)) //Is the destination still valid?
        return false;

    std::vector<TravelDestination*> TravelDestinations = { oldDestination };

    if (!SetBestTarget(requester, target, TravelDestinations))
        return false;
   
    target->SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);
    target->SetRetry(false, oldTarget->GetRetryCount(false) + 1);

    return target->IsActive();
}

bool ChooseTravelTargetAction::SetQuestTarget(Player* requester, TravelTarget* target, bool newQuests, bool activeQuests, bool completedQuests)
{
    std::vector<TravelDestination*> TravelDestinations;

    bool onlyClassQuest = !urand(0, 10);

    if (newQuests)
    {
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "getQuestTravelDestinations1", &context->performanceStack);
        TravelDestinations = sTravelMgr.GetDestinations(info, typeid(QuestTravelDestination), 0, (uint8)QuestTravelSubEntry::QUESTGIVER,0,true, 400 + bot->GetLevel() * 10); //Prefer new quests near the player at lower levels.
    }

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " new quest destinations found.");

    if (activeQuests || completedQuests)
    {
        QuestStatusMap& questMap = bot->getQuestStatusMap();

        //Find destinations related to the active quests.
        for (auto& quest : questMap)
        {
            if (quest.second.m_rewarded)
                continue;

            uint32 questId = quest.first;
            QuestStatusData* questStatus = &quest.second;

            Quest const* questTemplate = sObjectMgr.GetQuestTemplate(questId);

            if (!activeQuests && !bot->CanRewardQuest(questTemplate, false))
                continue;

            if (!completedQuests && bot->CanRewardQuest(questTemplate, false))
                continue;

            //Find quest takers or objectives
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "getQuestTravelDestinations2", &context->performanceStack);

            std::vector<TravelDestination*> questDestinations = sTravelMgr.GetDestinations(info, typeid(QuestTravelDestination), questId, 0, 0, true, 0);

            pmo.reset();

            if (onlyClassQuest && TravelDestinations.size() && questDestinations.size()) //Only do class quests if we have any.
            {
                QuestTravelDestination* firstDestination = (QuestTravelDestination * )TravelDestinations.front();
                if (firstDestination->IsClassQuest() && !questTemplate->GetRequiredClasses())
                    continue;

                if (!firstDestination->IsClassQuest() && questTemplate->GetRequiredClasses())
                    TravelDestinations.clear();
            }

            TravelDestinations.insert(TravelDestinations.end(), questDestinations.begin(), questDestinations.end());
        }
    }

    if (newQuests && TravelDestinations.empty())
    {
        auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "getQuestTravelDestinations3", &context->performanceStack);
        TravelDestinations = sTravelMgr.GetDestinations(info, typeid(QuestTravelDestination), 0, (uint8)QuestTravelSubEntry::QUESTGIVER); // If we really don't find any new quests look futher away.
    }

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " quest destinations found.");

    return SetBestTarget(requester, target, TravelDestinations);
}

bool ChooseTravelTargetAction::SetRpgTarget(Player* requester, TravelTarget* target)
{
    //Find rpg npcs
    std::vector<TravelDestination*> TravelDestinations = sTravelMgr.GetDestinations(info, typeid(RpgTravelDestination));

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " rpg destinations found.");

    return SetBestTarget(requester, target, TravelDestinations);
}

bool ChooseTravelTargetAction::SetGrindTarget(Player* requester, TravelTarget* target)
{
    //Find grind mobs.
    std::vector<TravelDestination*> TravelDestinations = sTravelMgr.GetDestinations(info, typeid(GrindTravelDestination), 0,0,0, true, 600 + bot->GetLevel() * 400);

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " grind destinations found.");

    return SetBestTarget(requester, target, TravelDestinations);
}

bool ChooseTravelTargetAction::SetBossTarget(Player* requester, TravelTarget* target)
{
    //Find boss mobs.
    std::vector<TravelDestination*> TravelDestinations = sTravelMgr.GetDestinations(info, typeid(BossTravelDestination));

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " boss destinations found.");

    return SetBestTarget(requester, target, TravelDestinations);
}

bool ChooseTravelTargetAction::SetExploreTarget(Player* requester, TravelTarget* target)
{
    //Find exploration locations (middle of a sub-zone).
    std::vector<TravelDestination*> TravelDestinations = sTravelMgr.GetDestinations(info, typeid(ExploreTravelDestination));

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " explore destinations found.");

    return SetBestTarget(requester, target, TravelDestinations);
}

char* strstri(const char* haystack, const char* needle);

bool ChooseTravelTargetAction::SetNpcFlagTarget(Player* requester, TravelTarget* target, std::vector<NPCFlags> flags, std::string name, std::vector<uint32> items, bool force)
{
    WorldPosition pos = WorldPosition(bot);
    WorldPosition* botPos = &pos;

    std::vector<TravelDestination*> TravelDestinations;

    //Loop over all npcs.
    for (auto& d : sTravelMgr.GetDestinations(info, typeid(RpgTravelDestination), 0, 0, 0, false))
    {
        if (d->GetEntry() <= 0)
            continue;

        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(d->GetEntry());

        if (!cInfo)
            continue;

        //Check if the npc has any of the required flags.
        bool foundFlag = false;
        for(auto flag : flags)
            if (cInfo->NpcFlags & flag)
            {
                foundFlag = true;
                break;
            }

        if (!foundFlag)
            continue;

        //Check if the npc has (part of) the required name.
        if (!name.empty() && !strstri(cInfo->Name, name.c_str()) && !strstri(cInfo->SubName, name.c_str()))
            continue;

        //Check if the npc sells any of the wanted items.
        if (!items.empty())
        {
            bool foundItem = false;
            VendorItemData const* vItems = nullptr;
            VendorItemData const* tItems = nullptr;

            vItems = sObjectMgr.GetNpcVendorItemList(d->GetEntry());

//#ifndef MANGOSBOT_ZERO    
            uint32 vendorId = cInfo->VendorTemplateId;
            if (vendorId)
                tItems = sObjectMgr.GetNpcVendorTemplateItemList(vendorId);
//#endif

            for (auto item : items)
            {
                if (vItems && !vItems->Empty())
                for(auto vitem : vItems->m_items) 
                   if (vitem->item == item)
                    {
                        foundItem = true;
                        break;
                    }
                if(tItems && !tItems->Empty())
                for (auto titem : tItems->m_items)
                    if (titem->item == item)
                    {
                        foundItem = true;
                        break;
                    }
            }

            if (!foundItem)
                continue;
        }

        //Check if the npc is friendly.
        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
        ReputationRank reaction = ai->getReaction(factionEntry);

        if (reaction  < REP_NEUTRAL)
            continue;

        TravelDestinations.push_back(d);
    }

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " npc flag targets found.");

    bool isActive = SetBestTarget(requester, target, TravelDestinations);

    if (!target->GetDestination())
        return false;

    if (force)
    {
        target->SetForced(true);
        return true;
    }

    return isActive;
}

bool ChooseTravelTargetAction::SetGOTypeTarget(Player* requester, TravelTarget* target, GameobjectTypes type, std::string name, bool force)
{
    WorldPosition pos = WorldPosition(bot);
    WorldPosition* botPos = &pos;

    std::vector<TravelDestination*> TravelDestinations;

    //Loop over all npcs.
    for (auto& d : sTravelMgr.GetDestinations(info, typeid(RpgTravelDestination)))
    {
        if (d->GetEntry() >= 0)
            continue;

        GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(-1 * d->GetEntry());

        if (!gInfo)
            continue;

        //Check if the object has any of the required type.
        if (gInfo->type != type)
            continue;

        //Check if the npc has (part of) the required name.
        if (!name.empty() && !strstri(gInfo->name, name.c_str()))
            continue;        

        TravelDestinations.push_back(d);
    }

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(TravelDestinations.size()) + " go type targets found.");

    bool isActive = SetBestTarget(requester, target, TravelDestinations);

    if (!target->GetDestination())
        return false;

    if (force)
    {
        target->SetForced(true);
        return true;
    }

    return isActive;
}


bool ChooseTravelTargetAction::SetNullTarget(TravelTarget* target)
{
    sTravelMgr.SetNullTravelTarget(target);
    
    return true;
}

std::vector<std::string> split(const std::string& s, char delim);
char* strstri(const char* haystack, const char* needle);

//Find a destination based on (part of) it's name. Includes zones, ncps and mobs. Picks the closest one that matches.
TravelDestination* ChooseTravelTargetAction::FindDestination(Player* bot, std::string name, bool zones, bool npcs, bool quests, bool mobs, bool bosses)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();

    PlayerTravelInfo info(bot);

    AiObjectContext* context = ai->GetAiObjectContext();

    std::vector<TravelDestination*> dests;

    //Quests
    if (quests)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, typeid(QuestTravelDestination),0,0,0,false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Zones
    if (zones)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, typeid(ExploreTravelDestination), 0, 0, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Npcs
    if (npcs)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, typeid(RpgTravelDestination), 0, 0, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Mobs
    if (mobs)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, typeid(GrindTravelDestination), 0, 0, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Bosses
    if (bosses)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, typeid(BossTravelDestination), 0, 0, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    WorldPosition botPos(bot);

    if (dests.empty())
        return nullptr;

    TravelDestination* dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });

    return dest;
};

bool ChooseTravelTargetAction::isUseful()
{
    if (!ai->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
            return false;

    if (AI_VALUE(bool, "has available loot"))
    {
        LootObject lootObject = AI_VALUE(LootObjectStack*, "available loot")->GetLoot(sPlayerbotAIConfig.lootDistance);
        if (lootObject.IsLootPossible(bot))
            return false;
    }

    return true;
}


bool ChooseTravelTargetAction::needForQuest(Unit* target)
{
    bool justCheck = (bot->GetObjectGuid() == target->GetObjectGuid());

    QuestStatusMap& questMap = bot->getQuestStatusMap();
    for (auto& quest : questMap)
    {
        const Quest* questTemplate = sObjectMgr.GetQuestTemplate(quest.first);
        if (!questTemplate)
            continue;

        uint32 questId = questTemplate->GetQuestId();

        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questId)))
        {
            if (!justCheck && !target->HasInvolvedQuest(questId))
                continue;

            return true;
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            QuestStatusData questStatus = quest.second;

            if (questTemplate->GetQuestLevel() > (int)bot->GetLevel())
                continue;

            for (int j = 0; j < QUEST_OBJECTIVES_COUNT; j++)
            {
                int32 entry = questTemplate->ReqCreatureOrGOId[j];

                if (entry && entry > 0)
                {
                    int required = questTemplate->ReqCreatureOrGOCount[j];
                    int available = questStatus.m_creatureOrGOcount[j];

                    if(required && available < required && (target->GetEntry() == entry || justCheck))
                        return true;
                }         

                if (justCheck)
                {
                    int32 itemId = questTemplate->ReqItemId[j];

                    if (itemId && itemId > 0)
                    {
                        int required = questTemplate->ReqItemCount[j];
                        int available = questStatus.m_itemcount[j];

                        if (required && available < required)
                            return true;
                    }
                }
            }

            if (!justCheck)
            {
                CreatureInfo const* data = sObjectMgr.GetCreatureTemplate(target->GetEntry());

                if (data)
                {
                    uint32 lootId = data->LootId;

                    if (lootId)
                    {
                        if (LootTemplates_Creature.HaveQuestLootForPlayer(lootId, bot))
                            return true;
                    }
                }
            }
        }

    }
    return false;
}

bool ChooseTravelTargetAction::needItemForQuest(uint32 itemId, const Quest* questTemplate, const QuestStatusData* questStatus)
{
    for (int i = 0; i < QUEST_OBJECTIVES_COUNT; i++)
    {
        if (questTemplate->ReqItemId[i] != itemId)
            continue;

        int required = questTemplate->ReqItemCount[i];
        int available = questStatus->m_itemcount[i];

        if (!required)
            continue;

        return available < required;
    }

    return false;
}

bool FocusTravelTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();

    if (text == "?")
    {
        std::set<uint32> questIds = AI_VALUE(focusQuestTravelList, "focus travel target");
        std::ostringstream out;
        if (questIds.empty())
            out << "No quests selected.";
        else
        {
            out << "I will try to only do the following " << questIds.size() << " quests:";

            for (auto questId : questIds)
            {
                const Quest* quest = sObjectMgr.GetQuestTemplate(questId);

                if (quest)
                    out << ChatHelper::formatQuest(quest);
            }

        }
        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }

    std::set<uint32> questIds = ChatHelper::ExtractAllQuestIds(text);

    if (questIds.empty() && !text.empty())
    {
        if (Qualified::isValidNumberString(text))
            questIds.insert(stoi(text));
        else
        {
            std::vector<std::string> qualifiers = Qualified::getMultiQualifiers(text, ",");

            for (auto& qualifier : qualifiers)
                if (Qualified::isValidNumberString(qualifier))
                    questIds.insert(stoi(text));
        }
    }

    SET_AI_VALUE(focusQuestTravelList, "focus travel target", questIds);

    if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellError(requester, "travel strategy disabled bot needs this to actually do the quest.");

    if (!ai->HasStrategy("rpg quest", BotState::BOT_STATE_NON_COMBAT))
        ai->TellError(requester, "rpg quest strategy disabled bot needs this to actually do the quest.");

    std::ostringstream out;
    if (questIds.empty())
        out << "I will now do all quests.";
    else
    {
        out << "I will now only try to do the following " << questIds.size() << " quests:";

        for (auto questId : questIds)
        {
            const Quest* quest = sObjectMgr.GetQuestTemplate(questId);

            if (quest)
                out << ChatHelper::formatQuest(quest);
        }

    }
    ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    TravelTarget* oldTarget = AI_VALUE(TravelTarget*, "travel target");

    oldTarget->SetExpireIn(1000);
    
    return true;
}
