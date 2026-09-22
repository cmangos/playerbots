#pragma once

#include <map>

#include "Action.h"
#include "Queue.h"
#include "Trigger.h"
#include "Multiplier.h"
#include "AiObjectContext.h"
#include "Strategy.h"
#include "playerbot/BotState.h"

namespace ai
{
    class ActionExecutionListener
    {
    public:
        virtual bool Before(Action* action, const Event& event) = 0;
        virtual bool AllowExecution(Action* action, const Event& event) = 0;
        virtual void After(Action* action, bool executed, const Event& event) = 0;
        virtual bool OverrideResult(Action* action, bool executed, const Event& event) = 0;
        virtual ~ActionExecutionListener() {};
    };

    // -----------------------------------------------------------------------------------------------------------------------

    class ActionExecutionListeners : public ActionExecutionListener
    {
    public:
        virtual ~ActionExecutionListeners() override;

    // ActionExecutionListener
    public:
        virtual bool Before(Action* action, const Event& event) override;
        virtual bool AllowExecution(Action* action, const Event& event) override;
        virtual void After(Action* action, bool executed, const Event& event) override;
        virtual bool OverrideResult(Action* action, bool executed, const Event& event) override;

    public:
        void Add(ActionExecutionListener* listener)
        {
            listeners.push_back(listener);
        }
        void Remove(ActionExecutionListener* listener)
        {
            listeners.remove(listener);
        }

    private:
        std::list<ActionExecutionListener*> listeners;
    };

    // -----------------------------------------------------------------------------------------------------------------------

    enum ActionResult
    {
        ACTION_RESULT_UNKNOWN,
        ACTION_RESULT_OK,
        ACTION_RESULT_IMPOSSIBLE,
        ACTION_RESULT_USELESS,
        ACTION_RESULT_FAILED
    };

    class Engine : public PlayerbotAIAware
    {
    public:
        Engine(PlayerbotAI* ai, AiObjectContext *factory, BotState state);

	    void Init();
        void addStrategy(const std::string& name);
		void addStrategies(std::string first, ...);
        bool removeStrategy(const std::string& name, bool init = true);
        bool HasStrategy(const std::string& name);
        Strategy* GetStrategy(const std::string& name) const;
        void removeAllStrategies();
        void toggleStrategy(const std::string& name);
        std::string ListStrategies();
        std::list<std::string_view> GetStrategies();
		bool ContainsStrategy(StrategyType type);
		void ChangeStrategy(const std::string& names);
		void PrintStrategies(Player* requester, const std::string& engineType);
        std::string GetLastAction() { return lastAction; }
        const Action* GetLastExecutedAction() const { return lastExecutedAction; }

    public:
	    virtual bool DoNextAction(Unit*, int depth, bool minimal, bool isStunned);
	    ActionResult ExecuteAction(const std::string& name, Event& event);
        bool CanExecuteAction(const std::string& name, bool isUseful = true, bool isPossible = true);

    public:
        void AddActionExecutionListener(ActionExecutionListener* listener)
        {
            actionExecutionListeners.Add(listener);
        }
        void removeActionExecutionListener(ActionExecutionListener* listener)
        {
            actionExecutionListeners.Remove(listener);
        }

    public:
	    virtual ~Engine(void);

    protected:
        bool MultiplyAndPush(NextAction** actions, float forceRelevance, bool skipPrerequisites, const Event& event, const char* pushType);
        void Reset();
        void ProcessTriggers(bool minimal);
        void PushDefaultActions();
        void PushAgain(ActionNode* actionNode, float relevance, const Event& event);
        ActionNode* CreateActionNode(const std::string& name);
        virtual Action* InitializeAction(ActionNode* actionNode);
        virtual bool ListenAndExecute(Action* action, Event& event);
        // Hands an external (packet) trigger back once its action has had its turn.
        void ReleaseExternalEvent(const std::string& source);
        // Drops armed entries whose TriggerNode is gone, so they cannot suppress later packets of the
        // same opcode forever. Called from Init() once the trigger list has been rebuilt.
        void PruneUnhandledExternalEvents();

    private:
        void LogAction(const char* format, ...);
        void LogValues();

    protected:
	    Queue queue;
	    std::list<TriggerNode*> triggers;
        std::list<Multiplier*> multipliers;
        AiObjectContext* aiObjectContext;
        std::map<std::string, Strategy*> strategies;
        float lastRelevance;
        std::string lastAction;
        ActionExecutionListeners actionExecutionListeners;
        BotState state;
        Action* lastExecutedAction;

        // External (packet) triggers whose event has been queued but not yet handed to its action,
        // keyed by trigger name (= the event source). They are exempt from the end-of-tick trigger
        // reset, so a request that loses the tick (the engine runs one action per tick) or whose
        // basket is dropped from the queue is re-pushed instead of silently lost. Entries are removed
        // by ReleaseExternalEvent() as soon as the action has had its turn.
        std::map<std::string, Trigger*> unhandledExternalEvents;

    public:
		bool testMode;
        bool initMode = true;
    };
}
