#pragma once

#include "TestCommand.h"

namespace ai
{
    class HandleSpawnBot : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "spawn"; }
    };

    class HandleDespawnBot : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "despawn"; }
    };

    class HandleFormParty : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "form"; }
    };
}