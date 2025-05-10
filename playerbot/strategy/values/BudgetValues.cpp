#include "playerbot/playerbot.h"
#include "BudgetValues.h"
#include "ItemUsageValue.h"
#include "MountValues.h"

using namespace ai;

uint32 RepairCostValue::RepairCost(const Item* item, bool fullCost)
{
    uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
    if (!maxDurability)
        return 0;

    uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

    uint32 LostDurability = maxDurability;
        
    if (!fullCost)
    {
        LostDurability = LostDurability - curDurability;

        if (LostDurability == 0)
            return 0;
    }

    ItemPrototype const* ditemProto = item->GetProto();

    DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
    if (!dcost)
        return 0;

    uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
    DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
    if (!dQualitymodEntry)
        return 0;

    uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];

    return uint32(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));
}

uint32 RepairCostValue::Calculate()
{
    uint32 TotalCost = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        
        Item* item = bot->GetItemByPos(pos);

        if (!item)
            continue;

        TotalCost += RepairCost(item);
    }

    return TotalCost;
}

uint32 MaxGearRepairCostValue::Calculate()
{
    uint32 TotalCost = 0;
    for (int i = EQUIPMENT_SLOT_START; i < INVENTORY_SLOT_ITEM_END; ++i)
    {
        uint16 pos = ((INVENTORY_SLOT_BAG_0 << 8) | i);
        Item* item = bot->GetItemByPos(pos);

        if (!item)
            continue;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
        if (!maxDurability)
            continue;

        uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

        if (i >= EQUIPMENT_SLOT_END && curDurability >= maxDurability) //Only count items equiped or already damanged.
            continue;        

        TotalCost += RepairCost(item, true);
    }

    return TotalCost;
}



