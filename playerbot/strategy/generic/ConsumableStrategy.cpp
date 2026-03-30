#include "playerbot/playerbot.h"
#include "ConsumableStrategy.h"

using namespace ai;

void ConsumableStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use consumable", 2.0f), NULL)));
}

void ConsumableStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use consumable", 2.0f), NULL)));
}