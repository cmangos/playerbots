#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // MonitorCombat - combat-related monitors
    // =====================================================
    class MonitorCombatHp : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "hp"; }
    };

    class MonitorCombatMob : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "mob"; }
    };

    class MonitorCombatPartyWiped : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "party wiped"; }
    };

    class MonitorCombatDeadMobs : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "dead mobs"; }
    };
}