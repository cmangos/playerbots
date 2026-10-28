#include "playerbot/playerbot.h"
#include "TestRegistry.h"

using namespace ai;

static uint32 const DUMMY_ENTRY = 6788;
static uint32 const DUMMY_LEVEL = 80;

void TestRegistry::RegisterLabTests()
{
    // Enough-space probing across cross-core lab candidates. Three tests per map:
    //   isolation   - park the bot at the anchor; ambient units must not engage it
    //   containment - the AI must engage and kill a pulled quest-objective dummy at the anchor
    //   room        - the zone must path end-to-end between two anchors within 10 min

    struct Lab
    {
        uint32 mapId;
        const char* name;
        float cx, cy, cz;
        float fx, fy, fz;
    };

    static const Lab labs[] = {
        {34, "stockade",        97.0f,   -44.0f, -34.8f,   167.0f, 135.0f,  -33.9f},
        {47, "razorfen_kraul",  2138.0f, 1621.0f, 81.7f,   2069.0f, 2015.0f, 64.0f},
        {70, "uldaman",         122.0f,  285.0f, -52.1f,   -358.0f, 119.0f, -44.4f},
        {209, "zulfarrak",      1817.0f, 877.0f, 9.0f,     1694.0f, 1305.0f, 35.4f},
        {249, "onyxias_lair",   -27.0f,  -219.0f, -88.6f,  -200.0f, -212.0f, -68.6f},
        {329, "stratholme",     3506.0f, -3079.0f, 135.1f, 4057.0f, -3675.0f, 132.7f},
    };

    for (const Lab& lab : labs)
    {
        std::string centerName = std::string("lab_") + lab.name + "_center";
        std::string farName = std::string("lab_") + lab.name + "_far";

        RegisterNamedLocation(centerName,
            GuidPosition(ObjectGuid(), WorldPosition(lab.mapId, lab.cx, lab.cy, lab.cz)));
        RegisterNamedLocation(farName,
            GuidPosition(ObjectGuid(), WorldPosition(lab.mapId, lab.fx, lab.fy, lab.fz)));

        // containment v2: the AI must own the fight, so the pulled dummy is a real quest
        // kill objective (quest 2139, Den Mother 6788) instead of the old raw pull-only probe
        // that the AI ignored (possible-attack-targets stayed empty, monitor "Encounter not
        // resolved in 5 min" FAILed all six maps).
        std::string failEngaged =
            "monitor hp < 95 => fail \"Ambient unit mauled the parked bot - lab zone not usable as-is\"";
        std::string failDrift =
            std::string("monitor distance to ") + centerName +
            " > 60 => fail \"Ambient pull drifted the bot > 60m off its anchor\"";
        std::string failLeak =
            std::string("monitor distance to ") + centerName +
            " > 70 => fail \"Fight carried the bot > 70m from anchor - chase leak\"";
        std::string freshPass =
            std::string("monitor distance to ") + centerName +
            " < 25 => pass \"Zone traversed end-to-end\"";
        std::string continentLine =
            std::string("monitor not on map ") + std::to_string(lab.mapId) +
            " => abort \"Bot left the lab map\"";

        RegisterTest(std::string("lab_isolation_") + lab.name, {
            "# Parked at the anchor: ambient units on this map must not engage or drag the bot",
            "nc -rpg,+stay",
            "gm visible off",
            "monitor bot dead => abort \"Bot died during isolation probe\"",
            failEngaged,
            failDrift,
            "monitor time > 300 => pass \"Isolation clean for 5 min\"",
            std::string("teleport ") + centerName,
            "clear mobs radius=120",
            "wait 30",
            "observe"
        });

        RegisterTest(std::string("lab_containment_") + lab.name, {
            "# AI-owned fight at the anchor: accept the kill quest, pull the dummy, the bot must engage and kill it in place",
            "nc -rpg,+stay",
            "gm visible on",
            "gm on",
            continentLine,
            "monitor bot dead => abort \"Bot died during containment probe\"",
            "monitor quest objective 2139 0 count > 0 => pass \"Pulled dummy killed by the bot's own AI (kill credit)\"",
            std::string("monitor mob ") + std::to_string(DUMMY_ENTRY) +
                " is dead => pass \"Encounter resolved inside the zone\"",
            failLeak,
            "monitor time > 420 => fail \"Encounter not resolved in 7 min\"",
            std::string("require bot is level=") + std::to_string(DUMMY_LEVEL),
            std::string("teleport ") + centerName,
            "wait 30",
            "clear mobs radius=1500",
            "accept quest 2139",
            "gm off",
            std::string("pull ") + std::to_string(DUMMY_ENTRY),
            "observe"
        });

        RegisterTest(std::string("lab_room_") + lab.name, {
            "# End-to-end traversal between the far anchor and the center across the zone",
            "nc -rpg,+stay",
            "gm visible off",
            continentLine,
            "monitor bot dead => abort \"Bot died during room probe\"",
            freshPass,
            "monitor time > 600 => fail \"Zone not traversable in 10 min - blocked or not connected\"",
            std::string("teleport ") + farName,
            "wait 30",
            std::string("set destination ") + centerName,
            "observe"
        });
    }
}

