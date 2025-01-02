#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    typedef std::set<uint32> focusQuestTravelList;

    class FocusTravelTargetValue : public ManualSetValue<focusQuestTravelList>
	{
	public:
        FocusTravelTargetValue(PlayerbotAI* ai, focusQuestTravelList defaultValue = {}, std::string name = "forced travel target") : ManualSetValue<focusQuestTravelList>(ai, defaultValue, name) {};
    };
}
