#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/TravelNode.h"

#include <algorithm>
#include <sstream>

using namespace ai;

namespace
{
    using ScenarioParams = std::map<std::string, std::string>;

    bool ResolveInstanceEntry(const MapEntry* mapEntry, std::string& mapName, GuidPosition& entry)
    {


        TravelNode* best = nullptr;
        for (auto& node : sTravelNodeMap.getNodes())
        {
            if (node->getMapId() != mapEntry->MapID)
                continue;

            std::string nodeName = node->getName();
            std::transform(nodeName.begin(), nodeName.end(), nodeName.begin(), ::tolower);
            if (nodeName.find("entrance") != std::string::npos)
            {
                best = node;
                break;
            }

            if (!best && node->isPortal())
                best = node;
        }

        if (!best)
        {
            static std::set<uint32> loggedMaps;
            if (loggedMaps.insert(mapEntry->MapID).second)
                sLog.outError("[TESTGEN] no travel node on instance map %u (%s) - scenario tests skipped for it",
                    mapEntry->MapID, mapEntry->name[0]);
            return false;
        }


        mapName = mapEntry->name[0];
        mapName.erase(std::remove_if(mapName.begin(), mapName.end(), ::isspace), mapName.end());
        mapName += "_inside";
        TestRegistry::RegisterNamedLocation(mapName, GuidPosition(ObjectGuid(), *best->getPosition()));
        return TestRegistry::ParseLocation(mapName, entry) && entry.getMapId() == mapEntry->MapID;
    }
}

std::string TestRegistry::ApplyScenarioParams(const std::string& line, const std::map<std::string, std::string>& params)
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

std::vector<std::string> TestRegistry::ApplyScenarioParams(const std::vector<std::string>& script,
    const std::map<std::string, std::string>& params)
{
    std::vector<std::string> expanded;
    expanded.reserve(script.size());
    for (const std::string& line : script)
        expanded.push_back(ApplyScenarioParams(line, params));

    return expanded;
}

void TestRegistry::GenerateBossWalkTest()
{
    // maxDistance 0: the generator uses an EMPTY PlayerTravelInfo whose center is the map-0 origin
    // (0,0,0). The default maxDistance of 10000 yd then silently dropped every destination whose
    // portal was farther than that from the origin - Zul'Gurub (map 309) bosses were all >10000 and
    // generated no tests at all (BL-45). The generator teleports bots to the instance, so travel
    // distance is irrelevant here; only the FLT_MAX unpathable-map check should apply.
    DestinationList bossDestinations = sTravelMgr.GetDestinations(PlayerTravelInfo(), (uint32)TravelDestinationPurpose::Boss, {}, false, 0);

    std::vector<std::string> instanceGroupTemplate = {
        "# instance progression with large group and dead-mob observation",
        "require bot is level=$(level)",
        "monitor dead mobs > $(dead_mobs_min) => pass \"Observed dead mobs in instance\"",
        "monitor party xp > $(party_xp_min) => pass \"Party gained xp clearing trash\"",
        "monitor time > $(timeout_s) => fail \"Timeout while traversing instance after <time elapsed> (mobs <mobs killed>, traveled <distance traveled> / wanted <distance wanted>)\"",
        "mgroup size=$(group_size) gear=best",
        "gm visible on",
        "gm off",
        "wait 30",
        "teleport $(instance_entry)",
        "teleport group",
        "wait 10",
        "$(start_command)",
        "set destination $(boss_destination)",
        "wait 60",
        "monitor not on map $(instance_entry) => abort \"Bot left instance map\"",
        "monitor group size < $(group_size) => abort \"Group dissolved\"",
        "observe"};

    for (auto& destination : bossDestinations)
    {
        for (auto& point : destination->GetPoints())
        {
            if (!point->getMapEntry())
                continue;

            const MapEntry* mapEntry = point->getMapEntry();

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
                mapName.erase(remove_if(mapName.begin(), mapName.end(), isspace), mapName.end());
            }

            if (!ParseLocation(mapName, entry))
            {
                if (mapName.find(" ") != std::string::npos)
                    mapName = mapName.substr(0, mapName.find(" "));
            }

            if (!ResolveInstanceEntry(mapEntry, mapName, entry))
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

            RegisterNamedLocation(bossName, GuidPosition(ObjectGuid(), *point));

            RegisterTest("scenario_trash_" + bossName, ApplyScenarioParams(instanceGroupTemplate, ScenarioParams{
                {"timeout_s",        "1200"                                         },
                {"start_command",    startCommand                                   },
                {"level",            std::to_string(instanceTemplate->levelMin + 10)},
                {"group_size",       maxPlayers                                     },
                {"dead_mobs_min",    "5"                                            },
                {"party_xp_min",     "5000"                                         },
                {"instance_entry",   mapName                                        },
                {"boss_destination", bossName                                       }
            }));
        }
    }
}

