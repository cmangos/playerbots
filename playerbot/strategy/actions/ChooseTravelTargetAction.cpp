
#include "playerbot/playerbot.h"
#include "playerbot/LootObjectStack.h"
#include "ChooseTravelTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/TravelValues.h"
#include <iomanip>

using namespace ai;

bool ChooseTravelTargetAction::Execute(Event& event)
{
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    if(!travelTarget->IsPreparing())
        return false;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    FutureDestinations* futureDestinations = AI_VALUE(FutureDestinations*, "future travel destinations");
    std::string futureTravelPurpose = AI_VALUE2(std::string, "manual string", "future travel purpose");

    if (Qualified::isValidNumberString(futureTravelPurpose))
        futureTravelPurpose = TravelDestinationPurposeName.at(TravelDestinationPurpose(stoi(futureTravelPurpose)));

    if (!futureDestinations->valid())
    {
        travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);
        context->ClearValues("no active travel destinations");        
        return false;
    }

    if (futureDestinations->wait_for(std::chrono::seconds(0)) == std::future_status::timeout)
        return false;

    PartitionedTravelList destinationList = futureDestinations->get();

    travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);

    ai->TellDebug(ai->GetMaster(), "Got " + std::to_string(destinationList.size()) + " new destination ranges for " + futureTravelPurpose, "debug travel");

    TravelTarget newTarget = TravelTarget(ai);

    if (futureTravelPurpose == "pvp" || futureTravelPurpose == "city" || futureTravelPurpose == "petition" || futureTravelPurpose == "tabard"
        || futureTravelPurpose.find("trainer") == 0 || futureTravelPurpose == "mount")
        newTarget.SetForced(true);

    if (!SetBestTarget(bot, &newTarget, destinationList))
    {
        SET_AI_VALUE2(bool, "no active travel destinations", futureTravelPurpose, true);
        ai->TellDebug(ai->GetMaster(), "No target set", "debug travel");
        return false;
    }

    if (!newTarget.IsActive() && !newTarget.IsForced())
       return false;    

    setNewTarget(requester, &newTarget, travelTarget);
    
    return true;
}

bool ChooseTravelTargetAction::isUseful()
{
    if (!ai->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    if (!AI_VALUE(bool, "can move around"))
        return false;

    if (AI_VALUE(bool, "travel target active"))
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
            return false;

    return true;
}

void ChooseTravelTargetAction::setNewTarget(Player* requester, TravelTarget* newTarget, TravelTarget* oldTarget)
{
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
    oldTarget->SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);

    //If we are idling but have a master. Idle only 10 seconds.
    if (ai->GetMaster() && oldTarget->IsActive() && typeid(*oldTarget->GetDestination()) == typeid(NullTravelDestination))
        oldTarget->SetExpireIn(10 * IN_MILLISECONDS);
    else if (oldTarget->IsForced()) //Make sure travel goes into cooldown after getting to the destination.
        oldTarget->SetExpireIn(HOUR * IN_MILLISECONDS);

    if (typeid(oldTarget->GetDestination()) == typeid(QuestObjectiveTravelDestination) || typeid(oldTarget->GetDestination()) == typeid(QuestRelationTravelDestination))
    {
        QuestTravelDestination* dest = dynamic_cast<QuestTravelDestination*>(oldTarget->GetDestination());
        std::string condition = "group or::{following party,near leader,quest stage active::{" + std::to_string(dest->GetQuestId()) + "," + std::to_string((uint8)dest->GetPurpose()) + "}}";
        oldTarget->AddCondition(condition);
    }

    //Clear rpg and attack/grind target. We want to travel, not hang around some more.
    RESET_AI_VALUE(GuidPosition,"rpg target");
    RESET_AI_VALUE(ObjectGuid,"attack target");
    RESET_AI_VALUE(bool, "travel target active");
    context->ClearValues("no active travel destinations");
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

        out << "new," << "\"" << destination->GetTitle() << "\",\"" << message << "\"";

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

            out << "previous," << "\"" << destination->GetTitle() << "\",\""<< message << "\"";

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
        }

        SET_AI_VALUE2(WorldPosition, "custom position", "last choose travel", botPos);
    }
}

