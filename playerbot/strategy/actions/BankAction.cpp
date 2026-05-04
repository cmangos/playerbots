
#include "playerbot/playerbot.h"
#include "BankAction.h"
#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

namespace
{
    void ResetBankActionItemCaches(PlayerbotAI* ai, const std::string& itemId, const std::string& itemQualifier,
        const std::string& usageQualifier = "")
    {
        if (!ai || itemId.empty() || itemQualifier.empty())
            return;

        AiObjectContext* context = ai->GetAiObjectContext();
        if (!context)
            return;

        RESET_AI_VALUE(uint8,"bag space");
        RESET_AI_VALUE2(uint32,"item count", itemId);
        RESET_AI_VALUE2(uint32,"bank item count", itemId);
        RESET_AI_VALUE2(ItemUsage,"item usage", itemQualifier);
        RESET_AI_VALUE2(std::list<Item*>, "inventory items", itemQualifier);

        if (!usageQualifier.empty())
        {
            RESET_AI_VALUE2(uint32,"item count", usageQualifier);
            RESET_AI_VALUE2(std::list<Item*>, "inventory items", usageQualifier);
        }
    }
}

bool BankAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();

    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs no los");
    for (std::list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* npc = ai->GetUnit(*i);
        if (!npc || !npc->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_BANKER))
            continue;

        return ExecuteCommand(requester, text, npc);
    }

    ai->TellError(requester, "Cannot find banker nearby");
    return false;
}

bool BankAction::ExecuteCommand(Player* requester, const std::string& text, Unit* bank)
{
    if (text.empty() || text == "?")
    {
        ListItems(requester);
        return true;
    }

    bool result = false;
    if (text[0] == '-')
    {
        std::list<Item*> found = ai->InventoryParseItems(text.substr(1), IterateItemsMask::ITERATE_ITEMS_IN_BANK);
        for (std::list<Item*>::iterator i = found.begin(); i != found.end(); i++)
        {
            Item* item = *i;
            result &= Withdraw(requester, item->GetProto()->ItemId);
        }
    }
    else
    {
        std::list<Item*> found = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        if (found.empty())
            return false;

        for (std::list<Item*>::iterator i = found.begin(); i != found.end(); i++)
        {
            Item* item = *i;
            if (!item)
                continue;

            result &= Deposit(requester, item);
        }
    }

    return result;
}

bool BankAction::Withdraw(Player* requester, const uint32 itemid)
{
    Item* pItem = FindItemInBank(itemid);
    if (!pItem)
        return false;

    const ItemPrototype* proto = pItem->GetProto();
    if (!proto)
        return false;

    const std::string itemId = std::to_string(proto->ItemId);
    const std::string itemQualifier = ItemQualifier(pItem).GetQualifier();
    const std::string itemText = chat->formatItem(pItem, pItem->GetCount());

    ResetBankActionItemCaches(ai, itemId, itemQualifier);

    ItemPosCountVec dest;
#ifdef MANGOSBOT_TWO
    uint8 bagSlot;
    InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, bagSlot, false);
#else
    InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
#endif

    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, NULL);
        return false;
    }

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->StoreItem(dest, pItem, true);
    ResetBankActionItemCaches(ai, itemId, itemQualifier);

    std::ostringstream out;
    out << "got " << itemText << " from bank";
    ai->TellPlayer(requester, out.str());
    return true;
}

bool BankAction::Deposit(Player* requester, Item* pItem)
{
    std::ostringstream out;

    const ItemPrototype* proto = pItem ? pItem->GetProto() : nullptr;
    if (!proto)
        return false;

    const std::string itemId = std::to_string(proto->ItemId);
    const std::string itemQualifier = ItemQualifier(pItem).GetQualifier();
    const std::string itemText = chat->formatItem(pItem, pItem->GetCount());

    ResetBankActionItemCaches(ai, itemId, itemQualifier);

    ItemPosCountVec dest;
#ifdef MANGOSBOT_TWO
    uint8 bagSlot;
    InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, pItem, false, bagSlot);
#else
    InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
#endif

    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, NULL);
        return false;
    }

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->BankItem(dest, pItem, true);
    ResetBankActionItemCaches(ai, itemId, itemQualifier);

    out << "put " << itemText << " to bank";
    ai->TellPlayer(requester, out.str());
	return true;
}

