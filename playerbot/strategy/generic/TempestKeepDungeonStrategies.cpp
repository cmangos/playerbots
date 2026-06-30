
#include "playerbot/playerbot.h"
#include "TempestKeepDungeonStrategies.h"

using namespace ai;

void TempestKeepDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start void reaver fight",
        NextAction::array(0, new NextAction("enable void reaver fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start solarian fight",
        NextAction::array(0, new NextAction("enable solarian fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start kaelthas fight",
        NextAction::array(0, new NextAction("enable kaelthas fight strategy", 100.0f), NULL)));
}

// ----- Void Reaver -----

void VoidReaverFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Arcane Orb: move away from impact zone
    triggers.push_back(new TriggerNode(
        "tk void reaver arcane orb",
        NextAction::array(0, new NextAction("tk void reaver avoid orb", 100.0f), NULL)));
}

void VoidReaverFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end void reaver fight",
        NextAction::array(0, new NextAction("disable void reaver fight strategy", 100.0f), NULL)));
}

void VoidReaverFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end void reaver fight",
        NextAction::array(0, new NextAction("disable void reaver fight strategy", 100.0f), NULL)));
}

// ----- High Astromancer Solarian -----

void SolarianFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Wrath of the Astromancer: move away from group (bomb debuff)
    triggers.push_back(new TriggerNode(
        "tk solarian wrath debuff",
        NextAction::array(0, new NextAction("tk solarian move from group", 100.0f), NULL)));
}

void SolarianFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end solarian fight",
        NextAction::array(0, new NextAction("disable solarian fight strategy", 100.0f), NULL)));
}

void SolarianFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end solarian fight",
        NextAction::array(0, new NextAction("disable solarian fight strategy", 100.0f), NULL)));
}

// ----- Kael'thas Sunstrider -----

void KaelthasFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Placeholder for Kael'thas multi-phase fight
    // Phase management is extremely complex, starting with basic boss detection
}

void KaelthasFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end kaelthas fight",
        NextAction::array(0, new NextAction("disable kaelthas fight strategy", 100.0f), NULL)));
}

void KaelthasFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end kaelthas fight",
        NextAction::array(0, new NextAction("disable kaelthas fight strategy", 100.0f), NULL)));
}
