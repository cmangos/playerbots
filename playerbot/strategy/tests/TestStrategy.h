#pragma once

#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    class TestStrategy : public Strategy, public Qualified
    {
    public:
        TestStrategy(PlayerbotAI* ai) : Strategy(ai), Qualified() {}
        virtual std::string getName() override { return qualifier.empty() ? "test" : "test::" + qualifier; }
        virtual int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override { InitNonCombatTriggers(triggers); }
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override { InitNonCombatTriggers(triggers); };

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "test"; }
        virtual std::string GetHelpDescription()
        {
            return "Activates the live integration test framework.\n"
                   "Usage: strategy +test:<testName>";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return {}; }
#endif
    };
}
