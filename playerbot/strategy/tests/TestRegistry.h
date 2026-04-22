#pragma once

#include <string>
#include <vector>
#include <map>

namespace ai
{
    class TestRegistry
    {
    public:
        static void RegisterTest(const std::string& name, const std::vector<std::string>& script);
        static bool HasTest(const std::string& name);
        static std::vector<std::string> GetTestScript(const std::string& name);
        static std::vector<std::string> GetAvailableTests();

        static bool ParseLocation(const std::string& str, GuidPosition& out);
        static bool LookupNamedLocation(const std::string& name, GuidPosition& out);

    private:
        static void EnsureTestsRegistered();
        static void EnsureLocationsInit();
    };
}