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

    for (const auto& missingReagent : GetMissingReagents(ai))
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
        if (!itemVendors.empty())
        {
            vendorEntries.insert(itemVendors.begin(), itemVendors.end());
            continue;
        }

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
    return profession::ShouldTravelToSpellFocus(IsEnabledFor(ai), plan.IsValid(), plan.spellFocusId,
        !plan.GetMissingReagents(ai).empty(), IsCraftCooldownReady(ai), ai->HasActivePlayerMaster());
}

ProfessionCraftingPlan ProfessionCraftingPlanValue::Calculate()
{
    ProfessionCraftingPlan bestPlan;
    if (!IsEnabledFor(ai))
        return bestPlan;

    int64 bestScore = std::numeric_limits<int64>::min();
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
            uint32 desired = std::min<uint32>(
                spell->ReagentCount[reagent] * candidate.craftCount,
                sPlayerbotAIConfig.professionMaterialTarget);
            uint32 current = ai->GetInventoryItemsCountWithId(reagentId);

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

        if (score > bestScore)
        {
            bestScore = score;
            bestPlan = candidate;
        }
    }

    return bestPlan;
}

bool CanCraftProfessionValue::Calculate()
{
    ProfessionCraftingPlan plan = AI_VALUE(ProfessionCraftingPlan, "profession crafting plan");
    if (!ProfessionCraftingPlanValue::IsEnabledFor(ai) || !plan.IsValid() ||
        !ProfessionCraftingPlanValue::IsCraftCooldownReady(ai) ||
        !plan.GetMissingReagents(ai).empty() || AI_VALUE(uint8, "bag space") > 80)
        return false;

    SpellEntry const* spell = sServerFacade.LookupSpellInfo(plan.spellId);
    if (!spell || spell->RequiresSpellFocus)
        return false;

    return AI_VALUE2(bool, "can craft spell", plan.spellId);
}

ProfessionMaterialSources ProfessionMaterialSourcesValue::Calculate()
{
    return AI_VALUE(ProfessionCraftingPlan, "profession crafting plan").GetMaterialSources(ai);
}
