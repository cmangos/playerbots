#pragma once
#include "playerbot/LootObjectStack.h"
#include "GenericActions.h"

namespace ai
{
    class LootStrategyAction : public ChatCommandAction
    {
    public:
        LootStrategyAction(PlayerbotAI* ai) : ChatCommandAction(ai, "ll") {}
        virtual bool Execute(Event& event) override;

    private:
        void TellLootList(Player* requester, const std::string& name);
        void TellGoList(Player* requester, const std::string& name);
    };
}
