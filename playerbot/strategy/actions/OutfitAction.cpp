
#include "playerbot/playerbot.h"
#include "OutfitAction.h"
#include "playerbot/strategy/values/OutfitListValue.h"
#include "playerbot/strategy/ItemVisitors.h"

using namespace ai;

bool OutfitAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();

    if (param == "?")
    {
        List(requester);
        ai->TellPlayer(requester, "outfit <name> +[item] to add items");
        ai->TellPlayer(requester, "outfit <name> -[item] to remove items");
        ai->TellPlayer(requester, "outfit <name> equip/replace to equip items");
    }
    else
    {
        std::string name = ai->InventoryParseOutfitName(param);
        ItemIds items = ai->InventoryParseOutfitItems(param);
        if (!name.empty())
        {
            Save(name, items);
            std::ostringstream out;
            out << "Setting outfit " << name << " as " << param;
            ai->TellPlayer(requester, out);
            return true;
        }

        items = chat->parseItems(param);

        int space = param.find(" ");
        if (space == -1)
            return false;

        name = param.substr(0, space);
        ItemIds outfit = ai->InventoryFindOutfitItems(name);
        std::string command = param.substr(space + 1);
        if (command == "equip")
        {
            std::ostringstream out;
            out << "Equipping outfit " << name;
            ai->TellPlayer(requester, out);
            EquipItems(requester, outfit);
            return true;
        }
        else if (command == "replace")
        {
            std::ostringstream out;
            out << "Replacing current equip with outfit " << name;
            ai->TellPlayer(requester, out);
            for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
            {
                Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
                if (!pItem)
                    continue;

                uint8 bagIndex = pItem->GetBagSlot();
                uint8 dstBag = NULL_BAG;

                WorldPacket packet(CMSG_AUTOSTORE_BAG_ITEM, 3);
                packet << bagIndex << slot << dstBag;
                bot->GetSession()->HandleAutoStoreBagItemOpcode(packet);
            }
            EquipItems(requester, outfit);
            return true;
        }
        else if (command == "reset")
        {
            std::ostringstream out;
            out << "Resetting outfit " << name;
            ai->TellPlayer(requester, out);
            Save(name, ItemIds());
            return true;
        }
        else if (command == "update")
        {
            std::ostringstream out;
            out << "Updating with current items outfit " << name;
            ai->TellPlayer(requester, out);
            Update(name);
            return true;
        }

        bool remove = param.size() > 1 && param.substr(space + 1, 1) == "-";
        for (ItemIds::iterator i = items.begin(); i != items.end(); i++)
        {
            uint32 itemid = *i;
            ItemPrototype const *proto = sItemStorage.LookupEntry<ItemPrototype>(*i);
            std::ostringstream out;
            out << chat->formatItem(proto);
            if (remove)
            {
                std::set<uint32>::iterator j = outfit.find(itemid);
                if (j != outfit.end())
                    outfit.erase(j);

                out << " removed from ";
            }
            else
            {
                outfit.insert(itemid);
                out << " added to ";
            }
            out << name;
            ai->TellPlayer(requester, out.str());
        }
        Save(name, outfit);
    }

    return true;
}

void OutfitAction::Save(std::string name, ItemIds items)
{
    std::list<std::string>& outfits = AI_VALUE(std::list<std::string>&, "outfit list");
    for (std::list<std::string>::iterator i = outfits.begin(); i != outfits.end(); ++i)
    {
        std::string outfit = *i;
        if (name == ai->InventoryParseOutfitName(outfit))
        {
            outfits.erase(i);
            break;
        }
    }

    if (items.empty()) return;

    std::ostringstream out;
    out << name << "=";
    bool first = true;
    for (ItemIds::iterator i = items.begin(); i != items.end(); i++)
    {
        if (first) first = false; else out << ",";
        out << *i;
    }
    outfits.push_back(out.str());
}

