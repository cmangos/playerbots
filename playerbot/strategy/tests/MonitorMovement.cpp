#include "playerbot/playerbot.h"
#include "MonitorMovement.h"
#include "playerbot/WorldPosition.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestRegistry.h"
#include "playerbot/TravelNode.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/strategy/values/LastMovementValue.h"

#include <cmath>
#include <cstdlib>

using namespace ai;

namespace
{
    bool ParseOperatorNumber(const std::string& clause, std::string& op, float& number)
    {
        size_t index = 0;
        op.clear();

        while (index < clause.size() && (clause[index] == '<' || clause[index] == '>' || clause[index] == '=' || clause[index] == '!'))
            op += clause[index++];

        std::string rest = clause.substr(index);
        const size_t begin = rest.find_first_not_of(" \t");

        if (begin == std::string::npos)
            return false;

        rest = rest.substr(begin);
        const size_t end = rest.find_last_not_of(" \t");
        rest = rest.substr(0, end + 1);

        char* endPtr = nullptr;
        number = (float)std::strtod(rest.c_str(), &endPtr);

        return endPtr != rest.c_str() && *endPtr == 0;
    }

    bool CompareNumber(float actual, const std::string& op, float wanted)
    {
        if (op == "<")
            return actual < wanted;

        if (op == "<=")
            return actual <= wanted;

        if (op == ">")
            return actual > wanted;

        if (op == ">=")
            return actual >= wanted;

        if (op == "==" || op == "=")
            return std::abs(actual - wanted) < 0.001f;

        if (op == "!=")
            return std::abs(actual - wanted) >= 0.001f;

        return false;
    }
}

bool MonitorMovementDistance::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    return DistanceMet(monitorStr, bot);
}

