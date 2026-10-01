#include "CraftValues.h"
#include "ItemUsageValue.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/PlayerbotAI.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "playerbot/strategy/values/LootValues.h"
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"
#include "playerbot/strategy/values/SharedValueContext.h"
#include "Grids/GridNotifiers.h"

using namespace ai;

std::vector<uint32> CraftSpellsValue::Calculate()
{
    std::vector<uint32> spellIds;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second.state == PLAYERSPELL_REMOVED || spell.second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

#ifdef MANGOSBOT_TWO
        if (pSpellInfo->Effect[0] == SPELL_EFFECT_CREATE_ITEM_2 && spellId == 61288 && bot->IsSpellReady(61288)) //Todo handle other item_2 spells
        {
            spellIds.push_back(spellId);
            continue;
        }
#endif

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_CREATE_ITEM)
            continue;      

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
        {
            if (pSpellInfo->EffectItemType[i])
            {
                spellIds.push_back(spellId);
                break;
            }
        }
    }

    return spellIds;
}

CraftToolRequirements CraftToolRequirementsValue::Calculate()
{
    CraftToolRequirements requirements;
    std::vector<uint32> recipes = AI_VALUE(std::vector<uint32>, "craft spells");
    std::vector<uint32> enchants = AI_VALUE(std::vector<uint32>, "enchant spells");
    recipes.insert(recipes.end(), enchants.begin(), enchants.end());
    for (uint32 spellId : recipes)
    {
        SpellEntry const* spell = sServerFacade.LookupSpellInfo(spellId);
        if (!spell)
            continue;
        for (uint32 tool : spell->Totem)
            if (tool)
                requirements.items.insert(tool);
#ifndef MANGOSBOT_ZERO
        for (uint32 category : spell->TotemCategory)
            if (category)
                requirements.categories.insert(category);
#endif
    }
    return requirements;
}

bool CraftToolRequirements::UsesItem(const ItemPrototype* item) const
{
    if (!item)
        return false;
    if (items.count(item->ItemId))
        return true;
#ifndef MANGOSBOT_ZERO
    for (uint32 category : categories)
        if (item->TotemCategory && IsTotemCategoryCompatiableWith(item->TotemCategory, category))
            return true;
#endif
    return false;
}

bool CraftToolRequirements::NeedsItem(const ItemPrototype* item, Player* player) const
{
    if (!item || !player || player->HasItemCount(item->ItemId, 1))
        return false;
    if (items.count(item->ItemId))
        return true;
#ifndef MANGOSBOT_ZERO
    for (uint32 category : categories)
        if (!player->HasItemTotemCategory(category) && item->TotemCategory &&
            IsTotemCategoryCompatiableWith(item->TotemCategory, category))
            return true;
#endif
    return false;
}

CraftToolItemMap* CraftToolItemsValue::Calculate()
{
    CraftToolItemMap* result = new CraftToolItemMap;
#ifndef MANGOSBOT_ZERO
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); ++id)
    {
        ItemPrototype const* item = sObjectMgr.GetItemPrototype(id);
        if (item && item->TotemCategory)
            (*result)[item->TotemCategory].push_back(id);
    }
#endif
    return result;
}

std::vector<uint32> EnchantSpellsValue::Calculate()
{
    std::vector<uint32> spellIds;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second.state == PLAYERSPELL_REMOVED || spell.second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_ENCHANT_ITEM || !pSpellInfo->ReagentCount[0])
            continue;


        spellIds.push_back(spellId);
    }

    return spellIds;
}

uint32 HasReagentsForValue::Calculate()
{
    if (ai->HasCheat(BotCheatMask::item))
        return true;

    uint32 spellId = stoi(getQualifier());

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

    if (!pSpellInfo)
        return false;

    uint32 craftCount = 9999;

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
    {
        if (pSpellInfo->ReagentCount[i] > 0 && pSpellInfo->Reagent[i])
        {
            const ItemPrototype* reqProto = sObjectMgr.GetItemPrototype(pSpellInfo->Reagent[i]);

            uint32 count = AI_VALUE2(uint32, "item count", ChatHelper::formatItem(reqProto));

            if (count < pSpellInfo->ReagentCount[i])
                return 0;

            if (craftCount > count / pSpellInfo->ReagentCount[i])
                craftCount = count / pSpellInfo->ReagentCount[i];
        }
    }

    return craftCount;
}

