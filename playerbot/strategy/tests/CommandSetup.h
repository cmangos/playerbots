#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // CommandSetup - setup commands (teleport, gm, item, etc.)
    // =====================================================
    class CommandSetupTeleport : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "teleport"; }
    };

    class CommandSetupGM : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "gm"; }
    };

    class CommandSetupGiveItem : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "give"; }
    };

    class CommandSetupEquipItem : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "equip"; }
    };

    class CommandSetupClearMobs : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "clear"; }
    };

    class CommandSetupSetDestination : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "set destination"; }
    };

    // BL-47(a) follow-up (five_signets): verify that the creature behind a named location is actually
    // ALIVE in the world near the bot. A dead/despawned giver turned into a 900 s accept-timeout FAIL;
    // this turns the same condition into an honest ABORT at setup. Usage:
    //   "require creature alive <location> [yd]"  (yd defaults to 300)
    class CommandRequireCreatureAlive : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "require creature alive"; }
    };

    // BL-46: aim the bot's rpg machinery at a specific world object (e.g. quest_<id>_giver).
    // The rest of the chain (arrival -> rpg trigger -> accept) runs normally afterwards.
    class CommandSetupRpgTarget : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "set rpg target"; }
    };

    class CommandSetupTeleportGroup : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "teleport group"; }
    };

    class CommandSetupPull : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "pull"; }
    };

    class CommandSetValue : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "set value"; }
    };
}