#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/values/GuildValues.h"

namespace ai
{
    class GuildCraftOrderAction : public Action
    {
    public:
        GuildCraftOrderAction(PlayerbotAI* ai) : Action(ai, "guild craft order") {}

        bool Execute(Event& event) override;
        bool isUseful() override;

    private:
        uint32 FindCraftSpell(uint32 itemId);
    };
}