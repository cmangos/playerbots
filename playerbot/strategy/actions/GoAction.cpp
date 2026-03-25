
#include "playerbot/playerbot.h"
#include "GoAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/Formations.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/TravelMgr.h"
#include "MotionGenerators/PathFinder.h"
#include "ChooseTravelTargetAction.h"
#include "playerbot/TravelMgr.h"
#include "TellLosAction.h"
#include "Entities/Transports.h"

using namespace ai;

constexpr std::string_view LOS_GOS_PARAM = "los gos";

std::vector<std::string> split(const std::string& s, char delim);
char* strstri(const char* haystack, const char* needle);

bool GoAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    std::string param = event.getParam();

    if (param.empty())
    {
       param = getQualifier();
    }

    if (param == "?")
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        Map2ZoneCoordinates(x, y, bot->GetZoneId());
        std::ostringstream out;
        out << "I am at " << x << "," << y;
        ai->TellPlayer(requester, out.str());
        return true;
    }

    if (name == "where" ||  param.find("where") != std::string::npos)
    {
        return TellWhereToGo(param, requester);
    }
    if (param.find("how") != std::string::npos && param.size() > 4)
    {
        std::string destination = param.substr(4);

        DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, destination);
        if (dests.empty())
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }

        WorldPosition botPos(bot);

        TravelDestination* dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });

        if (!dest)
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }
        return TellHowToGo(dest, requester);
    }
    std::map<std::string, int> goTos;
    goTos.emplace(std::pair("zone", 5));
    goTos.emplace(std::pair("quest", 6));
    goTos.emplace(std::pair("npc", 4));
    goTos.emplace(std::pair("mob", 4));
    goTos.emplace(std::pair("boss", 5));
    goTos.emplace(std::pair("to", 3));
    for (const auto& option : goTos)
    {
        if (param.find(option.first) == 0 && param.size() > option.second)
        {             
            std::string destination = param.substr(option.second);
            DestinationList dests;
            TravelDestination* dest = nullptr;
            if (option.first == "to")
            {
                dests = ChooseTravelTargetAction::FindDestination(bot, destination);
            }
            else
            {
                dests = ChooseTravelTargetAction::FindDestination(bot, destination, option.first == "zone", option.first == "npc", option.first == "quest", option.first == "mob", option.first == "boss");
            }

            if (dests.empty())
            {
                ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
                return false;
            }

            WorldPosition botPos(bot);

            dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });


            if (!dest)
            {
                ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
                return false;
            }

            if (LeaderAlreadyTraveling(dest))
                return false;

            if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT) || (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || (ai->HasStrategy("wander", BotState::BOT_STATE_NON_COMBAT)) && ai->GetMaster() && !ai->IsSelfMaster()))
                return TellHowToGo(dest, requester);

            if (bot->IsTaxiFlying())
                ai->TellPlayerNoFacing(requester, "I am currently on a taxi.");

            return TravelTo(dest, requester);

        }
    }

    if (bot->IsTaxiFlying())
        ai->TellPlayerNoFacing(requester, "I am currently on a taxi.");

    if (param.find("travel") != std::string::npos && param.size()> 7)
    {
        std::string destination = param.substr(7);

        DestinationList dests;
        TravelDestination* dest = nullptr;

        dests = ChooseTravelTargetAction::FindDestination(bot, destination);

        if (dests.empty())
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }

        WorldPosition botPos(bot);

        dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });

        return TravelTo(dest, requester);
    }

    if (MoveToGo(param, requester))
        return true;

    if (MoveToUnit(param, requester))
        return true;

    if (MoveToGps(param, requester))
        return true;


    if (MoveToMapGps(param, requester))
        return true;

    return MoveToPosition(param, requester);

    ai->TellPlayer(requester, "Whisper 'go x,y', 'go [game object]', 'go unit' or 'go position' and I will go there." + ChatHelper::formatValue("help", "action:go", "go help") + " for more information.");
    return false;
}