void TestRegistry::RegisterLabStayGuardTests()
{
    static const char* pins[] = { "stay", "guard" };

    for (const char* pin : pins)
    {
        std::string pinStrategy = std::string("nc -rpg,-travel,+") + pin;
        std::string prefix = std::string("lab_") + pin + "_";
        std::string spawnInReach = std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=10 toward lab_stockade_far";
        std::string spawnAggro = spawnInReach + " aggro";
        std::string spawnAtFeet = std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=3 toward lab_stockade_far";
        std::string spawnOutOfReach = std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=50 toward lab_stockade_far";
        std::string levelRequirement = std::string("require bot is level=") + std::to_string(DUMMY_LEVEL);
        std::string anchorDriftLimit = pin == std::string("stay") ? "15" : "75";

        RegisterTest(prefix + "reach_stockade", {
            "# Fresh hostile 10 yd out that attacks the parked bot: the pinned bot must fight back and kill it without chasing",
            pinStrategy,
            "gm visible on",
            "gm on",
            "monitor not on map 34 => abort \"Bot left the lab map\"",
            "monitor bot dead => abort \"Bot died during the reach probe\"",
            "monitor distance to lab_stockade_center > 40 => fail \"The fight carried the pinned bot beyond its anchor area\"",
            "monitor quest objective 2139 0 count > 0 => pass \"Pinned bot engaged and killed the in-reach hostile without chasing it\"",
            "monitor time > 420 => fail \"In-reach hostile not killed in 7 min\"",
            levelRequirement,
            "teleport lab_stockade_center",
            "wait 30",
            "clear mobs radius=1500",
            pinStrategy,
            "accept quest 2139",
            "gm off",
            spawnAggro,
            "observe"
        });

        RegisterTest(prefix + "ignore_stockade", {
            "# Fresh hostile 50 yd out along the proven traverse: the pinned bot must not engage it and must not move",
            pinStrategy,
            "gm visible on",
            "gm on",
            "monitor not on map 34 => abort \"Bot left the lab map\"",
            "monitor bot dead => abort \"Bot died during the ignore probe\"",
            std::string("monitor distance to lab_stockade_center > ") + anchorDriftLimit +
                " => fail \"The pinned bot left its anchor area while ignoring the out-of-reach hostile\"",
            "monitor hp < 95 => fail \"The parked bot was mauled while ignoring the out-of-reach hostile\"",
            "monitor quest objective 2139 0 count > 0 => fail \"The pinned bot chased down and killed the out-of-reach hostile\"",
            "monitor time > 300 => pass \"Out-of-reach hostile ignored for 5 min with the bot parked\"",
            levelRequirement,
            "teleport lab_stockade_center",
            "wait 30",
            "clear mobs radius=1500",
            pinStrategy,
            "accept quest 2139",
            "gm off",
            spawnOutOfReach,
            "observe"
        });

        RegisterTest(prefix + "wedge_stockade", {
            "# Stale live target 50 yd out on a pinned bot: it must be cleared without leaving the anchor, then a fresh hostile at melee range must be killed",
            pinStrategy,
            "gm visible on",
            "gm on",
            "monitor not on map 34 => abort \"Bot left the lab map\"",
            "monitor bot dead => abort \"Bot died during the wedge probe\"",
            "monitor distance to lab_stockade_center > 40 => fail \"Wedge recovery carried the pinned bot out of the anchor area\"",
            "monitor quest objective 2139 0 count > 0 => pass \"Stale out-of-reach target cleared; the pinned bot then killed a fresh in-reach hostile\"",
            "monitor time > 420 => fail \"Stale target not cleared in 7 min\"",
            levelRequirement,
            "teleport lab_stockade_center",
            "wait 30",
            "clear mobs radius=1500",
            pinStrategy,
            "accept quest 2139",
            "gm off",
            spawnOutOfReach + " select",
            "wait 20",
            spawnAtFeet,
            "observe"
        });
    }
}

