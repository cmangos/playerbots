#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // CleanupParty - cleanup spawned bots/party
    // =====================================================
    class CleanupParty : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "cleanup"; }
    };
}