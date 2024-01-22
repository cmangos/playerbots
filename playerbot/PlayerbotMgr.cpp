#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotDbStore.h"
#include "PlayerbotFactory.h"
#include "RandomPlayerbotMgr.h"
#include "ServerFacade.h"
#include "TravelMgr.h"
#include "Chat/ChannelMgr.h"
#include "Social/SocialMgr.h"


class LoginQueryHolder;
class CharacterHandler;

PlayerbotHolder::PlayerbotHolder() : PlayerbotAIBase()
{
    for (uint32 spellId = 0; spellId < sServerFacade.GetSpellInfoRows(); spellId++)
        sServerFacade.LookupSpellInfo(spellId);
}

PlayerbotHolder::~PlayerbotHolder()
{
}


void PlayerbotHolder::UpdateAIInternal(uint32 elapsed, bool minimal)
{
}

void PlayerbotHolder::UpdateSessions(uint32 elapsed)
{
    for (PlayerBotMap::const_iterator itr = GetPlayerBotsBegin(); itr != GetPlayerBotsEnd(); ++itr)
    {
        Player* const bot = itr->second;
        if (bot->GetPlayerbotAI() && bot->IsBeingTeleported())
        {
            bot->GetPlayerbotAI()->HandleTeleportAck();
        }
        else if (bot->IsInWorld())
        {
            bot->GetSession()->HandleBotPackets();
        }

        if (bot->GetPlayerbotAI() && bot->GetPlayerbotAI()->GetShouldLogOut() && !bot->IsStunnedByLogout() && !bot->GetSession()->isLogingOut())
        {
            LogoutPlayerBot(bot->GetObjectGuid().GetRawValue());
        }
    }
}

void PlayerbotHolder::LogoutAllBots()
{    
    /*
    while (true)
    {
        PlayerBotMap::const_iterator itr = GetPlayerBotsBegin();
        if (itr == GetPlayerBotsEnd()) break;
        Player* bot= itr->second;
        if (!bot->GetPlayerbotAI()->isRealPlayer())
            LogoutPlayerBot(bot->GetObjectGuid().GetRawValue());
    }
    */    
    PlayerBotMap bots = playerBots;

    for (auto& itr : bots)
    {
        Player* bot = itr.second;
        if (!bot)
            continue;

        if (!bot->GetPlayerbotAI() || bot->GetPlayerbotAI()->IsRealPlayer())
            continue;

        LogoutPlayerBot(bot->GetGUIDLow());
    }
}

void PlayerbotMgr::CancelLogout()
{
    Player* master = GetMaster();
    if (!master)
        return;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (!ai || ai->IsRealPlayer())
            continue;

        if (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut())
        {
            WorldPacket p;
            bot->GetSession()->HandleLogoutCancelOpcode(p);
            ai->TellPlayer(GetMaster(), BOT_TEXT("logout_cancel"));
        }
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (!ai || ai->IsRealPlayer())
            continue;

        if (bot->GetPlayerbotAI()->GetMaster() != master)
            continue;

        if (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut())
        {
            WorldPacket p;
            bot->GetSession()->HandleLogoutCancelOpcode(p);
        }
    }
}

