#include "playerbot/playerbot.h"
#include "CommandFlow.h"
#include "TestAction.h"
#include "TestRegistry.h"

using namespace ai;

TestResult CommandFlowObserve::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    ctx.observing = true;
    return TestResult::PASS;
}

TestResult CommandFlowPreconditions::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    return TestResult::PASS;
}

TestResult CommandFlowMonitor::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    ctx.monitors.push_back(params);
    return TestResult::PASS;
}

TestResult CommandFlowWait::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (!ctx.waitTime)
        ctx.waitTime = WorldTimer::getMSTime();

    uint32 waitDuration = 0;
    if (TryParseUInt32Strict(params, waitDuration, message, GetName()) != TestResult::PASS)
        return TestResult::IMPOSSIBLE;

    if (WorldTimer::getMSTimeDiff(ctx.waitTime, WorldTimer::getMSTime()) >= waitDuration)
    {
        ctx.waitTime = 0;
        return TestResult::PASS;
    }
    
    return TestResult::PENDING;
}

TestResult CommandFlowWaitDestination::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (ctx.destinationPosition.distance(bot) < 10.0f)
        return TestResult::PASS;

    if (!ctx.waitTime)
        ctx.waitTime = WorldTimer::getMSTime();

    uint32 waitDuration = 0;
    if (TryParseUInt32Strict(params, waitDuration, message, GetName()) != TestResult::PASS)
        return TestResult::IMPOSSIBLE;

    if (WorldTimer::getMSTimeDiff(ctx.waitTime, WorldTimer::getMSTime()) >= waitDuration)
    {
        ctx.waitTime = 0;
        return TestResult::PASS;
    }

    return TestResult::PENDING;
}

TestResult CommandFlowRepeat::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (params.empty())
    {
        ctx.pc = 0;
        return TestResult::PENDING;
    }

    uint32 pc = 0;
    if (TryParseUInt32Strict(params, pc, message, GetName()) != TestResult::PASS)
        return TestResult::IMPOSSIBLE;

    ctx.pc = static_cast<int>(pc);

    return TestResult::PENDING;
}