void OutfitAction::List(Player* requester)
{
    std::list<std::string>& outfits = AI_VALUE(std::list<std::string>&, "outfit list");
    for (std::list<std::string>::iterator i = outfits.begin(); i != outfits.end(); ++i)
    {
        std::string outfit = *i;
        std::string name = ai->InventoryParseOutfitName(outfit);
        ItemIds items = ai->InventoryParseOutfitItems(outfit);

        std::ostringstream out;
        out << name << ": ";
        for (ItemIds::iterator j = items.begin(); j != items.end(); ++j)
        {
            ItemPrototype const *proto = sItemStorage.LookupEntry<ItemPrototype>(*j);
            if (proto)
            {
                out << chat->formatItem(proto) << " ";
            }
        }
        ai->TellPlayer(requester, out);
    }
}

void OutfitAction::Update(std::string name)
{
    ListItemsVisitor visitor;
    ai->InventoryIterateItems(&visitor, IterateItemsMask::ITERATE_ITEMS_IN_EQUIP);

    ItemIds items;
    for (std::map<uint32, int>::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
        items.insert(i->first);

    Save(name, items);
}

static std::string ResistStrategyFor(uint8 cls, const std::string& school)
{
    if (cls == CLASS_PALADIN)
    {
        if (school == "fire" || school == "frost" || school == "shadow")
            return "aura " + school;
    }
    else if (cls == CLASS_SHAMAN)
    {
        if (school == "fire")
            return "totem water resistance";
        if (school == "frost")
            return "totem fire resistance";
        if (school == "nature")
            return "totem air resistance";
    }
    else if (cls == CLASS_HUNTER)
    {
        if (school == "nature")
            return "aspect wild";
    }

    return "";
}

static std::string ResistStrategyPlaceholder(const std::string& strategy)
{
    if (strategy.compare(0, 5, "aura ") == 0)
        return "aura";
    if (strategy.compare(0, 6, "totem ") == 0)
        return "totems";
    if (strategy.compare(0, 7, "aspect ") == 0)
        return "aspect";
    return "";
}

static bool IsAnyItemEquipped(Player* bot, const ItemIds& ids)
{
    for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; slot++)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (item && ids.find(item->GetProto()->ItemId) != ids.end())
            return true;
    }

    return false;
}

std::string ResistAction::FindResistOutfit(const std::string& school)
{
    if (school.empty())
        return "";

    const std::vector<std::string> candidates = { school, school + "res", "resist" + school, school + "resist" };
    for (const std::string& candidate : candidates)
    {
        if (!ai->InventoryFindOutfitItems(candidate).empty())
            return candidate;
    }

    return "";
}

bool ResistAction::WearsResistOutfit()
{
    const std::vector<std::string> schools = { "fire", "frost", "nature", "shadow" };
    for (const std::string& school : schools)
    {
        const std::string outfit = FindResistOutfit(school);
        if (!outfit.empty() && IsAnyItemEquipped(bot, ai->InventoryFindOutfitItems(outfit)))
            return true;
    }

    return false;
}

void ResistAction::ChangeResistStrategy(const std::string& previousSchool, const std::string& school)
{
    const uint8 cls = bot->getClass();
    const std::string previous = ResistStrategyFor(cls, previousSchool);
    const std::string next = ResistStrategyFor(cls, school);
    const BotState states[] = { BotState::BOT_STATE_COMBAT, BotState::BOT_STATE_NON_COMBAT };

    if (!previous.empty())
    {
        const std::string snapshot = AI_VALUE(std::string, "resist order strategies");
        const size_t separator = snapshot.find('|');
        std::string restore[2];
        if (separator == std::string::npos)
        {
            restore[0] = ResistStrategyPlaceholder(previous);
            restore[1] = restore[0];
        }
        else
        {
            restore[0] = snapshot.substr(0, separator);
            restore[1] = snapshot.substr(separator + 1);
        }

        for (uint8 i = 0; i < 2; ++i)
        {
            ai->ChangeStrategy("-" + previous, states[i]);
            if (!restore[i].empty())
                ai->ChangeStrategy("+" + restore[i], states[i]);
        }
    }

    if (next.empty())
    {
        SET_AI_VALUE(std::string, "resist order strategies", "");
        return;
    }

    std::set<std::string> group = ai->GetAiObjectContext()->GetSiblingStrategy(next);
    group.insert(next);
    std::string active[2];
    for (uint8 i = 0; i < 2; ++i)
    {
        for (const std::string& strategy : group)
        {
            if (ai->HasStrategy(strategy, states[i]))
            {
                active[i] = strategy;
                break;
            }
        }
    }

    SET_AI_VALUE(std::string, "resist order strategies", active[0] + "|" + active[1]);

    for (uint8 i = 0; i < 2; ++i)
        ai->ChangeStrategy("+" + next, states[i]);
}

