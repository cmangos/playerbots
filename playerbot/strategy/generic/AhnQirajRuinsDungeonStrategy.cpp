#include "playerbot/playerbot.h"
#include "AhnQirajRuinsDungeonStrategies.h"
#include "DungeonMultipliers.h" // For generic raid multipliers
#include "DungeonTriggers.h"   // For StartBossFightTrigger, EndBossFightTrigger
#include "ChangeStrategyAction.h" // For ChangeAllStrategyAction

// Note: In "old style", you'd typically manually add entries.
// For this example, I'll still use the concepts of the custom triggers/actions.

using namespace ai;

// --- AhnQirajDungeonStrategy Implementation ---

void AhnQirajDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Start Kurinnaxx fight strategy
    triggers.push_back(new TriggerNode(
        "start kurinnaxx fight",
        NextAction::array(0, new NextAction("enable kurinnaxx strategy", 100.0f), NULL)));

    // (Add triggers for other AQ20 bosses here if/when you implement them)
}

void AhnQirajDungeonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific general AQ20 non-combat triggers for the overall dungeon strategy currently
    // (e.g., if there were specific raid-wide mechanics outside of boss fights)
}

void AhnQirajDungeonStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific general AQ20 dead triggers for the overall dungeon strategy currently
}


// --- KurinnaxxFightStrategy Implementation ---

void KurinnaxxFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Trigger to cure Toxic Volley poison
    triggers.push_back(new TriggerNode(
        "toxic volley poison aura",
        NextAction::array(0, new NextAction("cure toxic volley poison", 90.0f), NULL))); // High priority

    // Trigger for tank swap due to Mortal Wound stacks
    triggers.push_back(new TriggerNode(
        "kurinnaxx mortal wound high", // Trigger if current tank has high Mortal Wound stacks
        NextAction::array(0,
            new NextAction("taunt kurinnaxx", 95.0f),       // Off-tank should taunt (very high priority)
            new NextAction("kurinnaxx tank retreat", 90.0f), // Current tank should retreat
            NULL)));

    // Trigger to avoid Sand Trap during combat
    triggers.push_back(new TriggerNode(
        "kurinnaxx sand trap close", // Using the general "close" trigger
        NextAction::array(0, new NextAction("move away from kurinnaxx sand trap", 100.0f), NULL))); // High priority
}

void KurinnaxxFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Trigger to disable this strategy once Kurinnaxx is defeated (bot out of combat)
    triggers.push_back(new TriggerNode(
        "end kurinnaxx fight",
        NextAction::array(0, new NextAction("disable kurinnaxx strategy", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    // Trigger to disable this strategy if the bot dies during the fight
    triggers.push_back(new TriggerNode(
        "end kurinnaxx fight",
        NextAction::array(0, new NextAction("disable kurinnaxx strategy", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // Immediate reaction to being caught in a Sand Trap
    triggers.push_back(new TriggerNode(
        "kurinnaxx sand trap close", // Reaction-level priority, very urgent
        NextAction::array(0, new NextAction("move away from kurinnaxx sand trap", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers beyond general ones for Kurinnaxx currently.
    // You might add one if, for example, a specific role needs to prioritize a target
    // based on a unique Kurinnaxx debuff.
}