
#include "playerbot/playerbot.h"
#include "MovementActions.h"
#include "MotionGenerators/MotionMaster.h"
#include "MotionGenerators/MovementGenerator.h"
#include "playerbot/FleeManager.h"
#include "playerbot/LootObjectStack.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/strategy/values/Stances.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "playerbot/TravelMgr.h"
#include "Entities/Transports.h"
#ifdef MANGOSBOT_TWO
#include "Entities/Vehicle.h"
#endif
#include "playerbot/strategy/generic/CombatStrategy.h"

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
        target->GetNearPoint(bot, x, y, z, bot->GetObjectBoundingRadius(), std::min(dist, ai->GetRange("follow")), angle);
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

bool MovementAction::FlyDirect(const WorldPosition &startPosition, const WorldPosition &endPosition, WorldPosition& movePosition, TravelPath movePath)
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
        std::vector<WorldPosition> path;
        if (movePath.empty()) //Make a path starting at the end backwards to see if we can walk to some better place.
        {
            path = endPosition.getPathTo(startPosition, bot);            
        }
        else
        {
            std::reverse(path.begin(), path.end());
            path = movePath.getPointPath();
        }

        if (path.empty())
            return false;



        for (auto& p : path) //Find the furtest point where we can fly to directly.
            if (p.getMapId() == startPosition.getMapId() && p.isOutside() && p.canFly())
            {
                movePosition = p;
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

    AI_VALUE(LastMovement&, "last movement").lastAreaTrigger = movePosition;

    return true;
#endif
}

bool MovementAction::UseTaxi(PlayerbotAI* ai, uint32 entry, bool needNpc)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    Player* bot = ai->GetBot();

    TaxiPathEntry const* tEntry = sTaxiPathStore.LookupEntry(entry);

    if (!tEntry)
    {
#ifdef MANGOSBOT_TWO
        bot->OnTaxiFlightEject(true);
        ai->Unmount();
#endif
        bool goClick = ai->HandleSpellClick(entry); //Source gryphon of ebonhold.
        return goClick;
    }

    Creature* unit = nullptr;

    if (needNpc)
    {
        std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
        for (std::list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
        {
            unit = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_FLIGHTMASTER);
            if (unit)
                break;
        }

        if (!unit)
        {
            return false;
        }

        if (unit && !bot->m_taxi.IsTaximaskNodeKnown(tEntry->from))
        {
            bot->GetSession()->SendLearnNewTaxiNode(unit);

            unit->SetFacingTo(unit->GetAngle(bot));
        }
    }

    uint32 botMoney = bot->GetMoney();
    if (ai->HasCheat(BotCheatMask::gold) || ai->HasCheat(BotCheatMask::taxi))
    {
        bot->SetMoney(botMoney + tEntry->price);
    }

    bot->OnTaxiFlightEject(true);

    ai->Unmount();

    bool goTaxi = bot->ActivateTaxiPathTo({tEntry->from, tEntry->to}, unit, 1);

    if (!goTaxi)
        bot->SetMoney(botMoney);

    return goTaxi;
}

bool MovementAction::MoveOnTransport(PlayerbotAI* ai, GenericTransport* transport, bool doTeleport)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    Player* bot = ai->GetBot();
    WorldPosition botPos(bot);

    uint32 radius = 20;

    GenericTransport* botTrans = bot->GetTransport();

    std::vector<WorldPosition> path;

    WorldPosition transPos = botPos.RandomPointOnTrans(transport, 20.0f, doTeleport ? nullptr : bot, path);

    if (!transPos)
        return false;

    if (doTeleport)
    {
        bot->GetMap()->PlayerRelocation(bot, transPos.getX(), transPos.getY(), transPos.getZ(), bot->GetOrientation());
        transport->AddPassenger(bot, true);
        bot->SendHeartBeat();
        return true;
    }

    bot->SetTransport(botTrans);

    if (path.empty())
    {
        path = WorldPosition(transport).getPathStepFrom(botPos, bot);

        if (path.empty())
            return false;
    }
    else
    {
        transport->AddPassenger(bot, true);

        ai->StopMoving();

        if (!bot->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = bot->GetMotionMaster()->top())
                movgen->Interrupt(*bot);

        bot->SendHeartBeat();

        if (!bot->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = bot->GetMotionMaster()->top())
                movgen->Reset(*bot);
    }

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        for (auto& p : path)
        {
            Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            transport->AddPassenger(wpCreature, true);

            wpCreature->NearTeleportTo(p.getX(), p.getY(), p.getZ(), wpCreature->GetOrientation());

            ai->AddAura(wpCreature, 246);

            if (p == path.back())
                ai->AddAura(wpCreature, 1130);
        }
    }

    bot->GetMotionMaster()->Clear();

    std::vector<G3D::Vector3> pointPath = transPos.toPointsArray(path);
#ifndef MANGOSBOT_TWO
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false, false);
#else
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false);
#endif

    return true;
}

bool MovementAction::MoveOffTransport(PlayerbotAI* ai, WorldPosition exitPos, bool doTeleport)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    Player* bot = ai->GetBot();
    WorldPosition botPos(bot);

    if (!bot->GetTransport())
    {
        return false;
    }

    GenericTransport* transport = bot->GetTransport();

    transport->RemovePassenger(bot);

    if (doTeleport)
    {
        bot->TeleportTo(exitPos.getMapId(), exitPos.getX(), exitPos.getY(), exitPos.getZ(), exitPos.getO(), 0);
        return true;
    }

    bot->NearTeleportTo(bot->m_movementInfo.pos.x, bot->m_movementInfo.pos.y, bot->m_movementInfo.pos.z, bot->m_movementInfo.pos.o);

    std::vector<WorldPosition> path = WorldPosition(bot).getPathStepFrom(exitPos, bot, false);

    if (path.empty())
    {
        return false;
    }

    if (exitPos.sqDistance(path.back()) > 5.0f)
    {
        return false;
    }

    bot->GetMotionMaster()->Clear();

    std::vector<G3D::Vector3> pointPath = exitPos.toPointsArray(path);
#ifndef MANGOSBOT_TWO
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false, false);
#else
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false);
#endif

    return true;
}

bool MovementAction::UseTransport(PlayerbotAI* ai, uint32 entry, WorldPosition dockPosition, WorldPosition exitPosition, bool doTeleport)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    Player* bot = ai->GetBot();
    WorldPosition botPos(bot);

    GenericTransport* transport = bot->GetTransport();

    if (transport)
    {
        GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(transport->GetEntry());
        std::string transportName = transport->GetName();
        if (transportName.empty())
            transportName = data->name;

        if (dockPosition.mapid == bot->GetMapId() && dockPosition.sqDistance2d(transport) < INTERACTION_DISTANCE * INTERACTION_DISTANCE)
        {
            MoveOffTransport(ai, exitPosition, doTeleport);
            ai->TellDebug(ai->GetMaster(), "Leaving transport " + transportName, "debug move");
            return true;
        }

        if (urand(0, 50))
            MoveOnTransport(ai, transport, doTeleport);

        ai->TellDebug(ai->GetMaster(), "Waiting ontop of transport " + transportName + " at " + std::to_string((uint32)dockPosition.fDist(transport)) + "y from docking.", "debug move");

        return false;
    }

    float minDist = 0;

    std::string transportName;

    for (auto& trans : dockPosition.getTransports(entry))
    {
        float distance = dockPosition.sqDistance2d(trans);

        if (minDist && distance > minDist)
            continue;

        transport = trans;
        minDist = distance;

        GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(transport->GetEntry());
        transportName = transport->GetName();
        if (transportName.empty())
            transportName = data->name;
    }

    if (transport && dockPosition.mapid == bot->GetMapId() && dockPosition.sqDistance2d(transport) < INTERACTION_DISTANCE * INTERACTION_DISTANCE)
    {
        MoveOnTransport(ai, transport, doTeleport);

        return true;
    }

    if (transportName.empty())
        ai->TellDebug(ai->GetMaster(), "Waiting for transport on different map.", "debug move");
    else
        ai->TellDebug(ai->GetMaster(), "Waiting for transport " + std::string(transportName) + " at " + std::to_string((uint32)sqrt(minDist)) + "y from docking.", "debug move");

    return false;
}

bool MovementAction::MinimalMove(PlayerbotAI* ai)
{
    if (!sPlayerbotAIConfig.enableMinimalMove)
        return false;

    auto pmo1 = sPerformanceMonitor.start(PERF_MON_ACTION, "minimalMove", ai);

    AiObjectContext* context = ai->GetAiObjectContext();
    Player* bot = ai->GetBot();
    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");

    if (bot->IsTaxiFlying())
        return false;

    if (lastMove.lastPath.empty())
        return false;

    time_t now = time(0);

    if (lastMove.nextTeleport > now)
        return false;

    lastMove.nextTeleport = now + sPlayerbotAIConfig.passiveDelay/1000; //For teleports/transports/ect 

    std::vector<PathNodePoint>& path = lastMove.lastPath.getPath();

    auto nextStep = path.begin();

    bool doDelay = true;

    //Taxi handling: Start taxi and remove path until it ends.
    if (nextStep->type == PathNodeType::NODE_FLIGHTPATH)
    {
        if (nextStep->point.sqDistance(bot) > INTERACTION_DISTANCE * INTERACTION_DISTANCE)
        {
            bot->TeleportTo(nextStep->point);

            return true;
        }

        bool didTaxi = UseTaxi(ai, nextStep->entry, false);

        for (auto& step : path)
        {
            if (step.type == PathNodeType::NODE_FLIGHTPATH && step.entry == nextStep->entry)
                continue;

            lastMove.lastPath.cutTo(step, false); //Remove path until next walk or taxi.
            break;
        }

        return true;
    }

   //Transport handling: If not on transport wait for transport and teleport on it when it's near (and cut to last transport point). If on transport wait until it is near exit and teleport to exit.
    if (nextStep->type == PathNodeType::NODE_TRANSPORT && nextStep != path.end())
    {
        auto exitStep = std::next(nextStep);

        WorldPosition exitPos = (exitStep != path.end()) ? exitStep->point : nextStep->point;

        bool didTransport = UseTransport(ai, nextStep->entry, nextStep->point, exitPos, true);

        if (!didTransport) //We did not board yet or are on the transport so just wait.
        {
            return true;
        }

        if (bot->GetTransport()) //Just boarded
        {
            PathNodePoint lastStep = *nextStep;

            for (auto& step : path)
            {
                if (step.type == PathNodeType::NODE_TRANSPORT && step.entry == nextStep->entry)
                {
                    lastStep = step;
                    continue;
                }

                break;
            }

            lastMove.lastPath.cutTo(lastStep, false); //Remove path up to last transport point.

            return true;
        }

        //Ready to exit
        lastMove.lastPath.cutTo(*nextStep, true); //Removing boarding point.

        nextStep = path.begin();

        bot->TeleportTo(nextStep->point);

        return true;
    }

    //Skip over stuff we don't walk.
    if (!nextStep->isWalkable())
    {
        auto it = std::find_if(std::next(nextStep), path.end(), [](const auto& step) {
            return step.isWalkable();
        });

        if (it != path.end())
        {
            nextStep = it;
            doDelay = true;
        }
    }

    if (!nextStep->isWalkable())
        return false;

    if (ai->HasPlayerNearby(nextStep->point, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL)))
        return true;

    bot->TeleportTo(nextStep->point);

    if (std::next(nextStep) == path.end())
    {
        lastMove.lastPath.clear();
        return true;
    }

    uint32 time = 0;

    for (auto it = std::next(nextStep); it != path.end(); ++it)
    {
        time += (nextStep->point.distance(bot) / bot->GetSpeedInMotion()) * 1000;

        nextStep = it;

        if (!it->isWalkable() || time > sPlayerbotAIConfig.passiveDelay)
            break;
    }

    lastMove.nextTeleport = now + (time / 1000);

    lastMove.lastPath.cutTo(*nextStep, false);

    return true;
}

