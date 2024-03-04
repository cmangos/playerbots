
#include "playerbot/playerbot.h"
#include "PriestMultipliers.h"
#include "ShadowPriestStrategy.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

using namespace ai;

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* ai) : GenericPriestStrategy(ai)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

NextAction** ShadowPriestStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("shoot", 1.0f), NULL);
}

void ShadowPriestStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    GenericPriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "timer",
        NextAction::array(0, new NextAction("mind blast", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "shadowform",
        NextAction::array(0, new NextAction("shadowform", ACTION_HIGH + 10), NULL)));

    /*triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("dispersion", ACTION_EMERGENCY + 5), NULL)));*/

    triggers.push_back(new TriggerNode(
        "vampiric embrace",
        NextAction::array(0, new NextAction("vampiric embrace", 16.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "starshards",
        NextAction::array(0, new NextAction("starshards", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "silence",
        NextAction::array(0, new NextAction("silence", ACTION_INTERRUPT + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "silence on enemy healer",
        NextAction::array(0, new NextAction("silence on enemy healer", ACTION_INTERRUPT), NULL)));

    triggers.push_back(new TriggerNode(
        "shadowfiend",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("mana burn", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "target critical health",
        NextAction::array(0, new NextAction("shadow word: death", ACTION_INTERRUPT + 1), NULL)));
}

void ShadowPriestAoeStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array(0, new NextAction("shadow word: pain on attacker", 11.0f), NULL)));
}

void ShadowPriestDebuffStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "devouring plague",
        NextAction::array(0, new NextAction("devouring plague", 13.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "vampiric touch",
        NextAction::array(0, new NextAction("vampiric touch", 11.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "shadow word: pain",
        NextAction::array(0, new NextAction("shadow word: pain", 12.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "feedback",
        NextAction::array(0, new NextAction("feedback", 80.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "hex of weakness",
        NextAction::array(0, new NextAction("hex of weakness", 10.0f), NULL)));
}
