
#include "playerbot/playerbot.h"
#include "playerbot/LootObjectStack.h"
#include "ChooseTravelTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/TravelValues.h"
#include <iomanip>

using namespace ai;

bool ChooseTravelTargetAction::Execute(Event& event)
{
    if (AI_VALUE(TravelTarget*, "travel target")->IsPreparing())
        return false;

    context->ClearValues("travel destinations"); //Remove all earlier destination results so those actions get get again when needed.

    if (!event.getOwner() && AI_VALUE(TravelTarget*, "travel target")->IsActive()) //Do not pick new target automatically when it is active.
        return false;
    
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

    /*
    foundTarget = SetGroupTarget(requester, newTarget);                                 //Join groups members

    
    //Empty bags/repair
    if (!foundTarget && urand(1, 100) > 10)                                  //90% chance
    {
        TravelDestinationPurpose rpgPurpose = TravelDestinationPurpose::None;
        if (AI_VALUE2(bool, "group or", "should sell,can sell,following party,near leader")) //One of party members wants to sell items (full bags).
        {
            ai->TellDebug(requester, "Group needs to sell items (full bags)", "debug travel");
            rpgPurpose = TravelDestinationPurpose::Vendor;
        }
        else if (AI_VALUE2(bool, "group or", "should repair,can repair,following party,near leader")) //One of party members wants to repair.
        {
            ai->TellDebug(requester, "Group needs to repair", "debug travel");
            rpgPurpose = TravelDestinationPurpose::Repair;
        }
        else if (AI_VALUE2(bool, "group or", "should ah sell,can ah sell,following party,near leader") && bot->GetLevel() > 5) //One of party members wants to sell items to AH (full bags).
        {
            ai->TellDebug(requester, "Group needs to ah items (full bags)", "debug travel");
            rpgPurpose = TravelDestinationPurpose::AH;
        }
        else if (rpgPurpose != TravelDestinationPurpose::None && ai->HasStrategy("free", BotState::BOT_STATE_NON_COMBAT))
        {
            if (AI_VALUE(bool, "should sell") && AI_VALUE(bool, "can sell")) //Bot wants to sell (full bags).
            {
                ai->TellDebug(requester, "Bot needs to sell items (full bags)", "debug travel");
                rpgPurpose = TravelDestinationPurpose::Vendor;
            }
            if (AI_VALUE(bool, "should ah sell") && AI_VALUE(bool, "can ah sell")) //Bot wants to ah sell (repariable item that it wants to ah).
            {
                ai->TellDebug(requester, "Bot needs to ah items (full bags)", "debug travel");
                rpgPurpose = TravelDestinationPurpose::AH;
            }
            else if (AI_VALUE(bool, "should repair") && AI_VALUE(bool, "can repair")) //Bot wants to repair.
            {
                ai->TellDebug(requester, "Bot needs to repair", "debug travel");
                rpgPurpose = TravelDestinationPurpose::Repair;
            }
        }
                     
        if (rpgPurpose != TravelDestinationPurpose::None)
        {
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetRpgTarget1", &context->performanceStack);
            foundTarget = SetRpgTarget(requester, newTarget, rpgPurpose); //Go to town to sell items or repair
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

        DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, "Tarren Mill");
        if (dests.size())
        {
            TravelDestination* dest = *std::min_element(dests.begin(), dests.end(), [pos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(pos) < j->DistanceTo(pos); });
        
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
            foundTarget = SetRpgTarget(requester, newTarget, TravelDestinationPurpose::Mail);  //Find a mailbox
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
            foundTarget = SetRpgTarget(requester, newTarget, TravelDestinationPurpose::Mail);  //Find a mailbox
        }
    }

    if ((int)urand(1, 100) > (90 - (40 * (!bot->GetGroup()))))
    {
        std::vector<SkillType> gatheringSkills = { SKILL_MINING, SKILL_SKINNING, SKILL_HERBALISM, SKILL_FISHING };

        SkillType needSkillup = SKILL_NONE;

        for (auto& skill : gatheringSkills)
        {
            if (bot->GetSkillValue(skill) < std::min(bot->GetSkillMax(skill), bot->GetSkillMaxForLevel(bot)))
            {
                needSkillup = skill;
                break;
            }
        }

        if (needSkillup != SKILL_NONE)
        {
            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetGatherTarget", &context->performanceStack);
            if (needSkillup == SKILL_MINING)
            {
                foundTarget = SetGatherTarget(requester, newTarget, TravelDestinationPurpose::GatherMining);
                ai->TellDebug(requester, "Gather for mining skillup ", "debug travel");
            }
            if (needSkillup == SKILL_SKINNING)
            {
                foundTarget = SetGatherTarget(requester, newTarget, TravelDestinationPurpose::GatherSkinning);
                ai->TellDebug(requester, "Gather for skinning skillup ", "debug travel");
            }
            if (needSkillup == SKILL_HERBALISM)
            {
                foundTarget = SetGatherTarget(requester, newTarget, TravelDestinationPurpose::GatherHerbalism);
                ai->TellDebug(requester, "Gather for herbalism skillup ", "debug travel");
            }
            if (needSkillup == SKILL_FISHING)
            {
                foundTarget = SetGatherTarget(requester, newTarget, TravelDestinationPurpose::GatherFishing);
                ai->TellDebug(requester, "Gather for fishing skillup ", "debug travel");
            }
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
            foundTarget = SetRpgTarget(requester, newTarget, TravelDestinationPurpose::GenericRpg, false);
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
    */
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
    context->ClearValues("travel destinations");
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
        else if (typeid(*destination) == typeid(GatherTravelDestination))
            out << ",gather";
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

//Sets the target to the best destination.
bool ChooseTravelTargetAction::SetBestTarget(Player* requester, TravelTarget* target, PartitionedTravelList& travelPartitions, bool onlyActive)
{
    std::unordered_map<TravelDestination*, bool> isActive;

    bool hasTarget = false;

    for (auto& [partition, travelPointList] : travelPartitions)
    {
        for (auto& [destination, position, distance] : travelPointList)
        {
            if (isActive.find(destination) != isActive.end() && !isActive[destination])
                continue;

            if(isActive[destination] = destination->IsActive(bot, info))
            {
                if (partition != travelPartitions.end()->first && !urand(0, 10)) //10% chance to skip to a longer partition.            
                    break;

                target->SetTarget(destination, position);
                hasTarget = true;
                break;
            }
        }

        if (hasTarget)
            break;
    }         
     
    if(hasTarget)
        ai->TellDebug(requester, "Point at " + std::to_string(uint32(target->Distance(bot))) + "y selected.", "debug travel");

    return hasTarget;
}

char* strstri(const char* haystack, const char* needle);

bool ChooseTravelTargetAction::SetNpcFlagTarget(Player* requester, TravelTarget* target, std::vector<NPCFlags> flags, std::string name, std::vector<uint32> items, bool force)
{
    auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetNpcFlagTarget", &context->performanceStack);
    WorldPosition pos = WorldPosition(bot);
    WorldPosition* botPos = &pos;

    PartitionedTravelList TravelDestinations;
    uint32 found = 0;

    //Loop over all npcs.
    for (auto& [partition, points] : sTravelMgr.GetPartitions(pos, travelPartitions, info, (uint32)TravelDestinationPurpose::GenericRpg, 0, false))
    {
        for (auto& [dest, position, distance] : points)
        {
            if (dest->GetEntry() <= 0)
                continue;

            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(dest->GetEntry());

            if (!cInfo)
                continue;

            //Check if the npc has any of the required flags.
            bool foundFlag = false;
            for (auto flag : flags)
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

                vItems = sObjectMgr.GetNpcVendorItemList(dest->GetEntry());

                //#ifndef MANGOSBOT_ZERO    
                uint32 vendorId = cInfo->VendorTemplateId;
                if (vendorId)
                    tItems = sObjectMgr.GetNpcVendorTemplateItemList(vendorId);
                //#endif

                for (auto item : items)
                {
                    if (vItems && !vItems->Empty())
                        for (auto vitem : vItems->m_items)
                            if (vitem->item == item)
                            {
                                foundItem = true;
                                break;
                            }
                    if (tItems && !tItems->Empty())
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

            if (reaction < REP_NEUTRAL)
                continue;

            TravelDestinations[partition].push_back(TravelPoint(dest, position, distance));
            found++;
        }
    }

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(found) + " npc flag targets found.");

    bool isActive = SetBestTarget(requester, target, TravelDestinations, false);

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
DestinationList ChooseTravelTargetAction::FindDestination(PlayerTravelInfo info, std::string name, bool zones, bool npcs, bool quests, bool mobs, bool bosses)
{
    DestinationList dests;

    //Quests
    if (quests)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::QuestGiver, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Zones
    if (zones)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::Explore, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Npcs
    if (npcs)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::GenericRpg, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Mobs
    if (mobs)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::Grind, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Bosses
    if (bosses)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::Boss, 0, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    if (dests.empty())
        return {};

    return dests;
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

bool ChooseGroupTravelTargetAction::Execute(Event& event)
{
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

    PlayerTravelInfo info(bot);

    PartitionedTravelList groupTargets;

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

        TravelTarget* groupTarget = PAI_VALUE(TravelTarget*, "travel target");

        if (groupTarget->IsGroupCopy())
            continue;

        if (!groupTarget->IsActive())
            continue;

        if (!groupTarget->GetDestination()->IsActive(bot, info) || typeid(*groupTarget->GetDestination()) == typeid(RpgTravelDestination))
            continue;

        groupTargets[0].push_back(TravelPoint(groupTarget->GetDestination(), groupTarget->GetPosition(), groupTarget->GetPosition()->distance(bot)));
    }

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT))
        ai->TellPlayerNoFacing(requester, std::to_string(groupTargets[0].size()) + " group targets found.");

    TravelTarget* oldTarget = AI_VALUE(TravelTarget*, "travel target");

    TravelTarget newTarget = TravelTarget(ai);

    bool hasTarget = SetBestTarget(requester, &newTarget, groupTargets);

    if (hasTarget)
        newTarget.SetGroupCopy();

    //If the new target is not active we failed.
    if (!newTarget.IsActive() && !newTarget.IsForced())
        return false;

    setNewTarget(requester, &newTarget, oldTarget);

    return oldTarget->IsActive();
}

