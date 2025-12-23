#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class CombatStartTrigger : public Trigger
    {
    public:
        CombatStartTrigger(PlayerbotAI* ai) : Trigger(ai, "combat start") {}
        virtual bool IsActive() override;
    };

    class CombatEndTrigger : public Trigger
    {
    public:
        CombatEndTrigger(PlayerbotAI* ai) : Trigger(ai, "combat end") {}
        virtual bool IsActive() override;
    };

    class DeathTrigger : public Trigger
    {
    public:
        DeathTrigger(PlayerbotAI* ai) : Trigger(ai, "death") {}
        virtual bool IsActive() override;
    };

    class ResurrectTrigger : public Trigger
    {
    public:
        ResurrectTrigger(PlayerbotAI* ai) : Trigger(ai, "resurrect") {}
        virtual bool IsActive() override;
    };
}