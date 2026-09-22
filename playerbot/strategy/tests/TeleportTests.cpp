#include "playerbot/playerbot.h"
#include "TeleportTests.h"
#include "TestRegistry.h"
#include "playerbot/PlayerbotAI.h"

using namespace ai;

namespace
{
    // The bot created by the "spawn" command (ctx.spawnedBots is filled by CommandPartySpawnBot).
    // Returns nullptr while it is still entering the world.
    Player* GetSpawnedBot(TestContext& ctx)
    {
        if (ctx.spawnedBots.empty())
            return nullptr;

        Player* spawned = sObjectMgr.GetPlayer(ctx.spawnedBots.front());
        if (!spawned || !spawned->IsInWorld())
            return nullptr;

        return spawned;
    }

    bool ExpectsRejection(const std::string& params)
    {
        return params.find("expect rejected") != std::string::npos;
    }

    // State of the spawned bot, so a refused request says *why* it was refused.
    std::string DescribeTarget(Player* target)
    {
        std::ostringstream out;
        out << "alive=" << (target->IsAlive() ? 1 : 0)
            << " inCombat=" << (target->IsInCombat() ? 1 : 0)
            << " inWorld=" << (target->IsInWorld() ? 1 : 0)
            << " session=" << (target->GetSession() ? 1 : 0)
            << " hp=" << target->GetHealth()
            << " maxhp=" << target->GetMaxHealth()
            << " lvl=" << target->GetLevel()
            << " race=" << target->getRace()
            << " class=" << target->getClass()
            << " team=" << uint32(target->GetTeam())
            << " gm=" << (target->IsGameMaster() ? 1 : 0)
            << " map=" << target->GetMapId();
        return out.str();
    }

    // Summons a durable creature on the target and flags both into combat. Returns false if the
    // creature could not be summoned or the target refused to enter combat. Used where the callers
    // need the target in combat *in the same tick* - a separate command cannot hold the state across
    // an AI tick, because the bot's own AI drops it again.
    bool ForceCombat(Player* target, uint32 entry)
    {
        Creature* creature = target->SummonCreature(entry,
            target->GetPositionX() + 2.0f, target->GetPositionY(), target->GetPositionZ(),
            target->GetOrientation() + M_PI_F, TEMPSPAWN_MANUAL_DESPAWN, 0);

        if (!creature)
            return false;

        // SetLevel() alone does not recompute a spawned creature's stats (a level-60 entry still
        // reported hp 42 and was one-shot), so raise max health explicitly.
        creature->SetLevel(target->GetLevel());
        creature->SetMaxHealth(200000);
        creature->SetHealth(200000);

        target->SetSelectionGuid(creature->GetObjectGuid());
        target->Attack(creature, true);
        if (creature->AI())
            creature->AI()->AttackStart(target);

        target->SetInCombatWith(creature);
        creature->SetInCombatWith(target);

        return target->IsInCombat();
    }

    // Shared body for the two request commands: send, then compare against the expectation.
    TestResult RunRequest(const std::string& params, const char* what, bool sent, const std::string& details, std::string& message)
    {
        if (ExpectsRejection(params))
        {
            if (sent)
            {
                message = std::string(what) + " was sent but a rejection was expected";
                return TestResult::FAIL;
            }

            return TestResult::PASS;
        }

        if (!sent)
        {
            message = std::string(what) + " refused [" + details + "]";
            return TestResult::FAIL;
        }

        return TestResult::PASS;
    }
}