void PlayerbotHolder::LogoutPlayerBot(uint32 guid)
{
    Player* bot = GetPlayerBot(guid);
    if (bot)
    {
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (!ai)
            return;

        Group *group = bot->GetGroup();
        if (group && !bot->InBattleGround() && !bot->InBattleGroundQueue() && ai->HasActivePlayerMaster())
        {
            sPlayerbotDbStore.Save(ai);
        }
        sLog.outDebug("Bot %s logging out", bot->GetName());
        bot->SaveToDB();

        WorldSession* botWorldSessionPtr = bot->GetSession();
        WorldSession* masterWorldSessionPtr = nullptr;

        Player* master = ai->GetMaster();
        if (master)
            masterWorldSessionPtr = master->GetSession();

        // check for instant logout
        bool logout = botWorldSessionPtr->ShouldLogOut(time(nullptr));

        // make instant logout for now
        logout = true;

        if (masterWorldSessionPtr && masterWorldSessionPtr->ShouldLogOut(time(nullptr)))
            logout = true;
        
        if (masterWorldSessionPtr && masterWorldSessionPtr->GetState() != WORLD_SESSION_STATE_READY)
            logout = true;

        if (bot->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || bot->IsTaxiFlying() ||
            botWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))
        {
            logout = true;
        }

        if (master && (master->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_RESTING) || master->IsTaxiFlying() ||
            (masterWorldSessionPtr && masterWorldSessionPtr->GetSecurity() >= (AccountTypes)sWorld.getConfig(CONFIG_UINT32_INSTANT_LOGOUT))))
        {
            logout = true;
        }

        // if no instant logout, request normal logout
        if (!logout)
        {
            if (bot && (bot->IsStunnedByLogout() || bot->GetSession()->isLogingOut()))
                return;
            else if (bot)
            {
                ai->TellPlayer(ai->GetMaster(), BOT_TEXT("logout_start"));
                WorldPacket p;
                botWorldSessionPtr->HandleLogoutRequestOpcode(p);
                if (!bot)
                {
                    playerBots.erase(guid);
                    delete botWorldSessionPtr;    
                }
                return;
            }
            else
            {
                playerBots.erase(guid);     // deletes bot player ptr inside this WorldSession PlayerBotMap
                delete botWorldSessionPtr;  // finally delete the bot's WorldSession
            }
            return;
        } // if instant logout possible, do it
        else if (bot && (logout || !botWorldSessionPtr->isLogingOut()))
        {
            ai->TellPlayer(ai->GetMaster(), BOT_TEXT("goodbye"));
            playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap
#ifdef CMANGOS
            botWorldSessionPtr->LogoutPlayer(); // this will delete the bot Player object and PlayerbotAI object
#endif
#ifdef MANGOS
            //botWorldSessionPtr->LogoutPlayer(true); // this will delete the bot Player object and PlayerbotAI object
#endif
            if(!sWorld.FindSession(botWorldSessionPtr->GetAccountId())) //Real player sessions will get removed later.
                delete botWorldSessionPtr;  // finally delete the bot's WorldSession
        }
    }
}

void PlayerbotHolder::DisablePlayerBot(uint32 guid)
{
    Player* bot = GetPlayerBot(guid);
    if (bot)
    {
        bot->GetPlayerbotAI()->TellPlayer(bot->GetPlayerbotAI()->GetMaster(), BOT_TEXT("goodbye"));
        bot->GetPlayerbotAI()->StopMoving();
        MotionMaster& mm = *bot->GetMotionMaster();
        mm.Clear();

        Group* group = bot->GetGroup();
        if (group && !bot->InBattleGround() && !bot->InBattleGroundQueue() && bot->GetPlayerbotAI()->HasActivePlayerMaster())
        {
            sPlayerbotDbStore.Save(bot->GetPlayerbotAI());
        }
        sLog.outDebug("Bot %s logged out", bot->GetName());
        bot->SaveToDB();

        WorldSession* botWorldSessionPtr = bot->GetSession();
        playerBots.erase(guid);    // deletes bot player ptr inside this WorldSession PlayerBotMap

        if (bot->GetPlayerbotAI()) 
        {
            bot->RemovePlayerbotAI();
        }
    }
}

Player* PlayerbotHolder::GetPlayerBot(uint32 playerGuid) const
{
    PlayerBotMap::const_iterator it = playerBots.find(playerGuid);
    return (it == playerBots.end()) ? nullptr : it->second ? it->second : nullptr;
}

