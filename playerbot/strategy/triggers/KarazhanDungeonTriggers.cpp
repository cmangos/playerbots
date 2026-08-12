
#include "playerbot/playerbot.h"
#include "KarazhanDungeonTriggers.h"
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

bool PrinceMalchezaarTooCloseTrigger::IsActive()
{
    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy && strategy->HasPullStarted())
        return false;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target) target = AI_VALUE(Unit*, "current target");
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
        return AI_VALUE(Unit*, "master target");

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