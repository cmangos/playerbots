
#include "playerbot/playerbot.h"
#include "ItemUsageValue.h"
#include "CraftValues.h"
#include "MountValues.h"

#include "playerbot/RandomItemMgr.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

std::unordered_map<uint32, std::unordered_set<uint32>> ItemUsageValue::m_reagentItemIdsForCraftingSkills;
std::unordered_set<uint32> ItemUsageValue::m_allReagentItemIdsForCraftingSkills;
std::vector<uint32> ItemUsageValue::m_allReagentItemIdsForCraftingSkillsVector;

std::unordered_map<uint32, std::vector<std::pair<uint32, uint32>>> ItemUsageValue::m_craftingReagentItemIdsForCraftableItem;

std::unordered_set<uint32> ItemUsageValue::m_allItemIdsSoldByAnyVendors;
std::unordered_set<uint32> ItemUsageValue::m_itemIdsSoldByAnyVendorsWithLimitedMaxCount;

ItemQualifier::ItemQualifier(std::string qualifier, bool linkQualifier)
{
    itemId = 0;
    enchantId = 0;
    randomPropertyId = 0;
    gem1 = 0;
    gem2 = 0;
    gem3 = 0;
    gem4 = 0;
    proto = nullptr;

    std::vector<std::string> numbers = Qualified::getMultiQualifiers(qualifier, ":");

    if (numbers.empty())
        return;

    for (char& d : numbers[0]) //Check if itemId contains only numbers
        if (!isdigit(d))
            return;

    itemId = stoi(numbers[0]);

#ifdef MANGOSBOT_ZERO
    uint32 propertyPosition = linkQualifier ? 2 : 6;
#else
    uint32 propertyPosition = linkQualifier ? 6 : 6;
#endif

    if (numbers.size() > 1 && !numbers[1].empty())
        enchantId = stoi(numbers[1]);

    if (numbers.size() > propertyPosition && !numbers[propertyPosition].empty())
        randomPropertyId = stoi(numbers[propertyPosition]);

#ifndef MANGOSBOT_ZERO
    uint8 gemPosition = linkQualifier ? 2 : 2;

    if (numbers.size() > gemPosition + 3)
    {
        if (!numbers[gemPosition].empty())
            gem1 = stoi(numbers[gemPosition]);
        if (!numbers[gemPosition + 1].empty())
            gem2 = stoi(numbers[gemPosition + 1]);
        if (!numbers[gemPosition + 2].empty())
            gem3 = stoi(numbers[gemPosition + 2]);
        if (!numbers[gemPosition + 3].empty())
            gem4 = stoi(numbers[gemPosition + 3]);
    }
#endif
}

uint32 ItemQualifier::GemId(Item* item, uint8 gemSlot)
{
#ifdef MANGOSBOT_ZERO
    return 0;
#else
    uint32 enchantId = item->GetEnchantmentId(EnchantmentSlot(SOCK_ENCHANTMENT_SLOT + gemSlot));

    if (!enchantId)
        return 0;

    return enchantId;
#endif
}

