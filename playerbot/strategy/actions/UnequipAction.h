#pragma once
#include "GenericActions.h"

namespace ai
{
    class UnequipAction : public ChatCommandAction
    {
    public:
        UnequipAction(PlayerbotAI* ai) : ChatCommandAction(ai, "unequip") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

         static void UnequipItem(PlayerbotAI* ai, Player* requester, Item* item, bool silent = false);
    private:
        void UnequipItem(Player* requester, FindItemVisitor* visitor);
    };
}