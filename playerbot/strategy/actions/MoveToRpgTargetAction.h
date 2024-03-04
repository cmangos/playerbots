#pragma once

#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "playerbot/strategy/values/LastMovementValue.h"

namespace ai
{
    class MoveToRpgTargetAction : public MovementAction {
    public:
        MoveToRpgTargetAction(PlayerbotAI* ai) : MovementAction(ai, "move to rpg target") {}

        virtual bool Execute(Event& event);
        virtual bool isUseful();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "move to rpg target"; } //Must equal iternal name
        virtual string GetHelpDescription()
        {
            return "This will make the bot move towards the current rpg target.\n"
                "When near the target the bot will move to a spot around the target\n"
                "45 degrees closest to the bot or 45 infront if the target is moving.\n"
                "This action will only execute if the bot is not moving or traveling.";
        }
        virtual vector<string> GetUsedActions() { return {}; }
        virtual vector<string> GetUsedValues() { return { "rpg target" , "ignore rpg target" ,  "travel target" , "can move around" }; }
#endif 
    };

}
