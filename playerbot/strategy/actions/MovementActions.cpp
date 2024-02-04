#include "botpch.h"
#include "../../playerbot.h"
#include "../values/LastMovementValue.h"
#include "MovementActions.h"
#include "MotionMaster.h"
#include "MovementGenerator.h"
#include "../../FleeManager.h"
#include "../../LootObjectStack.h"
#include "../../PlayerbotAIConfig.h"
#include "../../ServerFacade.h"
#include "../values/PositionValue.h"
#include "../values/Stances.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "../../TravelMgr.h"
#include "Entities/Transports.h"
#ifdef MANGOSBOT_TWO
#include "Entities/Vehicle.h"
#endif
#include "../generic/CombatStrategy.h"

using namespace ai;

void MovementAction::CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig.reactDelay;

    //if(!important)
    //    delay *= 0.25;

    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSPAWN_TIMED_DESPAWN, delay);
    ai->AddAura(wpCreature, 246);

    if (!important)
        wpCreature->SetObjectScale(0.5f);
}

bool MovementAction::isPossible()
{
    return ai->CanMove();
}

bool MovementAction::isUseful()
{
    return !ai->HasStrategy("stay", ai->GetState());
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance)
{
    float angle = GetFollowAngle();
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z);
}

bool MovementAction::MoveNear(WorldObject* target, float distance)
{
    if (!target)
        return false;

#ifdef MANGOS
    distance += target->GetObjectBoundingRadius();
#endif

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    float followAngle = GetFollowAngle();
    for (float angle = followAngle; angle <= followAngle + 2 * M_PI; angle += M_PI_F / 4.0f)
    {
#ifdef CMANGOS
        float dist = distance + target->GetObjectBoundingRadius();
        target->GetNearPoint(bot, x, y, z, bot->GetObjectBoundingRadius(), min(dist, ai->GetRange("follow")), angle);
#endif
#ifdef MANGOS
        float x = target->GetPositionX() + cos(angle) * distance,
             y = target->GetPositionY()+ sin(angle) * distance,
             z = target->GetPositionZ();
#endif
        if (!bot->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true))
            continue;
        bool moved = MoveTo(target->GetMapId(), x, y, z);
        if (moved)
            return true;
    }

    //ai->TellError("All paths not in LOS");
    return false;
}

bool MovementAction::FlyDirect(WorldPosition &startPosition, WorldPosition &endPosition, WorldPosition& movePosition, TravelPath movePath, bool idle)
{
    //Fly directly.
#ifdef MANGOSBOT_ZERO
    return false;
#else
    if (!bot->IsFreeFlying())
        return false;

    if (!startPosition.isOutside())
        return false;

    float totalDistance = startPosition.distance(endPosition);  //Total distance to where we want to go
    float minDist = sPlayerbotAIConfig.targetPosRecalcDistance; //Minium distance a bot should move.
    float maxDist = sPlayerbotAIConfig.reactDistance;           //Maxium distance a bot can move in one single action.

    if (totalDistance < maxDist && !bot->IsFlying())
        return false;

    movePosition = endPosition;

    if (movePosition.getMapId() != startPosition.getMapId() || !movePosition.isOutside() || !movePosition.canFly()) //We can not fly to the end directly.
    {
        vector<WorldPosition> path;
        if (movePath.empty()) //Make a path starting at the end backwards to see if we can walk to some better place.
        {
            path = endPosition.getPathTo(startPosition, bot);
            std::reverse(path.begin(), path.end());
        }
        else
            path = movePath.getPointPath();

        if (path.empty())
            return false;

        auto pathEnd = path.end();
        for (auto& p = pathEnd; p-- != path.begin(); ) //Find the furtest point where we can fly to directly.
            if (p->getMapId() == startPosition.getMapId() && p->isOutside() && p->canFly())
            {
                movePosition = *p;
                totalDistance = startPosition.distance(movePosition);
                break;
            }
    }

    if (movePosition.getMapId() != startPosition.getMapId() || !movePosition.isOutside() || !movePosition.canFly())
        return false;

    if (movePosition.distance(startPosition) < minDist)
    {
        movePath.clear();
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (movePosition.currentHeight() < minDist)
            return false;
        else
            movePosition.setZ(movePosition.getHeight());
    }

    uint32 flyHeight = 0;
    float originalZ = endPosition.getZ();
    bool detailedMove = ai->AllowActivity(DETAILED_MOVE_ACTIVITY);

    //Crop the distance we can travel to maxDist;
    if (totalDistance > maxDist)
    {
        flyHeight = std::min(100.0f, totalDistance / 10.0f);

        movePosition = movePosition.limit(startPosition, maxDist);

        if (!bot->IsFlying())
        {
            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
        }
    }
    else
    {
        bool needLand = false;

        if (const TerrainInfo* terrain = bot->GetTerrain())
        {
            float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
            if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ < 5.0f) && (fabs(originalZ - ground) < 5.0f))
                needLand = true;
        }
        if (needLand)
        {
            WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);
        }
    }

    for (uint32 modZ = 0; modZ < maxDist / 5.0f; modZ++)
    {
        if (movePosition.currentHeight() > flyHeight && startPosition.IsInLineOfSight(movePosition))
            break;

        movePosition.setZ(movePosition.getZ() + 5.0f);

        if (movePosition.distance(startPosition) > maxDist)
            movePosition = movePosition.limit(startPosition, maxDist);
    }

    LastMovement& lastMove = AI_VALUE(LastMovement&,"last movement");
    if (sPlayerbotAIConfig.hasLog("bot_movement.csv") && lastMove.lastMoveShort != movePosition)
    {
        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        startPosition.printWKT({ startPosition, movePosition }, out, 1);
        out << to_string(bot->getRace()) << ",";
        out << to_string(bot->getClass()) << ",";
        float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
        out << subLevel << ",";
        out << "1";

        sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
    }

    if (totalDistance > maxDist && !detailedMove && !ai->HasPlayerNearby(movePosition)) //Why walk if you can fly?
    {
        time_t now = time(0);

        AI_VALUE(LastMovement&, "last movement").nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));

        return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    }

    MotionMaster& mm = *bot->GetMotionMaster();

    //Clean movement if not already moving the same way.
    if (mm.GetCurrent()->GetMovementGeneratorType() != POINT_MOTION_TYPE)
    {
        ai->StopMoving();
        mm.Clear();
    }
    else
    {
        float x, y, z;
        mm.GetDestination(x, y, z);

        if (movePosition.distance(WorldPosition(movePosition.getMapId(), x, y, z, 0)) > minDist)
        {
            ai->StopMoving();
            mm.Clear();
        }
    }
    mm.Clear(false, true);
    mm.MovePoint(movePosition.getMapId(), Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f), bot->IsFlying() ? FORCED_MOVEMENT_FLIGHT : FORCED_MOVEMENT_RUN, bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.f, bot->IsFlying());

    AI_VALUE(LastMovement&, "last movement").setShort(startPosition, movePosition);

    if (!idle)
        ClearIdleState();

    return true;
#endif
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react, bool noPath, bool ignoreEnemyTargets)
{
    WorldPosition endPosition(mapId, x, y, z, 0);
    if(!endPosition.isValid())
        return false;

    UpdateMovementState();

    if (!IsMovingAllowed())
        return false;

    bool isVehicle = false;
    Unit* mover = bot;
#ifdef MANGOSBOT_TWO
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (transportInfo && transportInfo->IsOnVehicle())
    {
        Unit* vehicle = (Unit*)transportInfo->GetTransport();
        if (vehicle && vehicle->GetVehicleInfo())
        {
            VehicleSeatEntry const* seat = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat());
            if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_CONTROL))
                return false;
        }
        
        isVehicle = true;
        mover = vehicle;
    }
