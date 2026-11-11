#pragma once
#include "playerbot/LootObjectStack.h"
#include "GenericActions.h"
#include "ChatShortcutActions.h"

namespace ai
{
    class LootStrategyAction : public ChatCommandAction
    {
    public:
        LootStrategyAction(PlayerbotAI* ai) : ChatCommandAction(ai, "ll") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void TellLootList(Player* requester, const std::string& name);
        void TellGoList(Player* requester, const std::string& name);
    };

    class CollectAction : public ChatCommandAction
    {
    public:
        CollectAction(PlayerbotAI* ai) : ChatCommandAction(ai, "collect") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        void TellCollectState(Player* requester);
    };

    class SurveyAction : public ChatCommandAction
    {
    public:
        SurveyAction(PlayerbotAI* ai) : ChatCommandAction(ai, "survey") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };

    class FindObjectAction : public ReturnPositionResetAction
    {
    public:
        FindObjectAction(PlayerbotAI* ai) : ReturnPositionResetAction(ai, "find") {}
        virtual bool Execute(Event& event) override;
    };

    class GetObjectAction : public ChatCommandAction
    {
    public:
        GetObjectAction(PlayerbotAI* ai) : ChatCommandAction(ai, "get") {}
        virtual bool Execute(Event& event) override;
    };
}
