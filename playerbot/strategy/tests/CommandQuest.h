#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // CommandQuest - quest setup commands for tests
    // =====================================================

    // "accept quest <id>" - Force-accept a quest by ID
    class CommandSetupAcceptQuest : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "accept quest"; }
    };

    // "complete quest <id>" - Force-complete all objectives of a quest (like AutoComplete)
    class CommandSetupForceCompleteQuest : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "complete quest"; }
    };

    // "reward quest <id>" - Force-reward a quest (complete + reward, skipping turn-in)
    class CommandSetupRewardQuest : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "reward quest"; }
    };

    // "do <chatcommand>" - Execute any bot chat command  
    class CommandSetupDo : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "do"; }
    };
}
