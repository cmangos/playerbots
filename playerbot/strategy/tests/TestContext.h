#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "Globals/ObjectMgr.h"

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
        std::vector<ObjectGuid> spawnedBots;     // for cleanup
        std::vector<std::string> monitors;       // active monitor expressions
        TestResult result;                       // PENDING / IMPOSSIBLE / ABORT / PASS / FAIL
        std::string resultMessage;
        std::string testName;                    // name of the running test

        TestContext() : pc(0), observing(false), testStartTime(0), result(TestResult::PENDING) {}

        void Reset()
        {
            script.clear();
            pc = 0;
            observing = false;
            testStartTime = 0;
            spawnedBots.clear();
            monitors.clear();
            result = TestResult::PENDING;
            resultMessage.clear();
            testName.clear();
        }
    };
}
