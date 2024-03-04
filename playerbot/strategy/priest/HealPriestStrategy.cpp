
#include "playerbot/playerbot.h"
#include "PriestMultipliers.h"
#include "HealPriestStrategy.h"
#include "GenericPriestStrategyActionNodeFactory.h"

using namespace ai;

HealPriestStrategy::HealPriestStrategy(PlayerbotAI* ai) : GenericPriestStrategy(ai)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

void HealPriestStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericPriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "almost full health",
        NextAction::array(0, new NextAction("renew", 43.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("heal on party", 41.0f), new NextAction("renew on party", 40.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("greater heal on party", 47.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("power word: shield on party", 51.0f), new NextAction("greater heal on party", 50.0f), NULL)));

    /*triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("prayer of mending", 49.0f), NULL)));*/

    /*triggers.push_back(new TriggerNode(
        "medium aoe heal",
        NextAction::array(0, new NextAction("circle of healing", 48.0f), NULL)));*/

    triggers.push_back(new TriggerNode(
        "binding heal",
        NextAction::array(0, new NextAction("binding heal", 52.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), NULL)));
}
