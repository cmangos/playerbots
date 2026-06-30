
#include "playerbot/playerbot.h"
#include "SerpentshrineCavernDungeonStrategies.h"

using namespace ai;

void SerpentshrineCavernDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start lurker below fight",
        NextAction::array(0, new NextAction("enable lurker below fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start leotheras fight",
        NextAction::array(0, new NextAction("enable leotheras fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start vashj fight",
        NextAction::array(0, new NextAction("enable vashj fight strategy", 100.0f), NULL)));
}

// ----- Lurker Below -----

void LurkerBelowFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Spout: move behind boss when casting
    triggers.push_back(new TriggerNode(
        "ssc lurker spout",
        NextAction::array(0, new NextAction("ssc lurker move behind boss", 100.0f), NULL)));
}

void LurkerBelowFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end lurker below fight",
        NextAction::array(0, new NextAction("disable lurker below fight strategy", 100.0f), NULL)));
}

void LurkerBelowFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end lurker below fight",
        NextAction::array(0, new NextAction("disable lurker below fight strategy", 100.0f), NULL)));
}

// ----- Leotheras the Blind -----

void LeotherasFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Whirlwind: melee run away
    triggers.push_back(new TriggerNode(
        "ssc leotheras whirlwind",
        NextAction::array(0, new NextAction("ssc leotheras flee whirlwind", 100.0f), NULL)));
}

void LeotherasFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end leotheras fight",
        NextAction::array(0, new NextAction("disable leotheras fight strategy", 100.0f), NULL)));
}

void LeotherasFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end leotheras fight",
        NextAction::array(0, new NextAction("disable leotheras fight strategy", 100.0f), NULL)));
}

// ----- Lady Vashj -----

void VashjFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Static Charge: move away from group
    triggers.push_back(new TriggerNode(
        "ssc vashj static charge",
        NextAction::array(0, new NextAction("ssc vashj move from group", 100.0f), NULL)));
}

void VashjFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end vashj fight",
        NextAction::array(0, new NextAction("disable vashj fight strategy", 100.0f), NULL)));
}

void VashjFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end vashj fight",
        NextAction::array(0, new NextAction("disable vashj fight strategy", 100.0f), NULL)));
}
