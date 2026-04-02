
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
    static std::unordered_map<std::string, uint32> s_cache;

    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), [](unsigned char c) { return std::tolower(c); });

    auto it = s_cache.find(lowerName);
    if (it != s_cache.end())
        return it->second;

    uint32 substringMatch = 0;

    for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
    {
        ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(itemId);
        if (!proto)
            continue;

        if (proto->Class == ITEM_CLASS_RECIPE ||
            proto->Class == ITEM_CLASS_WEAPON ||
            proto->Class == ITEM_CLASS_ARMOR)
            continue;

        if (name.size() == strlen(proto->Name1) && strstri(proto->Name1, name.c_str()))
        {
            s_cache[lowerName] = itemId;
            return itemId;
        }

        if (!substringMatch && strstri(proto->Name1, name.c_str()))
            substringMatch = itemId;
    }

    s_cache[lowerName] = substringMatch;
    return substringMatch;
}

std::vector<std::pair<uint32, int8>> ai::FindRepeatableQuestsRewardingItem(uint32 itemId)
{
    static std::unordered_map<uint32, std::vector<std::pair<uint32, int8>>> s_cache;

    auto cacheIt = s_cache.find(itemId);
    if (cacheIt != s_cache.end())
        return cacheIt->second;

    std::vector<std::pair<uint32, int8>> result;

    ObjectMgr::QuestMap const& questMap = sObjectMgr.GetQuestTemplates();
    for (auto& [questId, questPtr] : questMap)
    {
        Quest* quest = questPtr.get();
        if (!quest || !quest->IsActive())
            continue;

        if (!quest->IsRepeatable())
            continue;

        for (uint8 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
        {
            if (quest->RewChoiceItemId[i] == itemId)
            {
                result.push_back({ questId, (int8)i });
                break;
            }
        }

        for (uint8 i = 0; i < quest->GetRewItemsCount(); ++i)
        {
            if (quest->RewItemId[i] == itemId)
            {
                result.push_back({ questId, -1 });
                break;
            }
        }
    }

    s_cache[itemId] = result;
    return result;
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
            : itemId(id), shareList(list), deficit(0) {
        }

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

static std::unordered_map<uint32, uint32> CountGuildFinishedItemDeficits(
    Player* bot,
    const std::set<uint32>& itemIds,
    const std::vector<GuildShareItemEntry>& shareList)
{
    std::unordered_map<uint32, uint32> deficits;

    if (!bot->GetGuildId())
        return deficits;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return deficits;

    struct CountAllDeficits
    {
        const std::set<uint32>& itemIds;
        const std::vector<GuildShareItemEntry>& shareList;
        std::unordered_map<uint32, uint32>& deficits;

        CountAllDeficits(const std::set<uint32>& ids, const std::vector<GuildShareItemEntry>& list,
            std::unordered_map<uint32, uint32>& out)
            : itemIds(ids), shareList(list), deficits(out) {
        }

        void operator()(Player* player)
        {
            if (!player)
                return;

            for (const auto& entry : shareList)
            {
                if (itemIds.find(entry.itemId) == itemIds.end())
                    continue;

                if (!entry.MatchesPlayer(player))
                    continue;

                uint32 has = 0;
                PlayerbotAI* memberAi = player->GetPlayerbotAI();
                if (memberAi)
                    has = memberAi->GetInventoryItemsCountWithId(entry.itemId);
                else
                    has = player->GetItemCount(entry.itemId, true);

                if (has < entry.amount)
                    deficits[entry.itemId] += entry.amount - has;
            }
        }
    };

    CountAllDeficits worker(itemIds, shareList, deficits);
    guild->BroadcastWorker(worker);

    return deficits;
}

static std::unordered_map<uint32, uint32> BuildCraftSpellMap(Player* bot, const std::set<uint32>& itemIds)
{
    std::unordered_map<uint32, uint32> craftMap;
    std::set<uint32> remaining = itemIds;

    for (auto& [spellId, spellState] : bot->GetSpellMap())
    {
        if (remaining.empty())
            break;

        if (spellState.state == PLAYERSPELL_REMOVED || spellState.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (pSpellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM)
            {
                uint32 createdItem = pSpellInfo->EffectItemType[i];
                if (remaining.count(createdItem))
                {
                    craftMap[createdItem] = spellId;
                    remaining.erase(createdItem);
                }
            }
        }
    }

    return craftMap;
}

static std::unordered_map<uint32, uint32> ComputeRemainingNeeds(
    PlayerbotAI* ai,
    const std::set<uint32>& itemIds,
    const std::unordered_map<uint32, uint32>& deficits)
{
    std::unordered_map<uint32, uint32> remaining;
    for (uint32 itemId : itemIds)
    {
        auto deficitIt = deficits.find(itemId);
        uint32 guildDeficit = (deficitIt != deficits.end()) ? deficitIt->second : 0;
        if (guildDeficit == 0)
            continue;

        uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
        if (currentCount >= guildDeficit)
            continue;

        remaining[itemId] = guildDeficit - currentCount;
    }
    return remaining;
}

static bool HasSkipOrderNote(Player* bot)
{
    if (!bot->GetGuildId())
        return false;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    MemberSlot* member = guild->GetMemberSlot(bot->GetObjectGuid());
    if (!member)
        return false;

    std::string note = member->OFFnote;
    if (note.empty())
        return false;

    std::string lower = note;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });

    lower.erase(lower.begin(), std::find_if(lower.begin(), lower.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    lower.erase(std::find_if(lower.rbegin(), lower.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), lower.end());

    return lower == "skip order";
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

    if (!note.empty())
    {
        std::string lower = note;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
        std::string trimmed = TrimWhitespace(lower);
        if (trimmed == "skip order")
            return order;
    }

    if (note.empty())
    {
        GuildOrder craftOrder = AI_VALUE(GuildOrder, "guild share craft order");
        if (craftOrder.IsValid())
            return craftOrder;

        GuildOrder questRewardOrder = AI_VALUE(GuildOrder, "guild share quest reward order");
        if (questRewardOrder.IsValid())
            return questRewardOrder;

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

        GuildOrder questRewardOrder = AI_VALUE(GuildOrder, "guild share quest reward order");
        if (questRewardOrder.IsValid())
            return questRewardOrder;

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
        if (!ai)
            return true;
        return !ai->IsRanged(player, false);
    }

    case GuildShareFilter::FILTER_RANGED:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        if (!ai)
            return true;
        return ai->IsRanged(player, false);
    }

    case GuildShareFilter::FILTER_TANK:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        if (!ai)
            return true;
        return ai->IsTank(player, false);
    }

    case GuildShareFilter::FILTER_DPS:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        if (!ai)
            return true;
        return !ai->IsTank(player, false) && !ai->IsHeal(player, false);
    }

    case GuildShareFilter::FILTER_HEAL:
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();
        if (!ai)
            return true;
        return ai->IsHeal(player, false);
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

    if (HasSkipOrderNote(bot))
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

    std::unordered_map<uint32, uint32> craftSpells = BuildCraftSpellMap(bot, finishedItemIds);
    if (craftSpells.empty())
        return order;

    std::set<uint32> craftableIds;
    for (const auto& [itemId, spellId] : craftSpells)
        craftableIds.insert(itemId);

    std::unordered_map<uint32, uint32> deficits = CountGuildFinishedItemDeficits(bot, craftableIds, shareList);
    std::unordered_map<uint32, uint32> remaining = ComputeRemainingNeeds(ai, craftableIds, deficits);

    struct CraftCandidate
    {
        uint32 itemId;
        uint32 needed;
        std::string name;
    };

    std::vector<CraftCandidate> candidates;

    for (const auto& [itemId, needed] : remaining)
    {
        auto craftIt = craftSpells.find(itemId);
        if (craftIt == craftSpells.end())
            continue;

        uint32 craftSpellId = craftIt->second;

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

    std::unordered_map<uint32, uint32> deficits = CountGuildFinishedItemDeficits(bot, finishedItemIds, shareList);
    std::unordered_map<uint32, uint32> itemRemaining = ComputeRemainingNeeds(ai, finishedItemIds, deficits);

    std::map<uint32, uint32> reagentNeeds;

    for (const auto& [itemId, remaining] : itemRemaining)
    {
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

    // 20% chance to move to auctioneer instead of farming
    if (urand(1, 100) <= 20 && bot->GetMoney() >= 100)
    {
        std::vector<uint32> neededItemIds;
        for (const auto& [reagentId, totalNeeded] : reagentNeeds)
        {
            uint32 currentCount = ai->GetInventoryItemsCountWithId(reagentId);
            if (currentCount < totalNeeded)
                neededItemIds.push_back(reagentId);
        }

        if (!neededItemIds.empty())
        {
            uint32 chosenId = neededItemIds[urand(0, neededItemIds.size() - 1)];
            ItemPrototype const* chosenProto = sObjectMgr.GetItemPrototype(chosenId);
            if (chosenProto)
            {
                order.type = GuildOrderType::AuctionHouse;
                order.target = chosenProto->Name1;
                order.amount = reagentNeeds[chosenId] - ai->GetInventoryItemsCountWithId(chosenId);
                return order;
            }
        }
    }

    struct FarmCandidate
    {
        uint32 itemId;
        uint32 needed;
        uint32 priority;
    };

    std::vector<FarmCandidate> candidates;
    uint32 bestPriority = 4;

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

            if (dropsFromGatherNode && dropsFromMob)
                break;
        }

        uint32 priority = 4;

        if (dropsFromGatherNode && hasAnyGathering)
        {
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

        if (priority < bestPriority)
            bestPriority = priority;

        candidates.push_back({ reagentId, remaining, priority });
    }

    if (candidates.empty())
        return order;

    std::vector<FarmCandidate> bestCandidates;
    for (const auto& c : candidates)
    {
        if (c.priority == bestPriority)
            bestCandidates.push_back(c);
    }

    const FarmCandidate& chosen = bestCandidates[urand(0, bestCandidates.size() - 1)];
    ItemPrototype const* bestProto = sObjectMgr.GetItemPrototype(chosen.itemId);
    if (!bestProto)
        return order;

    order.type = GuildOrderType::Farm;
    order.target = bestProto->Name1;
    order.amount = chosen.needed;

    return order;
}

