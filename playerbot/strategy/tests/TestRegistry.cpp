#include "playerbot/playerbot.h"
#include "TestRegistry.h"

static std::map<std::string, std::vector<std::string>> sTestRegistry;
static bool sTestsRegistered = false;

static std::map<std::string, GuidPosition> sTestLocations;

static void InitTestLocations()
{
    sTestLocations["ironforge_outside"] = GuidPosition(ObjectGuid(), WorldPosition(0, -5150.0f, -856.0f, 508.4f));
    sTestLocations["zg_entrance"] = GuidPosition(ObjectGuid(), WorldPosition(309, -11917.0f, -1233.0f, 92.0f));
    sTestLocations["zg_boss1_room"] = GuidPosition(ObjectGuid(), WorldPosition(309, -11900.0f, -1650.0f, 92.0f));
}

static void EnsureLocationsInit()
{
    static bool initialized = false;
    if (!initialized)
    {
        InitTestLocations();
        initialized = true;
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
}

void TestRegistry::EnsureLocationsInit()
{
    ::EnsureLocationsInit();
}