void PlayerbotHolder::OnBotLogin(Player * const bot)
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai)
    {
        bot->CreatePlayerbotAI();
        ai = bot->GetPlayerbotAI();
    }

	OnBotLoginInternal(bot);

    playerBots[bot->GetGUIDLow()] = bot;

    Player* master = ai->GetMaster();

    if (!master && sPlayerbotAIConfig.IsFreeAltBot(bot))
    {
        ai->SetMaster(bot);
        master = bot;
    }

    if (master)
    {
        ObjectGuid masterGuid = master->GetObjectGuid();
        if (master->GetGroup() && !master->GetGroup()->IsLeader(masterGuid))
            master->GetGroup()->ChangeLeader(masterGuid);
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool groupValid = false;
        Group::MemberSlotList const& slots = group->GetMemberSlots();
        for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
        {
            ObjectGuid member = i->guid;
            if (master)
            {
                if (master->GetObjectGuid() == member)
                {
                    groupValid = true;
                    break;
                }
            }

            // Don't disband alt groups when master goes away 
            // (will need to manually disband with leave command)
            uint32 account = sObjectMgr.GetPlayerAccountIdByGUID(member);
            if (!sPlayerbotAIConfig.IsInRandomAccountList(account))
            {
                groupValid = true;
                break;
            }
        }

        if (!groupValid)
        {
            WorldPacket p;
            string member = bot->GetName();
            p << uint32(PARTY_OP_LEAVE) << member << uint32(0);
            bot->GetSession()->HandleGroupDisbandOpcode(p);
        }
    }

    ai->ResetStrategies();

    if (master && !master->IsTaxiFlying())
    {
        bot->GetMotionMaster()->MovementExpired();
#ifdef MANGOS
        bot->m_taxi.ClearTaxiDestinations();
#endif
    }

    // check activity
    ai->AllowActivity(ALL_ACTIVITY, true);
    // set delay on login
    ai->SetActionDuration(urand(2000, 4000));

    ai->TellPlayer(ai->GetMaster(), BOT_TEXT("hello"));

    // bots join World chat if not solo oriented
    if (bot->GetLevel() >= 10 && sRandomPlayerbotMgr.IsFreeBot(bot) && bot->GetPlayerbotAI() && bot->GetPlayerbotAI()->GetGrouperType() != GrouperType::SOLO)
    {
        // TODO make action/config
        // Make the bot join the world channel for chat
        WorldPacket pkt(CMSG_JOIN_CHANNEL);
#ifndef MANGOSBOT_ZERO
        pkt << uint32(0) << uint8(0) << uint8(0);
#endif
        pkt << std::string("World");
        pkt << ""; // Pass
        bot->GetSession()->HandleJoinChannelOpcode(pkt);

#ifdef MANGOSBOT_ZERO
        // bots join World Defense
        WorldPacket pkt2(CMSG_JOIN_CHANNEL);
#ifndef MANGOSBOT_ZERO
        pkt2 << uint32(0) << uint8(0) << uint8(0);
#endif
        pkt2 << std::string("WorldDefense");
        pkt2 << ""; // Pass
        bot->GetSession()->HandleJoinChannelOpcode(pkt2);
#endif
    }
    // join standard channels
    int32 locale = sConfig.GetIntDefault("DBC.Locale", 0 /*LocaleConstant::LOCALE_enUS*/); // bot->GetSession()->GetSessionDbcLocale();
    // -- In case we're using auto detect on config file
    if (locale == 255)
        locale = static_cast<int32>(LocaleConstant::LOCALE_enUS);
    
    AreaTableEntry const* current_zone = GetAreaEntryByAreaID(sTerrainMgr.GetAreaId(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()));
    ChannelMgr* cMgr = channelMgr(bot->GetTeam());
    std::string current_zone_name = current_zone ? current_zone->area_name[locale] : "";

    if (current_zone && cMgr)
    {
        for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
        {
            ChatChannelsEntry const* channel = sChatChannelsStore.LookupEntry(i);
            if (!channel) continue;

#ifndef MANGOSBOT_ZERO
            bool isLfg = (channel->flags & Channel::CHANNEL_DBC_FLAG_LFG) != 0;
#else
            bool isLfg = channel->ChannelID == 24;
#endif

            // skip non built-in channels or global channel without zone name in pattern
            if (!isLfg && (!channel || (channel->flags & 4) == 4))
                continue;

            //  new channel
            Channel* new_channel = nullptr;
            if (isLfg)
            {
#ifndef MANGOSBOT_ZERO
                new_channel = cMgr->GetJoinChannel(channel->pattern[locale], channel->ChannelID);
#else
                new_channel = cMgr->GetJoinChannel("LookingForGroup");
#endif
            }
            else
            {
                char new_channel_name_buf[100];
                snprintf(new_channel_name_buf, 100, channel->pattern[locale], current_zone_name.c_str());
#ifndef MANGOSBOT_ZERO
                new_channel = cMgr->GetJoinChannel(new_channel_name_buf, channel->ChannelID);
#else
                new_channel = cMgr->GetJoinChannel(new_channel_name_buf);
#endif
            }
            if (new_channel)
                new_channel->Join(bot, "");
        }
    }

    if (sRandomPlayerbotMgr.IsRandomBot(bot))
    {
        uint32 lowguid = bot->GetObjectGuid().GetCounter();
        auto result = CharacterDatabase.PQuery("SELECT 1 FROM character_social WHERE flags='%u' and friend='%d'", SOCIAL_FLAG_FRIEND, lowguid);
        if (result)
            bot->GetPlayerbotAI()->SetPlayerFriend(true);
        else
            bot->GetPlayerbotAI()->SetPlayerFriend(false);

        if (sPlayerbotAIConfig.instantRandomize && !sPlayerbotAIConfig.disableRandomLevels && !bot->GetTotalPlayedTime())
        {
            sRandomPlayerbotMgr.InstaRandomize(bot);
        }
    }
}