//Sets the target to the best destination.
bool ChooseTravelTargetAction::SetBestTarget(Player* requester, TravelTarget* target, PartitionedTravelList& partitionedList, bool onlyActive)
{
    bool distanceCheck = true;
    std::unordered_map<TravelDestination*, bool> isActive;

    bool hasTarget = false;

    for (auto& [partition, travelPointList] : partitionedList)
    {
        ai->TellDebug(requester, "Found " + std::to_string(travelPointList.size()) + " points at range " + std::to_string(partition), "debug travel");

        for (auto& [destination, position, distance] : travelPointList)
        {
            if (!target->IsForced() && isActive.find(destination) != isActive.end() && !isActive[destination])
                continue;

            if (distanceCheck) //Check if we have moved significantly after getting the destinations.
            {
                WorldPosition center(requester ? requester : bot);
                if (position->distance(center) > distance * 2 && position->distance(center) > 100)
                {
                    ai->TellDebug(requester, "We had some destinations but we moved too far since. Trying to get a new list.", "debug travel");
                    return false;
                }

                distanceCheck = false;
            }

            if(target->IsForced() || (isActive[destination] = destination->IsActive(bot, PlayerTravelInfo(bot))))
            {
                if (partition != std::prev(partitionedList.end())->first && !urand(0, 10)) //10% chance to skip to a longer partition.
                {
                    ai->TellDebug(requester, "Skipping range " + std::to_string(partition), "debug travel");
                    break;
                }

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

/*
char* strstri(const char* haystack, const char* needle);

bool ChooseTravelTargetAction::SetNpcFlagTarget(Player* requester, TravelTarget* target, std::vector<NPCFlags> flags, std::string name, std::vector<uint32> items, bool force)
{
    auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "SetNpcFlagTarget", &context->performanceStack);
    WorldPosition pos = WorldPosition(bot);
    WorldPosition* botPos = &pos;

    PartitionedTravelList TravelDestinations;
    uint32 found = 0;

    //Loop over all npcs.
    for (auto& [partition, points] : sTravelMgr.GetPartitions(pos, travelPartitions, PlayerTravelInfo(bot), (uint32)TravelDestinationPurpose::GenericRpg, 0, false))
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
*/

std::vector<std::string> split(const std::string& s, char delim);
char* strstri(const char* haystack, const char* needle);

//Find a destination based on (part of) it's name. Includes zones, ncps and mobs. Picks the closest one that matches.
DestinationList ChooseTravelTargetAction::FindDestination(PlayerTravelInfo info, std::string name, bool zones, bool npcs, bool quests, bool mobs, bool bosses)
{
    DestinationList dests;

    //Quests
    if (quests)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::QuestGiver, { 0 }, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Zones
    if (zones)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::Explore, { 0 }, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Npcs
    if (npcs)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::GenericRpg, { 0 }, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Mobs
    if (mobs)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::Grind, { 0 }, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    //Bosses
    if (bosses)
    {
        for (auto& d : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::Boss, { 0 }, false))
        {
            if (strstri(d->GetTitle().c_str(), name.c_str()))
                dests.push_back(d);
        }
    }

    if (dests.empty())
        return {};

    return dests;
};

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

    ai->TellDebug(requester, std::to_string(groupTargets[0].size()) + " group targets found.", "debug travel");

    if (groupTargets[0].empty())
        return false;

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

bool ChooseGroupTravelTargetAction::isUseful()
{
    if (!bot->GetGroup())
        return false;

    if (!ChooseTravelTargetAction::isUseful())
        return false;

    if (AI_VALUE(TravelTarget*, "travel target")->IsPreparing())
        return false;

    return true;
}

bool RefreshTravelTargetAction::Execute(Event& event)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    TravelDestination* oldDestination = target->GetDestination();

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    if (target->IsMaxRetry(false))
    {
        ai->TellDebug(requester, "Old destination was tried to many times.", "debug travel");
        return false;
    }

    if (!oldDestination) //Does this target have a destination?
        return false;

    if (!oldDestination->IsActive(bot, PlayerTravelInfo(bot))) //Is the destination still valid?
    {
        ai->TellDebug(requester, "Old destination was no longer valid.", "debug travel");
        return false;
    }

    std::vector<WorldPosition*> newPositions = oldDestination->NextPoint(*target->GetPosition());

    if (newPositions.empty())
    {
        ai->TellDebug(requester, "No new locations found for old destination.", "debug travel");
        return false;
    }

    target->SetTarget(oldDestination, newPositions.front());

    target->SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);
    target->SetRetry(false, target->GetRetryCount(false) + 1);

    RESET_AI_VALUE(bool, "travel target active");

    if (!target->IsActive())
    {
        ai->TellDebug(requester, "Target was not active after refresh.", "debug travel");
        return true;
    }

    ai->TellDebug(requester, "Refreshed travel target", "debug travel");

    return false;
}

bool RefreshTravelTargetAction::isUseful()
{
    if (!ChooseTravelTargetAction::isUseful())
        return false;

    if (AI_VALUE(TravelTarget*, "travel target")->IsPreparing())
        return false;

    if (!AI_VALUE(TravelTarget*, "travel target")->GetDestination()->IsActive(bot, PlayerTravelInfo(bot)))
        return false;

    if (!WorldPosition(bot).isOverworld())
        return false;

    if (urand(1, 100) <= 10)
        return false;

    return true;
}

bool ResetTargetAction::Execute(Event& event)
{
    TravelTarget* oldTarget = AI_VALUE(TravelTarget*, "travel target");

    context->ClearValues("no active travel destinations");

    TravelTarget newTarget = TravelTarget(ai);
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    setNewTarget(requester, &newTarget, oldTarget);

    ai->TellDebug(requester, "Cleared travel target fetches", "debug travel");

    return true;
}

bool ResetTargetAction::isUseful()
{
    if (!ChooseTravelTargetAction::isUseful())
        return false;

    if (AI_VALUE(TravelTarget*, "travel target")->IsPreparing())
        return false;

    return true;
}

bool RequestTravelTargetAction::Execute(Event& event)
{
    TravelDestinationPurpose actionPurpose = TravelDestinationPurpose(stoi(getQualifier()));

    WorldPosition center = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);

    ai->TellDebug(ai->GetMaster(), "Getting new destination ranges for " + TravelDestinationPurposeName.at(actionPurpose), "debug travel");

    *AI_VALUE(FutureDestinations*, "future travel destinations") = std::async(std::launch::async, [partitions = travelPartitions, travelInfo = PlayerTravelInfo(bot), center, purpose = actionPurpose]() { return sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)purpose); });

    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);
    AI_VALUE(TravelTarget*, "travel target")->SetConditions({ event.getSource() });
    SET_AI_VALUE2(std::string, "manual string", "future travel purpose", getQualifier());

    return true;
}

