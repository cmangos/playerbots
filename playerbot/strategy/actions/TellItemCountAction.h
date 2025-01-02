#pragma once
#include "GenericActions.h"

namespace ai
{
    class TellItemCountAction : public ChatCommandAction
    {
    public:
        TellItemCountAction(PlayerbotAI* ai) : ChatCommandAction(ai, "c") {}
        virtual bool Execute(Event& event) override;

        bool isUsefulWhenStunned() override { return true; }
    };
}