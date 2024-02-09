#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotHelpMgr.h"


#include "DatabaseEnv.h"
#include "PlayerbotAI.h"

#ifdef GenerateBotHelp
#include <iomanip>

#include "strategy/priest/PriestAiObjectContext.h"
#include "strategy/mage/MageAiObjectContext.h"
#include "strategy/warlock/WarlockAiObjectContext.h"
#include "strategy/warrior/WarriorAiObjectContext.h"
#include "strategy/shaman/ShamanAiObjectContext.h"
#include "strategy/paladin/PaladinAiObjectContext.h"
#include "strategy/druid/DruidAiObjectContext.h"
#include "strategy/hunter/HunterAiObjectContext.h"
#include "strategy/rogue/RogueAiObjectContext.h"
#include "strategy/deathknight/DKAiObjectContext.h"
#endif

PlayerbotHelpMgr::PlayerbotHelpMgr()
{
}

PlayerbotHelpMgr::~PlayerbotHelpMgr()
{
}

void PlayerbotHelpMgr::replace(string& text, const string what, const string with)
{
    size_t start_pos = 0;
    while ((start_pos = text.find(what, start_pos)) != std::string::npos) {
        text.replace(start_pos, what.size(), with);
        start_pos += with.size();
    }
}

string PlayerbotHelpMgr::makeList(vector<string>const parts, string partFormat, uint32 maxLength)
{
    string retString = "";
    string currentLine = "";

    for (auto part : parts)
    {
        if (part.length() + currentLine.length() > maxLength && !currentLine.empty())
        {
            currentLine.pop_back();
            retString += currentLine + "\n";
            currentLine.clear();
        }
        string subPart = partFormat;

        replace(subPart, "<part>", part);

        currentLine += subPart + " ";
    }

    return retString + currentLine;
}

#ifdef GenerateBotHelp
string PlayerbotHelpMgr::formatFloat(float num)
{
    ostringstream out;
    out << std::fixed << std::setprecision(3);
    out << num;
    return out.str().c_str();
}

bool PlayerbotHelpMgr::IsGenericSupported(PlayerbotAIAware* object)
{
    set<string> supported;
    string name = object->getName();
    if (dynamic_cast<Strategy*>(object))
        supported = genericContext->GetSupportedStrategies();
    else if (dynamic_cast<Trigger*>(object))
        supported = genericContext->GetSupportedTriggers();
    if (dynamic_cast<Action*>(object))
    {
        supported = genericContext->GetSupportedActions();
        name = supportedActionName[name];
    }
    else if (dynamic_cast<UntypedValue*>(object))
        supported = genericContext->GetSupportedValues();

    return supported.find(name) != supported.end();
}

string PlayerbotHelpMgr::GetObjectName(PlayerbotAIAware* object, string className)
{
    return IsGenericSupported(object) ? object->getName() : className + " " + object->getName();
}

string PlayerbotHelpMgr::GetObjectLink(PlayerbotAIAware* object, string className)
{
    string prefix = "unkown";
    if (dynamic_cast<Strategy*>(object))
        prefix = "strategy";
    if (dynamic_cast<Trigger*>(object))
        prefix = "trigger";
    if (dynamic_cast<Action*>(object))
        prefix = "action";
    if (dynamic_cast<UntypedValue*>(object))
        prefix = "value";

    if (className != "generic")
        return "[h:" + prefix + ":" + GetObjectName(object, className) + "|" + object->getName() + "]";
    
    return "[h:" + prefix + "|" + object->getName() + "]";   
}

