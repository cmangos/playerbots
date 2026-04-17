
#include "playerbot/playerbot.h"
#include "WanderStrategy.h"

using namespace ai;

void WanderStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wander far",
        NextAction::array(0, new NextAction("check mount state", ACTION_HIGH), new NextAction("follow", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "wander medium",
        NextAction::array(0, new NextAction("check mount state", 0.1f), new NextAction("follow", 0.1f), NULL)));

    triggers.push_back(new TriggerNode(
        "wander near",
        NextAction::array(0, new NextAction("stop follow", ACTION_IDLE), NULL)));
}