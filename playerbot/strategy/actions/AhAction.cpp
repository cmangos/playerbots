
#include "playerbot/playerbot.h"
#include "AhAction.h"
#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/RandomItemMgr.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

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

uint32 AhAction::GetPostTime()
{
#ifdef MANGOSBOT_ZERO
    return 8 * HOUR / MINUTE;
#else
    return 12 * HOUR / MINUTE;
#endif
}

bool AhAction::ExecuteCommand(Player* requester, std::string text, Unit* auctioneer)
{
    uint32 time = GetPostTime();

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
        uint32 postedItems = 0;

        for (auto item : items)
        {
            RESET_AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());
            if(AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier()) != ItemUsage::ITEM_USAGE_AH)
                continue;

            auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "IsMoreProfitableToSellToAHThanToVendor", ai);
            bool isMoreProfitableToSellToAHThanToVendor = ItemUsageValue::IsMoreProfitableToSellToAHThanToVendor(item->GetProto(), bot);
            pmo.reset();

            if (!isMoreProfitableToSellToAHThanToVendor)
                continue;

            uint32 deposit = AuctionHouseMgr::GetAuctionDeposit(auctionHouseEntry, time * MINUTE, item);

            RESET_AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ah);
            uint32 freeMoney = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ah);

            if (deposit > freeMoney)
                return false;

            const ItemPrototype* proto = item->GetProto();

            if (!pricePerItemCache[proto->ItemId])
            {
                uint32 basePerItem = ItemUsageValue::GetBotSellPrice(proto, bot);
                uint32 initialPricePercentage = urand(75, 100);
                uint32 pricePerItem = (basePerItem * initialPricePercentage) / 100;
                if (!pricePerItem)
                    pricePerItem = 1;
                pricePerItemCache[proto->ItemId] = pricePerItem;
            }

            bool didPost = PostItem(requester, item, pricePerItemCache[proto->ItemId] * item->GetCount(), auctioneer, time);

            if (didPost)
            {
                    postedItem |= true;
                    postedItems++;
            }

            if (!urand(0, 5 + (items.size()- postedItems)/10))
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

bool AhAction::PostItem(Player* requester, Item* item, uint32 price, Unit* auctioneer, uint32 time)
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
    packet << uint32(uint64(price) * 95 / 100); //bid price?
    packet << price; //buyout price?
    packet << time;

    bot->GetSession()->HandleAuctionSellItem(packet);

    if (bot->GetItemByGuid(itemGuid))
        return false;

    sPlayerbotAIConfig.logEvent(ai, "AhAction", proto->Name1, std::to_string(proto->ItemId));

    std::ostringstream out;
    out << "Posting " << ChatHelper::formatItem(itemQualifier, cnt) << " for " << ChatHelper::formatMoney(price) << " to the AH";
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

        uint32 checkNumAuctions = map.size();
        if (!sPlayerbotAIConfig.botCheckAllAuctionListings)
        {
            checkNumAuctions = urand(50, 250);
        }

        for (uint32 i = 0; i < checkNumAuctions; i++)
        {
            auto curAuction = std::next(std::begin(map), urand(0, map.size()-1));

            auction = curAuction->second;

            if (!auction)
                continue;

            if (std::find_if(auctionPowers.begin(), auctionPowers.end(), [auction](std::pair<AuctionEntry*, uint32> i){return i.first == auction;}) != auctionPowers.end())
                continue;

            auction = auctionHouse->GetAuction(auction->Id);

            if (!auction)
                continue;

            if (auction->owner == bot->GetGUIDLow())
                continue;

            // Resolved once for the whole iteration, so every use below is
            // visibly covered. itemCount is the load-bearing half: a malformed
            // row carrying 0 reaches `totalCost / itemCount` in the vendor arm
            // and the same division inside IsWorthBuyingFromAhToResellAtAH. The
            // prototype can be null once the item leaves item_template while its
            // auction lives on, and both arms dereference it.
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(auction->itemTemplate);
            if (!proto || !auction->itemCount)
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
            {
                auto pmo = sPerformanceMonitor.start(PERF_MON_VALUE, "IsWorthBuyingFromAhToResellAtAH", ai);
                bool isWorthBuyingFromAhToResellAtAH = ItemUsageValue::IsWorthBuyingFromAhToResellAtAH(proto, totalCost, auction->itemCount);
                pmo.reset();

                if (!isWorthBuyingFromAhToResellAtAH)
                    continue;
                power = 1000;
                break;
            }
            case ItemUsage::ITEM_USAGE_VENDOR:
                //basically if AH price is lower than vendor sell price then it's worth it
                if (totalCost / auction->itemCount >= (int32)proto->SellPrice)
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
         
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(auction).GetQualifier());

            std::string reason = ItemUsageValue::ReasonForNeed(usage, auction, auction->itemCount, bot);            

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

    // Dereferenced unguarded below. Checked here rather than relying on the
    // callers, because the auction is re-fetched from the auction house above
    // and this is the frame that reads the prototype.
    if (!proto)
        return false;

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

