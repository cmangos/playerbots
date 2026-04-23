#include "playerbot/playerbot.h"
#include "RequireState.h"
#include "playerbot/AiFactory.h"

using namespace ai;

TestResult RequireBotIs::Execute(const std::string& params, Player* bot,
            PlayerbotAI* ai, TestContext& ctx, std::string& message)
{

    std::vector<std::string> args = Qualified::getMultiQualifiers(params, " ");
    for (const auto& arg : args)
    {
        size_t eqPos = arg.find('=');
        if (eqPos == std::string::npos)
            continue;

        std::string key = arg.substr(0, eqPos);
        std::string value = arg.substr(eqPos + 1);
        if (key == "level")
        {
            uint32 expectedLevel = static_cast<uint32>(std::strtoul(value.c_str(), nullptr, 10));
            if (bot->GetLevel() != expectedLevel)
            {
                return TestResult::FAIL;
            }
        }
        else if (key == "class")
        {
            uint32 expectedClass = ChatHelper::parseClass(value);
            if (!expectedClass)
            {
                return TestResult::FAIL;
            }

            if (bot->getClass() != expectedClass)
            {
                return TestResult::FAIL;
            }
        }
        else if (key == "role")
        {
            BotRoles expectedRole = ChatHelper::parseRole(value);
            if (expectedRole == BOT_ROLE_NONE)
            {
                return TestResult::FAIL;
            }

            BotRoles actualRole = AiFactory::GetPlayerRoles(bot);
            if ((actualRole & expectedRole) != expectedRole)
            {
                return TestResult::FAIL;
            }
        }
    }

    return TestResult::PASS;
}

TestResult RequireEquip::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    size_t eqPos = params.find('=');

    if (eqPos == std::string::npos)
    {
        message = "wrong equip requirement " + params;
        return TestResult::IMPOSSIBLE;
    }

    std::string key = params.substr(0, eqPos);
    std::string value = params.substr(eqPos + 1);

    if (!Qualified::isValidNumberString(value))
    {
        message = value + " is not a valid item Id";
        return TestResult::IMPOSSIBLE;
    }

    uint32 itemId = stoi(value);

    uint32 slotId = ChatHelper::parseSlot(key);

    if (!slotId)
    {
        for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
            if (item->GetProto()->ItemId == slotId)
                return TestResult::PASS;
        }

        message = "Expected " + value + " in " + key + " but item was not found in equipment.";

        return TestResult::FAIL;
    }
    else
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slotId);

        if (!item)
        {
            message = "Expected " + value + " in " + key + " but slot was empty.";
            return TestResult::FAIL;
        }

        if (item->GetProto()->ItemId != itemId)
        {
            message = "Expected " + value + " in " + key + " but slot had item " + std::to_string(item->GetProto()->ItemId);
            return TestResult::FAIL;
        }
    }

    return TestResult::PASS;
}

        