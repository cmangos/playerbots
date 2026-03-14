
#include "playerbot/playerbot.h"
#include "EquipAction.h"

#include "playerbot/RandomItemMgr.h"
#include "playerbot/strategy/values/ItemCountValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

bool EquipAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    if (text == "?")
    {
        ListItems(requester);
        return true;
    }

    uint8 targetSlot = NULL_SLOT;
    if (text.find("mh ") == 0)
    {
        targetSlot = EQUIPMENT_SLOT_MAINHAND;
        text = text.substr(3);
    }
    else if (text.find("oh ") == 0)
    {
        targetSlot = EQUIPMENT_SLOT_OFFHAND;
        text = text.substr(3);
    }

    ItemIds ids = chat->parseItems(text);
    if (ids.empty())
    {
        //Get items based on text.
        std::list<Item*> found = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

        //Sort items on itemLevel descending.
        found.sort([](Item* i, Item* j) {return i->GetProto()->ItemLevel > j->GetProto()->ItemLevel; });

        std::vector< uint16> dests;
        for (auto& item : found)
        {
            uint32 itemId = item->GetProto()->ItemId;
            if (std::find(ids.begin(), ids.end(), itemId) != ids.end())
            {
                continue;
            }

            uint16 dest;
            InventoryResult msg = bot->CanEquipItem(targetSlot, dest, item, true);

            if (msg != EQUIP_ERR_OK)
            {
                continue;
            }

            if (std::find(dests.begin(), dests.end(), dest) != dests.end())
            {
                continue;
            }

            dests.push_back(dest);
            ids.insert(itemId);
        }
    }

    if (targetSlot != NULL_SLOT)
    {
        EquipItemsToSlot(requester, ids, targetSlot);
    }
    else
    {
        EquipItems(requester, ids);
    }
    return true;
}

void EquipAction::ListItems(Player* requester)
{
    ai->TellPlayer(requester, "=== Equip ===");

    std::map<uint32, int> items;
    std::map<uint32, bool> soulbound;
    for (int i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
    {
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
        {
            if (pItem)
            {
                items[pItem->GetProto()->ItemId] += pItem->GetCount();
            }
        }
    }

    ai->InventoryTellItems(requester, items, soulbound);
}

void EquipAction::EquipItems(Player* requester, ItemIds ids)
{
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        EquipItem(requester, &visitor);        
    }
}

void EquipAction::EquipItemsToSlot(Player* requester, ItemIds ids, uint8 targetSlot)
{
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
        std::list<Item*> items = visitor.GetResult();
        if (!items.empty())
        {
            EquipItemToSlot(requester, *items.begin(), targetSlot);
        }
    }
}

void EquipAction::EquipItem(Player* requester, FindItemVisitor* visitor)
{
    ai->InventoryIterateItems(visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    std::list<Item*> items = visitor->GetResult();
	if (!items.empty()) 
    {
        EquipItem(requester, *items.begin());
    }
}

//Return the bag slot with smallest bag
uint8 EquipAction::GetSmallestBagSlot()
{
    int8 curBag = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        const Bag* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
        {
            if (curBag > 0 && curSlots < pBag->GetBagSize())
            {
                continue;
            }
            
            curBag = bag;
            curSlots = pBag->GetBagSize();
        }
        else
        {
            return bag;
        }
    }

    return curBag;
}

