#include "playerbot/playerbot.h"
#include "MovementMonitors.h"
#include "playerbot/WorldPosition.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestRegistry.h"

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