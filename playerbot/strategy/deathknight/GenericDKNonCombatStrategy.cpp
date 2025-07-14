#include "playerbot/playerbot.h"
#include "DKMultipliers.h"
#include "GenericDKNonCombatStrategy.h"

using namespace ai;

class GenericDKNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKNonCombatStrategyActionNodeFactory()
    {
        creators["horn of winter"] = &horn_of_winter;
    }
private:
    static ActionNode* horn_of_winter(PlayerbotAI* ai)
    {
        return new ActionNode ("horn of winter",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
};

GenericDKNonCombatStrategy::GenericDKNonCombatStrategy(PlayerbotAI* ai) : NonCombatStrategy(ai)
{
    actionNodeFactories.Add(new GenericDKNonCombatStrategyActionNodeFactory());
}

void GenericDKNonCombatStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    NonCombatStrategy::InitNonCombatTriggers(triggers);

	triggers.push_back(new TriggerNode(
		"no pet",
		NextAction::array(0, new NextAction("raise dead", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "horn of winter",
        NextAction::array(0, new NextAction("horn of winter", 21.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "runeforge",
        NextAction::array(0, new NextAction("runeforge", ACTION_PASSTROUGH), NULL)));

	triggers.push_back(new TriggerNode(
		"auto runeforge",
		NextAction::array(0, new NextAction("runeforge", 1.0f), NULL)));
}

void DKBuffDpsStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "improved icy talons",
        NextAction::array(0, new NextAction("improved icy talons", 19.0f), NULL)));
}

void DKBuffDpsStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}