bool RefreshTravelTargetAction::Execute(Event& event)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    TravelDestination* oldDestination = target->GetDestination();

    if (target->IsMaxRetry(false))
        return false;

    if (!oldDestination) //Does this target have a destination?
        return false;

    if (!oldDestination->IsActive(bot, info)) //Is the destination still valid?
        return false;

    std::vector<WorldPosition*> newPositions = oldDestination->NextPoint(*target->GetPosition());

    if (newPositions.empty())
        return false;

    target->SetTarget(oldDestination, newPositions.front());

    target->SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);
    target->SetRetry(false, target->GetRetryCount(false) + 1);

    return target->IsActive();
}

bool ChooseAsyncTravelTargetAction::WaitForDestinations()
{
    if (hasDestinations)
        return false;  //We already fetched destinations. Continue with processing.

    if (!futureDestinations.valid())
        return false;  //We have requested no destinations yet. Request them.

    if (futureDestinations.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) 
        return true;   //We are waiting for destinations.

    destinationList = futureDestinations.get();

    //ai->TellPlayer(ai->GetMaster(), "Got new destinations for " + std::to_string((uint32)actionPurpose));

    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);
    SET_AI_VALUE2(PartitionedTravelList, "travel destinations", (uint32)actionPurpose, destinationList);

    hasDestinations = true;

    return false;     //We just got destinations (empty or full) process them.
}