GuildOrder GuildShareQuestRewardOrderValue::Calculate()
{
    GuildOrder order;

    if (!bot->GetGuildId())
        return order;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return order;

    struct QuestRewardCandidate
    {
        uint32 itemId;
        uint32 questId;
        int8 rewardChoiceIndex;
        uint32 needed;
        std::string questTitle;
    };

    std::vector<QuestRewardCandidate> candidates;

    for (const auto& entry : shareList)
    {
        if (!entry.MatchesPlayer(bot))
            continue;

        uint32 currentCount = ai->GetInventoryItemsCountWithId(entry.itemId);
        if (currentCount >= entry.amount)
            continue;

        uint32 needed = entry.amount - currentCount;

        auto questRewards = FindRepeatableQuestsRewardingItem(entry.itemId);
        if (questRewards.empty())
            continue;

        std::list<int32> dropEntries = GAI_VALUE2(std::list<int32>, "item drop list", entry.itemId);
        if (!dropEntries.empty())
            continue;

        std::set<uint32> singleItemSet = { entry.itemId };
        std::unordered_map<uint32, uint32> craftSpells = BuildCraftSpellMap(bot, singleItemSet);
        if (!craftSpells.empty())
            continue;

        for (const auto& [questId, rewardIdx] : questRewards)
        {
            Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
            if (!quest)
                continue;

            if (!bot->CanTakeQuest(quest, false) &&
                bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE &&
                bot->GetQuestStatus(questId) != QUEST_STATUS_COMPLETE)
                continue;

            candidates.push_back({ entry.itemId, questId, rewardIdx, needed, quest->GetTitle() });
        }
    }

    if (candidates.empty())
        return order;

    const QuestRewardCandidate& chosen = candidates[urand(0, candidates.size() - 1)];

    order.type = GuildOrderType::QuestReward;
    order.target = chosen.questTitle;
    order.amount = chosen.needed;
    order.questId = chosen.questId;
    order.rewardItemId = chosen.itemId;

    return order;
}