bool CanCraftSpellValue::Calculate()
{
    uint32 spellId = stoi(getQualifier());

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

    if (!pSpellInfo)
        return false;

    if (AI_VALUE2(uint32, "has reagents for", spellId) == 0)
        return false;

    return HasRequiredTools(pSpellInfo, bot);
}

bool CanCraftSpellValue::HasRequiredTools(const SpellEntry* spell, Player* player)
{
    if (!spell || !player)
        return false;
    for (uint32 tool : spell->Totem)
        if (tool && !player->HasItemCount(tool, 1))
            return false;
#ifndef MANGOSBOT_ZERO
    for (uint32 category : spell->TotemCategory)
        if (category && !player->HasItemTotemCategory(category))
            return false;
#endif

    return true;
}

bool ShouldCraftSpellValue::Calculate()
{
    uint32 spellId = stoi(getQualifier());

    if (SpellGivesSkillUp(spellId, bot))
        return true;

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!pSpellInfo)
        return false;

#ifdef MANGOSBOT_TWO
    if (pSpellInfo->Effect[0] == SPELL_EFFECT_CREATE_ITEM_2) //todo proper check what items are created and if they are needed.
        return true;
#endif

    for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
    {
        if (pSpellInfo->EffectItemType[i])
        {
            ItemUsage usage = AI_VALUE2_LAZY(ItemUsage, "item usage", std::to_string(pSpellInfo->EffectItemType[i]));

            bool needItem = false;

            switch (usage)
            {
                case ItemUsage::ITEM_USAGE_EQUIP:
                case ItemUsage::ITEM_USAGE_BAD_EQUIP:
                case ItemUsage::ITEM_USAGE_QUEST:
                case ItemUsage::ITEM_USAGE_USE:
                case ItemUsage::ITEM_USAGE_FORCE_NEED:
                {
                    needItem = true;
                    break;
                }
                case ItemUsage::ITEM_USAGE_SKILL:
                case ItemUsage::ITEM_USAGE_AMMO:
                case ItemUsage::ITEM_USAGE_DISENCHANT:
                case ItemUsage::ITEM_USAGE_AH:
                case ItemUsage::ITEM_USAGE_BROKEN_AH:
                case ItemUsage::ITEM_USAGE_VENDOR:
                case ItemUsage::ITEM_USAGE_FORCE_GREED:
                {
                    needItem = !ai->HasCheat(BotCheatMask::item);
                    break;
                }
                case ItemUsage::ITEM_USAGE_NONE:
                case ItemUsage::ITEM_USAGE_KEEP:
                case ItemUsage::ITEM_USAGE_BROKEN_EQUIP:
                case ItemUsage::ITEM_USAGE_GUILD_TASK:
                {
                    needItem = false;
                    break;
                }
                default:
                {
                    needItem = false;
                    break;
                }
            }

            if (needItem)
                return true;
        }
    }

    return false;
}

inline int SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_GREY) * 10;
    else if (SkillValue >= GreenLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_GREEN) * 10;
    else if (SkillValue >= YellowLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_YELLOW) * 10;

    return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_ORANGE) * 10;
}

bool ShouldCraftSpellValue::SpellGivesSkillUp(uint32 spellId, Player* bot)
{
    SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(spellId);
    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* skill = _spell_idx->second;
        if (skill->skillId)
        {
            uint32 skillValue = bot->GetSkillValuePure(skill->skillId);
            uint32 maxSkillValue = bot->GetSkillMaxPure(skill->skillId);

            if (maxSkillValue <= skillValue)
                continue;

            uint32 craft_skill_gain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_CRAFTING);

            if (SkillGainChance(skillValue,
                skill->max_value,
                (skill->max_value + skill->min_value) / 2,
                skill->min_value) > 0)
                return true;
        }
    }

    return false;
}

ProfessionCraftingPlanValue::ProfessionCraftingPlanValue(PlayerbotAI* ai) :
    CalculatedValue<ProfessionCraftingPlan>(ai, "profession crafting plan", sPlayerbotAIConfig.professionPlanCheckInterval)
{
}