static std::string TrimAuctionText(const std::string& text)
{
    size_t begin = text.find_first_not_of(' ');
    if (begin == std::string::npos)
        return "";

    size_t end = text.find_last_not_of(' ');
    return text.substr(begin, end - begin + 1);
}

static uint32 ParseAuctionPrice(const std::string& text)
{
    uint64 copper = 0;
    uint64 accumulated = 0;
    bool hasUnit = false;
    bool hasDigits = false;

    for (char c : text)
    {
        if (std::isdigit((unsigned char)c))
        {
            accumulated = std::min<uint64>(accumulated * 10 + (c - '0'), MAX_MONEY_AMOUNT);
            hasDigits = true;
        }
        else if (c == 'g' || c == 'G')
        {
            copper += accumulated * GOLD;
            accumulated = 0;
            hasUnit = true;
        }
        else if (c == 's' || c == 'S')
        {
            copper += accumulated * SILVER;
            accumulated = 0;
            hasUnit = true;
        }
        else if (c == 'c' || c == 'C')
        {
            copper += accumulated;
            accumulated = 0;
            hasUnit = true;
        }
        else if (c != ' ')
            return 0;
    }

    if (!hasDigits)
        return 0;

    copper += hasUnit ? accumulated : accumulated * GOLD;
    return uint32(std::min<uint64>(copper, MAX_MONEY_AMOUNT));
}

