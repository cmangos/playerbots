#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "playerbot/ChatHelper.h"
#include "Globals/ObjectMgr.h"
#include <regex>
#include <sstream>
#include "playerbot/TravelMgr.h"
#include <playerbot/TravelNode.h>

static std::map<std::string, std::vector<std::string>> sTestRegistry;
static bool sTestsRegistered = false;

static std::map<std::string, GuidPosition> sTestLocations;

struct TeleLoc
{
    std::string name;
    GuidPosition pos;
};

static std::vector<TeleLoc> sTeleLocations;

static void InitTestLocations()
{
    sTestLocations["ironforge_outside"] = GuidPosition(ObjectGuid(), WorldPosition(0, -5150.0f, -856.0f, 508.4f));
    sTestLocations["zg_boss1_room"] = GuidPosition(ObjectGuid(), WorldPosition(309, -12276.0f, -1399.0f, 130.9f));
}

static void InitTeleLocations()
{
    for (auto const& [id, tele] : sObjectMgr.GetGameTeleMap())
    {
        GuidPosition pos(ObjectGuid(), WorldPosition(tele.mapId, tele.position_x, tele.position_y, tele.position_z, tele.orientation));
        sTeleLocations.push_back({ tele.name, pos });
    }
}

static void EnsureLocationsInit()
{
    static bool initialized = false;
    if (!initialized)
    {
        InitTestLocations();
        InitTeleLocations();
        initialized = true;
    }
}

namespace
{
    using ScenarioParams = std::map<std::string, std::string>;

    uint32 ParseUintOrDefault(const std::string& value, uint32 fallback)
    {
        if (value.empty())
            return fallback;

        for (char c : value)
            if (!std::isdigit(static_cast<unsigned char>(c)))
                return fallback;

        return static_cast<uint32>(std::stoul(value));
    }

    uint32 ParseMGroupSize(const std::string& line)
    {
        static const uint32 defaultGroupSize = 5;
        std::istringstream iss(line);
        std::string token;

        while (iss >> token)
        {
            if (token.find("size=") != 0)
                continue;

            return ParseUintOrDefault(token.substr(std::string("size=").length()), defaultGroupSize);
        }

        return defaultGroupSize;
    }

    std::string ApplyScenarioParams(const std::string& line, const ScenarioParams& params)
    {
        std::string expanded = line;
        for (const auto& pair : params)
        {
            const std::string token = "$(" + pair.first + ")";
            size_t pos = 0;
            while ((pos = expanded.find(token, pos)) != std::string::npos)
            {
                expanded.replace(pos, token.length(), pair.second);
                pos += pair.second.length();
            }
        }
        return expanded;
    }

    std::vector<std::string> ApplyScenarioParams(const std::vector<std::string>& script, const ScenarioParams& params)
    {
        std::vector<std::string> expanded;
        expanded.reserve(script.size());
        for (const std::string& line : script)
            expanded.push_back(ApplyScenarioParams(line, params));
        return expanded;
    }

    void RegisterScenarioVariant(const std::string& namePrefix, const std::string& variant,
                                 const std::vector<std::string>& scriptTemplate, const ScenarioParams& params)
    {
        TestRegistry::RegisterTest(namePrefix + "_" + variant, ApplyScenarioParams(scriptTemplate, params));
    }
}