bool RequestTravelTargetAction::isUseful() {
    if (!ai->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    if (AI_VALUE(TravelTarget*, "travel target")->IsPreparing())
        return false;

    if (AI_VALUE(bool, "travel target active"))
        return false;

    if (AI_VALUE2(bool, "no active travel destinations", (getQualifier().empty() ? "quest" : getQualifier())))
        return false;

    if (!AI_VALUE(bool, "can move around"))
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
            return false;

    if (!isAllowed())
    {
        std::string futureTravelPurpose = AI_VALUE2(std::string, "manual string", "future travel purpose");

        if (Qualified::isValidNumberString(futureTravelPurpose))
            futureTravelPurpose = TravelDestinationPurposeName.at(TravelDestinationPurpose(stoi(futureTravelPurpose)));

        ai->TellDebug(ai->GetMaster(), "Skipped " + futureTravelPurpose + " because of skip chance", "debug travel");
        return false;
    }

    return true;
}

bool RequestTravelTargetAction::isAllowed() const
{
    TravelDestinationPurpose actionPurpose = TravelDestinationPurpose(stoi(getQualifier()));

    switch (actionPurpose)
    {
    case TravelDestinationPurpose::Repair:
    case TravelDestinationPurpose::Vendor:
    case TravelDestinationPurpose::AH:
        return urand(1, 100) < 90;
    case TravelDestinationPurpose::Mail:
        if (AI_VALUE(bool, "should get money"))
            return urand(1, 100) < 70;
        else
            return true;
    case TravelDestinationPurpose::GatherSkinning:
    case TravelDestinationPurpose::GatherMining:
    case TravelDestinationPurpose::GatherHerbalism:
    case TravelDestinationPurpose::GatherFishing:
        if (bot->GetGroup())
            return urand(1, 100) < 50;
        else
            return urand(1, 100) < 90;
    case TravelDestinationPurpose::Boss:
        return urand(1, 100) < 50;
    case TravelDestinationPurpose::Explore:
        return urand(1, 100) < 10;
    case TravelDestinationPurpose::GenericRpg:
        return urand(1, 100) < 50;
    case TravelDestinationPurpose::Grind:
        return true;
    default:
        return true;
    }
}

bool RequestNamedTravelTargetAction::Execute(Event& event)
{
    std::string travelName = getQualifier();

    WorldPosition center = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);

    ai->TellDebug(ai->GetMaster(), "Getting new destination ranges for travel " + getQualifier(), "debug travel");

    if (travelName == "pvp")
    {
        *AI_VALUE(FutureDestinations*, "future travel destinations") = std::async(std::launch::async, [travelInfo = PlayerTravelInfo(bot), center]()
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
    else if (travelName.find("trainer") == 0)
    {
        TrainerType type;

        if (travelName == "trainer class")
            type = TRAINER_TYPE_CLASS;
        if (travelName == "trainer mount")
            type = TRAINER_TYPE_MOUNTS;
        if (travelName == "trainer trade")
            type = TRAINER_TYPE_TRADESKILLS;
        if (travelName == "trainer pet")
            type = TRAINER_TYPE_PETS;

        std::vector<int32> trainerEntries = AI_VALUE2(std::vector <int32>, "available trainers", type);

        if (trainerEntries.empty())
        {
            ai->TellDebug(ai->GetMaster(), "No trainer entries found for " + getQualifier(), "debug travel");
            return false;
        }

        *AI_VALUE(FutureDestinations*, "future travel destinations") = std::async(std::launch::async, [entries = trainerEntries, partitions = travelPartitions, travelInfo = PlayerTravelInfo(bot), center]()
            {
                return sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)TravelDestinationPurpose::Trainer, entries, false);
            });
    }
    else if (travelName == "mount")
    {
        std::vector<int32> mountVendorEntries = AI_VALUE(std::vector <int32>, "available mount vendors");

        if (mountVendorEntries.empty())
        {
            ai->TellDebug(ai->GetMaster(), "No vendor entries found for " + getQualifier(), "debug travel");
            return false;
        }

        *AI_VALUE(FutureDestinations*, "future travel destinations") = std::async(std::launch::async, [entries = mountVendorEntries, partitions = travelPartitions, travelInfo = PlayerTravelInfo(bot), center]()
            {
                return sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)TravelDestinationPurpose::Vendor, entries, false);
            });
    }
    else
    {
        uint32 useFlags;
        
        if(travelName == "city")
            useFlags = NPCFlags::UNIT_NPC_FLAG_BANKER | NPCFlags::UNIT_NPC_FLAG_BATTLEMASTER | NPCFlags::UNIT_NPC_FLAG_AUCTIONEER;
        else if (travelName == "tabard")
            useFlags = NPCFlags::UNIT_NPC_FLAG_TABARDDESIGNER;
        else if (travelName == "petition")
            useFlags = NPCFlags::UNIT_NPC_FLAG_PETITIONER;


        *AI_VALUE(FutureDestinations*, "future travel destinations") = std::async(std::launch::async, [cityFlags = useFlags, partitions = travelPartitions, travelInfo = PlayerTravelInfo(bot), center]()
            {
                PartitionedTravelList list = sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)TravelDestinationPurpose::GenericRpg);

                for (auto& [partition, travelPoints] : list)
                {
                    travelPoints.erase(std::remove_if(travelPoints.begin(), travelPoints.end(), [cityFlags](TravelPoint point)
                        {
                            EntryTravelDestination* dest = (EntryTravelDestination*)std::get<TravelDestination*>(point);
                            if (!dest->GetCreatureInfo())
                                return true;

                            if (dest->GetCreatureInfo()->NpcFlags & cityFlags)
                                return false;

                            return true;
                        }), travelPoints.end());
                }
                return list;
            });
    }

    AI_VALUE(TravelTarget*, "travel target")->SetConditions({ event.getSource() });
    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);
    SET_AI_VALUE2(std::string, "manual string", "future travel purpose", getQualifier());

    return true;
}

