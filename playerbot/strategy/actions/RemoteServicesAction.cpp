#include "playerbot/playerbot.h"
#include "RemoteServicesAction.h"
#include "BankAction.h"
#include "playerbot/RemoteServiceAccess.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"

using namespace ai;

bool RemoteServicesAction::IsReady(PlayerbotAI* ai)
{
    if (!RemoteServiceAccess::IsUsableNow(ai) || ProfessionCraftingPlanValue::HasPendingCraft(ai))
        return false;
    AiObjectContext* context = ai->GetAiObjectContext();
    uint32 last = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "last remote services")));
    return profession::IsCooldownReady(static_cast<uint32>(time(nullptr)), last, 60);
}

bool RemoteServicesAction::Execute(Event& event)
{
    if (!IsReady(ai))
        return false;
    uint32 now = static_cast<uint32>(time(nullptr));
    // Advance even if there is nothing to do or a transaction is rejected.
    SET_AI_VALUE2(int32, "manual int", "last remote services", static_cast<int32>(now));
    bool changed = false;

    BankAction bank(ai);
    if (ai->HasStrategy("rpg bank", BotState::BOT_STATE_NON_COMBAT) &&
        AI_VALUE(bool, "should bank withdraw"))
    {
        changed = bank.AutoWithdraw() || changed;
        if (changed)
            ai->DoSpecificAction("equip upgrades", event, true);
    }

    if (AI_VALUE(bool, "can get mail") && AI_VALUE(bool, "should get mail"))
    {
        changed = ai->DoSpecificAction("mail", Event("rpg action", "take"), true) || changed;
        // Existing mail collection stores actual delivered attachments through
        // core handlers; invalidate cached requirements after that operation.
        ai->GetAiObjectContext()->ClearValues("profession crafting plan");
        ai->GetAiObjectContext()->ClearValues("profession material sources");
        ai->GetAiObjectContext()->ClearValues("item count");
        ai->GetAiObjectContext()->ClearValues("inventory items");
        ai->GetAiObjectContext()->ClearValues("item usage");
        RESET_AI_VALUE(uint8, "bag space");
        ai->DoSpecificAction("equip upgrades", event, true);
    }

    ProfessionCraftingPlan plan = AI_VALUE(ProfessionCraftingPlan, "profession crafting plan");
    bool professionPurchase = ProfessionCraftingPlanValue::ShouldTravelToAuctionHouse(ai, plan);
    if (professionPurchase)
        changed = ai->DoSpecificAction("ah bid", Event("rpg action", "profession"), true) || changed;

    // Ordinary AH browsing/selling is no longer bounded by a trip to town.
    // Keep a separate slow attempt deadline, including empty/failed scans.
    uint32 lastAuction = static_cast<uint32>(std::max<int32>(0,
        AI_VALUE2(int32, "manual int", "last remote auction")));
    if (profession::IsCooldownReady(now, lastAuction, 600))
    {
        SET_AI_VALUE2(int32, "manual int", "last remote auction", static_cast<int32>(now));
        if (AI_VALUE(bool, "can ah sell") && AI_VALUE(bool, "should ah sell"))
            changed = ai->DoSpecificAction("ah", Event("rpg action", "vendor"), true) || changed;
        if (!professionPurchase && AI_VALUE(bool, "can ah buy"))
            changed = ai->DoSpecificAction("ah bid", Event("rpg action", "vendor"), true) || changed;
    }

    if (ai->HasStrategy("rpg bank", BotState::BOT_STATE_NON_COMBAT) &&
        AI_VALUE(bool, "should bank deposit"))
        changed = bank.AutoDeposit() || changed;
    return changed;
}