#endif

    bool detailedMove = ai->AllowActivity(DETAILED_MOVE_ACTIVITY,true);

    if (!detailedMove)
    {
        time_t now = time(0);
        if (AI_VALUE(LastMovement&, "last movement").nextTeleport > now) //We can not teleport yet. Wait.
        {
            SetDuration((AI_VALUE(LastMovement&, "last movement").nextTeleport - now) * 1000);
            return true;
        }
    }

    float minDist = sPlayerbotAIConfig.targetPosRecalcDistance; //Minimum distance a bot should move.
    float maxDist = sPlayerbotAIConfig.sightDistance;           //Maximum distance a bot can move in one single action.
    float originalZ = z;                                        // save original destination height to check if bot needs to fly up

    bool generatePath = !bot->IsFlying() && !bot->HasMovementFlag(MOVEFLAG_SWIMMING) && !bot->IsInWater() && !sServerFacade.IsUnderwater(bot);
    if (noPath)
        generatePath = false;

    if(!isVehicle && !IsMovingAllowed())
    {
        if(sServerFacade.UnitIsDead(bot) || sServerFacade.isMoving(bot))
        {
            return false;
        }
    }

    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    WorldPosition startPosition = WorldPosition(bot);             //Current location of the bot
    WorldPosition movePosition;

    float totalDistance = startPosition.distance(endPosition);    //Total distance to where we want to go
    float maxDistChange = totalDistance * 0.1;                    //Maximum change between previous destination before needing a recalculation
    TravelPath movePath;

    if (totalDistance < minDist)
    {
        if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange)
            AI_VALUE(LastMovement&, "last movement").clear();
        if (mover == bot)
            ai->StopMoving();
        else
            mover->InterruptMoving(true);
        return false;
    }

    bool isWalking = false;

    if (FlyDirect(startPosition, endPosition, movePosition, lastMove.lastPath, idle)) //Try flying in a straight line to target.
        return true;

    if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange && startPosition.distance(lastMove.lastMoveShort) < maxDist && !bot->GetTransport()) //The last short movement was to the same place we want to move now.
        movePosition = endPosition;
    else if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPosition) < maxDistChange) //The last long movement was to the same place we want to move now.
    {
        movePath = lastMove.lastPath;
    }
    else
    {
        movePosition = endPosition;

        if (startPosition.getMapId() != endPosition.getMapId() || totalDistance > maxDist)
        {
            if (!sTravelNodeMap.getNodes().empty() && !bot->InBattleGround())
            {
                movePath = sTravelNodeMap.getFullPath(startPosition, endPosition, bot);

                if (movePath.empty())
                {
                    //We have no path. Beyond 450yd the standard pathfinder will probably move the wrong way.
                    if (sServerFacade.IsDistanceGreaterThan(totalDistance, maxDist * 3))
                    {
                        movePath.clear();
                        movePath.addPoint(endPosition);
                        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

                        if (mover == bot)
                            ai->StopMoving();
                        else
                            mover->InterruptMoving(true);
                        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                            ai->TellPlayerNoFacing(GetMaster(), "I have no path");
                        return false;
                    }

                    movePosition = endPosition;
                }
                else if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    vector<WorldPosition> beginPath = endPosition.getPathFromPath({ startPosition }, bot, 40);
                    sTravelNodeMap.m_nMapMtx.lock_shared();
                    TravelNodeRoute route = sTravelNodeMap.getRoute(startPosition, endPosition, beginPath, bot);       

                    string routeList;

                    for (auto node : route.getNodes())
                    {
                        routeList += node->getName() + "-";
                    }

                    if (!routeList.empty())
                        ai->TellPlayerNoFacing(GetMaster(), routeList);

                    route.cleanTempNodes();                 

                    sTravelNodeMap.m_nMapMtx.unlock_shared();
                }
            }
            else
            {
                //Use standard pathfinder to find a route. 
                movePosition = endPosition;
            }
        }
    }

    if (movePath.empty() && movePosition.distance(startPosition) > maxDist)
    {
        //Use standard pathfinder to find a route. 
        PathFinder path(mover);
        path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = path.getPathType();
        PointsArray& points = path.getPath();
        movePath.addPath(startPosition.fromPointsArray(points));
    }

    if (!lastMove.lastPath.empty() && !movePath.empty() && lastMove.lastPath.getBack().distance(endPosition) <= movePath.getBack().distance(endPosition)) //new path is worse than the last path. Keep going the old path.
    {
        movePath = lastMove.lastPath;
    }

    if (!movePath.empty())
    {
        float oldDist;
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            oldDist = WorldPosition().getPathLength(movePath.getPointPath());
        if (!bot->GetTransport() && movePath.makeShortCut(startPosition, sPlayerbotAIConfig.reactDistance, bot))
            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(GetMaster(), "Found a shortcut: old=" + to_string(uint32(oldDist)) + "y new=" + to_string(uint32(WorldPosition().getPathLength(movePath.getPointPath()))));

        if (movePath.empty())
        {

            AI_VALUE(LastMovement&, "last movement").setPath(movePath);

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(GetMaster(), "Too far from path. Rebuilding.");
            return true;
        }

        TravelNodePathType pathType = TravelNodePathType::none;
        uint32 entry = 0;
        WorldPosition telePosition = WorldPosition();
        movePosition = movePath.getNextPoint(startPosition, maxDist, pathType, entry, bot->GetTransport(), telePosition);

        if (pathType == TravelNodePathType::staticPortal && entry)// && !ai->isRealPlayer())
        {
            //Log bot movement
            if (sPlayerbotAIConfig.hasLog("bot_movement.csv"))
            {
                WorldPosition telePos;
                if (entry)
                {
                    AreaTrigger const* at = sObjectMgr.GetAreaTrigger(entry);
                    if (at)
                        telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
                }
                else
                    telePos = movePosition;

                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << bot->GetName() << ",";
                if (telePos && telePos != movePosition)
                    startPosition.printWKT({ startPosition, movePosition, telePos }, out, 1);
                else
                    startPosition.printWKT({ startPosition, movePosition }, out, 1);

                out << to_string(bot->getRace()) << ",";
                out << to_string(bot->getClass()) << ",";
                float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
                out << subLevel << ",";
                out << (entry ? entry : -1);

                sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
            }

            GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(entry);
            if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER)
                return false;

            uint32 spellId = goInfo->spellcaster.spellId;
            const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

            if (pSpellInfo->EffectTriggerSpell[0])
                pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

            if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
                return false;

            if (bot->IsMounted())
            {
                if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
                    return false;

                WorldPacket emptyPacket;
                bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
                bot->UpdateSpeed(MOVE_RUN, true);
                bot->UpdateSpeed(MOVE_RUN, false);

                if (bot->IsFlying())
                    bot->GetMotionMaster()->MoveFall();
            }

            Spell* spell = new Spell(bot, pSpellInfo, false);
            SpellCastTargets targets;
            targets.setUnitTarget(bot);
#ifdef MANGOS
            spell->prepare(&targets, NULL);
#endif
#ifdef CMANGOS
            spell->SpellStart(&targets, NULL);
#endif
            SpellCastResult castResult = spell->cast(true);

            return castResult == SPELL_CAST_OK;
        }

        if (pathType == TravelNodePathType::areaTrigger)
        {
            //Log bot movement
            if (sPlayerbotAIConfig.hasLog("bot_movement.csv"))
            {
                WorldPosition telePos;
                if (entry)
                {
                    AreaTrigger const* at = sObjectMgr.GetAreaTrigger(entry);
                    if (at)
                        telePos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);
                }
                else
                    telePos = movePosition;

                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << bot->GetName() << ",";
                if (telePos && telePos != movePosition)
                    startPosition.printWKT({ startPosition, movePosition, telePos }, out, 1);
                else
                    startPosition.printWKT({ startPosition, movePosition }, out, 1);

                out << to_string(bot->getRace()) << ",";
                out << to_string(bot->getClass()) << ",";
                float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
                out << subLevel << ",";
                out << (entry ? entry : -1);

                sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
            }

            if (entry)
            {
                AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = entry;
            }
            else
                return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), movePosition.getO(), 0);
        }

        if (pathType == TravelNodePathType::transport)
        {
            if (!bot->GetTransport()) //We are not yet on a transport.
            {
                for (auto& transport : startPosition.getTransports(entry))
                {
                    if (startPosition.isOnTransport(transport))
                    {
                        WorldPosition botPos(bot);
                        transport->AddPassenger(bot, true);
                        bot->NearTeleportTo(bot->m_movementInfo.pos.x, bot->m_movementInfo.pos.y, bot->m_movementInfo.pos.z, bot->m_movementInfo.pos.o);
                        MANGOS_ASSERT(botPos.fDist(bot) < 500.0f);
                        entry = 0;
                        return true;
                    }
                    else
                    {
                        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                            ai->TellPlayer(GetMaster(), "transport at " + to_string(uint32(telePosition.distance(transport))) + "yards of entry");

                        if (telePosition.distance(transport) < INTERACTION_DISTANCE) //Transport has arrived Move on.
                        {
                            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                                ai->TellPlayerNoFacing(GetMaster(), "Moving on to transport " + string(transport->GetName()));

                            movePosition = WorldPosition(transport);
                            movePosition.setZ(bot->GetPositionZ());

                            WorldPosition botPos(bot);                           
                            transport->AddPassenger(bot, true);
                            bot->NearTeleportTo(bot->m_movementInfo.pos.x, bot->m_movementInfo.pos.y, bot->m_movementInfo.pos.z, bot->m_movementInfo.pos.o);
                            MANGOS_ASSERT(botPos.fDist(bot) < 500.0f);
                            ai->SetMoveToTransport(true);

                            for (float angle = 0; angle < 8; angle++)
                            {
                                WorldPosition onBoatPos(movePosition);
                                if(bot->GetTransport()->IsTransport())
                                    onBoatPos += WorldPosition(0, cos(angle / 4 * M_PI_F) * 5.0f, sin(angle / 4 * M_PI_F) * 10.0f);
                                vector<WorldPosition> step = onBoatPos.getPathStepFrom(bot, bot);
                                if (!step.empty() && abs(step.back().getZ() - movePosition.getZ()) < 2.0f)
                                {
                                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                                        ai->TellPlayerNoFacing(GetMaster(), "Found spot on boat moving to random place around");
                                    movePosition = step.back();
                                    break;
                                }
                            }

                            entry = 0;
                        }
                    }
                }
                if (entry) //We are not on a transport. Wait for it.
                {
                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                        ai->TellPlayerNoFacing(GetMaster(), "Waiting on transport");

                    WaitForReach(1000.0f);

                    if (!urand(0, 10) || startPosition.sqDistance2d(movePosition) < INTERACTION_DISTANCE)
                    {
                        if (!movePosition.GetReachableRandomPointOnGround(bot, INTERACTION_DISTANCE * 2, true))
                            return true;

                        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                            ai->TellPlayerNoFacing(GetMaster(), "Wandering while waiting.");
                    }
                    else
                        return true;
                }
            }
            else //We are on transport.
            {
                if (ai->GetMoveToTransport() && startPosition.isOnTransport(bot->GetTransport()))
                {
                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                        ai->TellPlayerNoFacing(GetMaster(), "I'm on " + string(bot->GetTransport()->GetName()));
                    ai->SetMoveToTransport(false);
                    entry = 0;
                }

                if (movePosition.getMapId() == bot->GetMapId() && ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayer(GetMaster(), "transport at " + to_string(uint32(telePosition.distance(bot->GetTransport()))) + "yards of exit");

                if (movePosition.getMapId() == bot->GetMapId() && telePosition.distance(bot->GetTransport()) < INTERACTION_DISTANCE) //We have arived move off.
                {
                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                        ai->TellPlayerNoFacing(GetMaster(), "Moving off transport");

                    WorldPosition botPos(bot);
                    bot->GetTransport()->RemovePassenger(bot);
                    bot->NearTeleportTo(bot->m_movementInfo.pos.x, bot->m_movementInfo.pos.y, bot->m_movementInfo.pos.z, bot->m_movementInfo.pos.o);
                    MANGOS_ASSERT(botPos.fDist(bot) < 500.0f);
                    bot->StopMoving();
                }
                else //We are traveling with the boat.
                {
                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                        ai->TellPlayerNoFacing(GetMaster(), "Traveling with transport");

                    WaitForReach(1000.0f);

                    if (!urand(0, 10))
                    {
                        movePosition = bot;
                        if (movePosition.ComputePathToRandomPoint(bot, 10, true).empty() || !movePosition || movePosition.mapid != bot->GetMapId() || !movePosition.isOnTransport(bot->GetTransport()))
                            return true;

                        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                            ai->TellPlayerNoFacing(GetMaster(), "Wandering to random spot on boat");
                    }
                    else
                        return true;
                }
            }
        }

        if (pathType == TravelNodePathType::flightPath && entry)
        {
            TaxiPathEntry const* tEntry = sTaxiPathStore.LookupEntry(entry);

            if (tEntry)
            {
                Creature* unit = nullptr;

                if (!bot->m_taxi.IsTaximaskNodeKnown(tEntry->from))
                {
                    list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
                    for (list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
                    {
                        Creature* unit = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_FLIGHTMASTER);
                        if (!unit)
                            continue;

                        bot->GetSession()->SendLearnNewTaxiNode(unit);

                        unit->SetFacingTo(unit->GetAngle(bot));
                    }
                }

                uint32 botMoney = bot->GetMoney();
                if (ai->HasCheat(BotCheatMask::gold) || ai->HasCheat(BotCheatMask::taxi))
                {
                    bot->SetMoney(10000000);
                }

                bool goTaxi = bot->ActivateTaxiPathTo({ tEntry->from, tEntry->to }, unit, 1);

                if (ai->HasCheat(BotCheatMask::gold) || ai->HasCheat(BotCheatMask::taxi))
                {
                    bot->SetMoney(botMoney);
                }

                return goTaxi;
            }
        }

        if (pathType == TravelNodePathType::teleportSpell && entry)
        {
            if (entry == 8690)
            {
                if (AI_VALUE2(bool, "action useful", "hearthstone") && (!bot->IsFlying() || WorldPosition(bot).currentHeight() < 10.0f))
                {
                    return ai->DoSpecificAction("hearthstone", Event("move action"), true);

                }
                else
                {
                    movePath.clear();
                    AI_VALUE(LastMovement&, "last movement").setPath(movePath);
                    return false;
                }
            }
            else
            {
                if (sServerFacade.IsSpellReady(bot, entry) && (!bot->IsFlying() || WorldPosition(bot).currentHeight() < 10.0f))
                    if (ai->DoSpecificAction("cast", Event("rpg action", to_string(entry)), true))
                        return true;

                movePath.clear();
                AI_VALUE(LastMovement&, "last movement").setPath(movePath);
                return false;

            }
        }

        if (pathType == TravelNodePathType::walk && movePath.getPath().begin()->type != PathNodeType::NODE_FLIGHTPATH)
            isWalking = true;

        //if (!isTransport && bot->GetTransport())
        //    bot->GetTransport()->RemovePassenger(bot);
    }
    else if (bot->GetTransport()) //Wait until we can recalculate.
        return false;

    if(!movePath.empty() && movePath.getBack().distance(movePath.getFront()) > maxDist)
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

    if (!movePosition || movePosition.getMapId() != bot->GetMapId())
    {        
        if(!bot->GetTransport() || movePath.getPath().size() == 1)
            movePath.clear();
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))        
            ai->TellPlayerNoFacing(GetMaster(), "No point. Rebuilding.");

        return false;
    }

    if (movePosition.distance(startPosition) > maxDist && !bot->GetTransport())
    {//Use standard pathfinder to find a route. 
        PathFinder path(mover);
        path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = path.getPathType();
        PointsArray& points = path.getPath();
        movePath.addPath(startPosition.fromPointsArray(points));
        TravelNodePathType pathType;
        uint32 entry;
        WorldPosition telepos;
        movePosition = movePath.getNextPoint(startPosition, maxDist, pathType, entry, false, telepos);
    }

    //Stop the path when we might get aggro.
    if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT) && !bot->IsDead() && !ignoreEnemyTargets) 
    {
        list<ObjectGuid> targets = AI_VALUE_LAZY(list<ObjectGuid>, "possible attack targets");

        if (!targets.empty() && movePosition)
        {
            PathFinder path(mover);
            path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
            PathType type = path.getPathType();
            PointsArray& points = path.getPath();
            bool foundAggro = false;

            for (auto p : points)
            {
                WorldPosition point(startPosition.getMapId(), p.x, p.y, p.z, startPosition.getO());
                for (auto target : targets)
                {
                    if (!target.IsCreature())
                        continue;

                    Unit* unit = ai->GetUnit(target);
                    if (!unit)
                        continue;

                    if (unit->IsDead())
                        continue;

                    if (unit->GetLevel() > bot->GetLevel() + 5)
                        continue;

                    float range = unit->GetAttackDistance(bot);

                    if (WorldPosition(unit).sqDistance(point) > range * range)
                        continue;

                    if (!unit->CanAttackOnSight(bot))
                        continue;

                    if (!unit->IsWithinLOSInMap(bot))
                        continue;

                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                        ai->TellPlayerNoFacing(GetMaster(), "Found " + chat->formatWorldobject(unit) + " stopping early.");

                    movePosition = point;
                    foundAggro = true;
                    break;
                }
                if (foundAggro)
                    break;
            }
        }
    }


    if (movePosition == WorldPosition()) {
        movePath.clear();
        AI_VALUE(LastMovement&, "last movement").setPath(movePath);

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(GetMaster(), "No point. Rebuilding.");
        return false;
    }

    //Visual waypoints
    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        if (!movePath.empty())
        {
            for (auto i : movePath.getPath())
            {
                if(i.point.getMapId() == bot->GetMapId())
                    CreateWp(bot, i.point.getX(), i.point.getY(), i.point.getZ(), 0.0, 2334);
            }
        }
        else
            CreateWp(bot, movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0, 2334, true);
    }

    //Log bot movement
    if (sPlayerbotAIConfig.hasLog("bot_movement.csv") && lastMove.lastMoveShort != movePosition)
    {
        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        startPosition.printWKT({ startPosition, movePosition }, out, 1);
        out << to_string(bot->getRace()) << ",";
        out << to_string(bot->getClass()) << ",";
        float subLevel = ((float)bot->GetLevel() + ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)));
        out << subLevel << ",";
        out << 0;

        sPlayerbotAIConfig.log("bot_movement.csv", out.str().c_str());
    }

    if (!react)
    {
        if (totalDistance > maxDist)
        {
            WaitForReach(startPosition.distance(movePosition) - 10.0f);
        }
        else
        {
            WaitForReach(startPosition.distance(movePosition));
        }
    }

    if (!isVehicle)
    {
        bot->HandleEmoteState(0);
        if (!bot->IsStandState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);

        if (bot->IsNonMeleeSpellCasted(true, false, true))
        {
            ai->InterruptSpell(false);
        }
    }

    MotionMaster& mm = *mover->GetMotionMaster();

    /* Why do we do this?
    if (lastMove.lastMoveShort.distance(movePosition) < minDist)
    {
        bot->StopMoving();
        mm.Clear();
    }
    */

    // Clean movement if not already moving the same way.
    if (mm.GetCurrent()->GetMovementGeneratorType() != POINT_MOTION_TYPE)
    {
        if (mover == bot)
            ai->StopMoving();
        else if (mover)
            mover->InterruptMoving(true);
    }

    if (totalDistance > maxDist && !detailedMove && !ai->HasPlayerNearby(movePosition)) //Why walk if you can fly?
    {
        time_t now = time(0);

        AI_VALUE(LastMovement&, "last movement").nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));

        return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), startPosition.getAngleTo(movePosition));
    }

    // walk if master walks and is close
    bool masterWalking = false;
    Unit* master = ai->GetMaster();
    if (master && sServerFacade.IsFriendlyTo(bot, master))
    {
        if (master->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE) && sServerFacade.GetDistance2d(bot, master) < 20.0f)
            masterWalking = true;
    }

    // Prevent moving if requested to move into a hazard
    if(IsHazardNearPosition(movePosition))
    {
        if(!react)
        {
            SetDuration(sPlayerbotAIConfig.reactDelay);
        }

        return false;
    }