void TestRegistry::GenerateMovementTestsImpl(int maxTests, float minDist, float maxDist)
{
    if (sTeleLocations.size() < 2)
        return;

    std::string gmInvisible = "gm visible off";
    std::string gmVisible = "cleanup gm visible on";
    std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    std::string needAboveGRound = "monitor underground => fail \"Bot is underground at <current position>\"";
    std::string needCanReachNode = "monitor can not reach node => fail \"Bot cannot reach travel network at <current position>\"";

    int count = 0;
    for (size_t i = 0; i < sTeleLocations.size() && count < maxTests; ++i)
    {
        for (size_t j = 0; j < sTeleLocations.size() && count < maxTests; ++j)
        {
            if (i == j)
                continue;

            WorldPosition startPos = sTeleLocations[i].pos;
            WorldPosition endPos = sTeleLocations[j].pos;

            if (!startPos || !endPos)
                continue;

            if (startPos.isBg() || endPos.isBg())
                continue;

            if (startPos.isDungeon() && startPos.getMapEntry()->IsRaid())
                continue;

            if (endPos.isDungeon() && endPos.getMapEntry()->IsRaid())
                continue;

            float dist = startPos.distance(endPos);

            if (dist < minDist || dist > maxDist)
                continue;

            std::string prefix;
            int timeoutSecs = dist + 600;
            if (dist < 500)
            {
                prefix = "short";
            }
            else if (dist < 2000)
            {
                prefix = "medium";
            }
            else
            {
                prefix = "far";
            }

            std::string startName = sTeleLocations[i].name;
            std::transform(startName.begin(), startName.end(), startName.begin(), ::tolower);
            std::replace(startName.begin(), startName.end(), '\'', '_');
            std::string endName = sTeleLocations[j].name;
            std::transform(endName.begin(), endName.end(), endName.begin(), ::tolower);
            std::replace(endName.begin(), endName.end(), '\'', '_');


            std::ostringstream testName;
            testName << "movement_" << prefix << "_" << startName << "_" << endName;

            std::ostringstream timeout;
            timeout << "monitor time > " << timeoutSecs << " => fail \"Timeout: bot did not reach destination after <time elapsed> (traveled <distance traveled> / wanted <distance wanted>)\"";

            std::ostringstream reachCheck;
            reachCheck << "monitor distance to " << endName << " < 100 => pass \"Bot arrived at " << endName << " after <time elapsed>\"";

            std::vector<std::string> script = {
                gmInvisible,
                needAlive,
                needAboveGRound,
                needCanReachNode,
                timeout.str(),
                reachCheck.str(),
                "teleport " + startName,
                "set destination " + endName,
                "observe",
                gmVisible
            };

            RegisterTest(testName.str(), script);
            count++;

            if ((j % 10) == 0 && i < sTeleLocations.size())
                i++;
        }
    }
}

using namespace ai;

std::string TestRegistry::GetBotCreationRequirement(const std::string& testName)
{
    std::vector<std::string> script = GetTestScript(testName);

    for (const std::string& line : script)
    {
        if (line.find("require bot is") != 0)
            continue;

        std::string params = line.substr(std::string("require bot is ").length());
        return params;
    }

    return "";
}

uint32 TestRegistry::ExpectedBotSpawnCount(const std::string& testName)
{
    std::vector<std::string> script = GetTestScript(testName);

    uint32 expectedBots = 1;

    for (const std::string& line : script)
    {
        if (line.find("spawn") == 0)
        {
            expectedBots++;
            continue;
        }

        if (line.find("mgroup") == 0)
        {
            expectedBots = std::max(expectedBots, ParseMGroupSize(line));
        }
    }

    return expectedBots;
}

void TestRegistry::RegisterTest(const std::string& name, const std::vector<std::string>& script)
{
    sTestRegistry[name] = script;
}

bool TestRegistry::HasTest(const std::string& name)
{
    EnsureTestsRegistered();
    return sTestRegistry.find(name) != sTestRegistry.end();
}

std::vector<std::string> TestRegistry::GetTestScript(const std::string& name)
{
    EnsureTestsRegistered();
    auto it = sTestRegistry.find(name);
    if (it != sTestRegistry.end())
        return it->second;
    return {};
}

std::vector<std::string> TestRegistry::GetAvailableTests()
{
    EnsureTestsRegistered();
    std::vector<std::string> tests;
    for (const auto& pair : sTestRegistry)
        tests.push_back(pair.first);
    return tests;
}

void TestRegistry::GenerateMovementTests(int maxTests, float minDist, float maxDist)
{
    EnsureLocationsInit();
    GenerateMovementTestsImpl(maxTests, minDist, maxDist);
}

