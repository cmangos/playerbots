
#include "playerbot/playerbot.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "NamedObjectContext.h"
#include "StrategyContext.h"
#include "triggers/TriggerContext.h"
#include "actions/ActionContext.h"
#include "triggers/ChatTriggerContext.h"
#include "actions/ChatActionContext.h"
#include "triggers/WorldPacketTriggerContext.h"
#include "actions/WorldPacketActionContext.h"
#include "values/ValueContext.h"
#include "values/SharedValueContext.h"


using namespace ai;

AiObjectContext::AiObjectContext(PlayerbotAI* ai) : PlayerbotAIAware(ai)
{
    strategyContexts.Add(new StrategyContext());
    strategyContexts.Add(new MovementStrategyContext());
    strategyContexts.Add(new AssistStrategyContext());
    strategyContexts.Add(new QuestStrategyContext());

    actionContexts.Add(new ActionContext());
    actionContexts.Add(new ChatActionContext());
    actionContexts.Add(new WorldPacketActionContext());

    triggerContexts.Add(new TriggerContext());
    triggerContexts.Add(new ChatTriggerContext());
    triggerContexts.Add(new WorldPacketTriggerContext());

    valueContexts.Add(new ValueContext());

    //valueContexts.Add(&sSharedValueContext);
}

void AiObjectContext::ClearValues(string findName)
{
    set<string> names = valueContexts.GetCreated();
    for (set<string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        if (!findName.empty() && i->find(findName) == string::npos)
            continue;

        valueContexts.Erase(*i);
    }
}

void AiObjectContext::ClearExpiredValues(string findName, uint32 interval)
{
    set<string> names = valueContexts.GetCreated();
    for (set<string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        if (value->Protected())
            continue;

        if (!findName.empty() && i->find(findName) == string::npos)
            continue;

        if (!interval && !value->Expired())
            continue;

        if (interval && !value->Expired(interval))
            continue;

        valueContexts.Erase(*i);
    }
}


string AiObjectContext::FormatValues(string findName)
{
    ostringstream out;
    set<string> names = valueContexts.GetCreated();
    for (set<string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        if (!findName.empty() && i->find(findName) == string::npos)
            continue;

        string text = value->Format();
        if (text == "?")
            continue;

        out << "{" << *i << "=" << text << "}|";
    }
    out.seekp(-1, out.cur);
    return out.str();
}

void AiObjectContext::Update()
{
    /* Disabled until there is actually a strategy, trigger, action or value that has the Update() method. Currently this takes 8% cpu and does 'NOTHING'.
    strategyContexts.Update();
    triggerContexts.Update();
    actionContexts.Update();
    valueContexts.Update();
    */
}

void AiObjectContext::Reset()
{
    strategyContexts.Reset();
    triggerContexts.Reset();
    actionContexts.Reset();
    valueContexts.Reset();
}

list<string> AiObjectContext::Save()
{
    list<string> result;

    set<string> names = valueContexts.GetCreated();
    for (set<string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        UntypedValue* value = GetUntypedValue(*i);
        if (!value)
            continue;

        string data = value->Save();
        if (data == "?")
            continue;

        string name = *i;
        ostringstream out;
        out << name;

        out << ">" << data;
        result.push_back(out.str());
    }
    return result;
}

void AiObjectContext::Load(list<string> data)
{
    for (list<string>::iterator i = data.begin(); i != data.end(); ++i)
    {
        string row = *i;
        vector<string> parts = split(row, '>');
        if (parts.size() != 2) continue;

        string name = parts[0];
        string text = parts[1];

        UntypedValue* value = GetUntypedValue(name);
        if (!value) continue;

        value->Load(text);
    }
}
