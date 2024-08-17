
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

                // default desired price if there are no item listings
                // (is the max price because why not? sounds reasonable to try selling at max price if there are no listings)
                // make it urand(max, max - 25% of maximum margin) for some randomness
                uint32 maxAhPrice = ItemUsageValue::GetBotAHSellMaxPrice(proto);
                uint32 minAhPrice = ItemUsageValue::GetBotAHSellMinPrice(proto);
                uint32 desiredPricePerItem = maxAhPrice - static_cast<uint32>((maxAhPrice - minAhPrice) * frand(0.0f, 0.25f));

                // check if it would be reasonable to sell lower than current cheapest listing
                // also check if the item poster is not self to not to undercut yourself
                if (lowestBuyoutItemPricePerItem > 0 && lowestBuyoutItemPricePerItemOwnerDbGuid != bot->GetDbGuid())
                {
                    //try to undercut by randomly 1 copper to 10% (may result in not actually undercutting but it's alright, even better)
                    uint32 undercutByMoney = std::max(static_cast<uint32>(1), static_cast<uint32>(lowestBuyoutItemPricePerItem * frand(0.0f, 0.1f)));

                    if (undercutByMoney < lowestBuyoutItemPricePerItem)
                    {
                        desiredPricePerItem = lowestBuyoutItemPricePerItem - undercutByMoney;
                    }
                    else
                    {
                        desiredPricePerItem = lowestBuyoutItemPricePerItem - 1;
                    }
                }

                //price should be reasonable, NEVER cheaper than minimal AH price
                desiredPricePerItem = std::max(minAhPrice, desiredPricePerItem);

                //store in immediate cache to use the same price for subsequent postings
                pricePerItemCache[proto->ItemId] = desiredPricePerItem;

                //resulting undercut value for reporting
                resultingUndercut = (lowestBuyoutItemPricePerItem > 0 && desiredPricePerItem < lowestBuyoutItemPricePerItem) ? lowestBuyoutItemPricePerItem - desiredPricePerItem : 0;
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
        ItemUsage usage;
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

            uint32 totalCost = std::min(auction->buyout, uint32(std::max(auction->bid, auction->startbid) * frand(1.05f, 1.25f)));

            usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(auction).GetQualifier());

            if (freeMoney.find(usage) == freeMoney.end() || totalCost > AI_VALUE2(uint32, "free money for", freeMoney[usage]))
                continue;

            uint32 power = 1;

            switch (usage)
            {
            case ItemUsage::ITEM_USAGE_EQUIP:
            case ItemUsage::ITEM_USAGE_BAD_EQUIP:
                power = sRandomItemMgr.GetLiveStatWeight(bot, auction->itemTemplate);
                break;
            case ItemUsage::ITEM_USAGE_AH:
                if (!ItemUsageValue::IsWorthBuyingFromAhToResellAtAH(sObjectMgr.GetItemPrototype(auction->itemTemplate), totalCost, auction->itemCount))
                    continue;
                power = 1000;
                break;
            case ItemUsage::ITEM_USAGE_VENDOR:
                //basically if AH price is lower than vendor sell price then it's worth it
                if (totalCost / auction->itemCount >= (int32)sObjectMgr.GetItemPrototype(auction->itemTemplate)->SellPrice)
                    continue;
                power = 1000;
                break;
            case ItemUsage::ITEM_USAGE_FORCE_NEED:
            case ItemUsage::ITEM_USAGE_FORCE_GREED:
                power = 1000;
                break;
            }

            power *= 1000;
            power /= (totalCost +1);

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

            usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(auction).GetQualifier());

            uint32 currentBidPrice = std::max(auction->bid, auction->startbid);
            uint32 currentBuyoutPrice = auction->buyout;

            bool shouldBuyout = false;

            //determine if should look at buyout or bid price depending on item usage
            uint32 price = currentBuyoutPrice;

            if (usage == ItemUsage::ITEM_USAGE_VENDOR || usage == ItemUsage::ITEM_USAGE_FORCE_GREED || usage == ItemUsage::ITEM_USAGE_NONE)
            {
                //do not care for buyout price for items that bot does not need
                price = currentBidPrice;
            }
            else if (currentBidPrice < static_cast<uint32>(currentBuyoutPrice * 0.3f) && !urand(0,1))
            {
                //if bid price < 30% of buyout, then might as well (50/50) chance consider bid price directly
                price = currentBidPrice;
            }

            //first check if has money for buyout price (if checking against buyout price)
            if (price == currentBuyoutPrice && (freeMoney.find(usage) == freeMoney.end() || price > AI_VALUE2(uint32, "free money for", freeMoney[usage])))
            {
                //check for free money for bid price next if has no money for buyout
                price = currentBidPrice;
            }

            //check if have money for bid price (if checking against bid price)
            if (price != currentBuyoutPrice && (freeMoney.find(usage) == freeMoney.end() || price > AI_VALUE2(uint32, "free money for", freeMoney[usage])))
            {
                if (!urand(0, 5))
                    break;
                else
                    continue;
            }

            freeMoney[ItemUsage::ITEM_USAGE_EQUIP] = freeMoney[ItemUsage::ITEM_USAGE_BAD_EQUIP] = (uint32)NeedMoneyFor::gear;
            freeMoney[ItemUsage::ITEM_USAGE_USE] = (uint32)NeedMoneyFor::consumables;
            freeMoney[ItemUsage::ITEM_USAGE_SKILL] = freeMoney[ItemUsage::ITEM_USAGE_DISENCHANT] = (uint32)NeedMoneyFor::tradeskill;
            freeMoney[ItemUsage::ITEM_USAGE_AMMO] = (uint32)NeedMoneyFor::ammo;
            freeMoney[ItemUsage::ITEM_USAGE_QUEST] = freeMoney[ItemUsage::ITEM_USAGE_AH] = freeMoney[ItemUsage::ITEM_USAGE_VENDOR] = freeMoney[ItemUsage::ITEM_USAGE_FORCE_NEED] = freeMoney[ItemUsage::ITEM_USAGE_FORCE_GREED] = (uint32)NeedMoneyFor::anything;

            std::map<std::string, std::string> placeholders;
            std::string reason;
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(auction).GetQualifier());

            switch (usage)
            {
            case ItemUsage::ITEM_USAGE_EQUIP:
                reason = BOT_TEXT2("for equiping as upgrade.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_BAD_EQUIP:
                reason = BOT_TEXT2("for equiping until I can find something better.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_USE:
                reason = BOT_TEXT2("to use it when I need it.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_SKILL:
            case ItemUsage::ITEM_USAGE_DISENCHANT:
                reason = BOT_TEXT2("to use it for my profession.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_AMMO:
                BOT_TEXT2("to use as ammo.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_QUEST:
                reason = BOT_TEXT2("to complete an objective for a quest.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_AH:
                placeholders["%price_min"] = ChatHelper::formatMoney(ItemUsageValue::GetBotAHSellMinPrice(sObjectMgr.GetItemPrototype(auction->itemTemplate)));
                placeholders["%price_max"] = ChatHelper::formatMoney(ItemUsageValue::GetBotAHSellMaxPrice(sObjectMgr.GetItemPrototype(auction->itemTemplate)));
                reason = BOT_TEXT2("to repost on AH for %price_min to %price_max.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_VENDOR:
                placeholders["%price"] = ChatHelper::formatMoney(sObjectMgr.GetItemPrototype(auction->itemTemplate)->SellPrice);
                reason = BOT_TEXT2("to sell to a vendor for %price.", placeholders);
                break;
            case ItemUsage::ITEM_USAGE_FORCE_NEED:
            case ItemUsage::ITEM_USAGE_FORCE_GREED:
                reason = BOT_TEXT2("because I was told to get this item.", placeholders);
                break;
            }

            bidItems = BidItem(requester, auction, price, auctioneer, price == currentBuyoutPrice, reason);

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

    return BidItem(requester, auction, cost, auctioneer, cost == auction->buyout);
}

bool AhBidAction::BidItem(Player* requester, AuctionEntry* auction, uint32 price, Unit* auctioneer, bool isBuyout, std::string reason)
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
        if (isBuyout)
        {
            out << "Buying out " << ChatHelper::formatItem(itemQualifier, count) << " for " << ChatHelper::formatMoney(price) << " on the AH";
        }
        else
        {
            out << "Bidding " << ChatHelper::formatMoney(price) << " on " << ChatHelper::formatItem(itemQualifier, count) << " on the AH";
        }
        if (!reason.empty())
            out << " " << reason;
        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }
    return false;
}