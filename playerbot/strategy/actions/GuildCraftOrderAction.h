#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/values/CraftValues.h"

namespace ai
{
    class GuildCraftOrderAction : public Action
    {
    public:
        GuildCraftOrderAction(PlayerbotAI* ai) : Action(ai, "guild craft order") {}

        bool Execute(Event& event) override;
        bool isUseful() override;

    private:
        struct CraftOrder
        {
            std::string itemName;
            uint32 amount = 0;
        };

        static CraftOrder ParseCraftOrder(Player* bot);
        static uint32 FindItemByName(const std::string& name);
        uint32 FindCraftSpell(uint32 itemId);
    };
}