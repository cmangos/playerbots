#pragma once
#include "GenericActions.h"

namespace ai
{
    class BankAction : public ChatCommandAction
    {
    public:
        BankAction(PlayerbotAI* ai) : ChatCommandAction(ai, "bank") {}
        virtual bool Execute(Event& event) override;

    private:
        bool ExecuteCommand(Player* requester, const std::string& text, Unit* bank);
        void ListItems(Player* requester);
        bool Withdraw(Player* requester, const uint32 itemid);
        bool Deposit(Player* requester, Item* pItem);
        Item* FindItemInBank(uint32 ItemId);
    };
}
