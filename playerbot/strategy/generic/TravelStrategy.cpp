
#include "playerbot/playerbot.h"
#include "TravelStrategy.h"

using namespace ai;

NextAction** TravelStrategy::GetDefaultNonCombatActions()
{
    return NextAction::array(0, new NextAction("travel", 1.0f), NULL);
}

void TravelStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        //"random",
        "no travel target",
        NextAction::array(0, new NextAction("choose travel target", 6.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "far from travel target",
        NextAction::array(0, new NextAction("check mount state", 1), new NextAction("move to travel target", 1), NULL)));
}
