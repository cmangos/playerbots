#include "BossFocusManager.h"
#include "Entities/Creature.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Spells/Spell.h"
#include "Chat/Chat.h"

using namespace ai;

Creature* BossFocusManager::FindFocusMob()
{
    if (!ctx.focusMobEntry || ctx.focusMobKilled)
        return nullptr;

    Creature* focusMob = nullptr;
    bool foundDead = false;

    // Try stored GUID first
    if (ctx.focusMobGuid)
    {
        if (Creature* c = bot->GetMap()->GetCreature(ctx.focusMobGuid))
        {
            if (c->IsAlive())
                return c;
            else
                foundDead = true;
        }
    }

    // Search nearby
    if (!focusMob && !foundDead)
    {
        std::list<Creature*> creatures;
        MaNGOS::AnyUnitInObjectRangeCheck checker(bot, 500.0f);
        MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
        Cell::VisitWorldObjects(bot, searcher, 500.0f);
        for (auto& c : creatures)
        {
            if (c->GetEntry() == ctx.focusMobEntry)
            {
                if (c->IsAlive())
                {
                    focusMob = c;
                    ctx.focusMobGuid = c->GetObjectGuid();
                    break;
                }
                else
                    foundDead = true;
            }
        }
    }

    // Search map object store
    if (!focusMob && !foundDead)
    {
        auto& objectStore = bot->GetMap()->GetObjectsStore();
        for (auto itr = objectStore.begin<Creature>(); itr != objectStore.end<Creature>(); ++itr)
        {
            if (Creature* c = itr->second)
            {
                if (c->GetEntry() == ctx.focusMobEntry)
                {
                    if (c->IsAlive())
                    {
                        focusMob = c;
                        ctx.focusMobGuid = c->GetObjectGuid();
                    }
                    else
                        foundDead = true;
                    break;
                }
            }
        }
    }

    if (foundDead && !focusMob)
    {
        ctx.focusMobKilled = true;
        sLog.outString("[BossFocus] boss (entry %u) confirmed DEAD", ctx.focusMobEntry);
    }

    return focusMob;
}

void BossFocusManager::RespawnFocusMob()
{
    uint32 now = WorldTimer::getMSTime();
    if (now - lastFocusRespawnTime > 30000)
    {
        lastFocusRespawnTime = now;
        Creature* focusMob = bot->SummonCreature(ctx.focusMobEntry,
            bot->GetPositionX() + 5.0f, bot->GetPositionY(), bot->GetPositionZ(),
            bot->GetOrientation() + M_PI_F, TEMPSPAWN_MANUAL_DESPAWN, 0);
        if (focusMob)
        {
            ctx.focusMobGuid = focusMob->GetObjectGuid();
            sLog.outString("[BossFocus] re-spawned %s (entry %u)", focusMob->GetName(), ctx.focusMobEntry);
        }
    }
}

void BossFocusManager::EngageFocusMob(Creature* focusMob)
{
    // Disable leashing
    focusMob->GetCombatManager().SetLeashingDisable(true);

    // Ensure boss attacks bot
    if (focusMob->AI() && !focusMob->GetVictim())
        focusMob->AI()->AttackStart(bot);

    // Check if retarget needed
    Unit* currentTarget = bot->GetVictim();
    bool needRetarget = !currentTarget || currentTarget->GetObjectGuid() != focusMob->GetObjectGuid();

    if (needRetarget)
    {
        bot->SetSelectionGuid(focusMob->GetObjectGuid());
        bot->Attack(focusMob, true);

        // Engage group members
        if (Group* group = bot->GetGroup())
        {
            for (auto itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* member = itr->getSource();
                if (member && member != bot && member->IsAlive())
                {
                    member->SetSelectionGuid(focusMob->GetObjectGuid());
                    member->Attack(focusMob, true);
                }
            }
        }
    }
}

void BossFocusManager::LogObserveStatus()
{
    uint32 now = WorldTimer::getMSTime();
    if (now - lastObserveLogTime > 30000)
    {
        uint32 elapsed = (now - ctx.testStartTime) / 1000;

        std::string targetInfo = "no target";
        if (Unit* target = bot->GetVictim())
        {
            float hpPct = target->GetHealthPercent();
            targetInfo = std::string(target->GetName()) + " HP=" + std::to_string((int)hpPct) + "%";
        }

        sLog.outString("[TestAction] Bot %s observing test %s, elapsed %us, monitors: %u, alive: %s, map: %u, combat: %s, target: %s",
            bot->GetName(), ctx.testName.c_str(), elapsed,
            (uint32)ctx.monitors.size(), bot->IsAlive() ? "yes" : "no",
            bot->GetMapId(), bot->IsInCombat() ? "yes" : "no", targetInfo.c_str());
        lastObserveLogTime = now;
    }
}

void BossFocusManager::Update()
{
    if (!ctx.focusMobEntry || !ctx.observing)
        return;

    LogObserveStatus();

    Creature* focusMob = FindFocusMob();

    if (!focusMob && !ctx.focusMobKilled)
        RespawnFocusMob();
    else if (focusMob)
        EngageFocusMob(focusMob);
}
