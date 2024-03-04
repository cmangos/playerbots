#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"

namespace ai
{
    class ReachAreaTriggerAction : public MovementAction {
    public:
        ReachAreaTriggerAction(PlayerbotAI* ai) : MovementAction(ai, "reach area trigger") {}

        virtual bool Execute(Event& event);
    };

    class AreaTriggerAction : public MovementAction {
    public:
        AreaTriggerAction(PlayerbotAI* ai) : MovementAction(ai, "area trigger") {}

        virtual bool Execute(Event& event);
    };

}