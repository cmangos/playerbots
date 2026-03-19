
#include "playerbot/playerbot.h"
#include "GuildValues.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/strategy/values/SharedValueContext.h"
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

uint32 ai::CountGuildFinishedItemDeficit(Player* bot, uint32 itemId, const std::vector<GuildShareItemEntry>& shareList)
{
    if (!bot->GetGuildId())
        return 0;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return 0;

    struct CountOnlineMembers
    {
        uint32 itemId;
        const std::vector<GuildShareItemEntry>& shareList;
        uint32 deficit;

        CountOnlineMembers(uint32 id, const std::vector<GuildShareItemEntry>& list)
            : itemId(id), shareList(list), deficit(0) {}

        void operator()(Player* player)
        {
            if (!player)
                return;

            for (const auto& entry : shareList)
            {
                if (entry.itemId != itemId)
                    continue;

                if (!entry.MatchesPlayer(player))
                    continue;

                uint32 has = 0;
                if (PlayerbotAI* memberAi = player->GetPlayerbotAI())
                    has = memberAi->GetInventoryItemsCountWithId(itemId);
                else
                    has = player->GetItemCount(itemId, true);

                if (has < entry.amount)
                    deficit += entry.amount - has;

                break;
            }
        }
    };

    CountOnlineMembers worker(itemId, shareList);
    guild->BroadcastWorker(worker);

    return worker.deficit;
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
    {
        GuildOrder craftOrder = AI_VALUE(GuildOrder, "guild share craft order");
        if (craftOrder.IsValid())
            return craftOrder;

        return AI_VALUE(GuildOrder, "guild share farm order");
    }

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

    if (!order.IsValid())
    {
        GuildOrder craftOrder = AI_VALUE(GuildOrder, "guild share craft order");
        if (craftOrder.IsValid())
            return craftOrder;

        return AI_VALUE(GuildOrder, "guild share farm order");
    }

    return order;
}

static std::string TrimWS(const std::string& str)
{
    std::string result = str;
    result.erase(result.begin(), std::find_if(result.begin(), result.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    result.erase(std::find_if(result.rbegin(), result.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), result.end());
    return result;
}

uint8 GuildShareListValue::ParseClassName(const std::string& name)
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "warrior")  return CLASS_WARRIOR;
    if (lower == "paladin")  return CLASS_PALADIN;
    if (lower == "hunter")   return CLASS_HUNTER;
    if (lower == "rogue")    return CLASS_ROGUE;
    if (lower == "priest")   return CLASS_PRIEST;
    if (lower == "shaman")   return CLASS_SHAMAN;
    if (lower == "mage")     return CLASS_MAGE;
    if (lower == "warlock")  return CLASS_WARLOCK;
    if (lower == "druid")    return CLASS_DRUID;
#ifdef CLASS_DEATH_KNIGHT
    if (lower == "deathknight" || lower == "death knight" || lower == "dk") return CLASS_DEATH_KNIGHT;
#endif
    return 0;
}

GuildShareFilter GuildShareListValue::ParseRoleFilter(const std::string& name)
{
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    if (lower == "all")    return GuildShareFilter::FILTER_ALL;
    if (lower == "melee")  return GuildShareFilter::FILTER_MELEE;
    if (lower == "ranged") return GuildShareFilter::FILTER_RANGED;
    if (lower == "tank")   return GuildShareFilter::FILTER_TANK;
    if (lower == "dps")    return GuildShareFilter::FILTER_DPS;
    if (lower == "heal")   return GuildShareFilter::FILTER_HEAL;

    return GuildShareFilter::FILTER_CLASS;
}

bool GuildShareItemEntry::MatchesPlayer(Player* player) const
{
    if (!player)
        return false;

    switch (filter)
    {
    case GuildShareFilter::FILTER_CLASS:
        return player->getClass() == playerClass;

    case GuildShareFilter::FILTER_ALL:
        return true;

    case GuildShareFilter::FILTER_MELEE:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        return ai && !ai->IsRanged(player, false);
    }

    case GuildShareFilter::FILTER_RANGED:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        return ai && ai->IsRanged(player, false);
    }

    case GuildShareFilter::FILTER_TANK:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        return ai && ai->IsTank(player, false);
    }

    case GuildShareFilter::FILTER_DPS:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        return ai && !ai->IsTank(player, false) && !ai->IsHeal(player, false);
    }

    case GuildShareFilter::FILTER_HEAL:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        return ai && ai->IsHeal(player, false);
    }

    default:
        return false;
    }
}

