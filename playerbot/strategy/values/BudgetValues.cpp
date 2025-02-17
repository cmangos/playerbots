#include "playerbot/playerbot.h"
#include "BudgetValues.h"
#include "ItemUsageValue.h"
#include "MountValues.h"

using namespace ai;

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

        ItemPrototype const* ditemProto = item->GetProto();

        DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
        if (!dcost)
            continue;

        uint32 dQualitymodEntryId = (ditemProto->Quality + 1) * 2;
        DurabilityQualityEntry const* dQualitymodEntry = sDurabilityQualityStore.LookupEntry(dQualitymodEntryId);
        if (!dQualitymodEntry)
            continue;

        uint32 dmultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];

        uint32 costs = uint32(maxDurability * dmultiplier * double(dQualitymodEntry->quality_mod));


        TotalCost += costs;
    }

    return TotalCost;
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

        TotalCost += costs;
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
        if (bot->getClass() == CLASS_HUNTER) {
            moneyWanted = (level * level * level) / 10;  // Approximation based on level
        }
        break;

    case NeedMoneyFor::spells:
        moneyWanted = AI_VALUE2(uint32, "train cost", TRAINER_TYPE_CLASS);
        break;

    case NeedMoneyFor::travel:
        moneyWanted = bot->isTaxiCheater() ? 0 : 1500;  // 15s for standard travel
        break;

    case NeedMoneyFor::gear:
    case NeedMoneyFor::tradeskill:
        moneyWanted = level * level * level;  // Same formula for both cases
        break;

    case NeedMoneyFor::consumables:
        moneyWanted = (level * level * level) / 10;  // Approximation based on level
        break;

    case NeedMoneyFor::guild:
        if (ai->HasStrategy("guild", BotState::BOT_STATE_NON_COMBAT)) {
            if (bot->GetGuildId() && level > 20) {
                moneyWanted = AI_VALUE2(uint32, "item count", chat->formatQItem(5976)) ? 0 : 10000;  // Tabard cost
            }
            else if (level > 10) {
                moneyWanted = AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) ? 0 : 1000;  // Guild charter cost
            }
        }
        break;
    case NeedMoneyFor::skilltraining:
        moneyWanted = AI_VALUE2(uint32, "train cost", TRAINER_TYPE_TRADESKILLS);
        break;
    case NeedMoneyFor::ah:
    {
        //Save deposit needed for all items the bot wants to AH.
        uint32 time;
#ifdef MANGOSBOT_ZERO
            8 * HOUR;
#else
            12 * HOUR;
#endif
        float totalDeposit = 0;

        auto calculateDeposit = [&](Item* item) {
            float deposit = float(item->GetProto()->SellPrice * item->GetCount() * (time / MIN_AUCTION_TIME));
            deposit *= 15 * 3.0f / 100.0f;

            float min_deposit = float(sWorld.getConfig(CONFIG_UINT32_AUCTION_DEPOSIT_MIN));
            if (deposit < min_deposit) {
                deposit = min_deposit;
            }
            deposit *= sWorld.getConfig(CONFIG_FLOAT_RATE_AUCTION_DEPOSIT);
            return deposit;
            };

        for (Item* item : AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_AH))) {
            totalDeposit += calculateDeposit(item);
        }

        for (Item* item : AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_BROKEN_AH))) {
            totalDeposit += calculateDeposit(item);

            // Additional cost for broken items
            uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);
            if (!maxDurability) continue;

            uint32 curDurability = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
            uint32 lostDurability = maxDurability - curDurability;
            if (!lostDurability) continue;

            ItemPrototype const* ditemProto = item->GetProto();
            DurabilityCostsEntry const* dcost = sDurabilityCostsStore.LookupEntry(ditemProto->ItemLevel);
            if (!dcost) continue;

            uint32 dQualityModEntryId = (ditemProto->Quality + 1) * 2;
            DurabilityQualityEntry const* dQualityModEntry = sDurabilityQualityStore.LookupEntry(dQualityModEntryId);
            if (!dQualityModEntry) continue;

            uint32 dMultiplier = dcost->multiplier[ItemSubClassToDurabilityMultiplierId(ditemProto->Class, ditemProto->SubClass)];
            uint32 costs = uint32(lostDurability * dMultiplier * double(dQualityModEntry->quality_mod));
            totalDeposit += costs;
        }

        return static_cast<uint32>(totalDeposit);
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
            }
            if (mount.GetSpeed(true) > maxFlyMountSpeed)
            {
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
        if (level >= 77)
        {
            if (!bot->HasSpell(54197))
                moneyWanted += 1000 * GOLD;
        }
        break;
    }
    }

    return moneyWanted;
};

uint32 TotalMoneyNeededForValue::Calculate()
{
    NeedMoneyFor needMoneyFor = NeedMoneyFor(stoi(getQualifier()));

    auto needPtr = std::find(saveMoneyFor.begin(), saveMoneyFor.end(), needMoneyFor);

    if (needPtr == saveMoneyFor.end()) {
        return 0;
    }

    uint32 moneyWanted = 0;

    for (auto it = needPtr; it >= saveMoneyFor.begin(); --it) {
        moneyWanted += AI_VALUE2(uint32, "money needed for", static_cast<uint32>(*it));
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
        return 100000000;

    if (ai->HasActivePlayerMaster() || money > 10000 * 10000)
        return money;

    uint32 savedMoney = AI_VALUE2(uint32, "total money needed for", getQualifier()) - AI_VALUE2(uint32, "money needed for", getQualifier());

    if (savedMoney > money)
        return 0;

    return money - savedMoney;
};