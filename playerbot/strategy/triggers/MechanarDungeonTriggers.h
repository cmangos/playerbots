#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{
	class MechanarEnterDungeonTrigger : public EnterDungeonTrigger
	{
	public:
		MechanarEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter mechanar", "mechanar", 554) {}
	};

	class MechanarLeaveDungeonTrigger : public LeaveDungeonTrigger
	{
	public:
		MechanarLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave mechanar", "mechanar", 554) {}
	};

	class NethermancerSepethreaStartFightTrigger : public StartBossFightTrigger
	{
	public:
		NethermancerSepethreaStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start nethermancer sepethrea fight", "nethermancer sepethrea", 19221) {}
	};

	class NethermancerSepethreaEndFightTrigger : public EndBossFightTrigger
	{
	public:
		NethermancerSepethreaEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end nethermancer sepethrea fight", "nethermancer sepethrea", 19221) {}
	};

	class RagingFlamesTooCloseTrigger : public CloseToCreatureTrigger
	{
	public:
		RagingFlamesTooCloseTrigger(PlayerbotAI* ai) : CloseToCreatureTrigger(ai, "raging flames too close", 20481, 15.0f, true) {}
	};
}