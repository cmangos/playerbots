
#include "playerbot/playerbot.h"
#include "NearestAdsValue.h"

using namespace ai;

bool NearestAddsValue::AcceptUnit(Unit* unit)
{
    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
    return unit != target && PossibleTargetsValue::AcceptUnit(unit);
}
