#pragma once
#include "GenericActions.h"

namespace ai
{
    class CheatAction : public ChatCommandAction
    {
    public:
        CheatAction(PlayerbotAI* ai) : ChatCommandAction(ai, "cheat") {}
        virtual bool Execute(Event& event) override;
    private:
        static BotCheatMask GetCheatMask(std::string cheat);
        static std::string GetCheatName(BotCheatMask cheatMask);
        void ListCheats(Player* requester);
        void AddCheat(BotCheatMask cheatMask);
        void RemCheat(BotCheatMask cheatMask);
    };
}
