#pragma once
#include "GenericActions.h"

namespace ai
{
    class TellLosAction : public ChatCommandAction
    {
    public:
        TellLosAction(PlayerbotAI* ai) : ChatCommandAction(ai, "los") {}
        virtual bool Execute(Event& event) override;

    private:
        void ListUnits(Player* requester, std::string title, std::list<ObjectGuid> units);
        void ListGameObjects(Player* requester, std::string title, std::list<ObjectGuid> gos);
    };
}
