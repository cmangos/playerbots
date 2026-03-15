#include "playerbot/playerbot.h"
#include "GuildMeetingTrigger.h"
#include "Guilds/GuildMgr.h"
#include <regex>

using namespace ai;

static bool ParseTimeTokenToHM(const std::string& token, int& outHour, int& outMin)
{
    // Parse Guild MOTD
    // Format 24h: Meeting: <location> <start HH:MM> <end HH:MM>
    // Format 12h: Meeting: <location> <start HH:MM AM/PM> <end HH:MM AM/PM>
    std::smatch m;
    std::regex re(R"((\d{1,2}):(\d{2})([AaPp][Mm])?)");
    if (!std::regex_match(token, m, re))
        return false;

    int hour = std::stoi(m[1].str());
    int minute = std::stoi(m[2].str());
    bool hasAmPm = m[3].matched;
    std::string ampm;
    if (hasAmPm)
        ampm = m[3].str();

    if (minute < 0 || minute > 59 || hour < 0 || hour > 23)
        return false;

    if (hasAmPm)
    {
        std::string up = ampm;
        for (auto &c : up) c = toupper((unsigned char)c);
        if (up == "AM")
        {
            if (hour == 12) hour = 0;
        }
        else if (up == "PM")
        {
            if (hour != 12) hour = (hour % 12) + 12;
        }
    }

    outHour = hour;
    outMin = minute;
    return true;
}

bool GuildMeetingTrigger::ParseMotd(const std::string& motd, std::string& outLocation, time_t& outStartTime, time_t& outEndTime) const
{
    if (motd.empty())
        return false;

    std::string prefix = "Meeting:";
    auto pos = motd.find(prefix);
    if (pos == std::string::npos)
        return false;

    std::string body = motd.substr(pos + prefix.size());

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

    std::string endToken = tokens.back(); tokens.pop_back();
    std::string startToken = tokens.back(); tokens.pop_back();

    int sh = 0, sm = 0, eh = 0, em = 0;
    if (!ParseTimeTokenToHM(startToken, sh, sm))
        return false;
    if (!ParseTimeTokenToHM(endToken, eh, em))
        return false;

    std::ostringstream loc;
    for (size_t i = 0; i < tokens.size(); ++i)
    {
        if (i) loc << " ";
        loc << tokens[i];
    }
    outLocation = loc.str();
    if (outLocation.empty())
        return false;

    time_t now = time(nullptr);
    tm startLocal = *(localtime(&now));
    tm endLocal = startLocal;

    startLocal.tm_hour = sh;
    startLocal.tm_min = sm;
    startLocal.tm_sec = 0;
    endLocal.tm_hour = eh;
    endLocal.tm_min = em;
    endLocal.tm_sec = 0;

    time_t start = mktime(&startLocal);
    time_t end = mktime(&endLocal);

    if (start == (time_t)-1 || end == (time_t)-1)
        return false;

    time_t twelveHours = 12 * 3600;
    if (start + twelveHours < now)
        start += 24 * 3600;
    if (end + twelveHours < now)
        end += 24 * 3600;

    if (end < start)
        end += 24 * 3600;

    outStartTime = start;
    outEndTime = end;
    return true;
}

bool GuildMeetingTrigger::IsActive()
{
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    std::string motd = guild->GetMOTD();
    std::string location;
    time_t startTime = 0;
    time_t endTime = 0;
    if (!ParseMotd(motd, location, startTime, endTime))
        return false;

    time_t now = time(nullptr);
    const time_t startWindow = startTime - 30 * 60;
    const time_t endWindow = endTime;

    if (now >= startWindow && now <= endWindow)
    {
        this->param = location;
        return true;
    }

    return false;
}