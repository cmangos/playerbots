#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class ActiveSpellValue : public CalculatedValue<uint32>
	{
	public:
        ActiveSpellValue(PlayerbotAI* ai, std::string name = "active spell") : CalculatedValue<uint32>(ai, name) {}

        virtual uint32 Calculate();
    };
}
