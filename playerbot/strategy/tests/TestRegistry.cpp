#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "Globals/ObjectMgr.h"

static std::map<std::string, std::vector<std::string>> sTestRegistry;
static bool sTestsRegistered = false;

static std::map<std::string, GuidPosition> sTestLocations;

struct TeleLoc
{
    std::string name;
    GuidPosition pos;
};

static std::vector<TeleLoc> sTeleLocations;

static void InitTestLocations()
{
    sTestLocations["ironforge_outside"] = GuidPosition(ObjectGuid(), WorldPosition(0, -5150.0f, -856.0f, 508.4f));
    sTestLocations["zg_entrance"] = GuidPosition(ObjectGuid(), WorldPosition(309, -11917.0f, -1233.0f, 92.0f));
    sTestLocations["zg_boss1_room"] = GuidPosition(ObjectGuid(), WorldPosition(309, -11900.0f, -1650.0f, 92.0f));
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

void TestRegistry::GenerateMovementTestsImpl(int maxTests, float minDist, float maxDist)
{
    if (sTeleLocations.size() < 2)
        return;

    std::string gmInvisible = "gm visible off";
    std::string gmVisible = "gm visible on";
    std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    std::string needAboveGRound = "monitor underground => fail \"Bot is underground at <current position>\"";
    std::string needCanReachNode = "monitor can reach node => fail \"Bot cannot reach travel network at <current position>\"";

    int count = 0;
    for (size_t i = 0; i < sTeleLocations.size() && count < maxTests; ++i)
    {
        for (size_t j = 0; j < sTeleLocations.size() && count < maxTests; ++j)
        {
            if (i == j)
                continue;

            float dist = sTeleLocations[i].pos.distance(sTeleLocations[j].pos);
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
            startName.erase(remove(startName.begin(), startName.end(), '\''), startName.end());
            std::string endName = sTeleLocations[j].name;
            std::transform(endName.begin(), endName.end(), endName.begin(), ::tolower);
            endName.erase(remove(endName.begin(), endName.end(), '\''), endName.end());

            std::ostringstream testName;
            testName << "movement_" << prefix << "_" << startName << "_" << endName;

            std::ostringstream timeout;
            timeout << "monitor time > " << timeoutSecs << " => fail \"Timeout: bot did not reach destination after <time elapsed>\"";

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

void TestRegistry::RegisterTest(const std::string& name, const std::vector<std::string>& script)
{
    sTestRegistry[name] = script;
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

    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

    auto it = sTestLocations.find(lowerName);
    if (it != sTestLocations.end())
    {
        out = it->second;
        return true;
    }
    return false;
}

bool TestRegistry::ParseLocation(const std::string& str, GuidPosition& out)
{
    if (LookupNamedLocation(str, out))
        return true;

    out = GuidPosition(GuidPosition::CreationMask::UNKNOWN, str);

    if (out)
        return true;

    return false;
}

void TestRegistry::EnsureTestsRegistered()
{
    if (sTestsRegistered)
        return;

    sTestsRegistered = true;

    static std::string gmInvisible = "gm visible off";
    static std::string gmVisible = "gm visible on";

    static std::string needAlliance = "monitor faction horde => abort \"Bot needs to be alliance\"";
    static std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";

    static std::string Timeout2Min = "monitor time > 120 => fail \"Timeout: bot did not reach destination\"";
    static std::string Timeout10Min = "monitor time > 600 => fail \"Timeout: bot did not reach destination\"";

    //Movement tests
    RegisterTest("movement_walk_short_inside_ironforge", {gmInvisible, needAlive, Timeout2Min, "monitor distance to ironforge < 50 => pass \"Bot reached Ironforge gate\"", "teleport ironforge_outside", "set destination ironforge", "observe", gmVisible});

    RegisterTest("movement_walk_long_coldridge_ironforge", {gmInvisible, needAlive, Timeout10Min, "monitor distance to ironforge < 100 => pass \"Bot arrived at Ironforge\"", "teleport coldridge", "set destination ironforge", "observe", gmVisible});

    RegisterTest("movement_fly_short_ironforge_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport ironforge", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_darkshire_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport darkshire", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_westfall_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport westfall", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_loch_modan_to_ironforge", {gmInvisible, needAlive, Timeout10Min, "monitor distance to ironforge < 100 => pass \"Bot arrived at Ironforge\"", "teleport loch modan", "set destination ironforge", "observe", gmVisible});

    RegisterTest("movement_walk_to_zg_entrance", {gmInvisible, needAlive, Timeout2Min, "monitor distance to zg_entrance < 50 => pass \"Bot reached ZG entrance\"", "teleport elwynn", "set destination zg_entrance", "observe", gmVisible});

    RegisterTest("movement_fly_stormwind_to_orgrimmar", {gmInvisible, needAlive, Timeout10Min, "monitor distance to orgrimmar < 100 => pass \"Bot arrived at Orgrimmar\"", "teleport stormwind", "set destination orgrimmar", "observe", gmVisible});

    RegisterTest("movement_fly_ironforge_to_orgrimmar", {gmInvisible, needAlive, Timeout10Min, "monitor distance to orgrimmar < 100 => pass \"Bot arrived at Orgrimmar\"", "teleport ironforge", "set destination orgrimmar", "observe", gmVisible});

    RegisterTest("movement_walk_long_redridge_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport redridge", "set destination stormwind", "observe", gmVisible});

    RegisterTest("movement_walk_long_duskwood_to_stormwind", {gmInvisible, needAlive, Timeout10Min, "monitor distance to stormwind < 100 => pass \"Bot arrived at Stormwind\"", "teleport duskwood", "set destination stormwind", "observe", gmVisible});

    GenerateMovementTests(1000, 5.0f, 100000.0f);
}

void TestRegistry::EnsureLocationsInit()
{
    ::EnsureLocationsInit();
}