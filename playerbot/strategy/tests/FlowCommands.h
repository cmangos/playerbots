#pragma once

#include "TestCommand.h"

namespace ai
{
    class HandleObserve : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "observe"; }
    };

    class HandleCleanup : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "cleanup"; }
    };

    class HandleAssert : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "assert"; }
    };

    class HandlePreconditions : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "preconditions"; }
    };

    class HandleMonitor : public TestCommand
    {
    public:
        bool Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error) override;
    protected:
        std::string GetName() const override { return "monitor"; }
    };
}