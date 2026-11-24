#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/TravelNode.h"

namespace ai
{
    enum class TransportRideState : uint8
    {
        NONE = 0,
        RIDE = 1
    };

    struct TransportRide
    {
        TransportRideState state = TransportRideState::NONE;
        uint32 entry = 0;
        ObjectGuid guid;
        uint32 startMs = 0;
        uint32 budgetMs = 0;
        WorldPosition boardStop;
        WorldPosition exitStop;
        WorldPosition exitDock;
        uint32 nextWanderMs = 0;
        uint32 routeLostMs = 0;

        bool active() const { return state == TransportRideState::RIDE; }

        void clear()
        {
            state = TransportRideState::NONE;
            entry = 0;
            guid = ObjectGuid();
            startMs = 0;
            budgetMs = 0;
            boardStop = WorldPosition();
            exitStop = WorldPosition();
            exitDock = WorldPosition();
            nextWanderMs = 0;
            routeLostMs = 0;
        }
    };

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
            boardStop = other.boardStop;
            boardEntry = other.boardEntry;
            boardDeadlineMs = other.boardDeadlineMs;
            boardWaitMs = other.boardWaitMs;
            boardSpot = other.boardSpot;
            boardSpotStop = other.boardSpotStop;
            ride = other.ride;
            lastPath = other.lastPath;
            lastMoveShort = other.lastMoveShort;
            nextTeleport = other.nextTeleport;
            moveEvent = Event();
        }

        void clear()
        {
            lastPath.clear();
            lastFollow = NULL;
            lastAreaTrigger = 0;
            lastTransportEntry = 0;
            boardStop = WorldPosition();
            boardEntry = 0;
            boardWaitMs = 0;
            boardDeadlineMs = 0;
            boardSpot = WorldPosition();
            boardSpotStop = WorldPosition();
            ride.clear();
            lastFlee = 0;
            lastMoveShort = WorldPosition();
            lastFleeTo = WorldPosition();
            nextTeleport = 0;
            moveEvent = Event();
        }

        void Set(Unit* lastFollow)
        {
            setPath(TravelPath());
            this->lastFollow = lastFollow;
        }

        void setPath(TravelPath path) { lastPath = path; }
    public:
        std::vector<uint32> taxiNodes;
        ObjectGuid taxiMaster;
        Unit* lastFollow;
        uint32 lastAreaTrigger;
        uint32 lastTransportEntry;
        WorldPosition boardStop;
        uint32 boardEntry;
        uint32 boardWaitMs;
        uint32 boardDeadlineMs;
        WorldPosition boardSpot;
        WorldPosition boardSpotStop;
        TransportRide ride;
        time_t lastFlee;
        WorldPosition lastFleeTo;
        TravelPath lastPath;
        WorldPosition lastMoveShort;
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
