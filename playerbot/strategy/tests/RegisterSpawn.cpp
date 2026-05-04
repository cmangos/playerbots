#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "playerbot/ChatHelper.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/RandomPlayerbotFactory.h"
#include "Globals/ObjectMgr.h"

#include <algorithm>

using namespace ai;

namespace
{
    std::string ToLowerToken(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        std::replace(value.begin(), value.end(), ' ', '_');
        std::replace(value.begin(), value.end(), '\'', '_');
        return value;
    }

    bool IsPlayableClass(uint8 cls)
    {
        if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE))
            return false;

        return sChrClassesStore.LookupEntry(cls) != nullptr;
    }

    bool IsPlayableRace(uint8 race)
    {
        if (!((1 << (race - 1)) & RACEMASK_ALL_PLAYABLE))
            return false;

        return sChrRacesStore.LookupEntry(race) != nullptr;
    }

    int32 DeterministicQualityFor(uint32 level, uint8 race, uint8 cls)
    {
        // Only check gear quality at level 20+ where DB has sufficient items
        if (level < 20)
            return -1;

        std::vector<int32> choices;
        choices.push_back(-1);  // none
        choices.push_back(ITEM_QUALITY_UNCOMMON);
        if (level >= 20)
            choices.push_back(ITEM_QUALITY_RARE);
        if (level >= 40)
            choices.push_back(ITEM_QUALITY_EPIC);

        uint32 seed = level * 73856093u ^ race * 19349663u ^ cls * 83492791u;
        return choices[seed % choices.size()];
    }

    std::string QualityName(int32 quality)
    {
        switch (quality)
        {
            case ITEM_QUALITY_NORMAL: return "white";
            case ITEM_QUALITY_UNCOMMON: return "green";
            case ITEM_QUALITY_RARE: return "blue";
            case ITEM_QUALITY_EPIC: return "epic";
            default: return "none";
        }
    }
}

void TestRegistry::RegisterSpawnTests()
{
    RandomPlayerbotFactory availabilityInit(0);
    (void)availabilityInit;

    for (uint8 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        if (!IsPlayableClass(cls))
            continue;

        for (uint8 race = 1; race < MAX_RACES; ++race)
        {
            if (!IsPlayableRace(race))
                continue;

            if (!RandomPlayerbotFactory::isAvailableRace(cls, race))
                continue;

            PlayerInfo const* pInfo = sObjectMgr.GetPlayerInfo(race, cls);
            if (!pInfo)
                continue;

            std::string raceToken = ToLowerToken(ChatHelper::formatRace(race));
            std::string classToken = ToLowerToken(ChatHelper::formatClass(cls));
            std::string spawnLoc = "spawn_" + raceToken + "_" + classToken;
            RegisterNamedLocation(spawnLoc, GuidPosition(ObjectGuid(), WorldPosition(pInfo->mapId, pInfo->positionX, pInfo->positionY, pInfo->positionZ, pInfo->orientation)));

            RegisterTest("spawn_" + raceToken + "_" + classToken,
            {
                "require bot is level=1 race=" + raceToken + " class=" + classToken,
                "monitor time > 20 => fail \"Timeout without reply\"",
                "monitor distance to " + spawnLoc + " > 50 => fail \"Bot is not near race spawn location\"",
                "monitor starter gear count < 2 => fail \"Bot has less than 2 starter gear items equipped\"",
                "record",
                "who",
                "monitor outgoing message * => pass \"Bot replied to who\"",
                "observe"
            });
        }
    }
}

void TestRegistry::RegisterRandomizeTests()
{
    RandomPlayerbotFactory availabilityInit(0);
    (void)availabilityInit;

    uint32 maxLevel = std::min(sPlayerbotAIConfig.randomBotMaxLevel, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL));
    for (uint32 level = 2; level <= maxLevel; ++level)
    {
        for (uint8 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            if (!IsPlayableClass(cls))
                continue;

            for (uint8 race = 1; race < MAX_RACES; ++race)
            {
                if (!IsPlayableRace(race))
                    continue;

                if (!RandomPlayerbotFactory::isAvailableRace(cls, race))
                    continue;

                PlayerInfo const* pInfo = sObjectMgr.GetPlayerInfo(race, cls);
                if (!pInfo)
                    continue;

                std::string raceToken = ToLowerToken(ChatHelper::formatRace(race));
                std::string classToken = ToLowerToken(ChatHelper::formatClass(cls));

                int32 quality = DeterministicQualityFor(level, race, cls);
                std::string qualityName = QualityName(quality);

                std::string requireLine = "require bot is level=" + std::to_string(level) + " class=" + classToken + " race=" + raceToken;
                if (quality != -1)
                    requireLine += " gear=" + qualityName;

                std::vector<std::string> script = {
                    requireLine,
                    "monitor time > 20 => fail \"Timeout without reply\"",
                    "monitor area level diff > 10 => fail \"Bot is not in an appropriate zone for level\"",
                    "record",
                    "who",
                    "monitor outgoing message * => pass \"Bot replied to who\"",
                    "observe"
                };

                if (quality != -1)
                {
                    script.insert(script.begin() + 3,
                        "monitor equip quality " + qualityName + " < 2 => fail \"Bot has fewer than 2 equipped items of expected quality\"");
                }

                RegisterTest("Randomize_" + std::to_string(level) + "_" + classToken + "_" + raceToken, script);
            }
        }
    }
}