std::map<uint32, uint32> ProfessionCraftingPlan::GetMissingReagents(PlayerbotAI* ai) const
{
    std::map<uint32, uint32> liveMissing;
    if (!ai)
        return liveMissing;

    for (const auto& [itemId, desiredCount] : required)
    {
        uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
        if (currentCount < desiredCount)
            liveMissing[itemId] = desiredCount - currentCount;
    }

    return liveMissing;
}

std::map<uint32, uint32> ProfessionCraftingPlan::GetMissingSupplies(PlayerbotAI* ai) const
{
    std::map<uint32, uint32> supplies = GetMissingReagents(ai);
    for (const auto& tool : ProfessionCraftingPlanValue::GetMissingTools(ai))
        supplies[tool.first] = std::max(supplies[tool.first], tool.second);
    return supplies;
}

namespace
{
    bool HasCashVendorStock(const VendorItemData* stock, uint32 itemId)
    {
        if (!stock)
            return false;
        for (auto item : stock->m_items)
        {
            if (item->item != itemId)
                continue;
            uint32 extendedCost = 0;
#ifndef MANGOSBOT_ZERO
            extendedCost = item->ExtendedCost;
#endif
            // BuyAction::BuyItem attempts the first matching slot and returns,
            // including on failure. A later cash offer cannot rescue that slot.
            return profession::IsCashVendorStock(true, item->maxcount != 0, extendedCost);
        }
        return false;
    }

    bool IsPracticalProfessionVendor(int32 entry, uint32 itemId, Player* bot,
        const PlayerTravelInfo& travelInfo)
    {
        CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(entry);
        if (!info || GuidPosition(HIGHGUID_UNIT, entry).IsHostileTo(bot))
            return false;
        if (!HasCashVendorStock(sObjectMgr.GetNpcVendorItemList(entry), itemId) &&
            (!info->VendorTemplateId ||
                !HasCashVendorStock(sObjectMgr.GetNpcVendorTemplateItemList(info->VendorTemplateId), itemId)))
            return false;

        const DestinationList* destinations = sTravelMgr.GetEntryDestinations(TravelDestinationPurpose::Vendor, entry);
        if (!destinations)
            return false;
        for (TravelDestination* destination : *destinations)
        {
            if (!destination->IsPossible(travelInfo) || destination->DistanceTo(travelInfo.GetPosition()) == FLT_MAX)
                continue;
            for (WorldPosition* point : destination->GetPoints())
                if (point && TravelMgr::IsLocationLevelValid(*point, travelInfo))
                    return true;
        }
        return false;
    }
}

std::set<uint32> ProfessionToolPurchasesValue::Calculate()
{
    std::set<uint32> purchases;
    if (!ProfessionCraftingPlanValue::IsEnabledFor(ai))
        return purchases;
    CraftToolRequirements requirements = AI_VALUE(CraftToolRequirements, "craft tool requirements");
    for (uint32 item : requirements.items)
        if (!bot->HasItemCount(item, 1))
            purchases.insert(item);
#ifndef MANGOSBOT_ZERO
    CraftToolItemMap* toolItems = GAI_VALUE(CraftToolItemMap*, "craft tool items");
    PlayerTravelInfo travelInfo(bot);
    for (uint32 category : requirements.categories)
    {
        if (bot->HasItemTotemCategory(category))
            continue;
        bool alreadyPlanned = false;
        for (uint32 itemId : purchases)
        {
            ItemPrototype const* item = sObjectMgr.GetItemPrototype(itemId);
            if (item && IsTotemCategoryCompatiableWith(item->TotemCategory, category))
                alreadyPlanned = true;
        }
        if (alreadyPlanned)
            continue;

        ItemPrototype const* best = nullptr;
        bool bestHasVendor = false;
        for (const auto& group : *toolItems)
        {
            if (!IsTotemCategoryCompatiableWith(group.first, category))
                continue;
            for (uint32 itemId : group.second)
            {
                ItemPrototype const* item = sObjectMgr.GetItemPrototype(itemId);
                if (!item)
                    continue;
                bool hasVendor = false;
                for (int32 entry : GAI_VALUE2(std::list<int32>, "item vendor list", itemId))
                    if (IsPracticalProfessionVendor(entry, itemId, bot, travelInfo))
                    {
                        hasVendor = true;
                        break;
                    }
                if (!best || (hasVendor && !bestHasVendor) ||
                    (hasVendor == bestHasVendor && (item->BuyPrice < best->BuyPrice ||
                        (item->BuyPrice == best->BuyPrice && itemId < best->ItemId))))
                {
                    best = item;
                    bestHasVendor = hasVendor;
                }
            }
        }
        if (best)
            purchases.insert(best->ItemId);
    }
#endif
    return purchases;
}

