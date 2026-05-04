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
#include "MonitorCombat.h"
#include "MonitorState.h"
#include "CommandSetup.h"
#include "CommandParty.h"
#include "CommandFlow.h"
#include "CommandDebug.h"
#include "CleanupParty.h"
#include "RequireState.h"

#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include "MonitorMovement.h"
#include "BossFocusManager.h"

using namespace ai;

TestAction::TestAction(PlayerbotAI* ai, std::string name)
    : Action(ai, name, 1), ctx(), bossFocusMgr(std::make_unique<BossFocusManager>(bot, ai, ctx))
{
    RegisterCommands();
    RegisterMonitors();
    commands.push_back(std::make_unique<RequireEquip>());
    commands.push_back(std::make_unique<CleanupParty>());
    TestRegistry::GetAvailableTests();
}

void TestAction::RegisterCommands()
{
    commands.push_back(std::make_unique<RequireBotIs>());
    commands.push_back(std::make_unique<CommandSetupTeleportGroup>());
    commands.push_back(std::make_unique<CommandSetupTeleport>());
    commands.push_back(std::make_unique<CommandSetupGM>());
    commands.push_back(std::make_unique<CommandSetupSetDestination>());
    commands.push_back(std::make_unique<CommandSetupPull>());
    commands.push_back(std::make_unique<CommandSetupGiveItem>());
    commands.push_back(std::make_unique<CommandSetupEquipItem>());
    commands.push_back(std::make_unique<CommandSetupClearMobs>());
    commands.push_back(std::make_unique<CommandPartySpawnBot>());
    commands.push_back(std::make_unique<CommandPartyDespawnBot>());
    commands.push_back(std::make_unique<CommandPartyForm>());
    commands.push_back(std::make_unique<CommandPartySpawnGroup>());
    commands.push_back(std::make_unique<CommandFlowObserve>());
    commands.push_back(std::make_unique<CommandFlowMonitor>());
    commands.push_back(std::make_unique<CommandFlowWait>());
    commands.push_back(std::make_unique<CommandFlowWaitDestination>());
    commands.push_back(std::make_unique<CommandFlowRepeat>());
    commands.push_back(std::make_unique<CommandSetValue>());
    commands.push_back(std::make_unique<CommandDebug>());
    commands.push_back(std::make_unique<CommandRecord>());
    commands.push_back(std::make_unique<CommandRead>());
}

void TestAction::RegisterMonitors()
{
    monitors.push_back(std::make_unique<MonitorStateDead>());
    monitors.push_back(std::make_unique<MonitorStateTime>());
    monitors.push_back(std::make_unique<MonitorCombatHp>());
    monitors.push_back(std::make_unique<MonitorMovementDistance>());
    monitors.push_back(std::make_unique<MonitorNotOnMap>());
    monitors.push_back(std::make_unique<MonitorMovementUnderground>());
    monitors.push_back(std::make_unique<MonitorMovementCanNotReachNodes>());
    monitors.push_back(std::make_unique<MonitorMovementSpeed>());
    monitors.push_back(std::make_unique<MonitorMovementSpawnDistance>());
    monitors.push_back(std::make_unique<MonitorCombatMob>());
    monitors.push_back(std::make_unique<MonitorCombatDeadMobs>());
    monitors.push_back(std::make_unique<MonitorCombatPartyWiped>());
    monitors.push_back(std::make_unique<MonitorStateFaction>());
    monitors.push_back(std::make_unique<MonitorStateGroupSize>());
    monitors.push_back(std::make_unique<MonitorStateLootGuid>());
    monitors.push_back(std::make_unique<MonitorStateStarterGearCount>());
    monitors.push_back(std::make_unique<MonitorStateEquipQuality>());
    monitors.push_back(std::make_unique<MonitorStateAreaLevelDiff>());
    monitors.push_back(std::make_unique<MonitorAiValue>());
    monitors.push_back(std::make_unique<MonitorOutgoingMessage>());
}

