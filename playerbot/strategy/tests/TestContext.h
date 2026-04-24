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
        std::vector<std::string> script;        // the test DSL lines
        int pc;                                  // program counter
        bool observing;                          // true = observe mode
        uint32 testStartTime;                    // getMSTime() at test start
        uint32 waitTime;                         // time to wait in current wait command
        uint32 undergroundCount;
        std::vector<ObjectGuid> spawnedBots;     // for cleanup
        std::vector<std::string> monitors;       // active monitor expressions
        std::vector<std::string> deferredCleanups;
        size_t cleanupPc;
        bool cleanupPrepared;
        TestResult result;                       // PENDING / IMPOSSIBLE / ABORT / PASS / FAIL
        std::string resultMessage;
        std::string testName;                    // name of the running test
        WorldPosition testStartPosition;
        GuidPosition destinationPosition;

        TestContext() : pc(0), observing(false), testStartTime(0), waitTime(0), undergroundCount(0), cleanupPc(0), cleanupPrepared(false), result(TestResult::PENDING) {}

        void Reset();
    };
}
