#pragma once
#include "GenericActions.h"

namespace ai
{
    class SetFocusHealTargetAction : public ChatCommandAction
    {
    public:
        SetFocusHealTargetAction(PlayerbotAI* ai, std::string name = "focus heal target") : ChatCommandAction(ai, name) {}
        bool Execute(Event& event) override;
    };

    class SetWaitForAttackTimeAction : public ChatCommandAction
    {
    public:
        SetWaitForAttackTimeAction(PlayerbotAI* ai, std::string name = "wait for attack time") : ChatCommandAction(ai, name) {}
        bool Execute(Event& event) override;
    };

    class SetFollowTargetAction : public ChatCommandAction
    {
    public:
        SetFollowTargetAction(PlayerbotAI* ai, std::string name = "follow target") : ChatCommandAction(ai, name) {}
        bool Execute(Event& event) override;
    };
}
