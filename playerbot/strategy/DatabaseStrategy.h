#pragma once

#include "Strategy.h"

namespace ai
{
    class DatabaseStrategy : public Strategy
    {
    public:
        DatabaseStrategy(PlayerbotAI* ai, std::shared_ptr<StrategyRecord> record) :
            Strategy(ai), record(record) {}

        int GetType() override { return record->Type; }
        std::string getName() override { return record->Name; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() override { return record->Name; } //Must equal iternal name
        virtual std::string GetHelpDescription() override { return record->Description; }
        virtual std::vector<std::string> override GetRelatedStrategies() { return record->RelatedStrategies; }
#endif

    protected:
        void InitTriggers(std::list<TriggerNode*>& triggers, TriggerFlags flags);
        virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) override;
        virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) override;

    private:
        std::shared_ptr<StrategyRecord> record;
    };
}
