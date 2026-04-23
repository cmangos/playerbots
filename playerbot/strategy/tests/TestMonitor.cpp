#include "TestMonitor.h"
#include "playerbot/WorldPosition.h"
#include "playerbot/PlayerbotTextMgr.h"

using namespace ai;

TestResult TestMonitor::Check(const std::string& monitorStr, Player* bot, TestContext& ctx, std::string& message) const
{
    if (IsConditionMet(monitorStr, bot, ctx))
    {
        size_t arrowPos = monitorStr.find("=>");
        if (arrowPos == std::string::npos)
            return TestResult::PENDING;

        size_t quoteStart = monitorStr.find("\"", arrowPos);
        if (quoteStart == std::string::npos)
            return TestResult::PENDING;

        size_t quoteEnd = monitorStr.find("\"", quoteStart + 1);
        if (quoteEnd == std::string::npos)
            return TestResult::PENDING;

        message = monitorStr.substr(quoteStart + 1, quoteEnd - quoteStart - 1);

        // Replace dynamic placeholders in message
        std::map<std::string, std::string> placeholders;
        if (bot->IsInWorld())
        {
            WorldPosition pos(bot);
            placeholders["<current position>"] = pos.print(2, true) + " m" + std::to_string(pos.getMapId());
        }
        placeholders["<time elapsed>"] = std::to_string((WorldTimer::getMSTime() - ctx.testStartTime) / 1000) + "s";
        
        if (!placeholders.empty())
        {
            PlayerbotTextMgr::ReplacePlaceholders(message, placeholders);
        }

        std::string resultType = monitorStr.substr(arrowPos + 3, quoteStart - (arrowPos + 2)-2);        

        if (resultType == "pass")
            return TestResult::PASS;
        else if (resultType == "fail")
            return TestResult::FAIL;
        else if (resultType == "impossible")
            return TestResult::IMPOSSIBLE;
        else if (resultType == "abort")
            return TestResult::ABORT;
        else
            return TestResult::PENDING;

        return TestResult::PENDING;
    }
    
    return TestResult::PENDING;
}