#pragma once
#include "TestComponent.h"

namespace ai
{
    class CommandRecordMessages : public TestCommand
    {
    public:
        std::string GetName() const override { return "record messages"; }
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    };
}
