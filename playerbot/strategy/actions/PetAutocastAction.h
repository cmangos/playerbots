#pragma once
#include "playerbot/strategy/Action.h"

namespace ai
{
    // Turns autocast on for every pet spell except the ones a bot should never fire on its own
    // (Cower, Prowl, Spell Lock, Devour Magic).
    class PetAutocastAction : public Action
    {
    public:
        PetAutocastAction(PlayerbotAI* ai) : Action(ai, "pet autocast") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    };
}
