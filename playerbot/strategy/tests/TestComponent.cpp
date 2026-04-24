#include "TestComponent.h"
#include "playerbot/WorldPosition.h"
#include "playerbot/PlayerbotTextMgr.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <limits>

using namespace ai;

// =====================================================
// TestMonitor implementation
// =====================================================

namespace
{
    bool IsWhitespaceOnly(const std::string& value)
    {
        return value.find_first_not_of(" \t\r\n") == std::string::npos;
    }

    std::string Trim(const std::string& value)
    {
        const size_t begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
            return "";

        const size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(begin, end - begin + 1);
    }

    bool HasOnlyTrailingWhitespace(const char* cursor)
    {
        while (*cursor)
        {
            if (*cursor != ' ' && *cursor != '\t' && *cursor != '\r' && *cursor != '\n')
                return false;
            ++cursor;
        }

        return true;
    }

    uint32 CountNearbyDeadMobs(Player* bot, float radius)
    {
        std::list<Creature*> creatures;
        MaNGOS::AnyUnitInObjectRangeCheck checker(bot, radius);
        MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(creatures, checker);
        Cell::VisitWorldObjects(bot, searcher, radius);

        uint32 deadCount = 0;
        for (Creature* creature : creatures)
        {
            if (!creature->IsAlive())
                ++deadCount;
        }

        return deadCount;
    }
}

