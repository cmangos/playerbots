
#include "playerbot/playerbot.h"
#include "MechanarDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

void MechanarDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"start nethermancer sepethrea fight",
		NextAction::array(0, new NextAction("enable nethermancer sepethrea fight strategy", 100.0f), NULL)));
}

void NethermancerSepethreaFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"raging flames too close",
		NextAction::array(0, new NextAction("move away from raging flames", 100.0f), NULL)));
}

void NethermancerSepethreaFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end nethermancer sepethrea fight",
		NextAction::array(0, new NextAction("disable nethermancer sepethrea fight strategy", 100.0f), NULL)));
}

void NethermancerSepethreaFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end nethermancer sepethrea fight",
		NextAction::array(0, new NextAction("disable nethermancer sepethrea fight strategy", 100.0f), NULL)));
}