bool MovementAction::WaitForTransport()
{
    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");

    // Check if we need to resume transport journey
    if (!lastMove.lastTransportEntry)
        return false;

    GenericTransport* transport = bot->GetTransport();

    if (!transport || transport->GetEntry() != lastMove.lastTransportEntry || lastMove.lastPath.getPath().front().type != PathNodeType::NODE_TRANSPORT || lastMove.lastPath.getPath().front().entry != lastMove.lastTransportEntry)
    {
        lastMove.lastTransportEntry = 0;
        return false;
    }

    TravelPath path = lastMove.lastPath;

    if(!path.UpcommingSpecialMovement(bot, 0.0f, bot->GetTransport()))
        return false;

    PathNodePoint dockPoint = path.getPath().front();
    PathNodePoint telePoint = *std::next(path.getPath().begin());
        
    if (!UseTransport(ai, dockPoint.entry, dockPoint.point, telePoint.point, sPlayerbotAIConfig.transportTeleportType > 0))
        return true;

    lastMove.lastTransportEntry = 0;
    return false;
}

TravelPath MovementAction::ResolveMovePath(const WorldPosition& startPosition, const WorldPosition& endPosition, Unit* mover, LastMovement& lastMove)    
{
    float totalDistance = startPosition.distance(endPosition);
    float maxDistChange = totalDistance * 0.1f;

    // Last long path still leads to roughly the same destination.
    if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPosition) < maxDistChange)
    {
        return lastMove.lastPath;
    }

    bool needsLongPath = false;
        
    if (startPosition.getMapId() != endPosition.getMapId())
        needsLongPath = true;
    else if (totalDistance > sPlayerbotAIConfig.sightDistance)
        needsLongPath = true;
#ifdef MANGOSBOT_TWO
    else if (startPosition.getMapId() == 609 && fabs(startPosition.getZ() - endPosition.getZ()) > 20.0f)
        needsLongPath = true;
#endif

    TravelPath outMovePath;

    if (needsLongPath && !sTravelNodeMap.getNodes().empty() && !bot->InBattleGround())
    {
        outMovePath = sTravelNodeMap.getFullPath(startPosition, endPosition, bot); //Pathfind using nodes.
    }
    else
    {
        std::vector<WorldPosition> path = startPosition.getPathTo(endPosition, bot); //Navemesh pathfinding only.

        outMovePath.addPath(path);
    }

    if (!lastMove.lastPath.empty() && !outMovePath.empty() && lastMove.lastPath.getBack().distance(endPosition) <= outMovePath.getBack().distance(endPosition))
        outMovePath = lastMove.lastPath;

    if (outMovePath.empty())
        outMovePath.addPoint(endPosition);

    return outMovePath;
}

bool MovementAction::HandleSpecialMovement(TravelPath& path)
{
    PathNodePoint currentPoint = path.getPath().front();
    PathNodePoint nextPoint;
    if (path.getPath().size() > 1)
        nextPoint = *std::next(path.getPath().begin());

    //Game object portals
    if (currentPoint.type == PathNodeType::NODE_STATIC_PORTAL && currentPoint.entry)
    {
        GameObjectInfo const* goInfo = sGOStorage.LookupEntry<GameObjectInfo>(currentPoint.entry);
        if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER)
            return false;

        uint32 spellId = goInfo->spellcaster.spellId;
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

        if (pSpellInfo->EffectTriggerSpell[0])
            pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

        bool hasTeleportEffect = pSpellInfo->Effect[0] == SPELL_EFFECT_TELEPORT_UNITS || pSpellInfo->Effect[1] == SPELL_EFFECT_TELEPORT_UNITS || pSpellInfo->Effect[2] == SPELL_EFFECT_TELEPORT_UNITS;
        if (!hasTeleportEffect)
            return false;

        if (bot->IsMounted())
        {
            if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
                return false;
            ai->Unmount();
        }

        std::list<ObjectGuid> gos = *context->GetValue<std::list<ObjectGuid>>("nearest game objects");
        for (auto i = gos.begin(); i != gos.end(); ++i)
        {
            GameObject* go = ai->GetGameObject(*i);
            if (!go || go->GetEntry() != currentPoint.entry)
                continue;

            if (!bot->GetGameObjectIfCanInteractWith(go->GetObjectGuid(), MAX_GAMEOBJECT_TYPE))
                continue;

            std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
            *packet << *i;
            bot->GetSession()->QueuePacket(std::move(packet));
            return true;
        }

        return false;
    }

    if (currentPoint.type == PathNodeType::NODE_AREA_TRIGGER)
    {
        if (currentPoint.entry)
            AI_VALUE(LastMovement&, "last area trigger").lastAreaTrigger = currentPoint.entry;
        else
            return bot->TeleportTo(nextPoint.point.getMapId(), nextPoint.point.getX(), nextPoint.point.getY(), nextPoint.point.getZ(), nextPoint.point.getO(), 0) ? true : false;
    }

    //We are getting 'on' transport.
    if (nextPoint.type == PathNodeType::NODE_TRANSPORT)
    {
        bool usedTransport = UseTransport(ai, nextPoint.entry, nextPoint.point, WorldPosition(), sPlayerbotAIConfig.transportTeleportType > 0);

        uint32 lastTransportEntry = 0;

        if (usedTransport)
            AI_VALUE(LastMovement&, "last movement").lastTransportEntry = nextPoint.entry;

        WaitForReach(1000.0f);
        return true;
    }

    if (currentPoint.type == PathNodeType::NODE_TRANSPORT)
    {
        bool usedTransport = UseTransport(ai, currentPoint.entry, currentPoint.point, nextPoint.point, sPlayerbotAIConfig.transportTeleportType > 0);

        uint32 lastTransportEntry = 0;

        if (!usedTransport)
        {
            if (bot->GetTransport())
                lastTransportEntry = nextPoint.entry;
        }
        else
        {
            if (!bot->GetTransport())
                return bot->TeleportTo(nextPoint.point.getMapId(), nextPoint.point.getX(), nextPoint.point.getY(), nextPoint.point.getZ(), nextPoint.point.getO(), 0) ? true : false;

            lastTransportEntry = nextPoint.entry;
        }

        if (lastTransportEntry)
            AI_VALUE(LastMovement&, "last movement").lastTransportEntry = lastTransportEntry;

        WaitForReach(1000.0f);
        return true;
    }

    if (nextPoint.type == PathNodeType::NODE_FLIGHTPATH && nextPoint.entry)
        return UseTaxi(ai, nextPoint.entry, true) ? true : false;

    if (nextPoint.type == PathNodeType::NODE_TELEPORT && nextPoint.entry)
    {
        bool canCastNow = !bot->IsFlying() || WorldPosition(bot).currentHeight() < 10.0f;

        if (nextPoint.entry == 8690) // Hearthstone
        {
            if (AI_VALUE2(bool, "action useful", "hearthstone") && canCastNow)
                return ai->DoSpecificAction("hearthstone", Event("move action"), true) ? true : false;
        }
        else if (sServerFacade.IsSpellReady(bot, nextPoint.entry) && canCastNow && AI_VALUE2(uint32, "has reagents for", nextPoint.entry) > 0)
        {
            if (AI_VALUE2(uint32, "current mount speed", "self target"))
            {
                ai->Unmount();
#ifdef MANGOSBOT_TWO
                return false;
#endif
            }

            ai->RemoveShapeshift();

            if (ai->DoSpecificAction("cast", Event("rpg action", chat->formatWorldobject(bot) + " " + std::to_string(nextPoint.entry)), true))
                return true;
        }

        AI_VALUE(LastMovement&, "last movement").setPath(TravelPath());
        return false;
    }

    return false;
}

void MovementAction::UpdateFlyingState(
    WorldPosition& movePosition,
    float totalDistance,
    float originalZ,
    float maxDist,
    bool isWalking)
{
#ifndef MANGOSBOT_ZERO
    bool isFly = bot->IsFlying();
    bool isFar = false;
    bool needFly = false;
    bool needLand = false;

    // Ascend when the next walking waypoint is far and in clear LOS overhead.
    if (totalDistance > maxDist && isWalking)
    {
        isFar = true;
        needFly = true;

        Position pos = bot->GetPosition();
#ifdef MANGOSBOT_TWO
        if (!bot->GetMap()->IsInLineOfSight(pos.x, pos.y, pos.z + 100.f, movePosition.getX(), movePosition.getY(), movePosition.getZ() + 100.f, bot->GetPhaseMask(), true))
#else
        if (!bot->GetMap()->IsInLineOfSight(pos.x, pos.y, pos.z + 100.f, movePosition.getX(), movePosition.getY(), movePosition.getZ() + 100.f, true))
#endif
            needFly = false;

        if (needFly)
        {
            if (const TerrainInfo* terrain = bot->GetTerrain())
            {
                float destHeight = terrain->GetHeightStatic(movePosition.getX(), movePosition.getY(), movePosition.getZ());
                float destGround = terrain->GetWaterOrGroundLevel(movePosition.getX(), movePosition.getY(), movePosition.getZ(), destHeight);

                float botHeight = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                float botGround = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), botHeight);

                // Fly up if the ground isn't blocking the path (skip tunnels).
                if (totalDistance > maxDist && destGround <= movePosition.getZ())
                    movePosition.setZ(std::min(std::max(destGround, botGround) + 100.0f,
                        std::max(movePosition.getZ() + 10.0f, bot->GetPositionZ() + 10.0f)));
                else
                    movePosition.setZ(std::max(std::max(destGround, botGround), bot->GetPositionZ() - 10.0f));
            }
        }
    }

    // Ascend toward a raised destination even when not far (e.g. ledge above).
    if (!isFar && !isFly && originalZ > bot->GetPositionZ() && (originalZ - bot->GetPositionZ()) > 5.0f)
        needFly = true;

    // Descend when close to the ground-level destination.
    if (!isFar && isFly)
    {
        if (const TerrainInfo* terrain = bot->GetTerrain())
        {
            float height = terrain->GetHeightStatic(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            float ground = terrain->GetWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), height);
            if (bot->GetPositionZ() > originalZ && (bot->GetPositionZ() - originalZ < 5.0f) && (fabs(originalZ - ground) < 5.0f))
                needLand = true;
        }
    }

    // Apply FLYING flags.
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

    // Remove FLYING flags on landing.
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
#endif // !MANGOSBOT_ZERO
}

