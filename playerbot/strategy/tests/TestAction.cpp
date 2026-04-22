#include "playerbot/playerbot.h"
#include "TestAction.h"
#include "TestRegistry.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/BotState.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/TravelMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Entities/Unit.h"
#include "Spells/Spell.h"
#include "MovementMonitors.h"
#include "CombatMonitors.h"
#include "StateMonitors.h"
#include "SetupCommands.h"
#include "PartyCommands.h"
#include "FlowCommands.h"

#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>

using namespace ai;

TestAction::TestAction(PlayerbotAI* ai, std::string name)
    : Action(ai, name, 1000)
    , result(TestResult::PENDING)
    , pc(0)
{
    monitors.push_back(std::make_unique<BotDeadMonitor>());
    monitors.push_back(std::make_unique<CheckTimeMonitor>());
    monitors.push_back(std::make_unique<CheckHpMonitor>());
    monitors.push_back(std::make_unique<CheckDistanceMonitor>());
    monitors.push_back(std::make_unique<CheckMobMonitor>());
    monitors.push_back(std::make_unique<CheckPartyWipedMonitor>());
    monitors.push_back(std::make_unique<FactionMonitor>());


    commands.push_back(std::make_unique<HandleTeleport>());
    commands.push_back(std::make_unique<HandleSetGM>());
    commands.push_back(std::make_unique<HandleSetDestination>());
    commands.push_back(std::make_unique<HandleGiveItem>());
    commands.push_back(std::make_unique<HandleEquipItem>());
    commands.push_back(std::make_unique<HandleClearMobs>());
    commands.push_back(std::make_unique<HandleSpawnBot>());
    commands.push_back(std::make_unique<HandleDespawnBot>());
    commands.push_back(std::make_unique<HandleFormParty>());
    commands.push_back(std::make_unique<HandleObserve>());
    commands.push_back(std::make_unique<HandleCleanup>());
    commands.push_back(std::make_unique<HandleAssert>());
    commands.push_back(std::make_unique<HandleMonitor>());

    TestRegistry::GetAvailableTests();
}

bool TestAction::Execute(Event& event)
{
    std::string param = event.getParam();

    //LogToConsole("[TestAction] Execute called with param: " + param + " step" + std::to_string(pc));

    std::string testParam = param;

    if (result == TestResult::PENDING && !ctx.observing && pc == 0)
    {
        if (testParam.empty())
        {
            auto strategies = ai->GetStrategies(BotState::BOT_STATE_NON_COMBAT);
            for (const auto& strat : strategies)
            {
                std::string stratStr(strat);
                if (stratStr.find("test::") == 0)
                {
                    testParam = stratStr.substr(6);
                    break;
                }
            }
        }

        if (testParam.empty())
        {
            LogToConsole("[TestAction] No test name specified");
            TellMaster("No test name specified. Available tests:");
            auto tests = TestRegistry::GetAvailableTests();
            for (const auto& t : tests)
                TellMaster("  - " + t);

            result = TestResult::IMPOSSIBLE;
            ReportResult();
            return true;
        }

        if (!TestRegistry::HasTest(testParam))
        {
            LogToConsole(std::string("[TestAction] Unknown test: ") + testParam);
            TellMaster(std::string("Unknown test: ") + testParam + ". Available tests:");
            auto tests = TestRegistry::GetAvailableTests();
            for (const auto& t : tests)
                TellMaster("  - " + t);

            result = TestResult::IMPOSSIBLE;
            ReportResult();
            return true;
        }

        testName = testParam;
        ctx.testName = testName;
        ctx.script = TestRegistry::GetTestScript(testName);
        ctx.testStartTime = WorldTimer::getMSTime();

        TellMaster(std::string("Starting test: ") + testName);
        LogToConsole(std::string("[TestAction] Bot ") + bot->GetName() + " starting test: " + testName);
    }

    SET_AI_VALUE2(bool, "manual bool", "is running test", true);

    if (result != TestResult::PENDING)
    {
        ReportResult();
        return true;
    }

    if (ctx.observing)
    {
        //LogToConsole("[TestAction] Observe mode, checking monitors...");
        CheckMonitors();
        if (result != TestResult::PENDING)
        {
            return true;
        }
        return false;
    }

    if (pc >= (int)ctx.script.size())
    {
        SetResult(TestResult::PASS, "Script completed without explicit result");
        return true;
    }

    ExecuteCommand(ctx.script[pc++]);
    return true;
}

