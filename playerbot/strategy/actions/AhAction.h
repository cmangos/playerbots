#pragma once
#include "GenericActions.h"

namespace ai
{
    class AhAction : public ChatCommandAction
    {
    public:
        AhAction(PlayerbotAI* ai, std::string name = "ah") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;

    protected:
        static uint32 GetPostTime();
        bool PostItem(Player* requester, Item* item, uint32 price, Unit* auctioneer, uint32 time);

    private:
        virtual bool ExecuteCommand(Player* requester, std::string text, Unit* auctioneer);

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "ah"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make bots auction items to a nearby auction houses.\n"
                "Usage: ah [itemlink] <money>\n"
                "Example: ah vendor (post items based on item use)\n"
                "Example: ah [itemlink] 5g\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs", "inventory items", "item usage", "free money for" }; }
#endif
    };

    class AhBidAction : public AhAction
    {
    public:
        AhBidAction(PlayerbotAI* ai) : AhAction(ai, "ah bid") {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "ah bid"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This command will make bots bid on a specific item with a specific budget on a nearby auctionhouse.\n"
                "The highest item/gold auction will be used that falls below the given budget.\n"
                "Usage: ah bid [itemlink] <money>\n"
                "Example: ah bid vendor (bid on items based on item use)\n"
                "Example: ah bid [itemlink] 5g\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs", "item usage", "free money for" }; }
#endif 
    private:
        virtual bool ExecuteCommand(Player* requester, std::string text, Unit* auctioneer);
        bool BidItem(Player* requester, AuctionEntry* auction, uint32 price, Unit* auctioneer, bool isBuyout, std::string reason = "");
    };

    class AuctionCommandAction : public AhAction
    {
    public:
        AuctionCommandAction(PlayerbotAI* ai) : AhAction(ai, "auction") {}
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "auction"; }
        virtual std::string GetHelpDescription()
        {
            return "This command lists, posts or cancels the bot's own auctions.\n"
                "Usage: auction (list own auctions in every auction house)\n"
                "Usage: auction add [itemlink] <money> (post at a nearby auctioneer, money is the stack buyout)\n"
                "Usage: auction remove [itemlink] or auction remove <auction id> (cancel at a nearby auctioneer)\n"
                "Example: auction add [itemlink] 5g\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "nearest npcs", "inventory items" }; }
#endif

    private:
        Unit* FindAuctioneer();
        bool ListAuctions(Player* requester);
        bool AddAuction(Player* requester, const std::string& param, Unit* auctioneer);
        bool RemoveAuctions(Player* requester, const std::string& param, Unit* auctioneer);
    };
}
