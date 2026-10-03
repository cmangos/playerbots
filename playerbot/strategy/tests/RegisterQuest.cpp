#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "Quests/QuestDef.h"
#include "Globals/ObjectMgr.h"
#include "GameEvents/GameEventMgr.h"
#include <cmath>

#include <cctype>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

using namespace ai;

// =====================================================
// Quest suite generator - BL-46
//
// Generates one test per quest (and per objective, per level profile):
//   pickup           - teleport to the quest giver, the bot must accept the quest via its AI
//   obj<k>_tick      - quest accepted (state injected), teleport to objective k's spawn,
//                      pass on any objective progress
//   obj<k>_clear     - same, but every other objective is force-satisfied at setup and the
//                      pass requires the full objective count
//   handin           - quest accepted + completed (state injected), teleport to the taker,
//                      the bot must hand in via its AI
//
// Level profiles per test: the quest level, the edge-XP level (quest level + 9 - the highest
// character level at which Quest::GetXPReward still returns >0) and max level (80).
// Naming: quest_<faction>_<minlvl>_<qlvl>_<title>_<type>_l<level>
//
// First pass scope: creature (kill) objectives on overworld spawns only; gameobject and
// item-only objectives, dungeon/raid objectives and event/escort quests are skipped with
// [QUESTGEN] counters.
// =====================================================

namespace
{
    struct QuestSpawn
    {
        uint32 mapId = 0;
        float x = 0, y = 0, z = 0;
        bool continent = false;
        uint32 guid = 0;   // creature.spawn guid - real guid so injected rpg targets pass IsCreature/IsGameObject checks
        uint32 entry = 0;
    };

    using QuestSpawnMap = std::unordered_map<uint32, QuestSpawn>;

    uint32 const QUEST_LEVEL_CAP = 80;
    uint32 const MAX_QUESTGEN_SKIP_LOGS = 20;
    uint32 const MAX_CHAIN_DEPTH = 12;

    std::string SanitizeQuestName(std::string title)
    {
        for (char& c : title)
            c = static_cast<char>(std::isalnum(static_cast<unsigned char>(c)) ? std::tolower(static_cast<unsigned char>(c)) : '_');

        std::string out;
        bool lastUnderscore = false;
        for (char c : title)
        {
            if (c == '_')
            {
                if (!lastUnderscore)
                    out += '_';
                lastUnderscore = true;
            }
            else
            {
                out += c;
                lastUnderscore = false;
            }
        }

        while (!out.empty() && out.front() == '_')
            out.erase(out.begin());
        while (!out.empty() && out.back() == '_')
            out.pop_back();

        if (out.size() > 40)
            out.resize(40);

        return out;
    }

    // "a"/"h"/"n" from the RequiredRaces mask; 0 (both/all) counts as neutral.
    std::string FactionToken(uint32 requiredRaces)
    {
        bool horde = (requiredRaces & RACEMASK_HORDE) != 0;
        bool alliance = (requiredRaces & RACEMASK_ALLIANCE) != 0;

        if (horde && alliance)
            return "n";
        if (horde)
            return "h";
        if (alliance)
            return "a";
        return "n";
    }

