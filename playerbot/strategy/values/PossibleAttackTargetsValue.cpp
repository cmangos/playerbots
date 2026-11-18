
#include "playerbot/playerbot.h"
#include "PossibleAttackTargetsValue.h"
#include "PossibleTargetsValue.h"

#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "AttackersValue.h"
#include "RtiTargetValue.h"
#include "EnemyPlayerValue.h"

using namespace ai;
using namespace MaNGOS;

std::list<ObjectGuid> PossibleAttackTargetsValue::Calculate()
{
    std::list<ObjectGuid> result;
    if (ai->AllowActivity(ALL_ACTIVITY))
    {
        if (bot->IsInWorld() && !bot->IsBeingTeleported())
        {
            // Check if we only need one possible attack target
            bool getOne = false;
            if (!qualifier.empty())
            {
                getOne = stoi(qualifier);
            }

            if (getOne)
            {
                // Try to get one possible attack target
                result = AI_VALUE2(std::list<ObjectGuid>, "attackers", 1);
                RemoveNonThreating(result, getOne);
            }

            // If the one possible attack target failed, retry with multiple attackers
            if (result.empty())
            {
                result = AI_VALUE(std::list<ObjectGuid>, "attackers");
                RemoveNonThreating(result, getOne);
            }
        }
    }

	return result;
}

void PossibleAttackTargetsValue::RemoveNonThreating(std::list<ObjectGuid>& targets, bool getOne)
{
    std::list<ObjectGuid> breakableCC;
    std::list<ObjectGuid> unbreakableCC;

    for (auto it = targets.begin(); it != targets.end();)
    {
        auto candidate = it++;
        Unit* target = ai->GetUnit(*candidate);
        if (!IsValid(target, bot, sPlayerbotAIConfig.sightDistance, true, false))
        {
            targets.erase(candidate);
            continue;
        }

        if (!HasIgnoreCCRti(target, bot))
        {
            if (HasBreakableCC(target, bot))
            {
                breakableCC.splice(breakableCC.end(), targets, candidate);
                continue;
            }
            if (HasUnBreakableCC(target, bot))
            {
                unbreakableCC.splice(unbreakableCC.end(), targets, candidate);
                continue;
            }
        }

        if (getOne)
        {
            targets.erase(targets.begin(), candidate);
            targets.erase(it, targets.end());
            return;
        }
    }

    if (targets.empty())
        targets.swap(unbreakableCC.empty() ? breakableCC : unbreakableCC);
}

bool PossibleAttackTargetsValue::HasIgnoreCCRti(Unit* target, Player* player)
{
    return RtiTargetValue::IsTarget(player, target);
}

bool PossibleAttackTargetsValue::HasBreakableCC(Unit* target, Player* player)
{
    if (target->IsPolymorphed())
    {
        return true;
    }

    if (sServerFacade.IsFrozen(target))
    {
        return true;
    }

    PlayerbotAI* ai = player->GetPlayerbotAI();
    if (ai)
    {
        if (ai->HasAura("sap", target))
        {
            return true;
        }

        if (ai->HasAura("gouge", target))
        {
            return true;
        }

        if (ai->HasAura("shackle undead", target))
        {
            return true;
        }
    }

    return false;
}

bool PossibleAttackTargetsValue::HasUnBreakableCC(Unit* target, Player* player)
{
    if (target->IsStunned())
    {
        return true;
    }

    if (sServerFacade.IsFeared(target))
    {
        return true;
    }

    if (sServerFacade.IsInRoots(target))
    {
        return true;
    }

    return false;
}

bool PossibleAttackTargetsValue::IsCcTarget(Unit* attacker, Player* player)
{
    PlayerbotAI* ai = player->GetPlayerbotAI();
    if (ai)
    {
        Group* group = ai->GetBot()->GetGroup();
        if (group)
        {
            Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                Player *player = sObjectMgr.GetPlayer(itr->guid);
                if (!player || !sServerFacade.IsAlive(player) || !ai->IsSafe(player))
                    continue;

                if (player->GetPlayerbotAI())
                {
                    if (PAI_VALUE(ObjectGuid,"rti cc target") == attacker->GetObjectGuid())
                        return true;

                    std::string rti = PAI_VALUE(std::string,"rti cc");
                    int index = RtiTargetValue::GetRtiIndex(rti);
                    if (index != -1)
                    {
                        uint64 guid = group->GetTargetIcon(index);
                        if (guid && attacker->GetObjectGuid() == ObjectGuid(guid))
                            return true;
                    }
                }
            }
        }
    }

    return false;
}

bool PossibleAttackTargetsValue::IsImmuneToDamage(Unit* target, Player* player)
{
    // Charmed
    if (sServerFacade.IsCharmed(target) && target->IsInTeam(player, true))
    {
        return true;
    }

    // Immune to damage
    // Before we check auras, check school derived immunity for creatures
    if (target->IsCreature())
    {
        if(((Creature*)target)->GetCreatureInfo()->SchoolImmuneMask == SPELL_SCHOOL_MASK_ALL)
            return true;
    }


    PlayerbotAI* ai = player->GetPlayerbotAI();
    if (!ai)
        return false;

    for (const Aura* aura : ai->GetAuras(target))
    {
        const SpellEntry* spellInfo = aura->GetSpellProto();
        if (!spellInfo)
            continue;

        if (spellInfo->Mechanic == MECHANIC_BANISH)
            return true;

        if (!aura->IsPositive())
            continue;

        if (spellInfo->Mechanic == MECHANIC_INVULNERABILITY ||
            spellInfo->Mechanic == MECHANIC_IMMUNE_SHIELD)
        {
            return true;
        }
    }

    return false;
}

