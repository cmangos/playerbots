#pragma once
#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    // Buff
    BUFF_TRIGGER(HornOfWinterTrigger, "horn of winter");
    BUFF_TRIGGER(BoneShieldTrigger, "bone shield");
    BUFF_TRIGGER(ImprovedIcyTalonsTrigger, "icy talons");
	BUFF_TRIGGER_A(DKPresenceTrigger, "blood presence");

	// Main abilities
	DEBUFF_TRIGGER_OWN(PlagueStrikeDebuffTrigger, "blood plague");
	DEBUFF_TRIGGER_OWN(IcyTouchDebuffTrigger, "frost fever");
	DEBUFF_ENEMY_TRIGGER_OWN(PlagueStrikeDebuffOnAttackerTrigger, "blood plague");
	DEBUFF_ENEMY_TRIGGER_OWN(IcyTouchDebuffOnAttackerTrigger, "frost fever");
	CAN_CAST_TRIGGER(FrostStrikeTrigger, "frost strike");
	CAN_CAST_TRIGGER_A(ObliterateTrigger, "obliterate");

	// Cds
	CAN_CAST_TRIGGER(SummonGargoyleTrigger, "summon gargoyle");
	CAN_CAST_TRIGGER(DancingWeaponTrigger, "dancing rune weapon");
	CAN_CAST_TRIGGER(UnholyFrenzyTrigger, "unholy frenzy");

	class BloodTapTrigger : public BuffTrigger {
	public:
		BloodTapTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "blood tap") {}
	};

	class RaiseDeadTrigger : public BuffTrigger {
	public:
		RaiseDeadTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "raise dead") {}
	};


	class RuneStrikeTrigger : public SpellCanBeCastedTrigger {
	public:
		RuneStrikeTrigger(PlayerbotAI* ai) : SpellCanBeCastedTrigger(ai, "rune strike") {}
	};

	CAN_CAST_TRIGGER(DeathCoilTrigger, "death coil");
	CAN_CAST_TRIGGER(PestilenceTrigger, "pestilence");
	CAN_CAST_TRIGGER(BloodStrikeTrigger, "blood strike");
	CAN_CAST_TRIGGER(HowlingBlastTrigger, "howling blast");

    class MindFreezeInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
		MindFreezeInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "mind freeze") {}
    };

	class StrangulateInterruptSpellTrigger : public InterruptSpellTrigger
	{
	public:
		StrangulateInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "strangulate") {}
	};

    class KillingMachineTrigger : public BoostTrigger
    {
    public:
		KillingMachineTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "killing machine") {}
    };

    class MindFreezeOnEnemyHealerTrigger : public InterruptEnemyHealerTrigger
    {
    public:
		MindFreezeOnEnemyHealerTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "mind freeze") {}
    };

	class ChainsOfIceSnareTrigger : public SnareTargetTrigger
	{
	public:
		ChainsOfIceSnareTrigger(PlayerbotAI* ai) : SnareTargetTrigger(ai, "chains of ice") {}
	};

	class StrangulateOnEnemyHealerTrigger : public InterruptEnemyHealerTrigger
	{
	public:
		StrangulateOnEnemyHealerTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "strangulate") {}
	};
}