void PlayerbotHelpMgr::LoadStrategies(string className, AiObjectContext* context)
{
    ai->SetAiObjectContext(context);

    vector<string> stratLinks;
    for (auto strategyName : context->GetSupportedStrategies())
    {
        if (strategyName == "custom")
            continue;

        Strategy* strategy = context->GetStrategy(strategyName);

        if (!strategy)
            sLog.outError("Strategy %s is defined but can not be created.", strategyName.c_str());

        if (className != "generic" && IsGenericSupported(strategy))
            continue;

        for (auto stat : states)
        {
            BotState state = stat.first;

            std::list<TriggerNode*> triggers;
            strategy->InitTriggers(triggers, state);

            if (!triggers.empty())
            {
                triggers.sort([](TriggerNode* a, TriggerNode* b) {return a->getName() < b->getName(); });

                for (auto& triggerNode : triggers)
                {
                    Trigger* trigger = context->GetTrigger(triggerNode->getName());

                    if (!trigger)
                        sLog.outError("Trigger %s is used in strategy %s but can not be created.", triggerNode->getName().c_str(), strategyName.c_str());

                    if (trigger)
                    {
                        triggerNode->setTrigger(trigger);

                        NextAction** nextActions = triggerNode->getHandlers();

                        vector<NextAction*> nextActionList;

                        for (int32 i = 0; i < NextAction::size(nextActions); i++)
                            nextActionList.push_back(nextActions[i]);

                        std::sort(nextActionList.begin(), nextActionList.end(), [](NextAction* a, NextAction* b) {return a->getName() < b->getName(); });

                        for (auto nextAction : nextActionList)
                        {
                            Action* action = context->GetAction(nextAction->getName());

                            if (!action)
                                sLog.outError("Action %s is defined in strategy %s for trigger %s but can not be created.", nextAction->getName().c_str(), strategyName.c_str(), triggerNode->getName().c_str());

                            if (action)
                            {
                                classMap[className][strategy][state][trigger][action] = nextAction->getRelevance();
                                supportedActionName[action->getName()] = nextAction->getName();
                            }
                        }
                    }
                }
            }

            if (strategy->getDefaultActions(state))
            {
                vector<NextAction*> nextActionList;

                for (int32 i = 0; i < NextAction::size(strategy->getDefaultActions(state)); i++)
                    nextActionList.push_back(strategy->getDefaultActions(state)[i]);

                std::sort(nextActionList.begin(), nextActionList.end(), [](NextAction* a, NextAction* b) {return a->getName() < b->getName(); });

                for (auto nextAction : nextActionList)
                {
                    Action* action = context->GetAction(nextAction->getName());

                    if (!action)
                        sLog.outError("Default action %s is defined in strategy %s but can not be created.", action->getName().c_str(), strategyName.c_str());

                    if (action)
                    {
                        classMap[className][strategy][state][nullptr][action] = nextAction->getRelevance();
                        supportedActionName[action->getName()] = nextAction->getName();
                    }
                }
            }
        }

        if (classMap[className][strategy].empty()) //Modifier strategies.
            classMap[className][strategy][BotState::BOT_STATE_ALL][nullptr][nullptr] = 0.0f;
    }
}

void PlayerbotHelpMgr::LoadAllStrategies()
{
    classMap.clear();

    genericContext = new AiObjectContext(ai);

    classContext["generic"] = genericContext;
    classContext["warrior"] = new WarriorAiObjectContext(ai);
    classContext["paladin"] = new PaladinAiObjectContext(ai);
    classContext["hunter"] = new HunterAiObjectContext(ai);
    classContext["rogue"] = new RogueAiObjectContext(ai);
    classContext["priest"] = new PriestAiObjectContext(ai);
//#ifdef MANGOSBOT_TWO
    classContext["deathknight"] = new DKAiObjectContext(ai);
//#endif
    classContext["shaman"] = new ShamanAiObjectContext(ai);
    classContext["mage"] = new MageAiObjectContext(ai);
    classContext["warlock"] = new WarlockAiObjectContext(ai);
    classContext["druid"] = new DruidAiObjectContext(ai);

    for (auto cc : classContext)
    {
        LoadStrategies(cc.first, cc.second);
    }
}

