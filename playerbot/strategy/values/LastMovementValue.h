#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/TravelNode.h"

namespace ai
{
    class LastMovement
    {
    public:
        LastMovement()
        {
            clear();
        }

        LastMovement(LastMovement& other)
        {
            taxiNodes = other.taxiNodes;
            taxiMaster = other.taxiMaster;
            lastFollow = other.lastFollow;
            lastAreaTrigger = other.lastAreaTrigger;
            lastTransportEntry = other.lastTransportEntry;
            lastMoveShort = other.lastMoveShort;
            lastMoveShortStart = other.lastMoveShortStart;
            lastPath = other.lastPath;
            nextTeleport = other.nextTeleport;
            moveEvent = Event();
        }

        void clear()
        {
            lastMoveShort = WorldPosition();
            lastMoveShortStart = WorldPosition();
            lastPath.clear();
            lastFollow = NULL;
            lastAreaTrigger = 0;
            lastTransportEntry = 0;
            lastFlee = 0;
            nextTeleport = 0;
            moveEvent = Event();
        }

        void Set(Unit* lastFollow)
        {
            setShort(WorldPosition(),WorldPosition());
            setPath(TravelPath());
            this->lastFollow = lastFollow;
        }

        void setShort(WorldPosition start, WorldPosition end)
        {
            lastMoveShortStart = start;
            lastMoveShort = end;
            lastFollow = NULL;
        }
        void setPath(TravelPath path) { lastPath = path; }
    public:
        std::vector<uint32> taxiNodes;
        ObjectGuid taxiMaster;
        Unit* lastFollow;
        uint32 lastAreaTrigger;
        uint32 lastTransportEntry;
        time_t lastFlee;
        WorldPosition lastMoveShortStart;
        WorldPosition lastMoveShort;
        TravelPath lastPath;
        time_t nextTeleport;
        Event moveEvent;
    };

    class LastMovementValue : public ManualSetValue<LastMovement&>
    {
    public:
        LastMovementValue(PlayerbotAI* ai) : ManualSetValue<LastMovement&>(ai, data) {}
    private:
        LastMovement data = LastMovement();
    };

    class StayTimeValue : public ManualSetValue<time_t>
    {
    public:
        StayTimeValue(PlayerbotAI* ai) : ManualSetValue<time_t>(ai, 0) {}
    };

    class LastLongMoveValue : public CalculatedValue<WorldPosition>
    {
    public:
        LastLongMoveValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "last long move", 30) {}

        WorldPosition Calculate() override;
    };


    class HomeBindValue : public CalculatedValue<WorldPosition>
    {
    public:
        HomeBindValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "home bind", 30) {}

        WorldPosition Calculate() override;

        virtual std::string Format() override;
    };
}