static std::string TrimCopy(const std::string& text)
{
    const size_t first = text.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    const size_t last = text.find_last_not_of(' ');
    return text.substr(first, last - first + 1);
}

bool ResistAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = TrimCopy(event.getParam());
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    const std::string current = AI_VALUE(std::string, "resist order");
    const size_t separator = current.find(':');
    const std::string currentSchool = current.substr(0, separator);
    const std::string currentOutfit = separator == std::string::npos ? "" : current.substr(separator + 1);

    if (text.empty() || text == "?")
    {
        if (currentSchool.empty())
            ai->TellPlayer(requester, "No resist order.");
        else
            ai->TellPlayer(requester, "Resisting " + currentSchool + (currentOutfit.empty() ? "" : " (outfit " + currentOutfit + ")") + ".");
        return true;
    }

    std::string school = text;
    if (school.compare(0, 6, "resist") == 0)
        school = TrimCopy(school.substr(6));

    if (school == "none" || school == "clear" || school == "reset" || school == "off")
        school = "";
    else if (school != "fire" && school != "frost" && school != "nature" && school != "shadow")
    {
        ai->TellPlayer(requester, "Usage: resist <fire|frost|nature|shadow|none>");
        return false;
    }

    if (school.empty() && currentSchool.empty())
    {
        ai->TellPlayer(requester, "No resist order.");
        return true;
    }

    ChangeResistStrategy(currentSchool, school);

    const bool inCombat = bot->IsInCombat();
    std::string outfit;
    bool backupMissing = false;
    if (!inCombat)
    {
        outfit = FindResistOutfit(school);
        ItemIds backup = ai->InventoryFindOutfitItems("noresist");

        if (!outfit.empty())
        {
            if (currentOutfit.empty() && !(!backup.empty() && WearsResistOutfit()))
                Update("noresist");
            else if (!currentOutfit.empty() && currentOutfit != outfit && !backup.empty())
                EquipItems(requester, backup);

            EquipItems(requester, ai->InventoryFindOutfitItems(outfit));
        }
        else if (!currentOutfit.empty())
        {
            if (!backup.empty())
            {
                EquipItems(requester, backup);
                Save("noresist", ItemIds());
            }
            else
                backupMissing = true;
        }
    }

    SET_AI_VALUE(std::string, "resist order", school.empty() ? "" : (outfit.empty() ? school : school + ":" + outfit));

    if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
        sPlayerbotDbStore.Save(ai);

    std::ostringstream out;
    if (school.empty())
        out << "Resist order cleared";
    else
    {
        const std::string strategy = ResistStrategyFor(bot->getClass(), school);
        out << "Resisting " << school << ": ";
        out << (strategy.empty() ? "no " + school + " resist spell for my class" : "using " + strategy);
        if (!inCombat)
            out << ", " << (outfit.empty() ? "no resist outfit (name one " + school + " or " + school + "res)" : "equipped outfit " + outfit);
    }

    if (inCombat)
        out << ", gear will not change in combat";
    if (backupMissing)
        out << ", could not restore my normal gear (no noresist outfit)";
    out << ".";

    ai->TellPlayer(requester, out.str());
    return true;
}
