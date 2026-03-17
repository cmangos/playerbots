#include "playerbot/playerbot.h"
#include "GuildCraftOrderAction.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "playerbot/strategy/values/GuildValues.h"

using namespace ai;

uint32 GuildCraftOrderAction::FindCraftSpell(uint32 itemId)
{
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
                return spellId;
        }
    }

    return 0;
}

bool GuildCraftOrderAction::Execute(Event& event)
{
    GuildOrder order = AI_VALUE(GuildOrder, "guild order");
    if (!order.IsCraftOrder())
        return false;

    uint32 itemId = GuildOrderValue::FindItemByName(order.target);
    if (!itemId)
    {
        ai->TellDebug(ai->GetMaster(), "Guild craft order: item not found: " + order.target, "debug travel");
        return false;
    }

    uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
    if (currentCount >= order.amount)
    {
        ai->TellDebug(ai->GetMaster(), "Guild craft order: already have " + std::to_string(currentCount) + "/" + std::to_string(order.amount) + ", skipping", "debug travel");
        return false;
    }

    uint32 craftSpellId = FindCraftSpell(itemId);
    if (!craftSpellId)
    {
        ai->TellDebug(ai->GetMaster(), "Guild craft order: I don't know how to craft: " + order.target, "debug travel");
        return false;
    }

    if (!ai->HasCheat(BotCheatMask::item) && !AI_VALUE2(bool, "can craft spell", craftSpellId))
    {
        ai->TellDebug(ai->GetMaster(), "Guild craft order: missing reagents for: " + order.target, "debug travel");
        return false;
    }

    // Calculate the amount to craft
    uint32 remaining = order.amount - currentCount;

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(craftSpellId);
    if (!pSpellInfo)
        return false;

    uint32 newItemId = pSpellInfo->EffectItemType[0];
    if (newItemId)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(newItemId);
        if (proto && remaining > proto->GetMaxStackSize())
            remaining = proto->GetMaxStackSize();
    }

    if (remaining < 1)
        remaining = 1;

    std::ostringstream cmd;
    cmd << "castnc " << craftSpellId << " " << remaining;
    ai->HandleCommand(CHAT_MSG_WHISPER, cmd.str(), *bot);

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
    ai->TellDebug(ai->GetMaster(), "Guild craft order: crafting " + std::to_string(remaining) + "x " + std::string(proto ? proto->Name1 : order.target), "debug travel");

    return true;
}

bool GuildCraftOrderAction::isUseful()
{
    if (!bot->GetGuildId())
        return false;

    if (bot->IsMoving())
        return false;

    if (bot->IsMounted())
        return false;

    GuildOrder order = AI_VALUE(GuildOrder, "guild order");
    if (!order.IsCraftOrder())
        return false;

    uint32 itemId = GuildOrderValue::FindItemByName(order.target);
    if (!itemId)
        return false;

    uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
    if (currentCount >= order.amount)
        return false;

    return true;
}