 #pragma once


#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
 
 class MoveToCircleFormationAction : public MovementAction, public Qualified
    {
    public:
        MoveToCircleFormationAction(PlayerbotAI* ai) : MovementAction(ai, "move to circle formation") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
        virtual bool isPossible();
    private:
        bool ParseQualifierParameters(float& outRadius, float& outAngleDegrees, Player* requester_for_messaging);
        WorldPosition CalculateFormationSpot(float radius, float angleDegrees, Unit* circleCenterTarget);
    };