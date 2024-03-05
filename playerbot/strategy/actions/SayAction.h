#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"

namespace ai
{
    class SayAction : public Action, public Qualified
    {
    public:
        SayAction(PlayerbotAI* ai);
        virtual bool Execute(Event& event);
        virtual bool isUseful();
        virtual std::string getName() { return "say::" + qualifier; }

    private:
    };

    class ChatReplyAction : public Action
    {
    public:
        ChatReplyAction(PlayerbotAI* ai) : Action(ai, "chat message") {}
        virtual bool Execute(Event& event) { return true; }
        bool isUseful() { return true; }
        static void ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name);
    };
}
