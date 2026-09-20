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

    // Scripts are written as "wait <seconds>" (e.g. "wait 5"). The older spelling is still accepted
    // as "wait time <seconds>".
    std::string value = params;
    if (value.find("time ") == 0)
        value = value.substr(5);

    uint32 waitSeconds = 0;
    if (TryParseUInt32Strict(value, waitSeconds, message, GetName()) != TestResult::PASS)
        return TestResult::IMPOSSIBLE;

    if (WorldTimer::getMSTimeDiff(ctx.waitTime, WorldTimer::getMSTime()) >= waitSeconds * 1000)
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

    // Seconds, like "wait" - scripts use "wait destination 600" to mean ten minutes.
    std::string value = params;
    if (value.find("time ") == 0)
        value = value.substr(5);

    uint32 waitSeconds = 0;
    if (TryParseUInt32Strict(value, waitSeconds, message, GetName()) != TestResult::PASS)
        return TestResult::IMPOSSIBLE;

    if (WorldTimer::getMSTimeDiff(ctx.waitTime, WorldTimer::getMSTime()) >= waitSeconds * 1000)
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