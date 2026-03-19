#include "playerbot/playerbot.h"
#include "GuildShareAhBuyAction.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/ServerFacade.h"
#include "AuctionHouse/AuctionHouseMgr.h"
#include "Guilds/GuildMgr.h"
#include "Mails/Mail.h"

using namespace ai;

bool GuildShareAhBuyAction::isUseful()
{
    if (!bot->GetGuildId())
        return false;

    if (bot->IsInCombat())
        return false;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return false;

    if (!FindNearbyAuctioneer())
        return false;

    uint32 money = bot->GetMoney();
    if (money < 100)
        return false;

    if (AI_VALUE(uint8, "bag space") > 90)
        return false;

    return true;
}

Unit* GuildShareAhBuyAction::FindNearbyAuctioneer()
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

bool GuildShareAhBuyAction::CanBotCraftItem(uint32 itemId)
{
    for (auto& [spellId, spellState] : bot->GetSpellMap())
    {
        if (spellState.state == PLAYERSPELL_REMOVED || spellState.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (pSpellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM && pSpellInfo->EffectItemType[i] == itemId)
                return true;
        }
    }
    return false;
}

bool GuildShareAhBuyAction::CanBotUseReagent(ItemPrototype const* reagentProto)
{
    if (!reagentProto)
        return false;

    static const SkillType craftSkills[] = {
        SKILL_ALCHEMY,
        SKILL_BLACKSMITHING,
        SKILL_ENGINEERING,
        SKILL_LEATHERWORKING,
        SKILL_TAILORING,
        SKILL_ENCHANTING,
        SKILL_COOKING,
        SKILL_FIRST_AID
    };

    for (SkillType skill : craftSkills)
    {
        if (ai->HasSkill(skill) && ItemUsageValue::IsItemUsedBySkill(reagentProto, skill))
            return true;
    }

    return false;
}

uint32 GuildShareAhBuyAction::CountMailboxItems(uint32 itemId)
{
    uint32 count = 0;
    time_t curTime = time(nullptr);

    for (PlayerMails::iterator itr = bot->GetMailBegin(); itr != bot->GetMailEnd(); ++itr)
    {
        Mail* mail = *itr;
        if (!mail || mail->state == MAIL_STATE_DELETED || curTime < mail->deliver_time)
            continue;

        if (!mail->has_items)
            continue;

        for (MailItemInfoVec::const_iterator itemItr = mail->items.begin(); itemItr != mail->items.end(); ++itemItr)
        {
            if (itemItr->item_template == itemId)
            {
                Item* mailItem = bot->GetMItem(itemItr->item_guid);
                count += mailItem ? mailItem->GetCount() : 1;
            }
        }
    }

    return count;
}

std::map<uint32, uint32> GuildShareAhBuyAction::GetNeededItems()
{
    std::map<uint32, uint32> needed;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return needed;

    std::set<uint32> finishedItemIds;
    for (const auto& entry : shareList)
        finishedItemIds.insert(entry.itemId);

    for (uint32 itemId : finishedItemIds)
    {
        uint32 guildDeficit = CountGuildFinishedItemDeficit(bot, itemId, shareList);
        if (guildDeficit == 0)
            continue;

        uint32 botOwned = ai->GetInventoryItemsCountWithId(itemId) + CountMailboxItems(itemId);
        uint32 finishedRemaining = (botOwned >= guildDeficit) ? 0 : guildDeficit - botOwned;

        if (finishedRemaining > 0)
        {
            needed[itemId] += finishedRemaining;
        }

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        std::vector<std::pair<uint32, uint32>> reagents = ItemUsageValue::GetAllReagentItemIdsForCraftingItem(proto);

        if (reagents.empty() || !CanBotCraftItem(itemId))
            continue;

        for (const auto& [reagentId, reagentPerCraft] : reagents)
        {
            ItemPrototype const* reagentProto = sObjectMgr.GetItemPrototype(reagentId);
            if (!reagentProto)
                continue;

            if (ItemUsageValue::IsItemSoldByAnyVendor(reagentProto))
                continue;

            if (!CanBotUseReagent(reagentProto))
                continue;

            uint32 totalReagentNeeded = reagentPerCraft * guildDeficit;
            uint32 reagentOwned = ai->GetInventoryItemsCountWithId(reagentId) + CountMailboxItems(reagentId);
            if (reagentOwned >= totalReagentNeeded)
                continue;

            needed[reagentId] += totalReagentNeeded - reagentOwned;
        }
    }

    return needed;
}

