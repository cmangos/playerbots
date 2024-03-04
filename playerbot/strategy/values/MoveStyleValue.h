#pragma once
#include "playerbot/LootObjectStack.h"
#include "../Value.h"
#include "SubStrategyValue.h"

namespace ai
{
    class MoveStyleValue : public SubStrategyValue
	{
	public:
        MoveStyleValue(PlayerbotAI* ai, const string& defaultValue = "", const string& name = "move style", const string& allowedValues = "wait,noedge") : SubStrategyValue(ai, defaultValue, name, allowedValues) {}

        static bool WaitForEnemy(PlayerbotAI* ai) { return HasValue(ai, "wait"); }
        static bool CheckForEdges(PlayerbotAI* ai) { return HasValue(ai, "noedge"); }
        
	private:
        static bool HasValue(PlayerbotAI* ai, const string& value);
    };
}
