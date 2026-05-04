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
    std::string param = params;

    bool isBank = false;
    if (param.find("bank ") == 0)
    {
        isBank = true;
        param = param.substr(5); // Remove "bank " prefix
    }  
    
    uint32 itemId = atoi(param.c_str());
    if (itemId > 0)
    {

        Item* pItem = bot->StoreNewItemInInventorySlot(itemId, 1);

        if (!pItem)
        {
            message = "Failed to create item with ID: " + param;
            return TestResult::IMPOSSIBLE;
        }

        if (isBank)
        {
            ItemPosCountVec dest;
            InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
            if (msg != EQUIP_ERR_OK)
            {
                message = "Item can not be stored in bank: " + params;
                return TestResult::IMPOSSIBLE;
            }

            bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
            bot->BankItem(dest, pItem, true);
        }

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

    uint32 exceptEntry = 0;
    size_t exceptPos = params.find("except=");
    if (exceptPos != std::string::npos)
    {
        std::string exceptStr = params.substr(exceptPos + 7);
        size_t spacePos = exceptStr.find(' ');
        if (spacePos != std::string::npos)
            exceptStr = exceptStr.substr(0, spacePos);
        exceptEntry = atoi(exceptStr.c_str());
    }

    // Force load the grid at bot's location to ensure creatures are visible
    bot->GetMap()->ForceLoadGrid(bot->GetPositionX(), bot->GetPositionY());

    std::list<Creature*> creatures;
    MaNGOS::AnyUnitInObjectRangeCheck checker(bot, radius);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
    Cell::VisitWorldObjects(bot, searcher, radius);

    uint32 cleared = 0;
    for (auto& creature : creatures)
    {
        if (!creature->IsAlive())
            continue;

        if (exceptEntry && creature->GetEntry() == exceptEntry)
        {
            sLog.outString("[TestAction] clear: SKIPPING boss %s (entry %u)", creature->GetName(), creature->GetEntry());
            continue;
        }

        // Skip critters and non-combat pets
        if (creature->GetCreatureType() == CREATURE_TYPE_CRITTER || creature->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET)
            continue;

        creature->SetDeathState(JUST_DIED);
        creature->SetHealth(0);
        cleared++;
    }
    sLog.outString("[TestAction] clear: killed %u creatures, except entry %u", cleared, exceptEntry);
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

TestResult CommandSetupTeleportGroup::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        sLog.outString("[TestAction] teleport group: bot has no group, skipping");
        return TestResult::PASS;
    }

    float x = bot->GetPositionX();
    float y = bot->GetPositionY();
    float z = bot->GetPositionZ();
    uint32 mapId = bot->GetMapId();
    float orient = bot->GetOrientation();

    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || member == bot)
            continue;

        member->TeleportTo(mapId, x, y, z, orient);
        count++;
    }

    sLog.outString("[TestAction] Teleported %u group members to bot position (map %u, %.1f, %.1f, %.1f)", count, mapId, x, y, z);
    return TestResult::PASS;
}

