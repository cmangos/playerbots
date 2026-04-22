#pragma once

#include <string>

class Player;
class PlayerbotAI;

namespace ai
{
    class TestContext;

    class TestCommand
    {
    public:
        virtual bool Matches(const std::string& cmd, const std::string& params) const { return cmd == GetName(); }
        virtual bool Execute(const std::string& params, Player* bot,
                         PlayerbotAI* ai, TestContext& ctx, std::string& error) = 0;
        virtual ~TestCommand() = default;
    protected:
        virtual std::string GetName() const = 0;
    };
}