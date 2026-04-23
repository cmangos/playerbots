#include "playerbot/playerbot.h"
#include "FlowCommands.h"
#include "playerbot/TravelMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestAction.h"
#include "TestRegistry.h"
#include <algorithm>
#include <cctype>

using namespace ai;

bool HandleObserve::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    ctx.observing = true;
    return true;
}

bool HandleCleanup::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    for (auto& guid : ctx.spawnedBots)
    {
        if (guid.IsPlayer())
        {
            if (ai->GetHolder())
                ai->GetHolder()->DeleteBot(guid, false);
        }
        else if (Creature* creature = ai->GetCreature(guid))
        {
            creature->ForcedDespawn();
        }
    }
    ctx.spawnedBots.clear();
    ctx.observing = false;
    return true;
}

bool HandleAssert::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    if (params.find("distance to") == 0)
    {
        GuidPosition loc;
        float threshold = 0;
        bool lessThan = false;

        size_t ltPos = params.find("<");
        size_t gtPos = params.find(">");

        if (ltPos != std::string::npos)
        {
            lessThan = true;
            std::string locPart = params.substr(std::string("distance to").length(), ltPos - std::string("distance to").length());
            threshold = atof(params.substr(ltPos + 1).c_str());
            TestRegistry::ParseLocation(locPart, loc);
        }
        else if (gtPos != std::string::npos)
        {
            lessThan = false;
            std::string locPart = params.substr(std::string("distance to").length(), gtPos - std::string("distance to").length());
            threshold = atof(params.substr(gtPos + 1).c_str());
            TestRegistry::ParseLocation(locPart, loc);
        }

        float dist = bot->GetDistance(loc.coord_x, loc.coord_y, loc.coord_z);
        bool passed = lessThan ? (dist < threshold) : (dist > threshold);

        if (!passed)
        {
            error = "Assert failed: distance to " + params;
            return false;
        }
        return true;
    }
    else if (params.find("slot ") == 0)
    {
        // Syntax: assert slot mainhand == 18878
        //         assert slot offhand == 18805
        size_t eqPos = params.find("==");
        if (eqPos == std::string::npos)
        {
            error = "Assert failed: slot assert requires '=='";
            return false;
        }

        std::string lhs = params.substr(5, eqPos - 5);
        std::string rhs = params.substr(eqPos + 2);

        std::string slotName = lhs;
        slotName.erase(std::remove_if(slotName.begin(), slotName.end(), ::isspace), slotName.end());
        uint32 expectedEntry = atoi(rhs.c_str());

        uint8 slot = 255;
        if (slotName == "mainhand")
            slot = EQUIPMENT_SLOT_MAINHAND;
        else if (slotName == "offhand")
            slot = EQUIPMENT_SLOT_OFFHAND;

        if (slot == 255)
        {
            error = "Assert failed: unsupported slot '" + slotName + "'";
            return false;
        }

        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        uint32 actualEntry = item ? item->GetEntry() : 0;

        if (actualEntry != expectedEntry)
        {
            error = "Assert failed: slot " + slotName + " expected item " +
                std::to_string(expectedEntry) + " but was " + std::to_string(actualEntry);
            return false;
        }

        return true;
    }
    else if (params.find("hp") == 0)
    {
        size_t gtPos = params.find(">");
        if (gtPos != std::string::npos)
        {
            uint32 threshold = atoi(params.substr(gtPos + 1).c_str());
            if (bot->GetHealthPercent() <= threshold)
            {
                error = "Assert failed: HP " + std::to_string(bot->GetHealthPercent()) + "% <= " + std::to_string(threshold) + "%";
                return false;
            }
        }
    }

    return true;
}

bool HandleMonitor::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    ctx.monitors.push_back(params);
    return true;
}