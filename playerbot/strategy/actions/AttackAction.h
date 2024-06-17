#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"

namespace ai
{
	class AttackAction : public MovementAction
	{
	public:
		AttackAction(PlayerbotAI* ai, std::string name) : MovementAction(ai, name) {}

    public:
        virtual bool Execute(Event& event) override;
        virtual bool isPossible() override { return !bot->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_CLIENT_CONTROL_LOST); }; //Override movement stay.

    protected:
        bool Attack(Player* requester, Unit* target);
        bool IsTargetValid(Player* requester, Unit* target);
    };

    class AttackMyTargetAction : public AttackAction
    {
    public:
        AttackMyTargetAction(PlayerbotAI* ai, std::string name = "attack my target") : AttackAction(ai, name) { MakeVerbose(true); }

    public:
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class AttackRTITargetAction : public AttackAction
    {
    public:
        AttackRTITargetAction(PlayerbotAI* ai, std::string name = "attack rti target") : AttackAction(ai, name) { MakeVerbose(true); }

    public:
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class AttackDuelOpponentAction : public AttackAction
    {
    public:
        AttackDuelOpponentAction(PlayerbotAI* ai, std::string name = "attack duel opponent") : AttackAction(ai, name) {}

    public:
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };
}
