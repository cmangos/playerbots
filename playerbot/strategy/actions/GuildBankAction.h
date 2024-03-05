#pragma once
#include "GenericActions.h"

namespace ai
{
    class GuildBankAction : public ChatCommandAction
    {
    public:
        GuildBankAction(PlayerbotAI* ai) : ChatCommandAction(ai, "guild bank") {}
        virtual bool Execute(Event& event) override;

    private:
        bool Execute(std::string text, GameObject* bank, Player* requester);
        bool MoveFromCharToBank(Item* item, GameObject* bank, Player* requester);
    };
}