ItemUsage ItemUsageValue::Calculate()
{
    ItemQualifier itemQualifier(qualifier, false);
    uint32 itemId = itemQualifier.GetId();
    if (!itemId)
        return ItemUsage::ITEM_USAGE_NONE;

    const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
    if (!proto)
        return ItemUsage::ITEM_USAGE_NONE;

    //FORCE
    ForceItemUsage forceUsage = AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", proto->ItemId, ForceItemUsage::FORCE_USAGE_NONE);

    if (forceUsage == ForceItemUsage::FORCE_USAGE_GREED)
        return ItemUsage::ITEM_USAGE_FORCE_GREED;

    if (forceUsage == ForceItemUsage::FORCE_USAGE_NEED)
        return ItemUsage::ITEM_USAGE_FORCE_NEED;

    if (forceUsage == ForceItemUsage::FORCE_USAGE_KEEP)
        return ItemUsage::ITEM_USAGE_KEEP;

    //KEEP HEARTHSTONE
    if (proto->ItemId == 6948)
        return ItemUsage::ITEM_USAGE_KEEP;

    //SKILL
    if (ai->HasActivePlayerMaster())
    {
        if (IsItemUsefulForSkill(proto))
            return ItemUsage::ITEM_USAGE_SKILL;

        if (IsItemNeededForSkill(proto))
        {
            float stacks = CurrentStacks(ai, proto);
            if (stacks < 1)
                return ItemUsage::ITEM_USAGE_SKILL; //Buy more.
            else if (stacks == 1)
                return ItemUsage::ITEM_USAGE_KEEP; //Keep in inventory.
        }
    }
    else
    {
        bool needItem = false;

        if (IsItemNeededForSkill(proto))
        {
            float stacks = CurrentStacks(ai, proto);
            if (stacks < 1)
                return ItemUsage::ITEM_USAGE_SKILL; //Buy more.
            else if (stacks == 1)
                return ItemUsage::ITEM_USAGE_KEEP; //Keep in inventory.
        }
        else
        {
            bool lowBagSpace = AI_VALUE(uint8, "bag space") > 50;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS || proto->Class == ITEM_CLASS_MISC || proto->Class == ITEM_CLASS_REAGENT)
                needItem = IsItemNeededForUsefullCraft(proto, lowBagSpace);
            else if (proto->Class == ITEM_CLASS_RECIPE)
            {
                if (bot->HasSpell(GetRecipeSpell(proto)))
                    needItem = false;
                else
                    needItem = bot->CanUseItem(proto) == EQUIP_ERR_OK;
            }
        }

        if (needItem)
        {
            float stacks = CurrentStacks(ai, proto);

            if (proto->Class == ITEM_CLASS_RECIPE && stacks > 0) //Only buy one recipe.
                return ItemUsage::ITEM_USAGE_KEEP;

            if (stacks < 2)
                return ItemUsage::ITEM_USAGE_SKILL; //Buy more.
            else if (stacks == 2)
                return ItemUsage::ITEM_USAGE_KEEP; //Do not buy more.
        }
    }

    //USE
    if (proto->Class == ITEM_CLASS_KEY)
        return ItemUsage::ITEM_USAGE_USE;

    if (proto->Class == ITEM_CLASS_CONSUMABLE && !ai->HasCheat(BotCheatMask::item))
    {
        std::string foodType = "";

        if (IsHpFoodOrDrink(proto))
        {
            foodType = "food";
        }
        else if (IsManaFoodOrDrink(proto))
        {
            foodType = "drink";
        }
        else if (IsHealingPotion(proto))
        {
            foodType = "healing potion";
        }
        else if (IsManaPotion(proto))
        {
            foodType = "mana potion";
        }
        else if (IsBandage(proto))
        {
            foodType = "bandage";
        }

        bool botHasHealingSpells = bot->getClass() == CLASS_PALADIN || bot->getClass() == CLASS_PRIEST || bot->getClass() == CLASS_DRUID || bot->getClass() == CLASS_SHAMAN;

        //itemlevel 1 consumables are mostly level-independent
        bool isAppropriateConsumableLevel = proto->RequiredLevel <= bot->GetLevel()
            && (proto->ItemLevel == 1 || proto->ItemLevel >= bot->GetLevel());

        bool isAppropriateConsumable = isAppropriateConsumableLevel
            && (IsHpFoodOrDrink(proto) || IsHealingPotion(proto) || (IsBandage(proto) && !botHasHealingSpells) || (bot->HasMana() && (IsManaFoodOrDrink(proto) || IsManaPotion(proto))));

        if (isAppropriateConsumable && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        {
            float stacks = BetterStacks(proto, foodType);

            if (stacks < 1)
            {
                stacks += CurrentStacks(ai, proto);
                return ItemUsage::ITEM_USAGE_USE; //Buy some to get to 1 stack
            }
        }
    }

    if (proto->Class == ITEM_CLASS_REAGENT && SpellsUsingItem(proto->ItemId, bot).size())
    {
        float stacks = CurrentStacks(ai, proto);

        if (stacks < 1)
            return ItemUsage::ITEM_USAGE_USE;
        else if (stacks < 2)
            return ItemUsage::ITEM_USAGE_KEEP;
    }

    //EQUIP
    if (MountValue::GetMountSpell(itemId) && bot->CanUseItem(proto) == EQUIP_ERR_OK && MountValue::GetSpeed(MountValue::GetMountSpell(itemId)))
    {
        std::vector<MountValue> mounts = AI_VALUE(std::vector<MountValue>, "mount list");

        if (mounts.empty())
            return ItemUsage::ITEM_USAGE_EQUIP;

        uint32 newSpeed[2] = { MountValue::GetSpeed(MountValue::GetMountSpell(itemId), false), MountValue::GetSpeed(MountValue::GetMountSpell(itemId), true) };

        bool hasBetterMount = false, hasSameMount = false;

        for (auto& mount : mounts)
        {
            for (bool canFly : {true, false})
            {
                if (!newSpeed[canFly])
                    continue;

                uint32 currentSpeed = mount.GetSpeed(canFly);

                if (currentSpeed > newSpeed[canFly])
                    hasBetterMount = true;
                else if (currentSpeed == newSpeed[canFly])
                    hasSameMount = true;
            }

            if (hasBetterMount)
                break;
        }

        if (!hasBetterMount)
            return hasSameMount ? ItemUsage::ITEM_USAGE_KEEP : ItemUsage::ITEM_USAGE_EQUIP;
    }

    ItemUsage equip = QueryItemUsageForEquip(itemQualifier);
    if (equip != ItemUsage::ITEM_USAGE_NONE)
        return equip;

    //DISENCHANT
    if ((proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON) && proto->Bonding != BIND_WHEN_PICKED_UP &&
        ai->HasSkill(SKILL_ENCHANTING) && proto->Quality >= ITEM_QUALITY_UNCOMMON)
        return ItemUsage::ITEM_USAGE_DISENCHANT;

    //QUEST
    if (!ai->GetMaster() || !sPlayerbotAIConfig.syncQuestWithPlayer || !IsItemUsefulForQuest(ai->GetMaster(), proto))
    {
        if (IsItemUsefulForQuest(bot, proto))
            return ItemUsage::ITEM_USAGE_QUEST;
        else if (IsItemUsefulForQuest(bot, proto, true) && CurrentStacks(ai, proto) < 2) //Do not sell quest items unless selling a full stack will stil keep enough in inventory.
            return ItemUsage::ITEM_USAGE_KEEP;
    }

    //AMMO
    if ((proto->Class == ITEM_CLASS_PROJECTILE || (proto->Class == ITEM_CLASS_WEAPON && proto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN)) && bot->CanUseItem(proto) == EQUIP_ERR_OK)
        if ((bot->getClass() == CLASS_HUNTER && proto->Class != ITEM_CLASS_WEAPON) || bot->getClass() == CLASS_ROGUE || bot->getClass() == CLASS_WARRIOR)
        {
            Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
            if (pItem)
            {
                uint32 ammoClass = ITEM_CLASS_PROJECTILE;
                uint32 subClass = 0;
                switch (pItem->GetProto()->SubClass)
                {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    subClass = ITEM_SUBCLASS_BULLET;
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    subClass = ITEM_SUBCLASS_ARROW;
                    break;
                case ITEM_SUBCLASS_WEAPON_THROWN:
                    ammoClass = ITEM_CLASS_WEAPON;
                    subClass = ITEM_SUBCLASS_WEAPON_THROWN;
                    break;
                }

                if (proto->Class == ammoClass && proto->SubClass == subClass)
                {
                    uint32 currentAmmoId = bot->GetUInt32Value(PLAYER_AMMO_ID);
                    const ItemPrototype* currentAmmoproto = nullptr;
                    if (currentAmmoId)
                        currentAmmoproto = sObjectMgr.GetItemPrototype(itemId);

                    float ammo = BetterStacks(proto, "ammo");
                    float needAmmo = (bot->getClass() == CLASS_HUNTER) ? 8 : 2;

                    if (ai->HasCheat(BotCheatMask::item))
                        needAmmo = 1;

                    if (ammo < 0) //No current better ammo.
                    {
                        if (!currentAmmoId)
                            return ItemUsage::ITEM_USAGE_EQUIP;

                        if (currentAmmoproto->ItemLevel > proto->ItemLevel)
                            return ItemUsage::ITEM_USAGE_EQUIP;
                    }

                    if (ammo < needAmmo) //We already have enough of the current ammo.
                    {
                        ammo += CurrentStacks(ai, proto);

                        if (ammo < needAmmo)         //Buy ammo to get to the proper supply
                            return ItemUsage::ITEM_USAGE_AMMO;
                        else if (ammo < needAmmo + 1)
                            return ItemUsage::ITEM_USAGE_KEEP;  //Keep the ammo until we have too much.
                    }
                }
            }
        }

    //KEEP
    if (proto->Quality >= ITEM_QUALITY_EPIC && !sRandomPlayerbotMgr.IsRandomBot(bot))
        return ItemUsage::ITEM_USAGE_KEEP;

    if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE)
    {
        uint8 maxExpansionCharacterLevel = 60;
#ifdef MANGOSBOT_ONE
        maxExpansionCharacterLevel = 70;
#endif
#ifdef MANGOSBOT_TWO
        maxExpansionCharacterLevel = 80;
#endif

        //keep relevant class consumables (e.g. rogue poisons)
        if (proto->AllowableClass == bot->getClass() && proto->RequiredLevel + 6 >= bot->GetLevel())
        {
            if (bot->GetLevel() == maxExpansionCharacterLevel && proto->RequiredLevel + 6 >= bot->GetLevel())
            {
                return ItemUsage::ITEM_USAGE_USE;
            }

            if (bot->GetLevel() == maxExpansionCharacterLevel && proto->RequiredLevel + 10 >= bot->GetLevel())
            {
                return ItemUsage::ITEM_USAGE_USE;
            }
        }
    }

    //VENDOR/AH
    if (proto->SellPrice > 0)
    {
        //if item value is significantly higher than its vendor sell price
        if (IsMoreProfitableToSellToAHThanToVendor(proto, bot))
        {
            if (proto->Bonding == NO_BIND)
                return ItemUsage::ITEM_USAGE_AH;

            if (proto->Bonding == BIND_WHEN_EQUIPPED)
            {
                Item* item = CurrentItem(proto);
                if (!item || !item->IsSoulBound())
                    return ItemUsage::ITEM_USAGE_AH;
            }
        }

        return ItemUsage::ITEM_USAGE_VENDOR;
    }

    //NONE
    return ItemUsage::ITEM_USAGE_NONE;
}

