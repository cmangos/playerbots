#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class FocusHealTargetValue : public ManualSetValue<std::list<ObjectGuid>>, public Qualified
	{
	public:
        FocusHealTargetValue(PlayerbotAI* ai) : ManualSetValue<std::list<ObjectGuid>>(ai, {}), Qualified() {}
    };
}
