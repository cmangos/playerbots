
#include "playerbot/playerbot.h"
#include "playerbot/strategy/Action.h"

namespace ai
{
    class TravelToGuildMeetingAction : public Action
    {
    public:
        TravelToGuildMeetingAction(PlayerbotAI* ai, std::string name = "travel to guild meeting") : Action(ai, name) {}

        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override;
    private:
        bool ParseMotdLocation(const std::string& motd, std::string& outLocation) const;
    };
}