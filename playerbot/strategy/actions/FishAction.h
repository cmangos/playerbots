#pragma once
#include "CastCustomSpellAction.h"
#include "UseItemAction.h"


namespace ai
{
    class MoveToFishAction : public MovementAction, public Qualified
    {
    public:
        MoveToFishAction(PlayerbotAI* ai) : MovementAction(ai, "move to fish"), Qualified() {}
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;
    };

    class FishAction : public CastCustomSpellAction
    {
    public:
        FishAction(PlayerbotAI* ai) : CastCustomSpellAction(ai, "fish") {}
        virtual bool isUseful() override;
        virtual bool Execute(Event& event) override;
    };

    class UseFishingBobberAction : public UseAction
    {
    public:
        UseFishingBobberAction(PlayerbotAI* ai) : UseAction(ai, "use fishing bobber") {}

        bool Execute(Event& event) override;
    };
}