void BankAction::ListItems(Player* requester)
{
    ai->TellPlayer(requester, "=== Bank ===");

    std::map<uint32, int> items;
    std::map<uint32, bool> soulbound;
    for (int i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem)
            {
                items[pItem->GetProto()->ItemId] += pItem->GetCount();
                soulbound[pItem->GetProto()->ItemId] = pItem->IsSoulBound();
            }
        }
    }

    for (int i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
    {
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pBag)
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* pItem = pBag->GetItemByPos(j))
                    {
                        if (pItem)
                        {
                            items[pItem->GetProto()->ItemId] += pItem->GetCount();
                            soulbound[pItem->GetProto()->ItemId] = pItem->IsSoulBound();
                        }
                    }
                }
            }
        }
    }

    ai->InventoryTellItems(requester, items, soulbound);
}

Item* BankAction::FindItemInBank(uint32 ItemId)
{
    for (uint8 slot = BANK_SLOT_ITEM_START; slot < BANK_SLOT_ITEM_END; slot++)
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (pItem)
        {
            const ItemPrototype* const pItemProto = pItem->GetProto();
            if (!pItemProto)
                continue;

            if (pItemProto->ItemId == ItemId)   // have required item
                return pItem;
        }
    }

    for (uint8 bag = BANK_SLOT_BAG_START; bag < BANK_SLOT_BAG_END; ++bag)
    {
        const Bag* const pBag = (Bag *) bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                Item* const pItem = bot->GetItemByPos(bag, slot);
                if (pItem)
                {
                    const ItemPrototype* const pItemProto = pItem->GetProto();
                    if (!pItemProto)
                        continue;

                    if (pItemProto->ItemId == ItemId)
                        return pItem;
                }
            }
        }
    }

    return NULL;
}

bool BankAction::AutoDeposit()
{
    bool deposited = false;

    std::string itemusageQualifier = "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_BANK);

    std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", itemusageQualifier);
    for (auto item : items)
    {
        if (!item)
            continue;

        const ItemPrototype* proto = item->GetProto();
        if (!proto)
            continue;

        const std::string itemId = std::to_string(proto->ItemId);
        const std::string itemQualifier = ItemQualifier(item).GetQualifier();

        // Don't bank items needed for guild orders
        ItemQualifier qualifier(item);
        std::string qualStr = qualifier.GetQualifier();
        ItemUsage currentUsage = AI_VALUE2(ItemUsage, "item usage", qualStr);
        if (currentUsage == ItemUsage::ITEM_USAGE_GUILD_TASK)
            continue;

        ItemPosCountVec dest;
#ifdef MANGOSBOT_TWO
        uint8 bagSlot;
        InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, item, false, bagSlot);
#else
        InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, item, false);
#endif
        if (msg != EQUIP_ERR_OK)
            continue;

        bot->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
        bot->BankItem(dest, item, true);
        ResetBankActionItemCaches(ai, itemId, itemQualifier, itemusageQualifier);
        deposited = true;
    }

    return deposited;
}

bool BankAction::AutoWithdraw()
{
    bool withdrew = false;

    if (AI_VALUE(uint8, "bag space") > 80)
        return false;

    auto checkAndWithdraw = [&](Item* pItem) -> bool
    {
        if (!pItem)
            return false;

        ItemPrototype const* proto = pItem->GetProto();
        if (!proto)
            return false;

        const std::string itemId = std::to_string(proto->ItemId);
        const std::string itemQualifier = ItemQualifier(pItem).GetQualifier();

        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemId);
        if (usage == ItemUsage::ITEM_USAGE_BANK)
            return false;        

        ItemPosCountVec dest;
#ifdef MANGOSBOT_TWO
        uint8 bagSlot;
        InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, bagSlot, false);
#else
        InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
#endif;
        if (msg != EQUIP_ERR_OK)
            return false;

        bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
        bot->StoreItem(dest, pItem, true);
        ResetBankActionItemCaches(ai, itemId, itemQualifier);
        return true;
    };

    for (auto& item : AI_VALUE2(std::list<Item*>, "bank items", "all"))
    {
        if (checkAndWithdraw(item))
            withdrew = true;
    }    

    return withdrew;
}
