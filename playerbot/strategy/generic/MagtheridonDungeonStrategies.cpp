
#include "playerbot/playerbot.h"
#include "MagtheridonDungeonStrategies.h"

using namespace ai;

void MagtheridonDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start magtheridon fight",
        NextAction::array(0, new NextAction("enable magtheridon fight strategy", 100.0f), NULL)));
}

void MagtheridonFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Blast Nova: click Manticron Cubes to banish boss
    triggers.push_back(new TriggerNode(
        "magtheridon blast nova",
        NextAction::array(0, new NextAction("magtheridon use cube", 100.0f), NULL)));
}

void MagtheridonFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end magtheridon fight",
        NextAction::array(0, new NextAction("disable magtheridon fight strategy", 100.0f), NULL)));
}

void MagtheridonFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end magtheridon fight",
        NextAction::array(0, new NextAction("disable magtheridon fight strategy", 100.0f), NULL)));
}
