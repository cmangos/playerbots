#pragma once

#include <string>
#include <vector>
#include <map>

namespace ai
{
    class TestRegistry
    {
    public:
        static void RegisterMoveTests();
        static void RegisterSpawnTests();
        static void RegisterRandomizeTests();
        static void RegisterInstanceTests();
        static void RegisterBankTests();

        static void RegisterTest(const std::string& name, const std::vector<std::string>& script);
        static void RegisterNamedLocation(const std::string& name, const GuidPosition& pos);
        static bool HasTest(const std::string& name);
        static std::vector<std::string> GetTestScript(const std::string& name);
        static std::vector<std::string> GetAvailableTests();
        static void GenerateMovementTests(int maxTests, float minDist, float maxDist);
        static void GenerateBossWalkTest();
        static void GenerateBossEncounterTest();
        static std::string GetBotCreationRequirement(const std::string& testName);
        static uint32 ExpectedBotSpawnCount(const std::string& testName);

        static bool ParseLocation(const std::string& str, GuidPosition& out);
        static bool LookupNamedLocation(const std::string& name, GuidPosition& out);

    private:
        static std::string ApplyScenarioParams(const std::string& line, const std::map<std::string, std::string>& params);
        static std::vector<std::string> ApplyScenarioParams(const std::vector<std::string>& script, const std::map<std::string, std::string>& params);
        static void GenerateMovementTestsImpl(int maxTests, float minDist, float maxDist);
        static void EnsureTestsRegistered();
        static void EnsureLocationsInit();
    };
}