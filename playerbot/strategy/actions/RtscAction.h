#pragma once

#include "SeeSpellAction.h"
#include "playerbot/strategy/values/RTSCValues.h"

namespace ai
{    
    #define RTSC_MOVE_SPELL 30758 //Aedm (Awesome Energetic do move)

    class RTSCAction : public SeeSpellAction
    {
    public:
        RTSCAction(PlayerbotAI* ai) : SeeSpellAction(ai, "rtsc") {}
        virtual bool Execute(Event& event);
    };
}