TestResult CommandSetupPull::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    uint32 entryId = atoi(params.c_str());
    if (!entryId)
    {
        message = "Invalid creature entry: " + params;
        return TestResult::IMPOSSIBLE;
    }

    // Force load the grid at bot's location to ensure creatures are visible
    bot->GetMap()->ForceLoadGrid(bot->GetPositionX(), bot->GetPositionY());

    // First: search via Cell::VisitWorldObjects
    Creature* target = nullptr;
    {
        std::list<Creature*> creatures;
        MaNGOS::AnyUnitInObjectRangeCheck checker(bot, 500.0f);
        MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
        Cell::VisitWorldObjects(bot, searcher, 500.0f);

        for (auto& creature : creatures)
        {
            if (creature->GetEntry() == entryId && creature->IsAlive())
            {
                target = creature;
                break;
            }
        }
    }

    // Second: search via Map object store (finds creatures not in loaded grid cells)
    if (!target)
    {
        auto& store = bot->GetMap()->GetObjectsStore();
        for (auto itr = store.begin<Creature>(); itr != store.end<Creature>(); ++itr)
        {
            if (Creature* c = itr->second)
            {
                if (c->GetEntry() == entryId && c->IsAlive())
                {
                    target = c;
                    sLog.outString("[TestAction] pull: found %s (entry %u) via map store at dist %.1f",
                        c->GetName(), entryId, bot->GetDistance(c));
                    break;
                }
            }
        }
    }

    // Third: spawn the creature if it doesn't exist in the instance
    if (!target)
    {
        static bool pullDebugLogged = false;
        if (!pullDebugLogged)
        {
            sLog.outString("[TestAction] pull %u: creature not found on map %u at (%.1f, %.1f, %.1f), spawning it",
                entryId, bot->GetMapId(),
                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            pullDebugLogged = true;
        }

        target = bot->SummonCreature(entryId,
            bot->GetPositionX() + 5.0f, bot->GetPositionY(), bot->GetPositionZ(),
            bot->GetOrientation() + M_PI_F,
            TEMPSPAWN_MANUAL_DESPAWN, 0);

        if (!target)
        {
            message = "Failed to spawn creature entry " + std::to_string(entryId);
            return TestResult::IMPOSSIBLE;
        }

        sLog.outString("[TestAction] pull: spawned %s (entry %u) near bot", target->GetName(), entryId);
    }

    bot->SetSelectionGuid(target->GetObjectGuid());
    bot->Attack(target, true);
    // Make the boss attack the bot too (prevents evade)
    if (target->AI())
        target->AI()->AttackStart(bot);
    // Disable leashing so the boss doesn't evade
    target->GetCombatManager().SetLeashingDisable(true);
    ctx.focusMobEntry = entryId;
    ctx.focusMobGuid = target->GetObjectGuid();
    sLog.outString("[TestAction] Bot %s pulling creature %s (entry %u) at distance %.1f",
        bot->GetName(), target->GetName(), entryId, bot->GetDistance(target));
    return TestResult::PASS;
}

TestResult CommandSetValue::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    // Expected format: "set value <datatype> <valueName> <valueToSetTo>"
    AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();

    std::string datatype;
    std::string valueStr;

    if (TrySplitOnce(params, " ", datatype, valueStr, message, GetName()) != TestResult::PASS)
    {
        message = "Invalid format for set value: " + params;
        return TestResult::IMPOSSIBLE;
    }

    std::string valueName;
    std::string valueToSetTo;
    if (TrySplitOnce(valueStr, "=>", valueName, valueToSetTo, message, GetName()) != TestResult::PASS)
    {
        message = "Invalid format for set value: " + params;
        return TestResult::IMPOSSIBLE;
    }

    bool isAiValue = context->HasSupportedValue(valueToSetTo);

    if (datatype == "bool")
    {
        bool value = isAiValue ? AI_VALUE(bool, valueToSetTo) : (valueToSetTo == "true");
        SET_AI_VALUE(bool, valueName, value);
        return TestResult::PASS;
    }
    else if (datatype == "uint32")
    {
        uint32 value = isAiValue ? AI_VALUE(uint32, valueToSetTo) : atoi(valueToSetTo.c_str());
        SET_AI_VALUE(uint32, valueName, value);
        return TestResult::PASS;
    }
    else if (datatype == "GuidPosition")
    {
        GuidPosition value = isAiValue ? AI_VALUE(GuidPosition, valueToSetTo) : GuidPosition(valueToSetTo);
        SET_AI_VALUE(GuidPosition, valueName, value);
        return TestResult::PASS;
    }
    else if (datatype == "string")
    {
        std::string value = isAiValue ? AI_VALUE(std::string, valueToSetTo) : valueToSetTo;
        SET_AI_VALUE(std::string, valueName, value);
        return TestResult::PASS;
    }

    message = "Unsupported datatype for set value: " + datatype;
    return TestResult::IMPOSSIBLE;
}