std::map<uint32, uint32> ProfessionCraftingPlanValue::GetMissingTools(PlayerbotAI* ai)
{
    std::map<uint32, uint32> missing;
    if (!ai || !IsEnabledFor(ai))
        return missing;
    AiObjectContext* context = ai->GetAiObjectContext();
    CraftToolRequirements requirements = AI_VALUE(CraftToolRequirements, "craft tool requirements");
    for (uint32 itemId : AI_VALUE(std::set<uint32>, "profession tool purchases"))
        if (requirements.NeedsItem(sObjectMgr.GetItemPrototype(itemId), ai->GetBot()))
            missing[itemId] = 1;
    return missing;
}

ProfessionMaterialSources ProfessionCraftingPlan::GetMaterialSources(PlayerbotAI* ai) const
{
    ProfessionMaterialSources sources;
    if (!ai || !IsValid())
        return sources;

    Player* bot = ai->GetBot();
    PlayerTravelInfo travelInfo(bot);
    GatherSourceMap* gatherSourceMap = GAI_VALUE(GatherSourceMap*, "gather source map");
    std::set<int32> vendorEntries;
    std::map<uint32, std::set<int32>> gatherEntries;

    for (const auto& missingReagent : GetMissingSupplies(ai))
    {
        uint32 itemId = missingReagent.first;
        bool hasPracticalGatherSource = false;
        auto gatherRange = gatherSourceMap->equal_range(itemId);
        for (auto itr = gatherRange.first; itr != gatherRange.second; ++itr)
        {
            GatherSource const& source = itr->second;
            if (!bot->HasSkill(source.skillId))
                continue;

            TravelDestinationPurpose purpose = TravelDestinationPurpose::None;
            switch (source.skillId)
            {
                case SKILL_MINING: purpose = TravelDestinationPurpose::GatherMining; break;
                case SKILL_HERBALISM: purpose = TravelDestinationPurpose::GatherHerbalism; break;
                case SKILL_SKINNING: purpose = TravelDestinationPurpose::GatherSkinning; break;
                case SKILL_FISHING: purpose = TravelDestinationPurpose::GatherFishing; break;
                default: continue;
            }

            GatherTravelDestination destination(purpose, 0, source.entry);
            if (!destination.IsPossible(travelInfo))
                continue;

            gatherEntries[static_cast<uint32>(purpose)].insert(source.entry);
            hasPracticalGatherSource = true;
        }

        if (hasPracticalGatherSource)
            continue;

        std::list<int32> itemVendors = GAI_VALUE2(std::list<int32>, "item vendor list", itemId);
        bool hasPracticalVendor = false;
        for (int32 entry : itemVendors)
            if (IsPracticalProfessionVendor(entry, itemId, bot, travelInfo))
            {
                vendorEntries.insert(entry);
                hasPracticalVendor = true;
            }
        if (hasPracticalVendor)
            continue;

        // Deliberate dropped-material farming is intentionally not inferred
        // here: arbitrary creature loot would require expensive probability
        // and level filtering. Normal loot remains active; AH is the targeted
        // fallback for non-gatherable, non-vendor reagents.
        sources.auctionItems.push_back(itemId);
    }

    for (const auto& [purpose, entries] : gatherEntries)
        sources.gatherEntries[purpose] = std::vector<int32>(entries.begin(), entries.end());
    sources.vendorEntries.assign(vendorEntries.begin(), vendorEntries.end());
    return sources;
}

bool ProfessionCraftingPlanValue::IsEnabledFor(PlayerbotAI* ai)
{
    if (!sPlayerbotAIConfig.professionProgressionEnabled || !sPlayerbotAIConfig.professionProgressionCanaryPercent)
        return false;

    Player* bot = ai->GetBot();
    if (!bot || ai->HasActivePlayerMaster() || !sRandomPlayerbotMgr.IsFreeBot(bot))
        return false;

    // Stable assignment keeps the canary cohort unchanged across restarts.
    return profession::IsInCanary(bot->GetGUIDLow(), sPlayerbotAIConfig.professionProgressionCanaryPercent);
}

