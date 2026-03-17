
#include "playerbot/playerbot.h"
#include "GuildValues.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/ServerFacade.h"
#include "Guilds/GuildMgr.h"

using namespace ai;

std::string GuildOrderValue::TrimWhitespace(const std::string& str)
{
    std::string result = str;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());
    return result;
}

bool GuildOrderValue::ParseOrderPrefix(const std::string& note, const std::string& prefix, std::string& outBody)
{
    auto pos = note.find(prefix);
    if (pos == std::string::npos)
        return false;

    std::string body = TrimWhitespace(note.substr(pos + prefix.size()));
    if (body.empty())
        return false;

    outBody = body;
    return true;
}

char* strstri(const char* haystack, const char* needle);

uint32 GuildOrderValue::FindItemByName(const std::string& name)
{
    uint32 substringMatch = 0;

    for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
    {
        ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(itemId);
        if (!proto)
            continue;

        if (name.size() == strlen(proto->Name1) && strstri(proto->Name1, name.c_str()))
            return itemId;

        if (!substringMatch && strstri(proto->Name1, name.c_str()))
            substringMatch = itemId;
    }

    return substringMatch;
}

GuildOrder GuildOrderValue::Calculate()
{
    GuildOrder order;

    if (!bot->GetGuildId())
        return order;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return order;

    MemberSlot* member = guild->GetMemberSlot(bot->GetObjectGuid());
    if (!member)
        return order;

    std::string note = member->OFFnote;
    if (note.empty())
        return order;

    std::string body;

    if (ParseOrderPrefix(note, "Craft:", body))
    {
        order.type = GuildOrderType::Craft;

        // Parse optional amount for craft orders:
        // Craft: <item> <amount>
        auto lastSpace = body.rfind(' ');
        if (lastSpace != std::string::npos)
        {
            std::string lastToken = body.substr(lastSpace + 1);
            bool isNumber = !lastToken.empty() && std::all_of(lastToken.begin(), lastToken.end(), ::isdigit);

            if (isNumber)
            {
                order.amount = std::stoul(lastToken);
                order.target = TrimWhitespace(body.substr(0, lastSpace));
                return order;
            }
        }

        order.target = body;
        order.amount = 1;
    }
    else if (ParseOrderPrefix(note, "Farm:", body))
    {
        order.type = GuildOrderType::Farm;

        // Parse optional amount for farm orders:
        // Farm: <item> <amount>
        auto lastSpace = body.rfind(' ');
        if (lastSpace != std::string::npos)
        {
            std::string lastToken = body.substr(lastSpace + 1);
            bool isNumber = !lastToken.empty() && std::all_of(lastToken.begin(), lastToken.end(), ::isdigit);

            if (isNumber)
            {
                order.amount = std::stoul(lastToken);
                order.target = TrimWhitespace(body.substr(0, lastSpace));
                return order;
            }
        }

        order.target = body;
    }
    else if (ParseOrderPrefix(note, "Kill:", body))
    {
        order.type = GuildOrderType::Kill;
        order.target = body;
    }
    else if (ParseOrderPrefix(note, "Explore:", body))
    {
        order.type = GuildOrderType::Explore;
        order.target = body;
    }

    return order;
}

GuildShareTarget GuildShareTargetValue::Calculate()
{
    GuildShareTarget result;

    if (!bot->GetGuildId())
        return result;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return result;

    // Collect items this bot has in bags (potential items to share)
    std::vector<Item*> botItems = ai->GetInventoryItems();
    if (botItems.empty())
        return result;

    // Build a set of item IDs this bot has, excluding items the bot wants to keep
    std::map<uint32, Item*> sharableItems;
    for (Item* item : botItems)
    {
        uint32 itemId = item->GetProto()->ItemId;

        // Skip items we already checked
        if (sharableItems.count(itemId))
            continue;

        // Don't give away items we have set to keep/need/equip/greed ourselves
        ForceItemUsage botForce = ai->GetAiObjectContext()->GetValue<ForceItemUsage>("force item usage", std::to_string(itemId))->Get();
        if (botForce != ForceItemUsage::FORCE_USAGE_NONE)
            continue;

        sharableItems[itemId] = item;
    }

    if (sharableItems.empty())
        return result;

    // Check nearby guild members
    std::list<ObjectGuid> nearGuids = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest friendly players")->Get();

    for (auto& guid : nearGuids)
    {
        Player* player = sObjectMgr.GetPlayer(guid);
        if (!player || player == bot)
            continue;

        // Must be in the same guild
        if (player->GetGuildId() != bot->GetGuildId())
            continue;

        // Must be a bot with AI
        PlayerbotAI* targetAi = player->GetPlayerbotAI();
        if (!targetAi)
            continue;

        // Must be nearby enough to trade
        if (sServerFacade.GetDistance2d(bot, player) > INTERACTION_DISTANCE)
            continue;

        // For each sharable item, check if the target has "keep need" set for it
        for (auto& [itemId, item] : sharableItems)
        {
            // Check if the target has "need" set for this item
            ForceItemUsage targetForce = targetAi->GetAiObjectContext()->GetValue<ForceItemUsage>("force item usage", std::to_string(itemId))->Get();
            if (targetForce != ForceItemUsage::FORCE_USAGE_NEED)
                continue;

            // Check if the target doesn't already have the item
            if (targetAi->GetInventoryItemsCountWithId(itemId) > 0)
                continue;

            result.receiver = player;
            result.itemId = itemId;
            return result;
        }
    }

    return result;
}

uint8 PetitionSignsValue::Calculate()
{
    if (bot->GetGuildId())
       return 0;

    std::list<Item*> petitions = AI_VALUE2(std::list<Item*>, "inventory items", chat->formatQItem(5863));

    if (petitions.empty())
        return 0;

    auto result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitions.front()->GetObjectGuid().GetCounter());

    return result ? (uint8)result->GetRowCount() : 0;
};

bool CanBuyTabard::Calculate()
{
	if (!bot->GetGuildId())
		return false;

	if (AI_VALUE(TravelTarget*,"travel target")->GetStatus() == TravelStatus::TRAVEL_STATUS_TRAVEL)
		return false;

	bool inCity = false;
	AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(sServerFacade.GetAreaId(bot));
	if (areaEntry)
	{
		if (areaEntry->zone)
			areaEntry = GetAreaEntryByAreaID(areaEntry->zone);

		if (areaEntry && areaEntry->flags & AREA_FLAG_CAPITAL)
			inCity = true;
	}

	if (!inCity)
		return false;

	if (AI_VALUE2(uint32, "item count", chat->formatQItem(5976)))
		return false;

	if (AI_VALUE2(uint32, "free money for", uint32(NeedMoneyFor::guild)) < 10000)
		return false;

	return true;
}
