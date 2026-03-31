#pragma once
#include "playerbot/strategy/Value.h"
#include "PartyMemberValue.h"

namespace ai
{
    class PartyMemberToSoulstone : public PartyMemberValue
    {
    public:
        PartyMemberToSoulstone(PlayerbotAI* ai, std::string name = "party member to soulstone") : PartyMemberValue(ai,name) {}

    protected:
        virtual Unit* Calculate() override;
    };
}
