#pragma once
#include "MovementActions.h"
#include "GenericActions.h"

namespace ai
{
    class PositionAction : public ChatCommandAction
    {
    public:
        PositionAction(PlayerbotAI* ai) : ChatCommandAction(ai, "position") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class MoveToPositionAction : public MovementAction
    {
    public:
        MoveToPositionAction(PlayerbotAI* ai, std::string name, std::string qualifier, bool idle = false) : MovementAction(ai, name), qualifier(qualifier), idle(idle) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;

    protected:
        std::string qualifier;
        bool idle;
    };

    class GuardAction : public MoveToPositionAction
    {
    public:
        GuardAction(PlayerbotAI* ai) : MoveToPositionAction(ai, "move to position", "guard") {}
        virtual bool isUseful() override;
    };

    class SetReturnPositionAction : public Action
    {
    public:
        SetReturnPositionAction(PlayerbotAI* ai) : Action(ai, "set return position") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class ReturnAction : public MoveToPositionAction
    {
    public:
        ReturnAction(PlayerbotAI* ai) : MoveToPositionAction(ai, "return", "return", true) {}
        virtual bool isUseful() override;
    };

    class ReturnToStayPositionAction : public MoveToPositionAction
    {
    public:
        ReturnToStayPositionAction(PlayerbotAI* ai) : MoveToPositionAction(ai, "move to position", "stay") {}
        virtual bool isPossible() override;
    };

    class ReturnToPullPositionAction : public MoveToPositionAction
    {
    public:
        ReturnToPullPositionAction(PlayerbotAI* ai) : MoveToPositionAction(ai, "return to pull position", "pull") {}
        virtual bool isPossible() override;
    };
}