string PlayerbotHelpMgr::GetStrategyBehaviour(string className, Strategy* strategy)
{
    string behavior;

    AiObjectContext* context = classContext[className];

    if (!classMap[className][strategy].empty())
    {
        for (auto stat : classMap[className][strategy])
        {
            BotState state = stat.first;

            if (state == BotState::BOT_STATE_ALL && !stat.second.begin()->first && !stat.second.begin()->second.begin()->first) //Modifier strategies
                return "";

            behavior += "\n" + initcap(states[state]) + " behavior:";

            for (auto trig : stat.second)
            {
                string line;

                Trigger* trigger = trig.first;

                if (!trig.first)
                    line = "Default:";
                else
                    line = "On: " + GetObjectLink(trigger, className) + " do ";

                for (auto act : trig.second)
                {
                    line += GetObjectLink(act.first, className) + " (" + formatFloat(act.second) + ")";
                }

                behavior += "\n" + line;
            }
        }
    }

    return behavior;
}

void PlayerbotHelpMgr::GenerateStrategyHelp()
{
    for (auto& strategyClass : classMap)
    {
        string className = strategyClass.first;

        vector<string> stratLinks;

        for (auto& strat : strategyClass.second)
        {
            Strategy* strategy = strat.first;

            string strategyName = strategy->getName();
            string linkName = GetObjectName(strategy, className);

            stratLinks.push_back(GetObjectLink(strategy, className));

            string helpTemplate = botHelpText["template:strategy"].m_templateText;

            string description, related;
            if (strategy->GetHelpName() == strategyName) //Only get description if defined in trigger
            {
                description = strategy->GetHelpDescription();
                related = makeList(strategy->GetRelatedStrategies(), "[h:strategy|<part>]");
                coverageMap["strategy"][strategyName] = true;
            }
            else
                coverageMap["strategy"][strategyName] = false;

            if (!related.empty())
                related = "\nRelated strategies:\n" + related;

            string behavior = GetStrategyBehaviour(className, strategy);

            replace(helpTemplate, "<name>", strategyName);
            replace(helpTemplate, "<description>", description);
            replace(helpTemplate, "<related>", related);
            replace(helpTemplate, "<behavior>", behavior);

            if (botHelpText["strategy:" + linkName].m_templateText != helpTemplate)
                botHelpText["strategy:" + linkName].m_templateChanged = true;
            botHelpText["strategy:" + linkName].m_templateText = helpTemplate;
        }

        std::sort(stratLinks.begin(), stratLinks.end());

        if (botHelpText["list:" + className + " strategy"].m_templateText != " strategies : \n" + makeList(stratLinks))
            botHelpText["list:" + className + " strategy"].m_templateChanged = true;
        botHelpText["list:" + className + " strategy"].m_templateText = className + " strategies : \n" + makeList(stratLinks);
    }
}

string PlayerbotHelpMgr::GetTriggerBehaviour(string className, Trigger* trigger)
{
    string behavior;

    AiObjectContext* context = classContext[className];

    for (auto sstat : states)
    {
        BotState state = sstat.first;
        string  stateName = sstat.second;
        string stateBehavior;

        for (auto strat : classMap[className])
        {
            auto stat = strat.second[state];

            if (stat.find(trigger) == stat.end())
                continue;

            if (stateBehavior.empty())
                stateBehavior += "\n" + initcap(states[state]) + " behavior:";

            string line;

            line = "Executes: ";

            for (auto act : stat[trigger])
            {
                line += GetObjectLink(act.first, className) + " (" + formatFloat(act.second) + ")";
            }

            line += " for " + GetObjectLink(strat.first, className);

            stateBehavior += "\n" + line;
        }
        behavior += stateBehavior;
    }

    return behavior;
}