void TestRegistry::GenerateBossWalkTest()
{    
    DestinationList bossDestinations = sTravelMgr.GetDestinations(PlayerTravelInfo(), (uint32)TravelDestinationPurpose::Boss, {}, false);

     std::vector<std::string> instanceGroupTemplate = {
        "# instance progression with large group and dead-mob observation",
        "require bot is level=$(level)",
        "monitor dead mobs > $(dead_mobs_min) => pass \"Observed dead mobs in instance\"",
        "monitor time > $(timeout_s) => fail \"Timeout while traversing instance after <time elapsed> (mobs <mobs killed>, traveled <distance traveled> / wanted <distance wanted>)\"",
        "teleport $(instance_entry)",
        "mgroup size=$(group_size) gear=best",
        "gm visible on",
        "gm off",
        "wait 10",
        "${start_command}",
        "set destination $(boss_destination)",
        "wait 60",
        "not on map $(instance_entry) => abort \"Bot left instance map\"",
        "observe"};

    for (auto & destination : bossDestinations)
    {
        for (auto& point : destination->GetPoints())
        {
            if (!point->getMapEntry())
                continue;

            const MapEntry * mapEntry = point->getMapEntry();

            if (!mapEntry->IsDungeon())
                continue;

            const InstanceTemplate* instanceTemplate = point->getInstanceTemplate();
            if (!instanceTemplate)
                continue;

            CreatureInfo const* bossInfo = static_cast<BossTravelDestination*>(destination)->GetCreatureInfo();

            if (!bossInfo)
                continue;

            GuidPosition entry;

            std::string mapName = mapEntry->name[0];

            if (!ParseLocation(mapName, entry))
            {
                //strip spaces from mapName
                mapName.erase(remove_if(mapName.begin(), mapName.end(), isspace), mapName.end());
            }

            if (!ParseLocation(mapName, entry))
            {
                if (mapName.find(" ") != std::string::npos)
                    mapName = mapName.substr(0, mapName.find(" "));
            }

            if (!ParseLocation(mapName, entry))
            {
                for (auto& node : sTravelNodeMap.getNodes())
                {
                    if (node->getMapId() != mapEntry->MapID)
                        continue;

                    if (!node->isPortal())
                        continue;

                    for (auto& [otherNode, path] : *node->getLinks())
                    {
                        if (path->getPathType() != TravelNodePathType::areaTrigger)
                            continue;

                        mapName = mapEntry->name[0];
                        sTestLocations[mapName] = GuidPosition(ObjectGuid(),*otherNode->getPosition());
                    }
                }
            }

            if (!ParseLocation(mapName, entry))
                continue;

            std::string startCommand = ".bot p @tank co + mark rti";

            std::string maxPlayers = "5";

            if (mapEntry->IsRaid())
            {
#ifdef MANGOS_TWO
                maxPlayers = std::to_string(instanceTemplate->maxPlayers);
#else
                maxPlayers = "25";
#endif
                startCommand = ".bot r @tank co + mark rti";
            }

            std::string bossName = mapEntry->name[0] + std::string("_") + bossInfo->Name;

            std::replace(bossName.begin(), bossName.end(), ' ', '_');
            std::replace(bossName.begin(), bossName.end(), '\'', '_');

            std::transform(bossName.begin(), bossName.end(), bossName.begin(), ::tolower);

            sTestLocations[bossName] = GuidPosition(ObjectGuid(), *point);

            RegisterScenarioVariant("scenario_trash", bossName, instanceGroupTemplate, {
               {"timeout_s",        "1200"                                         },
               {"start_command",    startCommand                                   },
               {"level",            std::to_string(instanceTemplate->levelMin + 10)},
               {"group_size",       maxPlayers                                     },
               {"dead_mobs_min",    "5"                                            },
               {"instance_entry",   mapName                                        },
               {"boss_destination", bossName                                       }
            });
            
        }
    }

    
}

bool TestRegistry::LookupNamedLocation(const std::string& name, GuidPosition& out)
{
    EnsureLocationsInit();

    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    auto it = sTestLocations.find(lowerName);
    if (it != sTestLocations.end())
    {
        out = it->second;
        return true;
    }
    return false;
}

