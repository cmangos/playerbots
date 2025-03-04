
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
    return SpellCanBeCastedTrigger::IsActive() && target &&
        ai->HasAura("blood plague", target, false, true) &&
        ai->HasAura("frost fever", target, false, true);
}

bool ScourgeStrikeTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellCanBeCastedTrigger::IsActive() && target &&
        ai->HasAura("blood plague", target, false, true) &&
        ai->HasAura("frost fever", target, false, true);
}

bool FrostStrikeTrigger::IsActive()
{
    return SpellCanBeCastedTrigger::IsActive() &&
        AI_VALUE2(uint8, "runic", "self target") > 80;
}


bool DeathCoilTrigger::IsActive()
{
    return SpellCanBeCastedTrigger::IsActive() &&
        AI_VALUE2(uint8, "runic", "self target") > 80;
}

bool PestilenceTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellCanBeCastedTrigger::IsActive() && 
        bot->HasSpell(63959) && target && // glyph of disease
        ai->HasAura("blood plague", target, false, true) &&
        ai->HasAura("frost fever", target, false, true);
}