void TestRegistry::GenerateBossEncounterTest()
{
    // maxDistance 0: the generator uses an EMPTY PlayerTravelInfo whose center is the map-0 origin
    // (0,0,0). The default maxDistance of 10000 yd then silently dropped every destination whose
    // portal was farther than that from the origin - Zul'Gurub (map 309) bosses were all >10000 and
    // generated no tests at all (BL-45). The generator teleports bots to the instance, so travel
    // distance is irrelevant here; only the FLT_MAX unpathable-map check should apply.
    DestinationList bossDestinations = sTravelMgr.GetDestinations(PlayerTravelInfo(), (uint32)TravelDestinationPurpose::Boss, {}, false, 0);

    std::vector<std::string> bossEncounterTemplate = {
        "# Boss encounter test - teleport near boss, clear trash, fight boss",
        "require bot is level=$(level)",
        "monitor mob $(boss_entry) is dead => pass \"Boss $(boss_name) killed\"",
        "monitor time > $(timeout_s) => fail \"Timeout: boss not killed after <time elapsed>\"",
        "monitor party wiped => fail \"Party wiped on $(boss_name)\"",
        "monitor bot dead => abort \"Bot died: released to graveyard and timed out in revive\"",
        "monitor not on map $(instance_entry) => abort \"Bot left instance map\"",
        "monitor group size < $(group_size) => abort \"Group dissolved\"",
        "mgroup size=$(group_size) gear=best",
        "gm on",
        "wait 30",
        "teleport $(instance_entry)",
        "wait 5",
        "teleport $(boss_destination)",
        "teleport group",
        "wait 5",
        "clear except=$(boss_entry) radius500",
        "pull $(boss_entry)",
        "clear except=$(boss_entry) radius500",
        "$(start_command)",
        "wait 3",
        "gm off",
        "pull $(boss_entry)",
        "observe"};

    for (auto& destination : bossDestinations)
    {
        for (auto& point : destination->GetPoints())
        {
            if (!point->getMapEntry())
                continue;

            const MapEntry* mapEntry = point->getMapEntry();

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
                mapName.erase(remove_if(mapName.begin(), mapName.end(), isspace), mapName.end());
            }

            if (!ParseLocation(mapName, entry))
            {
                if (mapName.find(" ") != std::string::npos)
                    mapName = mapName.substr(0, mapName.find(" "));
            }

            if (!ResolveInstanceEntry(mapEntry, mapName, entry))
                continue;

            std::ostringstream bossCoords;
            bossCoords << point->getMapId() << ";"
                << point->getX() << ";"
                << point->getY() << ";"
                << point->getZ() << ";0";

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

            RegisterTest("scenario_boss_" + bossName, ApplyScenarioParams(bossEncounterTemplate, ScenarioParams{
                {"timeout_s",        "600"                                          },
                {"start_command",    startCommand                                   },
                {"level",            std::to_string(instanceTemplate->levelMin + 10)},
                {"group_size",       maxPlayers                                     },
                {"instance_entry",   mapName                                        },
                {"boss_destination", bossCoords.str()                               },
                {"boss_entry",       std::to_string(bossInfo->Entry)                },
                {"boss_name",        bossInfo->Name                                 }
            }));
        }
    }
}

// BL-45 part 2: the layer-2 reach test. The trash template observes clearing, the encounter
// template teleports next to the boss and fights - but "enter the dungeon and TRAVEL to the boss
// (alive)" had no coverage. The reach template enters at the instance entry, sets the boss as the
// travel destination and passes when the party is near the boss while it is still alive.
void TestRegistry::GenerateBossReachTest()
{
    DestinationList bossDestinations = sTravelMgr.GetDestinations(PlayerTravelInfo(), (uint32)TravelDestinationPurpose::Boss, {}, false, 0);

    std::vector<std::string> bossReachTemplate = {
        "# reach boss test - enter instance, travel to the boss, party near boss with boss alive",
        "require bot is level=$(level)",
        "monitor distance to $(boss_location) < $(reach_dist) => pass \"Party reached $(boss_name) after <time elapsed>\"",
        "monitor mob $(boss_entry) is dead => fail \"Boss $(boss_name) died before the party reached it\"",
        "monitor time > $(timeout_s) => fail \"Timeout: boss not reached after <time elapsed> (traveled <distance traveled>)\"",
        "monitor party wiped => fail \"Party wiped on the way to $(boss_name)\"",
        "monitor bot dead => abort \"Bot died on the way to $(boss_name)\"",
        "monitor not on map $(instance_entry) => abort \"Bot left instance map\"",
        "monitor group size < $(group_size) => abort \"Group dissolved\"",
        "mgroup size=$(group_size) gear=best",
        "gm visible on",
        "gm off",
        "wait 30",
        "teleport $(instance_entry)",
        "teleport group",
        "wait 10",
        "$(start_command)",
        "set destination $(boss_destination)",
        "observe"};

    for (auto& destination : bossDestinations)
    {
        for (auto& point : destination->GetPoints())
        {
            if (!point->getMapEntry())
                continue;

            const MapEntry* mapEntry = point->getMapEntry();

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

            if (!ResolveInstanceEntry(mapEntry, mapName, entry))
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
            std::replace(bossName.begin(), bossName.end(), 39, 95); // 39 = apostrophe, 95 = underscore
            std::transform(bossName.begin(), bossName.end(), bossName.begin(), ::tolower);

            RegisterNamedLocation(bossName, GuidPosition(ObjectGuid(), *point));

            RegisterTest("scenario_reach_" + bossName, ApplyScenarioParams(bossReachTemplate, ScenarioParams{
                {"timeout_s",        "1800"                                         },
                {"start_command",    startCommand                                   },
                {"level",            std::to_string(instanceTemplate->levelMin + 10)},
                {"group_size",       maxPlayers                                     },
                {"reach_dist",       "60"                                           },
                {"instance_entry",   mapName                                        },
                {"boss_location",    bossName                                       },
                {"boss_destination", bossName                                       },
                {"boss_entry",       std::to_string(bossInfo->Entry)                },
                {"boss_name",        bossInfo->Name                                 }
            }));
        }
    }
}
void TestRegistry::RegisterInstanceTests()
{
    GenerateBossWalkTest();
    GenerateBossEncounterTest();
    GenerateBossReachTest();
}
