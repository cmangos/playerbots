#include "DatabaseStrategy.h"
#include "Action.h"
#include "Trigger.h"
#include "playerbot/record/StrategyRecord.h"

using namespace ai;

void DatabaseStrategy::InitTriggers(std::list<TriggerNode*>& triggers, TriggerFlags flags)
{
    for (TriggerRecord const& triggerRecord : record->Triggers)
    {
        if (!(triggerRecord.Flags & flags))
        {
            continue;
        }

        int size = triggerRecord.Actions.size();

        NextAction** actions = new NextAction*[size + 1];
        for (int i = 0; i < size; ++i)
        {
            ActionRecord const& actionRecord = triggerRecord.Actions[i];
            actions[i] = new NextAction(actionRecord.Name, actionRecord.Priority);
        }
        actions[size] = nullptr;

        triggers.push_back(new TriggerNode(triggerRecord.Name, actions));
    }
}

void DatabaseStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitTriggers(triggers, TRIGGER_FLAG_NON_COMBAT);
}

void DatabaseStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitTriggers(triggers, TRIGGER_FLAG_COMBAT);
}

void DatabaseStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    InitTriggers(triggers, TRIGGER_FLAG_DEAD);
}

void DatabaseStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    InitTriggers(triggers, TRIGGER_FLAG_REACTION);
}