inline void TellPosition(PlayerbotAI* ai, Player* requester)
{
    Player* bot = ai->GetBot();

    std::ostringstream out;

    if (bot->IsTaxiFlying())
    {
        out << "On a flight path";
        const Taxi::Map tMap = bot->GetTaxiPathSpline();
        if (!tMap.empty())
        {
            auto tEnd = tMap.back();
            WorldPosition taxiEnd(tEnd->mapid, tEnd->x, tEnd->y, tEnd->z);
            std::string endArea = taxiEnd.getAreaName();

            if (!endArea.empty())
                out << " to " << endArea;
        }
    }
    else if (bot->GetTransport())
    {
        GenericTransport* transport = bot->GetTransport();

        GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(transport->GetEntry());

        std::string transportName = transport->GetName();
        if (transportName.empty())
            transportName = data->name;

        out << "On transport";

        if (!transportName.empty())
            out << " " << transportName;
    }
    else
    {
        WorldPosition botPos(bot);

        if (requester != bot && botPos.distance(requester) < 60.0f)
        {
            out << uint32(botPos.distance(requester)) << " yards " << ChatHelper::formatAngle(WorldPosition(requester).getAngleTo(bot)) << " from you.";
        }
        else if (!WorldPosition(bot).getAreaName().empty())
            out << "In " << WorldPosition(bot).getAreaName();
        else
            out << "In " << bot->GetMap()->GetMapName();
    }

    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);
}

inline bool TellStuck(PlayerbotAI* ai, Player* requester)
{
    Player* bot = ai->GetBot();
    AiObjectContext* context = ai->GetAiObjectContext();

    std::ostringstream out;

    if (ai->HasActivePlayerMaster())
        return false;

    if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
        return false;

    if (!ai->AllowActivity(ALL_ACTIVITY))
    {
        RESET_AI_VALUE(WorldPosition, "current position");
        return false;
    }

    //Todo unify code here with code in stucktrigger

    if (ai->GetState() == BotState::BOT_STATE_COMBAT)
    {
        uint32 timeSinceCombatChange = AI_VALUE2(uint32, "time since last change", "combat::self target");

        if (timeSinceCombatChange > 2 * MINUTE)
        {
            out << "Stuck in combat for ";
            out << uint32(timeSinceCombatChange / MINUTE);
            out << " minutes";
            ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

            return true;
        }

        if (bot->duel && bot->duel->startTime - time(0) > 5 * MINUTE)
        {
            out << "Stuck in a dual for ";
            out << uint32((bot->duel->startTime - time(0)) / MINUTE);
            out << " minutes";
            ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

            return true;
        }
    }

    uint32 timeSinceLastMove = AI_VALUE2(uint32, "time since last change", "current position");

    if (timeSinceLastMove > 3 * MINUTE)
    {
        out << "Stuck in the same place for ";
        out << uint32(timeSinceLastMove / MINUTE);
        out << " minutes";
        ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

        return true;
    }

    uint32 distanceMoved = AI_VALUE2(uint32, "distance moved since", 5 * MINUTE);

    if (distanceMoved > 0 && distanceMoved < 50.0f)
    {
        out << "Haven't moved over 50y in the last 5 minutes";
        ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

        return true;
    }

    return false;
}

inline bool TellGrouped(PlayerbotAI* ai, Player* requester)
{
    Player* bot = ai->GetBot();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!bot->GetGroup())
        return false;

    std::ostringstream out;

    if (bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
    {
        out << "Leading a";
        if (bot->GetGroup()->IsRaidGroup())
            out << " raid";
        out << " group of ";
        out << bot->GetGroup()->GetMembersCount();

        ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

        return false;
    }

    if (ai->HasStrategy("free", BotState::BOT_STATE_NON_COMBAT))
        return false;

    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) ||
        ai->HasStrategy("wander", BotState::BOT_STATE_NON_COMBAT))
        out << "Following ";
    else
        out << "Grouped with ";

    out << ai->GetGroupMaster()->GetName();
    out << " at ";
    out << uint32(WorldPosition(bot).distance(ai->GetGroupMaster()));
    out << "y";

    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);

    return true;
}

inline void TellCombat(PlayerbotAI* ai, Player* requester)
{
    WorldPosition botPos(ai->GetBot());
    AiObjectContext* context = ai->GetAiObjectContext();

    std::ostringstream out;

    std::list<ObjectGuid> targets = AI_VALUE_LAZY(std::list<ObjectGuid>, "possible attack targets");

    uint32 nearTargets = 0;
    std::string nearestTarget = "";
    float nearTargetDistance = 999;
    for (auto& target : targets)
    {
        if (!target.IsCreature())
            continue;

        Unit* unit = ai->GetUnit(target);
        if (!unit)
            continue;

        WorldPosition unitPosition(unit);

        float dist = botPos.sqDistance2d(unitPosition);

        if (dist < nearTargetDistance)
        {
            nearTargets++;
            nearTargetDistance = dist;
            nearestTarget = ChatHelper::formatWorldobject(unit);
            continue;
        }

        if (dist > 10.0f)
            continue;

        nearTargets++;
    }

    if (!nearTargets)
    {
        out << "In combat without nearby enemies";
    }
    else
    {
        out << "Fighting " << nearestTarget;
        if (nearTargets > 1)
            out << " with " << nearTargets << " others nearby";
    }

    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);
}