bool RequestNamedTravelTargetAction::isAllowed() const
{
    std::string name = getQualifier();
    if (name == "city")
    {
        if (urand(1, 100) > 10)
            return false;
        return true;
    }
    else if (name == "pvp")
    {
        if (urand(0, 4))
            return false;
        return true;
    }
    else if (name == "mount")
    {
        if (urand(1, 100) > 100)
            return false;
        return true;
    }
    else if (name.find("trainer") == 0)
    {
        if (urand(1, 100) > 100)
            return false;
        return true;
    }
    else if (name == "tabard")
        return true;
    else if (name == "petition")
        return true;

    return false;
}

bool RequestQuestTravelTargetAction::Execute(Event& event)
{
    WorldPosition center = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);

    ai->TellDebug(ai->GetMaster(), "Getting new destination ranges for travel quest", "debug travel");

    QuestStatusMap& questMap = bot->getQuestStatusMap();

    std::vector<std::tuple<uint32, int32, float>> destinationFetches = { {(uint32)TravelDestinationPurpose::QuestGiver, 0, 400 + bot->GetLevel() * 10} };

    bool onlyClassQuest = !urand(0, 10);

    uint32 questObjectiveFlag = (uint32)TravelDestinationPurpose::QuestObjective1 | (uint32)TravelDestinationPurpose::QuestObjective2 | (uint32)TravelDestinationPurpose::QuestObjective3 | (uint32)TravelDestinationPurpose::QuestObjective4;

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
            for (uint32 objective = 0; objective < 4; objective++)
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
                destinationFetches = { destinationFetches.front() };
        }
    }

    *AI_VALUE(FutureDestinations*, "future travel destinations") = std::async(std::launch::async, [partitions = travelPartitions, travelInfo = PlayerTravelInfo(bot), center, destinationFetches]()
        {
            PartitionedTravelList list;
            for (auto [purpose, questId, range] : destinationFetches)
            {
                PartitionedTravelList subList = sTravelMgr.GetPartitions(center, partitions, travelInfo, purpose, { questId }, true, range);

                for (auto& [partition, points] : subList)
                    list[partition].insert(list[partition].end(), points.begin(), points.end());
            }

            if (list.empty())
                list = sTravelMgr.GetPartitions(center, partitions, travelInfo, (uint32)TravelDestinationPurpose::QuestGiver);

            return list;
        }
    );

    AI_VALUE(TravelTarget*, "travel target")->SetConditions({ event.getSource() });
    AI_VALUE(TravelTarget*, "travel target")->SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);
    SET_AI_VALUE2(std::string, "manual string", "future travel purpose", "quest");

    return true;
}

bool RequestQuestTravelTargetAction::isAllowed() const
{
    if (AI_VALUE(bool, "should get money"))
        return urand(1, 100) < 90;
    else
        return urand(1, 100) < 95;

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
