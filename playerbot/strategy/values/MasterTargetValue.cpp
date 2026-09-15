
#include "playerbot/playerbot.h"
#include "MasterTargetValue.h"
#include "TargetValue.h"

using namespace ai;

ObjectGuid MasterTargetValue::Calculate()
{
    return ai->GetGroupMaster() ? ai->GetGroupMaster()->GetObjectGuid() : ObjectGuid();
}