void TestAction::ExecuteCommand(const std::string& line)
{
    if (line.empty() || line[0] == '#')
        return;

    std::string cmd;
    std::string params;

    size_t spacePos = line.find(' ');
    if (spacePos != std::string::npos)
    {
        cmd = line.substr(0, spacePos);
        params = line.substr(spacePos + 1);
    }
    else
    {
        cmd = line;
        params = "";
    }

    for (const auto& command : commands)
    {
        if (command->Matches(cmd, params))
        {
            std::string error;
            if (!command->Execute(params, bot, ai, ctx, error))
            {
                if (!error.empty())
                    SetResult(TestResult::IMPOSSIBLE, error);
            }
            return;
        }
    }

    LogToConsole("[TestAction] Unknown command, falling through to bot: " + line);
    TellMaster("Executing command: " + line);
    ai->HandleCommand(CHAT_MSG_WHISPER, line, *bot);
}

void TestAction::CheckMonitors()
{
    //LogToConsole("[TestAction] CheckMonitors called, monitor count: " + std::to_string(ctx.monitors.size()));

    for (const auto& monitorStr : ctx.monitors)
    {
        for (const auto& monitor : monitors)
        {
            if (monitor->Matches(monitorStr))
            {
                std::string message;
                TestResult r = monitor->Check(monitorStr, bot, ctx, message);
                if (r != TestResult::PENDING)
                {
                    SetResult(r, message);
                    return;
                }
            }
        }
    }
}

void TestAction::SetResult(TestResult newResult, const std::string& msg)
{
    if (result != TestResult::PENDING)
        return;

    result = newResult;
    ctx.result = newResult;
    ctx.resultMessage = msg;

    std::string resultStr;
    switch (newResult)
    {
        case TestResult::PASS: resultStr = "PASS"; break;
        case TestResult::FAIL: resultStr = "FAIL"; break;
        case TestResult::ABORT: resultStr = "ABORT"; break;
        default: resultStr = "UNKNOWN"; break;
    }

    TellMaster(std::string("TEST ") + resultStr + ": " + msg);
    LogToConsole(std::string("[TestAction] Bot ") + bot->GetName() + " test " + resultStr + ": " + msg);
}

void TestAction::ReportResult()
{
    if (ctx.result == TestResult::PENDING)
        return;

    std::string resultStr;
    switch (ctx.result)
    {
        case TestResult::PASS: resultStr = "PASS"; break;
        case TestResult::FAIL: resultStr = "FAIL"; break;
        case TestResult::ABORT: resultStr = "ABORT"; break;
        case TestResult::IMPOSSIBLE: resultStr = "IMPOSSIBLE"; break;
        default: resultStr = "UNKNOWN"; break;
    }

    time_t now = time(nullptr);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    std::string logLine = "[BOTTEST] " + std::string(timestamp) + " | " +
                          bot->GetName() + " | " + testName + " | " + resultStr + " | " +
                          ctx.resultMessage;
    LogToConsole(logLine);
    LogToFile(logLine);

    RESET_AI_VALUE2(bool, "manual bool", "is running test");

#ifdef GenerateBotTests 
    if (ai->GetHolder())
        ai->GetHolder()->DepositTestResult(testName, resultStr);
#endif

    DeactivateStrategy();

    TellMaster("Test complete: " + resultStr);

    if (sRandomPlayerbotMgr.GetValue(bot, "temporary"))
    {
        std::string logLine = "[BOTTEST] " + std::string(timestamp) + " | " +
            bot->GetName() + " | deleting bot";

        LogToConsole(logLine);
        LogToFile(logLine);

        sRandomPlayerbotMgr.SetValue(bot, "temporary", 0);

        if (ai->GetHolder())
            ai->GetHolder()->DeleteBot(bot->GetObjectGuid(), false);        
    }
}

void TestAction::TellMaster(const std::string& msg)
{
    Player* master = ai->GetMaster();
    if (master)
        ai->TellPlayer(master, "[TEST] " + msg);
    else
        ai->TellPlayer(bot, "[TEST] " + msg);
}

void TestAction::LogToConsole(const std::string& msg)
{
    sLog.outString(msg.c_str());
}

void TestAction::LogToFile(const std::string& msg)
{
    std::ofstream file("bot_test_results.log", std::ios::app);
    if (file.is_open())
    {
        file << msg << std::endl;
        file.close();
    }
}

void TestAction::DeactivateStrategy()
{       
    std::string strategyName = "test::" + testName;
    ai->ChangeStrategy("-" + strategyName, BotState::BOT_STATE_COMBAT);
    ai->ChangeStrategy("-" + strategyName, BotState::BOT_STATE_NON_COMBAT);
            
    ctx.Reset();
    result = TestResult::PENDING;    
    pc = 0;
    testName.clear();
}

void TestAction::RegisterTest(const std::string& name, const std::vector<std::string>& script)
{
    TestRegistry::RegisterTest(name, script);
}

bool TestAction::HasTest(const std::string& name)
{
    return TestRegistry::HasTest(name);
}

std::vector<std::string> TestAction::GetTestScript(const std::string& name)
{
    return TestRegistry::GetTestScript(name);
}

std::vector<std::string> TestAction::GetAvailableTests()
{
    return TestRegistry::GetAvailableTests();
}