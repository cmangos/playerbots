#pragma once
#include "playerbot/strategy/Strategy.h"

enum class AranPhase : uint8
{
    PHASE_NONE = 1,
    PHASE_ARCANE = 2,
    PHASE_FIRE = 3,
    PHASE_FROST = 4
};

namespace ai
{
    class KarazhanDungeonStrategy : public Strategy
    {
    public:
        KarazhanDungeonStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "karazhan"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class NetherspiteFightStrategy : public Strategy
    {
    public:
        NetherspiteFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        static NetherspiteFightStrategy* Get(PlayerbotAI* ai);
        void SetBossGuid(ObjectGuid guid) { netherspiteGuid = guid; }
        ObjectGuid GetBossGuid() { return netherspiteGuid; }
        std::string getName() override { return "netherspite"; }
    private:
        ObjectGuid netherspiteGuid;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void OnStrategyAdded(BotState state) override;
    };

    class ShadeOfAranFightStrategy : public Strategy
    {
    public:
        ShadeOfAranFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        static ShadeOfAranFightStrategy* Get(PlayerbotAI* ai);
        void SetPhase(AranPhase phase) { currentPhase = phase; }
        AranPhase GetPhase() { return currentPhase; }
        void SetBossGuid(ObjectGuid guid) { aranGuid = guid; }
        ObjectGuid GetBossGuid() { return aranGuid; }
        void SetRangedBot(bool isRanged) { rangedBot = isRanged; }
        bool GetRangedBot() { return rangedBot; }
        void SetElementals(bool isOut) { elementalsOut = isOut; }
        bool GetElementals() { return elementalsOut; }
        std::string getName() override { return "shade of aran"; }
    private:
        ObjectGuid aranGuid;
        AranPhase currentPhase;
        bool rangedBot;
        bool elementalsOut;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*> &triggers) override;
        void OnStrategyAdded(BotState state) override;
    };
  
    class BigBadWolfFightStrategy : public Strategy
    {
    public:
        BigBadWolfFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "big bad wolf"; }
    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class PrinceMalchezaarFightStrategy : public Strategy
    {
    public:
        PrinceMalchezaarFightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "prince malchezaar"; }

    private:
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;
    };
}