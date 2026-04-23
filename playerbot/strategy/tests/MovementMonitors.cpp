#include "playerbot/playerbot.h"
#include "MovementMonitors.h"
#include "playerbot/WorldPosition.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestRegistry.h"
#include "playerbot/TravelNode.h"

using namespace ai;

bool CheckDistanceMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    size_t arrowPos = monitorStr.find("=>");
    if (arrowPos == std::string::npos)
        return false;

    std::string locPart = monitorStr.substr(GetName().length() + 1,
                                       arrowPos - GetName().length());

    GuidPosition loc;
    size_t ltPos = locPart.find("<");
    size_t gtPos = locPart.find(">");

    if (ltPos != std::string::npos)
    {
        std::string name = locPart.substr(0, ltPos-1);
        float threshold = atof(locPart.substr(ltPos + 1).c_str());

        if (!TestRegistry::ParseLocation(name, loc))
            return false;

        float dist = bot->GetDistance(loc.coord_x, loc.coord_y, loc.coord_z);
        if (dist < threshold)
            return true;
    }
    else if (gtPos != std::string::npos)
    {
        std::string name = locPart.substr(0, gtPos-1);
        float threshold = atof(locPart.substr(gtPos + 1).c_str());

        if (!TestRegistry::ParseLocation(name, loc))
            return false;

        float dist = bot->GetDistance(loc.coord_x, loc.coord_y, loc.coord_z);
        if (dist > threshold)
        {
            return true;
        }
    }

    return false;
}

bool CheckUndergroundMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    if (!bot->IsInWorld())
        return false;

    return WorldPosition(bot).isUnderground();
}

bool CheckCanReachNodesMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
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

bool CheckSpeedMonitor::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
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