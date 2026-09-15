#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class SelfTargetValue : public UnitCalculatedValue
	{
	public:
        SelfTargetValue(PlayerbotAI* ai, std::string name = "self target") : UnitCalculatedValue(ai, name) {}

        virtual ObjectGuid Calculate() override { return ai->GetBot() ? ai->GetBot()->GetObjectGuid() : ObjectGuid(); }
    };
}
