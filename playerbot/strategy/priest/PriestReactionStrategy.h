#pragma once
#include "playerbot/strategy/generic/ReactionStrategy.h"

namespace ai
{
    class PriestReactionStrategy : public ReactionStrategy
    {
    public:
        PriestReactionStrategy(PlayerbotAI* ai) : ReactionStrategy(ai) {}
        std::string getName() override { return "react"; }

    private:
        void InitReactionTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
