#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    // TO DO: Remove this when no dependencies are left
    class NonCombatStrategy : public Strategy
    {
    public:
        NonCombatStrategy(PlayerbotAI* ai) : Strategy(ai) {}
		virtual int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }

    protected:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class CollisionStrategy : public Strategy
    {
    public:
        CollisionStrategy(PlayerbotAI* ai) : Strategy(ai) {}
		int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "collision"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*> &triggers) override;
    };

    class MountStrategy : public Strategy
    {
    public:
        MountStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "mount"; }

    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class AttackTaggedStrategy : public Strategy
    {
    public:
        AttackTaggedStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "attack tagged"; }
    };

    class WorldBuffStrategy : public Strategy
    {
    public:
        WorldBuffStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "wbuff"; }

    protected:
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void OnStrategyRemoved(BotState state) override;
    };

    class SilentStrategy : public Strategy
    {
    public:
        SilentStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        virtual int GetType() { return STRATEGY_TYPE_NONCOMBAT; }
        virtual std::string getName() { return "silent"; }
    };
}
