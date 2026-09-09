#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class PetTargetValue : public UnitCalculatedValue
	{
	public:
        PetTargetValue(PlayerbotAI* ai, std::string name = "pet target") : UnitCalculatedValue(ai, name) {}

        virtual ObjectGuid Calculate() override 
        {
            Unit* pet = ai->GetBot()->GetPet();
            return pet ? pet->GetObjectGuid() : ObjectGuid();
        }
    };
}