ItemUsage ItemUsageValue::QueryItemUsageForEquip(ItemQualifier& itemQualifier)
{
    ItemPrototype const* itemProto = itemQualifier.GetProto();

    if (bot->CanUseItem(itemProto) != EQUIP_ERR_OK)
        return ItemUsage::ITEM_USAGE_NONE;

    if (itemProto->InventoryType == INVTYPE_NON_EQUIP)
        return ItemUsage::ITEM_USAGE_NONE;

    uint16 dest;

    std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", chat->formatItem(itemQualifier));
    InventoryResult result;
    if (!items.empty())
    {
        result = bot->CanEquipItem(NULL_SLOT, dest, items.front(), true, false);
    }
    else
    {
        Item* pItem = RandomPlayerbotMgr::CreateTempItem(itemProto->ItemId, 1, bot);
        if (!pItem)
            return ItemUsage::ITEM_USAGE_NONE;

        result = bot->CanEquipItem(NULL_SLOT, dest, pItem, true, false);
        pItem->RemoveFromUpdateQueueOf(bot);
        delete pItem;
    }

    if (result != EQUIP_ERR_OK)
        return ItemUsage::ITEM_USAGE_NONE;

    if (itemProto->Class == ITEM_CLASS_QUIVER)
    {
        Item* equippedRangedWeapon = bot->GetWeaponForAttack(WeaponAttackType::RANGED_ATTACK, false, false);

        if (bot->getClass() == CLASS_HUNTER && equippedRangedWeapon)
        {
            ItemPrototype const* rangedWeaponItemProto = equippedRangedWeapon->GetProto();
            if (!rangedWeaponItemProto)
                return ItemUsage::ITEM_USAGE_NONE;

            bool isCorrectQuiverTypeForCurrentWeapon = false;

            if (itemProto->SubClass == ITEM_SUBCLASS_AMMO_POUCH)
            {
                if (rangedWeaponItemProto->SubClass == ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_GUN)
                {
                    isCorrectQuiverTypeForCurrentWeapon = true;
                }
            }
            else {
                if (rangedWeaponItemProto->SubClass == ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_BOW
                    || rangedWeaponItemProto->SubClass == ItemSubclassWeapon::ITEM_SUBCLASS_WEAPON_CROSSBOW)
                {
                    isCorrectQuiverTypeForCurrentWeapon = true;
                }
            }

            if (!isCorrectQuiverTypeForCurrentWeapon)
                return ItemUsage::ITEM_USAGE_NONE;

            std::vector<Bag*> equippedQuivers = bot->GetPlayerbotAI()->GetEquippedQuivers();

            for (auto quiver : equippedQuivers)
            {
                if (quiver->GetProto()->ItemLevel < itemProto->ItemLevel)
                {
                    return ItemUsage::ITEM_USAGE_EQUIP;
                }

                if (quiver->GetProto()->ItemLevel == itemProto->ItemLevel && quiver->GetProto()->Quality < itemProto->Quality)
                {
                    return ItemUsage::ITEM_USAGE_EQUIP;
                }

                //no need to check for quiver container slots size, higher ilvl/quality checks is enough
            }
        }

        return ItemUsage::ITEM_USAGE_NONE;
    }

    if (itemProto->Class == ITEM_CLASS_CONTAINER)
    {
        if (itemProto->SubClass != ITEM_SUBCLASS_CONTAINER)
            return ItemUsage::ITEM_USAGE_NONE; //Todo add logic for non-bag containers. We want to look at professions/class and only replace if non-bag is larger than bag.

        if (GetSmallestBagSize() >= itemProto->ContainerSlots)
            return ItemUsage::ITEM_USAGE_NONE;

        return ItemUsage::ITEM_USAGE_EQUIP;
    }

    bool shouldEquip = false;

    uint32 specId = sRandomItemMgr.GetPlayerSpecId(bot);

    uint32 statWeight = sRandomItemMgr.ItemStatWeight(bot, itemQualifier);
    if (statWeight)
        shouldEquip = true;

    if (itemProto->Class == ITEM_CLASS_WEAPON && !sRandomItemMgr.ShouldEquipWeaponForSpec(bot->getClass(), specId, itemProto))
        shouldEquip = false;
    if (itemProto->Class == ITEM_CLASS_ARMOR && !sRandomItemMgr.ShouldEquipArmorForSpec(bot->getClass(), specId, itemProto))
        shouldEquip = false;

    Item* oldItem = bot->GetItemByPos(dest);

    //No item equiped
    if (!oldItem)
    {
        if (shouldEquip)
            return ItemUsage::ITEM_USAGE_EQUIP;
        else
            return ItemUsage::ITEM_USAGE_BAD_EQUIP;
    }

    const ItemPrototype* oldItemProto = oldItem->GetProto();

    if (AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", oldItemProto->ItemId, ForceItemUsage::FORCE_USAGE_NONE) == ForceItemUsage::FORCE_USAGE_EQUIP) //Current equip is forced. Do not unequip.
    {
        if (AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", itemProto->ItemId, ForceItemUsage::FORCE_USAGE_NONE) == ForceItemUsage::FORCE_USAGE_EQUIP)
            return ItemUsage::ITEM_USAGE_KEEP;
        else
            return ItemUsage::ITEM_USAGE_NONE;
    }

    uint32 oldStatWeight = sRandomItemMgr.ItemStatWeight(bot, oldItem);
    if (statWeight || oldStatWeight)
    {
        shouldEquip = statWeight >= oldStatWeight;
    }

    if (AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", itemProto->ItemId, ForceItemUsage::FORCE_USAGE_NONE) == ForceItemUsage::FORCE_USAGE_EQUIP) //New item is forced. Always equip it.
        return ItemUsage::ITEM_USAGE_EQUIP;

    bool existingShouldEquip = true;
    if (oldItemProto->Class == ITEM_CLASS_WEAPON && !oldStatWeight)
        existingShouldEquip = false;
    if (oldItemProto->Class == ITEM_CLASS_ARMOR && !statWeight)
        existingShouldEquip = false;

    //Compare items based on item level, quality or itemId.
    bool isBetter = false;
    if (statWeight > oldStatWeight)
        isBetter = true;
    else if (statWeight == oldStatWeight && itemProto->Quality > oldItemProto->Quality)
        isBetter = true;
    else if (statWeight == oldStatWeight && itemProto->Quality == oldItemProto->Quality && itemProto->ItemId > oldItemProto->ItemId)
        isBetter = true;

    Item* item = CurrentItem(itemProto);
    bool itemIsBroken = item && item->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;
    bool oldItemIsBroken = oldItem->GetUInt32Value(ITEM_FIELD_DURABILITY) == 0 && oldItem->GetUInt32Value(ITEM_FIELD_MAXDURABILITY) > 0;

    if (itemProto->ItemId != oldItemProto->ItemId && (shouldEquip || !existingShouldEquip) && isBetter)
    {
        switch (itemProto->Class)
        {
        case ITEM_CLASS_ARMOR:
            if (oldItemProto->SubClass <= itemProto->SubClass) {
                if (itemIsBroken && !oldItemIsBroken)
                    return ItemUsage::ITEM_USAGE_BROKEN_EQUIP;
                else
                    if (shouldEquip)
                        return ItemUsage::ITEM_USAGE_EQUIP;
                    else
                        return ItemUsage::ITEM_USAGE_BAD_EQUIP;
            }
            break;
        default:
            if (itemIsBroken && !oldItemIsBroken)
                return ItemUsage::ITEM_USAGE_BROKEN_EQUIP;
            else
                if (shouldEquip)
                    return ItemUsage::ITEM_USAGE_EQUIP;
                else
                    return ItemUsage::ITEM_USAGE_BAD_EQUIP;
        }
    }

    //Item is not better but current item is broken and new one is not.
    if (oldItemIsBroken && !itemIsBroken)
        return ItemUsage::ITEM_USAGE_EQUIP;

    return ItemUsage::ITEM_USAGE_NONE;
}

//Return smaltest bag size equipped
uint32 ItemUsageValue::GetSmallestBagSize()
{
    int8 curSlot = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        const Bag* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
        {
            if (curSlot > 0 && curSlots < pBag->GetBagSize())
                continue;

            curSlot = pBag->GetSlot();
            curSlots = pBag->GetBagSize();
        }
        else
            return 0;
    }

    return curSlots;
}

bool ItemUsageValue::IsItemUsefulForQuest(Player* player, ItemPrototype const* proto, bool ignoreInventory)
{
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 entry = player->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr.GetQuestTemplate(entry);
        if (!quest)
            continue;

        for (int i = 0; i < 4; i++)
        {
            if (quest->ReqItemId[i] != proto->ItemId)
                continue;

            if (player->GetPlayerbotAI() && AI_VALUE2(uint32, "item count", proto->Name1) >= quest->ReqItemCount[i] && !ignoreInventory)
                continue;

            return true;
        }
    }

    return false;
}

bool ItemUsageValue::IsItemNeededForSkill(ItemPrototype const* proto)
{
    switch (proto->ItemId)
    {
    case 2901: //Mining pick
        return ai->HasSkill(SKILL_MINING);
    case 5956: //Blacksmith Hammer
        return ai->HasSkill(SKILL_BLACKSMITHING) || ai->HasSkill(SKILL_ENGINEERING);
    case 6219: //Arclight Spanner
        return ai->HasSkill(SKILL_ENGINEERING);
    case 6218: //Runed copper rod
        return ai->HasSkill(SKILL_ENCHANTING);
    case 6339: //Runed silver rod
        return ai->HasSkill(SKILL_ENCHANTING);
    case 11130: //Runed golden rod
        return ai->HasSkill(SKILL_ENCHANTING);
    case 11145: //Runed truesilver rod
        return ai->HasSkill(SKILL_ENCHANTING);
    case 16207: //Runed Arcanite Rod
        return ai->HasSkill(SKILL_ENCHANTING);
    case 7005: //Skinning Knife
        return ai->HasSkill(SKILL_SKINNING);
    case 4471: //Flint and Tinder
        return ai->HasSkill(SKILL_COOKING);
    case 4470: //Simple Wood
        return ai->HasSkill(SKILL_COOKING);
    case 6256: //Fishing Rod
        return ai->HasSkill(SKILL_FISHING);
    }

    return false;
}


bool ItemUsageValue::IsItemUsefulForSkill(ItemPrototype const* proto)
{
    switch (proto->Class)
    {
    case ITEM_CLASS_TRADE_GOODS:
    case ITEM_CLASS_MISC:
    case ITEM_CLASS_REAGENT:
    {
        if (ai->HasSkill(SKILL_TAILORING) && IsItemUsedBySkill(proto, SKILL_TAILORING))
            return true;
        if (ai->HasSkill(SKILL_LEATHERWORKING) && IsItemUsedBySkill(proto, SKILL_LEATHERWORKING))
            return true;
        if (ai->HasSkill(SKILL_ENGINEERING) && IsItemUsedBySkill(proto, SKILL_ENGINEERING))
            return true;
        if (ai->HasSkill(SKILL_BLACKSMITHING) && IsItemUsedBySkill(proto, SKILL_BLACKSMITHING))
            return true;
        if (ai->HasSkill(SKILL_ALCHEMY) && IsItemUsedBySkill(proto, SKILL_ALCHEMY))
            return true;
        if (ai->HasSkill(SKILL_ENCHANTING) && IsItemUsedBySkill(proto, SKILL_ENCHANTING))
            return true;
        if (ai->HasSkill(SKILL_FISHING) && IsItemUsedBySkill(proto, SKILL_FISHING))
            return true;
        if (ai->HasSkill(SKILL_FIRST_AID) && IsItemUsedBySkill(proto, SKILL_FIRST_AID))
            return true;
        if (ai->HasSkill(SKILL_COOKING) && IsItemUsedBySkill(proto, SKILL_COOKING))
            return true;
#ifndef MANGOSBOT_ZERO
        if (ai->HasSkill(SKILL_JEWELCRAFTING) && IsItemUsedBySkill(proto, SKILL_JEWELCRAFTING))
            return true;
#endif
        if (ai->HasSkill(SKILL_MINING) &&
            (
                IsItemUsedBySkill(proto, SKILL_MINING)// ||
                //IsItemUsedBySkill(proto, SKILL_BLACKSMITHING) ||
#ifndef MANGOSBOT_ZERO
                //IsItemUsedBySkill(proto, SKILL_JEWELCRAFTING) ||
#endif
                //IsItemUsedBySkill(proto, SKILL_ENGINEERING)
                ))
            return true;
        if (ai->HasSkill(SKILL_SKINNING) &&
            (IsItemUsedBySkill(proto, SKILL_SKINNING)))// || IsItemUsedBySkill(proto, SKILL_LEATHERWORKING)))
            return true;
        if (ai->HasSkill(SKILL_HERBALISM) &&
            (IsItemUsedBySkill(proto, SKILL_HERBALISM)))// || IsItemUsedBySkill(proto, SKILL_ALCHEMY)))
            return true;
        break;
    }
    case ITEM_CLASS_RECIPE:
    {
        if (bot->HasSpell(GetRecipeSpell(proto)))
            break;

        switch (proto->SubClass)
        {
        case ITEM_SUBCLASS_LEATHERWORKING_PATTERN:
            return ai->HasSkill(SKILL_LEATHERWORKING);
        case ITEM_SUBCLASS_TAILORING_PATTERN:
            return ai->HasSkill(SKILL_TAILORING);
        case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:
            return ai->HasSkill(SKILL_ENGINEERING);
        case ITEM_SUBCLASS_BLACKSMITHING:
            return ai->HasSkill(SKILL_BLACKSMITHING);
        case ITEM_SUBCLASS_COOKING_RECIPE:
            return ai->HasSkill(SKILL_COOKING);
        case ITEM_SUBCLASS_ALCHEMY_RECIPE:
            return ai->HasSkill(SKILL_ALCHEMY);
        case ITEM_SUBCLASS_FIRST_AID_MANUAL:
            return ai->HasSkill(SKILL_FIRST_AID);
        case ITEM_SUBCLASS_ENCHANTING_FORMULA:
            return ai->HasSkill(SKILL_ENCHANTING);
        case ITEM_SUBCLASS_FISHING_MANUAL:
            return ai->HasSkill(SKILL_FISHING);
        }
    }
    }
    return false;
}

bool ItemUsageValue::IsItemNeededForUsefullCraft(ItemPrototype const* proto, bool checkAllReagents)
{
    std::vector<uint32> spellIds = AI_VALUE(std::vector<uint32>, "craft spells");

    for (uint32 spellId : spellIds)
    {
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

        if (!pSpellInfo)
            continue;

        bool isReagentFor = false;
        bool hasOtherReagents = true;

        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
        {
            if (!pSpellInfo->ReagentCount[i] || !pSpellInfo->Reagent[i])
                continue;

            if (pSpellInfo->Reagent[i] == proto->ItemId)
            {
                isReagentFor = true;
            }
            else if (checkAllReagents)
            {
                const ItemPrototype* reqProto = sObjectMgr.GetItemPrototype(pSpellInfo->Reagent[i]);

                uint32 count = AI_VALUE2(uint32, "item count", reqProto->Name1);

                if (count < pSpellInfo->ReagentCount[i])
                    hasOtherReagents = false;
            }
        }

        if (!isReagentFor || !hasOtherReagents)
            continue;

        if (!AI_VALUE2(bool, "should craft spell", spellId))
            continue;

        return true;
    }

    return false;
}

Item* ItemUsageValue::CurrentItem(ItemPrototype const* proto)
{
    Item* bestItem = nullptr;
    std::list<Item*> found = AI_VALUE2(std::list < Item*>, "inventory items", chat->formatItem(proto));

    for (auto item : found)
    {
        if (bestItem && item->GetUInt32Value(ITEM_FIELD_DURABILITY) < bestItem->GetUInt32Value(ITEM_FIELD_DURABILITY))
            continue;

        if (bestItem && item->GetCount() < bestItem->GetCount())
            continue;

        bestItem = item;
    }

    return bestItem;
}


float ItemUsageValue::CurrentStacks(PlayerbotAI* ai, ItemPrototype const* proto)
{
    uint32 maxStack = proto->GetMaxStackSize();

    AiObjectContext* context = ai->GetAiObjectContext();
    ChatHelper* chat = ai->GetChatHelper();

    std::list<Item*> found = AI_VALUE2(std::list<Item*>, "inventory items", chat->formatItem(proto));

    float itemCount = 0;

    for (auto stack : found)
    {
        itemCount += stack->GetCount();
    }

    return itemCount / maxStack;
}

float ItemUsageValue::BetterStacks(ItemPrototype const* proto, std::string itemType)
{
    std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", itemType);

    float stacks = 0;

    for (auto& otherItem : items)
    {
        const ItemPrototype* otherProto = otherItem->GetProto();

        if (otherProto->Class != proto->Class || otherProto->SubClass != proto->SubClass)
            continue;

        if (otherProto->ItemLevel < proto->ItemLevel)
            continue;

        if (otherProto->ItemId == proto->ItemId)
            continue;

        stacks += CurrentStacks(ai, otherProto);
    }

    return stacks;
}


std::vector<uint32> ItemUsageValue::SpellsUsingItem(uint32 itemId, Player* bot)
{
    std::vector<uint32> retSpells;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second.state == PLAYERSPELL_REMOVED || spell.second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
            if (pSpellInfo->ReagentCount[i] > 0 && pSpellInfo->Reagent[i] == itemId)
                retSpells.push_back(spellId);
    }

    return retSpells;
}