void EquipAction::EquipItemToSlot(Player* requester, Item* item, uint8 targetSlot)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint32 itemId = item->GetProto()->ItemId;

    uint16 dest;
    InventoryResult msg = bot->CanEquipItem(targetSlot, dest, item, true);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, item, nullptr);
        return;
    }

    uint8 destSlot = dest & 0xFF;
    if (destSlot != targetSlot)
    {
        ai->TellPlayer(requester, "Cannot equip this item to the specified slot.");
        return;
    }

    uint16 src = ((bagIndex << 8) | slot);
    uint16 dstPos = ((INVENTORY_SLOT_BAG_0 << 8) | targetSlot);

    bot->SwapItem(src, dstPos);

    sPlayerbotAIConfig.logEvent(ai, "EquipAction", item->GetProto()->Name1, std::to_string(item->GetProto()->ItemId));

    std::map<std::string, std::string> args;
    args["%item"] = chat->formatItem(item);
    ai->TellPlayer(requester, BOT_TEXT2("equip_command", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
}

void EquipAction::EquipItem(Player* requester, Item* item)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint32 itemId = item->GetProto()->ItemId;

    if (item->GetProto()->InventoryType == INVTYPE_AMMO)
    {
        bot->SetAmmo(itemId);
    }
    else
    {
        bool equipedBag = false;
        if (item->GetProto()->Class == ITEM_CLASS_CONTAINER || item->GetProto()->Class == ITEM_CLASS_QUIVER)
        {
            Bag* pBag = (Bag*)&item;
            uint8 newBagSlot = GetSmallestBagSlot();
            if (newBagSlot > 0)
            {
                uint16 src = ((bagIndex << 8) | slot);

                if (newBagSlot == item->GetBagSlot()) //The new bag is in the slots of the old bag. Move it to the pack first.
                {
                    uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | INVENTORY_SLOT_ITEM_START);
                    bot->SwapItem(src, dst);
                    src = dst;
                }

                uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | newBagSlot);
                bot->SwapItem(src, dst);
                equipedBag = true;
            }
        }

        if (!equipedBag) 
        {
            WorldPacket packet(CMSG_AUTOEQUIP_ITEM, 2);
            packet << bagIndex << slot;
            bot->GetSession()->HandleAutoEquipItemOpcode(packet);
        }
    }

    sPlayerbotAIConfig.logEvent(ai, "EquipAction", item->GetProto()->Name1, std::to_string(item->GetProto()->ItemId));

    std::map<std::string, std::string> args;
    args["%item"] = chat->formatItem(item);
    ai->TellPlayer(requester, BOT_TEXT2("equip_command", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
}

bool EquipUpgradesAction::Execute(Event& event)
{
    if (!sPlayerbotAIConfig.autoEquipUpgradeLoot && !sRandomPlayerbotMgr.IsRandomBot(bot))
        return false;

    if (event.getSource() == "trade status")
    {
        WorldPacket p(event.getPacket());
        p.rpos(0);
        uint32 status;
        p >> status;

        if (status != TRADE_STATUS_TRADE_ACCEPT)
        {
            return false;
        }
    }
    else if (event.getSource() == "item push result")
    {
        bool valid = false;
        WorldPacket& data = event.getPacket();
        if (!data.empty())
        {
            data.rpos(0);

            ObjectGuid guid;
            data >> guid;
            if (guid != bot->GetObjectGuid())
            {
                return false;
            }

            uint32 received, created, isShowChatMessage, slotId, itemId, suffixFactor, count;
            uint32 itemRandomPropertyId;
            //uint32 invCount;
            uint8 bagSlot;

            data >> received;                               // 0=looted, 1=from npc
            data >> created;                                // 0=received, 1=created
            data >> isShowChatMessage;                                      // IsShowChatMessage
            data >> bagSlot;
            // item slot, but when added to stack: 0xFFFFFFFF
            data >> slotId;
            data >> itemId;
            data >> suffixFactor;
            data >> itemRandomPropertyId;
            data >> count;
            // data >> invCount; // [-ZERO] count of items in inventory

            ItemQualifier itemQualifier(itemId, (int32)itemRandomPropertyId);
            const ItemPrototype* itemProto = itemQualifier.GetProto();
            if (itemProto && (itemProto->Class == ItemClass::ITEM_CLASS_WEAPON || 
                              itemProto->Class == ItemClass::ITEM_CLASS_ARMOR ||
                              itemProto->Class == ItemClass::ITEM_CLASS_CONTAINER))
            {
                valid = true;
            }
        }

        if (!valid)
        {
            return false;
        }
    }

    context->ClearExpiredValues("item usage", 10); //Clear old item usage.

    std::list<Item*> items;

    FindItemUsageVisitor visitor(bot, ItemUsage::ITEM_USAGE_EQUIP);
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    visitor.SetUsage(ItemUsage::ITEM_USAGE_BAD_EQUIP);
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    items = visitor.GetResult();

    bool didEquip = false;

    items.sort([plr = bot](Item* i, Item* j) {return sRandomItemMgr.ItemStatWeight(plr, i) > sRandomItemMgr.ItemStatWeight(plr, j); });

    for (auto& item : items)
    {
#ifdef MANGOSBOT_TWO
        if (item->GetProto()->Class == ITEM_CLASS_GLYPH)
            continue;
#endif

        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());
        if (usage == ItemUsage::ITEM_USAGE_EQUIP || usage == ItemUsage::ITEM_USAGE_BAD_EQUIP)
        {
            sLog.outDetail("Bot #%d <%s> auto equips item %d (%s)", bot->GetGUIDLow(), bot->GetName(), item->GetProto()->ItemId, usage == ItemUsage::ITEM_USAGE_EQUIP ? "better than current" : usage == ItemUsage::ITEM_USAGE_BAD_EQUIP ? "wrong item but empty slot" : "");

            // Calculate which slot to equip main-hand and one-handed weapons.
            const ItemPrototype* proto = item->GetProto();
            bool isWeapon = (proto->Class == ItemClass::ITEM_CLASS_WEAPON);
            bool isMainHandOnly = item->IsMainHandOnlyEnchant(TEMP_ENCHANTMENT_SLOT) || (proto->InventoryType == INVTYPE_WEAPONMAINHAND);
            bool isOneHandWeapon = isWeapon &&
                (proto->InventoryType == INVTYPE_WEAPON || proto->InventoryType == INVTYPE_WEAPONMAINHAND || proto->InventoryType == INVTYPE_WEAPONOFFHAND);

            if (isMainHandOnly)
            {
                EquipItemToSlot(GetMaster(), item, EQUIPMENT_SLOT_MAINHAND);
                didEquip = true;
            }
            else if (isOneHandWeapon)
            {
                bool pendingMainHandOnly = false;

                for (auto& other : items)
                {
                    if (other == item)
                        continue;

                    const ItemPrototype* oproto = other->GetProto();
                    bool oIsWeapon = (oproto->Class == ItemClass::ITEM_CLASS_WEAPON);
                    bool oIsMainHandOnly = other->IsMainHandOnlyEnchant(TEMP_ENCHANTMENT_SLOT) || (oproto->InventoryType == INVTYPE_WEAPONMAINHAND);
                    if (oIsWeapon && oIsMainHandOnly)
                    {
                        ItemUsage otherUsage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(other).GetQualifier());
                        if (otherUsage == ItemUsage::ITEM_USAGE_EQUIP || otherUsage == ItemUsage::ITEM_USAGE_BAD_EQUIP)
                        {
                            pendingMainHandOnly = true;
                            break;
                        }
                    }
                }

                // Check main-hand before off-hand weapons
                if (!pendingMainHandOnly)
                {
                    Item* equippedMain = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                    if (equippedMain && equippedMain != item)
                    {
                        const ItemPrototype* eproto = equippedMain->GetProto();
                        bool eIsWeapon = (eproto->Class == ItemClass::ITEM_CLASS_WEAPON);
                        bool eIsMainHandOnly = equippedMain->IsMainHandOnlyEnchant(TEMP_ENCHANTMENT_SLOT) || (eproto->InventoryType == INVTYPE_WEAPONMAINHAND);
                        if (eIsWeapon && eIsMainHandOnly)
                        {
                            ItemUsage eUsage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(equippedMain).GetQualifier());
                            if (eUsage == ItemUsage::ITEM_USAGE_BAD_EQUIP || eUsage == ItemUsage::ITEM_USAGE_EQUIP)
                                pendingMainHandOnly = true;
                        }
                    }

                    Item* equippedOff = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                    if (!pendingMainHandOnly && equippedOff && equippedOff != item)
                    {
                        const ItemPrototype* eoproto = equippedOff->GetProto();
                        bool eoIsWeapon = (eoproto->Class == ItemClass::ITEM_CLASS_WEAPON);
                        bool eoIsMainHandOnly = equippedOff->IsMainHandOnlyEnchant(TEMP_ENCHANTMENT_SLOT) || (eoproto->InventoryType == INVTYPE_WEAPONMAINHAND);
                        if (eoIsWeapon && eoIsMainHandOnly)
                        {
                            ItemUsage eoUsage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(equippedOff).GetQualifier());
                            if (eoUsage == ItemUsage::ITEM_USAGE_BAD_EQUIP || eoUsage == ItemUsage::ITEM_USAGE_EQUIP)
                                pendingMainHandOnly = true;
                        }
                    }
                }

                if (pendingMainHandOnly)
                {
                    EquipItemToSlot(GetMaster(), item, EQUIPMENT_SLOT_OFFHAND);
                    didEquip = true;
                }
                else
                {
                    EquipItem(GetMaster(), item);
                    didEquip = true;
                }
            }
            else
            {
                EquipItem(GetMaster(), item);
                didEquip = true;
            }
        }
    }

    // If off-hand slot is empty, retry equipping once
    if (didEquip)
    {
        Item* mainAfter = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* offAfter = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        if (!mainAfter || !offAfter)
        {
            auto retryFillWeaponSlots = [&]()
                {
                    FindItemUsageVisitor retryVisitor(bot, ItemUsage::ITEM_USAGE_EQUIP);
                    ai->InventoryIterateItems(&retryVisitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
                    retryVisitor.SetUsage(ItemUsage::ITEM_USAGE_BAD_EQUIP);
                    ai->InventoryIterateItems(&retryVisitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
                    std::list<Item*> retryItems = retryVisitor.GetResult();

                    retryItems.sort([plr = bot](Item* i, Item* j) { return sRandomItemMgr.ItemStatWeight(plr, i) > sRandomItemMgr.ItemStatWeight(plr, j); });

                    for (auto& rItem : retryItems)
                    {
                        if (mainAfter && offAfter)
                            break;

#ifdef MANGOSBOT_TWO
                        if (rItem->GetProto()->Class == ITEM_CLASS_GLYPH)
                            continue;
#endif
                        ItemUsage rUsage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(rItem).GetQualifier());
                        if (!(rUsage == ItemUsage::ITEM_USAGE_EQUIP || rUsage == ItemUsage::ITEM_USAGE_BAD_EQUIP))
                            continue;

                        const ItemPrototype* rproto = rItem->GetProto();
                        bool rIsWeapon = (rproto->Class == ItemClass::ITEM_CLASS_WEAPON);
                        bool rIsOneHand = rIsWeapon &&
                            (rproto->InventoryType == INVTYPE_WEAPON || rproto->InventoryType == INVTYPE_WEAPONMAINHAND || rproto->InventoryType == INVTYPE_WEAPONOFFHAND);

                        if (!mainAfter)
                        {
                            uint16 dest;
                            InventoryResult msg = bot->CanEquipItem(EQUIPMENT_SLOT_MAINHAND, dest, rItem, true);
                            if (msg == EQUIP_ERR_OK)
                            {
                                EquipItemToSlot(GetMaster(), rItem, EQUIPMENT_SLOT_MAINHAND);
                                mainAfter = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
                                continue;
                            }
                        }

                        if (!offAfter)
                        {
                            uint16 dest;
                            InventoryResult msg = bot->CanEquipItem(EQUIPMENT_SLOT_OFFHAND, dest, rItem, true);
                            if (msg == EQUIP_ERR_OK)
                            {
                                EquipItemToSlot(GetMaster(), rItem, EQUIPMENT_SLOT_OFFHAND);
                                offAfter = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                                continue;
                            }
                        }

                        if (rIsOneHand && !offAfter)
                        {
                            bool mainReserved = false;
                            for (auto& other : retryItems)
                            {
                                if (other == rItem) continue;
                                const ItemPrototype* oproto = other->GetProto();
                                if (oproto->Class != ItemClass::ITEM_CLASS_WEAPON) continue;
                                bool oIsMainOnly = other->IsMainHandOnlyEnchant(TEMP_ENCHANTMENT_SLOT) || (oproto->InventoryType == INVTYPE_WEAPONMAINHAND);
                                if (!oIsMainOnly) continue;
                                ItemUsage oUsage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(other).GetQualifier());
                                if (oUsage == ItemUsage::ITEM_USAGE_EQUIP || oUsage == ItemUsage::ITEM_USAGE_BAD_EQUIP)
                                {
                                    mainReserved = true;
                                    break;
                                }
                            }

                            if (mainReserved)
                            {
                                uint16 dest;
                                InventoryResult msg = bot->CanEquipItem(EQUIPMENT_SLOT_OFFHAND, dest, rItem, true);
                                if (msg == EQUIP_ERR_OK)
                                {
                                    EquipItemToSlot(GetMaster(), rItem, EQUIPMENT_SLOT_OFFHAND);
                                    offAfter = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
                                    continue;
                                }
                            }
                        }
                    }
                };

            retryFillWeaponSlots();
        }
    }

    return didEquip;
}