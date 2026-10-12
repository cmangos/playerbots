#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class BuyFailedAction : public Action {
    public:
        BuyFailedAction(PlayerbotAI* ai) : Action(ai, "buy failed") {}
        virtual bool Execute(Event& event) override;

    private:
        static std::map<BuyResult, std::string> messages;
    };
}