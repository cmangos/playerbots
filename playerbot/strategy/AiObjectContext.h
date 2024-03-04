#pragma once

#include "playerbot/PlayerbotAIAware.h"
#include "Action.h"
#include "Value.h"
#include "NamedObjectContext.h"
#include "Strategy.h"

namespace ai
{
    class UntypedValue;
    template<class T> class Value;
}

namespace ai
{
    class AiObjectContext : public PlayerbotAIAware
    {
    public:
        AiObjectContext(PlayerbotAI* ai);
        virtual ~AiObjectContext() {}

    public:
        virtual Strategy* GetStrategy(const string& name) { return strategyContexts.GetObject(name, ai); }
        virtual set<string> GetSiblingStrategy(const string& name) { return strategyContexts.GetSiblings(name); }
        virtual Trigger* GetTrigger(const string& name) { return triggerContexts.GetObject(name, ai); }
        virtual Action* GetAction(const string& name) { return actionContexts.GetObject(name, ai); }
        virtual UntypedValue* GetUntypedValue(const string& name) { return valueContexts.GetObject(name, ai); }

        template<class T>
        Value<T>* GetValue(const string& name)
        {
            return dynamic_cast<Value<T>*>(GetUntypedValue(name));
        }

        template<class T>
        Value<T>* GetValue(const string& name, const string& param)
        {
            return GetValue<T>((string(name) + "::" + param));
        }

        template<class T>
        Value<T>* GetValue(const string& name, int32 param)
        {
        	ostringstream out; out << param;
            return GetValue<T>(name, out.str());
        }

        bool HasValue(const string& name)
        {
            return valueContexts.IsCreated(name);
        }

        bool HasValue(const string& name, const string& param)
        {
            return HasValue((string(name) + "::" + param));
        }

        bool HasValue(const string& name, int32 param)
        {
            ostringstream out; out << param;
            return HasValue(name, out.str());
        }


        set<string> GetValues()
        {
            return valueContexts.GetCreated();
        }

        set<string> GetSupportedStrategies()
        {
            return strategyContexts.supports();
        }

        set<string> GetSupportedTriggers()
        {
            return triggerContexts.supports();
        }

        set<string> GetSupportedActions()
        {
            return actionContexts.supports();
        }

        set<string> GetSupportedValues ()
        {
            return valueContexts.supports();
        }

        void ClearValues(string findName = "");

        void ClearExpiredValues(string findName = "", uint32 interval = 0);

        string FormatValues(string findName = "");

    public:
        virtual void Update();
        virtual void Reset();
        virtual void AddShared(NamedObjectContext<UntypedValue>* sharedValues)
        {
            valueContexts.Add(sharedValues);
        }
        list<string> Save();
        void Load(list<string> data);

        vector<string> performanceStack;
    protected:
        NamedObjectContextList<Strategy> strategyContexts;
        NamedObjectContextList<Action> actionContexts;
        NamedObjectContextList<Trigger> triggerContexts;
        NamedObjectContextList<UntypedValue> valueContexts;
    };
}


#define AI_VALUE(type, name) context->GetValue<type>(name)->Get()
#define AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Get()

#define AI_VALUE_LAZY(type, name) context->GetValue<type>(name)->LazyGet()
#define AI_VALUE2_LAZY(type, name, param) context->GetValue<type>(name, param)->LazyGet()

#define HAS_AI_VALUE(name) context->HasValue(name)
#define HAS_AI_VALUE2(name, param) context->HasValue(name, param)
#define AI_VALUE_EXISTS(type, name, emptyval) (HAS_AI_VALUE(name) ? AI_VALUE(type, name) : emptyval)
#define AI_VALUE2_EXISTS(type, name, param, emptyval) (HAS_AI_VALUE2(name, param) ? AI_VALUE2(type, name, param) : emptyval)

#define SET_AI_VALUE(type, name, value) context->GetValue<type>(name)->Set(value)
#define SET_AI_VALUE2(type, name, param, value) context->GetValue<type>(name, param)->Set(value)
#define RESET_AI_VALUE(type, name) context->GetValue<type>(name)->Reset()
#define RESET_AI_VALUE2(type, name, param) context->GetValue<type>(name, param)->Reset()

#define PAI_VALUE(type, name) player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name)->Get()
#define PAI_VALUE2(type, name, param) player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name, param)->Get()
#define PHAS_AI_VALUE(name) player->GetPlayerbotAI()->GetAiObjectContext()->HasValue(name)
#define PHAS_AI_VALUE2(name, param) player->GetPlayerbotAI()->GetAiObjectContext()->HasValue(name, param)
#define MAI_VALUE(type, name) master->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name)->Get()
#define MAI_VALUE2(type, name, param) master->GetPlayerbotAI()->GetAiObjectContext()->GetValue<type>(name, param)->Get()
#define GAI_VALUE(type, name) sSharedObjectContext.GetValue<type>(name)->Get()
#define GAI_VALUE2(type, name, param) sSharedObjectContext.GetValue<type>(name, param)->Get()

#define MEM_AI_VALUE(type, name) dynamic_cast<MemoryCalculatedValue<type>*>(context->GetUntypedValue(name))
#define LOG_AI_VALUE(type, name) dynamic_cast<LogCalculatedValue<type>*>(context->GetUntypedValue(name))