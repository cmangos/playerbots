#include "TestContext.h"
#include "playerbot/PlayerbotMgr.h"

using namespace ai;

void TestContext::Reset()
{
    script.clear();
    pc = 0;
    observing = false;
    testStartTime = 0;
    waitTime = 0;
    monitors.clear();
    deferredCleanups.clear();
    cleanupPc = 0;
    cleanupPrepared = false;
    result = TestResult::PENDING;
    resultMessage.clear();
    testName.clear();
    testStartPosition = WorldPosition();
    destinationPosition = GuidPosition();

    for (ObjectGuid const& guid : spawnedBots)
    {
        if (guid && guid.IsPlayer())
        {
            sRandomPlayerbotMgr.DeleteBot(guid, true);
        }
    }
    spawnedBots.clear();
}