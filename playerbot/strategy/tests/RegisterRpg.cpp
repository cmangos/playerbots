
#include "playerbot/playerbot.h"
#include "../values/ItemUsageValue.h"
#include "playerbot/TravelMgr.h"
#include "TestRegistry.h"

using namespace ai;

void TestRegistry::RegisterBankTests()
{
    std::string bankUsage = std::to_string((uint8)ItemUsage::ITEM_USAGE_BANK);

    DestinationList BankDestinations = sTravelMgr.GetDestinations(PlayerTravelInfo(), (uint32)TravelDestinationPurpose::Bank, {2461}, false);
  if (!BankDestinations.empty() && !BankDestinations.front()->GetPoints().empty())
  {
    WorldPosition* bankPos = BankDestinations.front()->GetPoints().front();
    if (bankPos)
      TestRegistry::RegisterNamedLocation("Ironforge bank", GuidPosition(ObjectGuid(), WorldPosition(*bankPos)));
  }

    // Test 1: Item usage marking - items should be marked for bank deposit
    RegisterTest("bank_item_usage_marking", {"# Test items are marked for bank deposit (ITEM_USAGE_BANK)", 
       "require bot is level=22 class=warrior", 
       "give 3840", // Green Iron Pauldrons (req level 27 > 22 → BANK)
       "monitor value uint32 item count::3840 == 0 => fail \"Item not added to inventory\"",
       "monitor value uint32 item count::usage " + bankUsage + " > 0 => pass \"Item has correct usage\"",
       "monitor time > 20 => fail \"Timeout\"",
       "q 3840",
       "observe"});

    // Test 2: Deposit items to bank via banker interaction
    RegisterTest("bank_deposit_flow", {
        "# Test bot deposits bank-worthy items to banker", 
        "require bot is level=22 class=warrior race=human", 
        "nc -travel", 
        //"nc +debug", 
        //"nc +debug rpg",
        "give 3840", // Item to deposit
        "teleport Ironforge bank", // Teleport to Ironforge bank
        "set value GuidPosition rpg target => closest entry::2461", // Ironforge banker NPC ID
        "set value string next rpg action => rpg bank deposit",
        "monitor value uint32 bank item count::3840 > 0 => pass \"Item deposited to bank\"",
        "monitor time > 20 => fail \"Deposit timed out\"",
        "observe"
    });

    // Test 3: Mirror test - withdraw item from bank when no longer bank-worthy
    RegisterTest("bank_withdraw_flow", 
       {"# Test bot withdraws item from bank when equippable",
        "require bot is level=27 class=warrior race=human", // Now can equip 3840 (req level 22)
        "nc -travel", 
        "teleport Ironforge bank",
        "set value GuidPosition rpg target => closest entry::2461", // Ironforge banker NPC ID
        "set value string next rpg action => rpg bank withdraw",
        "give bank 3840",
        "monitor value uint32 item count::3840 > 0 => pass \"Item withdrawn to inventory\"",
        "monitor time > 20 => fail \"Withdrawal timed out\"",
        "observe"
    });
}
