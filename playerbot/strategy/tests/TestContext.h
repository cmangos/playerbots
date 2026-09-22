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

        TestContext() : pc(0), observing(false), testStartTime(0), monitorTime(0), waitTime(0), undergroundCount(0), focusMobEntry(0), focusMobKilled(false), cleanupPc(0), cleanupPrepared(false), whoResponded(false), result(TestResult::PENDING) {}

        void Reset();
    };
}
