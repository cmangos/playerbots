
#include "playerbot/playerbot.h"
#include "WarlockTriggers.h"
#include "WarlockActions.h"

using namespace ai;

bool DemonArmorTrigger::IsActive() 
{
	Unit* target = GetTarget();
	return !ai->HasAura("demon skin", target) &&
		   !ai->HasAura("demon armor", target) &&
		   !ai->HasAura("fel armor", target);
}

bool SpellstoneTrigger::IsActive() 
{
    return BuffTrigger::IsActive() && AI_VALUE2(uint32, "item count", getName()) > 0;
}

bool InfernoTrigger::IsActive()
{
	return AI_VALUE(uint8, "attackers count") > 1 && bot->HasSpell(1122) && bot->HasItemCount(5565, 1) && !urand(0, 2);
}

bool CorruptionTrigger::IsActive()
{
	Unit* target = GetTarget();
	return target && !ai->HasAura("corruption", target) && !ai->HasAura("seed of corruption", target) && !HasMaxDebuffs();
}

bool LifeTapTrigger::IsActive()
{
	const uint32 mana = AI_VALUE2(uint8, "mana", "self target");
	if (mana <= sPlayerbotAIConfig.lowMana)
	{
		const uint32 health = AI_VALUE2(uint8, "health", "self target");
		if (health >= sPlayerbotAIConfig.lowHealth)
		{
			return true;
		}
	}

	return false;
}

bool DrainSoulTrigger::IsActive()
{
	// If no item cheats enabled
    if (!ai->HasCheat(BotCheatMask::item))
    {
		// Check if it has less than 5 soul shards
        if (!bot->HasItemCount(6265, 5))
        {
			// Check if it has enough bag space
			if (AI_VALUE(uint8, "bag space") > 0)
			{
                // Check if target health is less than 15%
                const uint32 targetHealth = AI_VALUE2(uint8, "health", "current target");
                if (targetHealth <= 15)
                {
                    return true;
                }
			}
        }
	}

	return false;
}

bool CorruptionOnAttackerTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && target->IsAlive() && !ai->HasAura("corruption", target, false, true) && !ai->HasAura("seed of corruption", target, false, true);
}

bool NoCurseTrigger::IsActive()
{
	Unit* target = GetTarget();
	if (target)
	{
		return !ai->HasAura("curse of agony", target, false, true) &&
			   !ai->HasAura("curse of doom", target, false, true) &&
			   !ai->HasAura("curse of recklessness", target, false, true) &&
			   !ai->HasAura("curse of shadow", target, false, true) &&
			   !ai->HasAura("curse of the elements", target, false, true) &&
			   !ai->HasAura("curse of weakness", target, false, true) &&
			   !ai->HasAura("curse of tongues", target, false, true);
	}

	return false;
}

bool NoCurseOnAttackerTrigger::IsActive()
{
    list<ObjectGuid> attackers = AI_VALUE(list<ObjectGuid>, "possible attack targets");
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    for (list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* attacker = ai->GetUnit(*i);
        if (attacker && attacker != currentTarget)
        {
			if (!ai->HasAura("curse of agony", attacker, false, true) &&
				!ai->HasAura("curse of doom", attacker, false, true) &&
				!ai->HasAura("curse of recklessness", attacker, false, true) &&
				!ai->HasAura("curse of shadow", attacker, false, true) &&
				!ai->HasAura("curse of the elements", attacker, false, true) &&
				!ai->HasAura("curse of weakness", attacker, false, true) &&
				!ai->HasAura("curse of tongues", attacker, false, true))
			{
				return true;
			}
        }
    }

	return false;
}

bool FearPvpTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
	if (target && target->IsPlayer())
	{
		// Check if low health
        const uint8 health = AI_VALUE2(uint8, "health", "self target");
        if (health <= sPlayerbotAIConfig.lowHealth)
        {
			// Check if targeting bot
			if (target->GetSelectionGuid() == bot->GetObjectGuid())
			{
                // Check if the bot has feared anyone
                bool alreadyFeared = false;
                list<ObjectGuid> attackers = AI_VALUE(list<ObjectGuid>, "attackers");
                for (list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
                {
                    Unit* attacker = ai->GetUnit(*i);
                    if (ai->HasAura("fear", attacker, false, true))
                    {
                        alreadyFeared = true;
                        break;
                    }
                }

                if (!alreadyFeared)
                {
                    const float distance = target->GetDistance(bot);
                    return distance <= 10.0f;
                }
			}
		}
	}

	return false;
}

bool ConflagrateTrigger::IsActive()
{
	Unit* target = AI_VALUE(Unit*, "current target");
	if (target)
	{
		// Check if immolate in target
		Aura* aura = ai->GetAura("immolate", target, true);
		if (aura)
		{
			// Check if immolate is about to expire
			if (aura->GetAuraDuration() <= 7000)
			{
				return true;
			}
		}
	}

	return false;
}

bool DemonicSacrificeTrigger::IsActive()
{
	if (ai->HasStrategy("pet", BotState::BOT_STATE_COMBAT))
	{
		return ai->HasSpell(18788) &&
			   !ai->HasAura(18789, bot) && // Burning Wish (Imp)
			   !ai->HasAura(18790, bot) && // Fel Stamina (Voidwalker)
			   !ai->HasAura(18791, bot) && // Touch of Shadow (Succubus)
			   !ai->HasAura(18792, bot) && // Fel Energy (Felhunter)
			   !ai->HasAura(35701, bot);   // Touch of Shadow (Felguard)
	}

	return false;
}

bool SoulLinkTrigger::IsActive()
{
	return ai->HasSpell(19028) && !ai->HasAura(19028, bot) && AI_VALUE(Unit*, "pet target");
}

bool NoSpecificPetTrigger::IsActive()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    if (pet)
    {
        return pet->GetEntry() != entry;
    }

    return true;
}
