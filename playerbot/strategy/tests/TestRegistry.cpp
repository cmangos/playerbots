#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "playerbot/ChatHelper.h"
#include "Globals/ObjectMgr.h"
#include <regex>
#include <sstream>
#include "playerbot/TravelMgr.h"
#include <playerbot/TravelNode.h>

static std::map<std::string, std::vector<std::string>> sTestRegistry;
static bool sTestsRegistered = false;

static std::map<std::string, GuidPosition> sNamedTestLocations;

struct TeleLoc
{
    std::string name;
    GuidPosition pos;
};

static std::vector<TeleLoc> sTeleLocations;

static void InitTestLocations()
{
    sNamedTestLocations["ironforge_outside"] = GuidPosition(ObjectGuid(), WorldPosition(0, -5150.0f, -856.0f, 508.4f));
    sNamedTestLocations["zg_boss1_room"] = GuidPosition(ObjectGuid(), WorldPosition(309, -12276.0f, -1399.0f, 130.9f));
}

static void InitTeleLocations()
{
    for (auto const& [id, tele] : sObjectMgr.GetGameTeleMap())
    {
        GuidPosition pos(ObjectGuid(), WorldPosition(tele.mapId, tele.position_x, tele.position_y, tele.position_z, tele.orientation));
        sTeleLocations.push_back({ tele.name, pos });
    }
}

static void EnsureLocationsInit()
{
    static bool initialized = false;
    if (!initialized)
    {
        InitTestLocations();
        InitTeleLocations();
        initialized = true;
    }
}

namespace
{
    using ScenarioParams = std::map<std::string, std::string>;

    std::string NormalizeLocationKey(std::string value)
    {
        const size_t begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
            return "";

        const size_t end = value.find_last_not_of(" \t\r\n");
        value = value.substr(begin, end - begin + 1);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value;
    }

    uint32 ParseUintOrDefault(const std::string& value, uint32 fallback)
    {
        if (value.empty())
            return fallback;

        for (char c : value)
            if (!std::isdigit(static_cast<unsigned char>(c)))
                return fallback;

        return static_cast<uint32>(std::stoul(value));
    }

    uint32 ParseMGroupSize(const std::string& line)
    {
        static const uint32 defaultGroupSize = 5;
        std::istringstream iss(line);
        std::string token;

        while (iss >> token)
        {
            if (token.find("size=") != 0)
                continue;

            return ParseUintOrDefault(token.substr(std::string("size=").length()), defaultGroupSize);
        }

        return defaultGroupSize;
    }
}

void TestRegistry::GenerateMovementTestsImpl(int maxTests, float minDist, float maxDist)
{
    if (sTeleLocations.size() < 2)
        return;

    std::string gmInvisible = "gm visible off";
    std::string gmVisible = "cleanup gm visible on";
    std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    std::string needAboveGRound = "monitor underground => fail \"Bot is underground at <current position>\"";
    std::string needCanReachNode = "monitor can not reach node => fail \"Bot cannot reach travel network at <current position>\"";

    int count = 0;
    for (size_t i = 0; i < sTeleLocations.size() && count < maxTests; ++i)
    {
        for (size_t j = 0; j < sTeleLocations.size() && count < maxTests; ++j)
        {
            if (i == j)
                continue;

            WorldPosition startPos = sTeleLocations[i].pos;
            WorldPosition endPos = sTeleLocations[j].pos;

            if (!startPos || !endPos)
                continue;

            if (startPos.isBg() || endPos.isBg())
                continue;

            if (startPos.isDungeon() && startPos.getMapEntry()->IsRaid())
                continue;

            if (endPos.isDungeon() && endPos.getMapEntry()->IsRaid())
                continue;

            float dist = startPos.distance(endPos);

            if (dist < minDist || dist > maxDist)
                continue;

            std::string prefix;
            int timeoutSecs = dist + 600;
            if (dist < 500)
            {
                prefix = "short";
            }
            else if (dist < 2000)
            {
                prefix = "medium";
            }
            else
            {
                prefix = "far";
            }

            std::string startName = sTeleLocations[i].name;
            std::transform(startName.begin(), startName.end(), startName.begin(), ::tolower);
            std::replace(startName.begin(), startName.end(), '\'', '_');
            std::string endName = sTeleLocations[j].name;
            std::transform(endName.begin(), endName.end(), endName.begin(), ::tolower);
            std::replace(endName.begin(), endName.end(), '\'', '_');


            std::ostringstream testName;
            testName << "movement_" << prefix << "_" << startName << "_" << endName;

            std::ostringstream timeout;
            timeout << "monitor time > " << timeoutSecs << " => fail \"Timeout: bot did not reach destination after <time elapsed> (traveled <distance traveled> / wanted <distance wanted>)\"";

            std::ostringstream reachCheck;
            reachCheck << "monitor distance to " << endName << " < 100 => pass \"Bot arrived at " << endName << " after <time elapsed>\"";

            std::vector<std::string> script = {
                gmInvisible,
                needAlive,
                needAboveGRound,
                needCanReachNode,
                timeout.str(),
                reachCheck.str(),
                "teleport " + startName,
                "set destination " + endName,
                "observe",
                gmVisible
            };

            RegisterTest(testName.str(), script);
            count++;

            if ((j % 10) == 0 && i < sTeleLocations.size())
                i++;
        }
    }
}