TestResult CommandSummonRequest::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Player* target = GetSpawnedBot(ctx);
    if (!target)
        return TestResult::PENDING; // still logging in

    // The core summon response path refuses an in-combat target, and a freshly spawned bot can drift
    // into combat on its own - so clear it for the normal (expect-accepted) case. When the test
    // explicitly expects a rejection, combat IS the precondition, so leave it alone.
    if (!ExpectsRejection(params) && target->IsInCombat())
    {
        target->CombatStopWithPets(true, true);
    }

    // Expecting a rejection: the in-combat precondition must be forced in THIS tick. A separate
    // "engage spawn" command cannot hold combat across an AI tick - the bot's own AI drops it, so by
    // the time this command runs the target is out of combat again and the request is accepted.
    // A DEAD target is its own rejection precondition (SendSummonRequest refuses corpses outright),
    // so combat only needs forcing while the target is alive.
    if (ExpectsRejection(params) && target->IsAlive() && !target->IsInCombat())
    {
        if (!ForceCombat(target, 299))
        {
            message = "summon request expect-rejected: could not force the target into combat";
            return TestResult::FAIL;
        }

        sLog.outString("[SUMMON] summon request: forced %s into combat for the rejection case", target->GetName());
    }

    float x, y, z;
    bot->GetPosition(x, y, z);

    sLog.outString("[SUMMON] summon request: host %s on map %u at (%.1f, %.1f, %.1f); target %s on map %u",
        bot->GetName(), bot->GetMapId(), x, y, z, target->GetName(), target->GetMapId());

    const bool sent = PlayerbotAI::SendSummonRequest(bot, target, bot->GetMapId(), x, y, z);

    return RunRequest(params, "summon request", sent, DescribeTarget(target), message);
}

TestResult CommandResurrectRequest::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Player* target = GetSpawnedBot(ctx);
    if (!target)
        return TestResult::PENDING; // still logging in

    float x, y, z;
    bot->GetPosition(x, y, z);

    const bool sent = PlayerbotAI::SendResurrectRequest(bot, target, bot->GetMapId(), x, y, z);

    // Remember where we told the corpse to land: the monitor must measure against this, not against
    // the acting bot, which may random-teleport away long before the resurrect completes.
    if (sent)
    {
        ctx.resurrectMapId = bot->GetMapId();
        ctx.resurrectX = x;
        ctx.resurrectY = y;
        ctx.resurrectZ = z;
        ctx.hasResurrectRequest = true;
    }

    return RunRequest(params, "resurrect request", sent, DescribeTarget(target), message);
}

TestResult CommandKillSpawn::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Player* target = GetSpawnedBot(ctx);
    if (!target)
        return TestResult::PENDING; // still logging in

    if (target->IsAlive())
        target->KillPlayer();

    // Deliberately decisive: returning PENDING here would hang the test, because monitors are only
    // evaluated once the script reaches "observe".
    if (target->IsAlive())
    {
        message = "kill spawn: target is still alive after KillPlayer";
        return TestResult::FAIL;
    }

    return TestResult::PASS;
}

TestResult CommandMoveSpawn::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Player* target = GetSpawnedBot(ctx);
    if (!target)
        return TestResult::PENDING; // still logging in

    GuidPosition loc;
    if (!TestRegistry::ParseLocation(params, loc))
    {
        message = "move spawn: invalid location: " + params;
        return TestResult::IMPOSSIBLE;
    }

    const uint32 mapId = loc.mapid;
    const float x = loc.coord_x;
    const float y = loc.coord_y;
    const float z = loc.coord_z;

    // Route through RunOnOwningThread: it runs inline when the spawned bot is already on our map,
    // and queues to the world messager otherwise - so a cross-map move never touches another map
    // from this thread.
    ai->RunOnOwningThread(target, [mapId, x, y, z](Player* p)
    {
        p->TeleportTo(mapId, x, y, z, p->GetOrientation());
    });

    // Log the state right after the call: RunOnOwningThread runs inline when the bot shares our map,
    // so this reflects the teleport having been issued (beingTeleported=1) or silently refused.
    sLog.outString("[SUMMON] move spawn: %s -> %s (resolved map %u @ %.1f, %.1f, %.1f); now map=%u inWorld=%d beingTeleported=%d",
        target->GetName(), params.c_str(), mapId, x, y, z,
        target->GetMapId(), target->IsInWorld() ? 1 : 0, target->IsBeingTeleported() ? 1 : 0);
    return TestResult::PASS;
}

TestResult CommandHideSpawn::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Player* target = GetSpawnedBot(ctx);
    if (!target)
        return TestResult::PENDING; // still entering the world

    // GM mode is what stops the guards engaging; the combat stop clears anything that already
    // landed in the tick before this ran.
    target->SetGMVisible(false);
    target->CombatStopWithPets(true, true);
    return TestResult::PASS;
}