uint32 GuildShareQuestRewardItemValue::Calculate()
{
    if (!bot->GetGuildId())
        return 0;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return 0;

    std::unordered_map<uint32, uint32> neededItems;
    for (const auto& entry : shareList)
    {
        if (!entry.MatchesPlayer(bot))
            continue;

        uint32 currentCount = ai->GetInventoryItemsCountWithId(entry.itemId);
        if (currentCount < entry.amount)
            neededItems[entry.itemId] = entry.amount - currentCount;
    }

    if (neededItems.empty())
        return 0;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status != QUEST_STATUS_COMPLETE)
            continue;

        Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
        if (!quest)
            continue;

        for (uint8 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
        {
            uint32 rewItemId = quest->RewChoiceItemId[i];
            if (neededItems.find(rewItemId) != neededItems.end())
                return rewItemId;
        }

        for (uint8 i = 0; i < quest->GetRewItemsCount(); ++i)
        {
            uint32 rewItemId = quest->RewItemId[i];
            if (neededItems.find(rewItemId) != neededItems.end())
                return rewItemId;
        }
    }

    return 0;
}

GuildShareTarget GuildShareTargetValue::Calculate()
{
    GuildShareTarget result;

    if (!bot->GetGuildId())
        return result;

    Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
    if (!guild)
        return result;

    std::vector<GuildShareItemEntry> shareList = AI_VALUE(std::vector<GuildShareItemEntry>, "guild share list");
    if (shareList.empty())
        return result;

    std::unordered_set<uint32> shareItemIds;
    for (const auto& entry : shareList)
        shareItemIds.insert(entry.itemId);

    bool hasAnyShareItem = false;
    for (uint32 itemId : shareItemIds)
    {
        if (ai->GetInventoryItemsCountWithId(itemId) > 0)
        {
            hasAnyShareItem = true;
            break;
        }
    }

    if (!hasAnyShareItem)
        return result;

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

        for (const auto& entry : shareList)
        {
            if (!entry.MatchesPlayer(player))
                continue;

            uint32 botCount = ai->GetInventoryItemsCountWithId(entry.itemId);
            if (botCount == 0)
                continue;

            uint32 targetCount = targetAi->GetInventoryItemsCountWithId(entry.itemId);
            if (targetCount >= entry.amount)
                continue;

            bool botMatchesEntry = entry.MatchesPlayer(bot);
            if (botMatchesEntry)
            {
                if (botCount <= targetCount + 1)
                    continue;
            }

            uint32 needed = entry.amount - targetCount;

            if (botMatchesEntry)
            {
                uint32 evenShare = (botCount + targetCount) / 2;
                uint32 canGive = (botCount > evenShare) ? botCount - evenShare : 0;

                if (canGive == 0)
                    continue;

                if (needed > canGive)
                    needed = canGive;
            }

            result.receiver = player;
            result.itemId = entry.itemId;
            result.amount = needed;
            return result;
        }
    }

    return result;
}

