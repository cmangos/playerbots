#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class WaitForAttackTimeValue : public ManualSetValue<uint8>, public Qualified
	{
	public:
        WaitForAttackTimeValue(PlayerbotAI* ai) : ManualSetValue<uint8>(ai, 10), Qualified() {}
        virtual std::string Save() override { return std::to_string(value); }
        virtual bool Load(std::string inValue) override { value = stoi(inValue); return true; }
    };
}
