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
    protected:
        bool DistanceMet(const std::string& monitorStr, Player* bot) const;
    };

    class MonitorMovementArrived : public MonitorMovementDistance
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "arrived"; }
    };

    class MonitorMovementRodeTransport : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "rode transport"; }
    };

    class MonitorMovementLeftTransport : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "left transport"; }
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