void MovementAction::DispatchMovement(TravelPath movePath, bool generatePath, bool masterWalking)
{
    MotionMaster& mm = *bot->GetMotionMaster();

    mm.Clear();

    ForcedMovement moveMode = masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN;
#ifndef MANGOSBOT_ZERO
    if (bot->IsFlying())
        moveMode = FORCED_MOVEMENT_FLIGHT;
#endif

    std::vector<WorldPosition> path = movePath.getPointPath();

    if (!generatePath || !bot->IsFreeFlying())
    {
        WorldPosition movePosition = path.back();

#ifdef MANGOSBOT_ZERO
        mm.MovePoint(movePosition.getMapId(),
            movePosition.getX(),
            movePosition.getY(),
            movePosition.getZ(),
            moveMode,
            generatePath);
#else
        mm.MovePoint(movePosition.getMapId(),
            Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f),
            moveMode,
            bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.f,
            bot->IsFlying());
#endif
    }

    GeneratePathAvoidingHazards(path);

    std::vector<G3D::Vector3> pointPath = WorldPosition().toPointsArray(path);
    float size = WorldPosition().getPathLength(path);

    /*
#ifndef MANGOSBOT_TWO
    mm.MovePath(pointPath, moveMode, false, false);
#else
    mm.MovePath(pointPath, moveMode, false);
#endif
    */

    WorldPosition movePosition = path.back();

#ifdef MANGOSBOT_ZERO
    mm.MovePoint(movePosition.getMapId(),
        movePosition.getX(),
        movePosition.getY(),
        movePosition.getZ(),
        moveMode,
        generatePath);
#else
    mm.MovePoint(movePosition.getMapId(),
        Position(movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0.f),
        moveMode,
        bot->IsFlying() ? bot->GetSpeed(MOVE_FLIGHT) : 0.f,
        bot->IsFlying());
#endif

    WaitForReach(size);
}


Unit* MovementAction::GetMover(Player* bot)
{
#ifdef MANGOSBOT_TWO
    if (TransportInfo* transportInfo = bot->GetTransportInfo())
    {
        if (transportInfo->IsOnVehicle())
        {
            Unit* vehicle = (Unit*)transportInfo->GetTransport();
            if (vehicle && vehicle->GetVehicleInfo())
            {
                VehicleSeatEntry const* seat = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat());
                if (!seat || !seat->HasFlag(SEAT_FLAG_CAN_CONTROL))
                    return bot;
            }
            return vehicle;
        }
    }
#endif
    return bot;
}