void PlayerbotHelpMgr::GenerateTriggerHelp()
{
    for (auto& strategyClass : classMap)
    {
        string className = strategyClass.first;

        vector<string> trigLinks;
        vector<string> triggers;

        for (auto& strat : strategyClass.second)
        {
            for (auto stat : strat.second)
            {
                for (auto& trig : stat.second)
                {
                    Trigger* trigger = trig.first;

                    if (!trigger) //Ignore default actions
                        continue;

                    string triggerName = trigger->getName();
                    string linkName = GetObjectName(trigger, className);

                    if (std::find(triggers.begin(), triggers.end(), triggerName) != triggers.end())
                        continue;

                    triggers.push_back(triggerName);

                    trigLinks.push_back(GetObjectLink(trigger, className));

                    string helpTemplate = botHelpText["template:trigger"].m_templateText;

                    string description, relatedTrig, relatedVal;
                    if (trigger->GetHelpName() == triggerName) //Only get description if defined in trigger
                    {
                        description = trigger->GetHelpDescription();
                        relatedTrig = makeList(trigger->GetUsedTriggers(), "[h:trigger|<part>]");
                        relatedVal = makeList(trigger->GetUsedValues(), "[h:value|<part>]");
                        coverageMap["trigger"][triggerName] = true;
                    }
                    else
                        coverageMap["trigger"][triggerName] = false;

                    if (!relatedTrig.empty())
                        relatedTrig = "\nUsed triggers:\n" + relatedTrig;
                    if (!relatedVal.empty())
                        relatedVal = "\nUsed values:\n" + relatedVal;

                    string behavior = GetTriggerBehaviour(className, trigger);

                    replace(helpTemplate, "<name>", triggerName);
                    replace(helpTemplate, "<description>", description);
                    replace(helpTemplate, "<used trig>", relatedTrig);
                    replace(helpTemplate, "<used val>", relatedVal);
                    replace(helpTemplate, "<behavior>", behavior);

                    if (botHelpText["trigger:" + linkName].m_templateText != helpTemplate)
                        botHelpText["trigger:" + linkName].m_templateChanged = true;
                    botHelpText["trigger:" + linkName].m_templateText = helpTemplate;
                }
            }
        }

        std::sort(trigLinks.begin(), trigLinks.end());


        if (botHelpText["list:" + className + " trigger"].m_templateText != " triggers : \n" + makeList(trigLinks))
            botHelpText["list:" + className + " trigger"].m_templateChanged = true;
        botHelpText["list:" + className + " trigger"].m_templateText = className + " triggers : \n" + makeList(trigLinks);       
    }
}

string PlayerbotHelpMgr::GetActionBehaviour(string className, Action* nextAction)
{
    string behavior;

    AiObjectContext* context = classContext[className];

    for (auto sstat : states)
    {
        BotState state = sstat.first;
        string  stateName = sstat.second;
        string stateBehavior;

        for (auto strat : classMap[className])
        {
            auto stat = strat.second[state];

            for (auto trig : stat)
            {
                if (trig.second.find(nextAction) == trig.second.end())
                    continue;


                if (stateBehavior.empty())
                    stateBehavior += "\n" + initcap(states[state]) + " behavior:";

                string line;

                if (trig.first)
                    line = "Triggers from: " + GetObjectLink(trig.first, className);
                else
                    line = "Default action ";

                line += " with relevance (" + formatFloat(trig.second.find(nextAction)->second) + ")";
                line += " for " + GetObjectLink(strat.first, className);

                stateBehavior += "\n" + line;
            }
        }

        behavior += stateBehavior;
    }

    return behavior;
}

