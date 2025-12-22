#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class CurrentTargetValue : public UnitManualSetValue
	{
	public:
        CurrentTargetValue(PlayerbotAI* ai, std::string name = "current target") : UnitManualSetValue(ai, nullptr, name) {}
        virtual Unit* Get() override;
        virtual void Set(Unit* unit) override;

    private:
        ObjectGuid selection;
    };
}
