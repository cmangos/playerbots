
#include "playerbot/playerbot.h"
#include "AhAction.h"
#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/RandomItemMgr.h"
#include "playerbot/strategy/values/BudgetValues.h"

using namespace ai;

bool AhAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();

    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    for (std::list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* npc = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_AUCTIONEER);
        if (!npc)
            continue;

        if (!sRandomPlayerbotMgr.m_ahActionMutex.try_lock()) //Another bot is using the Auction right now. Try again later.
            return false;

        bool doneAuction = ExecuteCommand(requester, text, npc);

        sRandomPlayerbotMgr.m_ahActionMutex.unlock();

        return doneAuction;
    }

    ai->TellPlayerNoFacing(requester, "Cannot find auctioneer nearby");
    return false;
}

bool AhAction::ExecuteCommand(Player* requester, std::string text, Unit* auctioneer)
{
    uint32 time;
#ifdef MANGOSBOT_ZERO
    time = 8 * HOUR / MINUTE;
#else
    time = 12 * HOUR / MINUTE;
#endif

    if (text == "vendor")
    {
        AuctionHouseEntry const* auctionHouseEntry = bot->GetSession()->GetCheckedAuctionHouseForAuctioneer(auctioneer->GetObjectGuid());
        if (!auctionHouseEntry)
            return false;

        std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_AH));

        bool postedItem = false;

        std::map<uint32, uint32> pricePerItemCache;

        //resulting undercut value for reporting
        uint32 resultingUndercut = 0;

        for (auto item : items)
        {
            RESET_AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());
            if(AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier()) != ItemUsage::ITEM_USAGE_AH)
                continue;

            uint32 deposit = AuctionHouseMgr::GetAuctionDeposit(auctionHouseEntry, time, item);

            RESET_AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ah);
            uint32 freeMoney = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ah);

            if (deposit > freeMoney)
                return false;

            const ItemPrototype* proto = item->GetProto();

            if (!pricePerItemCache[proto->ItemId])
            {
                //check current AH listing prices for this item and try to set lower (undercut)

                uint32 lowestBuyoutItemPricePerItem = 0;
                uint32 lowestBuyoutItemPricePerItemOwnerDbGuid = 0;

                // check for the cheapest listing with the same count
                // (simple dumping protection, can be improved by comparing listing prices but it's a bit complicated)
                auto lowestBuyoutPriceListing = CharacterDatabase.PQuery(
                    "SELECT buyoutprice, item_count, itemowner FROM auction WHERE item_template = '%u' AND item_count = '%u' ORDER BY buyoutprice / item_count ASC LIMIT 1",
                    item->GetProto()->ItemId,
                    item->GetCount()
                );
                if (lowestBuyoutPriceListing)
                {
                    do
                    {
                        Field* fields = lowestBuyoutPriceListing->Fetch();
                        float lowestPriceTotal = static_cast<float>(fields[0].GetUInt32());
                        float lowestPriceItemCount = static_cast<float>(fields[1].GetUInt32());
                        //itemowner can be 0 I guess if item is posted by AHBot
                        lowestBuyoutItemPricePerItemOwnerDbGuid = fields[2].GetUInt32();
                        if (lowestPriceTotal > 0 && lowestPriceItemCount > 0)
                        {
                            lowestBuyoutItemPricePerItem = static_cast<uint32>(lowestPriceTotal / lowestPriceItemCount);
                        }
                    } while (lowestBuyoutPriceListing->NextRow());
                }

                //default desired price if there are no item listings
                uint32 pricePerItem = ItemUsageValue::GetBotAHSellPrice(proto, bot);

                // check if it would be reasonable to sell lower than current cheapest listing
                // also check if the item poster is not self to not to undercut yourself
                if (lowestBuyoutItemPricePerItem > 0 && lowestBuyoutItemPricePerItemOwnerDbGuid != bot->GetDbGuid())
                {
                    //try to undercut by randomly 1 copper to 1% (may result in not actually undercutting but it's alright, even better)
                    uint32 undercutByMoney = urand(
                        static_cast<uint32>(1),
                        static_cast<uint32>(lowestBuyoutItemPricePerItem * 0.01f)
                    );

                    if (undercutByMoney < lowestBuyoutItemPricePerItem)
                    {
                        pricePerItem = lowestBuyoutItemPricePerItem - undercutByMoney;
                    }
                    else
                    {
                        //no undercut if it is too high
                        pricePerItem = lowestBuyoutItemPricePerItem;
                    }
                }

                //price should be reasonable, NEVER cheaper or at vendor sell price
                pricePerItem = std::max(static_cast<uint32>(proto->SellPrice * 1.2f), pricePerItem);

                //store in immediate cache to use the same price for subsequent postings
                pricePerItemCache[proto->ItemId] = pricePerItem;

                //resulting undercut value for reporting
                resultingUndercut = (lowestBuyoutItemPricePerItem > 0 && pricePerItem < lowestBuyoutItemPricePerItem) ? lowestBuyoutItemPricePerItem - pricePerItem : 0;
            }

            postedItem |= PostItem(requester, item, pricePerItemCache[proto->ItemId] * item->GetCount(), auctioneer, time, resultingUndercut);

            if (!urand(0, 5))
                break;
        }

        return postedItem;
    }

    int pos = text.find(" ");
    if (pos == std::string::npos) return false;

    std::string priceStr = text.substr(0, pos);
    uint32 price = ChatHelper::parseMoney(priceStr);

    std::list<Item*> found = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    if (found.empty())
        return false;

    Item* item = *found.begin();

    return PostItem(requester, item, price, auctioneer, time);
}

