
#include "playerbot/playerbot.h"
#include "DungeonStrategy.h"

using namespace ai;

void DungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Add this combat triggers in case the bot gets summoned into the dungeon and goes straight into combat
    triggers.push_back(new TriggerNode(
        "enter onyxia's lair",
        NextAction::array(0, new NextAction("enable onyxia's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter molten core",
        NextAction::array(0, new NextAction("enable molten core strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter blackwing lair",
        NextAction::array(0, new NextAction("enable blackwing lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter karazhan",
        NextAction::array(0, new NextAction("enable karazhan strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter gruul's lair",
        NextAction::array(0, new NextAction("enable gruul's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter magtheridon's lair",
        NextAction::array(0, new NextAction("enable magtheridon's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter serpentshrine cavern",
        NextAction::array(0, new NextAction("enable serpentshrine cavern strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter tempest keep",
        NextAction::array(0, new NextAction("enable tempest keep strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter zul'aman",
        NextAction::array(0, new NextAction("enable zul'aman strategy", 100.0f), NULL)));
}

void DungeonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "enter onyxia's lair",
        NextAction::array(0, new NextAction("enable onyxia's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave onyxia's lair",
        NextAction::array(0, new NextAction("disable onyxia's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter molten core",
        NextAction::array(0, new NextAction("enable molten core strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave molten core",
        NextAction::array(0, new NextAction("disable molten core strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter blackwing lair",
        NextAction::array(0, new NextAction("enable blackwing lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave blackwing lair",
        NextAction::array(0, new NextAction("disable blackwing lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter karazhan",
        NextAction::array(0, new NextAction("enable karazhan strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave karazhan",
        NextAction::array(0, new NextAction("disable karazhan strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter gruul's lair",
        NextAction::array(0, new NextAction("enable gruul's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave gruul's lair",
        NextAction::array(0, new NextAction("disable gruul's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter magtheridon's lair",
        NextAction::array(0, new NextAction("enable magtheridon's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave magtheridon's lair",
        NextAction::array(0, new NextAction("disable magtheridon's lair strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter serpentshrine cavern",
        NextAction::array(0, new NextAction("enable serpentshrine cavern strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave serpentshrine cavern",
        NextAction::array(0, new NextAction("disable serpentshrine cavern strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter tempest keep",
        NextAction::array(0, new NextAction("enable tempest keep strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave tempest keep",
        NextAction::array(0, new NextAction("disable tempest keep strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "enter zul'aman",
        NextAction::array(0, new NextAction("enable zul'aman strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "leave zul'aman",
        NextAction::array(0, new NextAction("disable zul'aman strategy", 100.0f), NULL)));
}