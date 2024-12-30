
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
    WorldLocation location = *target->GetPosition();
    
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
            WorldPosition targetPos = *target->GetPosition();

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

            target->SetExpireIn(target->GetTimeLeft() + sPlayerbotAIConfig.maxWaitForMove);

            SetDuration(sPlayerbotAIConfig.maxWaitForMove);

            return true;
        }
    }

    float maxDistance = target->GetDestination()->GetRadiusMin();

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

        out << target->GetDestination()->GetTitle();

        if (!(*target->GetPosition() == WorldPosition()))
        {
            out << " at " << uint32(target->GetPosition()->distance(bot)) << "y";
        }

        if (target->GetStatus() != TravelStatus::TRAVEL_STATUS_EXPIRED)
            out << " for " << (target->GetTimeLeft() / 1000) << "s";

        if (target->GetRetryCount(true))
            out << " (move retry: " << target->GetRetryCount(true) << ")";
        else if (target->GetRetryCount(false))
            out << " (retry: " << target->GetRetryCount(false) << ")";

        ai->TellPlayerNoFacing(GetMaster(), out);
    }

    canMove = MoveTo(mapId, x, y, z, false, false);

    if (!canMove)
    {
        target->IncRetry(true);

        if (target->IsMaxRetry(true))
        {
            target->SetStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);

            if (sPlayerbotAIConfig.hasLog("travel_map.csv"))
            {
                WorldPosition botPos(bot);
                WorldPosition destPos = *target->GetPosition();
                TravelDestination* destination = target->GetDestination();

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
                    out << round(target->GetDestination()->DistanceTo(botPos)) << ",";

                out << "2," << "\"" << destination->GetTitle() << "\",\"" << "timeout" << "\"";

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
            }
        }
    }
    else
        target->DecRetry(true);

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        WorldPosition* pos = target->GetPosition();
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

    if (!AI_VALUE(TravelTarget*,"travel target")->IsTraveling())
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

    WorldPosition travelPos(*AI_VALUE(TravelTarget*, "travel target")->GetPosition());

    if (travelPos.isDungeon() && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid()) && sTravelMgr.MapTransDistance(bot, travelPos, true) < sPlayerbotAIConfig.sightDistance && !AI_VALUE2(bool, "group and", "near leader"))
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

