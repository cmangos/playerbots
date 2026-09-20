#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // CommandFlow - flow control commands
    // =====================================================
    class CommandFlowObserve : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "observe"; }
    };

    class CommandFlowPreconditions : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "preconditions"; }
    };

    class CommandFlowMonitor : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "monitor"; }
    };

    class CommandFlowWait : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        // Scripts are written as "wait <seconds>". Registered AFTER CommandFlowWaitDestination so
        // that "wait destination <n>" still resolves to the destination variant.
        std::string GetName() const override { return "wait"; }
    };

    class CommandFlowWaitDestination : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "wait destination"; }
    };

    class CommandFlowRepeat : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "repeat"; }
    };
}