bool GuildShareAhBuyAction::Execute(Event& event)
{
    Unit* auctioneer = FindNearbyAuctioneer();
    if (!auctioneer)
        return false;

    if (!sRandomPlayerbotMgr.m_ahActionMutex.try_lock())
        return false;

    bool bought = false;

    AuctionHouseEntry const* ahEntry = AuctionHouseMgr::GetAuctionHouseEntry(auctioneer);
    if (!ahEntry)
    {
        sRandomPlayerbotMgr.m_ahActionMutex.unlock();
        return false;
    }

    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(ahEntry);
    if (!auctionHouse)
    {
        sRandomPlayerbotMgr.m_ahActionMutex.unlock();
        return false;
    }

    std::map<uint32, uint32> neededItems = GetNeededItems();
    if (neededItems.empty())
    {
        sRandomPlayerbotMgr.m_ahActionMutex.unlock();
        return false;
    }

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    std::set<uint32> finishedItemIds;
    for (const auto& entry : shareList)
        finishedItemIds.insert(entry.itemId);

    uint32 availableBudget = AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild));

    struct AuctionCandidate
    {
        AuctionEntry* auction;
        uint32 itemId;
        uint32 buyout;
        uint32 count;
        bool isFinishedItem;
    };

    AuctionCandidate bestCandidate = { nullptr, 0, 0, 0, false };
    uint32 bestPricePerItem = std::numeric_limits<uint32>::max();

    AuctionHouseObject::AuctionEntryMapBounds bounds = auctionHouse->GetAuctionsBounds();
    for (auto itr = bounds.first; itr != bounds.second; ++itr)
    {
        AuctionEntry* auction = itr->second;
        if (!auction || auction->buyout == 0)
            continue; // Skip auctions with no buyout

        if (auction->owner == bot->GetGUIDLow())
            continue;

        Item* item = sAuctionMgr.GetAItem(auction->itemGuidLow);
        if (!item)
            continue;

        uint32 auctionItemId = item->GetProto()->ItemId;
        uint32 auctionCount = item->GetCount();

        auto it = neededItems.find(auctionItemId);
        if (it == neededItems.end())
            continue;

        // Don't buy more than we actually need
        if (auctionCount > it->second)
            continue;

        if (auction->buyout > availableBudget)
            continue;

        ItemPosCountVec dest;
        InventoryResult msg = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, auctionItemId, auctionCount);
        if (msg != EQUIP_ERR_OK)
            continue;

        bool isFinished = finishedItemIds.count(auctionItemId) > 0;
        uint32 pricePerItem = auction->buyout / auctionCount;

        // Prioritize items over crafting materials
        if (bestCandidate.auction)
        {
            if (bestCandidate.isFinishedItem && !isFinished)
                continue;

            if (!bestCandidate.isFinishedItem && isFinished)
            {
                bestPricePerItem = pricePerItem;
                bestCandidate = { auction, auctionItemId, auction->buyout, auctionCount, isFinished };
                continue;
            }
        }

        if (pricePerItem < bestPricePerItem)
        {
            bestPricePerItem = pricePerItem;
            bestCandidate = { auction, auctionItemId, auction->buyout, auctionCount, isFinished };
        }
    }

    if (bestCandidate.auction)
    {
        AuctionEntry* auction = bestCandidate.auction;

        auction->UpdateBid(auction->buyout, bot);

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(bestCandidate.itemId);
        std::ostringstream out;
        out << "Bought " << bestCandidate.count << "x ";
        if (proto)
            out << proto->Name1;
        else
            out << "item #" << bestCandidate.itemId;
        out << " from AH for guild share list (" << (bestCandidate.buyout / 10000) << "g)";

        ai->TellPlayerNoFacing(GetMaster(), out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        bought = true;
    }

    sRandomPlayerbotMgr.m_ahActionMutex.unlock();
    return bought;
}