#ifdef MANGOSBOT_ZERO
    Movement::PointsArray path;
    if(GeneratePathAvoidingHazards(movePosition, generatePath, path))
    {
        mm.MovePath(path, masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, false, false);
        WaitForReach(path);
    }
    else
    {
        mm.MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);
    }
    
#else
    if (!bot->IsFreeFlying())
    {
        Movement::PointsArray path;
        if (GeneratePathAvoidingHazards(movePosition, generatePath, path))
        {
            mm.MovePath(path, masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, false, false);
            WaitForReach(path);
        }
        else
        {
            mm.MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);
        }
    }
    else
    {
        bool needFly = false;
        bool needLand = false;
        bool isFly = bot->IsFlying();
        bool isFar = false;

        // if bot is on flying mount, fly up or down depending on distance to target
        if (totalDistance > maxDist && isWalking)
        {
            isFar = true;
            needFly = true;
            // only use in clear LOS betweek points
            Position pos = bot->GetPosition();
#ifdef MANGOSBOT_TWO
            if (!bot->GetMap()->IsInLineOfSight(pos.x, pos.y, pos.z + 100.f, movePosition.getX(), movePosition.getY(), movePosition.getZ() + 100.f, bot->GetPhaseMask(), true))
#else
            if (!bot->GetMap()->IsInLineOfSight(pos.x, pos.y, pos.z + 100.f, movePosition.getX(), movePosition.getY(), movePosition.getZ() + 100.f, true))
#endif
                needFly = false;

            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                if (needFly)
                {
                    // get ground level data at next waypoint
                    float height = terrain->GetHeightStatic(movePosition.getX(), movePosition.getY(), movePosition.getZ());
                    float ground = terrain->GetWaterOrGroundLevel(movePosition.getX(), movePosition.getY(), movePosition.getZ(), height);

                    float botheight = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                    float botground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), botheight);

                    // fly up if destination is far
                    if (totalDistance > maxDist && ground <= movePosition.getZ()) // check if ground level is not higher than path (tunnels)
                    {
                        movePosition.setZ(min(max(ground, botground) + 100.0f, max(movePosition.getZ() + 10.0f, bot->GetPositionZ() + 10.0f)));
                    }
                    else
                    {
                        movePosition.setZ(max(max(ground, botground), bot->GetPositionZ() - 10.0f));
                    }
                }
            }
        }

        if (!isFar && !isFly && originalZ > bot->GetPositionZ() && (originalZ - bot->GetPositionZ()) > 5.0f)
            needFly = true;

        if (needFly && !isFly)
        {
            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
        }

        if (!isFar && isFly)
        {
            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
                if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ < 5.0f) && (fabs(originalZ - ground) < 5.0f))
                    needLand = true;
            }
            if (needLand)
            {
                WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
                data << bot->GetPackGUID();
                bot->SendMessageToSet(data, true);

                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING2);
#endif
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);
            }
        }
        mm.MovePoint(movePosition.getMapId(), Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f), bot->IsFlying() ? FORCED_MOVEMENT_FLIGHT : FORCED_MOVEMENT_RUN, bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.f, bot->IsFlying());
    }
#endif

    AI_VALUE(LastMovement&, "last movement").setShort(startPosition, movePosition);

    if (!idle)
        ClearIdleState();

    return true;
}

bool MovementAction::MoveTo(Unit* target, float distance)
{
    if (!target || !target->IsInWorld())
    {
        //ai->TellError("Seems I am stuck");
        return false;
    }

    float bx = bot->GetPositionX(), by = bot->GetPositionY(), bz = bot->GetPositionZ();
    float tx = target->GetPositionX(), ty = target->GetPositionY(), tz = target->GetPositionZ();

    if (sServerFacade.IsHostileTo(bot, target))
    {
        Stance* stance = AI_VALUE(Stance*, "stance");
        WorldLocation loc = stance->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.mapid == -1)
        {
            //ai->TellError("Nowhere to move");
            return false;
        }

        tx = loc.coord_x;
        ty = loc.coord_y;
        tz = loc.coord_z;
    }

    float distanceToTarget = sServerFacade.GetDistance2d(bot, tx, ty);
    if (sServerFacade.IsDistanceGreaterThan(distanceToTarget, sPlayerbotAIConfig.targetPosRecalcDistance))
    {
        /*
        float angle = bot->GetAngle(tx, ty);
        float needToGo = distanceToTarget - distance;

        float maxDistance = ai->GetRange("spell");
        if (needToGo > 0 && needToGo > maxDistance)
            needToGo = maxDistance;
        else if (needToGo < 0 && needToGo < -maxDistance)
            needToGo = -maxDistance;

        float dx = cos(angle) * needToGo + bx;
        float dy = sin(angle) * needToGo + by;
        float dz = bz + (tz - bz) * needToGo / distanceToTarget;
        */

        float dx = tx;
        float dy = ty;
        float dz = tz;
        return MoveTo(target->GetMapId(), dx, dy, dz);
    }

    return true;
}

float MovementAction::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = master ? master->GetGroup() : bot->GetGroup();
    if (!group || group->GetMembersCount() == 1)
        return 0.0f;

    int index = 1;
    for (GroupReference *ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if( ref->getSource() == master)
            continue;

        if( ref->getSource() == bot)
            return 2 * M_PI / (group->GetMembersCount() -1) * index;

        index++;
    }
    return 0;
}