bool ItemUsageValue::IsHpFoodOrDrink(ItemPrototype const* proto)
{
    if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE
        && (proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_FOOD
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE_OTHER))
    {
        for (auto spell : proto->Spells)
        {
            if (spell.SpellCategory == 11)
            {
                return true;
            }
        }
    }

    return false;
}

bool ItemUsageValue::IsManaFoodOrDrink(ItemPrototype const* proto)
{
    if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE
        && (proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_FOOD
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE_OTHER))
    {
        for (auto spell : proto->Spells)
        {
            if (spell.SpellCategory == 59)
            {
                return true;
            }
        }
    }

    return false;
}

bool ItemUsageValue::IsHealingPotion(ItemPrototype const* proto)
{
    if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE &&
        (proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_POTION
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_FLASK
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_POTION
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE_OTHER))
    {
        for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
            if (spellInfo)
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->Effect[i] == SPELL_EFFECT_HEAL)
                        return true;
                }
        }
    }

    return false;
}

bool ItemUsageValue::IsManaPotion(ItemPrototype const* proto)
{
    if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE &&
        (proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_POTION
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_FLASK
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_POTION
            || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE_OTHER))
    {
        for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
        {
            const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
            if (spellInfo)
                for (int i = 0; i < 3; i++)
                {
                    if (spellInfo->Effect[i] == SPELL_EFFECT_ENERGIZE)
                        return true;
                }
        }
    }

    return false;
}