string PlayerbotHolder::ProcessBotCommand(string cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId)
{
    if (!sPlayerbotAIConfig.enabled || guid.IsEmpty())
        return "Bot system is disabled";

    uint32 botAccount = sObjectMgr.GetPlayerAccountIdByGUID(guid);
    bool isRandomBot = sRandomPlayerbotMgr.IsRandomBot(guid);
    bool isRandomAccount = sPlayerbotAIConfig.IsInRandomAccountList(botAccount);
    bool isMasterAccount = (masterAccountId == botAccount);

    if (!isRandomAccount && (!isMasterAccount && !admin && masterguid))
    {
        Player* master = sObjectMgr.GetPlayer(masterguid);
        if (master && (!sPlayerbotAIConfig.allowGuildBots || !masterGuildId || (masterGuildId && master->GetGuildIdFromDB(guid) != masterGuildId)))
            return "Not in your guild or account";
    }

    if (!isRandomAccount && this == &sRandomPlayerbotMgr)
    {
        return "Can not control alt-bots with this command.";
    }

    if (cmd == "add" || cmd == "login")
    {
        if (sObjectMgr.GetPlayer(guid))
            return "Player already logged in";

        // Only allow bots that are on the same account or same guild (if enabled)
        Player* master = sObjectMgr.GetPlayer(masterguid);
        uint32 guildId = Player::GetGuildIdFromDB(guid);
        if (master && (isMasterAccount || (sPlayerbotAIConfig.allowGuildBots && masterGuildId && guildId == masterGuildId) || admin))
        {
            if (isRandomAccount)
                sRandomPlayerbotMgr.AddRandomBot(guid.GetCounter());
            else if (isMasterAccount || sPlayerbotAIConfig.allowMultiAccountAltBots)
                AddPlayerBot(guid.GetCounter(), masterAccountId);
            else
                return "Not in your account";
        }
        else
        {
            return "Not in your guild or account";
        }

        return "ok";
    }
    else if (cmd == "remove" || cmd == "logout" || cmd == "rm")
    {
        Player* player = sObjectMgr.GetPlayer(guid);
        if (!player)
            return "Player is offline";

        Player* master = sObjectMgr.GetPlayer(masterguid);
        uint32 guildId = Player::GetGuildIdFromDB(guid);
        if (master && (isMasterAccount || (sPlayerbotAIConfig.allowGuildBots && masterGuildId && guildId == masterGuildId) || admin))
        {
            if (isRandomAccount)
                sRandomPlayerbotMgr.Remove(player);
            else if (GetPlayerBot(guid.GetCounter()))
                LogoutPlayerBot(guid.GetCounter());
            else
                return "Not your bot";
        }
        else
        {
            return "Not in your guild or account";
        }

        return "ok";
    }

    if (admin)
    {
        Player* bot = GetPlayerBot(guid.GetCounter());
        if (!bot) bot = sRandomPlayerbotMgr.GetPlayerBot(guid.GetCounter());
        if (!bot)
            return "bot not found";

        Player* master = bot->GetPlayerbotAI()->GetMaster();
        if (master)
        {
            if (cmd == "gear" || cmd == "equip")
            {
                PlayerbotFactory factory(bot, bot->GetLevel());
                factory.EquipGear();
                return "random gear equipped";
            }
            if (cmd == "equip=upgrade" || cmd == "gear=upgrade" || cmd == "upgrade" || cmd == "equip=sync")
            {
                const bool syncWithMaster = cmd == "equip=sync";
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.UpgradeGear(syncWithMaster);
                return "gear upgraded";
            }
            if (cmd == "train" || cmd == "learn")
            {
#ifndef MANGOSBOT_ONE
                bot->learnClassLevelSpells();
#endif
                return "class level spells learned";
            }
            if (cmd == "food" || cmd == "drink")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.AddFood();
                return "food added";
            }
            if (cmd == "potions" || cmd == "pots")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.AddPotions();
                return "potions added";
            }
            if (cmd == "consumes" || cmd == "consumables" || cmd == "consums")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.AddConsumes();
                return "consumables added";
            }
            if (cmd == "regs" || cmd == "reg" || cmd == "reagents")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.AddReagents();
                return "reagents added";
            }
            if (cmd == "prepare" || cmd == "prep" || cmd == "refresh")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.Refresh();
                return "consumes/regs added";
            }
            if (cmd == "init")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(true, false);
                return "ok";
            }
            if (cmd == "init=white" || cmd == "init=common")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_NORMAL);
                factory.Randomize(false, false);
                return "ok";
            }
            else if (cmd == "init=green" || cmd == "init=uncommon")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_UNCOMMON);
                factory.Randomize(false, false);
                return "ok";
            }
            else if (cmd == "init=blue" || cmd == "init=rare")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_RARE);
                factory.Randomize(false, false);
                return "ok";
            }
            else if (cmd == "init=epic" || cmd == "init=purple")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_EPIC);
                factory.Randomize(false, false);
                return "ok";
            }
            else if (cmd == "init=legendary" || cmd == "init=yellow")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false, false);
                return "ok";
            }
            else if (cmd == "init=sync")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.Randomize(false, true);
                return "ok";
            }
            else if (cmd == "enchants")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.EnchantEquipment();
                return "ok";
            }
            else if (cmd == "ammo")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.InitAmmo();
                return "ok";
            }
            else if (cmd == "pet")
            {
                PlayerbotFactory factory(bot, master->GetLevel(), ITEM_QUALITY_LEGENDARY);
                factory.InitPet();
                factory.InitPetSpells();
                return "ok";
            }
        }

        if (cmd == "levelup" || cmd == "level")
        {
            PlayerbotFactory factory(bot, bot->GetLevel());
            factory.Randomize(true, false);
            return "ok";
        }
        else if (cmd == "refresh")
        {
            PlayerbotFactory factory(bot, bot->GetLevel());
            factory.Refresh();
            return "ok";
        }
        else if (cmd == "random")
        {
            sRandomPlayerbotMgr.Randomize(bot);
            return "ok";
        }
    }

    return "unknown command";
}

