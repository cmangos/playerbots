
#include "playerbot/playerbot.h"
#include "TradeValues.h"
#include "ItemUsageValue.h"
#include "playerbot/TravelMgr.h"

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

    std::list<ItemUsage> myUsages = { ItemUsage::ITEM_USAGE_NONE , ItemUsage::ITEM_USAGE_VENDOR, ItemUsage::ITEM_USAGE_AH, ItemUsage::ITEM_USAGE_DISENCHANT };

    for (auto& myUsage : myUsages)
    {
        std::list<Item*> myItems = AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)myUsage));
        myItems.reverse();

        for (auto& item : myItems)
        {
            if (!item->CanBeTraded())
                continue;

            // do not trade equipped items
            if (item->IsEquipped())
                continue;

            TradeData* trade = bot->GetTradeData();

            if (trade)
            {

                if (trade->HasItem(item->GetObjectGuid())) //This specific item isn't being traded.
                    continue;

                if (IsTradingItem(item->GetEntry())) //A simular item isn't being traded.
                    continue;

                if (std::any_of(giveItems.begin(), giveItems.end(), [item](Item* i) {return i->GetEntry() == item->GetEntry(); })) //We didn't already add a simular item to this list.
                    continue;
            }

            ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());

            if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
                giveItems.push_back(item);
        }
    }

    return giveItems;
}
