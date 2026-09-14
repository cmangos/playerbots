
#include "playerbot/playerbot.h"
#include "CurrentTargetValue.h"

#include "playerbot/ServerFacade.h"
using namespace ai;

ObjectGuid CurrentTargetValue::Get()
{
    if (selection.IsEmpty())
        return ObjectGuid();

    Unit* unit = sObjectAccessor.GetUnit(*bot, selection);
    if (unit && !bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance))
        return ObjectGuid();

    return unit ? unit->GetObjectGuid() : ObjectGuid();
}

void CurrentTargetValue::Set(ObjectGuid unitGuid)
{
    selection = unitGuid;
}
