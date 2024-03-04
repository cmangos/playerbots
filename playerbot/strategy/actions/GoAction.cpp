
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

using namespace ai;

vector<string> split(const string& s, char delim);
char* strstri(const char* haystack, const char* needle);

bool GoAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    string param = event.getParam();
    if (param == "?")
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        Map2ZoneCoordinates(x, y, bot->GetZoneId());
        ostringstream out;
        out << "I am at " << x << "," << y;
        ai->TellPlayer(requester, out.str());
        return true;
    }

    if (param.find("where") != string::npos)
    {
        return TellWhereToGo(param, requester);
    }
    if (param.find("how") != string::npos && param.size() > 4)
    {
        string destination = param.substr(4);
        TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, destination);
        if (!dest)
        {
            ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
            return false;
        }
        return TellHowToGo(dest, requester);
    }
    map<string, int> goTos;
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
            string destination = param.substr(option.second);
            TravelDestination* dest = nullptr;
            if (option.first == "to")
            {
                dest = ChooseTravelTargetAction::FindDestination(bot, destination);
            }
            else
            {
                dest = ChooseTravelTargetAction::FindDestination(bot, destination, option.first == "zone", option.first == "npc", option.first == "quest", option.first == "mob", option.first == "boss");
            }

            if (!dest)
            {
                ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + destination);
                return false;
            }

            if (LeaderAlreadyTraveling(dest))
                return false;

            if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT) || (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && ai->GetMaster() && !ai->IsSelfMaster()))
                return TellHowToGo(dest, requester);

            return TravelTo(dest, requester);

        }
    }
    if (param.find("travel") != string::npos && param.size()> 7)
    {
        string destination = param.substr(7);

        TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, destination);

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

bool GoAction::TellWhereToGo(string& param, Player* requester) const
{
    string text;

    if (param.size() > 6)
        text = param.substr(6);

    ChooseTravelTargetAction* travelAction = new ChooseTravelTargetAction(ai);

    TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();

    target->setStatus(TravelStatus::TRAVEL_STATUS_EXPIRED);

    travelAction->getNewTarget(requester, target, target);

    if (!target->getDestination() || target->getDestination()->getTitle().empty())
    {
        ai->TellPlayerNoFacing(requester, "I have no place I want to go to.");
        return false;
    }

    string title = target->getDestination()->getTitle();

    if (title.find('[') != string::npos)
        title = title.substr(title.find("[") + 1, title.find("]") - title.find("[") - 1);


    TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, title);

    if (!dest)
        dest = target->getDestination();

    if (!dest)
    {
        ai->TellPlayerNoFacing(requester, "I have no place I want to go to");
        return false;
    }

    string link = ChatHelper::formatValue("command", "go to " + title, title, "FF00FFFF");

    ai->TellPlayerNoFacing(requester, "I would like to travel to " + link + "(" + target->getDestination()->getTitle() + ")");

    delete travelAction;
    return true;
}

bool GoAction::LeaderAlreadyTraveling(TravelDestination* dest) const
{
    if (!bot->GetGroup())
        return false;

    if (bot == ai->GetGroupMaster())
        return false;

    if (!ai->GetGroupMaster()->GetPlayerbotAI())
        return false;

    Player* player = ai->GetGroupMaster();
    TravelTarget* masterTarget = PAI_VALUE(TravelTarget*, "travel target");

    if (!masterTarget->getDestination())
        return false;

    if (masterTarget->getDestination() != dest)
        return false;

    return true;
}

