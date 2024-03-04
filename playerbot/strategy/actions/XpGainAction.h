#pragma once

#include "playerbot/strategy/Action.h"

namespace ai
{
    class XpGainAction : public Action
    {
    public:
        XpGainAction(PlayerbotAI* ai) : Action(ai, "xp gain") {}
        virtual bool Execute(Event& event);
    private:
        void GiveXP(int32 xp, Unit* victim);
    };
}