bool MovementAction::IsMovingAllowed(Unit* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

    float distance = sServerFacade.GetDistance2d(bot, target);
    if (!bot->InBattleGround() && distance > sPlayerbotAIConfig.reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    float distance = sqrt(bot->GetDistance(x, y, z));
    if (!bot->InBattleGround() && distance > sPlayerbotAIConfig.reactDistance)
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed()
{
    return ai->CanMove();
}

bool MovementAction::Follow(Unit* target, float distance)
{
    if (!distance)
        distance = ai->GetRange("follow");
    return Follow(target, distance, GetFollowAngle());
}

void MovementAction::UpdateMovementState()
{
    if (bot->IsInWater() || sServerFacade.IsUnderwater(bot))
    {
		bot->m_movementInfo.AddMovementFlag(MOVEFLAG_SWIMMING);
        bot->UpdateSpeed(MOVE_SWIM, true);
    }
    else
    {
		bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_SWIMMING);
        bot->UpdateSpeed(MOVE_SWIM, true);
    }

#ifndef MANGOSBOT_ZERO
    if (bot->IsFlying())
        bot->UpdateSpeed(MOVE_FLIGHT, true);
#endif

    // Temporary speed increase in group
    //if (ai->HasRealPlayerMaster())
    //    bot->UpdateSpeed(MOVE_RUN, true, 1.1f);
}

bool MovementAction::Follow(Unit* target, float distance, float angle)
{
    MotionMaster &mm = *bot->GetMotionMaster();

    distance = distance <= target->GetObjectBoundingRadius() ? 0 : distance - target->GetObjectBoundingRadius();

    UpdateMovementState();

    if (FollowOnTransport(target))
        return true;

    if (!target)
        return false;

    //Move to target corpse if alive.
    if (!target->IsAlive() && bot->IsAlive() && target->GetObjectGuid().IsPlayer())
    {
        Player* pTarget = (Player*)target;

        Corpse* corpse = pTarget->GetCorpse();

        if (corpse)
        {
            WorldPosition botPos(bot);
            WorldPosition cPos(corpse);

            if(botPos.fDist(cPos) > sPlayerbotAIConfig.spellDistance)
                return MoveTo(cPos.getMapId(),cPos.getX(),cPos.getY(), cPos.getZ());
            return false;
        }
    }

    if (sServerFacade.IsDistanceGreaterOrEqualThan(sServerFacade.GetDistance2d(bot, target), sPlayerbotAIConfig.sightDistance) || (target->IsFlying() && !bot->IsFreeFlying()) || target->IsTaxiFlying()/* || bot->GetTransport()*/)
    {
        if (target->GetObjectGuid().IsPlayer())
        {
            Player* pTarget = (Player*)target;

            if (pTarget->GetPlayerbotAI()) //Try to move to where the bot is going if it is closer and in the same direction.
            {
                WorldPosition botPos(bot);
                WorldPosition tarPos(target);
                WorldPosition longMove = pTarget->GetPlayerbotAI()->GetAiObjectContext()->GetValue<WorldPosition>("last long move")->Get();

                if (longMove)
                {
                    float lDist = botPos.fDist(longMove);
                    float tDist = botPos.fDist(tarPos);
                    float ang = botPos.getAngleBetween(tarPos, longMove);
                    if ((lDist * 1.5 < tDist && ang < M_PI_F / 2) || target->IsTaxiFlying())
                    {
                        return MoveTo(longMove.getMapId(), longMove.getX(), longMove.getY(), longMove.getZ());
                    }
                }
            }

            if (pTarget->IsTaxiFlying()) //Move to where the player is flying to.
            {
                const Taxi::Map tMap = pTarget->GetTaxiPathSpline();
                if (!tMap.empty())
                {
                    auto tEnd = tMap.back();

                    if (tEnd)
                        return MoveTo(tEnd->mapid, tEnd->x, tEnd->y, tEnd->z);
                }
            }
        }

        if (!target->IsTaxiFlying()/* || bot->GetTransport()*/)
           return MoveTo(target, ai->GetRange("follow"));
    }

    bot->HandleEmoteState(0);
    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCasted(true))
    {
        bot->CastStop();
        ai->InterruptSpell();
    }

    AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

#ifndef MANGOSBOT_ZERO
    if (bot->IsFreeFlying())
    {
        if (!bot->IsFlying() && target->IsFlying())
        {
            //Take off
            WorldPacket data(SMSG_SPLINE_MOVE_SET_FLYING, 9);
            data << bot->GetPackGUID();
            bot->SendMessageToSet(data, true);

            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FLYING2);
#endif
            if (!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                bot->m_movementInfo.AddMovementFlag(MOVEFLAG_LEVITATING);
        }

        if (bot->IsFlying() && !target->IsFlying())
        {
            //Land
            bool needLand = false;

            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
                if (bot->GetPositionZ() < ground + 5.0f)
                    needLand = true;
            }
            if (needLand)
            {
                WorldPacket data(SMSG_SPLINE_MOVE_UNSET_FLYING, 9);
                data << bot->GetPackGUID();
                bot->SendMessageToSet(data, true);

                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING);
#ifdef MANGOSBOT_ONE
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FLYING2))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FLYING2);
#endif
                if (bot->m_movementInfo.HasMovementFlag(MOVEFLAG_LEVITATING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_LEVITATING);

                if(!bot->m_movementInfo.HasMovementFlag(MOVEFLAG_FALLING))
                    bot->m_movementInfo.RemoveMovementFlag(MOVEFLAG_FALLING);
            }
        }
    }
#endif

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit* currentTarget = sServerFacade.GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetObjectGuid() == target->GetObjectGuid() && sServerFacade.GetChaseAngle(bot) == angle && sServerFacade.GetChaseOffset(bot) == distance)
            return false;
    }

    mm.MoveFollow(target, distance, angle, true, sPlayerbotAIConfig.boostFollow);
    return true;
}

Vector3 CalculatePerpendicularPoint(const Vector3& A, const Vector3& B, float offset, bool left = true)
{
    const Vector3 direction = (B - A).directionOrZero();
    Vector3 perpendicularDirection(-direction.y, direction.x, direction.z);

    if (!left)
    {
        perpendicularDirection.x *= -1.0f;
        perpendicularDirection.y *= -1.0f;
    }

    return B + (perpendicularDirection * offset);
}

bool MovementAction::ChaseTo(WorldObject* obj, float distance, float angle)
{
    if (!IsMovingAllowed())
    {
        return false;
    }

    if (!ai->IsSafe(obj))
        return false;

#ifdef MANGOSBOT_TWO
    TransportInfo* transportInfo = bot->GetTransportInfo();
    if (transportInfo && transportInfo->IsOnVehicle())
    {
        Unit* vehicle = (Unit*)transportInfo->GetTransport();
        VehicleSeatEntry const* seat = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat());
        if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_CONTROL))
            return false;

        //vehicle->GetMotionMaster()->Clear();
        return MoveNear(obj, 30.0f);
        //vehicle->GetMotionMaster()->MoveChase((Unit*)obj, 30.0f, angle);
        //return true;
    }
#endif

    if (ai->HasStrategy("behind", BotState::BOT_STATE_COMBAT))
        angle = GetFollowAngle() / 3 + obj->GetOrientation() + M_PI;

    UpdateMovementState();

    bot->HandleEmoteState(0);
    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

#ifndef MANGOSBOT_ZERO
    if (bot->InArena())
        return MoveNear(obj, std::max(ATTACK_DISTANCE, distance));
#endif

    // Calculate the chase position
    const WorldPosition botPosition(bot);
    const WorldPosition targetPosition(obj);
    const Vector3 botPoint = botPosition.getVector3();
    const Vector3 targetPoint = targetPosition.getVector3();

    const float distanceToTarget = botPosition.distance(targetPosition);

    if (distanceToTarget > sPlayerbotAIConfig.sightDistance)
        return MoveTo(targetPosition.getMapId(), targetPosition.getX(), targetPosition.getY(), targetPosition.getZ());

    const Vector3 directionToTarget = (targetPoint - botPoint).directionOrZero();
    const Vector3 endPoint = botPoint + (directionToTarget * std::min(distance, distanceToTarget));
    WorldPosition endPosition(obj->GetMapId(), endPoint.x, endPoint.y, endPoint.z);
    endPosition.setZ(endPosition.getHeight());

    // Check if the end position is inside a hazard
    HazardPosition hazardPosition;
    if (IsHazardNearPosition(endPosition, &hazardPosition))
    {
        // Try to generate a nearby position outside the hazard
        const Vector3 hazardPoint = hazardPosition.first.getVector3();
        const float hazardRangeOffset = hazardPosition.second * 1.5f;

        // Generate point translated to the left
        Vector3 possibleEndPoint = CalculatePerpendicularPoint(endPoint, hazardPoint, hazardRangeOffset, true);

        // Check if point is valid
        WorldPosition possibleEndPosition(bot->GetMapId(), possibleEndPoint.x, possibleEndPoint.y, possibleEndPoint.z);
        if (IsValidPosition(possibleEndPosition, botPosition))
        {
            endPosition.coord_x = possibleEndPoint.x;
            endPosition.coord_y = possibleEndPoint.y;
            endPosition.coord_z = possibleEndPoint.z;
        }
        else
        {
            // Generate point translated to the right
            possibleEndPoint = CalculatePerpendicularPoint(endPoint, hazardPoint, hazardRangeOffset, false);

            endPosition.coord_x = possibleEndPoint.x;
            endPosition.coord_y = possibleEndPoint.y;
            endPosition.coord_z = possibleEndPoint.z;
        }
    }

    MotionMaster& mm = *bot->GetMotionMaster();

    // Prevent moving if requested to move into a hazard
    if (IsValidPosition(endPosition, botPosition))
    {
        Movement::PointsArray path;
        if (GeneratePathAvoidingHazards(endPosition, true, path))
        {
            mm.Clear(false, true);
            mm.MovePath(path, FORCED_MOVEMENT_RUN, false, false);
            WaitForReach(path);
            return true;
        }
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        if (!bot->IsStopped() &&
            sServerFacade.GetChaseTarget(bot) == obj && 
            sServerFacade.GetChaseOffset(bot) == distance)
        {
            return true;
        }
    }

    // charge
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE && !bot->IsStopped())
    {
        return false;
    }

    if (!ai->IsSafe(obj)) return false;

    if (!endPosition.isValid()) return false;
    if (angle > 20) angle = 0;
    mm.MoveChase((Unit*)obj, distance, angle);
    float dist = sServerFacade.GetDistance2d(bot, obj);
    float distDiff = dist > distance ? dist - distance : 0.f;
    WaitForReach(distDiff);

    return true;
}

float MovementAction::MoveDelay(float distance)
{
    return distance / bot->GetSpeed(MOVE_RUN);
}