bool MonitorMovementDistance::DistanceMet(const std::string& monitorStr, Player* bot) const
{
    std::string locPart;
    std::string outcomePart;
    std::string parseMessage;
    if (TrySplitOnce(monitorStr, "=>", locPart, outcomePart, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    GuidPosition loc;
    std::string valueName;
    std::string op;
    std::string valueStr;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    float threshold = 0.0f;
    if (TryParseFloatStrict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    const size_t opPos = locPart.find(op);
    if (opPos == std::string::npos)
        return false;

    const std::string name = locPart.substr(0, opPos > 0 ? opPos - 1 : 0);
    if (!TestRegistry::ParseLocation(name, loc))
        return false;

    float dist = bot->GetDistance(loc.coord_x, loc.coord_y, loc.coord_z);

    const bool straightMeets = (op == "<") ? (dist < threshold) : (dist > threshold);
    if (straightMeets && bot->GetMapId() == loc.mapid)
    {
        TravelPath path = TravelNodeMap::getFullPath(loc, WorldPosition(bot), bot);
        if (!path.empty())
        {
            std::vector<WorldPosition> points = path.getPointPath();
            float pathLength = 0.0f;
            for (size_t i = 1; i < points.size(); ++i)
                pathLength += points[i - 1].distance(points[i]);

            if (pathLength > dist)
                dist = pathLength;
        }
    }

    if (op == "<")
        return dist < threshold;

    return dist > threshold;

}

bool MonitorMovementArrived::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    const size_t splitPos = monitorStr.find("=>");
    std::string spec = splitPos == std::string::npos ? monitorStr : monitorStr.substr(0, splitPos);
    std::string outcome = splitPos == std::string::npos ? std::string() : monitorStr.substr(splitPos);

    const std::string rideTag = " rode ";
    const size_t ridePos = spec.find(rideTag);

    if (ridePos != std::string::npos)
    {
        std::string rideClause = spec.substr(ridePos + rideTag.size());
        spec = spec.substr(0, ridePos);

        std::string op;
        float wanted = 0.0f;

        if (!ParseOperatorNumber(rideClause, op, wanted))
            return false;

        auto it = ctx.transportRideMs.find(0);
        float rodeSeconds = it == ctx.transportRideMs.end() ? 0.0f : it->second / 1000.0f;

        if (!CompareNumber(rodeSeconds, op, wanted))
            return false;
    }

    return DistanceMet(spec + outcome, bot);
}

bool MonitorMovementRodeTransport::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string leftSide, rightSide, parseMessage;
    if (TrySplitOnce(monitorStr, "=>", leftSide, rightSide, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    const size_t entryEnd = leftSide.find_first_of(" \t");
    if (entryEnd == std::string::npos)
        return false;

    std::string entryStr = leftSide.substr(0, entryEnd);
    std::string rideClause = leftSide.substr(entryEnd);

    uint32 entry = 0;

    if (entryStr != "any")
    {
        if (entryStr.empty() || entryStr.find_first_not_of("0123456789") != std::string::npos)
            return false;

        entry = (uint32)atol(entryStr.c_str());
    }

    std::string op;
    float wanted = 0.0f;

    if (!ParseOperatorNumber(rideClause, op, wanted))
        return false;

    auto it = ctx.transportRideMs.find(entry);
    float rodeSeconds = it == ctx.transportRideMs.end() ? 0.0f : it->second / 1000.0f;

    return CompareNumber(rodeSeconds, op, wanted);
}

bool MonitorMovementLeftTransport::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    return ctx.transportLeftAway;
}
bool MonitorNotOnMap::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string wantMapName, rightSide, parseMessage;
    if (TrySplitOnce(monitorStr, "=>", wantMapName, rightSide, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    if (!bot->IsAlive()) //Allow offmap corpse runs
        return false;

    WorldPosition botPos(bot);

    if (!botPos)
    {
        return true;
    }

    uint32 wantMapId = 0;
    bool wantMapIdKnown = false;

    std::string trimmed = wantMapName;
    const size_t begin = trimmed.find_first_not_of(" \t");
    if (begin != std::string::npos)
    {
        const size_t end = trimmed.find_last_not_of(" \t");
        trimmed = trimmed.substr(begin, end - begin + 1);

        if (!trimmed.empty() && trimmed.find_first_not_of("0123456789") == std::string::npos)
        {
            wantMapId = uint32(atol(trimmed.c_str()));
            wantMapIdKnown = true;
        }
    }

    if (!wantMapIdKnown)
    {
        GuidPosition loc;
        if (TestRegistry::ParseLocation(wantMapName, loc))
        {
            wantMapId = loc.getMapId();
            wantMapIdKnown = true;
        }
    }

    if (wantMapIdKnown)
    {
        if (botPos.getMapId() != wantMapId)
        {
            return true;
        }

        return false;
    }

    const MapEntry* entry = botPos.getMapEntry();
    if (!entry || !entry->name[0])
        return false;

    std::string currentMapName = entry->name[0];
    currentMapName.erase(std::remove_if(currentMapName.begin(), currentMapName.end(), ::isspace), currentMapName.end());
    std::transform(currentMapName.begin(), currentMapName.end(), currentMapName.begin(), ::tolower);

    std::string wantName = wantMapName;
    wantName.erase(std::remove_if(wantName.begin(), wantName.end(), ::isspace), wantName.end());
    std::transform(wantName.begin(), wantName.end(), wantName.begin(), ::tolower);

    if (currentMapName != wantName)
    {
        return true;
    }

    return false;
}

bool MonitorMovementUnderground::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    if (!bot->IsInWorld())
        return false;

    WorldPosition botpos(bot);

    if (!botpos) //empty position is underground.
        return true;

    botpos += WorldPosition(0, 0, 0, 0.5); //Give some leeway.

    float groundZ = botpos.getMap(botpos.getFirstInstanceId())->GetHeight(botpos.coord_x, botpos.coord_y, botpos.coord_z + 0.5f, true);
    float maxZ = botpos.getTerrain()->GetWaterOrGroundLevel(botpos.coord_x, botpos.coord_y, botpos.coord_z + 0.5f, groundZ, true, 1.0f);

    if (maxZ <= INVALID_HEIGHT)
        return false;

    float depth = maxZ - botpos.coord_z;
    if (depth > 10.0f)
        ctx.undergroundCount++;
    else if (ctx.undergroundCount > 0)
        ctx.undergroundCount--;

    return ctx.undergroundCount > 10;
}

bool MonitorMovementCanNotReachNodes::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    if (!bot->IsInWorld())
        return false;

    WorldPosition pos = WorldPosition(bot);

    // On a transport (elevator platform, boat, zeppelin) the bot is off the navmesh by
    // definition - pathfinding from there is meaningless. Don't count those ticks.
    // Bots that walk onto a GO transport are not always registered passengers, so also
    // check the geometry directly (isOnTransport ray-tests the vmaps) and treat "well
    // above the static ground" (elevator mid-shaft) as riding.
    bool riding = bot->GetTransport() || pos.currentHeight() > 5.0f;
    if (!riding)
    {
        for (GenericTransport* transport : pos.getTransports())
        {
            if (pos.isOnTransport(transport))
            {
                riding = true;
                break;
            }
        }
    }

    if (riding)
    {
        if (ctx.cannotReachCount > 0)
            ctx.cannotReachCount--;
        return false;
    }

    std::vector<WorldPosition> startNodePositions;
    {
        std::shared_lock<std::shared_timed_mutex> nodeLock(sTravelNodeMap.m_nMapMtx);
        for (TravelNode* node : sTravelNodeMap.getNodes(pos))
        {
            if (startNodePositions.size() >= 5)
                break;
            startNodePositions.push_back(*node->getPosition());
        }
    }

    for (const WorldPosition& nodePos : startNodePositions)
    {
        std::vector<WorldPosition> path = pos.getPathTo(nodePos, bot);
        // 3 yd, not 1: dock/entry nodes sit on pier and platform edges where the mesh thins
        // out, and pathfinding stops 1-3 yd short of them. Within 3 yd the bot can walk the
        // rest, and the node-link structure takes over.
        if (nodePos.isPathTo(path, 3.0f))
        {
            if (ctx.cannotReachCount > 0)
                ctx.cannotReachCount--;
            return false;
        }
    }


    // Grace period: right after a teleport the grid/mmaps for the destination may not be
    // loaded yet and pathfinding comes back empty; fail only when it persists.
    ctx.cannotReachCount++;
    // Transient blocks are normal around transports: the platform occupies the pier, the bot
    // rides next to static structures the vmap ray-test can mistake for ground. Only fail after
    // a long persistence; the per-test timeout monitor stays the real backstop.
    return ctx.cannotReachCount > 120;
}

bool MonitorMovementSpeed::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    static std::map<ObjectGuid, WorldPosition> lastPositions;
    static std::map<ObjectGuid, uint32> lastTimes;
    static std::map<ObjectGuid, bool> lastOnTransport;
    static std::map<ObjectGuid, bool> lastOnTaxi;

    ObjectGuid guid = bot->GetObjectGuid();

    if (!lastPositions.count(guid))
    {
        lastPositions[guid] = WorldPosition(bot);
        lastTimes[guid] = WorldTimer::getMSTime();
        lastOnTransport[guid] = bot->GetTransport() != nullptr;
        lastOnTaxi[guid] = bot->IsTaxiDebug();
        return false;
    }

    WorldPosition lastPos = lastPositions[guid];
    uint32 lastTime = lastTimes[guid];
    bool wasOnTransport = lastOnTransport[guid];
    bool wasOnTaxi = lastOnTaxi[guid];

    uint32 now = WorldTimer::getMSTime();
    float dt = (now - lastTime) / 1000.0f;

    if (dt < 0.1f)
        return false;

    WorldPosition currentPos = WorldPosition(bot);

    if (wasOnTransport && bot->GetTransport())
    {
        lastPositions[guid] = currentPos;
        lastTimes[guid] = now;
        lastOnTransport[guid] = true;
        lastOnTaxi[guid] = false;
        return false;
    }

    if (wasOnTaxi && bot->IsTaxiDebug())
    {
        lastPositions[guid] = currentPos;
        lastTimes[guid] = now;
        lastOnTransport[guid] = false;
        lastOnTaxi[guid] = true;
        return false;
    }

    float distance = currentPos.distance(lastPos);

    float speed = distance / dt;

    float expectedSpeed;
    if (bot->IsMounted())
        expectedSpeed = bot->GetSpeedRate(MOVE_RUN) * 1.2f;
    else if (bot->IsInCombat())
        expectedSpeed = bot->GetSpeedRate(MOVE_RUN) * 1.4f;
    else
        expectedSpeed = bot->GetSpeedRate(MOVE_RUN);

    lastPositions[guid] = currentPos;
    lastTimes[guid] = now;
    lastOnTransport[guid] = bot->GetTransport() != nullptr;
    lastOnTaxi[guid] = bot->IsTaxiDebug();

    if (speed > expectedSpeed * 3.0f)
        return true;

    return false;
}

bool MonitorMovementSpawnDistance::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    if (ctx.spawnedBots.empty())
        return false;

    Player* spawned = sObjectMgr.GetPlayer(ctx.spawnedBots.front());
    if (!spawned)
        return false;

    std::string leftSide;
    std::string rightSide;
    std::string parseMessage;
    if (TrySplitOnce(monitorStr, "=>", leftSide, rightSide, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    float dist = bot->GetDistance(spawned);

    std::string valueName;
    std::string op;
    std::string valueStr;
    if (TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    float threshold = 0.0f;
    if (TryParseFloatStrict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == "<")
        return dist < threshold;

    return dist > threshold;
}