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
    bool inCanary = ProfessionCraftingPlanValue::IsEnabledFor(ai);

    std::ostringstream status;
    status << "Profession progression: "
        << (sPlayerbotAIConfig.professionProgressionEnabled ? "enabled" : "disabled")
        << ", rollout " << sPlayerbotAIConfig.professionProgressionCanaryPercent << "%"
        << ", this bot " << (inCanary ? "active" : "inactive");
    ai->TellPlayerNoFacing(requester, status.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    bool foundProfession = false;
    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); ++id)
    {
        SkillLineEntry const* skill = sSkillLineStore.LookupEntry(id);
        if (!skill || !bot->HasSkill(id))
            continue;
        if (skill->categoryId != SKILL_CATEGORY_PROFESSION && skill->categoryId != SKILL_CATEGORY_SECONDARY)
            continue;

        ai->TellPlayerNoFacing(requester, ChatHelper::formatSkill(id, bot), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        foundProfession = true;
    }

    ProfessionCraftingPlan plan = AI_VALUE(ProfessionCraftingPlan, "profession crafting plan");
    if (!plan.IsValid())
    {
        ai->TellPlayerNoFacing(requester, "No skill-up recipe is currently planned.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return foundProfession;
    }

    SpellEntry const* spell = sServerFacade.LookupSpellInfo(plan.spellId);
    std::ostringstream planText;
    planText << "Plan: " << (spell ? ChatHelper::formatSpell(spell) : std::to_string(plan.spellId))
        << " x" << plan.craftCount;
    ai->TellPlayerNoFacing(requester, planText.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    for (const auto& [itemId, count] : plan.missing)
    {
        ItemQualifier qualifier(itemId);
        std::ostringstream missing;
        missing << "Missing: " << ChatHelper::formatItem(qualifier) << " x" << count;
        ai->TellPlayerNoFacing(requester, missing.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }

    return true;
}
