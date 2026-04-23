#include "playerbot/playerbot.h"
#include "PartyCommands.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "TestContext.h"
#include "playerbot/PlayerbotMgr.h"
#include "Groups/Group.h"

using namespace ai;

bool HandleSpawnBot::Execute(const std::string& params, Player* bot, PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    if (!ai->GetHolder())
    {
        error = "Failed to spawn bot with params: " + params;
        return false;
    }
    std::list<std::string> messages;
    ObjectGuid guid;
    ai->GetHolder()->CreateBot(bot, params + " temporary=true", messages, guid);

    if (!guid)
    {
        error = "Failed to spawn bot with params: " + params;
        return false;
    }

    ctx.spawnedBots.push_back(guid);

    return true;
}

bool HandleDespawnBot::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    if (!ai->GetHolder())
    {
        error = "Failed to delete spawned bots";
        return false;
    }

    for (auto& guid : ctx.spawnedBots)
    {
        ai->GetHolder()->DeleteBot(guid);
    }
    ctx.spawnedBots.clear();
    return true;
}

bool HandleFormParty::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    Group* group = bot->GetGroup();
    if (!group)
    {
        group = new Group;
        if (!group->Create(bot->GetObjectGuid(), bot->GetName()))
        {
            delete group;
            error = "Failed to create group";
            return false;
        }
        sObjectMgr.AddGroup(group);
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
            error = "Failed to add member to group: " + std::string(member->GetName());
            return false;
        }

        if (PlayerbotAI* memberAi = member->GetPlayerbotAI())
            memberAi->HandleCommand(CHAT_MSG_WHISPER, "follow " + std::string(bot->GetName()), *bot);
    }

    return true;
}

bool HandleSpawnGroup::Execute(const std::string& params, Player* bot,
                    PlayerbotAI* ai, TestContext& ctx, std::string& error)
{
    sRandomPlayerbotMgr.HandleGroup(bot, params + " temporary=true", SEC_PLAYER);

    return true;
}