bool GoAction::TellHowToGo(TravelDestination* dest, Player* requester) const
{
    WorldPosition botPos = WorldPosition(bot);
    WorldPosition* point = dest->nearestPoint(botPos);

    vector<WorldPosition> beginPath, endPath;
    TravelNodeRoute route = sTravelNodeMap.getRoute(botPos, *point, beginPath, bot);

    if (route.isEmpty())
    {
        ai->TellPlayerNoFacing(requester, "I don't know how to travel to " + dest->getTitle());
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

            vector<WorldPosition> path = travelPath->getPath();

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

        ai->TellPlayerNoFacing(requester, "if we want to travel to " + dest->getTitle());
        if (nextNode->getPosition()->getAreaName(true, true) != botPos.getAreaName(true, true))
            ai->TellPlayerNoFacing(requester, "we should head to " + nextNode->getName() + " in " + nextNode->getPosition()->getAreaName(true, true));
        else
            ai->TellPlayerNoFacing(requester, "we should head to " + nextNode->getName());

        pointAngle = botPos.getAngleTo(poi);
    }
    else
        ai->TellPlayerNoFacing(requester, "We are near " + dest->getTitle());

    ai->TellPlayer(requester, "it is " + to_string(uint32(round(poi.distance(botPos)))) + " yards to the " + ChatHelper::formatAngle(pointAngle));
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
        WorldPosition* point = dest->nearestPoint(botPos);

        if (!point)
            return false;

        target->setTarget(dest, point);
        target->setForced(true);

        ostringstream out; out << "Traveling to " << dest->getTitle();
        ai->TellPlayerNoFacing(requester, out.str());

        if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT))
            ai->ChangeStrategy("+travel once", BotState::BOT_STATE_NON_COMBAT);

        return true;
    }
    else
    {
        target->setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition);
        target->setForced(false);
        return false;
    }
}

bool GoAction::MoveToGo(string& param, Player* requester)
{
    list<ObjectGuid> gos = ChatHelper::parseGameobjects(param);
    if (gos.empty())
        return false;

    for (list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); ++i)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (go && sServerFacade.isSpawned(go))
        {
            if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, go), sPlayerbotAIConfig.reactDistance))
            {
                ai->TellError(requester, "It is too far away");
                return false;
            }

            ostringstream out; out << "Moving to " << ChatHelper::formatGameobject(go);
            ai->TellPlayerNoFacing(requester, out.str());
            return MoveNear(bot->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ() + 0.5f, ai->GetRange("follow"));
        }
    }
    return false;
}

bool GoAction::MoveToUnit(string& param, Player* requester)
{
    list<ObjectGuid> units;
    list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
    units.insert(units.end(), npcs.begin(), npcs.end());
    list<ObjectGuid> players = AI_VALUE(list<ObjectGuid>, "nearest friendly players");
    units.insert(units.end(), players.begin(), players.end());
    for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit && strstri(unit->GetName(), param.c_str()))
        {
            ostringstream out; out << "Moving to " << unit->GetName();
            ai->TellPlayerNoFacing(requester, out.str());
            return MoveNear(bot->GetMapId(), unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ() + 0.5f, ai->GetRange("follow"));
        }
    }

    return false;
}

bool GoAction::MoveToGps(string& param, Player* requester)
{
    if (param.find(";") != string::npos)
    {
        vector<string> coords = split(param, ';');
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

            ostringstream out;

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

bool GoAction::MoveToMapGps(string& param, Player* requester)
{
    if (param.find(",") != string::npos)
    {
        vector<string> coords = split(param, ',');
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
        ostringstream out; out << "Moving to " << x1 << "," << y1;
        ai->TellPlayerNoFacing(requester, out.str());
        return MoveNear(bot->GetMapId(), x, y, z + 0.5f, ai->GetRange("follow"));
    }
    return false;
}

bool GoAction::MoveToPosition(string& param, Player* requester)
{
    PositionEntry pos = context->GetValue<PositionMap&>("position")->Get()[param];
    if (pos.isSet())
    {
        if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, pos.x, pos.y), sPlayerbotAIConfig.reactDistance))
        {
            ai->TellError(requester, BOT_TEXT("error_far"));
            return false;
        }

        ostringstream out; out << "Moving to position " << param;
        ai->TellPlayerNoFacing(requester, out.str());
        return MoveNear(bot->GetMapId(), pos.x, pos.y, pos.z + 0.5f, ai->GetRange("follow"));
    }
    return false;
}