void TestRegistry::RegisterLabLootTests()
{
    RegisterTest("lab_loot_follow_far_corpse", {
        "# Screenshot repro 2026-10-25 (bot Ruth): a following bot ping-pongs between 'move to loot'",
        "# [far from current loot] and 'follow' [out of free move range] on loot it can never reach.",
        "# The Rabbit anchor has no loot table on purpose: the far corpse is the only loot in the stack.",
        "# The far Fang is planted AFTER the follow is running and the loot strategy is wide: a lootable",
        "# corpse decays to bones in ~15-35 s, so planting it first made the wide-strategy window a race",
        "# the setup kept losing. Loot appearing while already following is the real scenario anyway.",
        "# Expected: the follower stays with its anchor; repeated 'move to loot' attempts are the defect.",
        "monitor not on map 34 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot died during the loot ping-pong probe\"",
        "monitor distance to lab_stockade_center > 60 => fail \"Chase drifted the bot > 60m off its anchor, now at <current position>\"",
        "monitor loot stack > 90 => fail \"The far corpse never registered as loot - repro setup failed\"",
        "monitor action spam move to loot > 3 => fail \"Follower ping-pongs between move-to-loot and follow on the far corpse at <current position>\"",
        "monitor value bool can loot == true => pass \"Follower reached the far corpse in <time elapsed>\"",
        "monitor time > 150 => pass \"Follower ignored the far loot and stayed with its follow point for <time elapsed>\"",
        "monitor time > 420 => fail \"Timeout: loot neither reached nor ignored after <time elapsed>\"",
        "require bot is level=60",
        "teleport lab_stockade_center",
        "clear mobs radius=1500",
        "spawn mob 721 dist=5",
        "follow focus",
        "set value string loot strategy => equip,quest,skill,disenchant,use,vendor,trash",
        "spawn mob 471 dist=22 aggro",
        "await focus corpse 60",
        "observe"
    });
}

void TestRegistry::RegisterLabMicroTests()
{
    RegisterTest("lab_micro1_stockade", {
        "gm visible off",
        "monitor bot dead => abort \"Bot died\"",
        "monitor time > 300 => pass \"Stand clean\"",
        "teleport lab_stockade_center",
        "wait 30",
        "observe"
    });

    RegisterTest("lab_micro2_stockade", {
        "gm visible off",
        "monitor bot dead => abort \"Bot died\"",
        "monitor distance to lab_stockade_center > 60 => fail \"drift\"",
        "monitor time > 300 => pass \"Stand clean with distance monitor\"",
        "teleport lab_stockade_center",
        "wait 30",
        "observe"
    });

    RegisterTest("lab_micro3_stockade", {
        "gm visible off",
        "monitor bot dead => abort \"Bot died\"",
        "monitor value uint8 my attacker count > 0 => fail \"engaged\"",
        "monitor time > 300 => pass \"Stand clean with attacker monitor\"",
        "teleport lab_stockade_center",
        "wait 30",
        "observe"
    });
}