bool ProfessionCraftingPlanValue::IsCraftCooldownReady(PlayerbotAI* ai)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    uint32 now = static_cast<uint32>(time(nullptr));
    uint32 lastCraft = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "last profession craft")));
    return profession::IsCooldownReady(now, lastCraft, sPlayerbotAIConfig.professionCraftCooldown);
}

bool ProfessionCraftingPlanValue::HasPendingCraft(PlayerbotAI* ai)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    uint32 pending = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "pending profession craft")));
    if (!pending)
        return false;

    uint32 queuedAt = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "profession craft queued at")));
    uint32 timeout = std::max<uint32>(120, sPlayerbotAIConfig.expireActionTime / 1000 + 60);
    if (!profession::IsPendingCraftExpired(static_cast<uint32>(time(nullptr)), queuedAt,
            timeout, ai->GetBot()->IsNonMeleeSpellCasted(false)))
        return true;

    ClearPendingCraft(ai, pending);
    return false;
}

void ProfessionCraftingPlanValue::QueuePendingCraft(PlayerbotAI* ai, uint32 spellId, bool acceptedCast)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    uint32 pending = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "pending profession craft")));
    // Facing/requeued requests are not progress. Only an accepted cast may
    // extend ownership of an existing batch; otherwise its lease must expire.
    if (profession::ShouldRenewCraftLease(pending, spellId, acceptedCast))
        SET_AI_VALUE2(int32, "manual int", "profession craft queued at", static_cast<int32>(time(nullptr)));
    SET_AI_VALUE2(int32, "manual int", "pending profession craft", static_cast<int32>(spellId));
    RESET_AI_VALUE(bool, "can craft profession");
}

void ProfessionCraftingPlanValue::ClearPendingCraft(PlayerbotAI* ai, uint32 spellId)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    if (AI_VALUE2(int32, "manual int", "pending profession craft") != static_cast<int32>(spellId))
        return;
    SET_AI_VALUE2(int32, "manual int", "pending profession craft", 0);
    SET_AI_VALUE2(int32, "manual int", "profession craft queued at", 0);
    // This can be called from CanCraftProfessionValue::Calculate itself.
    // Reset its cache; ClearValues would delete the value while it executes.
    RESET_AI_VALUE(bool, "can craft profession");
}

bool ProfessionCraftingPlanValue::IsAhSearchReady(PlayerbotAI* ai)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    uint32 now = static_cast<uint32>(time(nullptr));
    uint32 lastSearch = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "last profession ah search")));
    return profession::IsCooldownReady(now, lastSearch, sPlayerbotAIConfig.professionAhSearchCooldown);
}

uint32 ProfessionCraftingPlanValue::GetAhBudget(PlayerbotAI* ai)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    Player* bot = ai->GetBot();
    uint32 tradeskillBudget = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::tradeskill);
    uint32 percentageBudget = static_cast<uint32>(
        static_cast<uint64>(bot->GetMoney()) * sPlayerbotAIConfig.professionAhBudgetPercent / 100);
    return std::min(tradeskillBudget, percentageBudget);
}

bool ProfessionCraftingPlanValue::ShouldTravelForGathering(PlayerbotAI* ai, const ProfessionCraftingPlan& plan)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    ProfessionMaterialSources sources = AI_VALUE(ProfessionMaterialSources, "profession material sources");
    return profession::ShouldTravelForSources(IsEnabledFor(ai), plan.IsValid(), sources.HasGathering(), true,
        AI_VALUE(uint8, "bag space") <= 80, ai->HasActivePlayerMaster());
}

bool ProfessionCraftingPlanValue::ShouldTravelToVendor(PlayerbotAI* ai, const ProfessionCraftingPlan& plan)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    ProfessionMaterialSources sources = AI_VALUE(ProfessionMaterialSources, "profession material sources");
    uint32 tradeskillBudget = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::tradeskill);
    return profession::ShouldTravelForSources(IsEnabledFor(ai), plan.IsValid(), sources.HasVendor(),
        tradeskillBudget > 0, AI_VALUE(uint8, "bag space") <= 80, ai->HasActivePlayerMaster());
}

