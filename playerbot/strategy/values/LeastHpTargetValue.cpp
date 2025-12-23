
#include "playerbot/playerbot.h"
#include "LeastHpTargetValue.h"
#include "TargetValue.h"

using namespace ai;

Unit* LeastHpTargetValue::Calculate()
{
    FindLeastHpTargetStrategy strategy(ai);
    return FindTarget(&strategy);
}
