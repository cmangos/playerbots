
#include "playerbot/playerbot.h"
#include "TravelToGuildMeetingAction.h"
#include "Guilds/GuildMgr.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "ChooseTravelTargetAction.h"

using namespace ai;

bool TravelToGuildMeetingAction::ParseMotdLocation(const std::string& motd, std::string& outLocation) const
{
    // Parse Guild MOTD
    // Format 24h: Meeting: <location> <start HH:MM> <end HH:MM>
    // Format 12h: Meeting: <location> <start HH:MM AM/PM> <end HH:MM AM/PM>
    if (motd.empty())
        return false;

    std::string prefix = "Meeting:";
    auto pos = motd.find(prefix);
    if (pos == std::string::npos)
        return false;

    std::string body = motd.substr(pos + prefix.size());
    // trim
    auto ltrim = [](std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    };
    auto rtrim = [](std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
    };
    ltrim(body);
    rtrim(body);
    if (body.empty())
        return false;

    std::vector<std::string> tokens;
    {
        std::istringstream iss(body);
        std::string t;
        while (iss >> t) tokens.push_back(t);
    }
    if (tokens.size() < 3)
        return false;

    tokens.pop_back();
    tokens.pop_back();

    std::ostringstream loc;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (i) loc << " ";
        loc << tokens[i];
    }
    outLocation = loc.str();
    return !outLocation.empty();
}

bool TravelToGuildMeetingAction::isUseful()
{
    if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT))
    {
        ai->TellDebug(ai->GetMaster(), "TravelToGuildMeetingAction: travel strategy disabled", "debug travel");
        return false;
    }

    if (!ai->AllowActivity(TRAVEL_ACTIVITY))
    {
        ai->TellDebug(ai->GetMaster(), "TravelToGuildMeetingAction: TRAVEL_ACTIVITY not allowed", "debug travel");
        return false;
    }

    if (!bot->GetGuildId())
    {
        ai->TellDebug(ai->GetMaster(), "TravelToGuildMeetingAction: bot has no guild", "debug travel");
        return false;
    }

    return true;
}

bool TravelToGuildMeetingAction::Execute(Event& event)
{
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    std::string motd = guild->GetMOTD();
    std::string location;
    if (!ParseMotdLocation(motd, location))
        return false;

    if (location.empty())
        return false;

    Player* requester = event.getOwner() ? event.getOwner() : (GetMaster() ? GetMaster() : bot);
    PlayerTravelInfo travelInfo(bot);
    WorldPosition center = requester ? requester : bot;

    DestinationList matches = ChooseTravelTargetAction::FindDestination(travelInfo, location, true, true, true, false, false);

    WorldPosition* bestPoint = nullptr;
    TravelDestination* bestDest = nullptr;
    float bestDist = std::numeric_limits<float>::max();

    if (!matches.empty())
    {
        std::list<uint8> chancesToGoFar = { 10,50,90 };
        for (auto& dest : matches)
        {
            WorldPosition* p = dest->GetNextPoint(center, chancesToGoFar);
            if (!p) continue;
            float d = p->distance(center);
            if (d < bestDist)
            {
                bestDist = d;
                bestPoint = p;
                bestDest = dest;
            }
            else
            {

            }
        }
    }

    if (bestDest && bestPoint)
    {
        TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

        travelTarget->SetTarget(bestDest, bestPoint);
        travelTarget->SetForced(true);
        travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_READY);

        SET_AI_VALUE2(std::string, "manual string", "future travel purpose", location);
        SET_AI_VALUE2(std::string, "manual string", "future travel condition", "guild meeting");
        SET_AI_VALUE2(int, "manual int", "future travel relevance", 199);

        RESET_AI_VALUE(GuidPosition, "rpg target");
        RESET_AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");
        RESET_AI_VALUE(ObjectGuid, "attack target");
        RESET_AI_VALUE(bool, "travel target active");
        context->ClearValues("no active travel destinations");

        ChooseTravelTargetAction::ReportTravelTarget(bot, requester, travelTarget, travelTarget);

        return true;
    }

    std::string request = "request named travel target::" + location;
    bool result = ai->DoSpecificAction(request, Event("guild meeting"), true);

    return result;
}