inline void TellDead(PlayerbotAI* ai, Player* requester)
{
    Player* bot = ai->GetBot();
    AiObjectContext* context = ai->GetAiObjectContext();

    std::string botArea = WorldPosition(bot).getAreaName();

    std::ostringstream out;

    out << "Dead";

    Corpse* corpse = bot->GetCorpse();

    if (!corpse)
    {
        out << " waiting to release";
    }
    else
    {
        out << " for " << (time(nullptr)  - corpse->GetGhostTime()) << "s";


        if (AI_VALUE(bool, "should spirit healer"))
        {
            GuidPosition grave = AI_VALUE(GuidPosition, "best graveyard");

            std::string graveArea = grave.getAreaName();

            out << " walking to graveyard ";

            if (graveArea == botArea)
            {                
                out << WorldPosition(bot).distance(grave) << "y away";
            }
            else
            {
                out << " in " << graveArea;
            }
        }
        else
        {
            out << " walking to corpse " << uint32(WorldPosition(bot).distance(corpse)) << "y away";
        }
    }

    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);
}

inline void TellNonCombat(PlayerbotAI* ai, Player* requester)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    GuidPosition botPos(ai->GetBot());

    std::ostringstream out;

    if (AI_VALUE(bool, "travel target traveling"))
        return;

    GuidPosition rpgTarget = AI_VALUE(GuidPosition, "rpg target");

    if (!rpgTarget)
        return;

    if (botPos.distance(rpgTarget) > 10.0f)
        out << "Walking " << uint32(botPos.distance(rpgTarget)) << "y for ";

    Action* action = ai->GetAiObjectContext()->GetAction("choose rpg target");

    if (action)
    {
        ChooseRpgTargetAction* targetAction = dynamic_cast<ChooseRpgTargetAction*>(action);

        std::string reason = targetAction->GetRpgActionReason(rpgTarget);

        if (reason.empty())
        {
            out << "being near ";
        }
        else
        {
            if (out.str().empty())
                reason[0] = std::toupper(static_cast<unsigned char>(reason[0]));

            out << reason << " ";
        }
    }
    else
        out << "near" ;

    WorldObject* object = rpgTarget.GetWorldObject(ai->GetBot()->GetInstanceId());

    if (object)
        out << ChatHelper::formatWorldobject(object);
    else
        out << ChatHelper::formatGuidPosition(rpgTarget, botPos);

    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK, false);
}

bool GoAction::TellWhereToGo(std::string& param, Player* requester) const
{
    TellPosition(ai, requester);

    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

    if (!TellGrouped(ai, requester) && travelTarget->IsActive())
        ChooseTravelTargetAction::ReportTravelTarget(bot, requester, travelTarget, nullptr);

    TellStuck(ai, requester);

    switch (ai->GetState())
    {
        case BotState::BOT_STATE_COMBAT:
            TellCombat(ai, requester);
            break;
        case BotState::BOT_STATE_DEAD:
            TellDead(ai, requester);
            break;
        case BotState::BOT_STATE_NON_COMBAT:
            TellNonCombat(ai, requester);
            break;
    }

    return true;
}

bool GoAction::LeaderAlreadyTraveling(TravelDestination* dest) const
{
    return AI_VALUE(bool, "travel target traveling");
}

