
#include "playerbot/playerbot.h"
#include "UseTrinketAction.h"
#include "Entities/Item.h"
#include "Entities/ItemPrototype.h"
#include "Entities/Player.h"

using namespace ai;

bool UseTrinketAction::Execute(Event& event)
{
    Player* requester = event.getOwner();
    std::list<Item*> trinkets = AI_VALUE(std::list<Item*>, "trinkets on use");

    for (Item* item : trinkets)
    {
        const ItemPrototype* proto = item->GetProto();
        if (!proto || proto->InventoryType != INVTYPE_TRINKET)
            continue;

        if (!allowPvPTrinket && IsPvPTrinket(item))  // Prevent PvP trinkets unless allowed
            continue;

        if (CanUseTrinket(item))
        {
            return UseItem(requester, item->GetEntry());
        }
    }

    return false;
}

bool UseTrinketAction::isPossible()
{
    return !AI_VALUE(std::list<Item*>, "trinkets on use").empty();
}

bool UseTrinketAction::CanUseTrinket(Item* item)
{
    return item && item->IsEquipped() && bot->CanUseItem(item) == EQUIP_ERR_OK && !item->IsInTrade();
}

bool UseTrinketAction::IsPvPTrinket(Item* item)
{
    if (!item) return false;
    uint32 entry = item->GetEntry();
    return (entry == 51377 || entry == 51378);  // PvP Trinket IDs
}