bool TestRegistry::ParseLocation(const std::string& str, GuidPosition& out)
{
    if (LookupNamedLocation(str, out))
        return true;

    out = GuidPosition(GuidPosition::CreationMask::UNKNOWN, str);

    if (out)
        return true;

    return false;
}

void TestRegistry::EnsureTestsRegistered()
{
    if (sTestsRegistered)
        return;

    sTestsRegistered = true;

    static std::string gmInvisible = "gm visible off";
    static std::string gmVisible = "gm visible on";

    static std::string needAlliance = "monitor faction horde => abort \"Bot needs to be alliance\"";
    static std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";

    static std::string Timeout2Min = "monitor time > 120 => fail \"Timeout: bot did not reach destination (traveled <distance traveled> / wanted <distance wanted>)\"";
    static std::string Timeout10Min = "monitor time > 600 => fail \"Timeout: bot did not reach destination (traveled <distance traveled> / wanted <distance wanted>)\"";

    //Movement tests
    RegisterTest("movement_walk_short_inside_ironforge", {gmInvisible, needAlive, Timeout2Min, "monitor distance to ironforge < 50 => pass \"Bot reached Ironforge gate\"", "teleport ironforge_outside", "set destination ironforge", "observe", gmVisible});

    RegisterTest("movement_walk_long_coldridge_ironforge", {gmInvisible, needAlive, Timeout10Min, "monitor distance to ironforge < 100 => pass \"Bot arrived at Ironforge\"", "teleport coldridge", "set destination ironforge", "observe", gmVisible});

    RegisterTest("movement_fly_short_ironforge_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport ironforge", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_darkshire_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport darkshire", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_westfall_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport westfall", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_wetlands_to_ironforge", {gmInvisible, needAlive, Timeout10Min, "monitor distance to ironforge < 100 => pass \"Bot arrived at Ironforge\"", "teleport wetlands", "set destination ironforge", "observe", gmVisible});

    RegisterTest("movement_walk_to_zg_entrance", {gmInvisible, needAlive, Timeout2Min, "monitor distance to zg_entrance < 50 => pass \"Bot reached ZG entrance\"", "teleport elwynn", "set destination zg_entrance", "observe", gmVisible});

    RegisterTest("movement_fly_stormwind_to_orgrimmar", {gmInvisible, needAlive, Timeout10Min, "monitor distance to orgrimmar < 100 => pass \"Bot arrived at Orgrimmar\"", "teleport stormwind", "set destination orgrimmar", "observe", gmVisible});

    RegisterTest("movement_fly_ironforge_to_orgrimmar", {gmInvisible, needAlive, Timeout10Min, "monitor distance to orgrimmar < 100 => pass \"Bot arrived at Orgrimmar\"", "teleport ironforge", "set destination orgrimmar", "observe", gmVisible});

    RegisterTest("movement_walk_long_redridge_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport redridge", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_duskwood_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport duskwood", "set destination stormwind", "observe", gmVisible});

    std::vector<std::string> skinningLootTemplate = {
        "# Grouped skinning/loot hold scenario",
        "monitor group size < 2 => fail \"Expected second bot in group\"",
        "monitor time > $(hold_s) => pass \"Group stayed stable during loot hold window\"",
        "monitor time > $(timeout_s) => fail \"Timeout in grouped skinning scenario\"",
        "spawn level=$(level) temporary=1 login=1",
        "wait 5",
        "form party",
        "wait 5",
        "teleport $(start_location)",
        "set destination $(walk_location)",
        "observe"
    };
    ::RegisterScenarioVariant("scenario_group_skinning_wait_loot", "default", skinningLootTemplate,
        {
            {"level", "60"},
            {"hold_s", "20"},
            {"timeout_s", "300"},
            {"start_location", "elwynn"},
            {"walk_location", "elwynn"}
        });

    std::vector<std::string> EquipTemplate = {
        "# equip upgrades with setup and expected equipment lists",
        gmInvisible,
        needAlive,
        "require bot is level=$(level) class=$(class) role=$(role) gear=empty",
        "monitor time > $(timeout_s) => pass \"Test complete items properly equiped\"",
        "give $(setup_equip_item_1)",
        "give $(setup_equip_item_2)",
        "do equip upgrades",
        "give $(bag_item_1)",
        "give $(bag_item_2)",
        "do equip upgrades",
        "wait 5",
        "require equip main hand=$(expected_item_1)",
        "require equip off hand=$(expected_item_2)",
        "observe",
        gmVisible
    };
    RegisterScenarioVariant("scenario_fury_equip_upgrades", "default", EquipTemplate,
        {
            {"timeout_s", "10"},
            {"level", "60"},
            {"class", "warrior"},
            {"role", "dps"},
            {"setup_equip_item_1", "8190"},
            {"setup_equip_item_2", "7687"},
            {"bag_item_1", "17015"},
            {"bag_item_2", "18805"},
            {"expected_item_1", "17015"},
            {"expected_item_2", "18805"}
        });
    
    /*
    std::vector<std::string> instanceGroupTemplate = {
        "# instance progression with large group and dead-mob observation",
        "require bot is level=60",
        "monitor dead mobs > $(dead_mobs_min) => pass \"Observed dead mobs in instance\"",
        "monitor time > $(timeout_s) => fail \"Timeout while traversing instance after <time elapsed> (mobs <mobs killed>, traveled <distance traveled> / wanted <distance wanted>)\"",
        "teleport $(instance_entry)",
        "mgroup size=$(group_size) gear=best",
        gmVisible,
        "gm off",
        "wait 10",
        ".bot r @tank co +mark rti",
        "set destination $(boss_destination)",
        "observe"
    };
    RegisterScenarioVariant("scenario_instance_group_progress", "zg_default", instanceGroupTemplate,
        {
            {"timeout_s", "1200"},
            {"group_size", "20"},
            {"dead_mobs_min", "0"},
            {"instance_entry", "zul'gurub"},
            {"boss_destination", "zg_boss1_room"}
        }); 
     */

    std::vector<std::string> followTemplate = {
        "# Follow catch-up between leader and spawned follower",
        gmInvisible,
        needAlive,
        "monitor spawn distance < $(follow_distance) => pass \"Follower caught up\"",
        "monitor time > $(timeout_s) => fail \"Timeout waiting for follower catch-up\"",
        "spawn level=$(level) temporary=1 login=1",
        "wait 5",
        "form party",
        "wait 5",
        "teleport $(leader_start)",
        "set destination $(leader_dest_a)",
        "wait destination 600",
        "set destination $(leader_dest_b)",
        "wait destination 600",
        "set destination $(leader_dest_a)",
        "wait destination 600",
        "set destination $(leader_dest_a)",
        "observe",
        gmVisible
    };
    RegisterScenarioVariant("scenario_follow_cross_zone", "default", followTemplate,
        {
            {"timeout_s", "1800"},
            {"follow_distance", "40"},
            {"level", "60"},
            {"leader_start", "ironforge_outside"},
            {"leader_dest_a", "ironforge"},
            {"leader_dest_b", "coldridge"}
        });

    std::vector<std::string> pullWhileTravelTemplate = {
        "# Pull progression while moving to destination behind mobs",
        "monitor dead mobs > $(kills_min) => pass \"Killed mobs while traveling\"",
        "monitor time > $(timeout_s) => fail \"Timeout while traversing mob route after <time elapsed> (mobs <mobs killed>, traveled <distance traveled> / wanted <distance wanted>)\"",
        "teleport $(start_location)",
        "set destination $(behind_mobs_destination)",
        "observe"
    };
    RegisterScenarioVariant("scenario_pull_while_traveling", "default", pullWhileTravelTemplate,
        {
            {"timeout_s", "600"},
            {"kills_min", "0"},
            {"start_location", "westfall"},
            {"behind_mobs_destination", "elwynn"}
        });

    GenerateMovementTests(1000, 5.0f, 100000.0f);
    GenerateBossWalkTest();
}

void TestRegistry::EnsureLocationsInit()
{
    ::EnsureLocationsInit();
}