void PlayerbotHelpMgr::GenerateActionHelp()
{
    for (auto& strategyClass : classMap)
    {
        string className = strategyClass.first;

        vector<string> actionLinks;
        vector<string> actions;

        for (auto& strat : strategyClass.second)
        {
            for (auto stat : strat.second)
            {
                for (auto& trig : stat.second)
                {
                    Trigger* trigger = trig.first;

                    if (!trigger) //Ignore default actions
                        continue;

                    for (auto& act : trig.second)
                    {
                        string ActionName = act.first->getName();
                        Action* action = act.first;

                        string linkName = GetObjectName(action, className);

                        if (std::find(actions.begin(), actions.end(), ActionName) != actions.end())
                            continue;

                        actions.push_back(ActionName);

                        actionLinks.push_back(GetObjectLink(action, className));

                        string helpTemplate = botHelpText["template:action"].m_templateText;

                        string description, relatedAct, relatedVal;
                        if (action->GetHelpName() == ActionName) //Only get description if defined in trigger
                        {
                            description = action->GetHelpDescription();
                            relatedAct = makeList(action->GetUsedActions(), "[h:action|<part>]");
                            relatedVal = makeList(action->GetUsedValues(), "[h:value|<part>]");
                            coverageMap["action"][ActionName] = true;
                        }
                        else
                            coverageMap["action"][ActionName] = false;

                        if (!relatedAct.empty())
                            relatedAct = "\nUsed actions:\n" + relatedAct;
                        if (!relatedVal.empty())
                            relatedVal = "\nUsed values:\n" + relatedVal;

                        string behavior = GetActionBehaviour(className, act.first);

                        replace(helpTemplate, "<name>", ActionName);
                        replace(helpTemplate, "<description>", description);
                        replace(helpTemplate, "<used act>", relatedAct);
                        replace(helpTemplate, "<used val>", relatedVal);
                        replace(helpTemplate, "<behavior>", behavior);

                        if (botHelpText["action:" + linkName].m_templateText != helpTemplate)
                            botHelpText["action:" + linkName].m_templateChanged = true;
                        botHelpText["action:" + linkName].m_templateText = helpTemplate;
                    }
                }
            }
        }

        std::sort(actionLinks.begin(), actionLinks.end());

        if (botHelpText["list:" + className + " action"].m_templateText != " action : \n" + makeList(actionLinks))
            botHelpText["list:" + className + " action"].m_templateChanged = true;
        botHelpText["list:" + className + " action"].m_templateText = className + " action : \n" + makeList(actionLinks);
    }
}