uint32 MoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    uint32 moneyWanted = 0;

    uint32 level = bot->GetLevel();

    switch (needMoneyFor)
    {
    case NeedMoneyFor::none:
        moneyWanted = 0;
        break;
    case NeedMoneyFor::repair:
        moneyWanted = AI_VALUE(uint32, "max repair cost");
        break;
    case NeedMoneyFor::ammo:
        moneyWanted = (bot->getClass() == CLASS_HUNTER) ? (level * level * level) / 10 : 0; //Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo replace (should be best ammo buyable x 8 stacks cost)
        break;
    case NeedMoneyFor::spells:
        moneyWanted = AI_VALUE2(uint32, "train cost", TRAINER_TYPE_CLASS);
        break;
    case NeedMoneyFor::travel:
        moneyWanted = bot->isTaxiCheater() ? 0 : 1500; //15s for traveling half a continent. Todo: Add better calculation (Should be ???)
        break;
    case NeedMoneyFor::gear:
        moneyWanted = level * level * level; //Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80): Todo replace (Should be ~total cost of all >green gear equiped)
        break;
    case NeedMoneyFor::consumables:
        moneyWanted = (level * level * level) / 10; //Or level^3 (1s @ lvl10, 30s @ lvl30, 2g @ lvl60, 5g @ lvl80): Todo replace (Should be best food/drink x 2 stacks cost)
        break;
    case NeedMoneyFor::guild:
        if (ai->HasStrategy("guild", BotState::BOT_STATE_NON_COMBAT))
        {
            if (bot->GetGuildId() && bot->GetLevel() > 20)
                moneyWanted = AI_VALUE2(uint32, "item count", chat->formatQItem(5976)) ? 0 : 10000; //1g (tabard)
            else if (bot->GetLevel() > 10)
                moneyWanted = AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) ? 0 : 1000; //10s (guild charter)
        }
        break;
    case NeedMoneyFor::skilltraining:
        moneyWanted = AI_VALUE2(uint32, "train cost", TRAINER_TYPE_TRADESKILLS);
        break;
    case NeedMoneyFor::tradeskill:
        moneyWanted = (level * level * level); //Or level^3 (10s @ lvl10, 3g @ lvl30, 20g @ lvl60, 50g @ lvl80): Todo replace (Should be buyable reagents that combined allow crafting of usefull items)
        break;
    case NeedMoneyFor::ah:
    {
        //Save deposit needed for all items the bot wants to AH.
        uint32 time;
#ifdef MANGOSBOT_ZERO
        time = 8 * HOUR;
#else
        time = 12 * HOUR;
#endif
        float totalDeposit = 0;
        for (auto item : AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_AH)))
        {
            float deposit = float(item->GetProto()->SellPrice * item->GetCount() * (time / MIN_AUCTION_TIME));

            deposit = deposit * 15 * 3.0f / 100.0f;

            float min_deposit = float(sWorld.getConfig(CONFIG_UINT32_AUCTION_DEPOSIT_MIN));

            if (deposit < min_deposit)
                deposit = min_deposit;

            deposit *= sWorld.getConfig(CONFIG_FLOAT_RATE_AUCTION_DEPOSIT);

            totalDeposit += deposit;
        }

        for (auto item : AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_BROKEN_AH)))
        {
            float deposit = float(item->GetProto()->SellPrice * item->GetCount() * (time / MIN_AUCTION_TIME));

            deposit = deposit * 15 * 3.0f / 100.0f;

            float min_deposit = float(sWorld.getConfig(CONFIG_UINT32_AUCTION_DEPOSIT_MIN));

            if (deposit < min_deposit)
                deposit = min_deposit;

            deposit *= sWorld.getConfig(CONFIG_FLOAT_RATE_AUCTION_DEPOSIT);

            totalDeposit += deposit;

            uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            if (!maxDurability)
                continue;

            uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);

            uint32 LostDurability = maxDurability - curDurability;

            if (LostDurability == 0)
                continue;

            ItemPrototype const* ditemProto = item->GetProto();

            DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
            if (!dcost)
                continue;

            uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
            DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
            if (!dQualitymodEntry)
                continue;

            uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];
            uint32 costs = uint32(LostDurability * dmultiplier * double(dQualitymodEntry->quality_mod));

            totalDeposit += costs;
        }

        return totalDeposit;
        break;
    }
    case NeedMoneyFor::mount:
    {
        uint32 maxMountSpeed = 0;
        uint32 maxFlyMountSpeed = 0;

        moneyWanted = AI_VALUE2(uint32, "train cost", TRAINER_TYPE_MOUNTS);

        for (auto& mount : AI_VALUE(std::vector<MountValue>, "mount list"))
        {
            if (mount.GetSpeed(false) > maxMountSpeed)
            {
                maxMountSpeed = mount.GetSpeed(false);
                maxFlyMountSpeed = mount.GetSpeed(true);
            }
        }

        if (level >= 40)
        {
            if (maxMountSpeed < 59)
                moneyWanted += 10 * GOLD;
        }
        if (level >= 60)
        {
            if (maxMountSpeed < 99)
                moneyWanted += 100 * GOLD;
        }
        if (level >= 70)
        {
            if (maxFlyMountSpeed < 99)
                moneyWanted += 100 * GOLD;
            else if (maxFlyMountSpeed < 279)
                moneyWanted += 200 * GOLD;
        }
        //todo WOTLK
        break;
    }
    }

    return moneyWanted;
};

uint32 TotalMoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    uint32 moneyWanted = AI_VALUE2(uint32, "money needed for", (uint32)needMoneyFor);

    auto needPtr = std::find(saveMoneyFor.begin(), saveMoneyFor.end(), needMoneyFor);

    while (needPtr != saveMoneyFor.begin())
    {
        needPtr--;

        NeedMoneyFor alsoNeed = *needPtr;

        moneyWanted = moneyWanted + AI_VALUE2(uint32, "money needed for", (uint32)alsoNeed);
    }

    return moneyWanted;
}

bool HasAllMoneyForValue::Calculate()
{
    uint32 money = bot->GetMoney();

    if (ai->HasCheat(BotCheatMask::gold))
        return true;

    uint32 needMoney;

    if (ai->HasActivePlayerMaster())
        needMoney = AI_VALUE2(uint32, "money needed for", getQualifier());
    else
        needMoney = AI_VALUE2(uint32, "total money needed for", getQualifier());

    if (needMoney <= money)
        return true;

    return false;
}


uint32 FreeMoneyForValue::Calculate() 
{
    uint32 money = bot->GetMoney();

    if (ai->HasCheat(BotCheatMask::gold))
        return 10000000;

    if (ai->HasActivePlayerMaster())
        return money;

    uint32 savedMoney = AI_VALUE2(uint32, "total money needed for", getQualifier()) - AI_VALUE2(uint32, "money needed for", getQualifier());

    if (savedMoney > money)
        return 0;

    return money - savedMoney;
};