bool GoAction::TellHowToGo(TravelDestination* dest, Player* requester) const
{
    WorldPosition botPos = WorldPosition(bot);
    WorldPosition* point = dest->GetClosestPoint(botPos);

    if (!point)
    {
        ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + dest->GetTitle());
        return false;
    }

    std::vector<WorldPosition> beginPath, endPath;
    TravelNodeRoute route = sTravelNodeMap.getRoute(botPos, *point, beginPath, bot);

    if (route.isEmpty())
    {
        ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + dest->GetTitle());
        return false;
    }

    WorldPosition poi = *point;
    float pointAngle = botPos.getAngleTo(poi);

    if (botPos.distance(poi) > sPlayerbotAIConfig.reactDistance || route.getNodes().size() == 1)
    {
        poi = botPos;
        TravelNode* nearNode = nullptr;
        TravelNode* nextNode = nullptr;

        nextNode = nearNode = route.getNodes().front();

        for (auto node : route.getNodes())
        {
            if (node == nearNode)
                continue;

            TravelNodePath* travelPath = nextNode->getPathTo(node);

            std::vector<WorldPosition> path = travelPath->getPath();

            for (auto& p : path)
            {
                if (p.distance(botPos) > sPlayerbotAIConfig.reactDistance)
                    continue;

                if (p.distance(*point) > poi.distance(*point))
                    continue;

                poi = p;
                nextNode = node;
            }
        }

        if (nearNode)
            ai->TellPlayerNoFacing(requester, "We are now near " + nearNode->getName() + ".");

        ai->TellPlayerNoFacing(requester, "if we want to travel to " + dest->GetTitle());
        if (nextNode->getPosition()->getAreaName(true, true) != botPos.getAreaName(true, true))
            ai->TellPlayerNoFacing(requester, "we should head to " + nextNode->getName() + " in " + nextNode->getPosition()->getAreaName(true, true));
        else
            ai->TellPlayerNoFacing(requester, "we should head to " + nextNode->getName());

        pointAngle = botPos.getAngleTo(poi);
    }
    else
        ai->TellPlayerNoFacing(requester, "We are near " + dest->GetTitle());

    ai->TellPlayer(requester, "it is " + std::to_string(uint32(round(poi.distance(botPos)))) + " yards to the " + ChatHelper::formatAngle(pointAngle));
    sServerFacade.SetFacingTo(bot, pointAngle, true);
    bot->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
    ai->Poi(poi.getX(), poi.getY());

    return true;
}

bool GoAction::TravelTo(TravelDestination* dest, Player* requester) const
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");
    WorldPosition botPos = WorldPosition(bot);
    if (dest)
    {
        WorldPosition* point = dest->GetClosestPoint(botPos);

        if (!point)
            return false;

        target->SetTarget(dest, point);
        target->SetForced(true);
        target->SetConditions({ "not::manual bool::is travel refresh"});

        std::ostringstream out; out << "Traveling to " << dest->GetTitle();
        ai->TellPlayerNoFacing(requester, out.str());

        if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT))
            ai->ChangeStrategy("+travel once", BotState::BOT_STATE_NON_COMBAT);

        return true;
    }
    else
    {
        sTravelMgr.SetNullTravelTarget(target);
        target->SetForced(false);
        return false;
    }
}

bool GoAction::MoveToGo(std::string& param, Player* requester)
{
   auto loopthroughobjects = [&](const std::list<GameObject*>& gos) -> bool
      {
         for (GameObject* go : gos)
         {
            if (go && sServerFacade.isSpawned(go))
            {
               if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, go), sPlayerbotAIConfig.reactDistance))
               {
                  ai->TellError(requester, "It is too far away");
                  return false;
               }

               std::ostringstream out; out << "Moving to " << ChatHelper::formatGameobject(go);
               ai->TellPlayerNoFacing(requester, out.str());

               WorldPosition pos;
               go->GetPosition(pos);
               const float angle = GetFollowAngle();
               const float distance = INTERACTION_DISTANCE;
               pos += WorldPosition(0, cos(angle)* distance, sin(angle)* distance, 0.5f);

               UpdateStrategyPosition(pos);

               return MoveTo(pos);
            }
         }

         return false;
      };

   std::list<ObjectGuid> goguids = ChatHelper::parseGameobjects(param);

   if (goguids.size())
   {
      return loopthroughobjects(TellLosAction::GoGuidListToObjList(ai, goguids));
   }

   if (param.find(LOS_GOS_PARAM) == 0)
   {
      std::vector<LosModifierStruct> mods = TellLosAction::ParseLosModifiers(param.substr(LOS_GOS_PARAM.size()));

      return loopthroughobjects(TellLosAction::FilterGameObjects(requester, TellLosAction::GoGuidListToObjList(ai, AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los")), mods));
   }

    return false;
}

bool GoAction::MoveToUnit(std::string& param, Player* requester)
{
    std::list<ObjectGuid> units;
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    units.insert(units.end(), npcs.begin(), npcs.end());
    std::list<ObjectGuid> players = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");
    units.insert(units.end(), players.begin(), players.end());
    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit && strstri(unit->GetName(), param.c_str()))
        {
            std::ostringstream out; out << "Moving to " << unit->GetName();
            ai->TellPlayerNoFacing(requester, out.str());
            return MoveNear(bot->GetMapId(), unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ() + 0.5f, ai->GetRange("follow"));
        }
    }

    return false;
}

