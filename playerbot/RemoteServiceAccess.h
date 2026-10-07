#pragma once

class Player;
class PlayerbotAI;

namespace ai
{
    enum class RemoteService { Auction, Mail, Bank };

    // Grants one service to one bot on the calling thread for a synchronous
    // transaction only. Never changes session security or grants GM privileges.
    class RemoteServiceAccess
    {
    public:
        RemoteServiceAccess(PlayerbotAI* ai, RemoteService service);
        ~RemoteServiceAccess();
        RemoteServiceAccess(const RemoteServiceAccess&) = delete;
        RemoteServiceAccess& operator=(const RemoteServiceAccess&) = delete;

        static bool HasCoreBridge();
        static bool IsEnabledFor(PlayerbotAI* ai);
        static bool IsUsableNow(PlayerbotAI* ai);
        static bool IsAllowed(Player* player, RemoteService service);

    private:
        bool active = false;
        Player* previousOwner = nullptr;
        RemoteService previousService = RemoteService::Bank;
        Player* auctionPlayer = nullptr;
        int previousAuctionMode = 0;
        static thread_local Player* owner;
        static thread_local RemoteService service;
    };
}