bool AuctionCommandAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = TrimAuctionText(event.getParam());

    std::string command = param;
    std::string rest;
    size_t space = param.find(' ');
    if (space != std::string::npos)
    {
        command = param.substr(0, space);
        rest = TrimAuctionText(param.substr(space + 1));
    }
    std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    if (param.empty() || command == "list")
        return ListAuctions(requester);

    bool add = command == "add";
    bool cancel = command == "remove" || command == "cancel";
    if ((!add && !cancel) || rest.empty())
    {
        ai->TellPlayerNoFacing(requester, "Usage: auction | auction add [item] <money> | auction remove [item] or <auction id>", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return false;
    }

    Unit* auctioneer = FindAuctioneer();
    if (!auctioneer)
    {
        ai->TellPlayerNoFacing(requester, add ? "I need to be near an auctioneer to post an auction" : "I need to be near an auctioneer to cancel an auction", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return false;
    }

    std::unique_lock<std::mutex> guard(sRandomPlayerbotMgr.m_ahActionMutex, std::try_to_lock);
    if (!guard.owns_lock())
    {
        ai->TellPlayerNoFacing(requester, "The auction house is busy, try again.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return false;
    }

    if (add)
        return AddAuction(requester, rest, auctioneer);

    return RemoveAuctions(requester, rest, auctioneer);
}

Unit* AuctionCommandAction::FindAuctioneer()
{
    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs");
    for (auto& guid : npcs)
    {
        Unit* npc = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_AUCTIONEER);
        if (npc)
            return npc;
    }
    return nullptr;
}

bool AuctionCommandAction::ListAuctions(Player* requester)
{
    static const char* houseNames[MAX_AUCTION_HOUSE_TYPE] = { "Alliance", "Horde", "Neutral" };

    std::vector<std::string> lines;
    time_t now = time(nullptr);

    {
        std::unique_lock<std::mutex> guard(sRandomPlayerbotMgr.m_ahActionMutex, std::try_to_lock);
        if (!guard.owns_lock())
        {
            ai->TellPlayerNoFacing(requester, "The auction house is busy, try again.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            return false;
        }

        for (uint32 houseType = 0; houseType < MAX_AUCTION_HOUSE_TYPE; ++houseType)
        {
            AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(AuctionHouseType(houseType));
            if (!auctionHouse)
                continue;

            for (auto const& entry : auctionHouse->GetAuctions())
            {
                AuctionEntry* auction = entry.second;
                if (!auction || auction->owner != bot->GetGUIDLow())
                    continue;

                ItemQualifier itemQualifier(auction);
                std::ostringstream out;
                out << "#" << auction->Id << " " << ChatHelper::formatItem(itemQualifier, auction->itemCount);

#ifdef MANGOSBOT_TWO
                if (auction->moneyDeliveryTime)
                {
                    out << " sold for " << ChatHelper::formatMoney(auction->bid) << ", payment pending (" << houseNames[houseType] << ")";
                    lines.push_back(out.str());
                    continue;
                }
#endif

                if (auction->bid)
                    out << " bid " << ChatHelper::formatMoney(auction->bid);
                else
                    out << " min bid " << ChatHelper::formatMoney(auction->startbid);

                if (auction->buyout)
                    out << ", buyout " << ChatHelper::formatMoney(auction->buyout);

                if (auction->expireTime > now)
                    out << ", " << secsToTimeString(auction->expireTime - now, true) << " left";
                else
                    out << ", expiring";

                out << " (" << houseNames[houseType] << ")";
                lines.push_back(out.str());
            }
        }
    }

    if (lines.empty())
    {
        ai->TellPlayerNoFacing(requester, "I have no active auctions", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return true;
    }

    ai->TellPlayerNoFacing(requester, "My active auctions (" + std::to_string(lines.size()) + "):", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    const size_t maxLines = 15;
    for (size_t i = 0; i < lines.size() && i < maxLines; ++i)
        ai->TellPlayerNoFacing(requester, lines[i], PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);

    if (lines.size() > maxLines)
        ai->TellPlayerNoFacing(requester, "...and " + std::to_string(lines.size() - maxLines) + " more", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);

    return true;
}

bool AuctionCommandAction::AddAuction(Player* requester, const std::string& param, Unit* auctioneer)
{
    std::string itemText = param;
    uint32 price = 0;

    size_t linkEnd = param.rfind("|r");
    if (linkEnd != std::string::npos)
    {
        itemText = param.substr(0, linkEnd + 2);
        std::string priceText = TrimAuctionText(param.substr(linkEnd + 2));
        price = ParseAuctionPrice(priceText);
        if (!priceText.empty() && !price)
        {
            ai->TellPlayerNoFacing(requester, "Price must look like 5 (gold), 5g, 50s or 5g50s20c", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            return false;
        }
    }
    else
    {
        size_t lastSpace = param.rfind(' ');
        if (lastSpace != std::string::npos)
        {
            uint32 trailingPrice = ParseAuctionPrice(param.substr(lastSpace + 1));
            if (trailingPrice)
            {
                price = trailingPrice;
                itemText = TrimAuctionText(param.substr(0, lastSpace));
            }
        }
    }

    std::list<Item*> found = ai->InventoryParseItems(itemText, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    if (found.empty())
    {
        ai->TellPlayerNoFacing(requester, "I don't have that item in my bags", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return false;
    }

    Item* item = found.front();
    ItemPrototype const* proto = item->GetProto();
    if (!proto)
        return false;

    AuctionHouseEntry const* auctionHouseEntry = bot->GetSession()->GetCheckedAuctionHouseForAuctioneer(auctioneer->GetObjectGuid());
    if (!auctionHouseEntry)
        return false;

    if (!price)
    {
        uint64 pricePerItem = (uint64(ItemUsageValue::GetBotSellPrice(proto, bot)) * urand(75, 100)) / 100;
        if (!pricePerItem)
            pricePerItem = 1;
        price = uint32(std::min<uint64>(pricePerItem * item->GetCount(), MAX_MONEY_AMOUNT));
    }

    if (price < 2)
        price = 2;

    uint32 time = GetPostTime();
    uint32 deposit = AuctionHouseMgr::GetAuctionDeposit(auctionHouseEntry, time * MINUTE, item);
    if (deposit > bot->GetMoney())
    {
        std::ostringstream out;
        out << "I need " << ChatHelper::formatMoney(deposit) << " for the deposit on " << ChatHelper::formatItem(item, item->GetCount());
        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return false;
    }

    ItemQualifier itemQualifier(item);
    uint32 count = item->GetCount();

    if (PostItem(requester, item, price, auctioneer, time))
        return true;

    std::ostringstream out;
    out << "I could not post " << ChatHelper::formatItem(itemQualifier, count) << " to the AH";
    ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return false;
}

bool AuctionCommandAction::RemoveAuctions(Player* requester, const std::string& param, Unit* auctioneer)
{
    AuctionHouseEntry const* auctionHouseEntry = bot->GetSession()->GetCheckedAuctionHouseForAuctioneer(auctioneer->GetObjectGuid());
    if (!auctionHouseEntry)
        return false;

    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);
    if (!auctionHouse)
        return false;

    std::set<uint32> itemIds = ChatHelper::ExtractAllItemIds(param);
    uint32 auctionId = 0;
    if (itemIds.empty())
    {
        std::string idText = param[0] == '#' ? param.substr(1) : param;
        if (idText.empty() || idText.size() > 9 || !std::all_of(idText.begin(), idText.end(), [](unsigned char c) { return std::isdigit(c); }))
        {
            ai->TellPlayerNoFacing(requester, "Usage: auction remove [item] or auction remove <auction id>", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
            return false;
        }
        auctionId = std::stoul(idText);
    }

    std::vector<uint32> auctionIds;
    std::vector<std::string> bidAuctions;
    for (auto const& entry : auctionHouse->GetAuctions())
    {
        AuctionEntry* auction = entry.second;
        if (!auction || auction->owner != bot->GetGUIDLow())
            continue;

#ifdef MANGOSBOT_TWO
        if (auction->moneyDeliveryTime)
            continue;
#endif

        if (auctionId ? auction->Id != auctionId : itemIds.find(auction->itemTemplate) == itemIds.end())
            continue;

        if (auction->bid)
        {
            ItemQualifier itemQualifier(auction);
            std::ostringstream out;
            out << "Auction #" << auction->Id << " " << ChatHelper::formatItem(itemQualifier, auction->itemCount) << " has a bid, I can't cancel it";
            bidAuctions.push_back(out.str());
            continue;
        }

        auctionIds.push_back(auction->Id);
    }

    for (const std::string& line : bidAuctions)
        ai->TellPlayerNoFacing(requester, line, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);

    if (auctionIds.empty())
    {
        if (bidAuctions.empty())
            ai->TellPlayerNoFacing(requester, "I have no matching auction at this auction house", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        return false;
    }

    bool removed = false;
    for (uint32 id : auctionIds)
    {
        AuctionEntry* auction = auctionHouse->GetAuction(id);
        if (!auction || auction->bid)
            continue;

        ItemQualifier itemQualifier(auction);
        uint32 count = auction->itemCount;

        WorldPacket packet;
        packet << auctioneer->GetObjectGuid();
        packet << id;

        bot->GetSession()->HandleAuctionRemoveItem(packet);

        std::ostringstream out;
        if (!auctionHouse->GetAuction(id))
        {
            removed = true;
            out << "Cancelled auction #" << id << " " << ChatHelper::formatItem(itemQualifier, count) << ", the item is coming back by mail";
        }
        else
            out << "I could not cancel auction #" << id << " " << ChatHelper::formatItem(itemQualifier, count);
        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    }

    return removed;
}