using namespace ai;

std::string TestRegistry::GetBotCreationRequirement(const std::string& testName)
{
    std::vector<std::string> script = GetTestScript(testName);

    for (const std::string& line : script)
    {
        if (line.find("require bot is") != 0)
            continue;

        std::string params = line.substr(std::string("require bot is ").length());
        return params;
    }

    return "";
}

uint32 TestRegistry::ExpectedBotSpawnCount(const std::string& testName)
{
    std::vector<std::string> script = GetTestScript(testName);

    uint32 expectedBots = 1;

    for (const std::string& line : script)
    {
        if (line.find("spawn") == 0)
        {
            expectedBots++;
            continue;
        }

        if (line.find("mgroup") == 0)
        {
            expectedBots = std::max(expectedBots, ParseMGroupSize(line));
        }
    }

    return expectedBots;
}

void TestRegistry::RegisterTest(const std::string& name, const std::vector<std::string>& script)
{
    sTestRegistry[name] = script;
}

void TestRegistry::RegisterNamedLocation(const std::string& name, const GuidPosition& pos)
{
    EnsureLocationsInit();

    std::string key = NormalizeLocationKey(name);
    sNamedTestLocations[key] = pos;
}

bool TestRegistry::HasTest(const std::string& name)
{
    EnsureTestsRegistered();
    return sTestRegistry.find(name) != sTestRegistry.end();
}

std::vector<std::string> TestRegistry::GetTestScript(const std::string& name)
{
    EnsureTestsRegistered();
    auto it = sTestRegistry.find(name);
    if (it != sTestRegistry.end())
        return it->second;
    return {};
}

std::vector<std::string> TestRegistry::GetAvailableTests()
{
    EnsureTestsRegistered();
    std::vector<std::string> tests;
    for (const auto& pair : sTestRegistry)
        tests.push_back(pair.first);
    return tests;
}

void TestRegistry::GenerateMovementTests(int maxTests, float minDist, float maxDist)
{
    EnsureLocationsInit();
    GenerateMovementTestsImpl(maxTests, minDist, maxDist);
}

bool TestRegistry::LookupNamedLocation(const std::string& name, GuidPosition& out)
{
    EnsureLocationsInit();

    std::string lowerName = NormalizeLocationKey(name);

    auto it = sNamedTestLocations.find(lowerName);
    if (it != sNamedTestLocations.end())
    {
        out = it->second;
        return true;
    }

    for (const auto& entry : sNamedTestLocations)
    {
        if (NormalizeLocationKey(entry.first) != lowerName)
            continue;

        out = entry.second;
        return true;
    }

    return false;
}

bool TestRegistry::ParseLocation(const std::string& str, GuidPosition& out)
{
    std::string normalized = NormalizeLocationKey(str);
    if (normalized.empty())
        return false;

    if (LookupNamedLocation(normalized, out))
        return true;

    out = GuidPosition(GuidPosition::CreationMask::UNKNOWN, normalized);

    if (out)
        return true;

    return false;
}

void TestRegistry::EnsureTestsRegistered()
{
    if (sTestsRegistered)
        return;

    sTestsRegistered = true;

    RegisterMoveTests();
    RegisterSpawnTests();
    RegisterRandomizeTests();
    RegisterInstanceTests();
    RegisterBankTests();
    RegisterQuestDkStartTests();
}

void TestRegistry::EnsureLocationsInit()
{
    ::EnsureLocationsInit();
}