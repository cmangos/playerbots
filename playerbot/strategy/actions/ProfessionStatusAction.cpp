#include "playerbot/playerbot.h"
#include "ProfessionStatusAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

bool ProfessionStatusAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    bool enabledForBot = ProfessionCraftingPlanValue::IsEnabledFor(ai);

    std::ostringstream status;
    status << "Profession progression: "
        << (sPlayerbotAIConfig.professionProgressionEnabled ? "enabled" : "disabled")
        << ", participation " << sPlayerbotAIConfig.professionProgressionPercent << "%"
        << ", this bot " << (enabledForBot ? "active" : "inactive");
    ai->TellPlayerNoFacing(requester, status.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);

    bool foundProfession = false;
    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); ++id)
    {
        SkillLineEntry const* skill = sSkillLineStore.LookupEntry(id);
        if (!skill || !bot->HasSkill(id))
            continue;
        if (skill->categoryId != SKILL_CATEGORY_PROFESSION && skill->categoryId != SKILL_CATEGORY_SECONDARY)
            continue;

        ai->TellPlayerNoFacing(requester, ChatHelper::formatSkill(id, bot), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);
        foundProfession = true;
    }

    for (const auto& tool : ProfessionCraftingPlanValue::GetMissingTools(ai))
    {
        ItemQualifier qualifier(tool.first);
        std::ostringstream missing;
        missing << "Missing tool: " << ChatHelper::formatItem(qualifier) << " x" << tool.second;
        ai->TellPlayerNoFacing(requester, missing.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);
    }

    ProfessionCraftingPlan plan = AI_VALUE(ProfessionCraftingPlan, "profession crafting plan");
    if (!plan.IsValid())
    {
        ai->TellPlayerNoFacing(requester, "No skill-up recipe is currently planned.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);
        return foundProfession;
    }

    SpellEntry const* spell = sServerFacade.LookupSpellInfo(plan.spellId);
    std::ostringstream planText;
    planText << "Plan: " << (spell ? ChatHelper::formatSpell(spell) : std::to_string(plan.spellId))
        << " x" << plan.craftCount
        << ", craft cooldown "
        << (ProfessionCraftingPlanValue::IsCraftCooldownReady(ai) ? "ready" : "waiting");
    if (plan.spellFocusId)
        planText << ", spell focus " << plan.spellFocusId;
    if (plan.goalSpellId && plan.goalSpellId != plan.spellId)
    {
        const SpellEntry* goal = sServerFacade.LookupSpellInfo(plan.goalSpellId);
        planText << ", prerequisite for " << (goal ? ChatHelper::formatSpell(goal) : std::to_string(plan.goalSpellId));
    }
    if (plan.processingInputId)
    {
        ItemQualifier inputQualifier(plan.processingInputId);
        planText << ", input " << ChatHelper::formatItem(inputQualifier);
    }
    ai->TellPlayerNoFacing(requester, planText.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);

    std::map<uint32, uint32> missingReagents = plan.GetMissingReagents(ai);
    for (const auto& [itemId, count] : missingReagents)
    {
        ItemQualifier qualifier(itemId);
        std::ostringstream missing;
        missing << "Missing: " << ChatHelper::formatItem(qualifier) << " x" << count;
        ai->TellPlayerNoFacing(requester, missing.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);
    }

    if (!plan.GetMissingSupplies(ai).empty())
    {
        ProfessionMaterialSources sources = AI_VALUE(ProfessionMaterialSources, "profession material sources");
        std::ostringstream acquisition;
        acquisition << "Acquisition: " << sources.gatherEntries.size() << " gather purpose(s), "
            << sources.vendorEntries.size() << " vendor(s), "
            << sources.auctionItems.size() << " AH material(s); AH cooldown "
            << (ProfessionCraftingPlanValue::IsAhSearchReady(ai) ? "ready" : "waiting");
        ai->TellPlayerNoFacing(requester, acquisition.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true);
    }

    return true;
}
