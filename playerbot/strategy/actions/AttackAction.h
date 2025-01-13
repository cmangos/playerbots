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

        bool isUseful() override
        {
            if (Value<Unit*>* vunit = context->GetValue<Unit*>(GetTargetName()))
            {   
                if (Unit* unit = vunit->Get())
                {
                    float distsq = unit->GetPosition().GetDistance2d(bot->GetPosition());
                    if (distsq <= 5.0f * 5.0f)
                    {
                        return true;
                    }
                }
            }

            return MovementAction::isUseful();
        }

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
