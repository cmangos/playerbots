
#include "playerbot/playerbot.h"
#include "OnyxiasLairDungeonStrategies.h"

using namespace ai;

void OnyxiasLairDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start onyxia fight",
        NextAction::array(0, new NextAction("enable onyxia fight strategy", 100.0f), NULL)));
}

void OnyxiaFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // ----------- Phase 1 (100% - 65%) -----------

    triggers.push_back(new TriggerNode(
        "ony near tail",
        NextAction::array(0, new NextAction("ony move to side", 60 + 2), NULL))); // 60 -> AC ACTION_RAID

    triggers.push_back(new TriggerNode(
        "ony avoid eggs",
        NextAction::array(0, new NextAction("ony avoid eggs move", ACTION_EMERGENCY + 5), NULL)));

    // ----------- Phase 2 (65% - 40%) -----------

    triggers.push_back(new TriggerNode(
        "ony deep breath warning",
        NextAction::array(0, new NextAction("ony move to safe zone", ACTION_EMERGENCY + 5), NULL)));

    triggers.push_back(new TriggerNode(
        "ony fireball splash incoming",
        NextAction::array(0, new NextAction("ony spread out", ACTION_EMERGENCY + 2), NULL)));

    triggers.push_back(new TriggerNode(
        "ony whelps spawn",
        NextAction::array(0, new NextAction("ony kill whelps", 60 + 1), NULL))); // 60 -> AC ACTION_RAID
}

void OnyxiaFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end onyxia fight",
        NextAction::array(0, new NextAction("disable onyxia fight strategy", 100.0f), NULL)));
}

void OnyxiaFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end onyxia fight",
        NextAction::array(0, new NextAction("disable onyxia fight strategy", 100.0f), NULL)));
}

void OnyxiaFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // ...
}