TestResult CommandEngageSpawn::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Player* target = GetSpawnedBot(ctx);
    if (!target)
        return TestResult::PENDING; // still logging in

    const uint32 entry = (uint32)atoi(params.c_str());
    if (!entry)
    {
        message = "engage spawn: invalid creature entry: " + params;
        return TestResult::IMPOSSIBLE;
    }

    // Deliberately NO early return when already in combat: a freshly spawned bot is often briefly
    // in combat with something that dies immediately, so combat drops a tick later. Always add a
    // durable attacker instead.
    Creature* creature = target->SummonCreature(entry,
        target->GetPositionX() + 2.0f, target->GetPositionY(), target->GetPositionZ(),
        target->GetOrientation() + M_PI_F, TEMPSPAWN_MANUAL_DESPAWN, 0);

    if (!creature)
    {
        message = "engage spawn: failed to summon creature " + std::to_string(entry);
        return TestResult::IMPOSSIBLE;
    }

    // Make the prey survive. SetLevel() alone does NOT recompute a spawned creature's stats - a
    // level-60 entry still reported hp 42 and was one-shot, which drops combat the next tick and
    // lets the summon request through. Raise max health explicitly.
    creature->SetLevel(target->GetLevel());
    creature->SetMaxHealth(200000);
    creature->SetHealth(200000);

    target->SetSelectionGuid(creature->GetObjectGuid());
    target->Attack(creature, true);
    if (creature->AI())
        creature->AI()->AttackStart(target);

    // Belt and braces: flag the pair in combat both ways, so it holds even if neither AI lands a hit
    // on this tick.
    target->SetInCombatWith(creature);
    creature->SetInCombatWith(target);

    if (!target->IsInCombat())
    {
        message = "engage spawn: target still not in combat after attacking creature " + std::to_string(entry);
        return TestResult::FAIL;
    }

    sLog.outString("[SUMMON] engage spawn: %s now in combat with entry %u (level %u, hp %u)",
        target->GetName(), entry, creature->GetLevel(), creature->GetHealth());
    return TestResult::PASS;
}

bool MonitorSpawnOnMap::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Player* spawned = GetSpawnedBot(ctx);
    if (!spawned)
        return false;

    return spawned->GetMapId() == bot->GetMapId() && spawned->GetInstanceId() == bot->GetInstanceId();
}

bool MonitorSpawnAlive::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Player* spawned = GetSpawnedBot(ctx);
    if (!spawned)
        return false;

    return spawned->IsAlive();
}

bool MonitorSpawnResurrected::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Player* spawned = GetSpawnedBot(ctx);
    if (!spawned)
        return false;

    const bool requestedBy = spawned->isRessurectRequestedBy(bot->GetObjectGuid());

    // Measure the landing against the position our request specified, never against the acting bot:
    // the caller is a random bot that can random-teleport thousands of yards (or into a battleground)
    // while the 120 s observe window runs. The comparison is optional - without one the monitor
    // asserts only that the corpse was revived onto the map our request named while still carrying
    // it. Cross-map has to use that weaker form: the target is itself a roaming random bot, so the
    // manager can teleport it on while the resurrect's far teleport is in flight, and the core then
    // applies the resurrect wherever that teleport delivered it (Player::ResurrectUsingRequestDataInit
    // defers to a pending teleport), leaving a correct resurrect at an uncontrollable position.
    const bool onRequestMap = ctx.hasResurrectRequest && spawned->GetMapId() == ctx.resurrectMapId;
    const bool alive = spawned->IsAlive();

    std::string valueName;
    std::string op;
    std::string valueStr;
    std::string parseMessage;
    float threshold = 0.0f;
    const bool hasThreshold =
        TryParseComparisonValue(monitorStr, valueName, op, valueStr, parseMessage, GetName()) == TestResult::PASS &&
        TryParseFloatStrict(valueStr, threshold, parseMessage, GetName()) == TestResult::PASS;

    const float dist = onRequestMap ? spawned->GetDistance(ctx.resurrectX, ctx.resurrectY, ctx.resurrectZ) : -1.0f;
    const bool closeEnough = !hasThreshold || (onRequestMap && ((op == "<") ? (dist < threshold) : (dist > threshold)));

    if (!alive || !requestedBy)
        return false;

    return closeEnough;
}

bool MonitorSpawnDead::IsConditionMet(const std::string& monitorStr, Player* bot, TestContext& ctx) const
{
    Player* spawned = GetSpawnedBot(ctx);
    if (!spawned)
        return false;

    return !spawned->IsAlive();
}

// =============================================================================================
// BL-14 .. BL-25 - cross-map summon / resurrect scripts
// =============================================================================================

