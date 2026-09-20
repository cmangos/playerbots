#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // TeleportTests - cross-map summon / resurrect coverage (BL-14 .. BL-25)
    //
    // These drive the same server-side request paths the real callers use
    // (UseMeetingStoneAction / CastCustomSpellAction / WorldBuffTravelActions), without needing an
    // innkeeper, meeting stone or warlock pet in range, so the map/thread behaviour can be
    // exercised from a scripted test.
    // =====================================================

    // Sends SMSG_SUMMON_REQUEST asking the test's spawned bot to come to the acting bot.
    //   "summon request"                 - expects the request to be sent
    //   "summon request expect rejected" - expects it to be refused (target dead / in combat)
    class CommandSummonRequest : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "summon request"; }
    };

    // The dead-target equivalent - sends the resurrect request the caller would send for a corpse.
    class CommandResurrectRequest : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "resurrect request"; }
    };

    // Kills the test's spawned bot so the dead-target paths can be exercised. Decisive: it never
    // returns PENDING, because a PENDING command outside "observe" has no timeout to break out of.
    class CommandKillSpawn : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "kill spawn"; }
    };

    // Moves the test's spawned bot to a named location ("move spawn <name>"). Routed through
    // RunOnOwningThread so a cross-map move never touches another map from our own thread.
    class CommandMoveSpawn : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "move spawn"; }
    };

    // Puts the test's spawned bot into combat ("engage spawn <creatureEntry>") by summoning a
    // creature on top of it and attacking. Needed to force the in-combat branches (BL-20, BL-22).
    class CommandEngageSpawn : public TestCommand
    {
    public:
        TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) override;
    protected:
        std::string GetName() const override { return "engage spawn"; }
    };

    // =====================================================
    // Monitors - these assert on the *spawned* bot, not on the acting bot.
    // Every other monitor in the framework looks at the acting bot, which cannot express
    // "the bot I summoned actually arrived / actually got resurrected".
    // =====================================================

    // "spawn on map" -> the first spawned bot is in world on the acting bot's map and instance
    class MonitorSpawnOnMap : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "spawn on map"; }
    };

    // "spawn alive" -> the first spawned bot is alive (proves a self-resurrect happened)
    class MonitorSpawnAlive : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "spawn alive"; }
    };

    // "spawn dead" -> the first spawned bot is dead. Pair with a time monitor to assert it STAYS
    // dead (BL-22: a summon must not resurrect a corpse).
    class MonitorSpawnDead : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "spawn dead"; }
    };
}
