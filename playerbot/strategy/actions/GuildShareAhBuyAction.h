
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/values/GuildValues.h"

namespace ai
{
    class GuildShareAhBuyAction : public Action
    {
    public:
        GuildShareAhBuyAction(PlayerbotAI* ai) : Action(ai, "guild ah buy") {}

        bool Execute(Event& event) override;
        bool isUseful() override;

    private:
        Unit* FindNearbyAuctioneer();
        std::map<uint32, uint32> GetNeededItems();
        bool CanBotCraftItem(uint32 itemId);
        bool CanBotUseReagent(ItemPrototype const* reagentProto);
        uint32 CountMailboxItems(uint32 itemId);
    };
}