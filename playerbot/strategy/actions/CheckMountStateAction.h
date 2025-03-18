#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"
#include "UseItemAction.h"

namespace ai
{
    class CheckMountStateAction : public UseAction 
    {
    public:
        CheckMountStateAction(PlayerbotAI* ai) : UseAction(ai, "check mount state") {}

        virtual bool Execute(Event& event) override;
        virtual bool isPossible() { return true; }
        virtual bool isUseful();

    private:
        virtual bool CanFly() const;
        bool CanMountInBg() const;
        float GetAttackDistance() const;

    private:
        bool Mount(Player* requester, bool limitSpeedToGroup = false);
        bool UnMount() const;
    };
}