bool AhAction::PostItem(Player* requester, Item* item, uint32 price, Unit* auctioneer, uint32 time, uint32 undercutByInformationalValue)
{
    ObjectGuid itemGuid = item->GetObjectGuid();
    ItemPrototype const* proto = item->GetProto();

    ItemQualifier itemQualifier(item);

    uint32 cnt = item->GetCount();

    WorldPacket packet;
    packet << auctioneer->GetObjectGuid();
#ifdef MANGOSBOT_TWO
    packet << (uint32)1;
#endif
    packet << itemGuid;
#ifdef MANGOSBOT_TWO
    packet << cnt;
#endif
    packet << price * 95 / 100; //bid price?
    packet << price; //buyout price?
    packet << time;

    bot->GetSession()->HandleAuctionSellItem(packet);

    if (bot->GetItemByGuid(itemGuid))
        return false;

    sPlayerbotAIConfig.logEvent(ai, "AhAction", proto->Name1, std::to_string(proto->ItemId));

    std::ostringstream out;
    if (undercutByInformationalValue > 0)
    {
        out << "Posting " << ChatHelper::formatItem(itemQualifier, cnt) << " for " << ChatHelper::formatMoney(price) << " to the AH (undercutting by " << ChatHelper::formatMoney(undercutByInformationalValue) << " per item)";
    }
    else
    {
        out << "Posting " << ChatHelper::formatItem(itemQualifier, cnt) << " for " << ChatHelper::formatMoney(price) << " to the AH";
    }
    ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    return true;
}