bool PlayerbotMgr::HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args)
{
	if (!sPlayerbotAIConfig.enabled)
	{
		handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return false;
	}

    WorldSession *m_session = handler->GetSession();

    if (!m_session)
    {
        handler->PSendSysMessage("You may only add bots from an active session");
        return false;
    }

    Player* player = m_session->GetPlayer();
    PlayerbotMgr* mgr = player->GetPlayerbotMgr();
    if (!mgr)
    {
        handler->PSendSysMessage("you cannot control bots yet");
        return false;
    }

    list<string> messages = mgr->HandlePlayerbotCommand(args, player);
    if (messages.empty())
        return true;

    for (list<string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        handler->PSendSysMessage("%s",i->c_str());
    }

    return true;
}

list<string> PlayerbotHolder::HandlePlayerbotCommand(char const* args, Player* master)
{
    list<string> messages;

    if (!*args)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return messages;
    }

    string command = args;

    char *cmd = strtok ((char*)args, " ");
    const char *charname = strtok (NULL, " ");
    if (!cmd)
    {
        messages.push_back("usage: list/reload or add/init/remove PLAYERNAME");
        return messages;
    }

    if (!strcmp(cmd, "list"))
    {
        messages.push_back(ListBots(master));
        return messages;
    }

    if (!strcmp(cmd, "reload"))
    {
        messages.push_back("Reloading config");
        sPlayerbotAIConfig.Initialize();
        return messages;
    }   

    if (!strcmp(cmd, "tweak"))
    {
        sPlayerbotAIConfig.tweakValue = sPlayerbotAIConfig.tweakValue++;
        if (sPlayerbotAIConfig.tweakValue > 2)
            sPlayerbotAIConfig.tweakValue = 0;

        messages.push_back("Set tweakvalue to " + to_string(sPlayerbotAIConfig.tweakValue));
        return messages;
    }

    if (!strcmp(cmd, "always"))
    {
        if (sPlayerbotAIConfig.selfBotLevel == 0)
        {
            messages.push_back("Self-bot is disabled");
            return messages;
        }
        else if (sPlayerbotAIConfig.selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        {
            messages.push_back("You do not have permission to enable player ai");
            return messages;
        }
        else if (sPlayerbotAIConfig.selfBotLevel == 2 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        {
            messages.push_back("Player ai is only available while online");
            return messages;
        }

        ObjectGuid guid;
        uint32 accountId;
        string alwaysName;

        if (!charname)
        {
            guid = master->GetObjectGuid(); 
            accountId = master->GetSession()->GetAccountId();
            alwaysName = master->GetName();
        }
        else
        {
            guid = sObjectMgr.GetPlayerGuidByName(charname);
            if (!guid)
            {
                messages.push_back("character not found");
                return messages;
            }

            accountId = sObjectMgr.GetPlayerAccountIdByGUID(guid);

            if (accountId != master->GetSession()->GetAccountId() && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            {
                messages.push_back("Player ai can not be forced for characters from a different account.");
                return messages;
            }

            alwaysName = charname;
        }

        uint32 always = sRandomPlayerbotMgr.GetValue(guid.GetCounter(), "always");

        if (!always || always == 2)
        {
            sRandomPlayerbotMgr.SetValue(guid.GetCounter(), "always", 1);
            messages.push_back("Enable offline player ai for " + alwaysName);
            sPlayerbotAIConfig.freeAltBots.push_back(make_pair(accountId, guid.GetCounter()));
        }
        else
        {
            sRandomPlayerbotMgr.SetValue(guid.GetCounter(), "always", 2);
            messages.push_back("Disable offline player ai for " + alwaysName);

            if (guid != master->GetObjectGuid())
            {
                Player* bot = sRandomPlayerbotMgr.GetPlayerBot(guid);

                if (bot && bot->GetPlayerbotAI() && !bot->GetPlayerbotAI()->GetMaster() && sPlayerbotAIConfig.IsFreeAltBot(bot))
                {
                    sRandomPlayerbotMgr.LogoutPlayerBot(guid);
                }
            }

            auto it = remove_if(sPlayerbotAIConfig.freeAltBots.begin(), sPlayerbotAIConfig.freeAltBots.end(), [guid](std::pair<uint32, uint32> i) {return i.second == guid.GetCounter(); });

            if (it != sPlayerbotAIConfig.freeAltBots.end())
                sPlayerbotAIConfig.freeAltBots.erase(it, sPlayerbotAIConfig.freeAltBots.end());
        }
        
        return messages;
    }

    if (!strcmp(cmd, "self"))
    {        
        if (master->GetPlayerbotAI())
        {
            DisablePlayerBot(master->GetGUIDLow());
           
            if (sRandomPlayerbotMgr.GetValue(master->GetObjectGuid().GetCounter(), "selfbot"))
            {
                messages.push_back("Disable player ai (on login)");
                sRandomPlayerbotMgr.SetValue(master->GetObjectGuid().GetCounter(), "selfbot", 0);
            }
            else
                messages.push_back("Disable player ai");
        }
        else if (sPlayerbotAIConfig.selfBotLevel == 0)
            messages.push_back("Self-bot is disabled");
        else if (sPlayerbotAIConfig.selfBotLevel == 1 && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
            messages.push_back("You do not have permission to enable player ai");
        else
        {
            OnBotLogin(master);

            if (charname && !strcmp(charname, "login"))
            {
                messages.push_back("Enable player ai (on login)");
                sRandomPlayerbotMgr.SetValue(master->GetObjectGuid().GetCounter(), "selfbot", 1);
            }
            else
                messages.push_back("Enable player ai");
        }
       return messages;
     }

    if (command.find("debug ") != std::string::npos)
    {
        bool hasBot = false;

        if (!master)
        {
            master = GetPlayerBotsBegin()->second;
        }

        PlayerbotAI* ai = master->GetPlayerbotAI();      
        if (ai)
        {
            hasBot = true;
        }
        else
        {
            master->CreatePlayerbotAI();
            ai = master->GetPlayerbotAI();
            ai->SetMaster(master);
            ai->ResetStrategies();
        }

        command = command.substr(6);

        if(ai->DoSpecificAction("cdebug", Event(".bot",command,master), true))
        {
            messages.push_back("debug failed");
        }

        if (!hasBot)
        {
            if (master->GetPlayerbotAI()) 
            {
                master->RemovePlayerbotAI();
            }
        }

        return messages;
    }

    if (!charname)
    {
        if (master && master->GetTarget() && master->GetTarget()->IsPlayer() && !((Player*)master->GetTarget())->isRealPlayer())
        {
            charname = master->GetTarget()->GetName();
        }
        else
        {
            messages.push_back("usage: list or add/init/remove PLAYERNAME");
            return messages;
        }
    }

    std::string cmdStr = cmd;
    std::string charnameStr = charname;

    set<string> bots;
    if (charnameStr == "*" && master)
    {
        Group* group = master->GetGroup();
        if (!group)
        {
            messages.push_back("you must be in group");
            return messages;
        }

        Group::MemberSlotList slots = group->GetMemberSlots();
        for (Group::member_citerator i = slots.begin(); i != slots.end(); i++)
        {
			ObjectGuid member = i->guid;

			if (member.GetRawValue() == master->GetObjectGuid().GetRawValue())
				continue;

			string bot;
			if (sObjectMgr.GetPlayerNameByGUID(member, bot))
			    bots.insert(bot);
        }
    }

    if (charnameStr == "!" && master && master->GetSession()->GetSecurity() > SEC_GAMEMASTER)
    {
        for (PlayerBotMap::const_iterator i = GetPlayerBotsBegin(); i != GetPlayerBotsEnd(); ++i)
        {
            Player* bot = i->second;
            if (bot && bot->IsInWorld())
                bots.insert(bot->GetName());
        }
    }

    vector<string> chars = split(charnameStr, ',');
    for (vector<string>::iterator i = chars.begin(); i != chars.end(); i++)
    {
        string s = *i;

        uint32 accountId = GetAccountId(s);
        if (!accountId)
        {
            bots.insert(s);
            continue;
        }

        auto results = CharacterDatabase.PQuery(
            "SELECT name FROM characters WHERE account = '%u'",
            accountId);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                string charName = fields[0].GetString();
                bots.insert(charName);
            } while (results->NextRow());
        }
	}

    for (set<string>::iterator i = bots.begin(); i != bots.end(); ++i)
    {
        string bot = *i;
        ostringstream out;
        out << cmdStr << ": " << bot << " - ";

        ObjectGuid member = sObjectMgr.GetPlayerGuidByName(bot);
        if (!member)
        {
            out << "character not found";
        }
        else if (master && member.GetRawValue() != master->GetObjectGuid().GetRawValue())
        {
            out << ProcessBotCommand(cmdStr, member,
                    master->GetObjectGuid(),
                    master->GetSession()->GetSecurity() >= SEC_GAMEMASTER,
                    master->GetSession()->GetAccountId(),
                    master->GetGuildId());
        }
        else if (!master)
        {
            out << ProcessBotCommand(cmdStr, member, ObjectGuid(), true, -1, -1);
        }

        messages.push_back(out.str());
    }

    return messages;
}

uint32 PlayerbotHolder::GetAccountId(string name)
{
    uint32 accountId = 0;

    auto results = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", name.c_str());
    if(results)
    {
        Field* fields = results->Fetch();
        accountId = fields[0].GetUInt32();
    }

    return accountId;
}

string PlayerbotHolder::ListBots(Player* master)
{
    set<string> bots;
    map<uint8,string> classNames;
    classNames[CLASS_DRUID] = "Druid";
    classNames[CLASS_HUNTER] = "Hunter";
    classNames[CLASS_MAGE] = "Mage";
    classNames[CLASS_PALADIN] = "Paladin";
    classNames[CLASS_PRIEST] = "Priest";
    classNames[CLASS_ROGUE] = "Rogue";
    classNames[CLASS_SHAMAN] = "Shaman";
    classNames[CLASS_WARLOCK] = "Warlock";
    classNames[CLASS_WARRIOR] = "Warrior";
#ifdef MANGOSBOT_TWO
    classNames[CLASS_DEATH_KNIGHT] = "DeathKnight";
#endif

    map<string, string> online;
    list<string> names;
    map<string, string> classes;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        string name = bot->GetName();
        bots.insert(name);

        names.push_back(name);
        online[name] = "+";
        classes[name] = classNames[bot->getClass()];
    }

    if (master)
    {
        auto results = CharacterDatabase.PQuery("SELECT class,name FROM characters where account = '%u'",
                master->GetSession()->GetAccountId());
        if (results != NULL)
        {
            do
            {
                Field* fields = results->Fetch();
                uint8 cls = fields[0].GetUInt8();
                string name = fields[1].GetString();
                if (bots.find(name) == bots.end() && name != master->GetSession()->GetPlayerName())
                {
                    names.push_back(name);
                    online[name] = "-";
                    classes[name] = classNames[cls];
                }
            } while (results->NextRow());
        }
    }

    names.sort();

    Group* group = master->GetGroup();
    if (group)
    {
        Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player *member = sObjectMgr.GetPlayer(itr->guid);
            if (member && sRandomPlayerbotMgr.IsFreeBot(member))
            {
                string name = member->GetName();

                names.push_back(name);
                online[name] = "+";
                classes[name] = classNames[member->getClass()];
            }
        }
    }

    ostringstream out;
    bool first = true;
    out << "Bot roster: ";
    for (list<string>::iterator i = names.begin(); i != names.end(); ++i)
    {
        if (first) first = false; else out << ", ";
        string name = *i;
        out << online[name] << name << " " << classes[name];
    }

    return out.str();
}


