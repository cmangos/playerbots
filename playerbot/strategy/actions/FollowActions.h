#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"

namespace ai
{
	class FollowAction : public MovementAction {
	public:
		FollowAction(PlayerbotAI* ai, string name = "follow") : MovementAction(ai, name) {}
		virtual bool Execute(Event& event);
        virtual bool isUseful();
        virtual bool CanDeadFollow(Unit* target);
	};

	class StopFollowAction : public MovementAction {
	public:
		StopFollowAction(PlayerbotAI* ai, string name = "stop follow") : MovementAction(ai, name) {}
		virtual bool Execute(Event& event) { ai->StopMoving(); return true; };
	};

    class FleeToMasterAction : public FollowAction {
    public:
        FleeToMasterAction(PlayerbotAI* ai) : FollowAction(ai, "flee to master") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

}
