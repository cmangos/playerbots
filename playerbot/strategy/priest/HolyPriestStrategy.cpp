
#include "playerbot/playerbot.h"
#include "PriestMultipliers.h"
#include "HolyPriestStrategy.h"

namespace ai
{
    class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
    {
    public:
        HolyPriestStrategyActionNodeFactory()
        {
            creators["smite"] = &smite;
        }
    private:
        static ActionNode* smite(PlayerbotAI* ai)
        {
            return new ActionNode ("smite",
                /*P*/ NULL,
                /*A*/ NextAction::array(0, new NextAction("shoot"), NULL),
                /*C*/ NULL);
        }
    };
};

using namespace ai;

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* ai) : HealPriestStrategy(ai)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

NextAction** HolyPriestStrategy::GetDefaultCombatActions()
{
    return NextAction::array(0, new NextAction("smite", 10.0f), new NextAction("mana burn", 9.0f), new NextAction("starshards", 8.0f), NULL);
}

void HolyPriestStrategy::InitCombatTriggers(std::list<TriggerNode*> &triggers)
{
    HealPriestStrategy::InitCombatTriggers(triggers);

    triggers.push_back(new TriggerNode(
        "holy fire",
        NextAction::array(0, new NextAction("holy fire", ACTION_NORMAL + 9), NULL)));

    triggers.push_back(new TriggerNode(
        "shadowfiend",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "medium mana",
        NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "low mana",
        NextAction::array(0, new NextAction("mana burn", ACTION_HIGH), NULL)));
}