bool AhBidAction::ExecuteCommand(Player* requester, std::string text, Unit* auctioneer)
{
    AuctionHouseEntry const* auctionHouseEntry = bot->GetSession()->GetCheckedAuctionHouseForAuctioneer(auctioneer->GetObjectGuid());
    if (!auctionHouseEntry)
        return false;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    if (!auctionHouse)
        return false;

    AuctionHouseObject::AuctionEntryMap const& map = auctionHouse->GetAuctions();

    if (map.empty())
        return false;

    AuctionEntry* auction = nullptr;

    std::vector<std::pair<AuctionEntry*, uint32>> auctionPowers;

    if (text == "vendor")
    {
        auto data = WorldPacket();
        uint32 count, totalcount = 0;
        auctionHouse->BuildListBidderItems(data, bot, 9999, count, totalcount);

        if (totalcount > 10) //Already have 10 bids, stop.
            return false;

        std::unordered_map <ItemUsage, int32> freeMoney;

        freeMoney[ItemUsage::ITEM_USAGE_EQUIP] = freeMoney[ItemUsage::ITEM_USAGE_BAD_EQUIP] = (uint32)NeedMoneyFor::gear;
        freeMoney[ItemUsage::ITEM_USAGE_USE] = (uint32)NeedMoneyFor::consumables;
        freeMoney[ItemUsage::ITEM_USAGE_SKILL] = freeMoney[ItemUsage::ITEM_USAGE_DISENCHANT] =(uint32)NeedMoneyFor::tradeskill;
        freeMoney[ItemUsage::ITEM_USAGE_AMMO] = (uint32)NeedMoneyFor::ammo;
        freeMoney[ItemUsage::ITEM_USAGE_QUEST] = freeMoney[ItemUsage::ITEM_USAGE_AH] = freeMoney[ItemUsage::ITEM_USAGE_VENDOR] = freeMoney[ItemUsage::ITEM_USAGE_FORCE_NEED] = freeMoney[ItemUsage::ITEM_USAGE_FORCE_GREED] = (uint32)NeedMoneyFor::anything;

        uint32 checkNumAuctions = urand(50, 250);

        for (uint32 i = 0; i < checkNumAuctions; i++)
        {
            auto curAuction = std::next(std::begin(map), urand(0, map.size()-1));

            auction = curAuction->second;

            if (!auction)
                continue;

            auction = auctionHouse->GetAuction(auction->Id);

            if (!auction)
                continue;

            if (auction->owner == bot->GetGUIDLow())
                continue;

            uint32 cost = std::min(auction->buyout, uint32(std::max(auction->bid, auction->startbid) * frand(1.05f, 1.25f)));

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(auction).GetQualifier());

            if (freeMoney.find(usage) == freeMoney.end() || cost > AI_VALUE2(uint32, "free money for", freeMoney[usage]))
                continue;

            uint32 power = 1;

            switch (usage)
            {
            case ItemUsage::ITEM_USAGE_EQUIP:
            case ItemUsage::ITEM_USAGE_BAD_EQUIP:
                power = sRandomItemMgr.GetLiveStatWeight(bot, auction->itemTemplate);
                break;
            case ItemUsage::ITEM_USAGE_AH:
                if (cost >= (int32)ItemUsageValue::GetBotAHSellPrice(sObjectMgr.GetItemPrototype(auction->itemTemplate), bot))
                    continue;
                power = 1000;
                break;
            case ItemUsage::ITEM_USAGE_VENDOR:
                if (cost >= (int32)sObjectMgr.GetItemPrototype(auction->itemTemplate)->SellPrice)
                    continue;
                power = 1000;
                break;
            case ItemUsage::ITEM_USAGE_FORCE_NEED:
            case ItemUsage::ITEM_USAGE_FORCE_GREED:
                power = 1000;
                break;
            }

            power *= 1000;
            power /= (cost+1);

            auctionPowers.push_back(std::make_pair(auction, power));
        }

        std::sort(auctionPowers.begin(), auctionPowers.end(), [](std::pair<AuctionEntry*, uint32> i, std::pair<AuctionEntry*, uint32> j) {return i > j; });

        bool bidItems = false;

        for (auto auctionPower : auctionPowers)
        {
            auction = auctionPower.first;

            if (!auction)
                continue;

            auction = auctionHouse->GetAuction(auction->Id);

            if (!auction)
                continue;

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(auction).GetQualifier());

            uint32 price = std::min(auction->buyout, uint32(std::max(auction->bid, auction->startbid) * frand(1.05f, 1.25f)));

            if (freeMoney.find(usage) == freeMoney.end() || price > AI_VALUE2(uint32, "free money for", freeMoney[usage]))
            {
                if (!urand(0, 5))
                    break;
                else
                    continue;
            }

            bidItems = BidItem(requester, auction, price, auctioneer);
                
            if (bidItems)
                totalcount++;

            if (!urand(0, 5) || totalcount > 10)
                break;

            RESET_AI_VALUE2(uint32, "free money for", freeMoney[usage]);
        }

        return bidItems;
    }

    int pos = text.find(" ");
    if (pos == std::string::npos) return false;

    std::string priceStr = text.substr(0, pos);
    uint32 price = ChatHelper::parseMoney(priceStr);

    for (auto curAuction : map)
    {
        auction = curAuction.second;

        if (auction->owner == bot->GetGUIDLow())
            continue;

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(auction->itemTemplate);

        if (!proto)
            continue;

        if(!proto->Name1)
            continue;
        
        if (!strstri(proto->Name1, text.c_str()))
            continue;

        if (price && auction->bid + 5 > price)
            continue;

        uint32 cost = std::min(auction->buyout, uint32(std::max(auction->bid, auction->startbid) * frand(1.05f, 1.25f)));

        uint32 power = auction->itemCount;
        power *= 1000;
        power /= cost;

        auctionPowers.push_back(std::make_pair(auction, power));
    }

    if (auctionPowers.empty())
        return false;

    std::sort(auctionPowers.begin(), auctionPowers.end(), [](std::pair<AuctionEntry*, uint32> i, std::pair<AuctionEntry*, uint32> j) {return i > j; });

    auction = auctionPowers.begin()->first;

    uint32 cost = std::min(auction->buyout, uint32(std::max(auction->bid, auction->startbid) * frand(1.05f, 1.25f)));

    return BidItem(requester, auction, cost, auctioneer);
}

bool AhBidAction::BidItem(Player* requester, AuctionEntry* auction, uint32 price, Unit* auctioneer)
{
    AuctionHouseEntry const* auctionHouseEntry = bot->GetSession()->GetCheckedAuctionHouseForAuctioneer(auctioneer->GetObjectGuid());
    if (!auctionHouseEntry)
        return false;

    // always return pointer
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    if (!auctionHouse)
        return false;

    auction = auctionHouse->GetAuction(auction->Id);

    if (!auction)
        return false;

    WorldPacket packet;
    packet << auctioneer->GetObjectGuid();
    packet << auction->Id;
    packet << price;

    uint32 oldMoney = bot->GetMoney();
    ItemQualifier itemQualifier(auction);
    uint32 count = auction->itemCount;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(auction->itemTemplate);

    bot->GetSession()->HandleAuctionPlaceBid(packet);

    if (bot->GetMoney() < oldMoney)
    {
        sPlayerbotAIConfig.logEvent(ai, "AhBidAction", proto->Name1, std::to_string(proto->ItemId));
        std::ostringstream out;
        out << "Bidding " << ChatHelper::formatMoney(price) << " on " << ChatHelper::formatItem(itemQualifier, count) << " on the AH";
        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }
    return false;
}