    // One require line drives both the in-test assertions (RequireBotIs) and the bot creation
    // parameters (GetBotCreationRequirement feeds CreateBot, which takes the same key=value
    // tokens). Classes/races are emitted numerically - ChatHelper parses ids as well.
    //
    // Defaults: no class restriction -> paladin (solid solo class). Race is left to the factory
    // (GetRandomRace keeps race/class valid) unless the quest restricts the race; a quest that
    // restricts BOTH race and class is skipped by the caller (combination risk).
    std::string RequireLine(uint32 level, Quest const* quest)
    {
        std::ostringstream out;
        out << "require bot is level=" << level;

        uint32 classMask = quest->GetRequiredClasses();
        uint32 raceMask = quest->GetRequiredRaces();

        // Race/class masks in this tree are 1 << (id - 1) (convertEnumToFlag).
        if (classMask)
        {
            for (uint32 cls = 1; cls <= 11; ++cls)
            {
                if (classMask & (1 << (cls - 1)))
                {
                    out << " class=" << cls;
                    break;
                }
            }
        }
        else
        {
            out << " class=2"; // paladin
        }

        if (!classMask && raceMask)
        {
            // Only emit a race the chosen class can actually be: Player::Create rejects invalid
            // race/class pairs (e.g. orc paladin), and a failed creation wedges the queued test.
            for (uint32 race = 1; race <= 11; ++race)
            {
                if ((raceMask & (1 << (race - 1))) && sObjectMgr.GetPlayerInfo(race, 2))
                {
                    out << " race=" << race;
                    break;
                }
            }
        }

        if ((raceMask & RACEMASK_ALLIANCE) && !(raceMask & RACEMASK_HORDE))
            out << " faction=alliance";
        else if ((raceMask & RACEMASK_HORDE) && !(raceMask & RACEMASK_ALLIANCE))
            out << " faction=horde";
        else
            out << " faction=alliance"; // neutral/both: deterministic pool

        return out.str();
    }

    // Walk the PrevQuestId chain and emit "reward quest <id>" spoof lines so the bot can take
    // the target quest. Returns false when the chain is broken or "either/or" (negative ids).
    bool ChainSpoofLines(Quest const* quest, std::vector<std::string>& lines)
    {
        int32 prev = quest->GetPrevQuestId();
        uint32 depth = 0;

        while (prev > 0 && depth < MAX_CHAIN_DEPTH)
        {
            Quest const* prevQuest = sObjectMgr.GetQuestTemplate(static_cast<uint32>(prev));
            if (!prevQuest)
                return false;

            std::ostringstream out;
            out << "reward quest " << prev;
            lines.push_back(out.str());

            prev = prevQuest->GetPrevQuestId();
            ++depth;
        }

        return prev <= 0;
    }
}