bool ProfessionCraftingPlanValue::ShouldTravelToAuctionHouse(PlayerbotAI* ai, const ProfessionCraftingPlan& plan)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    ProfessionMaterialSources sources = AI_VALUE(ProfessionMaterialSources, "profession material sources");
    bool ahBuyingEnabled = sPlayerbotAIConfig.professionAhPurchaseLimit > 0 &&
        sPlayerbotAIConfig.professionAhBudgetPercent > 0;
    return profession::ShouldTravelToAuctionHouse(IsEnabledFor(ai), plan.IsValid(), sources.HasAuctionHouse(),
        ahBuyingEnabled, GetAhBudget(ai) > 0, IsAhSearchReady(ai), AI_VALUE(uint8, "bag space") <= 80,
        ai->HasActivePlayerMaster());
}

bool ProfessionCraftingPlanValue::ShouldTravelToSpellFocus(PlayerbotAI* ai, const ProfessionCraftingPlan& plan)
{
    // Acquire learned-recipe tools even when a different skill owns the plan.
    // Leave an active target alone; normal travel request arbitration applies.
    if (sPlayerbotAIConfig.professionVendorPurchaseLimit && !GetMissingTools(ai).empty() &&
        ShouldTravelToVendor(ai, plan))
        return false;
    return profession::ShouldTravelToSpellFocus(IsEnabledFor(ai), plan.IsValid(), plan.spellFocusId,
        !plan.GetMissingReagents(ai).empty(), IsCraftCooldownReady(ai), ai->HasActivePlayerMaster());
}

GameObject* ProfessionCraftingPlanValue::GetCurrentSpellFocus(
    PlayerbotAI* ai, const ProfessionCraftingPlan& plan)
{
    if (!ai || !plan.IsValid() || !plan.spellFocusId)
        return nullptr;

    AiObjectContext* context = ai->GetAiObjectContext();
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    if (!travelTarget || travelTarget->GetStatus() != TravelStatus::TRAVEL_STATUS_WORK ||
        !travelTarget->GetDestination() ||
        travelTarget->GetDestination()->GetPurpose() != TravelDestinationPurpose::CraftingFocus)
        return nullptr;

    GuidPosition* focusPosition = dynamic_cast<GuidPosition*>(travelTarget->GetPosition());
    Player* bot = ai->GetBot();
    if (!bot)
        return nullptr;
    GameObject* focus = focusPosition && bot ? focusPosition->GetGameObject(bot->GetInstanceId()) : nullptr;
    MaNGOS::GameObjectFocusCheck matches(bot, plan.spellFocusId);
    if (focus && focus->GetGOInfo() && matches(focus))
        return focus;

    // Entry destinations can report arrival at another spawn of the same
    // entry. Reuse the existing cached local GO list, not the distant point.
    for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los"))
    {
        GameObject* nearby = ai->GetGameObject(guid);
        if (nearby && nearby->GetGOInfo() && matches(nearby))
            return nearby;
    }
    return nullptr;
}