bool ItemUsageValue::IsBandage(ItemPrototype const* proto)
{
    if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE
        && proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_BANDAGE)
    {
        return true;
    }

    return false;
}

uint32 ItemUsageValue::GetRecipeSpell(ItemPrototype const* proto)
{
    if (proto->Spells[2].SpellId)
        return proto->Spells[2].SpellId;

    if (proto->Spells[0].SpellId)
    {
        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(proto->Spells[0].SpellId);

        if (!pSpellInfo)
            return 0;

        for (int j = 0; j < 3; ++j)
        {
            if (pSpellInfo->Effect[j] == SPELL_EFFECT_LEARN_SPELL)
            {
                if (pSpellInfo->EffectTriggerSpell[j])
                    return pSpellInfo->EffectTriggerSpell[j];
            }
        }
    }
    return 0;
}

void ItemUsageValue::PopulateProfessionReagentIds()
{
    for (uint32 i = 0; i < sSkillLineStore.GetNumRows(); ++i)
    {
        SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(i);
        if (!skillInfo)
            continue;

        if (skillInfo->categoryId == SKILL_CATEGORY_PROFESSION || skillInfo->categoryId == SKILL_CATEGORY_SECONDARY)
        {
            for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
            {
                SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
                if (!skillLine)
                    continue;

                // wrong skill
                if (skillLine->skillId != skillInfo->id)
                    continue;

                SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(skillLine->spellId);
                if (spellInfo)
                {
                    bool isReagentFor = false;

                    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
                    {
                        if (!spellInfo->ReagentCount[i] || !spellInfo->Reagent[i])
                            continue;

                        m_reagentItemIdsForCraftingSkills[skillLine->skillId].insert(spellInfo->Reagent[i]);
                        m_allReagentItemIdsForCraftingSkills.insert(spellInfo->Reagent[i]);
                        m_allReagentItemIdsForCraftingSkillsVector.push_back(spellInfo->Reagent[i]);
                    }
                }
            }
        }
    }
}

