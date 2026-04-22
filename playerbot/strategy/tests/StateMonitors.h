#pragma once

#include "TestMonitor.h"

namespace ai
{
    class CheckTimeMonitor : public TestMonitor
    {
    private:
        virtual bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        virtual std::string GetName() const override { return "time"; }
    };

    class BotDeadMonitor : public TestMonitor
    {
    private:
        virtual bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        virtual std::string GetName() const override { return "bot dead"; }
    };

    class FactionMonitor : public TestMonitor
    {
    private:
        virtual bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        virtual std::string GetName() const override { return "faction"; }
    };
}