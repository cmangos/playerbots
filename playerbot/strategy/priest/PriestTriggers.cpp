
#include "playerbot/playerbot.h"
#include "PriestTriggers.h"
#include "PriestActions.h"

using namespace ai;

bool ShadowformTrigger::IsActive()
{
    return !ai->HasAura("shadowform", bot);
}

bool ShadowfiendTrigger::IsActive()
{
    return BoostTrigger::IsActive() && bot->IsSpellReady(34433);
}

bool ManaBurnTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;
    return target->IsPlayer();
}

