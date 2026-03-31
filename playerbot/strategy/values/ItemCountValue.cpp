#include "playerbot/playerbot.h"
#include "ItemCountValue.h"
#include "Entities/Item.h"
#include "Entities/ItemPrototype.h"
#include "Entities/Player.h"
#include <unordered_map>
#include <ctime>

using namespace ai;

struct CacheEntry { uint32 count; time_t tick; };
static std::unordered_map<Player*, std::unordered_map<std::string, CacheEntry>> g_itemCountCache;

static std::list<Item*> Find(PlayerbotAI* ai, std::string qualifier)
{
    std::list<Item*> result;
    Player* bot = ai->GetBot();
	IterateItemsMask mask = IterateItemsMask((uint8)IterateItemsMask::ITERATE_ITEMS_IN_EQUIP | (uint8)IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    result = ai->InventoryParseItems(qualifier, mask);
    return result;
}

uint32 ItemCountValue::Calculate()
{
    Player* bot = ai->GetBot();
    time_t currentTick = std::time(nullptr);

    auto& botCache = g_itemCountCache[bot];
    auto it = botCache.find(qualifier);
    if (it != botCache.end())
    {
        if (it->second.tick == currentTick)
            return it->second.count;
    }

    uint32 count = 0;
    std::list<Item*> items = Find(ai, qualifier);
    for (Item* item : items)
        count += item->GetCount();

    botCache[qualifier] = CacheEntry{ count, currentTick };
    return count;
}

std::list<Item*> InventoryItemValue::Calculate()
{
    return Find(ai, qualifier);
}

std::list<Item*> EquipedUsableTrinketValue::Calculate()
{
	std::list<Item*> trinkets;
	std::list<Item*> result;

	Player* bot = ai->GetBot();

	if (Item* trinket1 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET1))
		trinkets.push_back(trinket1);

	if (Item* trinket2 = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_TRINKET2))
		trinkets.push_back(trinket2);

	if (trinkets.empty())
		return result;

	for (Item * item : trinkets)
	{
		ItemPrototype const* proto = item->GetProto();

		for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
		{
			if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE &&
				proto->Spells[i].SpellId > 0 &&
				bot->IsSpellReady(proto->Spells[i].SpellId, proto))
			{
				result.push_back(item);
			}
		}
	}

	return result;
}

namespace ai {
    void InvalidateItemCountCache(Player* bot)
    {
        if (!bot) return;
        g_itemCountCache.erase(bot);
    }

    void InvalidateItemCountCache(Player* bot, const std::string& qualifier)
    {
        if (!bot) return;
        auto it = g_itemCountCache.find(bot);
        if (it != g_itemCountCache.end())
            it->second.erase(qualifier);
    }
}