// Parse the guild info tab "Share: <class/role> <item1> <amount>, <item2> <amount>, ..."
std::vector<GuildShareItemEntry> GuildShareListValue::Calculate()
{
    std::vector<GuildShareItemEntry> result;

    if (!bot->GetGuildId())
        return result;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return result;

    std::string ginfo = guild->GetGINFO();
    if (ginfo.empty())
        return result;

    auto sharePos = ginfo.find("Share:");
    if (sharePos == std::string::npos)
        return result;

    std::string shareSection = ginfo.substr(sharePos + 6);

    std::istringstream stream(shareSection);
    std::string line;

    while (std::getline(stream, line))
    {
        line = TrimWS(line);
        if (line.empty())
            continue;

        auto firstSpace = line.find(' ');
        if (firstSpace == std::string::npos)
            continue;

        std::string filterName = TrimWS(line.substr(0, firstSpace));

        if (!filterName.empty() && filterName.back() == ':')
            filterName.pop_back();

        // Try role filter first (All, Melee, Ranged, Tank, DPS, Heal)
        GuildShareFilter roleFilter = ParseRoleFilter(filterName);
        uint8 playerClass = 0;

        if (roleFilter == GuildShareFilter::FILTER_CLASS)
        {
            playerClass = ParseClassName(filterName);
            if (!playerClass)
                break;
        }

        std::string itemsSection = TrimWS(line.substr(firstSpace + 1));
        if (itemsSection.empty())
            continue;

        std::istringstream itemStream(itemsSection);
        std::string itemEntry;

        while (std::getline(itemStream, itemEntry, ','))
        {
            itemEntry = TrimWS(itemEntry);
            if (itemEntry.empty())
                continue;

            auto lastSpace = itemEntry.rfind(' ');
            if (lastSpace == std::string::npos)
                continue;

            std::string amountStr = TrimWS(itemEntry.substr(lastSpace + 1));
            bool isNumber = !amountStr.empty() && std::all_of(amountStr.begin(), amountStr.end(), ::isdigit);
            if (!isNumber)
                continue;

            uint32 amount = std::stoul(amountStr);
            std::string itemName = TrimWS(itemEntry.substr(0, lastSpace));
            if (itemName.empty() || !amount)
                continue;

            uint32 itemId = GuildOrderValue::FindItemByName(itemName);
            if (!itemId)
            {
                sLog.outDetail("GuildShareList: item not found: '%s'", itemName.c_str());
                continue;
            }

            GuildShareItemEntry entry;
            entry.filter = roleFilter;
            entry.playerClass = playerClass;
            entry.itemId = itemId;
            entry.amount = amount;
            result.push_back(entry);
        }
    }

    return result;
}

GuildOrder GuildShareCraftOrderValue::Calculate()
{
    GuildOrder order;

    if (!bot->GetGuildId())
        return order;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return order;

    std::set<uint32> finishedItemIds;
    for (const auto& entry : shareList)
        finishedItemIds.insert(entry.itemId);

    struct CraftCandidate
    {
        uint32 itemId;
        uint32 needed;
        std::string name;
    };

    std::vector<CraftCandidate> candidates;

    for (uint32 itemId : finishedItemIds)
    {
        uint32 guildDeficit = CountGuildFinishedItemDeficit(bot, itemId, shareList);
        if (guildDeficit == 0)
            continue;

        uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
        if (currentCount >= guildDeficit)
            continue;

        uint32 needed = guildDeficit - currentCount;

        uint32 craftSpellId = 0;
        for (auto& [spellId, spellState] : bot->GetSpellMap())
        {
            if (spellState.state == PLAYERSPELL_REMOVED || spellState.disabled || IsPassiveSpell(spellId))
                continue;

            const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
            if (!pSpellInfo)
                continue;

            for (int i = 0; i < 3; ++i)
            {
                if (pSpellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM && pSpellInfo->EffectItemType[i] == itemId)
                {
                    craftSpellId = spellId;
                    break;
                }
            }

            if (craftSpellId)
                break;
        }

        if (!craftSpellId)
            continue;

        if (!ai->HasCheat(BotCheatMask::item) && !AI_VALUE2(bool, "can craft spell", craftSpellId))
            continue;

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        candidates.push_back({ itemId, needed, proto->Name1 });
    }

    if (candidates.empty())
        return order;

    const CraftCandidate& chosen = candidates[urand(0, candidates.size() - 1)];

    order.type = GuildOrderType::Craft;
    order.target = chosen.name;
    order.amount = chosen.needed;
    return order;
}