void ItemUsageValue::PopulateReagentItemIdsForCraftableItemIds()
{
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine)
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(skillLine->spellId);
        if (spellInfo)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (spellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM)
                {
                    uint32 craftedItemId = spellInfo->EffectItemType[i];

                    if (craftedItemId)
                    {
                        for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
                        {
                            if (spellInfo->Reagent[x] <= 0)
                            {
                                continue;
                            }

                            uint32 reagentItemId = spellInfo->Reagent[x];
                            uint32 reagentsRequiredCount = spellInfo->ReagentCount[x];
                            if (reagentItemId)
                            {
                                m_craftingReagentItemIdsForCraftableItem[craftedItemId].push_back({ reagentItemId , reagentsRequiredCount });
                            }
                        }
                    }
                }
            }
        }
    }
}

void ItemUsageValue::PopulateSoldByVendorItemIds()
{
    if (auto result = WorldDatabase.PQuery("%s", "SELECT item, entry FROM npc_vendor"))
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            bar.step();
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (!entry)
                continue;
            m_allItemIdsSoldByAnyVendors.insert(fields[0].GetUInt32());
        } while (result->NextRow());
    }

    if (auto result = WorldDatabase.PQuery("%s", "SELECT item, entry FROM npc_vendor WHERE maxcount > 0"))
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            bar.step();
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (!entry)
                continue;
            m_itemIdsSoldByAnyVendorsWithLimitedMaxCount.insert(fields[0].GetUInt32());
        } while (result->NextRow());
    }
}

std::vector<uint32> ItemUsageValue::GetAllReagentItemIdsForCraftingSkillsVector()
{
    return m_allReagentItemIdsForCraftingSkillsVector;
}

std::vector<std::pair<uint32, uint32>> ItemUsageValue::GetAllReagentItemIdsForCraftingItem(ItemPrototype const* proto)
{
    return m_craftingReagentItemIdsForCraftableItem[proto->ItemId];
}

bool ItemUsageValue::IsItemSoldByAnyVendor(ItemPrototype const* proto)
{
    return m_allItemIdsSoldByAnyVendors.count(proto->ItemId) > 0;
}

