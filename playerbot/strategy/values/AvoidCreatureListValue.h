#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class AvoidCreatureListValue : public ManualSetValue<std::set<uint32>&>
	{
	public:
        AvoidCreatureListValue(PlayerbotAI* ai, std::string name = "avoid creature list") : ManualSetValue<std::set<uint32>&>(ai, list, name) {}

        virtual std::string Save() override;
        virtual bool Load(std::string value) override;

    private:
        std::set<uint32> list;
    };
}