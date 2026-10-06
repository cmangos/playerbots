
#include "playerbot/playerbot.h"
#include "KarazhanDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

void KarazhanDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"start netherspite fight",
		NextAction::array(0, new NextAction("enable netherspite fight strategy", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"start shade of aran fight",
		NextAction::array(0, new NextAction("enable shade of aran fight strategy", 100.0f), NULL)));

triggers.push_back(new TriggerNode(
		"start big bad wolf fight",
		NextAction::array(0, new NextAction("enable big bad wolf fight strategy", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"start prince malchezaar fight",
		NextAction::array(0, new NextAction("enable prince malchezaar fight strategy", 100.0f), NULL)));
}

void NetherspiteFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"void zone too close",
		NextAction::array(0, new NextAction("move away from void zone", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"add nether portal - perseverence for tank",
		NextAction::array(0, new NextAction("add nether portal - perseverence for tank", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"remove nether portal buffs from netherspite",
		NextAction::array(0, new NextAction("remove nether portal buffs from netherspite", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"remove nether portal - perseverence",
		NextAction::array(0, new NextAction("remove nether portal - perseverence", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"remove nether portal - serenity",
		NextAction::array(0, new NextAction("remove nether portal - serenity", 101.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"remove nether portal - dominance",
		NextAction::array(0, new NextAction("remove nether portal - dominance", 101.0f), NULL)));
}

void NetherspiteFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end netherspite fight",
		NextAction::array(0, new NextAction("disable netherspite fight strategy", 100.0f), NULL)));
}

void NetherspiteFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end netherspite fight",
		NextAction::array(0, new NextAction("disable netherspite fight strategy", 100.0f), NULL)));
}

void NetherspiteFightStrategy::OnStrategyAdded(BotState state)
{
	if (!Get(ai)->GetBossGuid())
	{
		// Find Netherspite and store him
		std::list<Unit*> creatures;
        MaNGOS::AllCreaturesOfEntryInRangeCheck u_check(ai->GetBot(), 15689, 100);
		MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, u_check);
        Cell::VisitAllObjects(ai->GetBot(), searcher, 100);

		if (creatures.empty())
            return;

		Unit* target = creatures.front();

		NetherspiteFightStrategy* strategy = NetherspiteFightStrategy::Get(ai);
		if (strategy)
		{
			if (target && target->GetEntry() == 15689)
			{
				strategy->SetBossGuid(target->GetObjectGuid());
			}
		}
	}
}

NetherspiteFightStrategy* NetherspiteFightStrategy::Get(PlayerbotAI* ai)
{
    return ai ? ai->GetStrategy<NetherspiteFightStrategy>("netherspite", BotState::BOT_STATE_COMBAT) : nullptr;
}

void ShadeOfAranFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"shade of aran casting flame wreath",
		NextAction::array(0, new NextAction("start aran fire phase", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"shade of aran casting arcane explosion",
		NextAction::array(0, new NextAction("start aran arcane phase", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"shade of aran elementals out",
		NextAction::array(0, new NextAction("start aran elementals", 100.0f), NULL)));
}

void ShadeOfAranFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end shade of aran fight",
		NextAction::array(0, new NextAction("disable shade of aran fight strategy", 100.0f), NULL)));
}

void ShadeOfAranFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end shade of aran fight",
		NextAction::array(0, new NextAction("disable shade of aran fight strategy", 100.0f), NULL)));
}

void ShadeOfAranFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"shade of aran casting blizzard",
		NextAction::array(0, new NextAction("start aran frost phase", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"shade of aran done casting arcane explosion",
		NextAction::array(0, new NextAction("end aran arcane phase", 100.0f), NULL)));

	triggers.push_back(new TriggerNode(
		"shade of aran casting arcane explosion",
		NextAction::array(0, new NextAction("move away from shade of aran", 100.0f), NULL)));
}

ShadeOfAranFightStrategy* ShadeOfAranFightStrategy::Get(PlayerbotAI* ai)
{
    return ai ? ai->GetStrategy<ShadeOfAranFightStrategy>("shade of aran", BotState::BOT_STATE_COMBAT) : nullptr;
}

void ShadeOfAranFightStrategy::OnStrategyAdded(BotState state)
{
	if (!Get(ai)->GetBossGuid())
	{
		// Find Shade of aran and store him
		std::list<Unit*> creatures;
        MaNGOS::AllCreaturesOfEntryInRangeCheck u_check(ai->GetBot(), 16524, 100);
		MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, u_check);
        Cell::VisitAllObjects(ai->GetBot(), searcher, 100);

		if (creatures.empty())
            return;

		Unit* target = creatures.front();

		ShadeOfAranFightStrategy* strategy = ShadeOfAranFightStrategy::Get(ai);
		if (strategy)
		{
			if (target && target->GetEntry() == 16524)
			{
				strategy->SetBossGuid(target->GetObjectGuid());
				// fight starts with unknown phase
				strategy->SetPhase(AranPhase::PHASE_NONE);
				strategy->SetRangedBot(ai->IsRanged(ai->GetBot()));
				if (target->GetHealthPercent() <= 40.0f && !strategy->GetElementals())
					strategy->SetElementals(true);
				else
					strategy->SetElementals(false);
				if (ai->GetRange("flee") < 5.0f)
                    ai->GetAiObjectContext()->GetValue<float>("range", "flee")->Set(20.0f);
                
			}
		}
	}
}

void BigBadWolfFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"big bad wolf too close",
		NextAction::array(0, new NextAction("move away from big bad wolf", 100.0f), NULL)));
}

void BigBadWolfFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end big bad wolf fight",
		NextAction::array(0, new NextAction("disable big bad wolf fight strategy", 100.0f), NULL)));
}

void BigBadWolfFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end big bad wolf fight",
		NextAction::array(0, new NextAction("disable big bad wolf fight strategy", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"netherspite infernal too close",
		NextAction::array(0, new NextAction("move away from netherspite infernal", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end prince malchezaar fight",
		NextAction::array(0, new NextAction("disable prince malchezaar fight strategy", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"end prince malchezaar fight",
		NextAction::array(0, new NextAction("disable prince malchezaar fight strategy", 100.0f), NULL)));
}

void PrinceMalchezaarFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
	triggers.push_back(new TriggerNode(
		"prince malchezaar too close",
		NextAction::array(0, new NextAction("move away from prince malchezaar", 100.0f), NULL)));
}
