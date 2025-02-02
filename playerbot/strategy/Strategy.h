#pragma once
#include "Action.h"
#include "Multiplier.h"
#include "Trigger.h"
#include "NamedObjectContext.h"
#include "playerbot/BotState.h"
#include "playerbot/record/StrategyRecord.h"

namespace ai
{
	class ActionNode;
	class NextAction;

	enum ActionPriority
	{
	    ACTION_IDLE = 1,
	    ACTION_NORMAL = 10,
	    ACTION_HIGH = 20,
	    ACTION_MOVE = 30,
	    ACTION_INTERRUPT = 40,
	    ACTION_DISPEL = 50,
	    ACTION_LIGHT_HEAL = 60,
	    ACTION_MEDIUM_HEAL = 70,
	    ACTION_CRITICAL_HEAL = 80,
	    ACTION_EMERGENCY = 90,
		ACTION_PASSTROUGH = 100
	};

    class Strategy : public PlayerbotAIAware
    {
    public:
        Strategy(PlayerbotAI* ai);
        virtual ~Strategy() {}

    public:
        void InitTriggers(std::list<TriggerNode*> &triggers, BotState state);
        void InitMultipliers(std::list<Multiplier*> &multipliers, BotState state);

		virtual NextAction** getDefaultActions(BotState state);
		virtual int GetType() { return STRATEGY_TYPE_GENERIC; }
        virtual ActionNode* GetAction(std::string name);
		virtual std::string getName() = 0;
        void Update() {} //Nonfunctional see AiObjectContext::Update() to enable.
        void Reset() {}

		virtual void OnStrategyAdded(BotState state) {}
		virtual void OnStrategyRemoved(BotState state) {}
#ifdef GenerateBotHelp
		virtual std::string GetHelpName() { return "dummy"; } //Must equal iternal name
		virtual std::string GetHelpDescription() { return "This is a strategy."; }
		virtual std::vector<std::string> GetRelatedStrategies() { return {}; }
#endif
	protected:
		virtual NextAction** GetDefaultCombatActions() { return nullptr; }
		virtual NextAction** GetDefaultNonCombatActions() { return nullptr; }
		virtual NextAction** GetDefaultDeadActions() { return nullptr; }
		virtual NextAction** GetDefaultReactionActions() { return nullptr; }

		virtual void InitCombatTriggers(std::list<TriggerNode*>& triggers) {}
		virtual void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) {}
		virtual void InitDeadTriggers(std::list<TriggerNode*>& triggers) {}
		virtual void InitReactionTriggers(std::list<TriggerNode*>& triggers) {}

		virtual void InitCombatMultipliers(std::list<Multiplier*>& multipliers) {}
		virtual void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) {}
		virtual void InitDeadMultipliers(std::list<Multiplier*>& multipliers) {}
		virtual void InitReactionMultipliers(std::list<Multiplier*>& multipliers) {}

    protected:
        NamedObjectFactoryList<ActionNode> actionNodeFactories;
    };
}