bool GoAction::MoveToGps(std::string& param, Player* requester)
{
    if (param.find(";") != std::string::npos)
    {
        std::vector<std::string> coords = split(param, ';');
        float x = atof(coords[0].c_str());
        float y = atof(coords[1].c_str());
        float z;
        if (coords.size() > 2)
            z = atof(coords[2].c_str());
        else
            z = bot->GetPositionZ();

        if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
        {
            PathFinder path(bot);

            path.calculate(x, y, z, false);

            Vector3 end = path.getEndPosition();
            Vector3 aend = path.getActualEndPosition();

            PointsArray& points = path.getPath();
            PathType type = path.getPathType();

            std::ostringstream out;

            out << x << ";" << y << ";" << z << " =";

            out << "path is: ";

            out << type;

            out << " of length ";

            out << points.size();

            out << " with offset ";

            out << (end - aend).length();


            for (auto i : points)
            {
                CreateWp(bot, i.x, i.y, i.z, 0.0, 11144);
            }

            ai->TellPlayer(requester, out);
        }

        if (bot->IsWithinLOS(x, y, z, true))
            return MoveNear(bot->GetMapId(), x, y, z, 0);
        else
            return MoveTo(bot->GetMapId(), x, y, z, false, false);
    }
    return false;
}

bool GoAction::MoveToMapGps(std::string& param, Player* requester)
{
    if (param.find(",") != std::string::npos)
    {
        std::vector<std::string> coords = split(param, ',');
        float x = atof(coords[0].c_str());
        float y = atof(coords[1].c_str());

        Zone2MapCoordinates(x, y, bot->GetZoneId());

        Map* map = bot->GetMap();
        float z = bot->GetPositionZ();

        if (!WorldPosition(bot->GetMapId(), x, y, z).isValid())
            return false;

        bot->UpdateAllowedPositionZ(x, y, z);

        if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, x, y), sPlayerbotAIConfig.reactDistance))
        {
            ai->TellPlayer(requester, BOT_TEXT("error_far"));
            return false;
        }

        const TerrainInfo* terrain = map->GetTerrain();
        if (terrain->IsUnderWater(x, y, z) || terrain->IsInWater(x, y, z))
        {
            ai->TellError(requester, BOT_TEXT("error_water"));
            return false;
        }

#ifdef MANGOSBOT_TWO
        float ground = map->GetHeight(bot->GetPhaseMask(), x, y, z + 0.5f);
#else
        float ground = map->GetHeight(x, y, z + 0.5f);
#endif
        if (ground <= INVALID_HEIGHT)
        {
            ai->TellError(requester, BOT_TEXT("error_cant_go"));
            return false;
        }

        float x1 = x, y1 = y;
        Map2ZoneCoordinates(x1, y1, bot->GetZoneId());
        std::ostringstream out; out << "Moving to " << x1 << "," << y1;
        ai->TellPlayerNoFacing(requester, out.str());
        return MoveNear(bot->GetMapId(), x, y, z + 0.5f, ai->GetRange("follow"));
    }
    return false;
}

bool GoAction::MoveToPosition(std::string& param, Player* requester)
{
    PositionEntry pos = context->GetValue<PositionMap&>("position")->Get()[param];
    if (pos.isSet())
    {
        if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, pos.x, pos.y), sPlayerbotAIConfig.reactDistance))
        {
            ai->TellError(requester, BOT_TEXT("error_far"));
            return false;
        }

        std::ostringstream out; out << "Moving to position " << param;
        ai->TellPlayerNoFacing(requester, out.str());
        return MoveNear(bot->GetMapId(), pos.x, pos.y, pos.z + 0.5f, ai->GetRange("follow"));
    }
    return false;
}

void GoAction::UpdateStrategyPosition(const WorldPosition& position)
{
   if (ai->HasStrategy("stay", ai->GetState()))
   {
      PositionMap& posMap = AI_VALUE(PositionMap&, "position");
      PositionEntry& stayPosition = posMap["stay"];

      stayPosition.Set(position.getX(), position.getY(), position.getZ(), position.getMapId());
      posMap["stay"] = stayPosition;
      posMap["return"] = stayPosition;
   }
   else if (ai->HasStrategy("guard", ai->GetState()))
   {
      PositionMap& posMap = AI_VALUE(PositionMap&, "position");
      PositionEntry& guardPosition = posMap["guard"];

      guardPosition.Set(position.getX(), position.getY(), position.getZ(), position.getMapId());
      posMap["guard"] = guardPosition;
      posMap["return"] = guardPosition;
   }
}