void PlayerbotHelpMgr::GenerateValueHelp()
{
    AiObjectContext* genericContext = classContext["generic"];
    set<string> genericValues = genericContext->GetSupportedValues();

    unordered_map<string, vector<string>> valueLinks;

    for (auto& strategyClass : classMap)
    {
        string className = strategyClass.first;

        vector<string> values;

        AiObjectContext* context = classContext[className];

        set<string> allValues = context->GetSupportedValues();
        for (auto& valueName : allValues)
        {
            if (className != "generic" && std::find(genericValues.begin(), genericValues.end(), valueName) != genericValues.end())
                continue;

            UntypedValue* value = context->GetUntypedValue(valueName);

            string linkName = GetObjectName(value, className);

            if (std::find(values.begin(), values.end(), valueName) != values.end())
                continue;

            values.push_back(valueName);

            string valueType;

            string helpTemplate = botHelpText["template:value"].m_templateText;

            string description, usedVal, relatedTrig, relatedAct, relatedVal;
            if (value->GetHelpName() == valueName) //Only get description if defined in trigger
            {
                vector<string> usedInTrigger, usedInAction, usedInValue;
                for (auto& strat : strategyClass.second)
                {
                    for (auto stat : strat.second)
                    {
                        for (auto& trig : stat.second)
                        {
                            if (!trig.first)
                                continue;

                            if (trig.first->GetHelpName() == trig.first->getName())
                                for (auto val : trig.first->GetUsedValues())
                                    if (val == valueName)
                                        if (std::find(usedInTrigger.begin(), usedInTrigger.end(), trig.first->getName()) == usedInTrigger.end())
                                            usedInTrigger.push_back(trig.first->getName());

                            for (auto& act : trig.second)
                            {
                                if (act.first->GetHelpName() == act.first->getName())
                                    for (auto val : act.first->GetUsedValues())
                                        if (val == valueName)
                                            if (std::find(usedInAction.begin(), usedInAction.end(), act.first->getName()) == usedInAction.end())
                                                usedInAction.push_back(act.first->getName());
                            }
                        }
                    }
                }

                for (auto& otherValueName : allValues)
                {
                    UntypedValue* otherValue = context->GetUntypedValue(valueName);

                    if (otherValue->GetHelpName() == otherValueName)
                        for (auto val : otherValue->GetUsedValues())
                            if (val == valueName)
                                if (std::find(usedInValue.begin(), usedInValue.end(), otherValue->getName()) == usedInValue.end())
                                    usedInValue.push_back(otherValue->getName());
                }

                description = value->GetHelpDescription();
                valueType = value->GetHelpTypeName();
                relatedTrig = makeList(usedInTrigger, "[h:trigger|<part>]");
                relatedAct = makeList(usedInAction, "[h:action|<part>]");
                relatedVal = makeList(usedInValue, "[h:value|<part>]");
                usedVal = makeList(value->GetUsedValues(), "[h:value|<part>]");
                coverageMap["value"][valueName] = true;
            }
            else
                coverageMap["value"][valueName] = false;

            valueLinks[valueType].push_back(GetObjectLink(value, className));

            if (!usedVal.empty())
                usedVal = "\nUsed values:\n" + usedVal;
            if (!relatedTrig.empty())
                relatedTrig = "\nUsed in triggers:\n" + relatedTrig;
            if (!relatedAct.empty())
                relatedAct = "\nUsed in actions:\n" + relatedAct;
            if (!relatedVal.empty())
                relatedVal = "\nUsed in values:\n" + relatedVal;

            replace(helpTemplate, "<name>", valueName);
            replace(helpTemplate, "<description>", description);
            replace(helpTemplate, "<used in trig>", relatedTrig);
            replace(helpTemplate, "<used in act>", relatedAct);
            replace(helpTemplate, "<used in val>", relatedVal);
            replace(helpTemplate, "<used val>", usedVal);

            if (botHelpText["value:" + linkName].m_templateText != helpTemplate)
                botHelpText["value:" + linkName].m_templateChanged = true;
            botHelpText["value:" + linkName].m_templateText = helpTemplate;
        }
    }

    vector<string> valueTypes;

    for (auto valueLinkSet : valueLinks)
        if (!valueLinkSet.first.empty())
            valueTypes.push_back(valueLinkSet.first);

    valueTypes.push_back("");

    vector<string> typeLinks;

    for (auto type : valueTypes)
    {
        vector<string> links = valueLinks[type];
        std::sort(links.begin(), links.end());

        if (type.empty())
            type = "other";

        botHelpText["list:" + type + " value"].m_templateText = initcap(type) + " values : \n" + makeList(links);
        typeLinks.push_back("[h:list|" + type + " value]");
    }

    string valueHelp = botHelpText["object:value"].m_templateText;

    valueHelp = valueHelp.substr(0, valueHelp.find("Values:"));
    valueHelp += "Values:" + makeList(typeLinks);

    if (botHelpText["object:value"].m_templateText != valueHelp)
        botHelpText["object:value"].m_templateChanged = true;
    botHelpText["object:value"].m_templateText = valueHelp;
}

void PlayerbotHelpMgr::GenerateChatFilterHelp()
{
    CompositeChatFilter* filter = new CompositeChatFilter(ai);

    vector<string> filterLinks;

    for (auto subFilter : filter->GetFilters())
    {
        string helpTemplate = botHelpText["template:chatfilter"].m_templateText;

        string filterName = subFilter->GetHelpName();
        string description = subFilter->GetHelpDescription();
        string examples;
        for (auto& example : subFilter->GetFilterExamples())
        {
            string line = example.first + ": " + example.second;
            examples += (examples.empty() ? "" : "\n") + line;
        }

        replace(helpTemplate, "<name>", filterName);
        replace(helpTemplate, "<description>", description);
        replace(helpTemplate, "<examples>", examples);

        filterLinks.push_back("[h:chatfilter|" + filterName + "]");

        if (botHelpText["chatfilter:" + filterName].m_templateText != helpTemplate)
            botHelpText["chatfilter:" + filterName].m_templateChanged = true;
        botHelpText["chatfilter:" + filterName].m_templateText = helpTemplate;
    }

    string filterHelp = botHelpText["object:chatfilter"].m_templateText;

    filterHelp = filterHelp.substr(0, filterHelp.find("Filters:"));
    filterHelp += "Filters:" + makeList(filterLinks);

    if (botHelpText["object:chatfilter"].m_templateText != filterHelp)
        botHelpText["object:chatfilter"].m_templateChanged = true;
    botHelpText["object:chatfilter"].m_templateText = filterHelp;
  
    delete filter;
}

