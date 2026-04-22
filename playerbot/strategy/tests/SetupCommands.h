#pragma once

#include "TestCommand.h"

namespace ai
{
    class HandleTeleport : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "teleport"; }
    };

    class HandleSetGM : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "gm"; }
    };

    class HandleGiveItem : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "give"; }
    };

    class HandleEquipItem : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "equip"; }
    };

    class HandleClearMobs : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "clear"; }
    };

    class HandleSetDestination : public TestCommand
    {
    public:
        bool Matches(const std::string& cmd, const std::string& params) const override;
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "set"; }
    };
}