#include "playerbot/playerbot.h"
#include "playerbot/strategy/Trigger.h"
#include <string>
#include <ctime>

namespace ai
{
    class GuildMeetingTrigger : public Trigger
    {
    public:
        GuildMeetingTrigger(PlayerbotAI* ai) : Trigger(ai, "guild meeting", 60) {}

        bool IsActive() override;

    private:
        // Parse Guild MOTD
        // Format 24h: Meeting: <location> <start HH:MM> <end HH:MM>
        // Format 12h: Meeting: <location> <start HH:MM AM/PM> <end HH:MM AM/PM>
        bool ParseMotd(const std::string& motd, std::string& outLocation, time_t& outStartTime, time_t& outEndTime) const;
    };
}