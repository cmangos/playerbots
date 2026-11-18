
#include "playerbot/playerbot.h"
#include "HunterTriggers.h"
#include "HunterActions.h"

using namespace ai;

bool HunterNoAmmoTrigger::IsActive()
{
    Item const* ranged = bot->GetWeaponForAttack(RANGED_ATTACK, true, false);
    if (!ranged)
        return false;

    uint32 subclass = ranged->GetProto()->SubClass;
    if (subclass != ITEM_SUBCLASS_WEAPON_BOW && subclass != ITEM_SUBCLASS_WEAPON_GUN &&
        subclass != ITEM_SUBCLASS_WEAPON_CROSSBOW)
        return false;

    return !HunterEquipAmmoAction::HasAmmoEquipped(bot, ranged) &&
        (ai->HasCheat(BotCheatMask::item) || AI_VALUE2(uint32, "item count", "ammo") > 0);
}

bool HunterNoStingsActiveTrigger::IsActive()
{
	Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    return target && AI_VALUE2(uint8, "health", "current target") > 40 &&
        !ai->HasAura("serpent sting", target) &&
        !ai->HasAura("scorpid sting", target) &&
        !ai->HasAura("viper sting", target);
}

bool HuntersPetDeadTrigger::IsActive()
{
    return AI_VALUE(bool, "pet dead") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HuntersPetLowHealthTrigger::IsActive()
{
    Unit* pet = ai->GetUnit(AI_VALUE(ObjectGuid, "pet target"));
    return pet && AI_VALUE2(uint8, "health", "pet target") < 40 &&
        !AI_VALUE2(bool, "dead", "pet target") && !AI_VALUE2(bool, "mounted", "self target");
}

bool HunterPetNotHappy::IsActive()
{
    return !AI_VALUE(bool, "pet happy") && !AI_VALUE2(bool, "mounted", "self target");
}

bool ViperStingOnAttackerTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        const bool noStings = !ai->HasAura("serpent sting", target) &&
                              !ai->HasAura("scorpid sting", target) &&
                              !ai->HasAura("viper sting", target);
        if (noStings)
        {
            if (target->GetPower(POWER_MANA) >= 10)
            {
                return DebuffOnAttackerTrigger::IsActive();
            }
        }
    }

    return false;
}

bool SerpentStingOnAttackerTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        const bool noStings = !ai->HasAura("serpent sting", target) &&
                              !ai->HasAura("scorpid sting", target) &&
                              !ai->HasAura("viper sting", target);
        if (noStings)
        {
            if (target->GetPower(POWER_MANA) < 10 || ai->HasStrategy("sting serpent", BotState::BOT_STATE_COMBAT))
            {
                return DebuffOnAttackerTrigger::IsActive();
            }
        }
    }

    return false;
}
