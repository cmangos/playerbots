#include "playerbot/playerbot.h"
#include "CommandSetup.h"
#include "playerbot/WorldPosition.h"
#include "playerbot/TravelMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestAction.h"
#include "TestRegistry.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

TestResult CommandSetupTeleport::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    GuidPosition loc;
    if (!TestRegistry::ParseLocation(params, loc))
    {
        message = "Invalid teleport location: " + params;
        return TestResult::IMPOSSIBLE;
    }

    if (bot->TeleportTo(loc.mapid, loc.coord_x, loc.coord_y, loc.coord_z, bot->GetOrientation()))
    {
        return TestResult::PASS;
    }
    else
    {
        message = "Teleport failed to " + params;
        return TestResult::IMPOSSIBLE;
    }
}

TestResult CommandSetupGM::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (params == "on")
    {
        bot->SetGameMaster(true);
        bot->GetSession()->SendNotification(LANG_GM_ON);
        return TestResult::PASS;
    }
    else if (params == "off")
    {
        bot->SetGameMaster(false);
        bot->GetSession()->SendNotification(LANG_GM_OFF);
        return TestResult::PASS;
    }
    else if (params == "visible on")
    {
        bot->SetGMVisible(true);
        bot->GetSession()->SendNotification(LANG_INVISIBLE_VISIBLE);
        return TestResult::PASS;
    }
    else if (params == "visible off")
    {
        bot->SetGMVisible(false);
        bot->GetSession()->SendNotification(LANG_INVISIBLE_INVISIBLE);
        return TestResult::PASS;
    }
    else if (params == "fly on")
    {
        bot->SetCanFly(true);
        bot->GetSession()->SendNotification(LANG_COMMAND_FLYMODE_STATUS);
        return TestResult::PASS;
    }
    else if (params == "fly off")
    {
        bot->SetCanFly(false);
        bot->GetSession()->SendNotification(LANG_COMMAND_FLYMODE_STATUS);
        return TestResult::PASS;
    }
    else
    {
        message = "Invalid parameter for setgm: " + params + ". Use 'on' or 'off'.";
        return TestResult::IMPOSSIBLE;
    }
}

TestResult CommandSetupGiveItem::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    uint32 itemId = atoi(params.c_str());
    if (itemId > 0)
    {
        bot->StoreNewItemInInventorySlot(itemId, 1);
        return TestResult::PASS;
    }
    else
    {
        message = "Invalid item ID: " + params;
        return TestResult::IMPOSSIBLE;
    }
}

TestResult CommandSetupEquipItem::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    uint32 itemId = atoi(params.c_str());
    if (itemId > 0)
    {
        bot->StoreNewItemInInventorySlot(itemId, 1);
        return TestResult::PASS;
    }
    else
    {
        message = "Invalid item ID: " + params;
        return TestResult::IMPOSSIBLE;
    }
}

TestResult CommandSetupClearMobs::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    float radius = 50.0f;
    size_t pos = params.find("radius");
    if (pos != std::string::npos)
    {
        std::string radStr = params.substr(pos + 6);
        radius = atof(radStr.c_str());
    }

    std::list<Creature*> creatures;
    MaNGOS::AnyUnitInObjectRangeCheck checker(bot, radius);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
    Cell::VisitWorldObjects(bot, searcher, radius);

    uint32 cleared = 0;
    for (auto& creature : creatures)
    {
        if (creature->IsAlive() && sServerFacade.IsHostileTo(bot, creature))
        {
            Unit::Kill(bot, creature, DIRECT_DAMAGE, nullptr, false, false);
            cleared++;
        }
    }
    return TestResult::PASS;
}

TestResult CommandSetupSetDestination::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    std::string dest = params;
    if (params.find("destination ") == 0)
        dest = params.substr(std::string("destination ").length());
    
    GuidPosition loc;
    if (!TestRegistry::ParseLocation(dest, loc))
    {
        message = "Invalid destination: " + dest;
        return TestResult::IMPOSSIBLE;
    }

    ctx.destinationPosition = loc;

    AiObjectContext* context = ai->GetAiObjectContext();
    TravelTarget* target = AI_VALUE(TravelTarget*,"travel target");
    if (target)
    {
        TemporaryTravelDestination* tempDest = new TemporaryTravelDestination(loc);
        target->SetTarget(tempDest, tempDest->GetPosition());
        target->SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);
        target->SetForced(true);
        target->SetConditions({"not::manual bool::is travel refresh"});

        if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT))
            ai->ChangeStrategy("+travel once", BotState::BOT_STATE_NON_COMBAT);
    }
    else
    {
        message = "Could not get travel target value";
        return TestResult::IMPOSSIBLE;
    }
    return TestResult::PASS;
}