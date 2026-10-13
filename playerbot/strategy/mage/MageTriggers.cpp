
#include "playerbot/playerbot.h"
#include "MageTriggers.h"
#include "MageActions.h"

using namespace ai;

bool AnyMageArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !ai->HasAura("ice armor", target) &&
           !ai->HasAura("frost armor", target) &&
           !ai->HasAura("molten armor", target) &&
           !ai->HasAura("mage armor", target);
}

bool MageArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !ai->HasAura("mage armor", target);
}

bool IceArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !ai->HasAura("ice armor", target) &&
           !ai->HasAura("frost armor", target);
}

bool MoltenArmorTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !ai->HasAura("molten armor", target);
}

bool ManaShieldTrigger::IsActive()
{
    return !ai->HasAura("mana shield", bot) && AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumMana;
}

bool IceLanceTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && ai->HasAnyAuraOf(target, "frost nova", "frostbite", NULL);
}

bool HotStreakTrigger::IsActive()
{
    //Usage by id. By name hot streak aura doesn't work.
    return ai->HasAura(48108, bot);
}

bool FireballOrFrostfireBoltFreeTrigger::IsActive()
{
    return ai->HasAura("fireball!", bot);
}

bool NoImprovedScorchDebuffTrigger::IsActive()
{
    if (bot->HasSpell(11095) || bot->HasSpell(12872) || bot->HasSpell(12873))
    {
        return DebuffTrigger::IsActive();
    }

    return false;
}

bool ArcaneBlastTrigger::IsActive()
{
    // when not max stacks of arcane blast, this is true
    Unit* target = GetTarget();
#ifdef MANGOSBOT_TWO
	return target && !ai->HasAura(spell, target, true, checkIsOwner) && target->IsAlive();
#else
	return target && (!ai->HasAura(spell, target, true, checkIsOwner) || AI_VALUE2(uint8, "mana", "self target") > sPlayerbotAIConfig.mediumMana) && target->IsAlive();
#endif
}

bool ArcaneBlastFullTrigger::IsActive()
{
    // when max stacks of arcane blast, this is true
    Unit* target = GetTarget();
	return target && (ai->HasAura(spell, target, true, checkIsOwner) && AI_VALUE2(uint8, "mana", "self target") <= sPlayerbotAIConfig.mediumMana) && target->IsAlive();
}