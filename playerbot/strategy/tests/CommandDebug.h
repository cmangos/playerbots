#pragma once
#include "TestComponent.h"

namespace ai
{
    class CommandDebug : public TestCommand
    {
    public:
        std::string GetName() const override { return "debug"; }
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    };

    class CommandRecord : public TestCommand
    {
    public:
        std::string GetName() const override { return "record"; }
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    };

    class CommandRead : public TestCommand
    {
    public:
        std::string GetName() const override { return "read"; }
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    };
}