PlayerbotMgr::PlayerbotMgr(Player* const master) : PlayerbotHolder(),  master(master), lastErrorTell(0)
{
}

PlayerbotMgr::~PlayerbotMgr()
{
}

void PlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool minimal)
{
    SetAIInternalUpdateDelay(sPlayerbotAIConfig.reactDelay);
    CheckTellErrors(elapsed);
}

void PlayerbotMgr::HandleCommand(uint32 type, const string& text, uint32 lang)
{
    Player *master = GetMaster();
    if (!master)
        return;

    if (!sPlayerbotAIConfig.enabled)
        return;

    if (text.find(sPlayerbotAIConfig.commandSeparator) != string::npos)
    {
        vector<string> commands;
        split(commands, text, sPlayerbotAIConfig.commandSeparator.c_str());
        for (vector<string>::iterator i = commands.begin(); i != commands.end(); ++i)
        {
            HandleCommand(type, *i,lang);
        }
        return;
    }

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;

        if (type == CHAT_MSG_SAY)
            if (bot->GetMapId() != master->GetMapId() || sServerFacade.GetDistance2d(bot, master) > 25)
                continue;

        if (type == CHAT_MSG_YELL)
            if (bot->GetMapId() != master->GetMapId() || sServerFacade.GetDistance2d(bot, master) > 300)
                continue;

        bot->GetPlayerbotAI()->HandleCommand(type, text, *master, lang);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;

        if (type == CHAT_MSG_SAY)
            if (bot->GetMapId() != master->GetMapId() || sServerFacade.GetDistance2d(bot, master) > 25)
                continue;

        if (type == CHAT_MSG_YELL)
            if (bot->GetMapId() != master->GetMapId() || sServerFacade.GetDistance2d(bot, master) > 300)
                continue;

        if (bot->GetPlayerbotAI()->GetMaster() == master)
            bot->GetPlayerbotAI()->HandleCommand(type, text, *master, lang);
    }
}

