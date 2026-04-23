#include "TestContext.h"
#include "playerbot/PlayerbotMgr.h"

using namespace ai;

void TestContext::Reset()
{
    script.clear();
    pc = 0;
    observing = false;
    testStartTime = 0;
    monitors.clear();
    result = TestResult::PENDING;
    resultMessage.clear();
    testName.clear();

    for (ObjectGuid const& guid : spawnedBots)
    {
        if (guid && guid.IsPlayer())
        {
            sRandomPlayerbotMgr.DeleteBot(guid, true);
        }
    }
    spawnedBots.clear();
}