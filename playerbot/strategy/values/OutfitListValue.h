#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    typedef std::list<std::string> Outfit;
    class OutfitListValue : public ManualSetValue<Outfit&>
	{
	public:
        OutfitListValue(PlayerbotAI* ai, std::string name = "outfit list") : ManualSetValue<Outfit&>(ai, list, name) {}

        virtual std::string Save() override;
        virtual bool Load(std::string value) override;

    private:
        Outfit list;
    };
}