void TestRegistry::RegisterTeleportTests()
{
    static const std::string gmInvisible = "gm visible off";
    static const std::string gmVisible = "gm visible on";
    static const std::string needAlive = "monitor bot dead => abort \"Bot died, test interrupted\"";
    static const std::string needSpawn = "monitor spawn on map => pass \"Spawned bot reached the summoner's map\"";

    // BL-14 - alive bot summoned on the same map. The spawned bot is left in Stormwind and the
    // summoner moves to Elwynn (both map 0), so the closing distance proves the summon happened.
    RegisterTest("teleport_summon_alive_same_map", {
        gmInvisible,
        needAlive,
        "monitor spawn distance < 15 => pass \"Summoned bot arrived next to the summoner (same map)\"",
        "monitor time > 120 => fail \"Timeout: summoned bot never arrived (same map)\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "hide spawn",
        "wait 5",
        "teleport elwynn",
        "summon request",
        "observe",
        gmVisible
    });

    // BL-15 - alive bot summoned across maps. Summoner crosses to Kalimdor; the spawned bot has to
    // follow it there through the request path. A bad map/thread assumption shows up as a crash.
    RegisterTest("teleport_summon_alive_cross_map", {
        gmInvisible,
        needAlive,
        needSpawn,
        "monitor time > 120 => fail \"Timeout: summoned bot never crossed maps\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "hide spawn",
        "wait 5",
        "teleport orgrimmar",
        "summon request",
        "observe",
        gmVisible
    });

    // BL-16 - dead bot summoned on the same map. Must be resurrected by our request, not merely
    // teleported or revived by the bot's own dead strategy (spirit healer), so the monitor requires
    // the resurrect to have landed the corpse next to the summoner.
    RegisterTest("teleport_summon_dead_same_map", {
        gmInvisible,
        needAlive,
        "monitor spawn resurrected < 30 => pass \"Summoned corpse resurrected next to the summoner (same map)\"",
        "monitor time > 120 => fail \"Timeout: summoned corpse was not resurrected (same map)\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "hide spawn",
        "wait 5",
        "teleport elwynn",
        "kill spawn",
        "resurrect request",
        "observe",
        gmVisible
    });

    // BL-22 - the plain summon path must never resurrect a corpse. `PlayerbotAI::SendSummonRequest`
    // refuses a dead target outright, and the only code that rezzes on this path is the *explicit*
    // `resurrectPlayer` branch of `SummonAction::Teleport` (covered by BL-16/BL-17). So a summon
    // request aimed at a corpse must be refused, and the corpse must stay dead. Note this asserts the
    // refusal, not "the summon path is rez-free" - the meeting-stone/`SummonAction` path deliberately
    // DOES resurrect a dead target so it can be summoned; driving that end-to-end needs an innkeeper
    // or meeting stone in the world and is not expressible from this harness yet.
    RegisterTest("teleport_summon_dead_refused", {
        gmInvisible,
        needAlive,
        "monitor spawn alive => fail \"A refused summon resurrected the corpse\"",
        "monitor time > 20 => pass \"Corpse stayed dead; the summon request was refused, not converted into a resurrect\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "hide spawn",
        "wait 5",
        "teleport elwynn",
        "kill spawn",
        "summon request expect rejected",
        "observe",
        gmVisible
    });

    // BL-17 - dead bot summoned across maps.
    //
    // No distance assertion here, unlike BL-16: the spawned target is a free-alt random bot, so
    // RandomPlayerbotMgr keeps roaming it (random teleport / arena queue) and can move it on while
    // the resurrect's far teleport is in flight. The core then applies the resurrect at wherever
    // that teleport delivered the corpse (Player::ResurrectUsingRequestDataInit defers to a pending
    // teleport), so the landing point is correct but not controllable. "Alive on the map our request
    // named, still carrying that request" is still sound causality for the cross-map case: a spirit
    // healer or self-resurrect cannot move a map-0 corpse onto map 1.
    RegisterTest("teleport_summon_dead_cross_map", {
        gmInvisible,
        needAlive,
        "monitor spawn resurrected => pass \"Summoned corpse resurrected onto the summoner's map (cross map)\"",
        "monitor time > 120 => fail \"Timeout: summoned corpse was not resurrected (cross map)\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "hide spawn",
        "wait 5",
        "teleport orgrimmar",
        "kill spawn",
        "resurrect request",
        "observe",
        gmVisible
    });
    // =========================================================================================
    // DIAGNOSTIC (temporary) - isolate the spawn/login crash.
    // These are NOT BL-14..28 coverage; they exist only to localise the libmysql crash and
    // should be deleted once the spawn path is fixed.
    //
    // Crash signature being chased: libmysql.dll 8.0.23.0, 0x80000003 (STATUS_BREAKPOINT),
    // offset 0x64ad2 - fires 3-4s after CreateBot returns "done", before LoginFreeBots sees
    // the new bot. CreateBot itself always completes (SaveToDB ok, LogoutPlayer ok).
    // =========================================================================================

    // Control: the known-crashing case (a stock scenario test uses the same line).
    RegisterTest("diag_spawn_login1", {
        gmInvisible,
        needAlive,
        "monitor time > 45 => pass \"Survived spawn login=1\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "observe"
    });

    // Same CreateBot, but login=0 -> autoAdd=false -> never pushed to freeAltBots, so the
    // session login never runs. Survives => the crash is in the login kickoff, not in
    // CreateBot's own DB writes (SaveToDB / LogoutPlayer).
    RegisterTest("diag_spawn_login0", {
        gmInvisible,
        needAlive,
        "monitor time > 45 => pass \"Survived spawn login=0 (no login kickoff)\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=0",
        "observe"
    });

    // The group route: mgroup -> HandleGroup -> HandleCreate -> the same CreateBot.
    // size=3 means the host counts as 1, so 2 bots get created back to back.
    RegisterTest("diag_mgroup_two_bots", {
        gmInvisible,
        needAlive,
        "monitor time > 60 => pass \"Survived mgroup (2 bots created)\"",
        "teleport stormwind",
        "mgroup size=3",
        "observe"
    });

    // =========================================================================================
    // Tests for the primitives added for BL-20 / BL-26 / BL-28: move spawn, engage spawn, spawn dead
    // =========================================================================================

    // BL-20 - an in-combat target cannot accept a summon, so the request must be refused (the caller
    // then falls back to a direct teleport). Forces the in-combat state with engage spawn.
    RegisterTest("teleport_summon_in_combat_rejected", {
        gmInvisible,
        needAlive,
        "monitor time > 20 => pass \"In-combat target refused the summon request\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1",
        "engage spawn 299",
        "summon request expect rejected",
        "observe"
    });

    // Smoke test for "move spawn" - cross-map and deterministic. Asserts on the MAP, not distance,
    // so an AI-wandering bot cannot produce a false negative. The host stays in Stormwind (map 0)
    // and the spawned bot is moved to Orgrimmar (map 1), so "spawn on map" must never become true.
    //
    // group= is required: without it the spawned bot joins the host's group, and LoginFreeBots'
    // deferred create-group handling teleports it to the master about a tick later - which lands it
    // back on the host's map and makes the move look like it failed (it did not; see BL-31).
    RegisterTest("teleport_move_spawn_cross_map", {
        gmInvisible,
        needAlive,
        "monitor spawn on map => fail \"Spawned bot never left the host's map\"",
        "monitor time > 20 => pass \"Moved spawned bot is off the host's map (cross-map move worked)\"",
        "teleport stormwind",
        "spawn level=60 temporary=1 login=1 group=",
        "move spawn orgrimmar",
        "wait 5",
        "observe"
    });
}

// ---------------------------------------------------------------------------------------------
// Not covered yet - each needs a test primitive that does not exist:
//
//   BL-18  warlock summon                - needs a warlock bot and its summon action
//   BL-19  world-buff travel summon      - needs a travel-summon trigger
//   BL-20  in-combat target refused      - needs a way to put the *spawned* bot into combat
//                                          ("pull" only makes the acting bot fight)
//   BL-21  summoner on a transport       - needs a board-transport setup command
//   BL-22  meeting stone must not rez    - needs to drive SummonAction, which needs an innkeeper
//                                          nearby (a direct request call bypasses that decision)
//   BL-23  real player rez on a dead bot - covered by the resurrect request above
//   BL-24  real player as summon target  - covered: the bot accept path uses the same opcodes
//   BL-25  RunOnOwningThread caller      - needs an assertion on which thread ran the callback
// ---------------------------------------------------------------------------------------------
