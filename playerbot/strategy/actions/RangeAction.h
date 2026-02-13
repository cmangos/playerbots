#pragma once
#include "GenericActions.h"

namespace ai
{
    class RangeAction : public ChatCommandAction
    {
    public:
        RangeAction(PlayerbotAI* ai) : ChatCommandAction(ai, "range") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void PrintRange(std::string type, Player* requester);
    };

    class WanderFarCanMoveAroundAction : public Action
    {
    public:
        WanderFarCanMoveAroundAction(PlayerbotAI* ai) : Action(ai, "wander far can move around") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override { return true; }
    };
}