ProfessionCraftingPlan ProfessionCraftingPlanValue::Calculate()
{
    ProfessionCraftingPlan bestPlan;
    if (!IsEnabledFor(ai))
        return bestPlan;

    std::vector<ProfessionCraftingPlan> plans;
    std::vector<profession::CraftCandidate> candidates;
    std::vector<uint32> spellIds = AI_VALUE(std::vector<uint32>, "craft spells");

    for (uint32 spellId : spellIds)
    {
        if (!ShouldCraftSpellValue::SpellGivesSkillUp(spellId, bot))
            continue;

        SpellEntry const* spell = sServerFacade.LookupSpellInfo(spellId);
        if (!spell)
            continue;

        uint32 skillId = 0;
        uint32 recipeMinSkill = 0;
        SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(spellId);
        for (SkillLineAbilityMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
        {
            SkillLineAbilityEntry const* ability = itr->second;
            SkillLineEntry const* skill = ability ? sSkillLineStore.LookupEntry(ability->skillId) : nullptr;
            if (!ability || !skill || !bot->HasSkill(ability->skillId))
                continue;
            if (skill->categoryId != SKILL_CATEGORY_PROFESSION && skill->categoryId != SKILL_CATEGORY_SECONDARY)
                continue;

            skillId = ability->skillId;
            recipeMinSkill = ability->min_value;
            break;
        }

        if (!skillId)
            continue;

        ProfessionCraftingPlan candidate;
        candidate.spellId = spellId;
        candidate.skillId = skillId;
        candidate.spellFocusId = spell->RequiresSpellFocus;
        candidate.craftCount = sPlayerbotAIConfig.professionCraftBatchSize;

        uint32 availableCasts = std::numeric_limits<uint32>::max();
        uint32 currentCounts[MAX_SPELL_REAGENTS] = {};
        for (uint8 reagent = 0; reagent < MAX_SPELL_REAGENTS; ++reagent)
        {
            if (spell->Reagent[reagent] <= 0 || spell->ReagentCount[reagent] <= 0)
                continue;
            uint32 perCast = spell->ReagentCount[reagent];
            candidate.craftCount = profession::BoundCraftBatch(candidate.craftCount,
                perCast, sPlayerbotAIConfig.professionMaterialTarget);
            currentCounts[reagent] = ai->GetInventoryItemsCountWithId(spell->Reagent[reagent]);
            availableCasts = std::min(availableCasts, currentCounts[reagent] / perCast);
        }
        candidate.craftCount = profession::AvailableCraftBatch(candidate.craftCount, availableCasts);

        for (uint8 effect = 0; effect < MAX_EFFECT_INDEX; ++effect)
        {
            if (spell->EffectItemType[effect])
            {
                candidate.itemId = spell->EffectItemType[effect];
                break;
            }
        }

        uint32 missingUnits = 0;
        for (uint8 reagent = 0; reagent < MAX_SPELL_REAGENTS; ++reagent)
        {
            if (spell->Reagent[reagent] <= 0 || spell->ReagentCount[reagent] <= 0)
                continue;

            uint32 reagentId = spell->Reagent[reagent];
            uint32 desired = spell->ReagentCount[reagent] * candidate.craftCount;
            uint32 current = currentCounts[reagent];

            candidate.required[reagentId] = desired;
            if (current < desired)
            {
                candidate.missing[reagentId] = desired - current;
                missingUnits += desired - current;
            }
        }

        // Prefer a recipe that can be crafted now, then the highest relevant
        // recipe with the smallest bounded material deficit.
        int64 score = candidate.missing.empty() ? 1000000000LL : 0;
        score += static_cast<int64>(recipeMinSkill) * 1000;
        score -= static_cast<int64>(missingUnits) * 10;

        candidates.push_back({skillId, spellId, score,
            candidate.missing.empty() && CanCraftSpellValue::HasRequiredTools(spell, bot)});
        plans.push_back(candidate);
    }

    // The ledger lives outside this calculated value so cast-driven cache
    // invalidation does not erase another ready skill's accumulated wait.
    profession::CraftingFairness& fairness = AI_VALUE(profession::CraftingFairness&, "profession fairness");
    size_t selected = fairness.Select(candidates, static_cast<uint32>(time(nullptr)), HasPendingCraft(ai));
    return selected == profession::CraftingFairness::NoCandidate ? bestPlan : plans[selected];
}

bool CanCraftProfessionValue::Calculate()
{
    ProfessionCraftingPlan plan = AI_VALUE(ProfessionCraftingPlan, "profession crafting plan");
    if (!ProfessionCraftingPlanValue::IsEnabledFor(ai) || !plan.IsValid() ||
        !ProfessionCraftingPlanValue::IsCraftCooldownReady(ai) ||
        !plan.GetMissingReagents(ai).empty() || AI_VALUE(uint8, "bag space") > 80)
        return false;

    SpellEntry const* spell = sServerFacade.LookupSpellInfo(plan.spellId);
    if (!spell || ProfessionCraftingPlanValue::HasPendingCraft(ai) || bot->IsNonMeleeSpellCasted(false))
        return false;

    bool atMatchingSpellFocus = ProfessionCraftingPlanValue::GetCurrentSpellFocus(ai, plan) != nullptr;
    if (!profession::IsCraftLocationReady(spell->RequiresSpellFocus, atMatchingSpellFocus))
        return false;

    return bot->HasSpell(plan.spellId) && CanCraftSpellValue::HasRequiredTools(spell, bot);
}

ProfessionMaterialSources ProfessionMaterialSourcesValue::Calculate()
{
    return AI_VALUE(ProfessionCraftingPlan, "profession crafting plan").GetMaterialSources(ai);
}
