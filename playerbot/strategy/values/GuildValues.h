#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class PetitionSignsValue : public SingleCalculatedValue<uint8>
    {
    public:
        PetitionSignsValue(PlayerbotAI* ai) : SingleCalculatedValue<uint8>(ai, "petition signs") {}

        uint8 Calculate();
    }; 
}
