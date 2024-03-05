
#include "playerbot/playerbot.h"
#include "UseTrinketAction.h"
#include "Entities/Item.h"
#include "Entities/ItemPrototype.h"
#include "Entities/Player.h"

using namespace ai;

bool UseTrinketAction::Execute(Event& event)
{
	Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
	std::list<Item*> trinkets = AI_VALUE(std::list<Item*>, "trinkets on use");
	for (Item* item : trinkets)
	{
		const ItemPrototype* proto = item->GetProto();
		if (proto->InventoryType == INVTYPE_TRINKET && item->IsEquipped())
		{
			if (bot->CanUseItem(item) == EQUIP_ERR_OK && !item->IsInTrade())
			{
				return UseItemAuto(requester, item);
			}
		}
	}

    return false;
}

bool UseTrinketAction::isPossible()
{
	return !AI_VALUE(std::list<Item*>, "trinkets on use").empty();
}
