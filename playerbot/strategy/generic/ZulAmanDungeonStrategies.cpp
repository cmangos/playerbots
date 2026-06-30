
#include "playerbot/playerbot.h"
#include "ZulAmanDungeonStrategies.h"

using namespace ai;

void ZulAmanDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start akilzon fight",
        NextAction::array(0, new NextAction("enable akilzon fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start janalai fight",
        NextAction::array(0, new NextAction("enable janalai fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start zuljin fight",
        NextAction::array(0, new NextAction("enable zuljin fight strategy", 100.0f), NULL)));
}

// ----- Akil'zon (Eagle Boss) -----

void AkilzonFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Static Disruption: spread out
    triggers.push_back(new TriggerNode(
        "za akilzon static disruption",
        NextAction::array(0, new NextAction("za akilzon spread", 100.0f), NULL)));

    // Electrical Storm: move to the player levitated by it
    triggers.push_back(new TriggerNode(
        "za akilzon electrical storm",
        NextAction::array(0, new NextAction("za akilzon move to storm", 100.0f), NULL)));
}

void AkilzonFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end akilzon fight",
        NextAction::array(0, new NextAction("disable akilzon fight strategy", 100.0f), NULL)));
}

void AkilzonFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end akilzon fight",
        NextAction::array(0, new NextAction("disable akilzon fight strategy", 100.0f), NULL)));
}

// ----- Jan'alai (Dragonhawk Boss) -----

void JanalaiFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Fire Bombs: move away from fire bomb zones
    triggers.push_back(new TriggerNode(
        "za janalai fire bombs",
        NextAction::array(0, new NextAction("za janalai avoid fire bombs", 100.0f), NULL)));
}

void JanalaiFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end janalai fight",
        NextAction::array(0, new NextAction("disable janalai fight strategy", 100.0f), NULL)));
}

void JanalaiFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end janalai fight",
        NextAction::array(0, new NextAction("disable janalai fight strategy", 100.0f), NULL)));
}

// ----- Zul'jin -----

void ZuljinFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Multi-phase boss - basic strategy placeholder
}

void ZuljinFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end zuljin fight",
        NextAction::array(0, new NextAction("disable zuljin fight strategy", 100.0f), NULL)));
}

void ZuljinFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end zuljin fight",
        NextAction::array(0, new NextAction("disable zuljin fight strategy", 100.0f), NULL)));
}