bool MovementAction::FollowOnTransport(Unit* target)
{
    bool const onDifferentTransports = bot->m_movementInfo.t_guid != target->m_movementInfo.t_guid;
    if (onDifferentTransports && sServerFacade.IsDistanceLessOrEqualThan(sServerFacade.GetDistance2d(bot, target), sPlayerbotAIConfig.sightDistance))
    {
        ai->StopMoving();
        bool sendHeartbeat = false;

        if (GenericTransport* pMyTransport = bot->GetTransport())
        {
            sendHeartbeat = true;
            pMyTransport->RemovePassenger(bot);
            bot->Relocate(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
        }

        if (GenericTransport* pHisTransport = target->GetTransport())
        {
            sendHeartbeat = true;
            bot->Relocate(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            pHisTransport->AddPassenger(bot);
        }

        if (sendHeartbeat)
            bot->SendHeartBeat();

        return true;
    }

    return false;
}


void MovementAction::WaitForReach(float distance)
{
    float duration = 1000.0f * MoveDelay(distance) + sPlayerbotAIConfig.reactDelay;
    if (duration > sPlayerbotAIConfig.maxWaitForMove)
        duration = sPlayerbotAIConfig.maxWaitForMove;

    /*Unit* target = *ai->GetAiObjectContext()->GetValue<Unit*>("current target");
    Unit* player = *ai->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
    if ((player || target) && duration > sPlayerbotAIConfig.globalCoolDown)
        duration = sPlayerbotAIConfig.globalCoolDown;*/

    if (duration < 0.0f)
        duration = 0.0f;

    SetDuration(duration);
}

void MovementAction::WaitForReach(const Movement::PointsArray& path)
{
    float distance = 0.0f;
    if(!path.empty())
    {
        const Vector3* previousPoint = &path[0]; 
        for (auto it = path.begin() + 1; it != path.end(); ++it)
        {
            const Vector3& pathPoint = (*it);
            distance += (*previousPoint - pathPoint).length();
            previousPoint = &pathPoint;
        }
    }

    WaitForReach(distance);
}

bool MovementAction::Flee(Unit *target)
{
    Player* master = GetMaster();
    if (!target)
        target = master;

    if (!target)
        return false;

    if (!sPlayerbotAIConfig.fleeingEnabled)
        return false;

    if (!IsMovingAllowed())
    {
        ai->TellError(GetMaster(), "I am stuck while fleeing");
        return false;
    }

    HostileReference* ref = sServerFacade.GetThreatManager(target).getCurrentVictim();
    const bool isTarget = ref && ref->getTarget() == bot;

    time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
    time_t now = time(0);
    uint32 fleeDelay = urand(2, sPlayerbotAIConfig.returnDelay / 1000);

    // let hunter kite mob
    if (isTarget && bot->getClass() == CLASS_HUNTER)
    {
        fleeDelay = 1;
    }

    if (lastFlee && sServerFacade.isMoving(bot))
    {
        if ((now - lastFlee) <= fleeDelay)
        {
            return true;
        }
    }
    
    const bool isHealer = ai->IsHeal(bot);
    const bool isTank = ai->IsTank(bot);
    const bool isDps = !isHealer && !isTank;
    const bool isRanged = ai->IsRanged(bot);
    const bool needHealer = !isHealer && AI_VALUE2(uint8, "health", "self target") < 50;

    Unit* fleeTarget = nullptr;
    Group* group = bot->GetGroup();
    if (group)
    {
        Unit* spareTarget = nullptr;
        vector<Unit*> possibleTargets;
        const float minFleeDistance = 5.0f;
        const float maxFleeDistance = isTarget ? 40.0f : ai->GetRange("spell") * 1.5;
        const float minRangedTargetDistance = ai->GetRange("spell") / 2 + ai->GetRange("follow");

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* groupMember = gref->getSource();

            // Ignore group member if is not alive or on a different zone
            if (!groupMember || groupMember->IsBeingTeleported() || groupMember == bot || groupMember == master || !sServerFacade.IsAlive(groupMember) || bot->GetMapId() != groupMember->GetMapId())
                continue;

            // Don't flee to group member if too close or too far
            float const distanceToGroupMember = sServerFacade.GetDistance2d(bot, groupMember);
            if (distanceToGroupMember < minFleeDistance || distanceToGroupMember > maxFleeDistance)
                continue;

            if (PlayerbotAI* groupMemberBotAi = groupMember->GetPlayerbotAI())
            {
                // Ignore if the group member is affected by an aoe spell
                if (groupMemberBotAi->GetAiObjectContext()->GetValue<bool>("has area debuff", "self target")->Get())
                    continue;
            }

            // If the bot is currently being targeted
            if(isTarget)
            {
                // Try to flee to tank
                if (ai->IsTank(groupMember))
                {
                    float distanceToTank = sServerFacade.GetDistance2d(bot, groupMember);
                    float distanceToTarget = sServerFacade.GetDistance2d(bot, target);
                    if (distanceToTank > minFleeDistance && distanceToTank < maxFleeDistance)
                    {
                        possibleTargets.push_back(groupMember);
                    }
                }
            }
            else
            {
                // Try to flee to healers (group healers together or approach a healer if needed)
                if ((isHealer && ai->IsHeal(groupMember)) || needHealer)
                {
                    const float distanceToTarget = sServerFacade.GetDistance2d(groupMember, target);
                    if (distanceToTarget > minRangedTargetDistance && (needHealer || groupMember->IsWithinLOSInMap(target, true)))
                    {
                        possibleTargets.push_back(groupMember);
                    }
                }
                // Try to flee to ranged (group ranged together)
                else if (isRanged && ai->IsRanged(groupMember))
                {
                    const float distanceToTarget = sServerFacade.GetDistance2d(groupMember, target);
                    if (distanceToTarget > minRangedTargetDistance && groupMember->IsWithinLOSInMap(target, true))
                    {
                        possibleTargets.push_back(groupMember);
                    }
                }
            }
        }

        if (!possibleTargets.empty())
        {
            fleeTarget = possibleTargets[urand(0, possibleTargets.size() - 1)];
        }
        else
        {
            // If nothing was found, let's try the master
            if (master && sServerFacade.IsAlive(master) && master->IsWithinLOSInMap(target, true))
            {
                // Don't flee to group member if too close or too far
                float const distanceToMaster = sServerFacade.GetDistance2d(bot, master);
                if (distanceToMaster > minFleeDistance && distanceToMaster < maxFleeDistance)
                {
                    if(isRanged)
                    {
                        const float distanceToTarget = sServerFacade.GetDistance2d(master, target);
                        if (distanceToTarget > minRangedTargetDistance)
                        {
                            fleeTarget = master;
                        }
                    }
                    else
                    {
                        fleeTarget = master;
                    }
                }
            }
        }
    }

    bool succeeded = false;
    if (fleeTarget)
    {
        succeeded = MoveNear(fleeTarget);
    }

    if (!ai->HasRealPlayerMaster() && !ai->IsRealPlayer(target))
    {
        bool fullDistance = false;
        if (target->IsPlayer())
            fullDistance = true;
        if (WorldPosition(bot).isOverworld())
            fullDistance = true;

        float distance = fullDistance ? (ai->GetRange("flee") * 2) : ai->GetRange("flee");

        MotionMaster* mm = bot->GetMotionMaster();

        if (mm->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
        {
            ChaseMovementGenerator* chase = (ChaseMovementGenerator*)mm->GetCurrent();

            if (chase->GetCurrentTarget() == target && sServerFacade.GetChaseOffset(bot) == distance)
                return true;
        }

        mm->MoveChase(target, distance, WorldPosition(bot).getAngleTo(target), true, false, true, false);
        return true;
    }

    // Generate a position to flee
    if(!succeeded)
    {
        if (lastFlee && bot->GetGroup())
        {
            if (!lastFlee)
            {
                AI_VALUE(LastMovement&, "last movement").lastFlee = now;
            }
            else
            {
                if ((now - lastFlee) > fleeDelay)
                {
                    AI_VALUE(LastMovement&, "last movement").lastFlee = 0;
                }
                else
                {
                    succeeded = false;
                }
            }
        }
        bool fullDistance = false;
        if (target->IsPlayer())
            fullDistance = true;
        if (WorldPosition(bot).isOverworld())
            fullDistance = true;

        FleeManager manager(bot, fullDistance ? (ai->GetRange("flee") * 2) : ai->GetRange("flee"), bot->GetAngle(target) + M_PI);
        if (!manager.isUseful())
        {
            return false;
        }

        if (!urand(0, 50) && ai->HasStrategy("emote", BotState::BOT_STATE_NON_COMBAT))
        {
            vector<uint32> sounds;
            sounds.push_back(304); // guard
            sounds.push_back(306); // flee
            ai->PlayEmote(sounds[urand(0, sounds.size() - 1)]);
        }

        float rx, ry, rz;
        if (!manager.CalculateDestination(&rx, &ry, &rz))
        {
            ai->TellError(GetMaster(), "Nowhere to flee");
            return false;
        }

        if(MoveTo(target->GetMapId(), rx, ry, rz))
        {
            AI_VALUE(LastMovement&, "last movement").lastFlee = time(0);
            succeeded = true;
        }
    }

    return succeeded;
}

void MovementAction::ClearIdleState()
{
    context->GetValue<time_t>("stay time")->Set(0);
    context->GetValue<ai::PositionMap&>("position")->Get()["random"].Reset();
}

bool MovementAction::IsValidPosition(const WorldPosition& position, const WorldPosition& visibleFromPosition)
{
    const WorldPosition botPosition(bot);
    return botPosition.canPathTo(position, bot) &&
           MaNGOS::IsValidMapCoord(position.getX(), position.getY(), position.getZ(), 0.0f) &&
           position.IsInLineOfSight(visibleFromPosition, bot->GetCollisionHeight()) &&
           !IsHazardNearPosition(position);
}

bool MovementAction::IsHazardNearPosition(const WorldPosition& position, HazardPosition* outHazard)
{
    AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();
    list<HazardPosition> hazards = AI_VALUE(list<HazardPosition>, "hazards");
    if (!hazards.empty())
    {
        for (const HazardPosition& hazard : hazards)
        {
            const WorldPosition& hazardPosition = hazard.first;
            const float hazardRange = hazard.second;
            const float distance = position.distance(hazardPosition);
            if (distance <= hazardRange)
            {
                if (outHazard)
                {
                    *outHazard = hazard;
                }

                return true;
            }
        }
    }

    return false;
}

bool MovementAction::GeneratePathAvoidingHazards(const WorldPosition& endPosition, bool generatePath, Movement::PointsArray& outPath)
{
    if (generatePath)
    {
        list<HazardPosition> hazards = AI_VALUE(list<HazardPosition>, "hazards");
        if (!hazards.empty())
        {
            PathFinder path(bot);
            path.calculate(endPosition.getX(), endPosition.getY(), endPosition.getZ(), false);
            Movement::PointsArray& pathPoints = path.getPath();
            Movement::PointsArray collidingHazards;

            bool pathModified = false;
            if(pathPoints.size() >= 2)
            {
                if (pathPoints.size() == 2)
                {
                    pathPoints.push_back(pathPoints.back());
                }

                // Start the iteration on the second point (the first and last points can't be modified)
                bool firstPoint = true;
                uint8 pointsInserted = 0;
                const uint8 maxPointsInserted = 20;
                WorldPosition previousPosition(bot->GetMapId(), pathPoints.begin()->x, pathPoints.begin()->y, pathPoints.begin()->z);
                for (uint32 i = 1; i < pathPoints.size() - 1; i++)
                {
                    bool pointInserted = false;
                    Vector3& pathPoint = pathPoints[i];
                    for (const HazardPosition& hazard : hazards)
                    {
                        const float hazardRange = hazard.second;
                        const float hazardRangeOffset = hazardRange * 1.5f;
                        const Vector3& hazardPosition = hazard.first.getVector3();

                        // Check if the path point is inside a hazard
                        {
                            const float distanceToHazard = (pathPoint - hazardPosition).length();
                            if (distanceToHazard <= hazardRange)
                            {
                                collidingHazards.push_back(hazardPosition);

                                // Move the point out of the hazard range in perpendicular from previous point
                                // Generate point translated to the left
                                Vector3 possiblePathPoint = CalculatePerpendicularPoint(previousPosition.getVector3(), hazardPosition, hazardRangeOffset, true);

                                // Check if point is valid
                                WorldPosition possiblePathPosition(bot->GetMapId(), possiblePathPoint.x, possiblePathPoint.y, possiblePathPoint.z);
                                if (IsValidPosition(possiblePathPosition, previousPosition))
                                {
                                    pathModified = true;
                                    pathPoint = possiblePathPoint;
                                }
                                else
                                {
                                    // Generate point translated to the right
                                    possiblePathPoint = CalculatePerpendicularPoint(previousPosition.getVector3(), hazardPosition, hazardRangeOffset, false);

                                    // Check if point is valid
                                    WorldPosition possiblePathPosition(bot->GetMapId(), possiblePathPoint.x, possiblePathPoint.y, possiblePathPoint.z);
                                    if (IsValidPosition(possiblePathPosition, previousPosition))
                                    {
                                        pathModified = true;
                                        pathPoint = possiblePathPoint;
                                    }
                                }
                            }
                        }

                        // Check if the line between the previous point and the current point goes through a hazard
                        // Don't check for the line between the first point and second
                        if (!firstPoint && (pointsInserted < maxPointsInserted))
                        {
                            Vector3 directionFromPreviousPoint = (pathPoint - previousPosition.getVector3());
                            const float distanceToPreviousPoint = std::max(directionFromPreviousPoint.length(), 0.0001f);
                            directionFromPreviousPoint = directionFromPreviousPoint / distanceToPreviousPoint;
                            Vector3 inBetweenPathPoint = previousPosition.getVector3() + (directionFromPreviousPoint * distanceToPreviousPoint * 0.5f);

                            // Check if the point between path points is inside a hazard
                            Vector3 directionFromHazard = (inBetweenPathPoint - hazardPosition);
                            const float distanceToHazard = std::max(directionFromHazard.length(), 0.0001f);
                            if (distanceToHazard <= hazardRange)
                            {
                                collidingHazards.push_back(hazardPosition);

                                // If so generate a new path point to go around it
                                inBetweenPathPoint = hazardPosition + ((directionFromHazard / distanceToHazard) * hazardRangeOffset);

                                // Check if the point is valid
                                WorldPosition inBetweenPathPosition(bot->GetMapId(), inBetweenPathPoint.x, inBetweenPathPoint.y, inBetweenPathPoint.z);
                                if (IsValidPosition(inBetweenPathPosition, previousPosition))
                                {
                                    // Insert the new point to the path (before current point)
                                    pathModified = true;
                                    pointInserted = true;
                                    pathPoints.insert(pathPoints.begin() + i, inBetweenPathPoint);
                                    pointsInserted++;
                                    continue;
                                }
                            }
                        }
                    }

                    if(pointInserted)
                    {
                        // Go back one step to validate the inserted point and move to next loop
                        i--;
                    }
                    else
                    {
                        firstPoint = false;
                        previousPosition.coord_x = pathPoint.x;
                        previousPosition.coord_y = pathPoint.y;
                        previousPosition.coord_z = pathPoint.z;
                    }
                }

                if (pathModified && !pathPoints.empty())
                {
                    outPath = pathPoints;

                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
                    {
                        for (const Vector3& pathPoint : pathPoints)
                        {
                            bot->SummonCreature(1, pathPoint.x, pathPoint.y, pathPoint.z, 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);
                        }

                        for (const Vector3& hazards : collidingHazards)
                        {
                            bot->SummonCreature(15631, hazards.x, hazards.y, hazards.z, 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);
                        }
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

bool FleeAction::Execute(Event& event)
{
    return Flee(AI_VALUE(Unit*, "current target"));
}

bool FleeWithPetAction::Execute(Event& event)
{
    Pet* pet = bot->GetPet();
    if (pet)
    {
        UnitAI* creatureAI = ((Creature*)pet)->AI();
        if (creatureAI)
        {
            creatureAI->SetReactState(REACT_PASSIVE);
            pet->AttackStop();
        }
    }

    return Flee(AI_VALUE(Unit*, "current target"));
}

bool RunAwayAction::Execute(Event& event)
{
    return Flee(AI_VALUE(Unit*, "master target"));
}

bool MoveToLootAction::Execute(Event& event)
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    WorldObject *wo = loot.GetWorldObject(bot);
    return MoveNear(wo, sPlayerbotAIConfig.contactDistance);
}

bool MoveOutOfEnemyContactAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig.contactDistance);
}

bool MoveOutOfEnemyContactAction::isUseful()
{
    return MovementAction::isUseful() && AI_VALUE2(bool, "inside target", "current target");
}

bool SetFacingTargetAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->IsTaxiFlying())
        return true;

    sServerFacade.SetFacingTo(bot, target);
    //SetDuration(sPlayerbotAIConfig.globalCoolDown);
    return true;
}

bool SetFacingTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "facing", "current target");
}

bool SetFacingTargetAction::isPossible()
{
    if (sServerFacade.IsFrozen(bot) || bot->IsPolymorphed() ||
        (sServerFacade.UnitIsDead(bot) && !bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() ||
        bot->HasAuraType(SPELL_AURA_MOD_CONFUSE) || sServerFacade.IsCharmed(bot) ||
        bot->HasAuraType(SPELL_AURA_MOD_STUN) || bot->IsTaxiFlying() ||
        bot->hasUnitState(UNIT_STAT_CAN_NOT_REACT_OR_LOST_CONTROL))
        return false;

    return true;
}

bool SetBehindTargetAction::Execute(Event& event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float angle = GetFollowAngle() / 3 + target->GetOrientation() + M_PI / 2.0f;

    float distance = bot->GetCombinedCombatReach(target, true) * 0.8f;
    float x = target->GetPositionX() + cos(target->GetOrientation()) * -1.0f * distance,
        y = target->GetPositionY() + sin(target->GetOrientation()) * -1.0f * distance,
        z = target->GetPositionZ();
    bot->UpdateGroundPositionZ(x, y, z);

    // prevent going into terrain
    float ox, oy, oz;
    target->GetPosition(ox, oy, oz);
#ifdef MANGOSBOT_TWO
    target->GetMap()->GetHitPosition(ox, oy, oz + bot->GetCollisionHeight(), x, y, z, bot->GetPhaseMask(), -0.5f);
#else
    target->GetMap()->GetHitPosition(ox, oy, oz + bot->GetCollisionHeight(), x, y, z, -0.5f);
#endif

    const bool isLos = target->IsWithinLOS(x, y, z + bot->GetCollisionHeight(), true);
    bool moved = MoveTo(bot->GetMapId(), x, y, z);
    if (!moved && !isLos)
    {
        distance = sPlayerbotAIConfig.contactDistance;
        x = target->GetPositionX() + cos(angle) * distance;
        y = target->GetPositionY() + sin(angle) * distance;
        z = target->GetPositionZ();
        bot->UpdateGroundPositionZ(x, y, z);
        moved = MoveTo(bot->GetMapId(), x, y, z);
    }

    return moved;
}

bool SetBehindTargetAction::isUseful()
{
    if(!MovementAction::isUseful())
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && !bot->IsFacingTargetsBack(target))
    {
        // Don't move behind if the target is too far away
        const float distance = bot->GetDistance(target, false);
        return distance <= 15.0f;
    }

    return false;
}

bool SetBehindTargetAction::isPossible()
{
    if(MovementAction::isPossible())
    {
        // Check if the target is targeting the bot
        Unit* target = AI_VALUE(Unit*, "current target");
        if (target)
        {
            // If the target is a player
            Player* playerTarget = dynamic_cast<Player*>(target);
            if(playerTarget)
            {
                return bot->GetObjectGuid() != playerTarget->GetSelectionGuid();
            }
            // If the target is a NPC
            else 
            {
                return !(target->GetVictim() && (target->GetVictim()->GetObjectGuid() == bot->GetObjectGuid()));
            }
        }
    }

    return false;
}

bool MoveOutOfCollisionAction::Execute(Event& event)
{
    float angle = M_PI * 2000 / (float)urand(1, 1000);
    float distance = ai->GetRange("follow");
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
    if(!MovementAction::isUseful())
        return false;

#ifdef MANGOSBOT_TWO
    // do not avoid collision on vehicle
    if (ai->IsInVehicle())
        return false;
#endif

    return AI_VALUE2(bool, "collision", "self target") && ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest friendly players")->Get().size() < 15 &&
        ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest non bot players")->Get().size() > 0;
}

bool MoveRandomAction::Execute(Event& event)
{
    //uint32 randnum = bot->GetGUIDLow();                            //Semi-random but fixed number for each bot.
    //uint32 cycle = floor(WorldTimer::getMSTime() / (1000*60));     //Semi-random number adds 1 each minute.

    //randnum = ((randnum + cycle) % 1000) + 1;

    uint32 randnum = urand(1, 2000);

    float angle = M_PI  * (float)randnum / 1000; //urand(1, 1000);
    float distance = urand(20,200);

    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance, bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveRandomAction::isUseful()
{    
    return !ai->HasRealPlayerMaster() && ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest friendly players")->Get().size() > urand(25, 100);
}

bool MoveToAction::Execute(Event& event)
{
    list<GuidPosition> guidList = AI_VALUE(list<GuidPosition>, getQualifier());

    if (guidList.empty())
        return false;

    GuidPosition guid = guidList.front();

    return MoveTo(guid.getMapId(), guid.getX(), guid.getY(), guid.getZ());
}

bool JumpAction::isUseful()
{
    return bot->IsInWorld() && ai->HasPlayerNearby() && !ai->IsJumping();
}

bool JumpAction::Execute(ai::Event &event)
{
    // don't jump while casting without real player command
    if (!event.getOwner() && bot->IsNonMeleeSpellCasted(false, false, true))
        return false;

    string param = event.getParam();
    string qualify = getQualifier();
    string options = !param.empty() ? param : !qualify.empty() ? qualify : "";
    bool jumpInPlace = false;
    bool jumpBackward = false;
    bool showLanding = false;
    bool isRtsc = false;

    // only show landing
    if (options.find("show") != string::npos && options.size() > 5)
    {
        options = param.substr(5);
        showLanding = true;
    }
    // rtsc stuff
    if (options == "rtsc")
    {
        isRtsc = true;
    }
    // handle options
    if (options.empty() || options == "i" || options == "inplace")
    {
        jumpInPlace = true;
    }
    if (options == "r" || options == "random")
    {
        jumpInPlace = frand(0.0f, 1.0f) < sPlayerbotAIConfig.jumpInPlaceChance;
        jumpBackward = frand(0.0f, 1.0f) < sPlayerbotAIConfig.jumpBackwardChance;
        if (sServerFacade.isMoving(bot))
        {
            jumpInPlace = false;
        }
        if (jumpInPlace)
            jumpBackward = false;
    }
    if (options == "b" || options == "back")
    {
        jumpBackward = true;
        jumpInPlace = false;
    }
    if (options == "f" || options == "forward")
    {
        jumpInPlace = false;
        jumpBackward = false;
    }
    if (ai->HasStrategy("stay", ai->GetState()))
    {
        if (!jumpInPlace && !showLanding)
            return false;
    }

    // find jump position
    if (options == "tome" || options == "follow" || options == "chase" || isRtsc)
    {
        WorldPosition const src = WorldPosition(bot);
        WorldPosition dest = WorldPosition();
        WorldPosition jumpPoint = WorldPosition();
        float requiredSpeed = 0.f;

        float distanceTo = 30.f;
        float distanceFrom = 30.f;

        if (options == "tome")
        {
            if (!event.getOwner())
                return false;

            dest = WorldPosition(event.getOwner());
        }

        if (isRtsc)
        {
            WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
            if(!spellPosition)
            {
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
                ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);
                return false;
            }

            // already have point - movement handled by rtsc jump command
            WorldPosition jumpPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
            if (jumpPosition)
            {
                jumpPoint = jumpPosition;
                requiredSpeed = jumpPosition.getO();
                jumpPoint.orientation = dest.getO();
            }

            dest = spellPosition;

            distanceTo = sPlayerbotAIConfig.sightDistance;
            distanceFrom = 10.f;
        }

        if (options == "follow")
        {
            if (!ai->HasRealPlayerMaster())
                return false;

            Unit* followTarget = AI_VALUE(Unit*, "follow target");
            if (!followTarget || !ai->IsSafe(followTarget))
                return false;

            if ((bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE ||
            bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE) &&
            (bot->GetMotionMaster()->GetCurrent()->GetCurrentTarget() != followTarget ||
            bot->GetMotionMaster()->GetCurrent()->IsReachable() ||
            /*bot->InBattleGround() ||*/
            bot->GetTransport()))
                return false;

            dest = WorldPosition(followTarget);

            // do not jump if can walk, MotionMaster IsReachable() doesn't seem to return false if can't reach
            if (CanWalkTo(src, dest, bot))
                return false;

            distanceTo = 30.f;
            distanceFrom = 40.f;
        }

        if (options == "chase")
        {
            Unit* chaseTarget = AI_VALUE(Unit*, "current target");
            if (!chaseTarget || !ai->IsSafe(chaseTarget))
                return false;

            if ((bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE ||
            bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == POINT_MOTION_TYPE) &&
            (bot->GetMotionMaster()->GetCurrent()->GetCurrentTarget() != chaseTarget ||
            bot->GetMotionMaster()->GetCurrent()->IsReachable() ||
            /*bot->InBattleGround() ||*/
            bot->GetTransport()))
                return false;

            dest = WorldPosition(chaseTarget);

            // do not jump if can walk, MotionMaster IsReachable() doesn't seem to return false if can't reach
            if (CanWalkTo(src, dest, bot))
                return false;

            distanceTo = 10.f;
            distanceFrom = sPlayerbotAIConfig.sightDistance;
        }

        if (!jumpPoint)
            jumpPoint = GetPossibleJumpStartFor(src, dest, bot, requiredSpeed, distanceTo, distanceFrom);
        if (jumpPoint && requiredSpeed > 0.f)
        {
            sLog.outDetail("%s: GetPossibleJumpStartFor success! Jump speed: %f", bot->GetName(), requiredSpeed);
            if (showLanding)
            {
                Creature* wpCreature = bot->SummonCreature(2334, jumpPoint.getX(), jumpPoint.getY(), jumpPoint.getZ() - 1, bot->GetOrientation(), TEMPSPAWN_TIMED_DESPAWN, 3000);
                PlayerbotAI::AddAura(wpCreature, 246);

                float pointAngle = src.getAngleTo(jumpPoint);
                sServerFacade.SetFacingTo(bot, pointAngle, true);
                bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                SetDuration(sPlayerbotAIConfig.reactDelay);
                return true;
            }

            // move to jump point
            if (src != jumpPoint && src.distance(jumpPoint) > sPlayerbotAIConfig.contactDistance)
            {
                if (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
                {
                    std::string text = "Moving to jumping position!";
                    bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                }

                // see spell action will handle the movement
                if (isRtsc)
                {
                    WorldPosition jumpPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
                    if (!jumpPosition)
                    {
                        jumpPoint.orientation = requiredSpeed;
                        SET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point", jumpPoint);
                        return true;
                    }
                }

                return MoveTo(jumpPoint.getMapId(), jumpPoint.getX(), jumpPoint.getY(), jumpPoint.getZ());
            }
            else // jump from current position
            {
                if (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
                {
                    std::string text = "Jumping to you!";
                    bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                }

                float pointAngle = jumpPoint.getAngleTo(dest);
                sServerFacade.SetFacingTo(bot, pointAngle, true);
                bool success = JumpTowards(jumpPoint, dest, bot, requiredSpeed);

                if (isRtsc)
                {
                    RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
                    RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
                    ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);
                }
                return success;
            }
        }
        return false;
    }

    float angle = bot->GetOrientation();
    if (jumpBackward)
        angle += M_PI_F;

    float jumpSpeed;
    if (jumpInPlace)
        jumpSpeed = 0.f;
    else
    {
        jumpSpeed = jumpBackward ? bot->GetSpeed(MOVE_RUN_BACK) : bot->GetSpeed(MOVE_RUN);
    }

    float timeToLand, distToLand, maxHeight;
    bool goodLanding = true;
    WorldPosition jumpLanding = JumpAction::CalculateJumpParameters(WorldPosition(bot), bot, angle, 7.96f, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, sPlayerbotAIConfig.jumpHeightLimit);
    if (jumpLanding)
    {
        // not jump randomly in the water
        if ((options == "r" || options == "random") && !event.getOwner() && (jumpLanding.isInWater() || jumpLanding.isUnderWater()))
        {
            sLog.outDetail("%s: Jump random fail: landing in water!", bot->GetName());
            return false;
        }

        // only show landing
        if (showLanding || ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
        {
            Creature* wpCreature = bot->SummonCreature(2334, jumpLanding.getX(), jumpLanding.getY(), jumpLanding.getZ() - 1, bot->GetOrientation(), TEMPSPAWN_TIMED_DESPAWN, 3000);
            PlayerbotAI::AddAura(wpCreature, 246);
            if (showLanding)
            {
                WorldPosition botPos = WorldPosition(bot);
                float pointAngle = botPos.getAngleTo(jumpLanding);
                sServerFacade.SetFacingTo(bot, pointAngle, true);
                bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                SetDuration(sPlayerbotAIConfig.reactDelay);
                return true;
            }
        }
        return DoJump(jumpLanding, angle, 7.96f, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, jumpInPlace, jumpBackward, showLanding);
    }
    return false;
}

WorldPosition JumpAction::CalculateJumpParameters(const WorldPosition& src, Unit* jumper, float angle, float vSpeed, float hSpeed, float &timeToLand, float &distanceToLand, float &maxHeight, bool &goodLanding, float maxJumpHeight)
{
    if (!jumper)
        return WorldPosition();

    // static data
    double const jumpVerticalAngle = 48.6717 * M_PI / 180; // approximate
    float const m_gravity = 19.2911f;

    // jump in place
    if (hSpeed == 0.f)
    {
        float timeForMaxHeight = vSpeed / m_gravity;
        maxHeight = vSpeed * timeForMaxHeight - m_gravity * timeForMaxHeight * timeForMaxHeight / 2;
        timeToLand = timeForMaxHeight * 2;
        distanceToLand = 0.f;

        // calculate collision up
        float ox, oy, oz;
        ox = src.getX();
        oy = src.getY();
        oz = src.getZ() + 0.5f;
        float fx = ox;
        float fy = oy;
        float fz = oz + maxHeight + jumper->GetCollisionHeight();
#ifdef MANGOSBOT_TWO
        if (jumper->GetMap()->GetHitPosition(ox, oy, oz, fx, fy, fz, jumper->GetPhaseMask(), -0.5f))
#else
        if (jumper->GetMap()->GetHitPosition(ox, oy, oz, fx, fy, fz, -0.5f))
#endif
        {
            // hit object above
            goodLanding = false;
            timeToLand = JumpAction::CalculateJumpTime(fz - oz, vSpeed, true);
            maxHeight = fz;
            return WorldPosition(src.getMapId(), fx, fy, fz - CONTACT_DISTANCE - jumper->GetCollisionHeight());
        }
        else
        {
            // can jump full height and back
            goodLanding = true;
            maxHeight = fz;
            return src;
        }
    }

    float vsin = sin(angle);
    float vcos = cos(angle);

    // calculate max height
    float velocity = vSpeed / (float)sin(jumpVerticalAngle);
    float timeForMaxHeight = vSpeed / m_gravity;
    maxHeight = vSpeed * timeForMaxHeight - m_gravity * timeForMaxHeight * timeForMaxHeight / 2;

    // calculate approximate distance on ideal surface
    float rough_distance = 2 * timeForMaxHeight * hSpeed;

    // calculate collision
    float const path_part = rough_distance / 10.0f;
    float check_dist = path_part;
    float ox, oy, oz;
    ox = src.getX();
    oy = src.getY();
    oz = src.getZ() + 0.5f;
    bool foundCollision = false;
    for (auto i = 1; i <= 30; i++)
    {
        // not found - calculate very far
        if (i >= 25 && check_dist > (rough_distance + path_part))
        {
            sLog.outDetail("%s: Jump checks too many, possible no collision!", jumper->GetName());
            check_dist += rough_distance;
        }

        float fx = src.getX() + check_dist * vcos;
        float fy = src.getY() + check_dist * vsin;
        float fz = src.getZ() + 0.5f + float((check_dist * tan(jumpVerticalAngle)) - (m_gravity * check_dist * check_dist)/(2 * velocity * velocity * cos(jumpVerticalAngle) * cos(jumpVerticalAngle)));

        // add collision height while ascending
        bool ascending = fz > src.getZ() && check_dist < (rough_distance / 2);
        if (ascending)
            fz += jumper->GetCollisionHeight();

        // add some collision distances
        fx += jumper->GetCollisionWidth() * vcos;
        fy += jumper->GetCollisionWidth() * vsin;

#ifdef MANGOSBOT_TWO
        foundCollision = jumper->GetMap()->GetHitPosition(ox, oy, oz, fx, fy, fz, jumper->GetPhaseMask(), -0.5f);
#else
        foundCollision = jumper->GetMap()->GetHitPosition(ox, oy, oz, fx, fy, fz, -0.5f);
#endif

        if (!foundCollision)
        {
            fx -= jumper->GetCollisionWidth() * vcos;
            fy -= jumper->GetCollisionWidth() * vsin;
            if (ascending)
                fz -= jumper->GetCollisionHeight();
        }

        // vmaps collision not found - check maps (terrain)
        if (!foundCollision && !ascending)
        {
            float newGroundZ = fz;
            jumper->UpdateAllowedPositionZ(fx, fy, newGroundZ);
            // calculated point is lower than terrain - land on terrain
            if (fz < newGroundZ)
            {
                foundCollision = true;
                fz = newGroundZ; // not correct, but should not be too far
            }
        }

        if (maxJumpHeight > 0.f && fabs(src.getZ() - fz) > maxJumpHeight)
            return WorldPosition();

        // can land and not lava/slime
        if (!IsJumpSafe(src, WorldPosition(src.getMapId(), fx, fy ,fz), jumper))
            return WorldPosition();

        // vmaps collision found
        if (foundCollision)
        {
            // hit something while ascending
            if (ascending)
            {
                goodLanding = false;
                // reduce landing height by collision height
                fz = fz - CONTACT_DISTANCE - jumper->GetCollisionHeight();
            }

            WorldPosition destination = WorldPosition(src.getMapId(), fx, fy ,fz);
            distanceToLand = sqrtf(src.sqDistance2d(destination));
            timeToLand = CalculateJumpTime(fz - (src.getZ() + 0.5f), vSpeed, ascending);

            // some error in time calculations - cancel the jump
            if (timeToLand == 0.f)
                return WorldPosition();

            // maybe hit a wall while descending
            if (goodLanding)
            {
                float groundZ = destination.getZ() + 0.5f;
                jumper->UpdateAllowedPositionZ(destination.getX(), destination.getY(), groundZ);
                // set to fall after land if not at the ground
                if (groundZ < destination.getZ() && fabs(oz - destination.getZ()) > 5.0f)
                {
                    goodLanding = false;
                }
            }

            maxHeight = fz;
            return destination;
        }

        ox = fx;
        oy = fy;
        oz = fz/* + 0.5f*/;

        check_dist += path_part;
    }

    sLog.outDetail("%s: Jump collision fail to calculate!", jumper->GetName());
    timeToLand = 0.f;
    distanceToLand = 0.f;
    return WorldPosition();
}

float JumpAction::CalculateJumpTime(float srcZ, float destZ, float vSpeed, float hSpeed, float distance)
{
    double jumpVerticalAngle = 48.6717 * M_PI / 180;
    float m_gravity = 19.2911f;
    float timeForMaxHeight = vSpeed / m_gravity;
    float rough_distance = 2 * timeForMaxHeight * hSpeed;
    bool ascending = destZ > srcZ && distance < (rough_distance / 2);
    float jumpTime = 0.f;
    float sqroot = vSpeed * vSpeed - (m_gravity * 2 * (destZ - srcZ));
    // some collision error allowing jump above max height
    if (sqroot < 0.f)
    {
        sLog.outDetail("Jump above max height! srcZ: %f, destZ: %f, distance: %f", srcZ, destZ, distance);
        return 0.f;
    }

    if (ascending)
    {
        jumpTime = (vSpeed - sqrtf(sqroot)) / m_gravity;
    }
    else
        jumpTime = (vSpeed + sqrtf(sqroot)) / m_gravity;

    return jumpTime;
}

float JumpAction::CalculateJumpTime(float z_diff, float vSpeed, bool ascending)
{
    float m_gravity = 19.2911f;
    float jumpTime = 0.f;
    float sqroot = vSpeed * vSpeed - (m_gravity * 2 * (z_diff));
    // some collision error allowing jump above max height
    if (sqroot < 0.f)
    {
        sLog.outDetail("Jump above max height!");
        return 0.f;
    }

    if (ascending)
    {
        jumpTime = (vSpeed - sqrtf(sqroot)) / m_gravity;
    }
    else
        jumpTime = (vSpeed + sqrtf(sqroot)) / m_gravity;

    return jumpTime;
}

bool JumpAction::IsJumpSafe(const WorldPosition &src, const WorldPosition &dest, Unit* jumper)
{
    return CanLand(dest, jumper) && IsNotMagmaSlime(dest, jumper);
}

bool JumpAction::CanWalkTo(const WorldPosition &src, const WorldPosition &dest, Unit* jumper, float maxDistance)
{
    if (!src || !dest)
        return false;

    if (src.getMapId() != dest.getMapId())
        return false;

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return false;

    vector<WorldPosition> path = dest.getPathStepFrom(src, jumper, true);
    if (path.empty())
    {
        sLog.outDetail("%s: Jump CanWalkTo Fail! No Path!", jumper->GetName());
        return false;
    }

    float pathLength = src.getPathLength(path);
    // todo add config
    if (pathLength > maxDistance)
    {
        sLog.outDetail("%s: Jump CanWalkTo Fail! Path is too big! Max Distance: %f, Path Distance %f", jumper->GetName(), maxDistance, pathLength);
        return false;
    }

    return true;
}

bool JumpAction::CanLand(const ai::WorldPosition &dest, Unit *jumper)
{
    // do not let jump to abyss
#ifdef MANGOSBOT_TWO
    float mapHeightCheck = jumper->GetMap()->GetHeight(jumper->GetPhaseMask(), dest.getX(), dest.getY(), dest.getZ() + 0.5f);
#else
    float mapHeightCheck = jumper->GetMap()->GetHeight(dest.getX(), dest.getY(), dest.getZ() + 0.5f);
#endif
    if (mapHeightCheck <= INVALID_HEIGHT)
    {
        sLog.outDetail("%s: Jump Fail! Invalid landing height: %f", jumper->GetName(), mapHeightCheck);
        return false;
    }
    return true;
}

bool JumpAction::IsNotMagmaSlime(const WorldPosition &dest, Unit *jumper)
{
    if (const TerrainInfo* terrain = dest.getTerrain())
    {
        if (!terrain->CanCheckLiquidLevel(dest.getX(), dest.getY()))
            return true;

        GridMapLiquidData data;
        if (terrain->getLiquidStatus(dest.getX(), dest.getY(), dest.getZ(), MAP_ALL_LIQUIDS, &data) == LIQUID_MAP_NO_WATER)
            return true;

        switch (data.type_flags)
        {
            case MAP_LIQUID_TYPE_MAGMA:
            case MAP_LIQUID_TYPE_SLIME:
            {
                sLog.outDetail("%s: Jump Fail! Landing is Magma or Slime!", jumper->GetName());
                return false;
            }
        }
    }

    return true;
}

bool JumpAction::CanJumpTo(const WorldPosition& src, const WorldPosition& dest, Unit* jumper, float jumpSpeed, float maxDistace)
{
    if (!src || !dest)
        return false;

    if (src.getMapId() != dest.getMapId())
        return false;

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return false;

    // some preparation
    static float m_gravity = 19.2911f;
    float timeForMaxHeight = 7.96f / m_gravity;
    float maxHeight = 7.96f * timeForMaxHeight - m_gravity * timeForMaxHeight * timeForMaxHeight / 2;

    // can't jump too high
    if ((src.getZ() + maxHeight) < dest.getZ())
        return false;

    float angle = src.getAngleTo(dest);

    float timeToLand, distToLand;
    bool goodLanding = true;
    WorldPosition jumpLanding = JumpAction::CalculateJumpParameters(src, jumper, angle, 7.96f, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, sPlayerbotAIConfig.jumpHeightLimit);
    if (jumpLanding)
    {
        return CanWalkTo(jumpLanding, dest, jumper, maxDistace);
    }

    return false;
}

bool JumpAction::JumpTowards(const ai::WorldPosition &src, const ai::WorldPosition &dest, Unit* jumper, float jumpSpeed)
{
    if (src.getMapId() != dest.getMapId())
        return false;

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return false;

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return false;

    bool jumpInPlace = false;
    bool jumpBackward = false;

    float angle = src.getAngleTo(dest);

    float timeToLand, distToLand, maxHeight;
    bool goodLanding = true;
    WorldPosition jumpLanding = JumpAction::CalculateJumpParameters(src, jumper, angle, 7.96f, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, sPlayerbotAIConfig.jumpHeightLimit);
    sLog.outDetail("%s: JumpTowards attempt! Jump speed: %f", bot->GetName(), jumpSpeed);
    if (jumpLanding && goodLanding)
    {
        return DoJump(jumpLanding, angle, 7.96f, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, jumpInPlace, jumpBackward, false);
    }

    sLog.outDetail("%s: Jump ForwardTo Fail!", jumper->GetName());
    return false;
}

bool JumpAction::DoJump(WorldPosition &dest, float angle, float vSpeed, float hSpeed, float timeToLand, float distanceToLand, float maxHeight, bool goodLanding, bool jumpInPlace, bool jumpBackward, bool showOnly)
{
    if (!dest)
        return false;

    // fix height
    if (goodLanding)
    {
        float ox = dest.getX();
        float oy = dest.getY();
        float oz = dest.getZ() + 0.5f;
        bot->UpdateAllowedPositionZ(ox, oy, oz);
        dest = WorldPosition(dest.getMapId(), ox, oy, oz);
    }

    if (!goodLanding)
        ai->SetFallAfterJump();

    ai->InterruptSpell(false);
    ai->StopMoving();
    ai->SetJumpDestination(dest);
    bot->SetFallInformation(0, maxHeight);

    // send move packet before jump
    if (!jumpInPlace)
    {
        bot->m_movementInfo.AddMovementFlag(jumpBackward ? MOVEFLAG_BACKWARD : MOVEFLAG_FORWARD);
        WorldPacket move(jumpBackward ? MSG_MOVE_START_BACKWARD : MSG_MOVE_START_FORWARD);
// write packet info
#ifdef MANGOSBOT_TWO
        move << bot->GetObjectGuid().WriteAsPacked();
#endif
        move << bot->m_movementInfo;
        bot->GetSession()->HandleMovementOpcodes(move);
    }

    float vsin = jumpInPlace ? 0 : sin(angle);
    float vcos = jumpInPlace ? 1 : cos(angle);

    // write jump info
    uint32 curTime = sWorld.GetCurrentMSTime();
    uint32 jumpTime = curTime + sWorld.GetAverageDiff() + uint32(timeToLand * IN_MILLISECONDS) + 1000;
    ai->SetJumpTime(jumpTime);
    bot->m_movementInfo.jump.zspeed = -vSpeed;
    bot->m_movementInfo.jump.cosAngle = vcos;
    bot->m_movementInfo.jump.sinAngle = vsin;
    bot->m_movementInfo.jump.xyspeed = hSpeed;

    sLog.outDetail("%s: Jump x: %f, y: %f, z: %f, time: %f, dist: %f, inPlace: %u, landTime: %u, curTime: %u", bot->GetName(), dest.getX(), dest.getY(), dest.getZ(), timeToLand, distanceToLand, jumpInPlace, jumpTime, curTime);

    // send jump packet
#ifdef MANGOSBOT_ZERO
    bot->m_movementInfo.AddMovementFlag(MOVEFLAG_JUMPING);
#else
    bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FALLING);
#endif

    WorldPacket jump(MSG_MOVE_JUMP);
    // write packet info
#ifdef MANGOSBOT_TWO
    jump << bot->GetObjectGuid().WriteAsPacked();
#endif
    jump << bot->m_movementInfo;
    bot->GetSession()->HandleMovementOpcodes(jump);

    // change position to land position
    // todo add in between points to avoid mobs instant aggro before landing
    bot->Relocate(dest.getX(), dest.getY(), dest.getZ());

    if (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
    {
        std::string text = "Jump: cos: " + std::to_string(vcos) + " sin: " + std::to_string(vsin) + " distance: " + std::to_string(distanceToLand);
        bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    }

    return true;
}

WorldPosition JumpAction::GetPossibleJumpStartFor(const WorldPosition& src, const WorldPosition& dest, Unit* jumper, float &requiredSpeed, float distanceTo, float distanceFrom)
{
    if (!src || !dest)
        return WorldPosition();

    if (src.getMapId() != dest.getMapId())
        return WorldPosition();

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return WorldPosition();

    // try jump from where at
    if (CanJumpTo(src, dest, jumper, 2.5f, distanceFrom))
    {
        requiredSpeed = 2.5f;
        return src;
    }
    else
    {
        // try slow jump
        if (CanJumpTo(src, dest, jumper, 7.0f, distanceFrom))
        {
            requiredSpeed = 7.0f;
            return src;
        }
    }

    // try find a closer point
    vector<WorldPosition> path = dest.getPathStepFrom(src, jumper);

    // no path found closer to it...
    if (path.empty() || path.size() == 2)
    {
        sLog.outDetail("%s: Jump Fail! Can't pathfind closer!", jumper->GetName());
        return WorldPosition();
    }

    float pathLength = src.getPathLength(path);
    for (auto& p : path)
    {
        if (p.fDist(src) > distanceTo)
            break;

        // try slow jump
        if (CanJumpTo(p, dest, jumper, 2.5f, distanceFrom))
        {
            requiredSpeed = 2.5f;
            return p;
        }
        else
        {
            // try fast jump
            if (CanJumpTo(p, dest, jumper, 7.0f, distanceFrom))
            {
                requiredSpeed = 7.0f;
                return p;
            }
            else
                continue;
        }
    }

    sLog.outDetail("%s: GetPossibleJumpStartFor Failed to find jump point!", jumper->GetName());
    return WorldPosition();
}