void TestRegistry::RegisterFleeTests()
{
    static const uint32 DUMMY_ENTRY = 6788;
    static const uint32 GRAY_ENTRY = 2346;

    RegisterNamedLocation("lab_flee_center",
        GuidPosition(ObjectGuid(), WorldPosition(1, -7150.9f, -2282.4f, -266.0f)));
    RegisterNamedLocation("lab_flee_far",
        GuidPosition(ObjectGuid(), WorldPosition(1, -7301.5f, -2279.1f, -265.2f)));

    std::string failFledSolo =
        "monitor distance to lab_flee_center > 60 => fail \"Bot fled a fight it should have fought\"";

    RegisterTest("lab_flee_solo", {
        "# 1v1 at the anchor: one hard hostile must be fought and killed, not fled - bot pinned so drift means fled",
        "nc -rpg,+stay",
        "gm visible on",
        "gm on",
        "monitor not on map 1 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot died to a single hostile\"",
        failFledSolo,
        std::string("monitor mob ") + std::to_string(DUMMY_ENTRY) +
            " is dead => pass \"Single hostile engaged and killed without fleeing\"",
        "monitor time > 420 => fail \"Single hostile not resolved in 7 min\"",
        "require bot is level=80 faction=alliance",
        "teleport lab_flee_center",
        "wait 30",
        "clear mobs radius=1500",
        "nc -rpg,+stay",
        "accept quest 2139",
        "gm off",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=12 toward lab_flee_far aggro level=75",
        "observe"
    });

    RegisterTest("lab_flee_pack", {
        "# Doomed pack: five hard hostiles on a full-health warrior - it must disengage and survive, not stand and die",
        "gm visible on",
        "gm on",
        "monitor not on map 1 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot stood and died against the pack it predicted it could not win\"",
        "monitor not in combat => pass \"Bot disengaged the doomed pack (combat cleared after engaging)\"",
        "monitor time > 420 => pass \"Bot survived the doomed pack for 7 min without dying\"",
        "require bot is level=80 class=1 faction=alliance",
        "teleport lab_flee_center",
        "wait 30",
        "clear mobs radius=1500",
        "accept quest 2139",
        "gm off",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=10 toward lab_flee_far aggro level=75",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=14 toward lab_flee_far aggro level=75",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=18 toward lab_flee_far aggro level=75",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=22 toward lab_flee_far aggro level=75",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=26 toward lab_flee_far aggro level=75",
        "observe"
    });

    RegisterTest("lab_flee_pack_leash", {
        "# Doomed pack, leashed: the disengage is only real when the pack resets home after the bot breaks contact",
        "gm visible on",
        "gm on",
        "monitor not on map 1 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot stood and died against the pack it predicted it could not win\"",
        std::string("monitor mob ") + std::to_string(DUMMY_ENTRY) +
            " is home => pass \"Pack reset home after the bot disengaged (leash proof)\"",
        "monitor time > 420 => fail \"Pack never reset within 7 min - the bot never truly left the pack\"",
        "require bot is level=80 class=1 faction=alliance",
        "teleport lab_flee_center",
        "wait 30",
        "clear mobs radius=1500",
        "accept quest 2139",
        "gm off",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=10 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=14 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=18 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=22 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=26 toward lab_flee_far aggro level=75 leash=30",
        "observe"
    });

    RegisterTest("lab_flee_pack_casters", {
        "# Doomed pack on a caster: a kiting mage must still end the chase (pack resets home), not kite forever",
        "gm visible on",
        "gm on",
        "monitor not on map 1 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot stood and died against the pack it predicted it could not win\"",
        std::string("monitor mob ") + std::to_string(DUMMY_ENTRY) +
            " is home => pass \"Caster ended the chase - pack reset home (leash proof)\"",
        "monitor time > 420 => fail \"Caster never ended the chase within 7 min - endless kite wedge\"",
        "require bot is level=80 class=8 faction=alliance",
        "teleport lab_flee_center",
        "wait 30",
        "clear mobs radius=1500",
        "accept quest 2139",
        "gm off",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=10 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=14 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=18 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=22 toward lab_flee_far aggro level=75 leash=30",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=26 toward lab_flee_far aggro level=75 leash=30",
        "observe"
    });

    RegisterTest("lab_flee_grays", {
        "# Numbers alone do not trigger flee: four forced grays never disengage the bot in 7 min",
        "nc -rpg,+stay",
        "gm visible on",
        "gm on",
        "monitor not on map 1 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot died to grays\"",
        failFledSolo,
        "monitor time > 420 => pass \"Gray attackers for 7 min never triggered flee\"",
        "require bot is level=80 faction=alliance",
        "teleport lab_flee_center",
        "wait 30",
        "clear mobs radius=1500",
        "nc -rpg,+stay",
        "accept quest 2139",
        "gm off",
        std::string("spawn mob ") + std::to_string(GRAY_ENTRY) + " dist=8 toward lab_flee_far aggro",
        std::string("spawn mob ") + std::to_string(GRAY_ENTRY) + " dist=9 toward lab_flee_far aggro",
        std::string("spawn mob ") + std::to_string(GRAY_ENTRY) + " dist=10 toward lab_flee_far aggro",
        std::string("spawn mob ") + std::to_string(GRAY_ENTRY) + " dist=11 toward lab_flee_far aggro",
        "observe"
    });

    RegisterTest("lab_flee_dungeon", {
        "# Dungeon exclusion: a pack inside a dungeon must be resolved by fighting, fleeing stays off",
        "gm visible on",
        "gm on",
        "monitor not on map 34 => abort \"Bot left the lab map\"",
        "monitor bot dead => abort \"Bot died resolving the dungeon pack\"",
        "monitor distance to lab_stockade_center > 150 => fail \"Bot fled inside a dungeon where outnumbered is disabled\"",
        "monitor not in combat => pass \"Dungeon pack resolved by fighting (flee exclusion honored)\"",
        "monitor time > 420 => fail \"Dungeon pack not resolved in 7 min\"",
        "require bot is level=80 class=1",
        "teleport lab_stockade_center",
        "wait 30",
        "clear mobs radius=1500",
        "accept quest 2139",
        "gm off",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=10 toward lab_stockade_far aggro",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=14 toward lab_stockade_far aggro",
        std::string("spawn mob ") + std::to_string(DUMMY_ENTRY) + " dist=18 toward lab_stockade_far aggro",
        "observe"
    });
}
