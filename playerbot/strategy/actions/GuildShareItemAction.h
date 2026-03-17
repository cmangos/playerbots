#pragma once
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/values/GuildValues.h"

namespace ai
{
    class GuildShareItemAction : public Action
    {
    public:
        GuildShareItemAction(PlayerbotAI* ai) : Action(ai, "guild share item") {}

        bool Execute(Event& event) override;
        bool isUseful() override;
    };
}