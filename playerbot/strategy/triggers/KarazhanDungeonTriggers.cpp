
#include "playerbot/playerbot.h"
#include "KarazhanDungeonTriggers.h"
#include "playerbot/strategy/generic/KarazhanDungeonStrategies.h"
#include "GenericTriggers.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;

bool NetherspiteBeamsCheatNeedRefreshTrigger::IsActive()
{
    //Checking that is portal phase
    std::list<Unit*> creatures;
    MaNGOS::AllCreaturesOfEntryInRangeCheck u_check(bot, 17369, 100);
    MaNGOS::UnitListSearcher<MaNGOS::AllCreaturesOfEntryInRangeCheck> searcher(creatures, u_check);
    Cell::VisitAllObjects(bot, searcher, 100);

    if (creatures.empty())
        return false;

    //Checking that is Netherspite target
    return AI_VALUE2(bool, "has aggro", "current target");
}

bool ShadeOfAranCastingArcaneExplosionTrigger::IsActive()
{
    //Checking that Shade of Aran is casting arcane explosion
    ShadeOfAranFightStrategy* strategy = ShadeOfAranFightStrategy::Get(ai);

    if (strategy)
    {
        // No need to check if we are on arcane currently
        if (strategy->GetPhase() == AranPhase::PHASE_ARCANE)
            return true;
            
        Unit* aran = ai->GetUnit(strategy->GetBossGuid());
        if (aran)
        {
            if (Spell const* genericSpell = aran->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                if (genericSpell->m_spellInfo->Id == 29973 && genericSpell->getState() != SPELL_STATE_FINISHED)
                    return true;
            } 
        }
    }

    return false;
}

bool ShadeOfAranDoneCastingArcaneExplosionTrigger::IsActive()
{
    //Checking that Shade of Aran is done casting arcane explosion
    ShadeOfAranFightStrategy* strategy = ShadeOfAranFightStrategy::Get(ai);

    if (strategy)
    {
        Unit* aran = ai->GetUnit(strategy->GetBossGuid());
        if (aran && strategy->GetPhase() == AranPhase::PHASE_ARCANE)
        {
            if (Spell const* genericSpell = aran->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                if (genericSpell->m_spellInfo->Id == 29973 && genericSpell->getState() == SPELL_STATE_FINISHED)
                    return true;
                else
                    return false;
            }
            else
                return true;
        }
    }

    return false;
}

bool ShadeOfAranCastingFlameWreathTrigger::IsActive()
{
    //Checking that Shade of Aran is casting flame wreath
    ShadeOfAranFightStrategy* strategy = ShadeOfAranFightStrategy::Get(ai);

    if (strategy)
    {
        // No need to check if we are on fire currently
        if (strategy->GetPhase() == AranPhase::PHASE_FIRE)
            return false;

        Unit* aran = ai->GetUnit(strategy->GetBossGuid());
        if (aran)
        {
            if (Spell const* genericSpell = aran->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                if (genericSpell->m_spellInfo->Id == 30004)
                    return true;
            } 
        }
    }

    return false;
}

bool ShadeOfAranCastingBlizzardTrigger::IsActive()
{
    //Checking that Shade of Aran is casting blizzard
    ShadeOfAranFightStrategy* strategy = ShadeOfAranFightStrategy::Get(ai);

    if (strategy)
    {
        // No need to check if we are on frost currently
        if (strategy->GetPhase() == AranPhase::PHASE_FROST)
            return false;

        Unit* aran = ai->GetUnit(strategy->GetBossGuid());
        if (aran)
        {
            if (Spell const* genericSpell = aran->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                if (genericSpell->m_spellInfo->Id == 29969)
                    return true;
            } 
        }
    }

    return false;
}

bool ShadeOfAranElementalsTrigger::IsActive()
{
    //Checking that the water elementals are out
    ShadeOfAranFightStrategy* strategy = ShadeOfAranFightStrategy::Get(ai);

    if (strategy && !strategy->GetElementals())
    {
        Unit* aran = ai->GetUnit(strategy->GetBossGuid());
        if (aran)
        {
            if (aran->GetHealthPercent() <= 40.0f)
                return true;
        }
    }

    return false;
}

bool PrinceMalchezaarTooCloseTrigger::IsActive()
{
    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy && strategy->HasPullStarted())
        return false;
    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "tank target"));
    if (!target) target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    if (bot->HasAura(30843) || (EnfeeblePart() && target && target->GetVictim() != bot) || MeleeWaitCheck(target)) 
        return true;
    if (ai->IsRanged(bot, true))
        return CloseToCreatureTrigger::IsActive();
    return false;
}

bool PrinceMalchezaarTooCloseTrigger::MeleeWaitCheck(Unit* target)
{
    // Check if we should be someone staying out of Shadow Nova blast
    if (target && target->GetHealthPercent() > 30.0f && target->IsCreature())
    {
        if (ai->IsMelee(bot, true) && target->GetVictim() != bot && target->GetDistance(bot) > 8.0f)
        {
            if (Spell const* genericSpell = target->GetCurrentSpell(CURRENT_GENERIC_SPELL))
            {
                if (genericSpell->m_spellInfo->Id == 30852 && genericSpell->getState() != SPELL_STATE_FINISHED)
                    return true;
            }
        }
    }

    return false;
}

bool PrinceMalchezaarTooCloseTrigger::EnfeeblePart()
{
    Group* group = bot->GetGroup();
    if (!group)
        return ai->GetUnit(AI_VALUE(ObjectGuid, "master target"));

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || !sServerFacade.IsAlive(member))
            continue;

        if (member->HasAura(30843))
            return true;
    }
    return false;
}