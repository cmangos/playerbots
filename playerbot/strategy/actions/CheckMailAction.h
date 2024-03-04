#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"

namespace ai
{
    class CheckMailAction : public Action
    {
    public:
        CheckMailAction(PlayerbotAI* ai) : Action(ai, "check mail") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();

    private:
        void ProcessMail(Mail* mail, Player* owner);
    };
}
