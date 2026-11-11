
#include "playerbot/playerbot.h"
#include "SetCraftAction.h"

#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
using namespace ai;

std::map<uint32, SkillLineAbilityEntry const*> SetCraftAction::skillSpells;

void SetCraftAction::LoadSkillSpells()
{
    if (!skillSpells.empty())
        return;

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (skillLine)
            skillSpells[skillLine->spellId] = skillLine;
    }
}

uint32 SetCraftAction::ParseProfession(const std::string& text)
{
    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    static const std::map<std::string, uint32> aliases = {
        {"a", SKILL_ALCHEMY}, {"b", SKILL_BLACKSMITHING}, {"c", SKILL_COOKING}, {"e", SKILL_ENGINEERING},
        {"f", SKILL_FIRST_AID}, {"firstaid", SKILL_FIRST_AID}, {"l", SKILL_LEATHERWORKING}, {"m", SKILL_ENCHANTING},
        {"magic", SKILL_ENCHANTING}, {"s", SKILL_MINING}, {"smelting", SKILL_MINING}, {"t", SKILL_TAILORING}
    };

    const auto alias = aliases.find(lower);
    if (alias != aliases.end())
        return alias->second;

    return ChatHelper::parseSkillName(lower);
}

void SetCraftAction::TellCraftableByProfession(Player* requester, uint32 skill)
{
    LoadSkillSpells();

    std::vector<std::string> lines;
    std::string line;
    uint32 count = 0;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(itr->first))
            continue;

        const auto skillLine = skillSpells.find(itr->first);
        if (skillLine == skillSpells.end() || skillLine->second->skillId != skill)
            continue;

        const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(itr->first);
        if (!pSpellInfo)
            continue;

        for (int i = 0; i < 3; ++i)
        {
            if (pSpellInfo->Effect[i] != SPELL_EFFECT_CREATE_ITEM)
                continue;

            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(pSpellInfo->EffectItemType[i]);
            if (!proto)
                continue;

            const std::string link = chat->formatItem(proto);
            if (!line.empty() && line.size() + link.size() > 240)
            {
                lines.push_back(line);
                line.clear();
            }

            line += (line.empty() ? "" : " ") + link;
            count++;
        }
    }

    if (!line.empty())
        lines.push_back(line);

    if (!count)
    {
        ai->TellPlayer(requester, "I do not know any " + ChatHelper::getSkillName(skill) + " recipes.");
        return;
    }

    ai->TellPlayer(requester, "I can craft (" + std::to_string(count) + ") " + ChatHelper::getSkillName(skill) + " recipes:");
    for (const std::string& entry : lines)
        ai->TellPlayer(requester, entry);
}

bool SetCraftAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    std::string link = event.getParam();

    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (link == "reset")
    {
        data.Reset();
        ai->TellPlayer(requester, "I will not craft anything");
        return true;
    }

    if (link == "?")
    {
        TellCraft(requester);
        return true;
    }

    const uint32 profession = ParseProfession(link);
    if (profession != SKILL_NONE)
    {
        TellCraftableByProfession(requester, profession);
        return true;
    }

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
    {
        ai->TellPlayer(requester, "Usage: 'craft [itemId]', 'craft <profession>' or 'craft reset'");
        return false;
    }

    uint32 itemId = *itemIds.begin();
    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
    if (!proto)
        return false;

    LoadSkillSpells();

    data.required.clear();
    data.obtained.clear();

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr) {
        const uint32 spellId = itr->first;

        if (itr->second.state == PLAYERSPELL_REMOVED || itr->second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        SkillLineAbilityEntry const* skillLine = skillSpells[spellId];
        if (skillLine)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (pSpellInfo->Effect[i] == SPELL_EFFECT_CREATE_ITEM)
                {
                    if (itemId == pSpellInfo->EffectItemType[i])
                    {
                        for (uint32 x = 0; x < MAX_SPELL_REAGENTS; ++x)
                        {
                            if (pSpellInfo->Reagent[x] <= 0)
                                { continue; }

                            uint32 itemid = pSpellInfo->Reagent[x];
                            uint32 reagentsRequired = pSpellInfo->ReagentCount[x];
                            if (itemid)
                            {
                                data.required[itemid] = reagentsRequired;
                                data.obtained[itemid] = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    if (data.required.empty())
    {
        ai->TellPlayer(requester, "I cannot craft this");
        return false;
    }

    data.itemId = itemId;

    TellCraft(requester);
    return true;
}

void SetCraftAction::TellCraft(Player* requester)
{
    CraftData& data = AI_VALUE(CraftData&, "craft");
    if (data.IsEmpty())
    {
        ai->TellPlayer(requester, "I will not craft anything");
        return;
    }

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(data.itemId);
    if (!proto)
        return;

    std::ostringstream out;
    out << "I will craft " << chat->formatItem(proto) << " using reagents: ";
    bool first = true;
    for (std::map<uint32, int>::iterator i = data.required.begin(); i != data.required.end(); ++i)
    {
        uint32 item = i->first;
        int required = i->second;
        ItemPrototype const* reagent = sObjectMgr.GetItemPrototype(item);
        if (reagent)
        {
            if (first) { first = false; } else out << ", ";
            out << chat->formatItem(reagent, required);
            uint32 given = data.obtained[item];
            if (given)
            {
                out << "|cffffff00(x" << given << " given)|r ";
            }
        }
    }

    out << " (craft fee: " << chat->formatMoney(GetCraftFee(data)) << ")";
    ai->TellPlayer(requester, out.str());
}

uint32 SetCraftAction::GetCraftFee(CraftData& data)
{
    if (data.IsEmpty())
        return 0;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(data.itemId);
    if (!proto)
        return 0;

    return ItemUsageValue::GetCraftingFee(proto);
}
