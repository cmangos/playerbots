#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class CurrentTargetValue : public UnitManualSetValue
	{
	public:
        CurrentTargetValue(PlayerbotAI* ai, std::string name = "current target") : UnitManualSetValue(ai, ObjectGuid(), name) {}
        virtual ObjectGuid Get() override;
        virtual void Set(ObjectGuid unitGuid) override;

    private:
        ObjectGuid selection;
    };
}
