#pragma once

#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class WorldPacketTrigger : public Trigger {
    public:
        WorldPacketTrigger(PlayerbotAI* ai, std::string command) : Trigger(ai, command), triggered(false) {}

        virtual void ExternalEvent(WorldPacket &packet, Player* owner = NULL) override
        {
            this->packet = packet;
            this->owner = owner;
            triggered = true;
        }

        virtual Event Check() override
        {
            if (!triggered)
                return Event();

            return Event(getName(), packet, owner);
        }

        virtual void Reset() override
        {
            triggered = false;
        }
    private:
        WorldPacket packet;
        bool triggered;
        Player* owner;
    };
}
