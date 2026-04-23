#include "playerbot/playerbot.h"
#include "CleanupParty.h"

using namespace ai;

// =====================================================
// CleanupParty implementation
// =====================================================
TestResult CleanupParty::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    // Default cleanup: despawn all spawned bots
    for (auto& guid : ctx.spawnedBots)
    {
        if (guid.IsPlayer())
        {
            if (ai->GetHolder())
                ai->GetHolder()->DeleteBot(guid, false);
        }
        else if (Creature* creature = ai->GetCreature(guid))
        {
            creature->ForcedDespawn();
        }
    }
    ctx.spawnedBots.clear();
    ctx.observing = false;
    return TestResult::PASS;
}