GuildOrder GuildShareFarmOrderValue::Calculate()
{
    GuildOrder order;

    if (!bot->GetGuildId())
        return order;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return order;

    bool hasHerbalism = ai->HasSkill(SKILL_HERBALISM);
    bool hasMining = ai->HasSkill(SKILL_MINING);
    bool hasSkinning = ai->HasSkill(SKILL_SKINNING);
    bool hasAlchemy = ai->HasSkill(SKILL_ALCHEMY);
    bool hasBlacksmithing = ai->HasSkill(SKILL_BLACKSMITHING);
    bool hasEngineering = ai->HasSkill(SKILL_ENGINEERING);
    bool hasLeatherworking = ai->HasSkill(SKILL_LEATHERWORKING);
    bool hasTailoring = ai->HasSkill(SKILL_TAILORING);
    bool hasEnchanting = ai->HasSkill(SKILL_ENCHANTING);
    bool hasCooking = ai->HasSkill(SKILL_COOKING);
    bool hasFirstAid = ai->HasSkill(SKILL_FIRST_AID);
    bool hasAnyGathering = hasHerbalism || hasMining || hasSkinning;

    bool hasAnyGatherOrCraft = hasAnyGathering ||
        hasAlchemy || hasBlacksmithing || hasEngineering || hasLeatherworking ||
        hasTailoring || hasEnchanting || hasCooking || hasFirstAid;

    if (!hasAnyGatherOrCraft)
        return order;

    std::set<uint32> finishedItemIds;
    for (const auto& entry : shareList)
        finishedItemIds.insert(entry.itemId);

    std::map<uint32, uint32> reagentNeeds;

    for (uint32 itemId : finishedItemIds)
    {
        uint32 guildDeficit = CountGuildFinishedItemDeficit(bot, itemId, shareList);
        if (guildDeficit == 0)
            continue;

        uint32 currentFinished = ai->GetInventoryItemsCountWithId(itemId);
        uint32 remaining = (currentFinished >= guildDeficit) ? 0 : guildDeficit - currentFinished;
        if (remaining == 0)
            continue;

        ItemPrototype const* shareProto = sObjectMgr.GetItemPrototype(itemId);
        if (!shareProto)
            continue;

        std::vector<std::pair<uint32, uint32>> reagents = ItemUsageValue::GetAllReagentItemIdsForCraftingItem(shareProto);

        if (reagents.empty())
        {
            reagentNeeds[itemId] += remaining;
            continue;
        }

        for (const auto& [reagentId, reagentCount] : reagents)
        {
            reagentNeeds[reagentId] += reagentCount * remaining;
        }
    }

    if (reagentNeeds.empty())
        return order;

    // Now find a reagent this bot can farm and still needs more of.
    // Priority: 0 = gathering nodes, the bot has relevant gathering skill
    //           1 = nodes and mobs, bot has relevant craft skill
    //           2 = mobs only, bot has relevant craft skill
    //           3 = mobs only, any bot can farm
    struct FarmCandidate
    {
        uint32 itemId;
        uint32 needed;
        uint32 priority;
    };

    std::vector<FarmCandidate> candidates;

    for (const auto& [reagentId, totalNeeded] : reagentNeeds)
    {
        ItemPrototype const* reagentProto = sObjectMgr.GetItemPrototype(reagentId);
        if (!reagentProto)
            continue;

        uint32 currentCount = ai->GetInventoryItemsCountWithId(reagentId);
        if (currentCount >= totalNeeded)
            continue;

        uint32 remaining = totalNeeded - currentCount;

        std::list<int32> dropEntries = GAI_VALUE2(std::list<int32>, "item drop list", reagentId);

        bool dropsFromGatherNode = false;
        bool dropsFromMob = false;
        for (int32 entry : dropEntries)
        {
            if (entry < 0)
                dropsFromGatherNode = true;
            else
                dropsFromMob = true;
        }

        // Determine priority based on drop source and bot's skills
        uint32 priority = 4; // default: cannot farm at all

        if (dropsFromGatherNode && hasAnyGathering)
        {
            // Best priority: item comes from herb/ore/skin nodes and bot has a gathering skill
            priority = 0;
        }
        else if (dropsFromMob)
        {
            bool hasRelevantCraftSkill = false;
            if (hasAlchemy && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_ALCHEMY))
                hasRelevantCraftSkill = true;
            else if (hasBlacksmithing && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_BLACKSMITHING))
                hasRelevantCraftSkill = true;
            else if (hasEngineering && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_ENGINEERING))
                hasRelevantCraftSkill = true;
            else if (hasLeatherworking && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_LEATHERWORKING))
                hasRelevantCraftSkill = true;
            else if (hasTailoring && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_TAILORING))
                hasRelevantCraftSkill = true;
            else if (hasEnchanting && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_ENCHANTING))
                hasRelevantCraftSkill = true;
            else if (hasCooking && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_COOKING))
                hasRelevantCraftSkill = true;
            else if (hasFirstAid && ItemUsageValue::IsItemUsedBySkill(reagentProto, SKILL_FIRST_AID))
                hasRelevantCraftSkill = true;

            if (dropsFromGatherNode)
            {
                priority = hasRelevantCraftSkill ? 1 : 2;
            }
            else
            {
                priority = hasRelevantCraftSkill ? 2 : 3;
            }
        }
        else if (dropsFromGatherNode && !hasAnyGathering)
        {
            priority = 3;
        }

        if (priority > 3)
            continue;

        candidates.push_back({ reagentId, remaining, priority });
    }

    if (candidates.empty())
        return order;

    // Calculate nodes > mobs if possible.
    uint32 bestPriority = candidates.front().priority;
    for (const auto& c : candidates)
    {
        if (c.priority < bestPriority)
            bestPriority = c.priority;
    }

    std::vector<FarmCandidate> bestCandidates;
    for (const auto& c : candidates)
    {
        if (c.priority == bestPriority)
            bestCandidates.push_back(c);
    }

    // Pick random valid item to farm
    const FarmCandidate& chosen = bestCandidates[urand(0, bestCandidates.size() - 1)];
    ItemPrototype const* bestProto = sObjectMgr.GetItemPrototype(chosen.itemId);
    if (!bestProto)
        return order;

    order.type = GuildOrderType::Farm;
    order.target = bestProto->Name1;
    order.amount = chosen.needed;

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

    std::vector<Item*> botItems = ai->GetInventoryItems();
    if (botItems.empty())
        return result;

    std::map<uint32, Item*> sharableItems;
    for (Item* item : botItems)
    {
        uint32 itemId = item->GetProto()->ItemId;

        if (sharableItems.count(itemId))
            continue;

        sharableItems[itemId] = item;
    }

    if (sharableItems.empty())
        return result;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");

    std::map<uint32, uint32> selfNeeded;
    for (const auto& entry : shareList)
    {
        if (entry.MatchesPlayer(bot))
        {
            if (entry.amount > selfNeeded[entry.itemId])
                selfNeeded[entry.itemId] = entry.amount;
        }
    }

    // Check nearby guild members
    std::list<ObjectGuid> nearGuids = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("nearest friendly players")->Get();

    for (auto& guid : nearGuids)
    {
        Player* player = sObjectMgr.GetPlayer(guid);
        if (!player || player == bot)
            continue;

        if (player->GetGuildId() != bot->GetGuildId())
            continue;

        PlayerbotAI* targetAi = player->GetPlayerbotAI();
        if (!targetAi)
            continue;

        if (sServerFacade.GetDistance2d(bot, player) > INTERACTION_DISTANCE)
            continue;

        // Only share items explicitly listed in the guild info "Share:" section
        for (const auto& entry : shareList)
        {
            if (!entry.MatchesPlayer(player))
                continue;

            auto it = sharableItems.find(entry.itemId);
            if (it == sharableItems.end())
                continue;

            uint32 targetCount = targetAi->GetInventoryItemsCountWithId(entry.itemId);
            if (targetCount >= entry.amount)
                continue;

            uint32 botCount = ai->GetInventoryItemsCountWithId(entry.itemId);
            uint32 botNeeds = selfNeeded.count(entry.itemId) ? selfNeeded[entry.itemId] : 0;
            if (botCount <= botNeeds)
                continue; // Bot doesn't have surplus beyond its own needs

            uint32 needed = entry.amount - targetCount;

            result.receiver = player;
            result.itemId = entry.itemId;
            result.amount = needed;
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