bool ItemUsageValue::IsItemSoldByAnyVendorButHasLimitedMaxCount(ItemPrototype const* proto)
{
    return m_itemIdsSoldByAnyVendorsWithLimitedMaxCount.count(proto->ItemId) > 0;
}

bool ItemUsageValue::IsItemUsedBySkill(ItemPrototype const* proto, SkillType skillType)
{
    return m_reagentItemIdsForCraftingSkills[skillType].count(proto->ItemId) > 0;
}

bool ItemUsageValue::IsItemUsedToCraftAnything(ItemPrototype const* proto)
{
    return m_allReagentItemIdsForCraftingSkills.count(proto->ItemId) > 0;
}

uint32 ItemUsageValue::GetAHMedianBuyoutPricePerItem(ItemPrototype const* proto)
{
    if (sPlayerbotAIConfig.shouldQueryAHListingsOutsideOfAH)
    {
        auto query = CharacterDatabase.PQuery(
            "  SELECT item_template, AVG(median)"
            "  FROM (SELECT item_template, (buyoutprice / item_count) median"
            "          FROM (SELECT item_template, item_count, buyoutprice, @rownum:= @rownum + 1 as `rownumber`, @total_rows:= @rownum"
            "                  FROM auction soh, (SELECT @rownum:= 0) r WHERE item_template = '%u'"
            "                 ORDER BY(buyoutprice / item_count)) x"
            "         WHERE x.rownumber IN(FLOOR((@total_rows + 1) / 2), FLOOR((@total_rows + 2) / 2))) y"
            "      GROUP BY item_template",
            proto->ItemId
        );
        if (query)
        {
            do
            {
                Field* fields = query->Fetch();

                uint32 itemId = (fields[0].GetUInt32());
                uint32 medianPrice = (fields[1].GetUInt32());

                return medianPrice;
            } while (query->NextRow());
        }
    }

    return 0;
}

uint32 ItemUsageValue::GetAHListingLowestBuyoutPricePerItem(ItemPrototype const* proto)
{
    if (sPlayerbotAIConfig.shouldQueryAHListingsOutsideOfAH)
    {
        auto query = CharacterDatabase.PQuery(
            "SELECT buyoutprice / item_count"
            " FROM auction"
            " WHERE item_template = '%u'"
            " ORDER BY buyoutprice ASC"
            " LIMIT 1",
            proto->ItemId
        );
        if (query)
        {
            do
            {
                Field* fields = query->Fetch();

                uint32 lowestBuyoutPrice = (fields[0].GetUInt32());

                return lowestBuyoutPrice;
            } while (query->NextRow());
        }
    }

    return 0;
}

bool ItemUsageValue::AreCurrentAHListingsTooCheap(ItemPrototype const* proto)
{
    uint32 lowestAhItemListingBuyoutPrice = GetAHListingLowestBuyoutPricePerItem(proto);
    uint32 lowestAcceptapleAhBuyoutPrice = GetBotAHSellMinPrice(proto);

    //check if AH listings are already at the bottom price (with a 1% margin for possible calculation errors and is generally better)
    if (lowestAhItemListingBuyoutPrice > 0 && lowestAhItemListingBuyoutPrice <= lowestAcceptapleAhBuyoutPrice + (lowestAcceptapleAhBuyoutPrice * 0.01f))
    {
        return true;
    }

    return false;
}

bool ItemUsageValue::IsMoreProfitableToSellToAHThanToVendor(ItemPrototype const* proto, Player* bot)
{
    if (proto->Bonding == ItemBondingType::BIND_WHEN_PICKED_UP)
    {
        return false;
    }

    if (AreCurrentAHListingsTooCheap(proto))
    {
        return false;
    }

    for (auto itemId : sPlayerbotAIConfig.ahOverVendorItemIds)
    {
        if (itemId == proto->ItemId)
        {
            return true;
        }
    }

    for (auto itemId : sPlayerbotAIConfig.vendorOverAHItemIds)
    {
        if (itemId == proto->ItemId)
        {
            return false;
        }
    }

    //exceptions
    //cool consumeables?
    if (proto->ItemId == 2662) //noggerfogger
        return true;

    if (proto->Class == ItemClass::ITEM_CLASS_QUEST)
    {
        return true;
    }

    if (IsItemUsedToCraftAnything(proto) && !IsItemSoldByAnyVendor(proto))
    {
        return true;
    }

    if (IsItemSoldByAnyVendorButHasLimitedMaxCount(proto) && !(proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE && proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_SCROLL))
    {
        return true;
    }

    switch (proto->Quality)
    {
    case ItemQualities::ITEM_QUALITY_POOR:
    {
        //gray items usually are only worth as much as their vendor sell price
        return false;
    }
    case ItemQualities::ITEM_QUALITY_NORMAL:
    {
        if (proto->Class == ItemClass::ITEM_CLASS_WEAPON)
        {
            //white weapons is trash
            return false;
        }

        if (proto->Class == ItemClass::ITEM_CLASS_ARMOR)
        {
            //shirts are nice to AH (or other cosmetics something?), also avoid something super cheap like starter boots
            if (proto->SubClass == ItemSubclassArmor::ITEM_SUBCLASS_ARMOR_MISC && GetItemBaseValue(proto) > 10)
            {
                return true;
            }

            //white armor is trash
            return false;
        }

        if (proto->Class == ItemClass::ITEM_CLASS_PROJECTILE)
        {
            //white projectile class is trash
            return false;
        }

        if (proto->Class == ItemClass::ITEM_CLASS_REAGENT)
        {
            //most reagents are sold by vendors
            return !IsItemSoldByAnyVendor(proto);
        }

        if (proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE)
        {
            if (proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_SCROLL)
            {
                //scrolls are trash
                return false;
            }

            //food & drinks
            if ((proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_CONSUMABLE || proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_FOOD))
            {

                return !IsItemSoldByAnyVendor(proto);
            }
        }

        return true;
    }
    //UNCOMMON+ are usually all worth at least the "buy from vendor" price
    case ItemQualities::ITEM_QUALITY_UNCOMMON:
    case ItemQualities::ITEM_QUALITY_RARE:
    case ItemQualities::ITEM_QUALITY_EPIC:
    case ItemQualities::ITEM_QUALITY_LEGENDARY:
    case ItemQualities::ITEM_QUALITY_ARTIFACT:
    {
        return true;
    }
    default:
        break;
    }

    return false;
}

