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
    // Optional trailing distance: "wait destination 600 4" passes within 4 yd instead of the
    // default 10. Rpg injection must happen inside INTERACTION_DISTANCE (~5.5 yd), otherwise
    // the far-from-rpg-target machinery drops the injected target again.
    std::string value = params;
    if (value.find("time ") == 0)
        value = value.substr(5);

    float arriveDistance = 10.0f;
    size_t space = value.rfind(' ');
    if (space != std::string::npos)
    {
        if (TryParseFloatStrict(value.substr(space + 1), arriveDistance, message, GetName()) != TestResult::PASS)
            return TestResult::IMPOSSIBLE;
        value = value.substr(0, space);
    }

    uint32 waitSeconds = 0;
    if (TryParseUInt32Strict(value, waitSeconds, message, GetName()) != TestResult::PASS)
        return TestResult::IMPOSSIBLE;

    if (ctx.destinationPosition.distance(bot) < arriveDistance)
        return TestResult::PASS;

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