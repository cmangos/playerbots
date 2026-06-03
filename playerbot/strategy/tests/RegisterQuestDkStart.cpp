#include "playerbot/playerbot.h"
#include "TestRegistry.h"

    static constexpr const char* disableNonQuestRpg = "nc -rpg,+rpg quest,-rpg vendor,-rpg bank,-rpg maintenance,-rpg player,-rpg bg,-rpg guild,-rpg explore,-rpg craft,-rpg jump";
using namespace ai;

void TestRegistry::RegisterQuestDkStartTests()
{
    // Death Knight starter zone (map 609) locations
    TestRegistry::RegisterNamedLocation("ebon_hold_upper", GuidPosition(ObjectGuid(), WorldPosition(609, 2383.65f, -5645.20f, 420.77f)));       // Upper level - quest givers
    TestRegistry::RegisterNamedLocation("ebon_hold_lower", GuidPosition(ObjectGuid(), WorldPosition(609, 2390.02f, -5640.91f, 377.09f)));       // Lower level - runeforge area
    TestRegistry::RegisterNamedLocation("ebon_hold_gryphon", GuidPosition(ObjectGuid(), WorldPosition(609, 2325.03f, -5659.60f, 382.24f)));     // Scourge gryphon (down)
    TestRegistry::RegisterNamedLocation("scarlet_enclave", GuidPosition(ObjectGuid(), WorldPosition(609, 2409.09f, -5722.37f, 154.00f)));       // Ground level - Scarlet Enclave
    TestRegistry::RegisterNamedLocation("scarlet_enclave_south", GuidPosition(ObjectGuid(), WorldPosition(609, 2402.86f, -5727.03f, 154.00f))); // Alt ground landing
    TestRegistry::RegisterNamedLocation("noxious_pass_start", GuidPosition(ObjectGuid(), WorldPosition(609, 2528.22f, -5580.44f, 162.02f)));    // Noxious Pass tunnel entrance
    TestRegistry::RegisterNamedLocation("noxious_pass_end", GuidPosition(ObjectGuid(), WorldPosition(609, 2563.52f, -5547.79f, 163.27f)));      // Noxious Pass tunnel exit 

    static const std::string gmInvisible = "gm visible off";
    static const std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    static const std::string needDK = "require bot is class=dk";
    static const std::string Timeout30Min = "monitor time > 1800 => fail \"Timeout: DK quest not completed (traveled <distance traveled>)\"";
    static const std::string Timeout60Min = "monitor time > 3600 => fail \"Timeout: DK quest not completed (traveled <distance traveled>)\"";
    static const std::string Timeout90Min = "monitor time > 5400 => fail \"Timeout: DK quest not completed (traveled <distance traveled>)\"";

    // ---- Chapter 1: The Emblazoned Runeblade (quest 12619) ----
    RegisterTest("dk_quest_emblazoned_runeblade", {
        "# Ch1: Bot should loot a crafting item and use it to forge a runeblade",
        needDK,
        gmInvisible, needAlive,
        Timeout30Min,
        "monitor quest complete 12619 => pass \"Bot completed The Emblazoned Runeblade\"",
        "teleport ebon_hold_upper",
        "accept quest 12619",
        "observe"
    });

    // ---- Chapter 2: Runeforging (quest 12842) ----
    RegisterTest("dk_quest_runeforging", {
        "# Ch2: Bot should use the runeforge to enchant weapon",
        needDK,
        gmInvisible, needAlive,
        Timeout30Min,
        "monitor quest complete 12842 => pass \"Bot completed Runeforging quest\"",
        "teleport ebon_hold_lower",
        "reward quest 12619",
        "accept quest 12842",
        "observe"
    });

    // ---- Chapter 3: The Endless Hunger (quest 12848) ----
    RegisterTest("dk_quest_endless_hunger", {
        "# Ch3: Bot should use Acherus Shackle Key on Soul Prisons and defeat Unworthy Initiates",
        needDK,
        gmInvisible, needAlive,
        Timeout30Min,
        "monitor quest complete 12848 => pass \"Bot completed The Endless Hunger\"",
        disableNonQuestRpg,
        "teleport ebon_hold_upper",
        "reward quest 12842",
        "accept quest 12848",
        "observe"
    });

    // ---- Chapter 4: Death Comes From On High (quest 12641) ----
    // AUTO-COMPLETE test - verifies the auto-complete system works
    RegisterTest("dk_quest_death_from_on_high", {
        "# Ch4: This quest should auto-complete (requires vehicle control bots can't do)",
        needDK,
        gmInvisible, needAlive,
        "monitor time > 120 => fail \"Auto-complete did not trigger for Death Comes From On High\"",
        "monitor quest complete 12641 => pass \"Bot auto-completed Death Comes From On High\"",
        "teleport ebon_hold_upper",
        "reward quest 12848",
        "accept quest 12641",
        "observe"
    });

    // ---- Chapter 5: The Scarlet Harvest (quest 12670) ----
    RegisterTest("dk_quest_scarlet_harvest", {
        "# Ch5: Bot should use scourge gryphon to fly down to Scarlet Enclave",
        needDK,
        gmInvisible, needAlive,
        Timeout30Min,
        "monitor distance to scarlet_enclave < 200 => pass \"Bot reached Scarlet Enclave via gryphon\"",
        disableNonQuestRpg,
        "teleport ebon_hold_gryphon",
        "reward quest 12641",
        "accept quest 12670",
        "set destination scarlet_enclave",
        "observe"
    });

    // ---- Chapter 5.5: Navigation - Ebon Hold levels ----
    RegisterTest("dk_movement_ebon_hold_levels", {
        "# Ch5.5: Bot should navigate from ground level back up to Ebon Hold",
        needDK,
        gmInvisible, needAlive,
        Timeout30Min,
        "monitor distance to ebon_hold_upper < 100 => pass \"Bot navigated back to Ebon Hold upper level\"",
        "teleport scarlet_enclave",
        "set destination ebon_hold_upper",
        "observe"
    });

    // ---- Chapter 6: Death's Challenge (quest 12733) ----
    RegisterTest("dk_quest_deaths_challenge", {
        "# Ch6: Bot should duel Death Knight Initiates via gossip to complete quest",
        needDK,
        gmInvisible, needAlive,
        Timeout60Min,
        "monitor quest complete 12733 => pass \"Bot completed Death's Challenge (5 duels)\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12670",
        "accept quest 12733",
        "observe"
    });

    // ---- Chapter 7: Grand Theft Palomino (quest 12680) ----
    RegisterTest("dk_quest_grand_theft_palomino", {
        "# Ch7: Bot should mount a horse via spell-click, ride to Salanar, deliver via vehicle ability",
        needDK,
        gmInvisible, needAlive,
        Timeout60Min,
        "monitor quest complete 12680 => pass \"Bot completed Grand Theft Palomino\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12733",
        "accept quest 12680",
        "observe"
    });

    // ---- Chapter 8: Into the Realm of Shadows (quest 12687) ----
    RegisterTest("dk_quest_realm_of_shadows", {
        "# Ch8: Bot should enter shadow realm, kill Dark Rider, steal deathcharger, deliver via gryphon",
        needDK,
        gmInvisible, needAlive,
        Timeout60Min,
        "monitor quest complete 12687 => pass \"Bot completed Into the Realm of Shadows\"",
        "monitor has mount => pass \"Bot obtained DK mount\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12680",
        "accept quest 12687",
        "observe"
    });

    // ---- Chapter 9: The Gift That Keeps On Giving (quest 12698) ----
    RegisterTest("dk_quest_gift_keeps_giving", {
        "# Ch9: Bot should use Gift of Harvester on miners, escort ghouls back to NPC",
        needDK,
        gmInvisible, needAlive,
        Timeout60Min,
        "monitor quest complete 12698 => pass \"Bot completed The Gift That Keeps On Giving\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12687",
        "accept quest 12698",
        "observe"
    });

    // ---- Chapter 10+11: Massacre At Light's Point (quest 12701) ----
    RegisterTest("dk_quest_massacre_lights_point", {
        "# Ch10-11: Bot should enter mine car, ride to boat, use cannon to kill 100 scarlet crusaders",
        needDK,
        gmInvisible, needAlive,
        Timeout90Min,
        "monitor quest complete 12701 => pass \"Bot completed Massacre At Light's Point\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12698",
        "accept quest 12701",
        "observe"
    });

    // ---- Chapter 12: How To Win Friends And Influence Enemies (quest 12720) ----
    RegisterTest("dk_quest_win_friends", {
        "# Ch12: Bot should open jeweled box, equip persuader weapons, interrogate crusaders",
        needDK,
        gmInvisible, needAlive,
        Timeout60Min,
        "monitor quest complete 12720 => pass \"Bot completed How To Win Friends\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12701",
        "accept quest 12720",
        "observe"
    });

    // ---- Chapter 13: Bloody Breakout (quest 12727) ----
    RegisterTest("dk_quest_bloody_breakout", {
        "# Ch13: Bot should stay in basement, kill spawned mobs, loot head container",
        needDK,
        gmInvisible, needAlive,
        Timeout60Min,
        "monitor quest complete 12727 => pass \"Bot completed Bloody Breakout\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12720",
        "accept quest 12727",
        "observe"
    });

    // ---- Chapter 15: An End To All Things (quest 12779) ----
    RegisterTest("dk_quest_end_to_all_things", {
        "# Ch15: Bot should summon frostbrood vanquisher, use breath attacks on crusaders and ballistas",
        needDK,
        gmInvisible, needAlive,
        Timeout90Min,
        "monitor quest complete 12779 => pass \"Bot completed An End To All Things\"",
        disableNonQuestRpg,
        "teleport scarlet_enclave",
        "reward quest 12727",
        "accept quest 12779",
        "observe"
    });

    // ---- Chapter 16: Noxious Pass tunnel navigation ----
    RegisterTest("dk_movement_noxious_pass", {
        "# Ch16: Bot should navigate through the Noxious Pass tunnel",
        needDK,
        gmInvisible, needAlive,
        "monitor time > 300 => fail \"Bot could not navigate Noxious Pass tunnel\"",
        "monitor distance to noxious_pass_end < 30 => pass \"Bot navigated through Noxious Pass\"",
        "teleport noxious_pass_start",
        "set destination noxious_pass_end",
        "observe"
    });

    // ---- Full DK Starter Zone Escape ----
    RegisterTest("dk_full_starter_zone_escape", {
        "# Full DK starter zone: Bot should progress through all quests and escape Ebon Hold",
        needDK,
        gmInvisible, needAlive,
        "monitor time > 14400 => fail \"Timeout: Bot did not escape Ebon Hold in 4 hours\"",
        "monitor not on map 609 => pass \"Bot escaped the DK starter zone!\"",
        "teleport ebon_hold_upper",
        "observe"
    });
}
