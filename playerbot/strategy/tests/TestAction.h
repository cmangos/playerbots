#pragma once

#include "playerbot/strategy/Action.h"
#include "TestContext.h"
#include "TestComponent.h"
#include "BossFocusManager.h"
#include <vector>
#include <memory>

struct WorldLocation;

namespace ai
{
    class TestAction : public Action
    {
    public:
        TestAction(PlayerbotAI* ai, std::string name = "test");
        virtual bool Execute(Event& event) override;
        virtual bool isUseful() override { return true; }
        virtual bool isPossible() override { return bot != nullptr; }
        virtual void Reset() override { ctx.Reset(); }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "test"; }
        virtual std::string GetHelpDescription()
        {
            return "Executes a live integration test script on the bot.\n"
                   "Usage: test <testName>\n"
                   "The bot will run the scripted test and report pass/fail.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif

        static void RegisterTest(const std::string& name, const std::vector<std::string>& script);
        static bool HasTest(const std::string& name);
        static std::vector<std::string> GetTestScript(const std::string& name);
        static std::vector<std::string> GetAvailableTests();
    private:
        void RegisterCommands();
        void RegisterMonitors();
        TestResult ExecuteCommand(const std::string& line, std::string& message);
        void CheckMonitors();
        void RunCleanup();
        void SetResult(TestResult result, const std::string& message);
        void ReportResult();
        void TellMaster(const std::string& msg);
        void LogToConsole(const std::string& msg);
        void LogToFile(const std::string& msg);
        void DeactivateStrategy();

        TestResult GetResult() const { return ctx.result; }
        bool IsObserving() const { return ctx.observing; }

    private:
        std::vector<std::unique_ptr<TestMonitor>> monitors;
        std::vector<std::unique_ptr<TestCommand>> commands;
        std::unique_ptr<BossFocusManager> bossFocusMgr;
        
        TestContext ctx;
   };
}