bool ChooseAsyncTravelTargetAction::SetBestDestination(Event& event)
{
    if (destinationList.empty())
        return false; //Nothing to set. Try getting new destinations.

    TravelTarget newTarget = TravelTarget(ai);
    SetBestTarget(bot, &newTarget, destinationList);

    TravelTarget* oldTarget = AI_VALUE(TravelTarget*, "travel target");

    if (!newTarget.IsActive() && !newTarget.IsForced())
        return false;

    RESET_AI_VALUE2(PartitionedTravelList, "travel destinations", (uint32)actionPurpose);
    context->ClearValues("need travel purpose");
    context->ClearValues("should travel named");
    SetDuration(2000);

    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);

    //ai->TellPlayer(ai->GetMaster(), "Set new destinations for " + std::to_string((uint32)actionPurpose));

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    setNewTarget(requester, &newTarget, oldTarget);

    return true;
}

bool ChooseAsyncTravelTargetAction::RequestNewDestinations(Event& event)
{
    if (hasDestinations)
        return false;

    if (destinationList.size())
        return false;

    if (futureDestinations.valid())
        return false;

    if (PassTrough())
        return false;

    //ai->TellPlayer(ai->GetMaster(), "Fetching new destinations for " + std::to_string((uint32)actionPurpose));
    
    WorldPosition center = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);

    futureDestinations = std::async(std::launch::async, [partitions = travelPartitions, travelInfo = info, center, purpose = actionPurpose]() {return sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)purpose); });

    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);

    return true;
}

