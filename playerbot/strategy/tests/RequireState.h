#pragma once

#include "TestComponent.h"

namespace ai
{
    class RequireBotIs : public TestRequire
    {
    public:
        virtual TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "require bot is"; }
    };

    class RequireEquip : public TestRequire
    {
    public:
        virtual TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "require equip"; }
    };
}