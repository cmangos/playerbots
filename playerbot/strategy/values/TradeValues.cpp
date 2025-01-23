
#include "playerbot/playerbot.h"
#include "TradeValues.h"
#include "ItemUsageValue.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/RandomItemMgr.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool ItemsUsefulToGiveValue::IsTradingItem(uint32 entry)
{
    TradeData* trade = bot->GetTradeData();

    if (!trade)
        return false;

    for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT; i++)
    {
        Item* tradeItem = trade->GetItem(TradeSlots(i));

        if (tradeItem && tradeItem->GetEntry() == entry)
            return true;;
    }

    return false;
}

std::list<Item*> ItemsUsefulToGiveValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");
    Player* player = guidP.GetPlayer();

    std::list<Item*> giveItems;

    if (ai->HasActivePlayerMaster() || !player->GetPlayerbotAI())
        return giveItems;

    // Define valid usages once
    std::unordered_set<ItemUsage> validUsages = {
        ItemUsage::ITEM_USAGE_NONE,
        ItemUsage::ITEM_USAGE_VENDOR,
        ItemUsage::ITEM_USAGE_AH,
        ItemUsage::ITEM_USAGE_BROKEN_AH,
        ItemUsage::ITEM_USAGE_DISENCHANT
    };

    // Retrieve all inventory items at once
    std::list<Item*> inventoryItems = AI_VALUE(std::list<Item*>, "inventory items");

    TradeData* trade = bot->GetTradeData();

    for (auto& item : inventoryItems)
    {
        if (!item->CanBeTraded() || item->IsEquipped())
            continue;

        if (trade)
        {
            if (trade->HasItem(item->GetObjectGuid()) || IsTradingItem(item->GetEntry()))
                continue;

            // Check if a similar item is already in the give list
            if (std::any_of(giveItems.begin(), giveItems.end(), [item](Item* i) {
                return i->GetEntry() == item->GetEntry();
                }))
                continue;
        }

        // Determine the item's usage
        ItemUsage itemUsage = PAI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());

        // Add to the list if usage is valid
        if (validUsages.find(itemUsage) == validUsages.end())
            giveItems.push_back(item);
    }

    return giveItems;
}

std::list<Item*> ItemsUsefulToEnchantValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    Player* player = guidP.GetPlayer();

    std::list<Item*> enchantItems;

    if (ai->HasActivePlayerMaster() || !player->GetPlayerbotAI())
        return enchantItems;

    std::vector<uint32> enchantSpells = AI_VALUE(std::vector<uint32>, "enchant spells");

    for (auto& spellId : enchantSpells)
    {
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

        if (!pSpellInfo)
            continue;

        uint32 castCount = AI_VALUE2(uint32, "has reagents for", spellId);

        if (!castCount)
            continue;

        Item* item = PAI_VALUE2(Item*, "item for spell", spellId);

        if (!item)
            continue;

        if (PHAS_AI_VALUE2("force item usage", item->GetProto()->ItemId))
            continue;

        // Only trade equipped items
        if (!item->IsEquipped())
            continue;

        TradeData* trade = bot->GetTradeData();

        if (trade)
        {

            if (trade->HasItem(item->GetObjectGuid())) //This specific item isn't being traded.
                continue;

            if (std::any_of(enchantItems.begin(), enchantItems.end(), [item](Item* i) {return i->GetEntry() == item->GetEntry(); })) //We didn't already add a simular item to this list.
                continue;
        }

        uint32 enchant_id = pSpellInfo->EffectMiscValue[0];

        uint32 currentEnchnatWeight = 0;
        if (item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT))
            currentEnchnatWeight = sRandomItemMgr.CalculateEnchantWeight(bot->getClass(), sRandomItemMgr.GetPlayerSpecId(bot), item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));

        uint32 newEnchantWeight = sRandomItemMgr.CalculateEnchantWeight(bot->getClass(), sRandomItemMgr.GetPlayerSpecId(bot), enchant_id);

        if (!item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT) || currentEnchnatWeight >= newEnchantWeight)
            continue;

        enchantItems.push_back(item);
    }

    return enchantItems;
}