#pragma once
#include "playerbot/LootObjectStack.h"
#include "GenericActions.h"

namespace ai
{
    class FlagAction : public ChatCommandAction, public Qualified
    {
    public:
        FlagAction(PlayerbotAI* ai) : ChatCommandAction(ai, "flag") {}
        virtual bool Execute(Event& event) override;
    };
}
