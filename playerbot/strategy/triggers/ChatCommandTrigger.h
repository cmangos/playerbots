#pragma once

#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class ChatCommandTrigger : public Trigger {
    public:
        ChatCommandTrigger(PlayerbotAI* ai, std::string command) : Trigger(ai, command) {}

        virtual void ExternalEvent(std::string param, Player* owner = NULL) override
        {
            this->param = param;
            this->owner = owner;
            triggered = true;
        }

        virtual Event Check() override
        {
            if (!triggered)
                return Event();

            return Event(getName(), param, owner);
        }

        virtual void Reset() override
        {
            triggered = false;
        }
    };
}
