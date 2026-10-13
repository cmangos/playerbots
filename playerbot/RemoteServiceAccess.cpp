#include "playerbot/playerbot.h"
#include "RemoteServiceAccess.h"
#include "Server/WorldSession.h"

using namespace ai;

thread_local Player* RemoteServiceAccess::owner = nullptr;
thread_local RemoteService RemoteServiceAccess::service = RemoteService::Bank;

bool RemoteServiceAccess::HasCoreBridge()
{
#if defined(CMANGOS_PLAYERBOT_REMOTE_SERVICES) && CMANGOS_PLAYERBOT_REMOTE_SERVICES == 1
    return true;
#else
    // The ordinary session handlers reject remote AH/mail without the core
    // bridge. Retain world interactions instead of silently losing purchases.
    return false;
#endif
}

bool RemoteServiceAccess::IsEnabledFor(PlayerbotAI* ai)
{
    return HasCoreBridge() && ai && sPlayerbotAIConfig.randomBotRemoteServices && ai->GetBot() &&
        !ai->IsRealPlayer() && !ai->HasRealPlayerMaster() &&
        sRandomPlayerbotMgr.IsRandomBot(ai->GetBot()) &&
        sRandomPlayerbotMgr.IsFreeBot(ai->GetBot());
}

bool RemoteServiceAccess::IsUsableNow(PlayerbotAI* ai)
{
    if (!IsEnabledFor(ai))
        return false;
    Player* player = ai->GetBot();
    return player->IsAlive() && !player->IsInCombat() && !player->IsTaxiFlying() &&
        !player->IsBeingTeleported() && !player->IsNonMeleeSpellCasted(false) &&
        !player->GetTradeData() && !player->GetLootGuid();
}

RemoteServiceAccess::RemoteServiceAccess(PlayerbotAI* ai, RemoteService requested)
{
    if (!IsUsableNow(ai))
        return;
    active = true;
    previousOwner = owner;
    previousService = service;
    owner = ai->GetBot();
    service = requested;
    if (requested == RemoteService::Auction)
    {
        auctionPlayer = owner;
        previousAuctionMode = owner->GetAuctionAccessMode();
        // The existing core self-player route selects the bot's own faction
        // and respects the realm's shared-AH setting. Restore on every exit.
        owner->SetAuctionAccessMode(0);
    }
}

RemoteServiceAccess::~RemoteServiceAccess()
{
    if (!active)
        return;
    if (auctionPlayer)
        auctionPlayer->SetAuctionAccessMode(previousAuctionMode);
    owner = previousOwner;
    service = previousService;
}

bool RemoteServiceAccess::IsAllowed(Player* player, RemoteService requested)
{
    return player && player == owner && service == requested &&
        IsUsableNow(player->GetPlayerbotAI());
}
