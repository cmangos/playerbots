
#include "playerbot/playerbot.h"
#include "DKTriggers.h"
#include "DKActions.h"

using namespace ai;

bool DKPresenceTrigger::IsActive()
{
    Unit* target = GetTarget();
    return !ai->HasAura("blood presence", target) &&
        !ai->HasAura("unholy presence", target) &&
        !ai->HasAura("frost presence", target);
}

bool ObliterateTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellCanBeCastedTrigger::IsActive() &&
        ai->HasAura("blood plague", target, false, true) &&
        ai->HasAura("frost fever", target, false, true);
}