void PlayerbotMgr::HandleMasterIncomingPacket(const WorldPacket& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterIncomingPacket(packet);
    }

    switch (packet.GetOpcode())
    {
        // if master is logging out, log out all bots
        case CMSG_LOGOUT_REQUEST:
        {
            LogoutAllBots();
            return;
        }
        // if master cancelled logout, cancel too
        case CMSG_LOGOUT_CANCEL:
        {
            CancelLogout();
            return;
        }
    }
}
void PlayerbotMgr::HandleMasterOutgoingPacket(const WorldPacket& packet)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (!bot->GetPlayerbotAI())
            continue;

        bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }

    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->GetPlayerbotAI()->HandleMasterOutgoingPacket(packet);
    }
}

void PlayerbotMgr::SaveToDB()
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        bot->SaveToDB();
    }
    for (PlayerBotMap::const_iterator it = sRandomPlayerbotMgr.GetPlayerBotsBegin(); it != sRandomPlayerbotMgr.GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (bot->GetPlayerbotAI()->GetMaster() == GetMaster())
            bot->SaveToDB();
    }
}

void PlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    bot->GetPlayerbotAI()->SetMaster(master);
    bot->GetPlayerbotAI()->ResetStrategies();
    sLog.outDebug("Bot %s logged in", bot->GetName());
}

