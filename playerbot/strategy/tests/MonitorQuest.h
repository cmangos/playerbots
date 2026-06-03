#pragma once

#include "TestComponent.h"

namespace ai
{
    // =====================================================
    // MonitorQuest - quest-related monitors
    // =====================================================

    // "quest <id> complete" => checks QUEST_STATUS_COMPLETE or QUEST_STATUS_REWARDED
    class MonitorQuestComplete : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "quest complete"; }
    };

    // "quest <id> rewarded" => checks QUEST_STATUS_REWARDED only
    class MonitorQuestRewarded : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "quest rewarded"; }
    };

    // "quest <id> active" => checks bot has the quest in log (any non-NONE status)
    class MonitorQuestActive : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "quest active"; }
    };

    // "quest <id> objective <n> done" => checks a specific quest objective is satisfied
    class MonitorQuestObjective : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "quest objective"; }
    };

    // "has item <id>" => checks bot has at least one of item
    class MonitorHasItem : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "has item"; }
    };

    // "on map <id>" => checks bot is on specific map
    class MonitorOnMap : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "on map"; }
    };

    // "has mount" => checks bot has a usable mount
    class MonitorHasMount : public TestMonitor
    {
    private:
        bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const override;
        std::string GetName() const override { return "has mount"; }
    };
}
