
#include "MaintenanceValues.h"
#include "Mails/Mail.h"

using namespace ai;

bool ShouldAHSellValue::Calculate() 
{
    if (ShouldSellValue::Calculate()) //We need space so we want to try to AH items anyway.
        return true;

    std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", "inventory");

    for (auto& item : items)
    {
        if (!item->GetUInt32Value(ITEM_FIELD_DURABILITY)) //Does the item need to be repaired?
            continue;

        uint32 maxDurability = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

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

        if (bot->GetMoney() && (costs * 100) / bot->GetMoney() <= 1) //Would repairing this item use more than 1% of our current gold?
            continue;

        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", ItemQualifier(item).GetQualifier());

        if (usage != ItemUsage::ITEM_USAGE_AH && usage != ItemUsage::ITEM_USAGE_BROKEN_AH) //Do we want to AH this item?
            continue;

        return true; //We have an item that can be damaged (and gives significant repair cost) that we want to auction. We should auction it now!
    }

    return false;
}


bool CanGetMailValue::Calculate() {
    if (!ai->HasStrategy("rpg vendor", BotState::BOT_STATE_NON_COMBAT))
        return false;

    if (AI_VALUE(bool, "should sell"))
        return false;

    time_t cur_time = time(0);

    for (PlayerMails::iterator itr = bot->GetMailBegin(); itr != bot->GetMailEnd(); ++itr)
    {
        if ((*itr)->state == MAIL_STATE_DELETED || cur_time < (*itr)->deliver_time)
            continue;

        if ((*itr)->has_items || (*itr)->money)
        {
            return true;
        }
    }

    return false;
}