std::vector<uint32> NeedsProfessionReagentsValue::GetMissingReagents(PlayerbotAI* ai)
{
    std::vector<uint32> missing;
    Player* bot = ai->GetBot();

    std::vector<GuildShareItemEntry> shareList = ai->GetAiObjectContext()->GetValue<std::vector<GuildShareItemEntry>>("guild share list")->Get();
    if (shareList.empty())
        return missing;

    std::set<uint32> finishedItemIds;
    for (const auto& entry : shareList)
        finishedItemIds.insert(entry.itemId);

    std::unordered_map<uint32, uint32> craftSpells = BuildCraftSpellMap(bot, finishedItemIds);
    if (craftSpells.empty())
        return missing;

    std::set<uint32> craftableItemIds;
    for (const auto& [itemId, spellId] : craftSpells)
        craftableItemIds.insert(itemId);

    if (craftableItemIds.empty())
        return missing;

    std::unordered_map<uint32, uint32> deficits = CountGuildFinishedItemDeficits(bot, craftableItemIds, shareList);
    std::unordered_map<uint32, uint32> remainingMap = ComputeRemainingNeeds(ai, craftableItemIds, deficits);

    if (remainingMap.empty())
        return missing;

    GuildOrder order = ai->GetAiObjectContext()->GetValue<GuildOrder>("guild order")->Get();
    std::set<uint32> itemsToCheck;

    if (order.IsValid())
    {
        uint32 orderItemId = GuildOrderValue::FindItemByName(order.target);

        if (order.IsCraftOrder())
        {
            if (orderItemId && remainingMap.count(orderItemId) && craftSpells.count(orderItemId))
                itemsToCheck.insert(orderItemId);
        }
        else if (order.IsTravelOrder())
        {
            if (orderItemId)
            {
                for (const auto& [itemId, needed] : remainingMap)
                {
                    auto craftIt = craftSpells.find(itemId);
                    if (craftIt == craftSpells.end())
                        continue;

                    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(craftIt->second);
                    if (!pSpellInfo)
                        continue;

                    bool usesFarmedItem = false;
                    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
                    {
                        if (pSpellInfo->Reagent[i] == static_cast<int32>(orderItemId))
                        {
                            usesFarmedItem = true;
                            break;
                        }
                    }

                    if (!usesFarmedItem)
                    {
                        ItemPrototype const* craftProto = sObjectMgr.GetItemPrototype(itemId);
                        if (craftProto)
                        {
                            auto reagents = ItemUsageValue::GetAllReagentItemIdsForCraftingItem(craftProto);
                            for (const auto& [reagentId, count] : reagents)
                            {
                                if (reagentId == orderItemId)
                                {
                                    usesFarmedItem = true;
                                    break;
                                }
                            }
                        }
                    }

                    if (usesFarmedItem)
                        itemsToCheck.insert(itemId);
                }
            }
        }
    }

    if (itemsToCheck.empty())
    {
        for (const auto& [itemId, needed] : remainingMap)
        {
            if (craftSpells.count(itemId))
                itemsToCheck.insert(itemId);
        }
    }

    std::unordered_set<uint32> seen;
    for (uint32 itemId : itemsToCheck)
    {
        auto craftIt = craftSpells.find(itemId);
        if (craftIt == craftSpells.end())
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(craftIt->second);
        if (!pSpellInfo)
            continue;

        for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
        {
            if (!pSpellInfo->ReagentCount[i] || !pSpellInfo->Reagent[i])
                continue;

            uint32 reagentId = pSpellInfo->Reagent[i];

            if (!seen.insert(reagentId).second)
                continue;

            const ItemPrototype* reagentProto = sObjectMgr.GetItemPrototype(reagentId);
            if (!reagentProto)
                continue;

            if (!ItemUsageValue::IsItemSoldByAnyVendor(reagentProto))
                continue;

            if (ItemUsageValue::IsItemSoldByAnyVendorButHasLimitedMaxCount(reagentProto))
                continue;

            uint32 currentCount = ai->GetInventoryItemsCountWithId(reagentId);
            if (currentCount > 0)
                continue;

            missing.push_back(reagentId);
        }
    }

    return missing;
}

bool NeedsProfessionReagentsValue::Calculate()
{
    bool hasAnyCraftingSkill =
        ai->HasSkill(SKILL_ALCHEMY) || ai->HasSkill(SKILL_BLACKSMITHING) ||
        ai->HasSkill(SKILL_ENGINEERING) || ai->HasSkill(SKILL_LEATHERWORKING) ||
        ai->HasSkill(SKILL_TAILORING) || ai->HasSkill(SKILL_ENCHANTING) ||
        ai->HasSkill(SKILL_COOKING) || ai->HasSkill(SKILL_FIRST_AID);

    if (!hasAnyCraftingSkill)
        return false;

    GuildOrder order = AI_VALUE(GuildOrder, "guild order");
    if (!order.IsValid())
        return false;

    return !GetMissingReagents(ai).empty();
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
