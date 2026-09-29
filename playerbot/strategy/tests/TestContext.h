#pragma once

#include <vector>
#include <string>
#include <set>
#include <cstdint>
#include "Globals/ObjectMgr.h"
#include "playerbot/GuidPosition.h"
#include "playerbot/WorldPosition.h"

class ObjectGuid;

namespace ai
{
    enum class TestResult
    {
        PENDING,
        IMPOSSIBLE,
        PASS,
        FAIL,
        ABORT
    };

    struct TestContext
    {
        std::vector<std::string> script;        
        int pc;                                 
        bool observing;                         
        uint32 testStartTime;                   
        uint32 monitorTime;
        uint32 waitTime;                        
        uint32 undergroundCount;

        // Consecutive ticks the "can not reach nodes" monitor saw no pathable node. Right after
        // a teleport the target grid/mmaps may not be loaded yet, so the monitor only fails after
        // a grace period instead of on the first tick (same pattern as undergroundCount).
        uint32 cannotReachCount;
        uint32 focusMobEntry;                   
        ObjectGuid focusMobGuid;                
        bool focusMobKilled;                    
        std::vector<ObjectGuid> spawnedBots;    
        std::vector<std::string> monitors;      
        std::vector<std::string> deferredCleanups;
        size_t cleanupPc;
        bool cleanupPrepared;
        bool whoResponded;
        TestResult result;                       
        std::string resultMessage;
        std::string testName;                    
        WorldPosition testStartPosition;
        GuidPosition destinationPosition;

        // Party XP total (all group members) captured when the test starts. The "party xp" monitor
        // measures the gain against this baseline: kill/loot-driven corpses near the host are too
        // transient (bots loot instantly) for sparse-start instances, so "party cleared trash" is
        // asserted on accumulated party XP instead.
        uint32 partyXpStart = 0;
        bool partyXpCaptured = false;

        // Every dead creature GUID the "dead mobs" monitor has ever observed this run. Counted
        // cumulatively per unique GUID because corpses despawn on loot - a "6 corpses at once"
        // snapshot rarely happens in sparse-start instances even under heavy killing.
        std::set<ObjectGuid> observedDeadMobs;

        // Where the most recent resurrect request told its target to land, and on which map. Monitors
        // must measure against this rather than the acting bot: the caller is a random bot that can
        // random-teleport thousands of yards (or into a battleground) while the observe window runs,
        // which made a proximity check against it report a false failure.
        uint32 resurrectMapId = 0;
        float resurrectX = 0.0f;
        float resurrectY = 0.0f;
        float resurrectZ = 0.0f;
        bool hasResurrectRequest = false;

        // Latched by the "group on map" monitor: each member is recorded the first time it is observed on
        // the bot's map. Per member rather than a single snapshot, because group members are roamed random
        // bots and are rarely all settled on the same map on the same tick - the monitor asserts that the
        // delivery happened, not that it held.
        std::set<ObjectGuid> groupMembersSeenOnMap;

        bool debug = false; // enable extra logging for debugging

        TestContext() : pc(0), observing(false), testStartTime(0), monitorTime(0), waitTime(0), undergroundCount(0), cannotReachCount(0), focusMobEntry(0), focusMobKilled(false), cleanupPc(0), cleanupPrepared(false), whoResponded(false), result(TestResult::PENDING) {}

        void Reset();
    };
}
