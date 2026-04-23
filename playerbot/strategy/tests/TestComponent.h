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