void TestRegistry::RegisterQuestSuiteTests()
{
    std::unordered_map<uint32, QuestSpawn> giverSpawns;
    std::unordered_map<uint32, QuestSpawn> takerSpawns;

    // creature_questrelation / creature_involvedrelation: `id` = creature entry, `quest` = quest id.
    if (auto result = WorldDatabase.Query(
        "SELECT qr.quest, c.map, c.position_x, c.position_y, c.position_z, c.guid, c.id FROM creature_questrelation qr "
        "JOIN creature c ON c.id = qr.id"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[0].GetUInt32();
            uint32 mapId = fields[1].GetUInt32();
            MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
            bool continent = mapEntry && mapEntry->IsContinent();
            if (!giverSpawns.count(questId))
                giverSpawns[questId] = { mapId, fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), continent, fields[5].GetUInt32(), fields[6].GetUInt32() };
            else if (!giverSpawns[questId].continent && continent)
                giverSpawns[questId] = { mapId, fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), continent, fields[5].GetUInt32(), fields[6].GetUInt32() };
        } while (result->NextRow());
    }

    if (auto result = WorldDatabase.Query(
        "SELECT ir.quest, c.map, c.position_x, c.position_y, c.position_z, c.guid, c.id FROM creature_involvedrelation ir "
        "JOIN creature c ON c.id = ir.id"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 questId = fields[0].GetUInt32();
            uint32 mapId = fields[1].GetUInt32();
            MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
            bool continent = mapEntry && mapEntry->IsContinent();
            if (!takerSpawns.count(questId))
                takerSpawns[questId] = { mapId, fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), continent, fields[5].GetUInt32(), fields[6].GetUInt32() };
            else if (!takerSpawns[questId].continent && continent)
                takerSpawns[questId] = { mapId, fields[2].GetFloat(), fields[3].GetFloat(), fields[4].GetFloat(), continent, fields[5].GetUInt32(), fields[6].GetUInt32() };
        } while (result->NextRow());
    }

    // Collect every creature objective entry, then resolve one spawn per entry in batched queries.
    std::unordered_map<uint32, std::unordered_set<uint32>> questObjectiveEntries;
    std::unordered_set<uint32> objectiveEntrySet;

    for (auto const& [questId, questPtr] : sObjectMgr.GetQuestTemplates())
    {
        Quest const* quest = questPtr.get();
        if (!quest)
            continue;

        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32 entry = quest->ReqCreatureOrGOId[i];
            if (entry > 0 && !quest->ReqSpell[i])
            {
                questObjectiveEntries[questId].insert(static_cast<uint32>(entry));
                objectiveEntrySet.insert(static_cast<uint32>(entry));
            }
        }
    }

    // BL-47(a) follow-up (five_signets / Field Marshal Snowfall 15701): the QUEST-level event check
    // below does not cover creatures whose SPAWN is event-linked. Snowfall's spawn
    // (game_event_creature event 120 = AV weekend) does not exist while the event is off, so the
    // accept can never fire and every pickup run burned its full timeout. Load creature-spawn ->
    // event links once; giver/taker spawns and objective spawns that are event-gated with no active
    // event are treated as missing below.
    std::unordered_map<uint32, std::vector<int16>> creatureSpawnEvents;
    if (auto result = WorldDatabase.Query("SELECT guid, event FROM game_event_creature"))
    {
        do
        {
            Field* fields = result->Fetch();
            creatureSpawnEvents[fields[0].GetUInt32()].push_back(fields[1].GetInt16());
        } while (result->NextRow());
    }

    // True when the spawn exists regardless of events (no event links, or at least one active event).
    auto spawnAvailable = [&creatureSpawnEvents](uint32 spawnGuid) -> bool
    {
        auto it = creatureSpawnEvents.find(spawnGuid);
        if (it == creatureSpawnEvents.end())
            return true;
        for (int16 eventId : it->second)
        {
            if (eventId <= 0)
                return true;
            if (sGameEventMgr.IsActiveEvent(static_cast<uint16>(eventId)))
                return true;
        }
        return false;
    };

    // First spawn wins; a continent spawn replaces a non-continent one.
    uint32 skipEventSpawnObj = 0; // objective spawns skipped for inactive-event gating (see load above)
    std::unordered_map<uint32, QuestSpawn> objectiveSpawns;
    std::vector<uint32> entryList(objectiveEntrySet.begin(), objectiveEntrySet.end());
    for (size_t offset = 0; offset < entryList.size(); offset += 500)
    {
        std::ostringstream in;
        for (size_t i = offset; i < offset + 500 && i < entryList.size(); ++i)
            in << (i > offset ? "," : "") << entryList[i];

        auto result = WorldDatabase.PQuery(
            "SELECT c.id, c.guid, c.map, c.position_x, c.position_y, c.position_z FROM creature c WHERE c.id IN (%s)",
            in.str().c_str());
        if (!result)
            continue;

        do
        {
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            uint32 guid = fields[1].GetUInt32();
            uint32 mapId = fields[2].GetUInt32();
            MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
            bool continent = mapEntry && mapEntry->IsContinent();

            // Event-gated spawn with no active event: this creature does not exist in the world
            // (five_signets/Snowfall class). Do not let it become the representative spawn.
            if (!spawnAvailable(guid))
            {
                ++skipEventSpawnObj;
                continue;
            }

            auto it = objectiveSpawns.find(entry);
            if (it == objectiveSpawns.end())
                objectiveSpawns[entry] = { mapId, fields[3].GetFloat(), fields[4].GetFloat(), fields[5].GetFloat() };
            else if (continent)
                it->second = { mapId, fields[3].GetFloat(), fields[4].GetFloat(), fields[5].GetFloat() };
        } while (result->NextRow());
    }

    // Downgrade: an entry with only non-continent spawns is skipped per objective below by checking
    // the spawn's map - remember which maps are continents.
    auto isOverworldSpawn = [&](QuestSpawn const& spawn)
    {
        MapEntry const* mapEntry = sMapStore.LookupEntry(spawn.mapId);
        return mapEntry && mapEntry->IsContinent();
    };

    // Event quests are only offered while their game event is live (Quest::IsActive gates
    // CanSeeStartQuest, so the dialog status stays NONE and the accept chain can never fire).
    // A force-complete spoof cannot fix that. Load quest -> event links once and pre-skip quests
    // whose events are all inactive (the childrens_week 172 case).
    std::unordered_map<uint32, std::vector<int16>> questEvents;
    if (auto result = WorldDatabase.Query("SELECT quest, event FROM game_event_quest"))
    {
        do
        {
            Field* fields = result->Fetch();
            questEvents[fields[0].GetUInt32()].push_back(fields[1].GetInt16());
        } while (result->NextRow());
    }

    // BL-47(a) follow-up: giver/taker spawn event-gating is handled by `spawnAvailable` (loaded above).
    // True when the spawn exists regardless of events (no event links, or at least one active event).

    uint32 pickupTests = 0, handinTests = 0, objTests = 0;
    uint32 pickupTravelTests = 0, pickupAimedTests = 0;
    uint32 skipNoGiver = 0, skipNoTaker = 0, skipNoSpawn = 0, skipGated = 0, skipChain = 0, skipGatedCombo = 0, skipNoLevel = 0;
    uint32 skipEventSpawn = 0;
    uint32 skipEventQuest = 0, skipCondition = 0, skipBreadcrumb = 0, skipZeroCount = 0;
    uint32 skipLogs = 0;

    auto logSkip = [&](char const* reason, uint32 questId)
    {
        if (skipLogs < MAX_QUESTGEN_SKIP_LOGS)
        {
            sLog.outError("[QUESTGEN] skip quest %u: %s", questId, reason);
            ++skipLogs;
        }
    };

    for (auto const& [questId, questPtr] : sObjectMgr.GetQuestTemplates())
    {
        Quest const* quest = questPtr.get();
        if (!quest)
            continue;

        if (quest->HasQuestFlag(QUEST_FLAGS_WEEKLY) ||
#ifndef MANGOSBOT_ZERO
            quest->HasQuestFlag(QUEST_FLAGS_DAILY) || quest->HasQuestFlag(QUEST_FLAGS_UNAVAILABLE) ||
#endif
            quest->HasQuestFlag(QUEST_FLAGS_AUTO_REWARDED) ||
            quest->HasSpecialFlag(QUEST_SPECIAL_FLAG_EXPLORATION_OR_EVENT) ||
            quest->HasQuestFlag(QUEST_FLAGS_RAID))
        {
            ++skipGated;
            logSkip("gated (daily/weekly/event/raid/unavailable)", questId);
            continue;
        }

        if (quest->GetPrevQuestId() < 0)
        {
            ++skipChain;
            logSkip("either/or prerequisite chain", questId);
            continue;
        }

        uint32 classMask = quest->GetRequiredClasses();
        uint32 raceMask = quest->GetRequiredRaces();
        if (classMask && raceMask)
        {
            ++skipGatedCombo;
            logSkip("race and class restricted (combo risk)", questId);
            continue;
        }

        if (!classMask && raceMask)
        {
            // Default paladin host: skip quests whose race mask offers no paladin-valid race
            // (e.g. the horde mask alone would make an orc paladin - invalid).
            bool validPair = false;
            for (uint32 race = 1; race <= 11 && !validPair; ++race)
                if ((raceMask & (1 << (race - 1))) && sObjectMgr.GetPlayerInfo(race, 2))
                    validPair = true;
            if (!validPair)
            {
                ++skipGatedCombo;
                logSkip("race mask has no valid race/class pair", questId);
                continue;
            }
        }

        // Skill/reputation gates make pickup acceptance impossible for a fresh bot.
        if (quest->GetRequiredSkillValue() || quest->GetRequiredMinRepFaction())
        {
            ++skipGatedCombo;
            logSkip("skill/reputation gated", questId);
            continue;
        }

        // Event quest with no active event: the giver does not offer it at all.
        {
            auto evIt = questEvents.find(questId);
            if (evIt != questEvents.end())
            {
                bool anyActive = false;
                for (int16 eventId : evIt->second)
                {
                    if (eventId <= 0) { anyActive = true; break; }
                    if (sGameEventMgr.IsActiveEvent(static_cast<uint16>(eventId))) { anyActive = true; break; }
                }
                if (!anyActive)
                {
                    ++skipEventQuest;
                    logSkip("event quest (event not active)", questId);
                    continue;
                }
            }
        }

        // Player-condition gates (player_condition) cannot be spoofed by force-completing quests.
        if (quest->GetRequiredCondition())
        {
            ++skipCondition;
            logSkip("required condition gate", questId);
            continue;
        }

        // Breadcrumb quests are only offered while the quest they point at is takeable
        // (SatisfyQuestBreadcrumbQuest) - not guaranteed for our injected hosts.
        if (quest->GetBreadcrumbForQuestId())
        {
            ++skipBreadcrumb;
            logSkip("breadcrumb quest (target must be takeable)", questId);
            continue;
        }

        // Raid/escort/legendary/dungeon-finder quest types are out of scope for the first pass.
        uint32 questType = quest->GetType();
        if (questType == QUEST_TYPE_RAID ||
#ifdef MANGOSBOT_TWO
            questType == QUEST_TYPE_RAID_10 || questType == QUEST_TYPE_RAID_25 || quest->IsDungeonFinderQuest() ||
#endif
            questType == QUEST_TYPE_ESCORT || questType == QUEST_TYPE_LEGENDARY ||
            questType == QUEST_TYPE_WORLD_EVENT)
        {
            ++skipGated;
            logSkip("raid/escort/legendary/event quest type", questId);
            continue;
        }

        int32 questLevel = quest->GetQuestLevel();
        if (questLevel < 1)
            questLevel = static_cast<int32>(quest->GetMinLevel());
        if (questLevel < 1)
        {
            ++skipNoLevel;
            logSkip("no usable quest level", questId);
            continue;
        }

        // Level profiles: quest level (clamped into [minLevel, cap]), edge-XP level, cap.
        std::unordered_set<uint32> levels;
        uint32 minLevel = std::max<uint32>(1, quest->GetMinLevel());
        uint32 atLevel = std::min(QUEST_LEVEL_CAP, std::max(minLevel, static_cast<uint32>(questLevel)));
        uint32 edgeLevel = std::min(QUEST_LEVEL_CAP, std::max(minLevel, static_cast<uint32>(questLevel) + 9));
        levels.insert(atLevel);
        levels.insert(edgeLevel);
        levels.insert(QUEST_LEVEL_CAP);

        std::string base = "quest_" + FactionToken(raceMask) + "_" + std::to_string(minLevel) + "_" +
            std::to_string(questLevel) + "_" + SanitizeQuestName(quest->GetTitle()) + "_" + std::to_string(questId);

        auto const& giver = giverSpawns.find(questId);
        auto const& taker = takerSpawns.find(questId);

        bool haveGiver = giver != giverSpawns.end() && giver->second.continent;
        bool haveTaker = taker != takerSpawns.end() && taker->second.continent;

        // Event-gated spawns (no active event) are as good as missing: the giver/taker NPC does not
        // exist in the world, so the AI can never interact with it (five_signets 8846 / Snowfall).
        if (haveGiver && !spawnAvailable(giver->second.guid))
        {
            ++skipEventSpawn;
            logSkip("giver spawn event-gated (event not active)", questId);
            haveGiver = false;
        }
        if (haveTaker && !spawnAvailable(taker->second.guid))
        {
            ++skipEventSpawn;
            logSkip("taker spawn event-gated (event not active)", questId);
            haveTaker = false;
        }

        std::vector<std::string> chainLines;
        bool chainOk = ChainSpoofLines(quest, chainLines);
        if (!chainOk)
        {
            ++skipChain;
            logSkip("broken prerequisite chain", questId);
            continue;
        }

        // ---- pickup (AI must accept the quest at the giver; prerequisites spoofed) ----
        if (haveGiver)
        {
            std::ostringstream locName;
            locName << "quest_" << questId << "_giver";
            // Real creature guid, not coords-only: RpgStartQuestTrigger::IsActive bails out when the
            // injected rpg target is neither creature nor gameobject, so a guid-less location would
            // make 'set rpg target' useless for the accept chain (BL-46 accept-deadlock).
            RegisterNamedLocation(locName.str(), GuidPosition(ObjectGuid(HIGHGUID_UNIT, giver->second.entry, giver->second.guid), WorldPosition(giver->second.mapId, giver->second.x, giver->second.y, giver->second.z)));

            QuestSpawn const& gp = giver->second;
            float ang = (questId % 100) * 0.0628f; // deterministic spread
            float ox = gp.x + 90.0f * cos(ang);
            float oy = gp.y + 90.0f * sin(ang);
            std::ostringstream nearName;
            nearName << "quest_" << questId << "_giver_near";
            RegisterNamedLocation(nearName.str(), GuidPosition(ObjectGuid(), WorldPosition(gp.mapId, ox, oy, gp.z)));

            for (uint32 level : levels)
            {
                std::vector<std::string> lines;
                lines.push_back("# quest pickup test " + std::to_string(questId));
                lines.push_back(RequireLine(level, quest));
                for (auto const& line : chainLines)
                    lines.push_back(line);
                lines.push_back("monitor quest active " + std::to_string(questId) + " => pass \"Quest picked up\"");
                lines.push_back("monitor time > 300 => fail \"Timeout: quest not picked up after <time elapsed>\"");
                lines.push_back("monitor bot dead => abort \"Bot died before picking up quest\"");
                lines.push_back("teleport quest_" + std::to_string(questId) + "_giver");
                lines.push_back("require creature alive quest_" + std::to_string(questId) + "_giver");
                lines.push_back("set destination quest_" + std::to_string(questId) + "_giver");
                lines.push_back("wait 15");
                lines.push_back("observe");

                std::ostringstream testName;
                testName << base << "_pickup_l" << level;
                RegisterTest(testName.str(), lines);
                ++pickupTests;

                // BL-46 travel flavor: drop the bot ~90 yd off the giver and let it genuinely
                // travel back. The arrival flip then happens inside MoveToTravelTargetAction
                // itself, so the rpg chain gets a fair chance instead of fighting a forced pin.
                {
                    std::vector<std::string> tlines;
                    tlines.push_back("# quest pickup travel test " + std::to_string(questId));
                    tlines.push_back(RequireLine(level, quest));
                    for (auto const& line : chainLines)
                        tlines.push_back(line);
                    tlines.push_back("monitor quest active " + std::to_string(questId) + " => pass \"Quest picked up on arrival\"");
                    // 900+, not 600: the Jordred sample failed honest at 600 s - the travel leg
                    // itself (walk from the near-point plus roam interference) can legitimately
                    // take longer than 600 s.
                    tlines.push_back("monitor time > 900 => fail \"Timeout: quest not picked up after travelling to the giver (<time elapsed>)\"");
                    tlines.push_back("monitor bot dead => abort \"Bot died before picking up quest\"");
                    // Standard flow end-to-end (operator-approved 2026-09-27): no rpg injection,
                    // no strategy pins. The nonforced destination carries the giver's entry, so
                    // while traveling only 'move to travel target' runs (rpg choose is gated on
                    // 'travel target traveling'); the nonforced 'set destination' grants a
                    // 15-minute leg budget (BL-47(a)) so the target survives the walk. On arrival
                    // CheckStatus flips TRAVEL -> WORK (fixed 5-minute window), ChooseRpgTarget
                    // favors the giver via the travel-target relevance bonus, and
                    // RpgStartQuestTrigger fires the accept - all default machinery.
                    tlines.push_back("teleport quest_" + std::to_string(questId) + "_giver_near");
                    // The near point is 90 yd off the giver - give the alive-check room to still see it.
                    tlines.push_back("require creature alive quest_" + std::to_string(questId) + "_giver 300");
                    tlines.push_back("set destination quest_" + std::to_string(questId) + "_giver nonforced");
                    tlines.push_back("observe");

                    std::ostringstream tname;
                    tname << base << "_pickup_travel_l" << level;
                    RegisterTest(tname.str(), tlines);
                    ++pickupTravelTests;
                }

                // BL-46 aimed flavor: bot already standing at the giver; only the aim is
                // injected. Isolates the accept action from rpg target choosing.
                {
                    std::vector<std::string> alines;
                    alines.push_back("# quest pickup aimed test " + std::to_string(questId));
                    alines.push_back(RequireLine(level, quest));
                    for (auto const& line : chainLines)
                        alines.push_back(line);
                    alines.push_back("monitor quest active " + std::to_string(questId) + " => pass \"Quest picked up (aimed)\"");
                    alines.push_back("monitor time > 300 => fail \"Timeout: quest not picked up after <time elapsed>\"");
                    alines.push_back("monitor bot dead => abort \"Bot died before picking up quest\"");
                    alines.push_back("teleport quest_" + std::to_string(questId) + "_giver");
                    alines.push_back("require creature alive quest_" + std::to_string(questId) + "_giver 150");
                    // Bot teleports INSIDE the radius: the first 'move to travel target' poll runs
                    // CheckStatus, sees IsIn(bot) true, and flips the nonforced destination
                    // TRAVEL -> WORK within the first ticks (fixed 5-minute window). The rpg target
                    // is injected (BL-47(a) destination carries the entry) and 'next rpg action' is
                    // pinned so vendor/repair/discover triggers cannot outrun the accept.
                    alines.push_back("set destination quest_" + std::to_string(questId) + "_giver nonforced");
                    alines.push_back("set rpg target quest_" + std::to_string(questId) + "_giver");
                    alines.push_back("set value string next rpg action => rpg start quest");
                    alines.push_back("observe");

                    std::ostringstream aname;
                    aname << base << "_pickup_aimed_l" << level;
                    RegisterTest(aname.str(), alines);
                    ++pickupAimedTests;
                }
            }
        }
        else
        {
            ++skipNoGiver;
        }

        // ---- handin (state-injected accept + complete; AI must turn in at the taker) ----
        if (haveTaker)
        {
            std::ostringstream locName;
            locName << "quest_" << questId << "_taker";
            // Real creature guid (see the giver registration above).
            RegisterNamedLocation(locName.str(), GuidPosition(ObjectGuid(HIGHGUID_UNIT, taker->second.entry, taker->second.guid), WorldPosition(taker->second.mapId, taker->second.x, taker->second.y, taker->second.z)));

            std::vector<std::string> lines;
            lines.push_back("# quest handin test " + std::to_string(questId));
            lines.push_back(RequireLine(atLevel, quest));
            lines.push_back("accept quest " + std::to_string(questId));
            lines.push_back("complete quest " + std::to_string(questId));
            lines.push_back("monitor quest rewarded " + std::to_string(questId) + " => pass \"Quest handed in\"");
            lines.push_back("monitor time > 600 => fail \"Timeout: quest not handed in after <time elapsed>\"");
            lines.push_back("monitor bot dead => abort \"Bot died before handing in quest\"");
            lines.push_back("teleport quest_" + std::to_string(questId) + "_taker");
            lines.push_back("require creature alive quest_" + std::to_string(questId) + "_taker");
            lines.push_back("set destination quest_" + std::to_string(questId) + "_taker");
            lines.push_back("wait 15");
            lines.push_back("observe");

            std::ostringstream testName;
            testName << base << "_handin_l" << atLevel;
            RegisterTest(testName.str(), lines);
            ++handinTests;
        }
        else
        {
            ++skipNoTaker;
        }

        // ---- objective tests (per objective, tick and clear flavors) ----
        auto objectiveSet = questObjectiveEntries.find(questId);
        if (objectiveSet == questObjectiveEntries.end())
            continue;

        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32 entry = quest->ReqCreatureOrGOId[i];
            if (!entry || entry < 0 || quest->ReqSpell[i])
                continue;

            auto spawn = objectiveSpawns.find(static_cast<uint32>(entry));
            if (spawn == objectiveSpawns.end() || !isOverworldSpawn(spawn->second))
            {
                ++skipNoSpawn;
                continue;
            }

            std::ostringstream locName;
            locName << "quest_obj_" << questId << "_" << static_cast<uint32>(i);
            RegisterNamedLocation(locName.str(), GuidPosition(ObjectGuid(), WorldPosition(spawn->second.mapId, spawn->second.x, spawn->second.y, spawn->second.z)));

            uint32 count = quest->ReqCreatureOrGOCount[i];
            if (!count)
            {
                ++skipZeroCount;
                continue;
            }
            uint32 clearTimeout = std::min<uint32>(1800, 600 + count * 60);

            for (uint32 level : levels)
            {
                {
                    std::vector<std::string> lines;
                    lines.push_back("# quest objective tick test " + std::to_string(questId) + " obj " + std::to_string(i));
                    lines.push_back(RequireLine(level, quest));
                    lines.push_back("accept quest " + std::to_string(questId));
                    lines.push_back("monitor quest objective " + std::to_string(questId) + " " + std::to_string(i) + " count > 0 => pass \"Objective ticked\"");
                    lines.push_back("monitor time > 420 => fail \"Timeout: objective not ticked after <time elapsed>\"");
                    lines.push_back("monitor bot dead => abort \"Bot died before ticking objective\"");
                    lines.push_back("teleport quest_obj_" + std::to_string(questId) + "_" + std::to_string(i));
                    lines.push_back("set destination quest_obj_" + std::to_string(questId) + "_" + std::to_string(i));
                    lines.push_back("wait 10");
                    lines.push_back("observe");

                    std::ostringstream testName;
                    testName << base << "_obj" << static_cast<uint32>(i) << "_tick_l" << level;
                    RegisterTest(testName.str(), lines);
                    ++objTests;
                }

                {
                    std::vector<std::string> lines;
                    lines.push_back("# quest objective clear test " + std::to_string(questId) + " obj " + std::to_string(i));
                    lines.push_back(RequireLine(level, quest));
                    lines.push_back("accept quest " + std::to_string(questId));
                    if (count > 1)
                        lines.push_back("force objectives " + std::to_string(questId) + " except " + std::to_string(i));
                    lines.push_back("monitor quest objective " + std::to_string(questId) + " " + std::to_string(i) + " => pass \"Objective cleared\"");
                    lines.push_back("monitor time > " + std::to_string(clearTimeout) + " => fail \"Timeout: objective not cleared after <time elapsed>\"");
                    lines.push_back("monitor bot dead => abort \"Bot died before clearing objective\"");
                    lines.push_back("teleport quest_obj_" + std::to_string(questId) + "_" + std::to_string(i));
                    lines.push_back("set destination quest_obj_" + std::to_string(questId) + "_" + std::to_string(i));
                    lines.push_back("wait 10");
                    lines.push_back("observe");

                    std::ostringstream testName;
                    testName << base << "_obj" << static_cast<uint32>(i) << "_clear_l" << level;
                    RegisterTest(testName.str(), lines);
                    ++objTests;
                }
            }
        }
    }

    sLog.outString("[QUESTGEN] generated %u pickup, %u pickup_travel, %u pickup_aimed, %u handin, %u objective tests",
        pickupTests, pickupTravelTests, pickupAimedTests, handinTests, objTests);
    sLog.outError("[QUESTGEN] skips: %u no-giver, %u no-taker, %u no-spawn, %u gated, %u chain, %u race+class, %u level, "
        "%u event-inactive, %u condition, %u breadcrumb, %u event-gated-spawn, %u event-gated-objspawns, %u zero-count",
        skipNoGiver, skipNoTaker, skipNoSpawn, skipGated, skipChain, skipGatedCombo, skipNoLevel,
        skipEventQuest, skipCondition, skipBreadcrumb, skipEventSpawn, skipEventSpawnObj, skipZeroCount);
}
