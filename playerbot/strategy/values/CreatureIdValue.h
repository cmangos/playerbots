#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
    class CreatureIdValue : public CalculatedValue<uint32>, public Qualified
	{
	public:
        CreatureIdValue(PlayerbotAI* ai);
        virtual uint32 Calculate() override;
    };
}