bool MovementAction::MoveTo2(const WorldPosition& endPos, bool idle, bool react, bool noPath, bool ignoreEnemyTargets)
{
    if (!endPos.isValid())
        return false;

    UpdateMovementState();

    if (!ai->CanMove())
        return false;

    Unit* mover = GetMover(bot);

    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");

    bool detailedMove = ai->AllowActivity(DETAILED_MOVE_ACTIVITY, true);
    if (!detailedMove && lastMove.nextTeleport)
    {
        time_t now = time(0);
        if (lastMove.nextTeleport > now)
        {
            SetDuration((lastMove.nextTeleport - now) * 1000);
            return true;
        }
    }
    else
        lastMove.nextTeleport = 0;

    if (WaitForTransport())
        return true;

    WorldPosition startPos(bot);
    float totalDistance = startPos.distance(endPos);
    float maxDistChange = totalDistance * 0.1f;

    if (totalDistance < sPlayerbotAIConfig.targetPosRecalcDistance)
    {
        if (!lastMove.lastPath.empty() && lastMove.lastPath.getBack().distance(endPos) <= totalDistance)
            lastMove.clear();

        if (mover == bot)
            ai->StopMoving();
        else
            mover->InterruptMoving(true);

        return false;
    }

    WorldPosition flyMovePosition;
    if (FlyDirect(startPos, endPos, flyMovePosition, lastMove.lastPath))
        return true;

    
    bool isWalking = false;

    TravelPath movePath = ResolveMovePath(startPos, endPos, mover, lastMove);

    lastMove.setPath(movePath);

    if (movePath.empty())
        return false;

     
    if (!bot->GetTransport())
        movePath.makeShortCut(startPos, sPlayerbotAIConfig.reactDistance, bot);

    if (movePath.empty())
    {
        lastMove.setPath(movePath);
        return true; // Path collapsed — will rebuild next tick.
    }


    TravelNodePathType pathType = TravelNodePathType::none;
    uint32 entry = 0;
    WorldPosition telePosition;
    bool specialMovement = movePath.UpcommingSpecialMovement(startPos, sPlayerbotAIConfig.reactDistance,bot->GetTransport());

    if (specialMovement)
        return HandleSpecialMovement(movePath);
    
    if (bot->GetTransport()) //Transports needed to be handled before now.
        return false;

    if (!movePath.empty())
    {
        lastMove.moveEvent = ai->GetLastEvent();
        lastMove.setPath(movePath);
    }

    movePath.ClipPath(ai, mover, ignoreEnemyTargets);

    if(ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        for (auto& p : movePath.getPath())
        {
            Creature* wpCreature = bot->SummonCreature(2334, p.point.getX(), p.point.getY(), p.point.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
            ai->AddAura(wpCreature, 246);
            if (p.point == movePath.getBack())
                ai->AddAura(wpCreature, 1130);
        }
    }   

    if (movePath.empty())
    {
        return false;
    }

    if (movePath.getFront().getMapId() == endPos.getMapId() && !endPos.isUnderWater())
    {
        for (auto& p : movePath.getPath())
        {
            if (p.point.isUnderWater())
            {
                p.point.setAtWaterSurface();
            }
        }
    }

    if (!react)
    {
        float fullPathDist = startPos.getPathLength(movePath.getPointPath());
        float waitDist = (totalDistance > sPlayerbotAIConfig.reactDistance) ? fullPathDist - 10.0f : fullPathDist;
        WaitForReach(waitDist);
    }

    if (bot == mover)
    {
        bot->HandleEmoteState(0);
        if (!bot->IsStandState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);

        if (bot->IsNonMeleeSpellCasted(true, false, true))
            ai->InterruptSpell(false);
    }

    if (totalDistance > sPlayerbotAIConfig.reactDistance && !detailedMove)
    {
        WorldPosition teleportPosition = movePath.getBack();
        if (!ai->HasPlayerNearby(teleportPosition))
        {
            time_t now = time(0);
            lastMove.nextTeleport = now + (time_t)MoveDelay(startPos.distance(teleportPosition));
            return bot->TeleportTo(teleportPosition.getMapId(),
                teleportPosition.getX(),
                teleportPosition.getY(),
                teleportPosition.getZ(),
                startPos.getAngleTo(teleportPosition));
        }
    }

    bool masterWalking = false;
    if (sPlayerbotAIConfig.walkDistance)
    {
        if (Unit* master = ai->GetMaster())
        {
            if (sServerFacade.IsFriendlyTo(bot, master) && master->m_movementInfo.HasMovementFlag(MOVEFLAG_WALK_MODE) && sServerFacade.GetDistance2d(bot, master) < sPlayerbotAIConfig.walkDistance)
            {
                masterWalking = true;
            }
        }
    }

    bool generatePath = !noPath && !bot->IsFlying() && !bot->HasMovementFlag(MOVEFLAG_SWIMMING) && !bot->IsInWater() && !sServerFacade.IsUnderwater(bot);

#ifndef MANGOSBOT_ZERO
    if (bot->IsFreeFlying())
    {
        if (bot->HasMovementFlag(MOVEFLAG_SWIMMING) && startPos.isInWater() && !startPos.isUnderWater() && !endPos.isInWater())
        {
            generatePath = true;
        }

        WorldPosition movePosition = movePath.getBack();
        //Todo fix this for paths.
        UpdateFlyingState(movePosition, totalDistance, startPos.getZ(), sPlayerbotAIConfig.reactDistance, isWalking);
    }
#endif


    // DEBUG: Check for Ironforge AH roof climbing bug
    // IF Auction House is at approx -4900, -950, 500 (Military Ward)
    const float IF_AH_X = -4900.0f;
    const float IF_AH_Y = -950.0f;
    const float IF_AH_Z = 500.0f;
    const float IF_AH_MAP = 0.0f;  // Eastern Kingdoms
    const float CHECK_RADIUS = 150.0f;
    
    if (bot->GetMapId() == (uint32)IF_AH_MAP && 
        startPos.sqDistance2d(WorldPosition(0, IF_AH_X, IF_AH_Y, 0)) < CHECK_RADIUS * CHECK_RADIUS && 
        !movePath.empty())
    {
        // Calculate total XY distance and total Z change in path
        float totalXY = 0.0f;
        float totalZ = 0.0f;
        float maxZDelta = 0.0f;
        WorldPosition prevPos = startPos;
        
        for (const auto& point : movePath.getPointPath())
        {
            float dXY = sqrtf(prevPos.sqDistance2d(WorldPosition(0, point.coord_x, point.coord_y, 0)));
            float dZ = fabs(point.coord_z - prevPos.coord_z);
            totalXY += dXY;
            totalZ += dZ;
            if (dZ > maxZDelta) maxZDelta = dZ;
            prevPos = point;
        }
        
        // Check if Z change is abnormally large compared to XY (climbing through roof)
        // Normal walking should have Z/X ratio < 0.5, roof climbing can be > 2.0
        bool isAbnormalClimb = (totalZ > 5.0f && totalXY > 0.1f && totalZ / totalXY > 1.5f) || maxZDelta > 50.0f;
        
        if (isAbnormalClimb)
        {
            bool isFromLastPath = (!lastMove.lastPath.empty() && lastMove.lastPath.getPointPath().size() == movePath.getPointPath().size());
            
            sLog.outError("[BOT PATH BUG] %s near IF AH - abnormal upward path detected!", bot->GetName());
            sLog.outError("[BOT PATH BUG] Bot pos: %.1f,%.1f,%.1f (map %d). Target: %.1f,%.1f,%.1f", 
                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(),
                movePath.getBack().coord_x, movePath.getBack().coord_y, movePath.getBack().coord_z);
            sLog.outError("[BOT PATH BUG] Path stats: %u points, XY=%.1f, Z_total=%.1f, maxZ_delta=%.1f, ratio=%.2f",
                (uint32)movePath.getPointPath().size(), totalXY, totalZ, maxZDelta, totalXY > 0 ? totalZ/totalXY : 0);
            sLog.outError("[BOT PATH BUG] Route type: %s (lastPath empty: %s, detailedMove: %s)",
                isFromLastPath ? "REUSED from lastPath" : "FRESH route",
                lastMove.lastPath.empty() ? "yes" : "no",
                detailedMove ? "yes" : "no");
            
            // Log first few path points
            char pathBuf[512];
            snprintf(pathBuf, sizeof(pathBuf), "[BOT PATH BUG] Path points: ");
            int logCount = std::min((int)movePath.getPointPath().size(), 5);
            for (int i = 0; i < logCount; i++)
            {
                const auto& p = movePath.getPointPath()[i];
                char pointBuf[64];
                snprintf(pointBuf, sizeof(pointBuf), "[#%d: %.1f,%.1f,%.1f] ", i, p.coord_x, p.coord_y, p.coord_z);
                strncat(pathBuf, pointBuf, sizeof(pathBuf) - strlen(pathBuf) - 1);
            }
            sLog.outError("%s", pathBuf);
        }
    }
    // END DEBUG

    DispatchMovement(movePath, generatePath, masterWalking);

    if (!idle)
        ClearIdleState();

    return true;
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react, bool noPath, bool ignoreEnemyTargets)
{
    return MoveTo2(WorldPosition(mapId, x, y, z), idle, react, noPath, ignoreEnemyTargets);

    WorldPosition endPosition(mapId, x, y, z, 0);
    if (!endPosition.isValid())
        return false;

    UpdateMovementState();

    if (!ai->CanMove())
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

    bool detailedMove = ai->AllowActivity(DETAILED_MOVE_ACTIVITY, true);

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

    if (!isVehicle && !ai->CanMove())
    {
        if (sServerFacade.UnitIsDead(bot))
        {
            return false;
        }
        if (sServerFacade.isMoving(bot))
        {
            return false;
        }
    }

    LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");

    if (WaitForTransport())
        return true;

    WorldPosition startPosition = WorldPosition(bot); //Current location of the bot
    WorldPosition movePosition;

    float totalDistance = startPosition.distance(endPosition); //Total distance to where we want to go
    float maxDistChange = totalDistance * 0.1;                 //Maximum change between previous destination before needing a recalculation
    TravelPath movePath;

    if (totalDistance < minDist)
    {
        if (lastMove.lastMoveShort.distance(endPosition) < maxDistChange)
            lastMove.clear();
        if (mover == bot)
            ai->StopMoving();
        else
            mover->InterruptMoving(true);
        return false;
    }

    bool isWalking = false;

    if (FlyDirect(startPosition, endPosition, movePosition, lastMove.lastPath)) //Try flying in a straight line to target.
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

        if (startPosition.getMapId() != endPosition.getMapId() || totalDistance > maxDist || (startPosition.getMapId() == 609 && fabs(startPosition.getZ() - endPosition.getZ()) > 20.0f))
        {
            if (!sTravelNodeMap.getNodes().empty() && !bot->InBattleGround())
            {
                movePath = sTravelNodeMap.getFullPath(startPosition, endPosition, bot);

                // DEBUG: Log travel node path result - use TellDebug for debug move
                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    std::ostringstream out;
                    out << "[Bot " << bot->GetName() << "] DEBUG: TravelNode path result: empty=" << movePath.empty()
                        << ", points=" << movePath.getPath().size()
                        << ", from=(" << startPosition.getX() << "," << startPosition.getY() << "," << startPosition.getZ() << ") "
                        << "to=(" << endPosition.getX() << "," << endPosition.getY() << "," << endPosition.getZ() << ")";
                    ai->TellDebug(ai->GetMaster(), out.str(), "debug move");
                }

                if (movePath.empty())
                {
                    // Debug path issues - use TellDebug for debug move
                    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                    {
                        std::vector<TravelNode*> startNodes = sTravelNodeMap.getNodes(startPosition);
                        std::vector<TravelNode*> endNodes = sTravelNodeMap.getNodes(endPosition);
                        std::ostringstream out;
                        out << "[Bot " << bot->GetName() << "] Path empty! dist:" << uint32(totalDistance)
                            << " map:" << startPosition.getMapId()
                            << " startNodes:" << startNodes.size()
                            << " endNodes:" << endNodes.size();
                        ai->TellDebug(ai->GetMaster(), out.str(), "debug move");
                    }

                    //We have no path. Beyond 450yd the standard pathfinder will probably move the wrong way.
                    if (sServerFacade.IsDistanceGreaterThan(totalDistance, maxDist * 3))
                    {
                        movePath.clear();
                        movePath.addPoint(endPosition);
                        lastMove.setPath(movePath);

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
                    std::vector<WorldPosition> beginPath = endPosition.getPathFromPath({startPosition}, bot, 40), endPath;
                    sTravelNodeMap.m_nMapMtx.lock_shared();
                    TravelNodeRoute route = sTravelNodeMap.getRoute(startPosition, endPosition, beginPath, endPath, bot);

                    std::string routeList = "Route: ";

                    for (auto node : route.getNodes())
                    {
                        routeList += node->getName() + (node == route.getNodes().back() ? "" : ",");
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
        // DEBUG: Before VMaps pathfinder - use TellDebug for debug move
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
        {
            std::ostringstream out;
            out << "[Bot " << bot->GetName() << "] DEBUG: Before VMaps PathFinder, movePath.empty=" << movePath.empty()
                << ", dist=" << movePosition.distance(startPosition);
            ai->TellDebug(ai->GetMaster(), out.str(), "debug move");
        }

        PathFinder pathfinder(mover);
        //Use standard pathfinder to find a route.
        pathfinder.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = pathfinder.getPathType();
        PointsArray& points = pathfinder.getPath();

        // DEBUG: After VMaps pathfinder - use TellDebug for debug move
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
        {
            std::ostringstream out;
            out << "[Bot " << bot->GetName() << "] DEBUG: After VMaps PathFinder: points=" << points.size()
                << ", pathType=" << (int)type;
            ai->TellDebug(ai->GetMaster(), out.str(), "debug move");
        }

        movePath.addPath(startPosition.fromPointsArray(points));
    }

    if (!lastMove.lastPath.empty() && !movePath.empty() && lastMove.lastPath.getBack().distance(endPosition) <= movePath.getBack().distance(endPosition)) //new path is worse than the last path. Keep going the old path.
    {
        movePath = lastMove.lastPath;
    }

    lastMove.setPath(movePath);

    if (!movePath.empty())
    {
        float oldDist;
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            oldDist = WorldPosition().getPathLength(movePath.getPointPath());

        if (!bot->GetTransport() && urand(0, 1))
            movePath.makeShortCut(startPosition, sPlayerbotAIConfig.reactDistance, bot);

        if (movePath.empty())
        {
            lastMove.setPath(movePath);

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                ai->TellPlayerNoFacing(GetMaster(), "Too far from path. Rebuilding.");
            return true;
        }

        TravelNodePathType pathType = TravelNodePathType::none;

        if (movePath.UpcommingSpecialMovement(startPosition, maxDist, bot->GetTransport()))
        {

            PathNodePoint nextPathPoint = movePath.getPath().front();
            PathNodeType pathType = nextPathPoint.type;
            uint32 entry = nextPathPoint.entry;

            if (pathType == PathNodeType::NODE_STATIC_PORTAL && entry) // && !ai->isRealPlayer())
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

                    std::ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                    out << bot->GetName() << ",";
                    if (telePos && telePos != movePosition)
                        startPosition.printWKT({startPosition, movePosition, telePos}, out, 1);
                    else
                        startPosition.printWKT({startPosition, movePosition}, out, 1);

                    out << std::to_string(bot->getRace()) << ",";
                    out << std::to_string(bot->getClass()) << ",";
                    float subLevel = ai->GetLevelFloat();
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

                    ai->Unmount();
                }

                std::list<ObjectGuid> gos = *context->GetValue<std::list<ObjectGuid>>("nearest game objects");
                for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
                {
                    GameObject* go = ai->GetGameObject(*i);
                    if (!go)
                        continue;

                    if (go->GetEntry() != entry)
                        continue;

                    if (!bot->GetGameObjectIfCanInteractWith(go->GetObjectGuid(), MAX_GAMEOBJECT_TYPE))
                        continue;

                    std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
                    *packet << *i;
                    bot->GetSession()->QueuePacket(std::move(packet));
                    return true;
                }

                return false;
            }

            if (pathType == PathNodeType::NODE_AREA_TRIGGER)
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

                    std::ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                    out << bot->GetName() << ",";
                    if (telePos && telePos != movePosition)
                        startPosition.printWKT({startPosition, movePosition, telePos}, out, 1);
                    else
                        startPosition.printWKT({startPosition, movePosition}, out, 1);

                    out << std::to_string(bot->getRace()) << ",";
                    out << std::to_string(bot->getClass()) << ",";
                    float subLevel = ai->GetLevelFloat();
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

            if (pathType == PathNodeType::NODE_TRANSPORT)
            {
                WorldPosition telePosition = std::next(movePath.getPath().begin())->point;
                bool usedTransport = UseTransport(ai, entry, bot->GetTransport() ? telePosition : movePosition, movePosition, sPlayerbotAIConfig.transportTeleportType > 0);
                if (!usedTransport)
                {
                    if (bot->GetTransport())
                        lastMove.lastTransportEntry = entry;

                    WaitForReach(1000.0f);
                }
                else
                {
                    if (!bot->GetTransport())
                    {
                        return bot->TeleportTo(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), movePosition.getO(), 0);
                    }

                    lastMove.lastTransportEntry = entry;

                    WaitForReach(1000.0f);
                }

                return true;

                /*
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
                            ai->TellPlayer(GetMaster(), "transport at " + std::to_string(uint32(telePosition.distance(transport))) + "yards of entry");

                        if (telePosition.distance(transport) < INTERACTION_DISTANCE) //Transport has arrived Move on.
                        {
                            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                                ai->TellPlayerNoFacing(GetMaster(), "Moving on to transport " + std::string(transport->GetName()));

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
                                std::vector<WorldPosition> step = onBoatPos.getPathStepFrom(bot, bot);
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
                        ai->TellPlayerNoFacing(GetMaster(), "I'm on " + std::string(bot->GetTransport()->GetName()));
                    ai->SetMoveToTransport(false);
                    entry = 0;
                }

                if (movePosition.getMapId() == bot->GetMapId() && ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                    ai->TellPlayer(GetMaster(), "transport at " + std::to_string(uint32(telePosition.distance(bot->GetTransport()))) + "yards of exit");

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
            */
            }

            if (pathType == PathNodeType::NODE_FLIGHTPATH && entry)
            {
                return UseTaxi(ai, entry, true);
            }

            if (pathType == PathNodeType::NODE_TELEPORT && entry)
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
                        lastMove.setPath(movePath);
                        return false;
                    }
                }
                else
                {
                    if (sServerFacade.IsSpellReady(bot, entry) && (!bot->IsFlying() || WorldPosition(bot).currentHeight() < 10.0f) && AI_VALUE2(uint32, "has reagents for", entry) > 0)
                    {
                        if (AI_VALUE2(uint32, "current mount speed", "self target"))
                        {
                            ai->Unmount();
#ifdef MANGOSBOT_TWO
                            return false;
#endif
                        }

                        ai->RemoveShapeshift();

                        if (ai->DoSpecificAction("cast", Event("rpg action", chat->formatWorldobject(bot) + " " + std::to_string(entry)), true))
                            return true;
                    }

                    movePath.clear();
                    lastMove.setPath(movePath);
                    return false;
                }
            }

            if (pathType == PathNodeType::NODE_PATH && movePath.getPath().begin()->type != PathNodeType::NODE_FLIGHTPATH)
                isWalking = true;
        }

        //if (!isTransport && bot->GetTransport())
        //    bot->GetTransport()->RemovePassenger(bot);
    }
    else if (bot->GetTransport()) //Wait until we can recalculate.
        return false;

    if (!movePath.empty() && movePath.getBack().distance(movePath.getFront()) > maxDist)
    {
        lastMove.moveEvent = ai->GetLastEvent();
        lastMove.setPath(movePath);
    }

    if (!movePosition || movePosition.getMapId() != bot->GetMapId())
    {
        if (!bot->GetTransport() || movePath.getPath().size() == 1)
            movePath.clear();
        lastMove.setPath(movePath);

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(GetMaster(), "No point. Rebuilding.");

        return false;
    }

    if (movePosition.distance(startPosition) > maxDist && !bot->GetTransport())
    { //Use standard pathfinder to find a route.
        PathFinder path(mover);
        path.calculate(movePosition.getX(), movePosition.getY(), movePosition.getZ(), false);
        PathType type = path.getPathType();
        PointsArray& points = path.getPath();
        movePath.addPath(startPosition.fromPointsArray(points));
        TravelNodePathType pathType;
        uint32 entry;
        WorldPosition telepos;
                       
        movePosition = movePath.getBack();
    }

    //Stop the path when we might get aggro.
    if (!ai->IsStateActive(BotState::BOT_STATE_COMBAT) && !bot->IsDead() && !ignoreEnemyTargets)
    {
        std::list<ObjectGuid> targets = AI_VALUE_LAZY(std::list<ObjectGuid>, "possible attack targets");

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

    if (movePosition == WorldPosition())
    {
        movePath.clear();
        lastMove.setPath(movePath);

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(GetMaster(), "No point. Rebuilding.");
        return false;
    }

    if (movePosition.getMapId() == endPosition.getMapId() && movePosition.isUnderWater() && !endPosition.isUnderWater()) //Try to swim on the surface.
    {
        movePosition.setZ(movePosition.getHeight(true));

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            ai->TellPlayerNoFacing(GetMaster(), "Setting movePosition to water surface.");
    }

    //Visual waypoints
    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        if (!movePath.empty())
        {
            for (auto i : movePath.getPath())
            {
                if (i.point.getMapId() == bot->GetMapId())
                    CreateWp(bot, i.point.getX(), i.point.getY(), i.point.getZ(), 0.0, 2334);
            }
        }
        else
            CreateWp(bot, movePosition.getX(), movePosition.getY(), movePosition.getZ(), 0, 2334, true);
    }

    //Log bot movement
    if (sPlayerbotAIConfig.hasLog("bot_movement.csv") && lastMove.lastMoveShort != movePosition)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        startPosition.printWKT({startPosition, movePosition}, out, 1);
        out << std::to_string(bot->getRace()) << ",";
        out << std::to_string(bot->getClass()) << ",";
        float subLevel = ai->GetLevelFloat();
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
    if (mm.GetCurrent()->GetMovementGeneratorType() != POINT_MOTION_TYPE || movePosition.fDist(lastMove.lastMoveShort) > 5.0f)
    {
        if (mover == bot)
            ai->StopMoving();
        else if (mover)
            mover->InterruptMoving(true);
    }

    if (totalDistance > maxDist && !detailedMove && !ai->HasPlayerNearby(movePosition)) //Why walk if you can fly?
    {
        time_t now = time(0);

        lastMove.nextTeleport = now + (time_t)MoveDelay(startPosition.distance(movePosition));

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
    if (IsHazardNearPosition(movePosition))
    {
        if (!react)
        {
            SetDuration(sPlayerbotAIConfig.reactDelay);
        }

        return false;
    }

#ifdef MANGOSBOT_ZERO
        mm.MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);