bool ChooseAsyncTravelTargetAction::Execute(Event& event)
{
    if (!event.getOwner() && AI_VALUE(TravelTarget*, "travel target")->IsActive()) //Do not pick new target automatically when it is active.
        return false;

    if (WaitForDestinations())
        return true;

    if (hasDestinations && destinationList.empty()) //We only have an empty list. Continue with different actions to get more.
        return false;

    if (AI_VALUE(TravelTarget*, "travel target")->IsPreparing()) //Another action is fetching destinations. 
        return false;

    info = PlayerTravelInfo(bot);

    if (SetBestDestination(event))
        return true;
    
    if (RequestNewDestinations(event))
        return true;

    return false;
}

bool ChooseAsyncTravelTargetAction::isUseful()
{
    if (!ChooseTravelTargetAction::isUseful())
        return false;

    actionPurpose = TravelDestinationPurpose(stoi(getQualifier()));

    hasDestinations = HAS_AI_VALUE2("travel destinations", (uint32)actionPurpose);

    if (hasDestinations)
    {
        destinationList = AI_VALUE2(PartitionedTravelList, "travel destinations", (uint32)actionPurpose);

        if (destinationList.empty())
            return false;
    }
    else
        destinationList.clear();

    return true;
}

bool ChooseAsyncNamedTravelTargetAction::PassTrough() const
{
    std::string_view name = getQualifier();
    if (name == "city")
    {
        if (urand(1, 100) <= 90)
            return true;
    }
    else if (name == "pvp")
    {
        if (!urand(0, 4))
            return true;
    }

    return false;
}

bool ChooseAsyncNamedTravelTargetAction::RequestNewDestinations(Event& event)
{
    std::string_view name = getQualifier();

    if (destinationList.size())
        return false;

    if (futureDestinations.valid())
        return false;

    if (PassTrough())
        return false;

    //ai->TellPlayer(ai->GetMaster(), "Fetching new destinations for " + std::string(name));

    WorldPosition center = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);

    if (name == "city")
    {
        futureDestinations = std::async(std::launch::async, [partitions = travelPartitions, travelInfo = info, center]() {return sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)TravelDestinationPurpose::GenericRpg); });
    }
    else if (name == "pvp")
    {
        futureDestinations = std::async(std::launch::async, [travelInfo = info, center]()
            {
                PartitionedTravelList list;
                for (auto& destination : ChooseTravelTargetAction::FindDestination(travelInfo, "Tarren Mill"))
                {
                    std::vector<WorldPosition*> points = destination->NextPoint(center);

                    if (points.empty())
                        continue;

                    list[0].push_back(TravelPoint(destination, points.front(), points.front()->distance(center)));
                }

                return list;
            }
        );
    }

    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);

    return true;
}

