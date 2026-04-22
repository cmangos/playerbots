#include "playerbot/playerbot.h"
#include "PartyCommands.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestContext.h"

using namespace ai;

bool HandleSpawnBot::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    return true;
}

bool HandleDespawnBot::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    for (auto& guid : ctx.spawnedBots)
    {
        if (Creature* creature = ai->GetCreature(guid))
            creature->ForcedDespawn();
    }
    ctx.spawnedBots.clear();
    return true;
}

bool HandleFormParty::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    return true;
}