#else
    if (!bot->IsFreeFlying())
    {
        // water transition
        if (bot->HasMovementFlag(MOVEFLAG_SWIMMING) && startPosition.isInWater() && !startPosition.isUnderWater() && !movePosition.isInWater())
            generatePath = true;

        mm.MovePoint(movePosition.getMapId(), movePosition.getX(), movePosition.getY(), movePosition.getZ(), masterWalking ? FORCED_MOVEMENT_WALK : FORCED_MOVEMENT_RUN, generatePath);
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
                        movePosition.setZ(std::min(std::max(ground, botground) + 100.0f, std::max(movePosition.getZ() + 10.0f, bot->GetPositionZ() + 10.0f)));
                    }
                    else
                    {
                        movePosition.setZ(std::max(std::max(ground, botground), bot->GetPositionZ() - 10.0f));
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

    lastMove.lastMoveShort = movePosition;

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

    return ai->CanMove();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    float distance = bot->GetDistance(x, y, z);
    if (!bot->InBattleGround() && distance > sPlayerbotAIConfig.reactDistance)
        return false;

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
}

bool MovementAction::Follow(Unit* target, float distance, float angle)
{
	if (!ai->IsSafe(target))
    return false;

    MotionMaster &mm = *bot->GetMotionMaster();

    distance = distance <= target->GetObjectBoundingRadius() ? 0 : distance - target->GetObjectBoundingRadius();

    UpdateMovementState();

    if (FollowOnTransport(target))
        return true;

    WorldPosition botPos(bot);
    WorldPosition tarPos(target);

    //Move to target corpse if alive.
    if (!target->IsAlive() && bot->IsAlive() && target->GetObjectGuid().IsPlayer())
    {
        Player* pTarget = (Player*)target;

        Corpse* corpse = pTarget->GetCorpse();

        if (corpse)
        {
            WorldPosition cPos(corpse);

            if(botPos.fDist(cPos) > sPlayerbotAIConfig.spellDistance)
                return MoveTo(cPos.getMapId(),cPos.getX(),cPos.getY(), cPos.getZ());
            return false;
        }
    }

    float tDist = botPos.fDist(tarPos);

    if (tDist > sPlayerbotAIConfig.sightDistance || (target->IsFlying() && !bot->IsFreeFlying()) || target->IsTaxiFlying())
    {
        if (target->GetObjectGuid().IsPlayer())
        {
            Player* player = (Player*)target;

            if (ai->IsSafe(player))
            {
                if (player->GetPlayerbotAI()) //Try to move to where the bot is going if it is closer and in the same direction.
                {
                    WorldPosition longMove = PAI_VALUE(WorldPosition, "last long move");

                    if (longMove)
                    {
                        return MoveTo(longMove.getMapId(), longMove.getX(), longMove.getY(), longMove.getZ());
                    }
                }
            }

            if (player->IsTaxiFlying()) //Move to where the player is flying to.
            {
                const Taxi::Map tMap = player->GetTaxiPathSpline();
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

    // Handle water transition
    {
        bool targetInWater = (tarPos.isInWater() || tarPos.isUnderWater()) && !botPos.isInWater() && !botPos.isUnderWater();
        bool selfInWater = (botPos.isInWater() || botPos.isUnderWater()) && !tarPos.isInWater() && !tarPos.isUnderWater();
        bool targetOnSurface = botPos.isUnderWater() && tarPos.isInWater() && !tarPos.isUnderWater();
        bool selfOnSurface = tarPos.isUnderWater() && botPos.isInWater() && !botPos.isUnderWater();
        if ((targetInWater || selfInWater || targetOnSurface || selfOnSurface) && !(tarPos.isUnderWater() && botPos.isUnderWater()))
        {
            // in or out of water
            WorldPosition moveToPos = (targetInWater || selfOnSurface) ? tarPos : botPos;
            Unit* targetToCheck = (targetInWater || selfOnSurface) ? target : bot;
            if (const TerrainInfo* terrain = moveToPos.getTerrain())
            {
                float bottom = terrain->GetHeightStatic(moveToPos.getX(), moveToPos.getY(), moveToPos.getZ());
                float waterLevel = terrain->GetWaterOrGroundLevel(moveToPos.getX(), moveToPos.getY(), moveToPos.getZ(), bottom, true);
                bool canSwimToTarget = selfOnSurface && botPos.IsInLineOfSight(tarPos);
                moveToPos.setZ(waterLevel);
                if (waterLevel > -200000.0f && waterLevel > bottom)
                {
                    PathFinder pathfinder(bot);
                    //Use standard pathfinder to find a route.
                    WorldPosition prevPoint = botPos;
                    pathfinder.calculate(moveToPos.getVector3(), tarPos.getVector3());
                    Movement::PointsArray& pathPoints = pathfinder.getPath();
                    if (pathPoints.size() >= 2)
                    {
                        for (uint32 i = 1; i < pathPoints.size() - 1; i++)
                        {
                            WorldPosition pathPoint(bot->GetMapId(), pathPoints[i].x, pathPoints[i].y, pathPoints[i].z);
                            if (selfInWater)
                            {
                                if (pathPoint.isInWater())
                                {
                                    prevPoint = pathPoint;
                                    continue;
                                }
                                if (!MoveTo(prevPoint))
                                {
                                    return MoveTo(pathPoint);
                                }
                                return true;
                            }
                        }
                    }
                    moveToPos = tarPos;
                    return MoveTo(moveToPos);
                }
            }
        }
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

    // Find jump shortcut
    if (ai->HasStrategy("follow jump", BotState::BOT_STATE_NON_COMBAT) && ai->AllowActivity())
    {
        WorldPosition botPos(bot);
        if (!botPos.isInWater())
        {
            bool tryJump = ai->DoSpecificAction("jump::follow", Event(), true);
            if (tryJump)
                return true;
        }
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit* currentTarget = sServerFacade.GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetObjectGuid() == target->GetObjectGuid() && sServerFacade.GetChaseAngle(bot) == angle && sServerFacade.GetChaseOffset(bot) == distance)
            return false;
    }

    mm.MoveFollow(target, distance, angle, true, sPlayerbotAIConfig.boostFollow);
    return true;
}

WorldPosition CalculatePerpendicularPoint(const WorldPosition& A, const WorldPosition& B, float offset, bool left = true)
{
    WorldPosition direction = (B - A);
    if (direction)
        direction = direction/direction.size();

    WorldPosition perpendicularDirection(0,-direction.getY(), direction.getX(), direction.getZ());

    if (!left)
    {
        perpendicularDirection.setX(-perpendicularDirection.getX());
        perpendicularDirection.setY(-perpendicularDirection.getY());
    }

    return B + (perpendicularDirection * offset);
}

bool MovementAction::ChaseTo(WorldObject* obj, float distance, float angle)
{
    if (!ai->CanMove())
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
        Vector3 possibleEndPoint = CalculatePerpendicularPoint(endPoint, hazardPoint, hazardRangeOffset, true).getVector3();

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
            possibleEndPoint = CalculatePerpendicularPoint(endPoint, hazardPoint, hazardRangeOffset, false).getVector3();

            endPosition.coord_x = possibleEndPoint.x;
            endPosition.coord_y = possibleEndPoint.y;
            endPosition.coord_z = possibleEndPoint.z;
        }
    }

    MotionMaster& mm = *bot->GetMotionMaster();

    // Prevent moving if requested to move into a hazard
    if (IsValidPosition(endPosition, botPosition))
    {
        std::vector<WorldPosition> path = botPosition.getPathTo(endPosition,bot);
        if (GeneratePathAvoidingHazards(path))
        {
            float distance = botPosition.getPathLength(path);
            mm.Clear(false, true);

            std::vector<G3D::Vector3> pointsArray = WorldPosition().toPointsArray(path);
#ifndef MANGOSBOT_TWO  
            mm.MovePath(pointsArray, FORCED_MOVEMENT_RUN, false, false);
#else
            mm.MovePath(pointsArray, FORCED_MOVEMENT_RUN, false);
#endif
            WaitForReach(distance);
            return true;
        }
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        if (!bot->IsStopped() &&
            sServerFacade.GetChaseTarget(bot) == obj && 
            sServerFacade.GetChaseOffset(bot) == distance)
        {
            bot->SetTarget(obj); //Needed to keep chase going in combat.
            bot->Attack((Unit*)obj, false); //Needed to keep chase going in combat.
            return true;
        }
    }

    // charge
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE && !bot->IsStopped())
    {
        return false;
    }

    if (!ai->IsSafe(obj)) return false;

    // Find jump shortcut
    if (ai->HasStrategy("follow chase", BotState::BOT_STATE_NON_COMBAT) && ai->AllowActivity())
    {
        bool tryJump = ai->DoSpecificAction("jump::chase", Event(), true);
        if (tryJump)
            return true;
    }

    if (!endPosition.isValid()) return false;
    if (angle > 20) angle = 0;

    bot->SetTarget(obj); //Needed to keep chase going in combat.
    bot->Attack((Unit*)obj, false); //Needed to keep chase going in combat.

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

    if (!ai->CanMove())
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
        std::vector<Unit*> possibleTargets;
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
            std::vector<uint32> sounds;
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
    std::list<HazardPosition> hazards = AI_VALUE(std::list<HazardPosition>, "hazards");
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

bool MovementAction::GeneratePathAvoidingHazards(std::vector<WorldPosition>& movePath)
{
    std::list<HazardPosition> hazards = AI_VALUE(std::list<HazardPosition>, "hazards");
    if (hazards.empty())
        return false;

    std::vector<WorldPosition> collidingHazards;
    bool pathModified = false;

    // Start the iteration on the second point (the first and last points can't be modified)
    bool firstPoint = true;
    uint8 pointsInserted = 0;
    const uint8 maxPointsInserted = 20;
    WorldPosition previousPosition = movePath.front();
    for (uint32 i = 1; i < movePath.size() - 1; i++)
    {
        bool pointInserted = false;
        WorldPosition pathPoint = movePath[i];
        for (auto& [hazardPosition, hazardRange] : hazards)
        {
            const float hazardRangeOffset = hazardRange * 1.5f;

            // Check if the path point is inside a hazard
            {
                const float distanceToHazard = pathPoint.distance(hazardPosition);
                if (distanceToHazard <= hazardRange)
                {
                    collidingHazards.push_back(hazardPosition);

                    // Move the point out of the hazard range in perpendicular from previous point
                    // Generate point translated to the left
                    WorldPosition possiblePathPoint = CalculatePerpendicularPoint(previousPosition, hazardPosition, hazardRangeOffset, true);

                    // Check if point is valid
                    WorldPosition possiblePathPosition(bot->GetMapId(), possiblePathPoint.getX(), possiblePathPoint.getY(), possiblePathPoint.getZ());
                    if (IsValidPosition(possiblePathPosition, previousPosition))
                    {
                        pathModified = true;
                        pathPoint = possiblePathPoint;
                    }
                    else
                    {
                        // Generate point translated to the right
                        possiblePathPoint = CalculatePerpendicularPoint(previousPosition, hazardPosition, hazardRangeOffset, false);

                        // Check if point is valid
                        WorldPosition possiblePathPosition(bot->GetMapId(), possiblePathPoint.getX(), possiblePathPoint.getY(), possiblePathPoint.getZ());
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
                WorldPosition directionFromPreviousPoint = (pathPoint - previousPosition);
                const float distanceToPreviousPoint = std::max(directionFromPreviousPoint.size(), 0.0001f);
                directionFromPreviousPoint = directionFromPreviousPoint / distanceToPreviousPoint;
                WorldPosition inBetweenPathPoint = previousPosition + (directionFromPreviousPoint * distanceToPreviousPoint * 0.5f);

                // Check if the point between path points is inside a hazard
                WorldPosition directionFromHazard = (inBetweenPathPoint - hazardPosition);
                const float distanceToHazard = std::max(directionFromHazard.size(), 0.0001f);
                if (distanceToHazard <= hazardRange)
                {
                    collidingHazards.push_back(hazardPosition);

                    // If so generate a new path point to go around it
                    inBetweenPathPoint = hazardPosition + ((directionFromHazard / distanceToHazard) * hazardRangeOffset);

                    // Check if the point is valid
                    WorldPosition inBetweenPathPosition(bot->GetMapId(), inBetweenPathPoint.getX(), inBetweenPathPoint.getY(), inBetweenPathPoint.getZ());
                    if (IsValidPosition(inBetweenPathPosition, previousPosition))
                    {
                        // Insert the new point to the path (before current point)
                        pathModified = true;
                        pointInserted = true;
                        movePath.insert(movePath.begin() + i, inBetweenPathPoint);
                        pointsInserted++;
                        continue;
                    }
                }
            }
        }

        if (pointInserted)
        {
            // Go back one step to validate the inserted point and move to next loop
            i--;
        }
        else
        {
            firstPoint = false;
            previousPosition.coord_x = pathPoint.getX();
            previousPosition.coord_y = pathPoint.getY();
            previousPosition.coord_z = pathPoint.getZ();
        }
    }

    if (pathModified && !movePath.empty())
    {
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
        {
            for (auto& pathPoint : movePath)
            {
                bot->SummonCreature(1, pathPoint.getX(), pathPoint.getY(), pathPoint.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);
            }

            for (auto& hazards : collidingHazards)
            {
                bot->SummonCreature(15631, hazards.getX(), hazards.getY(), hazards.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f);
            }
        }

        return true;
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
    {
        if (ai->HasStrategy("debug loot", BotState::BOT_STATE_NON_COMBAT))
        {
            WorldObject* wo = loot.GetWorldObject(bot);

            if (!wo)
            {
                ai->TellPlayerNoFacing(GetMaster(), "Can not move to loot " + std::to_string(loot.guid) +  " because it no longer exists.");
            }
            else
            {
                ai->TellPlayerNoFacing(GetMaster(), "Can not move to loot " + ChatHelper::formatWorldobject(wo) + " because it is not possible to loot.");
            }
        }

        return false;
    }

    WorldObject *wo = loot.GetWorldObject(bot);

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("debug loot", BotState::BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Moving to loot " << ChatHelper::formatWorldobject(wo);
        ai->TellPlayerNoFacing(GetMaster(), out);
    }

    if(sServerFacade.IsWithinLOSInMap(bot, wo))
        return MoveNear(wo, sPlayerbotAIConfig.contactDistance);

    return MoveTo(WorldPosition(wo));
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
    WorldPosition botPos(bot);
    float gx, gy, gz;
    gx = botPos.getX();
    gy = botPos.getY();
    gz = botPos.getZ();

    uint32 tries = 1;
    for (; tries < 10; ++tries)
    {
        gx = botPos.getX();
        gy = botPos.getY();
        gz = botPos.getZ();
#ifndef MANGOSBOT_TWO  
        if (bot->GetMap()->GetReachableRandomPointOnGround(gx, gy, gz, ai->GetRange("follow")))
#else
        if (bot->GetMap()->GetReachableRandomPointOnGround(bot->GetPhaseMask(), gx, gy, gz, ai->GetRange("follow")))
#endif
        {
            return MoveTo(bot->GetMapId(), gx, gy, gz);
        }
    }

    // old style
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

    return AI_VALUE2(bool, "collision", "self target") && ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest friendly players")->Get().size() < 15 &&
        ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest non bot players")->Get().size() > 0;
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
    return !ai->HasRealPlayerMaster() && ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest friendly players")->Get().size() > urand(25, 100);
}

bool MoveToAction::Execute(Event& event)
{
    std::list<GuidPosition> guidList = AI_VALUE_SAFE(std::list<GuidPosition>, getQualifier());

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

    std::string param = event.getParam();
    std::string qualify = getQualifier();
    std::string options = !param.empty() ? param : !qualify.empty() ? qualify : "";
    bool jumpInPlace = false;
    bool jumpBackward = false;
    bool showLanding = false;
    bool isRtsc = false;
    bool toPosition = false;

    // only show landing
    if (options.find("show") != std::string::npos && options.size() > 5)
    {
        options = options.substr(5);
        showLanding = true;
    }
    // to position
    if (options.find("position") != std::string::npos && options.size() > 9)
    {
        options = options.substr(9);
        toPosition = true;
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
        if (sServerFacade.isMoving(bot) || bot->IsMounted())
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
    if (options == "tome" || options == "follow" || options == "chase" || isRtsc || toPosition)
    {
        if (options == "follow" && !(ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("wander", BotState::BOT_STATE_NON_COMBAT)))
            return false;

        WorldPosition const src = WorldPosition(bot);
        WorldPosition dest = WorldPosition();
        WorldPosition jumpPoint = WorldPosition();
        WorldPosition possibleLanding = WorldPosition();
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
            /*bot->InBattleGround() ||*/
            bot->GetTransport()))
                return false;

            // do not try if very close
            Formation* formation = AI_VALUE(Formation*, "formation");
            if (formation)
            {
                WorldLocation loc = formation->GetLocation();
                if (!Formation::IsNullLocation(loc) && loc.mapid != -1)
                {
                    if (sServerFacade.GetDistance2d(bot, loc.coord_x, loc.coord_y) < ai->GetRange("follow") && fabs(src.getZ() - loc.coord_z) < ai->GetRange("follow"))
                        return false;
                }
            }

            dest = WorldPosition(followTarget);
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
            /*bot->InBattleGround() ||*/
            bot->GetTransport()))
                return false;

            dest = WorldPosition(chaseTarget);
            distanceTo = 30.f;
            distanceFrom = sPlayerbotAIConfig.sightDistance;
        }

        if (toPosition)
        {
            ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
            ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()[options];
            if (!pos.isSet())
                return false;

            dest = WorldPosition(pos.Get());
            distanceTo = 50.0f;
            distanceFrom = 50.0f;
        }

        // try nearby random points
        if (!jumpPoint && ai->AllowActivity())
            jumpPoint = GetPossibleJumpStartForInRange(src, dest, possibleLanding, bot, requiredSpeed, distanceTo, distanceFrom);
        // try with pathfinder
        if (!jumpPoint)
            jumpPoint = GetPossibleJumpStartFor(src, dest, possibleLanding, bot, requiredSpeed, distanceTo, distanceFrom);
        if (jumpPoint && requiredSpeed > 0.f)
        {
            // check if jumping is much faster
            if (options == "follow" || options == "chase" || toPosition)
            {
                if (!IsJumpFasterThanWalking(src, dest, possibleLanding, bot))
                    return false;
            }
            sLog.outDebug("%s: GetPossibleJumpStartFor success! Jump speed: %f", bot->GetName(), requiredSpeed);

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

                return MoveTo(jumpPoint.getMapId(), jumpPoint.getX(), jumpPoint.getY(), jumpPoint.getZ());
            }
            else // jump from current position
            {
                if (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
                {
                    std::string text = "Jumping to you!";
                    bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                }

                if (showLanding)
                {
                    Creature* wpCreature = bot->SummonCreature(2334, possibleLanding.getX(), possibleLanding.getY(), possibleLanding.getZ() - 1, bot->GetOrientation(), TEMPSPAWN_TIMED_DESPAWN, 3000);
                    PlayerbotAI::AddAura(wpCreature, 246);

                    float pointAngle = src.getAngleTo(possibleLanding);
                    sServerFacade.SetFacingTo(bot, pointAngle, true);
                    bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                    SetDuration(sPlayerbotAIConfig.reactDelay);
                    return true;
                }

                float pointAngle = jumpPoint.getAngleTo(possibleLanding ? possibleLanding : dest);
                sServerFacade.SetFacingTo(bot, pointAngle, true);
                bool success = JumpTowards(jumpPoint, possibleLanding ? possibleLanding : dest, bot, requiredSpeed, possibleLanding);

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
        jumpSpeed = jumpBackward ? bot->GetSpeed(MOVE_WALK) : bot->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed;

        if (options == "r" || options == "random")
        {
            // slow jump
            if (urand(0, 1))
                jumpSpeed = bot->GetSpeed(MOVE_WALK);
        }
    }

    float timeToLand, distToLand, maxHeight;
    bool goodLanding = true;
    std::vector<WorldPosition> path;
    WorldPosition jumpLanding = JumpAction::CalculateJumpParameters(WorldPosition(bot), bot, angle, sPlayerbotAIConfig.jumpVSpeed, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, path);
    if (jumpLanding)
    {
        // not jump randomly in the water
        if ((options == "r" || options == "random") && !event.getOwner() && (jumpLanding.isInWater() || jumpLanding.isUnderWater()))
        {
            sLog.outDetail("%s: Jump random fail: landing in water!", bot->GetName());
            return false;
        }

        // only show landing
        if (showLanding || ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
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

        // set highest jump point to relocate
        WorldPosition highestPoint = jumpLanding;
        for (auto& point : path)
        {
            if (point.getZ() > highestPoint.getZ())
                highestPoint = point;
        }

        return DoJump(jumpLanding, highestPoint, angle, sPlayerbotAIConfig.jumpVSpeed, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, jumpInPlace, jumpBackward, showLanding);
    }
    return false;
}

WorldPosition JumpAction::CalculateJumpParameters(const WorldPosition& src, Unit* jumper, float angle, float vSpeed, float hSpeed, float &timeToLand, float &distanceToLand, float &maxHeight, bool &goodLanding, std::vector<WorldPosition>& path, float maxJumpHeight)
{
    if (!jumper)
        return WorldPosition();

    // static data
    float const m_gravity = 19.2911f;
    float const timeForMaxHeight = vSpeed / m_gravity;
    float velocity = sqrt(vSpeed * vSpeed + hSpeed * hSpeed);
    double jumpVerticalAngle = 48.f * M_PI / 180; // approximate
    maxHeight = vSpeed * timeForMaxHeight - m_gravity * timeForMaxHeight * timeForMaxHeight / 2;   

    // jump in place
    if (hSpeed == 0.f)
    {
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
            // check distanct collision
            if (ascending)
                fz += jumper->GetCollisionHeight();

            fx += jumper->GetCollisionWidth() * vcos;
            fy += jumper->GetCollisionWidth() * vsin;

#ifdef MANGOSBOT_TWO
            foundCollision = jumper->GetMap()->GetHitPosition(ox, oy, oz, fx, fy, fz, jumper->GetPhaseMask(), -0.5f);
#else
            foundCollision = jumper->GetMap()->GetHitPosition(ox, oy, oz + 0.5f, fx, fy, fz, -0.5f);
#endif

            if (!foundCollision)
            {
                fx -= jumper->GetCollisionWidth() * vcos;
                fy -= jumper->GetCollisionWidth() * vsin;
                if (ascending)
                    fz -= jumper->GetCollisionHeight();
            }
        }

        path.push_back(WorldPosition(src.getMapId(), fx, fy, fz));

        // vmaps collision not found - check maps (terrain or water)
        if (!foundCollision && !ascending)
        {
            // check ground below current previous point
            float prevGroundZ = oz;
            float nextGroundZ = fz;
            jumper->UpdateAllowedPositionZ(ox, oy, prevGroundZ);
            jumper->UpdateAllowedPositionZ(fx, fy, nextGroundZ);
            // calculated point is lower than terrain - land on terrain
            if (fz < nextGroundZ && oz > prevGroundZ)
            {
                foundCollision = true;
                fx = ox;
                fy = oy;
                fz = prevGroundZ;
            }
        }

        if (maxJumpHeight > 0.f && fabs(src.getZ() - fz) > maxJumpHeight)
            return WorldPosition();

        // vmaps collision found
        if (foundCollision)
        {
            // hit something while ascending
            if (ascending)
            {
                goodLanding = false;
                // reduce landing height by collision height
                float fz_mod = fz - CONTACT_DISTANCE - jumper->GetCollisionHeight();
#ifdef MANGOSBOT_TWO
                jumper->GetMap()->GetHitPosition(fx, fy, fz, fx, fy, fz_mod, jumper->GetPhaseMask(), -0.5f);
#else
                jumper->GetMap()->GetHitPosition(fx, fy, fz, fx, fy, fz_mod, -0.5f);
#endif
                fz = fz_mod;
                //fz = fz - CONTACT_DISTANCE - jumper->GetCollisionHeight();
            }

            WorldPosition destination = WorldPosition(src.getMapId(), fx, fy ,fz);
            if (!IsJumpSafe(src, destination, jumper))
                return WorldPosition();

            distanceToLand = sqrtf(src.sqDistance2d(destination));
            timeToLand = CalculateJumpTime(fz - (src.getZ() + 0.5f), vSpeed, ascending);

            // some error in time calculations - cancel the jump
            if (timeToLand == 0.f)
                return WorldPosition();

            // maybe hit a wall while descending
            if (goodLanding)
            {
                float groundZ = destination.getZ() + 0.5f;
                if (!destination.isInWater())
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

    std::vector<WorldPosition> path = dest.getPathStepFrom(src, jumper, true);
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

bool JumpAction::IsJumpFasterThanWalking(const WorldPosition& src, const WorldPosition& dest, const WorldPosition& jumpLanding, Unit* jumper, float maxDistance)
{
    if (!src || !dest || !jumpLanding)
        return false;

    if (src.getMapId() != dest.getMapId() || src.getMapId() != jumpLanding.getMapId())
        return false;

    // landing too far from destination
    if (dest.fDist(jumpLanding) > maxDistance)
        return false;

    std::vector<WorldPosition> pathWalk = dest.getPathStepFrom(src, jumper, true);
    std::vector<WorldPosition> pathJump = dest.getPathStepFrom(jumpLanding, jumper, true);
    if (pathJump.empty())
        return false;
    if (pathWalk.empty())
        return true;

    float pathLengthWalk = src.getPathLength(pathWalk);
    float pathLengthJump = jumpLanding.getPathLength(pathJump);

    if (pathLengthWalk > 20.f && pathLengthWalk > (pathLengthJump * 2))
    {
        sLog.outDebug("%s: Jump IsJumpFasterThanWalking Jumping is faster than walking! Walk Distance: %f, Jump Distance: %f", jumper->GetName(), pathLengthWalk, pathLengthJump);
        return true;
    }

    sLog.outDebug("%s: Jump IsJumpFasterThanWalking Jumping is slower than walking! Walk Distance: %f, Jump Distance: %f", jumper->GetName(), pathLengthWalk, pathLengthJump);
    return false;
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

bool JumpAction::CanJumpTo(const WorldPosition& src, const WorldPosition& dest, WorldPosition& possiblelanding, float& jumpAngle, Unit* jumper, float jumpSpeed, float maxDistance)
{
    if (!src || !dest)
        return false;

    if (src.getMapId() != dest.getMapId())
        return false;

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return false;

    // some preparation
    static float m_gravity = 19.2911f;
    float timeForMaxHeight = sPlayerbotAIConfig.jumpVSpeed / m_gravity;
    float maxHeight = sPlayerbotAIConfig.jumpVSpeed * timeForMaxHeight - m_gravity * timeForMaxHeight * timeForMaxHeight / 2;

    // can't jump too high
    if ((src.getZ() + maxHeight) < dest.getZ())
        return false;

    float destAngle = src.getAngleTo(dest);
    for (float angle = destAngle; angle <= destAngle + 2 * M_PI; angle += M_PI_F / 4.0f)
    {
        float timeToLand, distToLand;
        bool goodLanding = true;
        std::vector<WorldPosition> path;
        WorldPosition jumpLanding = JumpAction::CalculateJumpParameters(src, jumper, angle, sPlayerbotAIConfig.jumpVSpeed, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, path);
        if (jumpLanding && CanWalkTo(jumpLanding, dest, jumper, maxDistance))
        {
            jumpAngle = angle;
            possiblelanding = jumpLanding;
            return true;
        }
    }

    return false;
}

bool JumpAction::JumpTowards(const ai::WorldPosition &src, const ai::WorldPosition &dest, Unit* jumper, float jumpSpeed, bool preSetLanding)
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
    std::vector<WorldPosition> path;
    WorldPosition jumpLanding = JumpAction::CalculateJumpParameters(src, jumper, angle, sPlayerbotAIConfig.jumpVSpeed, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, path);
    sLog.outDebug("%s: JumpTowards attempt! Jump speed: %f", bot->GetName(), jumpSpeed);
    if (jumpLanding && goodLanding)
    {
        // set highest jump point to relocate
        WorldPosition highestPoint = dest;
        for (auto& point : path)
        {
            if (point.getZ() > highestPoint.getZ())
                highestPoint = point;
        }

        return DoJump((preSetLanding ? dest : jumpLanding), highestPoint, angle, sPlayerbotAIConfig.jumpVSpeed, jumpSpeed, timeToLand, distToLand, maxHeight, goodLanding, jumpInPlace, jumpBackward, false);
    }

    sLog.outDetail("%s: Jump ForwardTo Fail!", jumper->GetName());
    return false;
}

bool JumpAction::DoJump(const WorldPosition &dest, const WorldPosition& highestPoint, float angle, float vSpeed, float hSpeed, float timeToLand, float distanceToLand, float maxHeight, bool goodLanding, bool jumpInPlace, bool jumpBackward, bool showOnly)
{
    if (!dest)
        return false;

    WorldPosition landing = dest;

    // fix height
    if (goodLanding && !dest.isInWater())
    {
        float ox = dest.getX();
        float oy = dest.getY();
        float oz = dest.getZ() + 0.5f;
        bot->UpdateAllowedPositionZ(ox, oy, oz);
        landing = WorldPosition(dest.getMapId(), ox, oy, oz);
    }

    if (!goodLanding)
        ai->SetFallAfterJump();

    ai->InterruptSpell(false);
    ai->StopMoving();
    ai->SetJumpDestination(landing);
    bot->SetFallInformation(0, maxHeight);

    bool slowJump = false;// !jumpBackward && hSpeed == bot->GetSpeed(MOVE_WALK);
    // TODO calculate slow jump (jump + move forward)

    // send move packet before jump
    if (!jumpInPlace && !slowJump)
    {
        bot->m_movementInfo.AddMovementFlag(jumpBackward ? MOVEFLAG_BACKWARD : MOVEFLAG_FORWARD);
        WorldPacket move(jumpBackward ? MSG_MOVE_START_BACKWARD : MSG_MOVE_START_FORWARD);
// write packet info
#ifdef MANGOSBOT_TWO
        move << bot->GetObjectGuid().WriteAsPacked();
#endif
        move << bot->m_movementInfo;
        ai->QueuePacket(move);
    }

    float vsin = jumpInPlace ? 0 : sin(angle);
    float vcos = jumpInPlace ? 1 : cos(angle);

    // write jump info
    uint32 curTime = sWorld.GetCurrentMSTime();
    uint32 jumpTime = curTime + sWorld.GetAverageDiff() * 2 + uint32(timeToLand * static_cast<float>(IN_MILLISECONDS));
    ai->SetJumpTime(jumpTime);
    bot->m_movementInfo.jump.zspeed = -vSpeed;
    bot->m_movementInfo.jump.cosAngle = vcos;
    bot->m_movementInfo.jump.sinAngle = vsin;
    bot->m_movementInfo.jump.xyspeed = slowJump ? 0.f : hSpeed;

    sLog.outDetail("%s: Jump x: %f, y: %f, z: %f, time: %f, dist: %f, inPlace: %u, landTime: %u, curTime: %u", bot->GetName(), landing.getX(), landing.getY(), landing.getZ(), timeToLand, distanceToLand, jumpInPlace, jumpTime, curTime);

    // send jump packet
#ifdef MANGOSBOT_ZERO
    bot->m_movementInfo.AddMovementFlag(MOVEFLAG_JUMPING);
#else
    bot->m_movementInfo.AddMovementFlag(MOVEFLAG_FALLING);
#endif
    
    // client doesn't seem to show proper bigger jump with faster than real speeds
    if (vSpeed > (7.96f * 1.3f) || hSpeed > (bot->GetSpeed(MOVE_RUN) * 1.3f))
    {
        WorldPacket data(MSG_MOVE_KNOCK_BACK);
        data << bot->GetMover()->GetPackGUID();
        data << bot->m_movementInfo;
        data << bot->m_movementInfo.jump.cosAngle;
        data << bot->m_movementInfo.jump.sinAngle;
        data << bot->m_movementInfo.jump.xyspeed;
        data << bot->m_movementInfo.jump.zspeed;
        bot->GetMover()->SendMessageToSetExcept(data, bot);
    }
    else
    {
        WorldPacket jump(MSG_MOVE_JUMP);
        // write packet info
#ifdef MANGOSBOT_TWO
        jump << bot->GetObjectGuid().WriteAsPacked();
#endif
        jump << bot->m_movementInfo;
        ai->QueuePacket(jump);
    }

    // send move packet after jump
    if (!jumpInPlace && slowJump)
    {
        bot->m_movementInfo.AddMovementFlag(jumpBackward ? MOVEFLAG_BACKWARD : MOVEFLAG_FORWARD);
        bot->m_movementInfo.jump.xyspeed = hSpeed;
        WorldPacket move(jumpBackward ? MSG_MOVE_START_BACKWARD : MSG_MOVE_START_FORWARD);
        // write packet info
#ifdef MANGOSBOT_TWO
        move << bot->GetObjectGuid().WriteAsPacked();
#endif
        move << bot->m_movementInfo;
        ai->QueuePacket(move);
    }

    // change position to highest position
    // todo add in between points to avoid mobs instant aggro before landing
    bot->Relocate(highestPoint.getX(), highestPoint.getY(), highestPoint.getZ());

    if (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
    {
        std::string text = "Jump: cos: " + std::to_string(vcos) + " sin: " + std::to_string(vsin) + " distance: " + std::to_string(distanceToLand) + " speed: " + std::to_string(hSpeed);
        bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    }

    return true;
}

WorldPosition JumpAction::GetPossibleJumpStartFor(const WorldPosition& src, const WorldPosition& dest, WorldPosition& possibleLanding, Unit* jumper, float &requiredSpeed, float distanceTo, float distanceFrom)
{
    if (!src || !dest)
        return WorldPosition();

    if (src.getMapId() != dest.getMapId())
        return WorldPosition();

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return WorldPosition();

    float jumpAngle;
    // try jump from where at
    if (CanJumpTo(src, dest, possibleLanding, jumpAngle, jumper, bot->GetSpeed(MOVE_WALK), distanceFrom))
    {
        requiredSpeed = bot->GetSpeed(MOVE_WALK);
        return src;
    }
    else
    {
        // try slow jump
        if (CanJumpTo(src, dest, possibleLanding, jumpAngle, jumper, jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed, distanceFrom))
        {
            requiredSpeed = jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed;
            return src;
        }
    }

    // try find a closer point
    std::vector<WorldPosition> path = dest.getPathStepFrom(src, jumper);

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
        if (CanJumpTo(p, dest, possibleLanding, jumpAngle, jumper, bot->GetSpeed(MOVE_WALK), distanceFrom))
        {
            requiredSpeed = bot->GetSpeed(MOVE_WALK);
            return p;
        }
        else
        {
            // try fast jump
            if (CanJumpTo(p, dest, possibleLanding, jumpAngle, jumper, jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed, distanceFrom))
            {
                requiredSpeed = jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed;
                return p;
            }
            else
                continue;
        }
    }

    sLog.outDetail("%s: GetPossibleJumpStartFor Failed to find jump point!", jumper->GetName());
    return WorldPosition();
}

WorldPosition JumpAction::GetPossibleJumpStartForInRange(const WorldPosition& src, const WorldPosition& dest, WorldPosition& possibleLanding, Unit* jumper, float& requiredSpeed, float distanceTo, float distanceFrom)
{
    if (!src || !dest)
        return WorldPosition();

    if (src.getMapId() != dest.getMapId())
        return WorldPosition();

    if (src.fDist(dest) > sPlayerbotAIConfig.sightDistance)
        return WorldPosition();

    float jumpAngle;
    // try jump from where at
    if (CanJumpTo(src, dest, possibleLanding, jumpAngle, jumper, jumper->GetSpeed(MOVE_WALK), distanceFrom))
    {
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
        {
            jumper->SummonCreature(VISUAL_WAYPOINT, src.getX(), src.getY(), src.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000);
        }
        requiredSpeed = jumper->GetSpeed(MOVE_WALK);
        return src;
    }
    else
    {
        // try slow jump
        if (CanJumpTo(src, dest, possibleLanding, jumpAngle, jumper, jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed, distanceFrom))
        {
            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            {
                jumper->SummonCreature(VISUAL_WAYPOINT, src.getX(), src.getY(), src.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000);
            }
            requiredSpeed = jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed;
            return src;
        }
    }

    // try find a point in range to jump from

    // some limitations
    /*if (range > 40.0f)
        range = 40.0f;
    if (range < 0.1f)
        range = 5.0f;*/

    std::vector<WorldPosition> jumpPoints;

    float gx, gy, gz;
    gx = src.getX();
    gy = src.getY();
    gz = src.getZ();

    uint32 tries = 1;
    uint32 successes = 0;
    uint32 attempts = 0;
    uint32 startTime = WorldTimer::getMSTime();
    for (; tries < 500; ++tries)
    {
        gx = src.getX();
        gy = src.getY();
        gz = src.getZ();
#ifndef MANGOSBOT_TWO  
        if (jumper->GetMap()->GetReachableRandomPointOnGround(gx, gy, gz, distanceTo))
#else
        if (jumper->GetMap()->GetReachableRandomPointOnGround(bot->GetPhaseMask(), gx, gy, gz, distanceTo))
#endif
        {
            WorldPosition p(jumper->GetMapId(), gx, gy, gz);
            ++attempts;
            if (attempts >= 100)
                break;

            // point is not reachable
            if (!CanWalkTo(src, p, jumper))
                continue;

            if (CanJumpTo(p, dest, possibleLanding, jumpAngle, jumper, jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed, distanceFrom))
            {
                requiredSpeed = jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed;
                jumpPoints.push_back(p);
            }
            else
                continue;

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            {
                jumper->SummonCreature(VISUAL_WAYPOINT, gx, gy, gz, 0, TEMPSPAWN_TIMED_DESPAWN, 10000);
            }
            ++successes;
            if (successes >= 10)
                break;
        }
    }

    if (!jumpPoints.empty())
    {
        WorldPosition closest = src.closestSq(jumpPoints);
        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
        {
            Creature* wpCreature = bot->SummonCreature(15631, closest.getX(), closest.getY(), closest.getZ(), closest.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
            wpCreature->SetObjectScale(0.2f);
        }

        // recalculate to get landing and angle
        CanJumpTo(closest, dest, possibleLanding, jumpAngle, jumper, jumper->GetSpeedRate(MOVE_RUN) * sPlayerbotAIConfig.jumpHSpeed, distanceFrom);
        return closest;
    }

    sLog.outDetail("%s: GetPossibleJumpStartFor Failed to find jump point!", jumper->GetName());
    return WorldPosition();
}