bool TestAction::Execute(Event& event)
{
    Player* requester = event.getOwner();
    if (!requester)
        requester = GetMaster();
    if (!requester)
        requester = bot;

    std::string param = event.getParam();

    //LogToConsole("[TestAction] Execute called with param: " + param + " step" + std::to_string(pc));

    std::string testParam = param;

    if (ctx.result == TestResult::PENDING && !ctx.observing && ctx.pc == 0)
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

            ctx.result = TestResult::IMPOSSIBLE;
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

            ctx.result = TestResult::IMPOSSIBLE;
            ReportResult();
            return true;
        }

        ctx.testName = testParam;
        ctx.script = TestRegistry::GetTestScript(ctx.testName);
        ctx.testStartTime = WorldTimer::getMSTime();
        if (bot->IsInWorld())
            ctx.testStartPosition = WorldPosition(bot);

        TellMaster(std::string("Starting test: ") + ctx.testName);
        LogToConsole(std::string("[TestAction] Bot ") + bot->GetName() + " starting test: " + ctx.testName);
    }

    SET_AI_VALUE2(bool, "manual bool", "is running test", true);

    if (ctx.result != TestResult::PENDING)
    {
        RunCleanup();
        ReportResult();
        return true;
    }

    if (ctx.observing)
    {
        bossFocusMgr->Update();
        CheckMonitors();
        if (ctx.result != TestResult::PENDING)
        {
            return true;
        }
        return false;
    }

    if (ctx.pc >= (int)ctx.script.size())
    {
        RunCleanup();
        SetResult(TestResult::PASS, "Script completed without explicit result");
        return true;
    }

    std::string message;
    TestResult commandResult = ExecuteCommand(ctx.script[ctx.pc], message);

    // Log command execution for boss tests
    if (ctx.testName.find("scenario_boss") != std::string::npos)
    {
        std::string result = (commandResult == TestResult::PASS ? "PASS" :
                commandResult == TestResult::FAIL               ? "FAIL" :
                commandResult == TestResult::ABORT              ? "ABORT" :
                commandResult == TestResult::IMPOSSIBLE         ? "IMPOSSIBLE" :
                                                                  "PENDING");
        sLog.outString("[TestAction] Bot %s cmd[%d]: %s => %s %s", bot->GetName(), ctx.pc,
            ctx.script[ctx.pc].c_str(), result.c_str(), message.c_str());
    }

    if (ctx.debug)
    {
        std::string result = (commandResult == TestResult::PASS ? "PASS" :
                commandResult == TestResult::FAIL               ? "FAIL" :
                commandResult == TestResult::ABORT              ? "ABORT" :
                commandResult == TestResult::IMPOSSIBLE         ? "IMPOSSIBLE" :
                                                                  "PENDING");

        ai->TellPlayer(requester, std::string("[TestAction] Executed command: ") + ctx.script[ctx.pc] + " => " + result + (message.empty() ? "" : (" (" + message + ")")));

        sLog.outString("[TestAction] Bot %s cmd[%d]: %s => %s %s", bot->GetName(), ctx.pc, ctx.script[ctx.pc].c_str(), result.c_str(), message.c_str());
    }

    if (commandResult == TestResult::PASS)
    {
        ctx.pc++;
    }
    else if (commandResult == TestResult::PENDING)
    {
        return false;
    }
    else
    {
        RunCleanup();
        SetResult(commandResult, message.empty() ? "Command " + ctx.script[ctx.pc] + " failed" : message);
    }

    return true;
}

TestResult TestAction::ExecuteCommand(const std::string& line, std::string& message)
{
    if (line.empty() || line[0] == '#')
        return TestResult::PASS;

    for (const auto& command : commands)
    {
        if (command->Matches(line))
        {
            std::string params;
            if (line.size() > command->GetNameSize())
                params = line.substr(command->GetNameSize());

            return command->Execute(params, bot, ai, ctx, message);
        }
    }

    if (line[0] == '.')
        if (ChatHandler(bot).ParseCommands(line.c_str()))
            return TestResult::PASS;

    ExternalEventHelper helper(context);
    if (helper.ParseChatCommand(line, bot))  
        return TestResult::PASS;       

    return TestResult::FAIL;
}

void TestAction::RunCleanup()
{   
    for (size_t i = static_cast<size_t>(std::max(0, ctx.pc)); i < ctx.script.size(); ++i)
    {
        std::string message;

        if (!dynamic_cast<TestCleanup*>(commands[i].get()))
            continue;

        TestResult commandResult = ExecuteCommand(ctx.script[ctx.pc], message);        
    }
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
                std::string message, params;

                if (monitorStr.size() > monitor->GetNameSize())
                    params = monitorStr.substr(monitor->GetNameSize());

                TestResult r = monitor->Check(params, bot, ctx, message);
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
    if (ctx.result != TestResult::PENDING)
        return;

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
        bot->GetName() + " | " + ctx.testName + " | " + resultStr + " | " +
                          ctx.resultMessage;
    LogToConsole(logLine);
    LogToFile(logLine);

    RESET_AI_VALUE2(bool, "manual bool", "is running test");

#ifdef GenerateBotTests 
    if (ai->GetHolder())
        ai->GetHolder()->DepositTestResult(ctx.testName, resultStr);
#endif

    DeactivateStrategy();

    TellMaster("Test complete: " + resultStr);

    if (sRandomPlayerbotMgr.GetValue(bot, "temporary"))
    {
        std::string logLine = "[BOTTEST] " + std::string(timestamp) + " | " +
            bot->GetName() + " | deleting bot";

        LogToConsole(logLine);

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
    sPlayerbotAIConfig.log("bot_test_results.log", msg.c_str());
}

void TestAction::DeactivateStrategy()
{       
    std::string strategyName = "test::" + ctx.testName;
    ai->ChangeStrategy("-" + strategyName, BotState::BOT_STATE_ALL);
            
    ctx.Reset();
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