#pragma once

#include <string>
#include "TestContext.h"

class Player;

namespace ai
{
    class TestMonitor
    {
    public:
        virtual bool Matches(const std::string& monitorStr) const { return monitorStr.find(GetName()) == 0; }
        virtual TestResult Check(const std::string& monitorStr, Player* bot,
                             TestContext& ctx, std::string& message) const;
        virtual ~TestMonitor() = default;
    protected:
        virtual std::string GetName() const = 0;
        virtual bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const = 0;
    };
}