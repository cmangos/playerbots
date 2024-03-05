#pragma once
#include "GenericActions.h"

namespace ai
{
	class ChangeCombatStrategyAction : public ChatCommandAction
    {
	public:
		ChangeCombatStrategyAction(PlayerbotAI* ai, std::string name = "co") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
    };

    class ChangeNonCombatStrategyAction : public ChatCommandAction
    {
    public:
        ChangeNonCombatStrategyAction(PlayerbotAI* ai, std::string name = "nc") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
    };

    class ChangeDeadStrategyAction : public ChatCommandAction
    {
    public:
        ChangeDeadStrategyAction(PlayerbotAI* ai, std::string name = "de") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
    };

    class ChangeReactionStrategyAction : public ChatCommandAction
    {
    public:
        ChangeReactionStrategyAction(PlayerbotAI* ai, std::string name = "react") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
    };

    class ChangeAllStrategyAction : public ChatCommandAction
    {
    public:
        ChangeAllStrategyAction(PlayerbotAI* ai, std::string name = "change strategy from all", std::string strategy = "") : ChatCommandAction(ai, name), strategy(strategy) {}
        virtual bool Execute(Event& event) override;

    private:
        std::string strategy;
    };
}
