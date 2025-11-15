#include "playerbot/playerbot.h"
#include "DKMultipliers.h"
#include "GenericDKNonCombatStrategy.h"

using namespace ai;

class GenericDKNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDKNonCombatStrategyActionNodeFactory()
    {
        creators["bone shield"] = &bone_shield;
        creators["horn of winter"] = &horn_of_winter;
    }
private:
    static ActionNode* bone_shield(PlayerbotAI* ai)
    {
        return new ActionNode ("bone shield",
            /*P*/ NULL,
            /*A*/ NULL,
            /*C*/ NULL);
    }
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
		"raise dead",
		NextAction::array(0, new NextAction("raise dead", ACTION_NORMAL + 1), NULL)));

    triggers.push_back(new TriggerNode(
        "horn of winter",
        NextAction::array(0, new NextAction("horn of winter", 21.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"bone shield",
		NextAction::array(0, new NextAction("bone shield", 21.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "runeforge",
        NextAction::array(0, new NextAction("runeforge", ACTION_PASSTROUGH), NULL)));

	triggers.push_back(new TriggerNode(
		"auto runeforge",
		NextAction::array(0, new NextAction("runeforge", 6.0f), NULL)));
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

void DKStartQuestStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("val::and::{need quest objective::{12680,0},trigger active::in vehicle}", NextAction::array(0, new NextAction("deliver stolen horse", 1.0f), NULL)));

    triggers.push_back(
        new TriggerNode("val::and::{need quest objective::{12687,0},trigger active::in vehicle}", NextAction::array(0, new NextAction("horsemans call", 1.0f), NULL)));

    triggers.push_back(
        new TriggerNode("val::and::{need quest objective::12701,trigger active::in vehicle::Mine Car}", NextAction::array(0, new NextAction("reset travel target", 80.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("scarlet cannon", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("electro - magnetic pulse", 2.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{not::need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("skeletal gryphon escape", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,not::trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("use::39700", 100.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("frozen deathbolt", 80.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("devour humanoid", 81.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{not::need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("leave vehicle", 100.0f), NULL)));

    triggers.push_back(new TriggerNode("val::need quest objective::13165,reward", NextAction::array(0, new NextAction("cast::death gate", 80.0f), NULL)));

    triggers.push_back(new TriggerNode("val::need quest objective::13165,reward", NextAction::array(0, new NextAction("use::Death Gate", 81.0f), NULL)));
};

void DKStartQuestStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("scarlet cannon", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("electro - magnetic pulse", 2.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{not::need quest objective::{12701,0},trigger active::in vehicle::Scarlet Cannon}",
                                       NextAction::array(0, new NextAction("skeletal gryphon escape", 1.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("frozen deathbolt", 81.0f), NULL)));

    triggers.push_back(new TriggerNode("val::and::{need quest objective::12779,trigger active::in vehicle::Frostbrood Vanquisher}",
                                       NextAction::array(0, new NextAction("devour humanoid", 80.0f), NULL)));
};
