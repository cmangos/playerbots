#include "TestMonitor.h"

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