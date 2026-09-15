
#include "playerbot/playerbot.h"
#include "LeastHpTargetValue.h"
#include "TargetValue.h"

using namespace ai;

ObjectGuid LeastHpTargetValue::Calculate()
{
    FindLeastHpTargetStrategy strategy(ai);
    Unit* target = FindTarget(&strategy);
    return target ? target->GetObjectGuid() : ObjectGuid();
}
