#include "CommandDebug.h"

using namespace ai;

TestResult CommandDebug::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    ctx.debug = true;
    return TestResult::PASS;
}

TestResult CommandRecord::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    ai->RecordMessages(true, !params.empty());
    return TestResult::PASS;
}

TestResult CommandRead::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    for (auto& msg : ai->GetRecordedMessages())
    {
        ai->TellPlayer(bot, std::string("[Recorded Message] ") + msg);
        sLog.outString("[TestAction] Bot %s outgoing %s", bot->GetName(), msg.c_str());
    }
    return TestResult::PASS;
}