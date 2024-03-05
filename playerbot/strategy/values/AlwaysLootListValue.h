#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class AlwaysLootListValue : public ManualSetValue<std::set<uint32>&>
	{
	public:
        AlwaysLootListValue(PlayerbotAI* ai, std::string name = "always loot list") : ManualSetValue<std::set<uint32>&>(ai, list, name) {}

        virtual std::string Save();
        virtual bool Load(std::string value);

    private:
        std::set<uint32> list;
    };
}
