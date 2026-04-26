#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // MonitorMovement - movement-related monitors
    // =====================================================
    class MonitorMovementDistance : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "distance to"; }
    };

    class MonitorNotOnMap : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "not on map"; }
    };

    class MonitorMovementUnderground : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "underground"; }
    };

    class MonitorMovementCanNotReachNodes : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "can not reach nodes"; }
    };

    class MonitorMovementSpeed : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "speed"; }
    };

    class MonitorMovementSpawnDistance : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "spawn distance"; }
    };
}