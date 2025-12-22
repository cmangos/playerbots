#pragma once
#include "UseItemAction.h"

namespace ai
{
    class UseTrinketAction : public UseAction 
    {
    public:

        UseTrinketAction(PlayerbotAI* ai) : UseAction(ai, "use trinket") {}
        virtual bool Execute(Event& event) override;
        virtual bool isPossible() override;
        virtual bool isUseful() override { return UseAction::isUseful() && !bot->HasStealthAura(); }
    };
}