void PlayerbotMgr::OnPlayerLogin(Player* player)
{
    // set locale priority for bot texts
    sPlayerbotTextMgr.AddLocalePriority(player->GetSession()->GetSessionDbLocaleIndex());
    sLog.outBasic("Player %s logged in, localeDbc %i, localeDb %i", player->GetName(), (uint32)(player->GetSession()->GetSessionDbcLocale()), player->GetSession()->GetSessionDbLocaleIndex());

    if(sPlayerbotAIConfig.selfBotLevel > 2 || sPlayerbotAIConfig.IsFreeAltBot(player) || sRandomPlayerbotMgr.GetValue(master->GetObjectGuid().GetCounter(), "selfbot"))
        HandlePlayerbotCommand("self", player);

    if (!sPlayerbotAIConfig.botAutologin)
        return;

    uint32 accountId = player->GetSession()->GetAccountId();
    auto results = CharacterDatabase.PQuery(
        "SELECT name FROM characters WHERE account = '%u'",
        accountId);
    if (results)
    {
        ostringstream out; out << "add ";
        bool first = true;
        do
        {
            Field* fields = results->Fetch();
            if (first) first = false; else out << ",";
            out << fields[0].GetString();
        } while (results->NextRow());

        HandlePlayerbotCommand(out.str().c_str(), player);
    }
}

void PlayerbotMgr::TellError(string botName, string text)
{
    set<string> names = errors[text];
    if (names.find(botName) == names.end())
    {
        names.insert(botName);
    }
    errors[text] = names;
}

void PlayerbotMgr::CheckTellErrors(uint32 elapsed)
{
    time_t now = time(0);
    if ((now - lastErrorTell) < sPlayerbotAIConfig.errorDelay / 1000)
        return;

    lastErrorTell = now;

    for (PlayerBotErrorMap::iterator i = errors.begin(); i != errors.end(); ++i)
    {
        string text = i->first;
        set<string> names = i->second;

        ostringstream out;
        bool first = true;
        for (set<string>::iterator j = names.begin(); j != names.end(); ++j)
        {
            if (!first) out << ", "; else first = false;
            out << *j;
        }
        out << "|cfff00000: " << text;
        
        ChatHandler(master->GetSession()).PSendSysMessage(out.str().c_str());
    }
    errors.clear();
}
