#include "playerbot/playerbot.h"
#include "playerbot/TransportTrace.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/TravelNode.h"
#include "Entities/Transports.h"
#include "Util/Timer.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace ai
{
    static const char* const TRANSPORT_TRACE_FILE = "bot_transport.csv";

    static std::string Sanitize(std::string text)
    {
        std::replace(text.begin(), text.end(), ',', ';');
        std::replace(text.begin(), text.end(), '\n', ' ');
        std::replace(text.begin(), text.end(), '\r', ' ');
        return text;
    }

    TransportTrace& TransportTrace::instance()
    {
        static TransportTrace trace;
        return trace;
    }

    bool TransportTrace::Enabled(uint32 level) const
    {
        return sPlayerbotAIConfig.transportTrace >= level && sPlayerbotAIConfig.hasLog(TRANSPORT_TRACE_FILE);
    }

    void TransportTrace::CountEvent(RideRecord& record, const std::string& event)
    {
        if (event == "pick_deck_spot")
            record.deckPicks++;
        else if (event == "board_tp" || event == "board_walk")
        {
            record.boardAttempts++;

            if (!record.rideStart)
                record.rideStart = WorldTimer::getMSTime();
        }
        else if (event == "eject_guard")
            record.ejects++;
        else if (event == "abort")
            record.aborts++;
    }

    void TransportTrace::Write(Player* bot, GenericTransport* transport, RideRecord& record, const std::string& state,
        const std::string& event, const std::string& reason, const WorldPosition& dock, const WorldPosition& exit, bool sampled)
    {
        WorldPosition botPos(bot);
        WorldPosition transportPos = transport ? WorldPosition(transport) : WorldPosition();
        WorldPosition localPos = transport ? botPos.relPoint(transportPos) : WorldPosition();

        uint32 now = WorldTimer::getMSTime();

        if (transport && record.lastTransportMs && record.lastTransportMapId == transport->GetMapId())
        {
            uint32 elapsed = WorldTimer::getMSTimeDiff(record.lastTransportMs, now);

            if (elapsed >= 100)
            {
                float speed = (float)std::sqrt(transportPos.sqDistance(record.lastTransportPos)) * 1000.0f / (float)elapsed;

                if (speed <= 40.0f)
                    record.lastSpeed = speed;

                record.lastTransportPos = transportPos;
                record.lastTransportMs = now;
            }
        }
        else if (transport)
        {
            record.lastTransportPos = transportPos;
            record.lastTransportMs = now;
        }

        if (transport)
            record.lastTransportMapId = transport->GetMapId();

        if (transport)
        {
            record.transportEntry = transport->GetEntry();
            record.transportGuid = transport->GetGUIDLow();
        }

        bool isPassenger = transport && transport->GetPassengers().count(bot) > 0;
        bool hasTransport = transport && bot->GetTransport() == transport;
        int geomOnTrans = -1;

        if (transport && bot->GetMapId() == transport->GetMapId())
            geomOnTrans = WorldPosition(bot).isOnTransport(transport) ? 1 : 0;

        float distToDock = -1.0f;

        if (dock && dock.getMapId() == bot->GetMapId())
            distToDock = std::sqrt(dock.sqDistance(botPos));

        float distToExit = -1.0f;

        if (exit && exit.getMapId() == bot->GetMapId())
            distToExit = std::sqrt(exit.sqDistance(botPos));

        std::ostringstream out;
        out << std::fixed << std::setprecision(2);
        out << sPlayerbotAIConfig.GetTimestampStr() << ",";
        out << time(nullptr) << ",";
        out << Sanitize(bot->GetName()) << ",";
        out << bot->GetGUIDLow() << ",";
        out << state << ",";
        out << event << ",";
        out << Sanitize(reason) << ",";
        out << (bot->GetPlayerbotAI() ? ai::TransportMode(bot->GetPlayerbotAI()) : sPlayerbotAIConfig.transportTeleportType) << ",";
        out << (transport ? transport->GetEntry() : 0) << ",";
        out << (transport ? transport->GetGUIDLow() : 0) << ",";
        out << (transport ? transport->GetMapId() : 0) << ",";
        out << bot->GetMapId() << ",";
        out << botPos.getX() << "," << botPos.getY() << "," << botPos.getZ() << ",";
        out << localPos.getX() << "," << localPos.getY() << "," << localPos.getZ() << ",";
        out << transportPos.getX() << "," << transportPos.getY() << "," << transportPos.getZ() << ",";
        out << record.lastSpeed << ",";
        out << (transport ? transport->GetPathProgress() : 0) << ",";
        out << (isPassenger ? 1 : 0) << ",";
        out << (hasTransport ? 1 : 0) << ",";
        out << geomOnTrans << ",";
        out << distToDock << ",";
        out << distToExit;

        sPlayerbotAIConfig.logRaw(TRANSPORT_TRACE_FILE, out.str());

        std::ostringstream line;
        line << std::fixed << std::setprecision(1);
        line << sPlayerbotAIConfig.GetTimestampStr().substr(11) << " " << state << "/" << event;

        if (!reason.empty())
            line << " " << reason;

        if (sampled)
            line << " sample";

        line << " dist=" << distToDock << " pass=" << (isPassenger ? 1 : 0) << " geom=" << geomOnTrans;

        record.history.push_back(line.str());

        while (record.history.size() > 10)
            record.history.pop_front();
    }

    void TransportTrace::Event(Player* bot, GenericTransport* transport, const std::string& state, const std::string& event,
        const std::string& reason, const WorldPosition& dock, const WorldPosition& exit)
    {
        if (!bot || !Enabled(1))
            return;

        std::lock_guard<std::mutex> guard(m_traceMutex);

        RideRecord& record = m_records[bot->GetGUIDLow()];

        if (!transport)
            transport = bot->GetTransport();

        uint32 now = WorldTimer::getMSTime();

        if (record.mapId && record.mapId != bot->GetMapId())
        {
            std::string mapReason = "from_" + std::to_string(record.mapId);
            record.mapId = bot->GetMapId();
            Write(bot, transport, record, "map_change", "map_change", mapReason, dock, exit, false);
        }

        record.mapId = bot->GetMapId();

        if (record.state != state && (!record.lastStateEnterMs || WorldTimer::getMSTimeDiff(record.lastStateEnterMs, now) >= 1000))
        {
            std::string fromState = record.state.empty() ? "none" : record.state;
            record.state = state;
            record.stateStart = now;
            record.lastStateEnterMs = now;
            Write(bot, transport, record, state, "state_enter", "from_" + fromState, dock, exit, false);
        }

        if (event != "state_enter" && event != "sample")
        {
            std::string key = state + "|" + event + "|" + reason;

            if (key == record.lastKey && WorldTimer::getMSTimeDiff(record.lastKeyMs, now) < 1000)
                return;

            record.lastKey = key;
            record.lastKeyMs = now;
        }

        CountEvent(record, event);
        Write(bot, transport, record, state, event, reason, dock, exit, false);
    }

    void TransportTrace::Sample(Player* bot, GenericTransport* transport, const WorldPosition& dock, const WorldPosition& exit)
    {
        if (!bot || !transport || !Enabled(2))
            return;

        uint32 now = WorldTimer::getMSTime();

        {
            std::lock_guard<std::mutex> guard(m_traceMutex);

            RideRecord& record = m_records[bot->GetGUIDLow()];

            if (record.lastSampleMs && WorldTimer::getMSTimeDiff(record.lastSampleMs, now) < 500)
                return;

            record.lastSampleMs = now;

            if (record.state != "ride" || record.mapId != bot->GetMapId())
                return;

            Write(bot, transport, record, "ride", "sample", "", dock, exit, true);
        }
    }

    void TransportTrace::RideEnded(Player* bot)
    {
        if (!bot)
            return;

        std::lock_guard<std::mutex> guard(m_traceMutex);

        auto it = m_records.find(bot->GetGUIDLow());

        if (it == m_records.end())
            return;

        it->second.state = "none";
        it->second.stateStart = WorldTimer::getMSTime();
        it->second.rideStart = 0;
        it->second.lastSampleMs = 0;
        it->second.lastTransportMs = 0;
        it->second.lastSpeed = 0.0f;
    }

    std::string TransportTrace::Describe(Player* bot)
    {
        if (!bot)
            return "no bot";

        std::lock_guard<std::mutex> guard(m_traceMutex);

        auto it = m_records.find(bot->GetGUIDLow());

        if (it == m_records.end())
            return "no transport state recorded (trace " + std::string(Enabled(1) ? "on" : "off") + ")";

        RideRecord& record = it->second;
        uint32 now = WorldTimer::getMSTime();

        std::ostringstream out;
        out << "trace " << (Enabled(1) ? "on" : "off") << " level " << sPlayerbotAIConfig.transportTrace;
        out << " state:" << (record.state.empty() ? "none" : record.state);

        if (record.stateStart)
            out << " " << (WorldTimer::getMSTimeDiff(record.stateStart, now) / 1000) << "s";

        out << " transport:" << record.transportEntry << "/" << record.transportGuid;
        out << " boards:" << record.boardAttempts << " deckPicks:" << record.deckPicks;
        out << " ejects:" << record.ejects << " aborts:" << record.aborts;
        out << " tSpeed:" << std::setprecision(1) << record.lastSpeed;

        for (auto& line : record.history)
            out << "\n" << line;

        return out.str();
    }
}
