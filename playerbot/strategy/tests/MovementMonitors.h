#pragma once

#include "TestMonitor.h"

namespace ai
{
    class CheckDistanceMonitor : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "distance to"; }
    };
}