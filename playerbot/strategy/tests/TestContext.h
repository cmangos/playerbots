#pragma once

#include <vector>
#include <string>
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

        bool debug = false; // enable extra logging for debugging

        TestContext() : pc(0), observing(false), testStartTime(0), monitorTime(0), waitTime(0), undergroundCount(0), focusMobEntry(0), focusMobKilled(false), cleanupPc(0), cleanupPrepared(false), whoResponded(false), result(TestResult::PENDING) {}

        void Reset();
    };
}
