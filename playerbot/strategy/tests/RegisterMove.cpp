#include "playerbot/playerbot.h"
#include "TestRegistry.h"

using namespace ai;

void TestRegistry::RegisterMoveTests()
{
    static const std::string gmInvisible = "gm visible off";
    static const std::string gmVisible = "gm visible on";
    static const std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    static const std::string timeout2Min = "monitor time > 120 => fail \"Timeout: bot did not reach destination (traveled <distance traveled> / wanted <distance wanted>)\"";
    static const std::string timeout10Min = "monitor time > 600 => fail \"Timeout: bot did not reach destination (traveled <distance traveled> / wanted <distance wanted>)\"";

    RegisterTest("movement_walk_short_inside_ironforge", {gmInvisible, needAlive, timeout2Min, "monitor distance to ironforge < 50 => pass \"Bot reached Ironforge gate\"", "teleport ironforge_outside", "set destination ironforge", "observe", gmVisible});
    RegisterTest("movement_walk_long_coldridge_ironforge", {gmInvisible, needAlive, timeout10Min, "monitor distance to ironforge < 100 => pass \"Bot arrived at Ironforge\"", "teleport coldridge", "set destination ironforge", "observe", gmVisible});
    RegisterTest("movement_fly_short_ironforge_to_stormwind", {gmInvisible, needAlive, timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport ironforge", "set destination stormwind", "observe", gmVisible});
    RegisterTest("movement_walk_long_darkshire_to_stormwind", {gmInvisible, needAlive, timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport darkshire", "set destination stormwind", "observe", gmVisible});
    RegisterTest("movement_walk_long_westfall_to_stormwind", {gmInvisible, needAlive, timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport westfall", "set destination stormwind", "observe", gmVisible});
    RegisterTest("movement_walk_long_wetlands_to_ironforge", {gmInvisible, needAlive, timeout10Min, "monitor distance to ironforge < 100 => pass \"Bot arrived at Ironforge\"", "teleport wetlands", "set destination ironforge", "observe", gmVisible});
    RegisterTest("movement_walk_to_zg_entrance", {gmInvisible, needAlive, timeout2Min, "monitor distance to zg_entrance < 50 => pass \"Bot reached ZG entrance\"", "teleport elwynn", "set destination zg_entrance", "observe", gmVisible});
    RegisterTest("movement_fly_stormwind_to_orgrimmar", {gmInvisible, needAlive, timeout10Min, "monitor distance to orgrimmar < 100 => pass \"Bot arrived at Orgrimmar\"", "teleport stormwind", "set destination orgrimmar", "observe", gmVisible});
    RegisterTest("movement_fly_ironforge_to_orgrimmar", {gmInvisible, needAlive, timeout10Min, "monitor distance to orgrimmar < 100 => pass \"Bot arrived at Orgrimmar\"", "teleport ironforge", "set destination orgrimmar", "observe", gmVisible});
    RegisterTest("movement_walk_long_redridge_to_stormwind", {gmInvisible, needAlive, timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport redridge", "set destination stormwind", "observe", gmVisible});
    RegisterTest("movement_walk_long_duskwood_to_stormwind", {gmInvisible, needAlive, timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport duskwood", "set destination stormwind", "observe", gmVisible});

    RegisterTest("scenario_group_skinning_wait_loot_default", {
        "monitor group size < 2 => fail \"Expected second bot in group\"",
        "monitor time > 20 => pass \"Group stayed stable during loot hold window\"",
        "monitor time > 300 => fail \"Timeout in grouped skinning scenario\"",
        "spawn level=60 temporary=1 login=1",
        "wait 5",
        "form party",
        "wait 5",
        "teleport elwynn",
        "set destination elwynn",
        "observe"
    });

    RegisterTest("scenario_fury_equip_upgrades_default", {
        gmInvisible,
        needAlive,
        "require bot is level=60 class=warrior role=dps gear=empty",
        "monitor time > 10 => pass \"Test complete items properly equiped\"",
        "give 8190",
        "give 7687",
        "do equip upgrades",
        "give 17015",
        "give 18805",
        "do equip upgrades",
        "wait 5",
        "require equip main hand=17015",
        "require equip off hand=18805",
        "observe",
        gmVisible
    });

    RegisterTest("scenario_follow_cross_zone_default", {
        gmInvisible,
        needAlive,
        "monitor spawn distance < 40 => pass \"Follower caught up\"",
        "monitor time > 1800 => fail \"Timeout waiting for follower catch-up\"",
        "spawn level=60 temporary=1 login=1",
        "wait 5",
        "form party",
        "wait 5",
        "teleport ironforge_outside",
        "set destination ironforge",
        "wait destination 600",
        "set destination coldridge",
        "wait destination 600",
        "set destination ironforge",
        "wait destination 600",
        "set destination ironforge",
        "observe",
        gmVisible
    });

    RegisterTest("scenario_pull_while_traveling_default", {
        "monitor dead mobs > 0 => pass \"Killed mobs while traveling\"",
        "monitor time > 600 => fail \"Timeout while traversing mob route after <time elapsed> (mobs <mobs killed>, traveled <distance traveled> / wanted <distance wanted>)\"",
        "teleport westfall",
        "set destination elwynn",
        "observe"
    });

    GenerateMovementTests(1000, 5.0f, 100000.0f);
}