TestResult TextComponent::TrySplitOnce(const std::string& input, const std::string& delimiter,
    std::string& left, std::string& right, std::string& message,
    const std::string& componentName, bool allowEmptyRight)
{
    const size_t pos = input.find(delimiter);
    if (pos == std::string::npos)
    {
        message = "Invalid format in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    left = input.substr(0, pos);
    right = input.substr(pos + delimiter.size());
    if (!allowEmptyRight && right.empty())
    {
        message = "Invalid format in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    return TestResult::PASS;
}

TestResult TextComponent::TryExtractBetween(const std::string& input, const std::string& beginDelimiter,
    const std::string& endDelimiter, std::string& value, std::string& message,
    const std::string& componentName)
{
    const size_t beginPos = input.find(beginDelimiter);
    if (beginPos == std::string::npos)
    {
        message = "Invalid format in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    const size_t contentStart = beginPos + beginDelimiter.size();
    const size_t endPos = input.find(endDelimiter, contentStart);
    if (endPos == std::string::npos || endPos < contentStart)
    {
        message = "Invalid format in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    value = input.substr(contentStart, endPos - contentStart);
    return TestResult::PASS;
}

TestResult TextComponent::TryParseComparisonValue(const std::string& input, char& op, std::string& value,
    std::string& message, const std::string& componentName)
{
    std::string leftSide;
    std::string rightSide;
    const TestResult splitResult = TrySplitOnce(input, "=>", leftSide, rightSide, message, componentName, true);
    if (splitResult != TestResult::PASS)
        return splitResult;

    const size_t ltPos = leftSide.find('<');
    const size_t gtPos = leftSide.find('>');

    if (ltPos == std::string::npos && gtPos == std::string::npos)
    {
        message = "Invalid format in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    size_t opPos = std::string::npos;
    if (ltPos != std::string::npos && (gtPos == std::string::npos || ltPos < gtPos))
    {
        op = '<';
        opPos = ltPos;
    }
    else
    {
        op = '>';
        opPos = gtPos;
    }

    value = leftSide.substr(opPos + 1);
    if (value.empty() || IsWhitespaceOnly(value))
    {
        message = "Invalid format in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    return TestResult::PASS;
}

TestResult TextComponent::TryParseUInt32Strict(const std::string& input, uint32& outValue,
    std::string& message, const std::string& componentName)
{
    errno = 0;
    char* endPtr = nullptr;
    const unsigned long parsed = std::strtoul(input.c_str(), &endPtr, 10);
    if (endPtr == input.c_str() || !HasOnlyTrailingWhitespace(endPtr) || errno == ERANGE ||
        parsed > std::numeric_limits<uint32>::max())
    {
        message = "Invalid numeric value in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    outValue = static_cast<uint32>(parsed);
    return TestResult::PASS;
}

TestResult TextComponent::TryParseUInt64Strict(const std::string& input, uint64& outValue,
    std::string& message, const std::string& componentName)
{
    errno = 0;
    char* endPtr = nullptr;
    const unsigned long long parsed = std::strtoull(input.c_str(), &endPtr, 10);
    if (endPtr == input.c_str() || !HasOnlyTrailingWhitespace(endPtr) || errno == ERANGE)
    {
        message = "Invalid numeric value in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    outValue = static_cast<uint64>(parsed);
    return TestResult::PASS;
}

TestResult TextComponent::TryParseFloatStrict(const std::string& input, float& outValue,
    std::string& message, const std::string& componentName)
{
    errno = 0;
    char* endPtr = nullptr;
    const float parsed = std::strtof(input.c_str(), &endPtr);
    if (endPtr == input.c_str() || !HasOnlyTrailingWhitespace(endPtr) || errno == ERANGE || !std::isfinite(parsed))
    {
        message = "Invalid numeric value in " + componentName + ": " + input;
        return TestResult::IMPOSSIBLE;
    }

    outValue = parsed;
    return TestResult::PASS;
}

TestResult TestMonitor::Check(const std::string& monitorStr, Player* bot, TestContext& ctx, std::string& message) const
{
    if (IsConditionMet(monitorStr, bot, ctx))
    {
        std::string leftSide;
        std::string rightSide;
        TestResult splitResult = TrySplitOnce(monitorStr, "=>", leftSide, rightSide, message, GetName());
        if (splitResult != TestResult::PASS)
            return splitResult;

        const size_t quoteStart = rightSide.find('"');
        if (quoteStart == std::string::npos)
        {
            message = "Invalid format in " + GetName() + ": " + monitorStr;
            return TestResult::IMPOSSIBLE;
        }

        std::string quotedMessage;
        splitResult = TryExtractBetween(rightSide.substr(quoteStart), "\"", "\"", quotedMessage, message, GetName());
        if (splitResult != TestResult::PASS)
            return splitResult;

        message = quotedMessage;

        // Replace dynamic placeholders in message
        std::map<std::string, std::string> placeholders;
        if (bot->IsInWorld())
        {
            WorldPosition pos(bot);
            placeholders["<current position>"] = pos.print(2, true) + " m" + std::to_string(pos.getMapId());

            if (ctx.testStartPosition)
                placeholders["<distance traveled>"] = std::to_string(static_cast<uint32>(ctx.testStartPosition.distance(pos))) + "m";

            placeholders["<mobs killed>"] = std::to_string(CountNearbyDeadMobs(bot, 120.0f));
        }

        if (ctx.testStartPosition && ctx.destinationPosition)
            placeholders["<distance wanted>"] = std::to_string(static_cast<uint32>(ctx.testStartPosition.distance(WorldPosition(ctx.destinationPosition)))) + "m";

        placeholders["<time elapsed>"] = std::to_string((WorldTimer::getMSTime() - ctx.testStartTime) / 1000) + "s";
        
        if (!placeholders.empty())
        {
            PlayerbotTextMgr::ReplacePlaceholders(message, placeholders);
        }

        std::string resultType = Trim(rightSide.substr(0, quoteStart));

        if (resultType == "pass")
            return TestResult::PASS;
        else if (resultType == "fail")
            return TestResult::FAIL;
        else if (resultType == "impossible")
            return TestResult::IMPOSSIBLE;
        else if (resultType == "abort")
            return TestResult::ABORT;
        else
        {
            message = "Invalid format in " + GetName() + ": " + monitorStr;
            return TestResult::IMPOSSIBLE;
        }
    }
    
    return TestResult::PENDING;
}