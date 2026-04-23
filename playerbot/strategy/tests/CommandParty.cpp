#include "playerbot/playerbot.h"
#include "CommandParty.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestContext.h"
#include "playerbot/PlayerbotMgr.h"
#include "Groups/Group.h"

using namespace ai;

TestResult CommandPartySpawnBot::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (!ai->GetHolder())
    {
        message = "Failed to spawn bot with params: " + params;
        return TestResult::IMPOSSIBLE;
    }
    std::list<std::string> messages;
    ObjectGuid guid;
    ai->GetHolder()->CreateBot(bot, params + " temporary=true", messages, guid);

    if (!guid)
    {
        message = "Failed to spawn bot with params: " + params;
        return TestResult::IMPOSSIBLE;
    }

    ctx.spawnedBots.push_back(guid);

    return TestResult::PASS;
}

TestResult CommandPartyDespawnBot::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    if (!ai->GetHolder())
    {
        message = "Failed to delete spawned bots";
        return TestResult::IMPOSSIBLE;
    }

    for (auto& guid : ctx.spawnedBots)
    {
        ai->GetHolder()->DeleteBot(guid);
    }
    ctx.spawnedBots.clear();
    return TestResult::PASS;
}

TestResult CommandPartyForm::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        group = new Group;
        if (!group->Create(bot->GetObjectGuid(), bot->GetName()))
        {
            delete group;
            message = "Failed to create group";
            return TestResult::IMPOSSIBLE;
        }
        sObjectMgr.AddGroup(group);
    }

    if (!ctx.spawnedBots.empty())
    {
        for (const auto& guid : ctx.spawnedBots)
        {
            if (!guid || !guid.IsPlayer())
                continue;

            Player* member = sObjectMgr.GetPlayer(guid);
            if (!member || !member->IsInWorld())
            {
                message = "Waiting for spawned bots to enter world before forming party";
                return TestResult::PENDING;
            }
        }
    }

    for (const auto& guid : ctx.spawnedBots)
    {
        if (!guid || !guid.IsPlayer())
            continue;

        Player* member = sObjectMgr.GetPlayer(guid);
        if (!member)
            continue;

        if (!group->IsMember(guid) && !group->AddMember(guid, member->GetName()))
        {
            message = "Failed to add member to group: " + std::string(member->GetName());
            return TestResult::IMPOSSIBLE;
        }

        if (PlayerbotAI* memberAi = member->GetPlayerbotAI())
            memberAi->HandleCommand(CHAT_MSG_WHISPER, "follow " + std::string(bot->GetName()), *bot);
    }

    return TestResult::PASS;
}

TestResult CommandPartySpawnGroup::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& message)
{
    sRandomPlayerbotMgr.HandleGroup(bot, params + " temporary=true", SEC_PLAYER);

    return TestResult::PASS;
}