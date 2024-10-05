
#include "playerbot/playerbot.h"
#include "MoveToTravelTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/LootObjectStack.h"
#include "MotionGenerators/PathFinder.h"
#include "playerbot/TravelMgr.h"
#include <iomanip>

using namespace ai;

bool MoveToTravelTargetAction::Execute(Event& event)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    WorldPosition botLocation(bot);
    WorldLocation location = *target->getPosition();
    
    Group* group = bot->GetGroup();
    if (group && !urand(0, 1) && bot == ai->GetGroupMaster())
    {        
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            if (member == bot)
                continue;

            if (!member->IsAlive())
                continue;

            if (!member->IsMoving())
                continue;

            if (member->GetPlayerbotAI() && !member->GetPlayerbotAI()->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
                continue;

            WorldPosition memberPos(member);
            WorldPosition targetPos = *target->getPosition();

            float memberDistance = botLocation.distance(memberPos);

            if (memberDistance < 50.0f)
                continue;
            if (memberDistance > sPlayerbotAIConfig.reactDistance * 20)
                continue;

           // float memberAngle = botLocation.getAngleBetween(targetPos, memberPos);

           // if (botLocation.getMapId() == targetPos.getMapId() && botLocation.getMapId() == memberPos.getMapId() && memberAngle < M_PI_F / 2) //We are heading that direction anyway.
           //     continue;

            if (!urand(0, 5))
            {
                std::ostringstream out;
                if (ai->GetMaster() && !bot->GetGroup()->IsMember(ai->GetMaster()->GetObjectGuid()))
                    out << "Waiting a bit for ";
                else
                    out << "Please hurry up ";

                out << member->GetName();

                ai->TellPlayerNoFacing(GetMaster(), out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            }

            target->setExpireIn(target->getTimeLeft() + sPlayerbotAIConfig.maxWaitForMove);

            SetDuration(sPlayerbotAIConfig.maxWaitForMove);

            return true;
        }
    }

    float maxDistance = target->getDestination()->getRadiusMin();

    //Evenly distribute around the target.
    float angle = 2 * M_PI * urand(0, 100) / 100.0;

    float x = location.coord_x;
    float y = location.coord_y;
    float z = location.coord_z;
    float mapId = location.mapid;

    //Move between 0.5 and 1.0 times the maxDistance.
    float mod = urand(50, 100)/100.0;   

    x += cos(angle) * maxDistance * mod;
    y += sin(angle) * maxDistance * mod;

    bool canMove = false;

    if (ai->HasStrategy("debug travel", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;

        out << "Moving to ";

        out << target->getDestination()->getTitle();

        if (!(*target->getPosition() == WorldPosition()))
        {
            out << " at " << uint32(target->getPosition()->distance(bot)) << "y";
        }

        if (target->getStatus() != TravelStatus::TRAVEL_STATUS_EXPIRED)
            out << " for " << (target->getTimeLeft() / 1000) << "s";

        if (target->getRetryCount(true))
            out << " (move retry: " << target->getRetryCount(true) << ")";
        else if (target->getRetryCount(false))
            out << " (retry: " << target->getRetryCount(false) << ")";

        ai->TellPlayerNoFacing(GetMaster(), out);
    }

    canMove = MoveTo(mapId, x, y, z, false, false);

    if (!canMove)
    {
        target->incRetry(true);

        if (target->isMaxRetry(true))
        {
            target->setStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);

            if (sPlayerbotAIConfig.hasLog("travel_map.csv"))
            {
                WorldPosition botPos(bot);
                WorldPosition destPos = *target->getPosition();
                TravelDestination* destination = target->getDestination();

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

                if (destination->getName() == "NullTravelDestination")
                    out << "0,";
                else
                    out << round(target->getDestination()->distanceTo(botPos)) << ",";

                out << "2," << "\"" << destination->getTitle() << "\",\"" << "timeout" << "\"";

                if (destination->getName() == "NullTravelDestination")
                    out << ",none";
                else if (destination->getName() == "QuestTravelDestination")
                    out << ",quest";
                else if (destination->getName() == "QuestRelationTravelDestination")
                    out << ",questgiver";
                else if (destination->getName() == "QuestObjectiveTravelDestination")
                    out << ",objective";
                else  if (destination->getName() == "RpgTravelDestination")
                {
                    RpgTravelDestination* RpgDestination = (RpgTravelDestination*)destination;
                    if (RpgDestination->getEntry() > 0)
                    {
                        CreatureInfo const* cInfo = RpgDestination->getCreatureInfo();

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
                        GameObjectInfo const* gInfo = RpgDestination->getGoInfo();

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
                else if (destination->getName() == "ExploreTravelDestination")
                    out << ",explore";
                else if (destination->getName() == "GrindTravelDestination")
                    out << ",grind";
                else if (destination->getName() == "BossTravelDestination")
                    out << ",boss";
                else
                    out << ",unknown";

                sPlayerbotAIConfig.log("travel_map.csv", out.str().c_str());
            }
        }
    }
    else
        target->decRetry(true);

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        WorldPosition* pos = target->getPosition();
        GuidPosition* guidP = dynamic_cast<GuidPosition*>(pos);

        std::string name = (guidP && guidP->GetWorldObject(bot->GetInstanceId())) ? chat->formatWorldobject(guidP->GetWorldObject(bot->GetInstanceId())) : "travel target";

        if (mapId == bot->GetMapId())
        {
            ai->Poi(x, y, name);
        }
        else
        {
            LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");
            if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(location) < 20.0f)
            {
                for (auto& p : lastMove.lastPath.getPointPath())
                {
                    if (p.getMapId() == bot->GetMapId())
                        ai->Poi(p.getX(), p.getY(), name);
                }
            }
        }
    }
     
    return canMove;
}

bool MoveToTravelTargetAction::isUseful()
{
    if (!ai->AllowActivity(TRAVEL_ACTIVITY))
        return false;

    if (!AI_VALUE(TravelTarget*,"travel target")->isTraveling())
        return false;

    if (bot->IsTaxiFlying())
        return false;

    if (MEM_AI_VALUE(WorldPosition, "current position")->LastChangeDelay() < 10)
#ifndef MANGOSBOT_ZERO
        if (bot->IsMovingIgnoreFlying())
            return false;
#else
        if (bot->IsMoving())
            return false;
#endif

    if (!AI_VALUE(bool, "can move around"))
        return false;

    WorldPosition travelPos(*AI_VALUE(TravelTarget*, "travel target")->getPosition());

    if (travelPos.isDungeon() && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid()) && sTravelMgr.mapTransDistance(bot, travelPos, true) < sPlayerbotAIConfig.sightDistance && !AI_VALUE2(bool, "group and", "near leader"))
        return false;
     
    if (AI_VALUE(bool, "has available loot"))
    {
        LootObject lootObject = AI_VALUE(LootObjectStack*, "available loot")->GetLoot(sPlayerbotAIConfig.lootDistance);
        if (lootObject.IsLootPossible(bot))
            return false;
    }

    if (!AI_VALUE2(bool, "can free move to", AI_VALUE(TravelTarget*,"travel target")->GetPosStr()))
        return false;

    return true;
}