void PlayerbotHelpMgr::PrintCoverage()
{
    for (auto typeCov : coverageMap)
    {
        vector<string> missingNames;
        uint32 totalNames = 0, hasNames = 0;

        for (auto cov : typeCov.second)
        {
            totalNames++;

            if (!cov.second)
                missingNames.push_back(cov.first);
        }

        hasNames = totalNames - missingNames.size();

        if (!totalNames)
            continue;

        sLog.outString("%s help coverage %d/%d : %d%%", typeCov.first, hasNames, totalNames, (hasNames * 100) / totalNames);

        if (missingNames.empty())
            continue;

        sLog.outBasic("Missing:");

        std::sort(missingNames.begin(), missingNames.end());
        for (auto name : missingNames)
            sLog.outBasic("%s", name.c_str());
    }
}

void PlayerbotHelpMgr::SaveTemplates()
{
    for (auto text : botHelpText)
    {
        if (!text.second.m_templateChanged)
            continue;

        string name = text.first;
        string temp = text.second.m_templateText;
        replace(name, "'", "''");
        replace(temp, "\r\n", "\n");
        replace(temp, "\n", "\\r\\n");
        replace(temp, "'", "''");
        if (text.second.m_new)
            WorldDatabase.PExecute("INSERT INTO `ai_playerbot_help_texts` (`name`, `template_text`, `template_changed`) VALUES ('%s', '%s', 1)", name.c_str(), temp.c_str());
        else
            WorldDatabase.PExecute("UPDATE `ai_playerbot_help_texts` set  `template_text` = '%s',  `template_changed` = 1 where `name` = '%s'", temp.c_str(), name.c_str());
    }
}

void PlayerbotHelpMgr::GenerateHelp()
{
    coverageMap.clear();

    WorldSession* session = new WorldSession(0, NULL, SEC_PLAYER,

#ifdef MANGOSBOT_TWO
        2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ONE
    2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ZERO
    0, LOCALE_enUS, "", 0);
#endif

    session->SetNoAnticheat();

    Player* bot = new Player(session);

    bot->Create(sObjectMgr.GeneratePlayerLowGuid(), "test", 1, 1, 0,
        0, // skinColor,
        0,
        0,
        0, // hairColor,
        0, 0);

    ai = new PlayerbotAI(bot);

    LoadAllStrategies();
    
    GenerateStrategyHelp();
    GenerateTriggerHelp();
    GenerateActionHelp();
    GenerateValueHelp();
    GenerateChatFilterHelp();

    PrintCoverage();

    SaveTemplates();

    delete ai;
    delete bot;
}
#endif

