#pragma once
#include "playerbot/playerbot.h"
#include "playerbot/WorldPosition.h"

#include <deque>
#include <mutex>
#include <string>
#include <unordered_map>

class Player;
class GenericTransport;

namespace ai
{
    class TransportTrace
    {
    public:
        static TransportTrace& instance();

        bool Enabled(uint32 level = 1) const;

        void Event(Player* bot, GenericTransport* transport, const std::string& state, const std::string& event,
            const std::string& reason = "", const WorldPosition& dock = WorldPosition(), const WorldPosition& exit = WorldPosition());

        void Sample(Player* bot, GenericTransport* transport, const WorldPosition& dock = WorldPosition(), const WorldPosition& exit = WorldPosition());

        void RideEnded(Player* bot);

        std::string Describe(Player* bot);

    private:
        TransportTrace() {}

        struct RideRecord
        {
            std::string state;
            uint32 rideStart = 0;
            uint32 stateStart = 0;
            uint32 lastKeyMs = 0;
            std::string lastKey;
            uint32 lastSampleMs = 0;
            uint32 lastStateEnterMs = 0;
            uint32 mapId = 0;
            uint32 transportEntry = 0;
            uint32 transportGuid = 0;
            uint32 deckPicks = 0;
            uint32 ejects = 0;
            uint32 boardAttempts = 0;
            uint32 aborts = 0;
            uint32 lastTransportMs = 0;
            WorldPosition lastTransportPos;
            uint32 lastTransportMapId = 0;
            float lastSpeed = 0.0f;
            std::deque<std::string> history;
        };

        void Write(Player* bot, GenericTransport* transport, RideRecord& record, const std::string& state, const std::string& event,
            const std::string& reason, const WorldPosition& dock, const WorldPosition& exit, bool sampled);

        void CountEvent(RideRecord& record, const std::string& event);

        std::mutex m_traceMutex;
        std::unordered_map<uint32, RideRecord> m_records;
    };
}
