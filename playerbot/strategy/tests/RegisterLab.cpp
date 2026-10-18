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