std::string PossibleAttackTargetsValue::Format()
{
    std::ostringstream out;

    for (auto& target : value)
    {
        if (target != value.front())
            out << ",";

        WorldObject* wo = ai->GetWorldObject(target);

        if (wo)
            out << wo->GetName();
        else
            out << target;
    }

    return out.str();
}

bool PossibleAttackTargetsValue::IsTapped(Unit* target, Player* player)
{
    if (player)
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();

        if (ai && ai->HasAura("tame beast", target))
            return false;

        const Creature* creature = dynamic_cast<Creature*>(target);
        if (creature)
        {
            Unit* victim = creature->GetVictim();
            Player* master = ai ? ai->GetMaster() : nullptr;
            PlayerbotAI* ai = player->GetPlayerbotAI();

             if (!victim) //Target is not attacking anything.
                return true;

            if (master && victim == master) //Target is attacking master.
                return true;

            if (player->IsInGroup(victim)) //Target is attacking groupmember.
                return true;

            if (!creature->HasLootRecipient()) //Target is untapped.
                return true;

            if (creature->IsTappedBy(player)) //Target is tapped by player.
                return true;

            if (master && target->getThreatManager().getThreat(master)) //Master as threat
                return true;

            if (ai && ai->HasStrategy("attack tagged", BotState::BOT_STATE_NON_COMBAT)) //Can attack tagged.
                if(ai->HasActivePlayerMaster() || !player->GetGroup() || player->GetGroup()->IsLeader(player->GetObjectGuid())) //Playing with a player or not in a group or master of group.
                    return true;
        }
    }

    return false;
}

bool PossibleAttackTargetsValue::IsValid(Unit* target, Player* player, float range, bool ignoreCC, bool checkAttackerValid)
{
    // Check for the valid attackers value
    if (checkAttackerValid && !AttackersValue::IsValid(target, player))
    {
        return false;
    }

    if (!IsPossibleTarget(target, player, range, ignoreCC))
        return false;

    if (sServerFacade.GetThreatManager(target).getCurrentVictim())
        return true;

    if (target->GetGuidValue(UNIT_FIELD_TARGET))
        return true;

    if (target->GetObjectGuid().IsPlayer())
        return true;

    PlayerbotAI* playerAI = player->GetPlayerbotAI();
    if (playerAI && (target->GetObjectGuid() == PAI_VALUE(ObjectGuid, "attack target")))
        return true;

    const bool isRtiTarget = HasIgnoreCCRti(target, player);
    if (!isRtiTarget && (HasBreakableCC(target, player) || HasUnBreakableCC(target, player)))
        return true;

    return isRtiTarget && playerAI && !playerAI->HasActivePlayerMaster();
}

bool PossibleAttackTargetsValue::IsPossibleTarget(Unit* target, Player* player, float range, bool ignoreCC)
{
    if(target)
    {
        // If the target is in an attackable distance
        if(!player->IsWithinDistInMap(target, range))
        {
            return false;
        }

        // If the target is immune to any damage
        if (IsImmuneToDamage(target, player))
        {
            return false;
        }

        // If the target is CC'ed or is a CC target (don't place a dot before we recast cc...)
        if(!ignoreCC && !HasIgnoreCCRti(target, player) && (HasBreakableCC(target, player) || HasUnBreakableCC(target, player)) || IsCcTarget(target, player))
        {
            return false;
        }

        // If the target is a NPC
        Player* enemyPlayer = dynamic_cast<Player*>(target);
        if (!enemyPlayer)
        {
            // If the target is not tapped (the player doesn't have the loot rights (gray name))
            if (!IsTapped(target, player))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool PossibleAddsValue::Calculate()
{
    PlayerbotAI *ai = bot->GetPlayerbotAI();
    std::list<ObjectGuid> possible = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible targets no los")->Get();
    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();

    for (std::list<ObjectGuid>::iterator i = possible.begin(); i != possible.end(); ++i)
    {
        ObjectGuid guid = *i;
        if (find(attackers.begin(), attackers.end(), guid) != attackers.end()) continue;

        Unit* add = ai->GetUnit(guid);
        if (add && !add->GetGuidValue(UNIT_FIELD_TARGET) && !sServerFacade.GetThreatManager(add).getCurrentVictim() && sServerFacade.IsHostileTo(add, bot))
        {
            for (std::list<ObjectGuid>::iterator j = attackers.begin(); j != attackers.end(); ++j)
            {
                Unit* attacker = ai->GetUnit(*j);
                if (!attacker) continue;

                float dist = sServerFacade.GetDistance2d(attacker, add);
                if (sServerFacade.IsDistanceLessOrEqualThan(dist, sPlayerbotAIConfig.aoeRadius * 1.5f)) continue;
                if (sServerFacade.IsDistanceLessOrEqualThan(dist, sPlayerbotAIConfig.aggroDistance)) return true;
            }
        }
    }
    return false;
}