bool ItemUsageValue::IsWorthBuyingFromVendorToResellAtAH(ItemPrototype const* proto, bool isLimitedSupply)
{
    if (proto->Bonding == ItemBondingType::BIND_WHEN_PICKED_UP)
    {
        return false;
    }

    if (AreCurrentAHListingsTooCheap(proto))
    {
        return false;
    }

    for (auto itemId : sPlayerbotAIConfig.vendorOverAHItemIds)
    {
        if (itemId == proto->ItemId)
        {
            return false;
        }
    }

    //exceptions
    //cool consumeables?
    if (proto->ItemId == 2662) //noggerfogger
        return true;

    //do not buy items that require reputation to use
    if (proto->RequiredReputationFaction > 0 || proto->RequiredReputationRank > 0)
    {
        return false;
    }

    //do not buy items that require honor to use
    if (proto->RequiredHonorRank > 0)
    {
        return false;
    }

    if (GetAHMedianBuyoutPricePerItem(proto) > static_cast<uint32>(proto->BuyPrice * 1.1f))
    {
        return true;
    }

    if (isLimitedSupply && !(proto->Class == ItemClass::ITEM_CLASS_CONSUMABLE && proto->SubClass == ItemSubclassConsumable::ITEM_SUBCLASS_SCROLL))
    {
        return true;
    }

    //do not buy items that require skill to use, unless limited supply or recipe (just in case)
    if (proto->RequiredSkill > 0 && !(proto->Class == ItemClass::ITEM_CLASS_RECIPE))
    {
        return false;
    }

    switch (proto->Quality)
    {
    case ItemQualities::ITEM_QUALITY_POOR:
    {
        return false;
    }
    case ItemQualities::ITEM_QUALITY_NORMAL:
    {
        //some white items may be worth it

        if (proto->Class == ItemClass::ITEM_CLASS_RECIPE)
        {
            if (proto->SubClass == ItemSubclassRecipe::ITEM_SUBCLASS_BOOK)
            {
                return true;
            }
        }

        return false;
    }
    //UNCOMMON+ are usually all worth it
    case ItemQualities::ITEM_QUALITY_UNCOMMON:
    case ItemQualities::ITEM_QUALITY_RARE:
    case ItemQualities::ITEM_QUALITY_EPIC:
    case ItemQualities::ITEM_QUALITY_LEGENDARY:
    case ItemQualities::ITEM_QUALITY_ARTIFACT:
    {
        return true;
    }
    default:
        break;
    }

    return false;
}

bool ItemUsageValue::IsWorthBuyingFromAhToResellAtAH(ItemPrototype const* proto, uint32 totalCost, uint32 itemCount)
{
    uint32 pricePerItem = totalCost / itemCount;

    //bottom half is probably always worth buying? That should reduce oversupply
    return pricePerItem <= GetBotAHSellMinPrice(proto) + ((GetBotAHSellMaxPrice(proto) - GetBotAHSellMinPrice(proto)) / 2);
}

double ItemUsageValue::GetRarityPriceMultiplier(ItemPrototype const* proto)
{
    float x = sRandomItemMgr.GetItemRarity(proto->ItemId);
    if (x < 0.001) return 1.0f;
    return 0.75 + exp((140 - x) / 50) / 6;
}

double ItemUsageValue::GetLevelPriceMultiplier(ItemPrototype const* proto)
{
    float x = 0.1f + proto->ItemLevel;
    return 0.5 + exp(x / 60) / 2;
}

uint32 ItemUsageValue::GetItemBaseValue(ItemPrototype const* proto, uint8 maxReagentLevel)
{
    if (proto->Quality == ItemQualities::ITEM_QUALITY_POOR)
    {
        return proto->SellPrice;
    }

    if (IsItemSoldByAnyVendor(proto))
    {
        //if item is sold by a vendor - price can never be lower than BuyPrice, because bots may buy from vendor to resell
        return proto->BuyPrice;
    }

    //calculate total value of reagents if the item is craftable
    if (maxReagentLevel > 0 && GetAllReagentItemIdsForCraftingItem(proto).size() > 0)
    {
        maxReagentLevel--;
        uint32 totalReagentsValue = 0;

        for (auto idCountPair : GetAllReagentItemIdsForCraftingItem(proto))
        {
            ItemPrototype const* reagentProto = ObjectMgr::GetItemPrototype(idCountPair.first);
            totalReagentsValue += GetItemBaseValue(reagentProto, maxReagentLevel) * idCountPair.second;
        }

        if (totalReagentsValue > 0)
        {
            return totalReagentsValue;
        }
    }

    //some items, which are not sold by vendors, have very low or very high vendor buy price, can't rely on it, need to adjust SellPrice
    return static_cast<uint32>(proto->SellPrice * GetRarityPriceMultiplier(proto) * GetLevelPriceMultiplier(proto) * 1.5f);
}

/*
* bots buy at this price
*/
uint32 ItemUsageValue::GetBotBuyPrice(ItemPrototype const* proto, Player* bot)
{
    return static_cast<uint32>(GetItemBaseValue(proto) * sRandomPlayerbotMgr.GetBuyMultiplier(bot));
}

/*
* bots sell at this price
*/
uint32 ItemUsageValue::GetBotSellPrice(ItemPrototype const* proto, Player* bot)
{
    //should never sell for less than sell to vendor price
    return std::max(
        static_cast<uint32>((GetItemBaseValue(proto) + 1) * sRandomPlayerbotMgr.GetSellMultiplier(bot)),
        static_cast<uint32>(proto->SellPrice * 1.1f)
    );
}

uint32 ItemUsageValue::GetBotAHSellMinPrice(ItemPrototype const* proto)
{
    //should never sell for less than base value
    // multiplied by % to give room for those who buy from vendor and sell to AH
    return static_cast<uint32>((GetItemBaseValue(proto) + 1) * 1.01f);
}

uint32 ItemUsageValue::GetBotAHSellMaxPrice(ItemPrototype const* proto)
{
    return static_cast<uint32>(GetItemBaseValue(proto) * 2.5f);
}

uint32 ItemUsageValue::GetCraftingFee(ItemPrototype const* proto)
{
    uint32 fixedMinCraftingFee = 100;
    uint32 level = std::max(proto->ItemLevel, proto->RequiredLevel);
    return fixedMinCraftingFee * level * level / 40;
}