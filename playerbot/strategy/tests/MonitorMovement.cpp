#include "playerbot/playerbot.h"
#include "MonitorMovement.h"
#include "playerbot/WorldPosition.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestRegistry.h"
#include "playerbot/TravelNode.h"

using namespace ai;

bool MonitorMovementDistance::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    std::string locPart;
    std::string outcomePart;
    std::string parseMessage;
    if (TrySplitOnce(monitorStr, "=>", locPart, outcomePart, parseMessage, GetName(), true) != TestResult::PASS)
        return false;

    GuidPosition loc;
    char op = 0;
    std::string valueStr;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
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

    const float dist = bot->GetDistance(loc.coord_x, loc.coord_y, loc.coord_z);
    if (op == '<')
        return dist < threshold;

    return dist > threshold;

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
        return true;

    std::string currentMapName = botPos.getMapEntry()->name[0];

    if (currentMapName != wantMapName)
        return true;

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

    if (botpos.isUnderground())
        ctx.undergroundCount++;
    else if (ctx.undergroundCount > 0)
        ctx.undergroundCount--;

    return ctx.undergroundCount > 10;
}

bool MonitorMovementCanReachNodes::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    if (!bot->IsInWorld())
        return false;

    WorldPosition pos = WorldPosition(bot);
    std::vector<TravelNode*> startNodes = sTravelNodeMap.getNodes(pos);

    for (uint32 i = 0; i < std::min(5,int(startNodes.size())); i++)
    {
        WorldPosition nodePos = *startNodes[i]->getPosition();
        if (nodePos.isPathTo(pos.getPathTo(nodePos, bot),1.0f))
            return true;
    }

    return false;
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

    char op = 0;
    std::string valueStr;
    if (TryParseComparisonValue(monitorStr, op, valueStr, parseMessage, GetName()) != TestResult::PASS)
        return false;

    float threshold = 0.0f;
    if (TryParseFloatStrict(valueStr, threshold, parseMessage, GetName()) != TestResult::PASS)
        return false;

    if (op == '<')
        return dist < threshold;

    return dist > threshold;
}