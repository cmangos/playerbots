#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class RtiValue : public ManualSetValue<std::string>
	{
	public:
        RtiValue(PlayerbotAI* ai);
        virtual std::string Save() override { return value; }
        virtual bool Load(std::string text) override { value = text; return true; }
    };

    class RtiCcValue : public ManualSetValue<std::string>
    {
    public:
        RtiCcValue(PlayerbotAI* ai);

        virtual std::string Save() override { return value; }
        virtual bool Load(std::string text) override { value = text; return true; }
    };

    class CombatOrderTargetValue : public ManualSetValue<std::string>
    {
    public:
        CombatOrderTargetValue(PlayerbotAI* ai);

        virtual std::string Save() override { return value; }
        virtual bool Load(std::string text) override { value = text; return true; }

        static Player* FindGroupMember(Player* bot, const std::string& name);
        static ObjectGuid FindTarget(PlayerbotAI* ai, bool lowestThreat);
    };
}
