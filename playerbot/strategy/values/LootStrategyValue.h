#pragma once
#include "playerbot/LootObjectStack.h"
#include "../Value.h"
#include "SubStrategyValue.h"

namespace ai
{
    class LootStrategyValue : public SubStrategyValue
	{
	public:
        LootStrategyValue(PlayerbotAI* ai, string defaultValue = "equip,quest,skill,disenchant,use,vendor", string name = "loot strategy", string allowedValues = "equip,quest,skill,disenchant,use,vendor,trash") : SubStrategyValue(ai, defaultValue, name, allowedValues) {}

        virtual void Set(string newValue) override;

        static bool CanLoot(ItemQualifier& itemQualifier, PlayerbotAI* ai);
    };
}
