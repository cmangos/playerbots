#include "playerbot/playerbot.h"
#include "GuildCraftOrderAction.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "Guilds/GuildMgr.h"

using namespace ai;

char* strstri(const char* haystack, const char* needle);

GuildCraftOrderAction::CraftOrder GuildCraftOrderAction::ParseCraftOrder(Player* bot)
{
    CraftOrder order;

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

    auto pos = note.find("Craft:");
    if (pos == std::string::npos)
        return order;

    std::string body = note.substr(pos + 6);
    // Trim leading whitespace
    body.erase(body.begin(), std::find_if(body.begin(), body.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    // Trim trailing whitespace
    body.erase(std::find_if(body.rbegin(), body.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), body.end());

    if (body.empty())
        return order;

    // Parse officer's note. Format:
    // Craft: <item> <amount>
    // If amount is missing, default to 1.
    auto lastSpace = body.rfind(' ');
    if (lastSpace != std::string::npos)
    {
        std::string lastToken = body.substr(lastSpace + 1);
        bool isNumber = !lastToken.empty() && std::all_of(lastToken.begin(), lastToken.end(), ::isdigit);

        if (isNumber)
        {
            order.amount = std::stoul(lastToken);
            order.itemName = body.substr(0, lastSpace);

            order.itemName.erase(std::find_if(order.itemName.rbegin(), order.itemName.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), order.itemName.end());
            return order;
        }
    }

    order.itemName = body;
    order.amount = 1;

    return order;
}

uint32 GuildCraftOrderAction::FindItemByName(const std::string& name)
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
    CraftOrder order = ParseCraftOrder(bot);
    if (order.itemName.empty())
        return false;

    uint32 itemId = FindItemByName(order.itemName);
    if (!itemId)
    {
        ai->TellDebug(ai->GetMaster(), "Guild craft order: item not found: " + order.itemName, "debug travel");
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
        ai->TellDebug(ai->GetMaster(), "Guild craft order: I don't know how to craft: " + order.itemName, "debug travel");
        return false;
    }

    if (!ai->HasCheat(BotCheatMask::item) && !AI_VALUE2(bool, "can craft spell", craftSpellId))
    {
        ai->TellDebug(ai->GetMaster(), "Guild craft order: missing reagents for: " + order.itemName, "debug travel");
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
    ai->TellDebug(ai->GetMaster(), "Guild craft order: crafting " + std::to_string(remaining) + "x " + std::string(proto ? proto->Name1 : order.itemName), "debug travel");

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

    CraftOrder order = ParseCraftOrder(bot);
    if (order.itemName.empty())
        return false;

    uint32 itemId = FindItemByName(order.itemName);
    if (!itemId)
        return false;

    uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
    if (currentCount >= order.amount)
        return false;

    return true;
}