void PlayerbotHelpMgr::FormatHelpTopic(string& text)
{
    //[h:subject:topic|name]
    size_t start_pos = 0;
    while ((start_pos = text.find("[h:", start_pos)) != std::string::npos) {
        size_t end_pos = text.find("]", start_pos);

        if (end_pos == std::string::npos)
            break;

        string oldLink = text.substr(start_pos, end_pos - start_pos + 1);

        if (oldLink.find("|") != std::string::npos)
        {
            string topicCode = oldLink.substr(3, oldLink.find("|") - 3);
            std::string topicName = oldLink.substr(oldLink.find("|") + 1);
            topicName.pop_back();

            if (topicCode.find(":") == std::string::npos)
                topicCode += ":" + topicName;

            string newLink = ChatHelper::formatValue("help", topicCode, topicName);

            text.replace(start_pos, oldLink.length(), newLink);
            start_pos += newLink.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
        else
            start_pos= end_pos;
    }

    //[c:command|name]
    start_pos = 0;
    while ((start_pos = text.find("[c:", start_pos)) != std::string::npos) {
        size_t end_pos = text.find("]", start_pos);

        if (end_pos == std::string::npos)
            break;

        string oldLink = text.substr(start_pos, end_pos - start_pos + 1);

        if (oldLink.find("|") != std::string::npos)
        {
            string topicCode = oldLink.substr(3, oldLink.find("|") - 3);
            std::string topicName = oldLink.substr(oldLink.find("|") + 1);
            topicName.pop_back();                                                                                      
            string newLink = ChatHelper::formatValue("command", topicCode, topicName, "0000FF00");

            text.replace(start_pos, oldLink.length(), newLink);
            start_pos += newLink.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
        else
            start_pos = end_pos;
    }

}

void PlayerbotHelpMgr::FormatHelpTopics()
{
    for (auto& helpText : botHelpText)
    {
        if (helpText.second.m_text.empty())
            helpText.second.m_text = helpText.second.m_templateText;

        FormatHelpTopic(helpText.second.m_text);

        for (uint8 i = 1; i < MAX_LOCALE; ++i)
        {
            FormatHelpTopic(helpText.second.m_text_locales[sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i))]);
        }
    }
}

void PlayerbotHelpMgr::LoadBotHelpTexts()
{
    sLog.outBasic("Loading playerbot texts...");
    auto results = WorldDatabase.PQuery("SELECT `name`, `template_text`, `text`, `text_loc1`, `text_loc2`, `text_loc3`, `text_loc4`, `text_loc5`, `text_loc6`, `text_loc7`, `text_loc8` FROM `ai_playerbot_help_texts`");
    int count = 0;
    if (results)
    {
        do
        {
            std::string text, templateText;
            std::map<int32, std::string> text_locale;
            Field* fields = results->Fetch();
            string name = fields[0].GetString();
            templateText = fields[1].GetString();
            text = fields[2].GetString();

            for (uint8 i = 1; i < MAX_LOCALE; ++i)
            {
                text_locale[sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i))] = fields[i + 2].GetString();
            }

            botHelpText[name] = BotHelpEntry(templateText, text, text_locale);

            count++;
        } while (results->NextRow());
    }
    sLog.outBasic("%d playerbot helptexts loaded", count);

#ifdef GenerateBotHelp
    GenerateHelp();
#endif

    FormatHelpTopics();
}

// general texts

string PlayerbotHelpMgr::GetBotText(string name)
{
    if (botHelpText.empty())
    {
        sLog.outError("Can't get bot help text %s! No bots help texts loaded!", name.c_str());
        return "";
    }
    if (botHelpText.find(name) == botHelpText.end())
    {
        sLog.outDetail("Can't get bot help text %s! No bots help texts for this name!", name.c_str());
        return "";
    }

    BotHelpEntry textEntry = botHelpText[name];
    int32 localePrio = sPlayerbotTextMgr.GetLocalePriority();
    if (localePrio == -1)
        return textEntry.m_text;
    else
    {
        if (!textEntry.m_text_locales[localePrio].empty())
            return textEntry.m_text_locales[localePrio];
        else
            return textEntry.m_text;
    }
}

bool PlayerbotHelpMgr::GetBotText(string name, string &text)
{
    text = GetBotText(name);
    return !text.empty();
}

vector<string> PlayerbotHelpMgr::FindBotText(string name)
{
    vector<string> found;
    for (auto text : botHelpText)
    {
        if (text.first.find(name) == string::npos)
            continue;

        found.push_back(text.first);
    }

    return found;
}