bool ChooseAsyncQuestTravelTargetAction::RequestNewDestinations(Event& event)
{
    if (destinationList.size())
        return false;

    if (futureDestinations.valid())
        return false;

    //ai->TellPlayer(ai->GetMaster(), "Fetching new destinations for quests");

    QuestStatusMap& questMap = bot->getQuestStatusMap();

    std::vector<std::tuple<uint32, uint32, float>> destinationFetches = { {(uint32)TravelDestinationPurpose::QuestGiver, 0, 400 + bot->GetLevel() * 10} };

    bool onlyClassQuest = !urand(0, 10);

    uint32 questObjectiveFlag = (uint32)TravelDestinationPurpose::QuestObjective1 | (uint32)TravelDestinationPurpose::QuestObjective2 | (uint32)TravelDestinationPurpose::QuestObjective3 | (uint32)TravelDestinationPurpose::QuestObjective4;

    WorldPosition center = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);

    //Find destinations related to the active quests.
    for (auto& [questId, questStatus] : questMap)
    {
        uint32 flag = 0; 
        if (questStatus.m_rewarded)
            continue;

        Quest const* questTemplate = sObjectMgr.GetQuestTemplate(questId);

        if (!questTemplate)
            continue;

        if (bot->CanRewardQuest(questTemplate, false))
            flag = (uint32)TravelDestinationPurpose::QuestTaker;
        else
        {
            for (uint32 objective = 1; objective < 5; objective++)
            {
                TravelDestinationPurpose purposeFlag = (TravelDestinationPurpose)(1 << (objective + 1));

                std::vector<std::string> qualifier = { std::to_string(questId), std::to_string(objective) };

                if (AI_VALUE2(bool, "group or", "following party,near leader,need quest objective::" + Qualified::MultiQualify(qualifier, ","))) //Noone needs the quest objective.
                    flag = flag | (uint32)purposeFlag;
            }
        }
            
        if (!flag)
            continue;

        destinationFetches.push_back({ flag, questId,0 });

        if (onlyClassQuest && destinationFetches.size() > 1) //Only do class quests if we have any.
        {
            Quest const* firstQuest = sObjectMgr.GetQuestTemplate(std::get<1>(destinationFetches[1]));

            if (firstQuest->GetRequiredClasses() && !questTemplate->GetRequiredClasses())
                continue;

            if (!firstQuest->GetRequiredClasses() && questTemplate->GetRequiredClasses())
                destinationFetches = { destinationFetches.front()};
        }
    }

    MANGOS_ASSERT(travelPartitions.size() && travelPartitions.size() < 1000);

    futureDestinations = std::async(std::launch::async, [partitions = travelPartitions, travelInfo = info, center, destinationFetches]()
        {
            MANGOS_ASSERT(partitions.size() && partitions.size() < 1000);
            PartitionedTravelList list;
            for (auto [purpose, questId, range] : destinationFetches)
            {
                PartitionedTravelList subList = sTravelMgr.GetPartitions(center, partitions, travelInfo, purpose, questId, true, range);
                MANGOS_ASSERT(subList.size() < 100000);

                for(auto& [partition, points] : subList)
                    list[partition].insert(list[partition].end(), points.begin(), points.end());
            }

            if (list.empty())
                list = sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)TravelDestinationPurpose::QuestGiver);

            return list;
        }
    );

    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);

    return true;
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
