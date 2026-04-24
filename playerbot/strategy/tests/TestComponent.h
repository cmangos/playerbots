#pragma once

#include <string>
#include "TestContext.h"

class Player;
class PlayerbotAI;

namespace ai
{
    class TestContext;

    class TextComponent
    {
    public:
        virtual bool Matches(const std::string& monitorStr) const { return monitorStr.find(GetName()) == 0; }
        uint32 GetNameSize() const { return GetName().size() + 1; }
    protected:
        static TestResult TrySplitOnce(const std::string& input, const std::string& delimiter,
            std::string& left, std::string& right, std::string& message,
            const std::string& componentName, bool allowEmptyRight = false);
        static TestResult TryExtractBetween(const std::string& input, const std::string& beginDelimiter,
            const std::string& endDelimiter, std::string& value, std::string& message,
            const std::string& componentName);
        static TestResult TryParseComparisonValue(const std::string& input, char& op, std::string& value,
            std::string& message, const std::string& componentName);
        static TestResult TryParseUInt32Strict(const std::string& input, uint32& outValue,
            std::string& message, const std::string& componentName);
        static TestResult TryParseUInt64Strict(const std::string& input, uint64& outValue,
            std::string& message, const std::string& componentName);
        static TestResult TryParseFloatStrict(const std::string& input, float& outValue,
            std::string& message, const std::string& componentName);
        virtual std::string GetName() const = 0;
    };

    class TestMonitor : public TextComponent
    {
    public:
        virtual TestResult Check(const std::string& monitorStr, Player* bot, TestContext& ctx, std::string& message) const;
    protected:
        virtual bool IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const = 0;
    };

    class TestCommand : public TextComponent
    {
    public:
        virtual TestResult Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message) = 0;
    };

    class TestRequire : public TestCommand
    {
    public:
    };

    class TestCleanup : public TestCommand
    {
    public:
    };
}