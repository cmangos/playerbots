#include "TestComponent.h"
#include "playerbot/WorldPosition.h"
#include "playerbot/PlayerbotTextMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;

// =====================================================
// TestMonitor implementation
// =====================================================

namespace
{
    uint32 CountNearbyDeadMobs(Player* bot, float radius)
    {
        std::list<Creature*> creatures;
        MaNGOS::AnyUnitInObjectRangeCheck checker(bot, radius);
        MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
        Cell::VisitWorldObjects(bot, searcher, radius);

        uint32 deadCount = 0;
        for (Creature* creature : creatures)
        {
            if (!creature->IsAlive())
                ++deadCount;
        }

        return deadCount;
    }
}

TestResult TestMonitor::Check(const std::string& monitorStr, Player* bot, TestContext& ctx, std::string& message) const
{
    if (IsConditionMet(monitorStr, bot, ctx))
    {
        size_t arrowPos = monitorStr.find("=>");
        if (arrowPos == std::string::npos)
            return TestResult::PENDING;

        size_t quoteStart = monitorStr.find("\"", arrowPos);
        if (quoteStart == std::string::npos)
            return TestResult::PENDING;

        size_t quoteEnd = monitorStr.find("\"", quoteStart + 1);
        if (quoteEnd == std::string::npos)
            return TestResult::PENDING;

        message = monitorStr.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

        // Replace dynamic placeholders in message
        std::map<std::string, std::string> placeholders;
        if (bot->IsInWorld())
        {
            WorldPosition pos(bot);
            placeholders["<current position>"] = pos.print(2, true) + " m" + std::to_string(pos.getMapId());

            if (ctx.testStartPosition)
                placeholders["<distance traveled>"] = std::to_string(static_cast<uint32>(ctx.testStartPosition.distance(pos))) + "m";

            placeholders["<mobs killed>"] = std::to_string(CountNearbyDeadMobs(bot, 120.0f));
        }

        if (ctx.testStartPosition && ctx.destinationPosition)
            placeholders["<distance wanted>"] = std::to_string(static_cast<uint32>(ctx.testStartPosition.distance(WorldPosition(ctx.destinationPosition)))) + "m";

        placeholders["<time elapsed>"] = std::to_string((WorldTimer::getMSTime() - ctx.testStartTime) / 1000) + "s";
        
        if (!placeholders.empty())
        {
            PlayerbotTextMgr::ReplacePlaceholders(message, placeholders);
        }

        std::string resultType = monitorStr.substr(arrowPos + 3, quoteStart - (arrowPos + 2)-2);        

        if (resultType == "pass")
            return TestResult::PASS;
        else if (resultType == "fail")
            return TestResult::FAIL;
        else if (resultType == "impossible")
            return TestResult::IMPOSSIBLE;
        else if (resultType == "abort")
            return TestResult::ABORT;
        else
            return TestResult::PENDING;

        return TestResult::PENDING;
    }
    
    return TestResult::PENDING;
}