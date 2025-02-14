#pragma once
#include "UseItemAction.h"

namespace ai
{
    class UseTrinketAction : public UseAction
    {
    public:
        UseTrinketAction(PlayerbotAI* ai, bool allowPvPTrinket = false)
            : UseAction(ai, "use trinket"), allowPvPTrinket(allowPvPTrinket) {}

        bool Execute(Event& event) override;
        bool isPossible() override;
        bool CanUseTrinket(Item* item);
        bool isUseful() override { return UseAction::isUseful() && !bot->HasStealthAura(); }

    private:
        bool allowPvPTrinket;
        bool IsPvPTrinket(Item* item);
    };
}