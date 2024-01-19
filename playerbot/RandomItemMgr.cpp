#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "RandomItemMgr.h"

#include "DBCStore.h"
#include "../../modules/Bots/ahbot/AhBot.h"
#include "DatabaseEnv.h"
#include "PlayerbotAI.h"

#include "../../modules/Bots/ahbot/AhBotConfig.h"
#include "ServerFacade.h"
#include "strategy/values/LootValues.h"

char * strstri (const char* str1, const char* str2);

uint64 BotEquipKey::GetKey()
{
    return level + 100 * clazz + 10000 * spec + 1000000 * slot + 100000000 * quality;
}

RandomItemMgr::RandomItemMgr()
{
    viableSlots[EQUIPMENT_SLOT_HEAD].insert(INVTYPE_HEAD);
    viableSlots[EQUIPMENT_SLOT_NECK].insert(INVTYPE_NECK);
    viableSlots[EQUIPMENT_SLOT_SHOULDERS].insert(INVTYPE_SHOULDERS);
    viableSlots[EQUIPMENT_SLOT_BODY].insert(INVTYPE_BODY);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_CHEST);
    viableSlots[EQUIPMENT_SLOT_CHEST].insert(INVTYPE_ROBE);
    viableSlots[EQUIPMENT_SLOT_WAIST].insert(INVTYPE_WAIST);
    viableSlots[EQUIPMENT_SLOT_LEGS].insert(INVTYPE_LEGS);
    viableSlots[EQUIPMENT_SLOT_FEET].insert(INVTYPE_FEET);
    viableSlots[EQUIPMENT_SLOT_WRISTS].insert(INVTYPE_WRISTS);
    viableSlots[EQUIPMENT_SLOT_HANDS].insert(INVTYPE_HANDS);
    viableSlots[EQUIPMENT_SLOT_FINGER1].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_FINGER2].insert(INVTYPE_FINGER);
    viableSlots[EQUIPMENT_SLOT_TRINKET1].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_TRINKET2].insert(INVTYPE_TRINKET);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_MAINHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_2HWEAPON);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_SHIELD);
    //viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_WEAPONMAINHAND);
    viableSlots[EQUIPMENT_SLOT_OFFHAND].insert(INVTYPE_HOLDABLE);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGED);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_THROWN);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RANGEDRIGHT);
    viableSlots[EQUIPMENT_SLOT_RANGED].insert(INVTYPE_RELIC);
    viableSlots[EQUIPMENT_SLOT_TABARD].insert(INVTYPE_TABARD);
    viableSlots[EQUIPMENT_SLOT_BACK].insert(INVTYPE_CLOAK);

    weightStatLink["sta"] = ITEM_MOD_STAMINA;
    weightStatLink["str"] = ITEM_MOD_STRENGTH;
    weightStatLink["agi"] = ITEM_MOD_AGILITY;
    weightStatLink["int"] = ITEM_MOD_INTELLECT;
    weightStatLink["spi"] = ITEM_MOD_SPIRIT;

    ItemStatLink[STAT_STAMINA] = "sta";
    ItemStatLink[STAT_STRENGTH] = "str";
    ItemStatLink[STAT_AGILITY] = "agi";
    ItemStatLink[STAT_INTELLECT] = "int";
    ItemStatLink[STAT_SPIRIT] = "spi";

#ifdef MANGOSBOT_TWO
    weightStatLink["splpwr"] = ITEM_MOD_SPELL_POWER;
    weightStatLink["atkpwr"] = ITEM_MOD_ATTACK_POWER;
    weightStatLink["feratkpwr"] = ITEM_MOD_FERAL_ATTACK_POWER;

    weightStatLink["exprtng"] = ITEM_MOD_EXPERTISE_RATING;
    weightStatLink["critstrkrtng"] = ITEM_MOD_CRIT_RATING;
    weightStatLink["hitrtng"] = ITEM_MOD_HIT_RATING;
    weightStatLink["hastertng"] = ITEM_MOD_HASTE_RATING;
    weightStatLink["armorpenrtng"] = ITEM_MOD_ARMOR_PENETRATION_RATING;
    
    weightStatLink["defrtng"] = ITEM_MOD_DEFENSE_SKILL_RATING;
    weightStatLink["dodgertng"] = ITEM_MOD_DODGE_RATING;
    weightStatLink["block"] = ITEM_MOD_BLOCK_VALUE;
    weightStatLink["blockrtng"] = ITEM_MOD_BLOCK_RATING;
    weightStatLink["parryrtng"] = ITEM_MOD_PARRY_RATING;
    
    weightStatLink["manargn"] = ITEM_MOD_MANA_REGENERATION;

    weightRatingLink["exprtng"] = CR_EXPERTISE;
    weightRatingLink["critstrkrtng"] = CR_CRIT_MELEE;
    weightRatingLink["hitrtng"] = CR_HIT_MELEE;
    weightRatingLink["hastertng"] = CR_HASTE_MELEE;
    weightRatingLink["armorpenrtng"] = CR_ARMOR_PENETRATION;

    weightRatingLink["defrtng"] = CR_DEFENSE_SKILL;
    weightRatingLink["dodgertng"] = CR_DODGE;
    weightRatingLink["blockrtng"] = CR_BLOCK;
    weightRatingLink["parryrtng"] = CR_PARRY;
#endif

#ifdef MANGOSBOT_ONE
    //weightStatLink["splpwr"] = ITEM_MOD_SPELL_POWER;
    //weightStatLink["atkpwr"] = ITEM_MOD_ATTACK_POWER;
    //weightStatLink["feratkpwr"] = ITEM_MOD_FERAL_ATTACK_POWER;

    weightStatLink["exprtng"] = ITEM_MOD_EXPERTISE_RATING;
    weightStatLink["critstrkrtng"] = ITEM_MOD_CRIT_RATING;
    weightStatLink["spellcritstrkrtng"] = ITEM_MOD_CRIT_SPELL_RATING;
    weightStatLink["hitrtng"] = ITEM_MOD_HIT_RATING;
    weightStatLink["spellhitrtng"] = ITEM_MOD_HIT_SPELL_RATING;
    weightStatLink["hastertng"] = ITEM_MOD_HASTE_RATING;
    weightStatLink["spellhastertng"] = ITEM_MOD_HASTE_SPELL_RATING;
    //weightStatLink["armorpenrtng"] = ITEM_MOD_ARMOR_PENETRATION_RATING;

    weightStatLink["defrtng"] = ITEM_MOD_DEFENSE_SKILL_RATING;
    weightStatLink["dodgertng"] = ITEM_MOD_DODGE_RATING;
    //weightStatLink["block"] = ITEM_MOD_BLOCK_VALUE;
    weightStatLink["blockrtng"] = ITEM_MOD_BLOCK_RATING;
    weightStatLink["parryrtng"] = ITEM_MOD_PARRY_RATING;

    //weightStatLink["manargn"] = ITEM_MOD_MANA_REGENERATION;

    weightRatingLink["exprtng"] = CR_EXPERTISE;
    weightRatingLink["critstrkrtng"] = CR_CRIT_MELEE;
    weightRatingLink["hitrtng"] = CR_HIT_MELEE;
    weightRatingLink["hastertng"] = CR_HASTE_MELEE;
    weightRatingLink["spellcritstrkrtng"] = CR_CRIT_SPELL;
    weightRatingLink["spellhastertng"] = CR_HASTE_SPELL;
    weightRatingLink["spellhitrtng"] = CR_HIT_SPELL;
    //weightRatingLink["armorpenrtng"] = CR_ARMOR_PENETRATION;

    weightRatingLink["defrtng"] = CR_DEFENSE_SKILL;
    weightRatingLink["dodgertng"] = CR_DODGE;
    weightRatingLink["blockrtng"] = CR_BLOCK;
    weightRatingLink["parryrtng"] = CR_PARRY;
#endif
}

void RandomItemMgr::Init()
{
    BuildItemInfoCache();
    BuildEquipCache();
    BuildAmmoCache();
    BuildPotionCache();
    BuildFoodCache();
    BuildTradeCache();
}

void RandomItemMgr::InitAfterAhBot()
{
    BuildRandomItemCache();
    //BuildRarityCache();
}

RandomItemMgr::~RandomItemMgr()
{
    for (map<RandomItemType, RandomItemPredicate*>::iterator i = predicates.begin(); i != predicates.end(); ++i)
        delete i->second;

    predicates.clear();
}

bool RandomItemMgr::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!args || !*args)
    {
        sLog.outError( "Usage: rnditem");
        return false;
    }

    return false;
}

RandomItemList RandomItemMgr::Query(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList &list = randomItemCache[(level - 1) / 10][type];

    RandomItemList result;
    for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
    {
        uint32 itemId = *i;
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        if (predicate && !predicate->Apply(proto))
            continue;

        result.push_back(itemId);
    }

    return result;
}

void RandomItemMgr::BuildRandomItemCache()
{
    auto results = PlayerbotDatabase.PQuery("select lvl, type, item from ai_playerbot_rnditem_cache");
    if (results)
    {
        sLog.outString("Loading random item cache");
        int count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 level = fields[0].GetUInt32();
            uint32 type = fields[1].GetUInt32();
            uint32 itemId = fields[2].GetUInt32();

            RandomItemType rit = (RandomItemType)type;
            randomItemCache[level][rit].push_back(itemId);
            count++;

        } while (results->NextRow());
        sLog.outString("Equipment cache loaded from %d records", count);
    }
    else
    {
        sLog.outString("Building random item cache from %u items", sItemStorage.GetMaxEntry());
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (sAhBotConfig.ignoreItemIds.find(proto->ItemId) != sAhBotConfig.ignoreItemIds.end())
                continue;

            if (strstri(proto->Name1, "qa") || strstri(proto->Name1, "test") || strstri(proto->Name1, "deprecated"))
                continue;

            if (!proto->ItemLevel/* || (proto->RequiredLevel && proto->RequiredLevel > sAhBotConfig.maxRequiredLevel) || proto->ItemLevel > sAhBotConfig.maxItemLevel*/)
                continue;

            if (!auctionbot.GetSellPrice(proto))
                continue;

            uint32 level = proto->ItemLevel;
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemType rit = (RandomItemType)type;
                if (predicates[rit] && !predicates[rit]->Apply(proto))
                    continue;

                randomItemCache[level / 10][rit].push_back(itemId);
                PlayerbotDatabase.PExecute("insert into ai_playerbot_rnditem_cache (lvl, type, item) values (%u, %u, %u)",
                        level / 10, type, itemId);
            }
        }

        uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
        if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
            maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);
        for (uint32 level = 0; level < (maxLevel / 10); level++)
        {
            for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; type++)
            {
                RandomItemList list = randomItemCache[level][(RandomItemType)type];
                sLog.outDetail("    Level %d..%d Type %d - %zu random items cached",
                        level * 10, level * 10 + 9,
                        type,
                        list.size());
                for (RandomItemList::iterator i = list.begin(); i != list.end(); ++i)
                {
                    uint32 itemId = *i;
                    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                    if (!proto)
                        continue;

                    sLog.outDetail("        [%d] %s", itemId, proto->Name1);
                }
            }
        }
    }
}

uint32 RandomItemMgr::GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate)
{
    RandomItemList const& list = Query(level, type, predicate);
    if (list.empty())
        return 0;

    uint32 index = urand(0, list.size() - 1);
    uint32 itemId = list[index];

    return itemId;
}

bool RandomItemMgr::CanEquipItem(BotEquipKey key, ItemPrototype const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Quality != key.quality)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return true;

    set<InventoryType> slots = viableSlots[(EquipmentSlots)key.slot];
    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
        return false;

    uint32 requiredLevel = proto->RequiredLevel;

    if (!requiredLevel)
    {
        requiredLevel = GetMinLevelFromCache(proto->ItemId);
    }
    if (!requiredLevel)
        return false;

    /*if (!requiredLevel)
        requiredLevel = key.level;*/

    // test
    return true;

    uint32 level = key.level;
    uint32 delta = 2;
    if (level < 15)
        delta = urand(7, 15);
    else if (proto->Class == ITEM_CLASS_WEAPON || proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        delta = urand(2, 3);
    else if (!(level % 10) || (level % 10) == 9)
        delta = 2;
    else if (level < 40)
        delta = urand(5, 10);
    else if (level < 60)
        delta = urand(3, 7);
    else if (level < 70)
        delta = urand(2, 5);
    else if (level < 80)
        delta = urand(2, 4);

    if (key.quality > ITEM_QUALITY_NORMAL &&
            (requiredLevel > level || requiredLevel < (level - delta)))
        return false;

    for (uint32 gap = 60; gap <= 80; gap += 10)
    {
        if (level > gap && requiredLevel <= gap)
            return false;
    }

    return true;
}

bool RandomItemMgr::CanEquipItemNew(ItemPrototype const* proto)
{
    if (proto->Duration & 0x80000000)
        return false;

    if (proto->Bonding == BIND_QUEST_ITEM || proto->Bonding == BIND_WHEN_USE)
        return false;

    if (proto->Class == ITEM_CLASS_CONTAINER)
        return false;

    bool properSlot = false;
    for (map<EquipmentSlots, set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end(); ++i)
    {
        set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
            properSlot = true;
    }

    return properSlot;
}

void RandomItemMgr::AddItemStats(uint32 mod, uint8 &sp, uint8 &ap, uint8 &tank)
{
    switch (mod)
    {
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
    case ITEM_MOD_MANA:
    case ITEM_MOD_INTELLECT:
    case ITEM_MOD_SPIRIT:
        sp++;
        break;
    }

    switch (mod)
    {
    case ITEM_MOD_AGILITY:
    case ITEM_MOD_STRENGTH:
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
        tank++;
        break;
    }

    switch (mod)
    {
    case ITEM_MOD_HEALTH:
    case ITEM_MOD_STAMINA:
    case ITEM_MOD_AGILITY:
    case ITEM_MOD_STRENGTH:
        ap++;
        break;
    }
}

bool RandomItemMgr::CheckItemStats(uint8 clazz, uint8 sp, uint8 ap, uint8 tank)
{
    switch (clazz)
    {
    case CLASS_PRIEST:
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        if (!sp || ap > sp || tank > sp)
            return false;
        break;
    case CLASS_PALADIN:
    case CLASS_WARRIOR:
        if ((!ap && !tank) || sp > ap || sp > tank)
            return false;
        break;
    case CLASS_HUNTER:
    case CLASS_ROGUE:
        if (!ap || sp > ap || sp > tank)
            return false;
        break;
    }

    return sp || ap || tank;
}

bool RandomItemMgr::ShouldEquipArmorForSpec(uint8 playerclass, uint8 spec, ItemPrototype const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if (!m_weightScales[spec].info.id)
        return false;

    std::unordered_set<uint32> resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };

    switch (playerclass)
    {
    case CLASS_WARRIOR:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        if (m_weightScales[spec].info.name == "arms" || m_weightScales[spec].info.name == "fury")
        {
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
        }
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE };
        break;
    }
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_SIGIL, ITEM_SUBCLASS_ARMOR_PLATE };
        break;
    }
#endif
    case CLASS_PALADIN:
    {
        if (m_weightScales[spec].info.name != "holy" && proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        if (m_weightScales[spec].info.name != "holy")
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE , ITEM_SUBCLASS_ARMOR_LIBRAM };
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL, ITEM_SUBCLASS_ARMOR_PLATE, ITEM_SUBCLASS_ARMOR_LIBRAM };
        break;
    }
    case CLASS_HUNTER:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        break;
    }
    case CLASS_ROGUE:
    {
        if (proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER };
        break;
    }
    case CLASS_PRIEST:
    {
        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };
        break;
    }
    case CLASS_SHAMAN:
    {
        if (m_weightScales[spec].info.name == "enhance" && proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

#ifdef MANGOSBOT_ZERO
        if (m_weightScales[spec].info.name == "enhance")
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        break;
#else
        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_TOTEM, ITEM_SUBCLASS_ARMOR_LEATHER, ITEM_SUBCLASS_ARMOR_MAIL };
        break;
#endif
    }
    case CLASS_MAGE:
    case CLASS_WARLOCK:
    {
        resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_CLOTH };
        break;
    }
    case CLASS_DRUID:
    {
        if ((m_weightScales[spec].info.name == "feraltank" || m_weightScales[spec].info.name == "feraldps") && proto->InventoryType == INVTYPE_HOLDABLE)
            return false;

        if (m_weightScales[spec].info.name == "feraltank" || m_weightScales[spec].info.name == "feraldps")
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_IDOL, ITEM_SUBCLASS_ARMOR_LEATHER };
        else
            resultArmorSubClass = { ITEM_SUBCLASS_ARMOR_IDOL, ITEM_SUBCLASS_ARMOR_CLOTH, ITEM_SUBCLASS_ARMOR_LEATHER };

        break;
    }
    }

    return resultArmorSubClass.find(proto->SubClass) != resultArmorSubClass.end();
}

bool RandomItemMgr::CanEquipArmor(uint8 clazz, uint8 spec, uint32 level, ItemPrototype const* proto)
{
    if (proto->InventoryType == INVTYPE_TABARD)
        return true;

    if ((clazz == CLASS_WARRIOR || clazz == CLASS_PALADIN || clazz == CLASS_SHAMAN)
            && proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD)
        return true;

    if ((clazz == CLASS_WARRIOR || (clazz == CLASS_PALADIN && spec != 4)) && level >= 40)
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_PLATE && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (((clazz == CLASS_WARRIOR || (clazz == CLASS_PALADIN && spec != 4)) && level < 40) ||
            ((clazz == CLASS_HUNTER || (clazz == CLASS_SHAMAN && spec != 21)) && level >= 40))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_MAIL && proto->InventoryType != INVTYPE_CLOAK)
        {
            if (spec == 22 && (proto->SubClass == ITEM_SUBCLASS_ARMOR_LEATHER || proto->SubClass == ITEM_SUBCLASS_ARMOR_CLOTH))
                return true;

            return false;
        }
    }

    if (((clazz == CLASS_HUNTER || clazz == CLASS_SHAMAN) && level < 40) ||
            (((clazz == CLASS_DRUID && !(spec == 29 || spec == 31)))  || clazz == CLASS_ROGUE))
    {
        if (proto->SubClass != ITEM_SUBCLASS_ARMOR_LEATHER && proto->InventoryType != INVTYPE_CLOAK)
            return false;
    }

    if (proto->Quality <= ITEM_QUALITY_NORMAL)
        return true;

    return true;

    //uint8 sp = 0, ap = 0, tank = 0;
    //for (int j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    //{
    //    // for ItemStatValue != 0
    //    if(!proto->ItemStat[j].ItemStatValue)
    //        continue;

    //    AddItemStats(proto->ItemStat[j].ItemStatType, sp, ap, tank);
    //}

    //return CheckItemStats(clazz, sp, ap, tank);
}

bool RandomItemMgr::ShouldEquipWeaponForSpec(uint8 playerclass, uint8 spec, ItemPrototype const* proto)
{
    EquipmentSlots slot_mh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_oh = EQUIPMENT_SLOT_START;
    EquipmentSlots slot_rh = EQUIPMENT_SLOT_START;
    for (map<EquipmentSlots, set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end(); ++i)
    {
        set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
        if (slots.find((InventoryType)proto->InventoryType) != slots.end())
        {
            if (i->first == EQUIPMENT_SLOT_MAINHAND)
                slot_mh = i->first;
            if (i->first == EQUIPMENT_SLOT_OFFHAND)
                slot_oh = i->first;
            if (i->first == EQUIPMENT_SLOT_RANGED)
                slot_rh = i->first;
        }
    }

    if (slot_mh == EQUIPMENT_SLOT_START && slot_oh == EQUIPMENT_SLOT_START && slot_rh == EQUIPMENT_SLOT_START)
        return false;

    if (!m_weightScales[spec].info.id)
        return false;

    std::unordered_set<uint32> mh_weapons;
    std::unordered_set<uint32> oh_weapons;
    std::unordered_set<uint32> r_weapons;

    switch (playerclass)
    {
    case CLASS_WARRIOR:
    {
        if (m_weightScales[spec].info.name == "prot")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        }
        else if (m_weightScales[spec].info.name == "arms")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_POLEARM };
            r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        }
        break;
    }
    case CLASS_PALADIN:
    {
        if (m_weightScales[spec].info.name == "prot")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_ARMOR_LIBRAM };
        }
        else if (m_weightScales[spec].info.name == "holy")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_SHIELD, ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_LIBRAM };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_POLEARM };
            r_weapons = { ITEM_SUBCLASS_ARMOR_LIBRAM };
        }
        break;
    }
    case CLASS_HUNTER:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_FIST, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_POLEARM, ITEM_SUBCLASS_WEAPON_STAFF };
        r_weapons = { ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        break;
    }
    case CLASS_ROGUE:
    {
        if (m_weightScales[spec].info.name == "assas")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER };
        }
        else if (m_weightScales[spec].info.name == "combat")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
        }

        r_weapons = { ITEM_SUBCLASS_WEAPON_THROWN, ITEM_SUBCLASS_WEAPON_BOW, ITEM_SUBCLASS_WEAPON_CROSSBOW, ITEM_SUBCLASS_WEAPON_GUN };
        break;
    }
    case CLASS_PRIEST:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE };
        oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
        r_weapons = { ITEM_SUBCLASS_WEAPON_WAND };
        break;
    }
    case CLASS_SHAMAN:
    {
        if (m_weightScales[spec].info.name == "resto")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC, ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_ARMOR_TOTEM };
        }
        else if (m_weightScales[spec].info.name == "enhance")
        {
#ifdef MANGOSBOT_ZERO
            mh_weapons = { ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_AXE2 };
#else
            mh_weapons = { ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_FIST };
#endif
            r_weapons = { ITEM_SUBCLASS_ARMOR_TOTEM };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC, ITEM_SUBCLASS_ARMOR_SHIELD };
            r_weapons = { ITEM_SUBCLASS_ARMOR_TOTEM };
        }
        break;
    }
    case CLASS_MAGE:
    case CLASS_WARLOCK:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_SWORD };
        oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
        r_weapons = { ITEM_SUBCLASS_WEAPON_WAND };
        break;
    }
    case CLASS_DRUID:
    {
        if (m_weightScales[spec].info.name == "feraltank")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        else if (m_weightScales[spec].info.name == "resto")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_MACE2 };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        else if (m_weightScales[spec].info.name == "feraldps")
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2, ITEM_SUBCLASS_WEAPON_MACE };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        else
        {
            mh_weapons = { ITEM_SUBCLASS_WEAPON_DAGGER, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_STAFF, ITEM_SUBCLASS_WEAPON_MACE2 };
            oh_weapons = { ITEM_SUBCLASS_ARMOR_MISC };
            r_weapons = { ITEM_SUBCLASS_ARMOR_IDOL };
        }
        break;
    }
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
    {
        mh_weapons = { ITEM_SUBCLASS_WEAPON_SWORD, ITEM_SUBCLASS_WEAPON_SWORD2, ITEM_SUBCLASS_WEAPON_AXE, ITEM_SUBCLASS_WEAPON_AXE2, ITEM_SUBCLASS_WEAPON_MACE, ITEM_SUBCLASS_WEAPON_MACE2 };
        r_weapons = { ITEM_SUBCLASS_ARMOR_SIGIL };
        break;
    }
#endif
    }

    if (slot_mh == EQUIPMENT_SLOT_MAINHAND)
    {
        return mh_weapons.find(proto->SubClass) != mh_weapons.end();
    }
    if (slot_oh == EQUIPMENT_SLOT_OFFHAND)
    {
        return oh_weapons.find(proto->SubClass) != oh_weapons.end();
    }
    if (slot_rh == EQUIPMENT_SLOT_RANGED)
    {
        return r_weapons.find(proto->SubClass) != r_weapons.end();
    }

    return false;
}

bool RandomItemMgr::CanEquipWeapon(uint8 clazz, ItemPrototype const* proto)
{
    switch (clazz)
    {
    case CLASS_PRIEST:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE)
            return false;
        break;
    case CLASS_MAGE:
    case CLASS_WARLOCK:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_WAND &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
            return false;
        break;
    case CLASS_WARRIOR:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
        break;
    case CLASS_PALADIN:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD)
            return false;
        break;
    case CLASS_SHAMAN:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
        break;
    case CLASS_DRUID:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_STAFF)
            return false;
        break;
    case CLASS_HUNTER:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW)
            return false;
        break;
    case CLASS_ROGUE:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_DAGGER &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_GUN &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_CROSSBOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_BOW &&
                proto->SubClass != ITEM_SUBCLASS_WEAPON_THROWN)
            return false;
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        if (proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD2 &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_MACE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_SWORD &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE &&
            proto->SubClass != ITEM_SUBCLASS_WEAPON_AXE2)
            return false;
        break;
#endif
    }

    return true;
}

void RandomItemMgr::BuildItemInfoCache()
{
    uint32 maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    for (uint32 i = 0; i <= MAX_STAT_SCALES; ++i)
    {
        WeightScale scale;
        scale.info.id = 0;
        scale.info.name = "";
        scale.info.classId = 0;
        m_weightScales[i] = scale;
    }

    // load weightscales
    sLog.outString("Loading weightscales info");
    auto results = PlayerbotDatabase.PQuery("select id, name, class from ai_playerbot_weightscales");

    if (results)
    {
        int totalcount = 0;
        int statcount = 0;
        int curClass = CLASS_WARRIOR;

        do
        {
            Field* fields = results->Fetch();
            uint32 id = fields[0].GetUInt32();
            string name = fields[1].GetString();
            uint32 clazz = fields[2].GetUInt32();

            WeightScale scale;
            scale.info.id = id;
            scale.info.name = name;
            scale.info.classId = clazz;
            m_weightScales[id] = scale;
            totalcount++;

        } while (results->NextRow());

        sLog.outString("Loaded %d weightscale class specs", totalcount);

        auto result = PlayerbotDatabase.PQuery("select id, field, val from ai_playerbot_weightscale_data");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 id = fields[0].GetUInt32();
                string field = fields[1].GetString();
                uint32 weight = fields[2].GetUInt32();

                WeightScaleStat stat;
                stat.stat = field;
                stat.weight = weight;

                m_weightScales[id].stats.push_back(stat);
                statcount++;

            } while (result->NextRow());
        }

        sLog.outString("Loaded %d weightscale stat weights", statcount);
    }

    if (m_weightScales[1].stats.empty())
    {
        sLog.outError("Error loading item weight scales");
        return;
    }

    // vendor items
    sLog.outString("Loading vendor item list...");
    std::vector<uint32> vendorItems;
    std::vector<uint32> allianceItems;
    std::vector<uint32> hordeItems;
    vendorItems.clear();
    if (auto result = WorldDatabase.PQuery("%s", "SELECT item, entry FROM npc_vendor"))
    {
        BarGoLink bar(result->GetRowCount());
        do
        {
            bar.step();
            Field* fields = result->Fetch();
            uint32 entry = fields[0].GetUInt32();
            if (!entry)
                continue;
            vendorItems.push_back(fields[0].GetUInt32());

#ifdef MANGOSBOT_ZERO
            uint32 vendorId = fields[1].GetUInt32();
            if (vendorId)
            {
                if (vendorId == 12782 || vendorId == 12777 || vendorId == 12785)
                    allianceItems.push_back(entry);
                if (vendorId == 14581 || vendorId == 12792 || vendorId == 12794)
                    hordeItems.push_back(entry);
            }
#endif
        } while (result->NextRow());
    }
    sLog.outString("Loaded %d vendor items...", vendorItems.size());
    sLog.outString("Loaded %d alliance only vendor items...", allianceItems.size());
    sLog.outString("Loaded %d horde only vendor items...", hordeItems.size());

    // calculate drop source
    sLog.outString("Loading loot templates...");
    DropMap* dropMap = new DropMap;

    int32 sEntry;

    for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); entry++)
    {
        sEntry = entry;

        LootTemplateAccess const* lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_UNIT, entry, uint32(1)), LOOT_CORPSE);

        if (lTemplateA)
            for (LootStoreItem const& lItem : lTemplateA->Entries)
                dropMap->insert(make_pair(lItem.itemid, sEntry));
    }

    for (uint32 entry = 0; entry < sGOStorage.GetMaxEntry(); entry++)
    {
        sEntry = entry;

        LootTemplateAccess const* lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_GAMEOBJECT, entry, uint32(1)), LOOT_CORPSE);

        if (lTemplateA)
            for (LootStoreItem const& lItem : lTemplateA->Entries)
                dropMap->insert(make_pair(lItem.itemid, -sEntry));
    }

    sLog.outString("Loaded %d loot templates...", dropMap->size());

    sLog.outString("Calculating stat weights for %d items...", sItemStorage.GetMaxEntry());
    BarGoLink bar(sItemStorage.GetMaxEntry());

    // generate stat weights for classes/specs
    for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
    {
        bar.step();

        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (!proto)
            continue;

        // skip non armor/weapon
        if (proto->Class != ITEM_CLASS_WEAPON &&
            proto->Class != ITEM_CLASS_ARMOR &&
            proto->Class != ITEM_CLASS_CONTAINER &&
            proto->Class != ITEM_CLASS_PROJECTILE)
            continue;

        if (!CanEquipItemNew(proto))
            continue;

        // skip test items
        if (strstr(proto->Name1, "(Test)") ||
            strstr(proto->Name1, "(TEST)") ||
            strstr(proto->Name1, "(test)") ||
            strstr(proto->Name1, "(JEFFTEST)") ||
            strstr(proto->Name1, "Test ") ||
            strstr(proto->Name1, "Test") ||
            strstr(proto->Name1, "TEST") ||
            strstr(proto->Name1, "TEST ") ||
            strstr(proto->Name1, " TEST") ||
            strstr(proto->Name1, "2200 ") ||
            strstr(proto->Name1, "Deprecated ") ||
            strstr(proto->Name1, "Unused ") ||
            strstr(proto->Name1, "Monster ") ||
            strstr(proto->Name1, "[PH]") ||
            strstr(proto->Name1, "(OLD)")
            )
            continue;

        // skip items with rank/rep requirements
        /*if (proto->RequiredHonorRank > 0 ||
            proto->RequiredSkillRank > 0 ||
            proto->RequiredCityRank > 0 ||
            proto->RequiredReputationRank > 0)
            continue;*/

        /*if (proto->RequiredHonorRank > 0 ||
            proto->RequiredSkillRank > 0 ||
            proto->RequiredCityRank > 0)
            continue;*/

        // skip random enchant items
        if (proto->RandomProperty)
            continue;

#ifndef MANGOSBOT_ZERO
        if (proto->RandomSuffix)
            continue;
#endif

#ifdef MANGOSBOT_TWO
        // skip heirloom items
        if (proto->Quality == ITEM_QUALITY_HEIRLOOM)
            continue;
#endif

        // check possible equip slots
        EquipmentSlots slot = EQUIPMENT_SLOT_END;
        for (map<EquipmentSlots, set<InventoryType> >::iterator i = viableSlots.begin(); i != viableSlots.end(); ++i)
        {
            set<InventoryType> slots = viableSlots[(EquipmentSlots)i->first];
            if (slots.find((InventoryType)proto->InventoryType) != slots.end())
                slot = i->first;
        }

        if (slot == EQUIPMENT_SLOT_END)
            continue;

        // Init Item cache
        ItemInfoEntry* cacheInfo = new ItemInfoEntry;
        uint32 itemSpec = ITEM_SPEC_NONE;

#ifdef MANGOSBOT_TWO
        // check faction
        if (proto->Flags2 & ITEM_FLAG2_FACTION_HORDE)
            cacheInfo->team = HORDE;

        if (proto->Flags2 & ITEM_FLAG2_FACTION_ALLIANCE)
            cacheInfo->team = ALLIANCE;

        if (!cacheInfo->team && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry(HORDE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = HORDE;

            if (FactionEntry const* faction = sFactionStore.LookupEntry(ALLIANCE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = ALLIANCE;
        }
#endif
#ifdef MANGOSBOT_ONE
        // check faction
        if (!cacheInfo->team && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(HORDE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = HORDE;

            if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(ALLIANCE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = ALLIANCE;
        }
#endif
#ifdef MANGOSBOT_ZERO
        // check faction
        if (!cacheInfo->team && proto->AllowableRace > 1 && proto->AllowableRace < 8388607)
        {
            if (FactionEntry const* faction = sFactionStore.LookupEntry(HORDE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = HORDE;

            if (FactionEntry const* faction = sFactionStore.LookupEntry(ALLIANCE))
                if ((proto->AllowableRace & faction->BaseRepRaceMask[0]) != 0)
                    cacheInfo->team = ALLIANCE;
        }

        // PvP vendors
        if (std::find(allianceItems.begin(), allianceItems.end(), proto->ItemId) != allianceItems.end())
            cacheInfo->team = ALLIANCE;
        if (std::find(hordeItems.begin(), hordeItems.end(), proto->ItemId) != hordeItems.end())
            cacheInfo->team = HORDE;
#endif

        if (cacheInfo->team)
            sLog.outDetail("Item: %d, team (item): %s", proto->ItemId, cacheInfo->team == ALLIANCE ? "Alliance" : "Horde");

        // check min level
        if (proto->RequiredLevel)
            cacheInfo->minLevel = proto->RequiredLevel;

        // check item source

        if (proto->Flags & ITEM_FLAG_NO_DISENCHANT)
        {
            cacheInfo->source = ITEM_SOURCE_PVP;
            sLog.outDetail("Item: %d, source: PvP Reward", proto->ItemId);
        }

        // check quests
        if (cacheInfo->source == ITEM_SOURCE_NONE || cacheInfo->source == ITEM_SOURCE_PVP)
        {
            vector<uint32> questIds = GetQuestIdsForItem(proto->ItemId);
            if (questIds.size())
            {
                bool isAlly = false;
                bool isHorde = false;
                for (vector<uint32>::iterator i = questIds.begin(); i != questIds.end(); ++i)
                {
                    Quest const* quest = sObjectMgr.GetQuestTemplate(*i);
                    if (quest)
                    {
                        cacheInfo->source = ITEM_SOURCE_QUEST;
                        cacheInfo->sourceId = *i;
                        if (!cacheInfo->minLevel)
                            cacheInfo->minLevel = quest->GetMinLevel();

                        // check quest team
                        if (!cacheInfo->team)
                        {
                            uint32 reqRace = quest->GetRequiredRaces();
                            if (reqRace)
                            {
                                if ((reqRace & RACEMASK_ALLIANCE) != 0)
                                    isAlly = true;
                                else if ((reqRace & RACEMASK_HORDE) != 0)
                                    isHorde = true;
                            }
                        }

                        if (quest->GetRequiredMinRepFaction())
                        {
                            cacheInfo->repFaction = quest->GetRequiredMinRepFaction();
                            int r = 0;
                            for (; r < MAX_REPUTATION_RANK; ++r)
                            {
                                if (quest->GetRequiredMinRepValue() == ReputationMgr::PointsInRank[r])
                                    cacheInfo->repRank = uint32(r);
                            }
#ifdef MANGOSBOT_ONE
                            if (FactionEntry const* faction = sFactionStore.LookupEntry<FactionEntry>(quest->GetRequiredMinRepFaction()))
#else
                            if (FactionEntry const* faction = sFactionStore.LookupEntry(quest->GetRequiredMinRepFaction()))
#endif
                            {
                                if (faction->team == ALLIANCE)
                                    cacheInfo->team = ALLIANCE;
                                if (faction->team == HORDE)
                                    cacheInfo->team = HORDE;
                            }
                        }
                    }
                }

                if (!cacheInfo->team)
                {
                    if (isAlly && isHorde)
                        cacheInfo->team = TEAM_BOTH_ALLOWED;
                    else if (isAlly)
                        cacheInfo->team = ALLIANCE;
                    else if (isHorde)
                        cacheInfo->team = HORDE;
                }

                sLog.outDetail("Item: %d, team (quest): %s", proto->ItemId, cacheInfo->team == ALLIANCE ? "Alliance" : cacheInfo->team == HORDE ? "Horde" : "Both");
                sLog.outDetail("Item: %d, source: quest %d, minlevel: %d", proto->ItemId, cacheInfo->sourceId, cacheInfo->minLevel);
            }
        }

        if (cacheInfo->minLevel)
            sLog.outDetail("Item: %d, minlevel: %d", proto->ItemId, cacheInfo->minLevel);

        // check vendors
        if (cacheInfo->source == ITEM_SOURCE_NONE)
        {
            for (vector<uint32>::iterator i = vendorItems.begin(); i != vendorItems.end(); ++i)
            {
                if (proto->ItemId == *i)
                {
                    cacheInfo->source = ITEM_SOURCE_VENDOR;
                    sLog.outDetail("Item: %d, source: vendor", proto->ItemId);
                    break;
                }
            }
        }

        // check drops
        list<int32> creatures;
        list<int32> gameobjects;

        auto range = dropMap->equal_range(itemId);

        for (auto itr = range.first; itr != range.second; ++itr)
        {
            if (itr->second > 0)
                creatures.push_back(itr->second);
            else
                gameobjects.push_back(abs(itr->second));
        }
        
        // check creature drop
        if (cacheInfo->source == ITEM_SOURCE_NONE)
        {
            if (creatures.size())
            {
                if (creatures.size() == 1)
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    cacheInfo->sourceId = creatures.front();
                    sLog.outDetail("Item: %d, source: creature drop, ID: %d", proto->ItemId, creatures.front());
                }
                else
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    sLog.outDetail("Item: %d, source: creatures drop, number: %d", proto->ItemId, creatures.size());
                }
            }
        }

        // check gameobject drop
        if (cacheInfo->source == ITEM_SOURCE_NONE || (cacheInfo->source == ITEM_SOURCE_DROP && !cacheInfo->sourceId))
        {
            if (gameobjects.size())
            {
                if (gameobjects.size() == 1)
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    cacheInfo->sourceId = gameobjects.front();
                    sLog.outDetail("Item: %d, source: gameobject, ID: %d", proto->ItemId, gameobjects.front());
                }
                else
                {
                    cacheInfo->source = ITEM_SOURCE_DROP;
                    sLog.outDetail("Item: %d, source: gameobjects, number: %d", proto->ItemId, gameobjects.size());
                }
            }
        }

        // check faction
        if (proto->RequiredReputationFaction > 0 && proto->RequiredReputationFaction != 35 && proto->RequiredReputationRank < 15)
        {
            cacheInfo->repFaction = proto->RequiredReputationFaction;
            cacheInfo->repRank = proto->RequiredReputationRank;
        }

        // check honor rank
        if (proto->RequiredHonorRank > 0)
        {
            cacheInfo->pvpRank = proto->RequiredHonorRank;
        }

        // check skill
        if (proto->RequiredSkill > 0)
        {
            cacheInfo->reqSkill = proto->RequiredSkill;
            cacheInfo->reqSkillRank = proto->RequiredSkillRank;
        }

        cacheInfo->quality = proto->Quality;
        cacheInfo->itemId = proto->ItemId;
        cacheInfo->slot = slot;
        cacheInfo->itemLevel = proto->ItemLevel;

        // calculate stat weights
        for (uint8 clazz = CLASS_WARRIOR; clazz < MAX_CLASSES; ++clazz)
        {
            // skip nonexistent classes
            if (!((1 << (clazz - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(clazz))
                continue;

            // skip wrong classes
            if ((proto->AllowableClass & (1 << (clazz - 1))) == 0)
                continue;

            for (uint32 spec = 1; spec <= MAX_STAT_SCALES; ++spec)
            {
                if (!m_weightScales[spec].info.id)
                    continue;

                if (m_weightScales[spec].info.classId != clazz)
                    continue;

                // check possible armor for spec
                if (proto->Class == ITEM_CLASS_ARMOR && (
                    slot == EQUIPMENT_SLOT_HEAD ||
                    slot == EQUIPMENT_SLOT_SHOULDERS ||
                    slot == EQUIPMENT_SLOT_CHEST ||
                    slot == EQUIPMENT_SLOT_WAIST ||
                    slot == EQUIPMENT_SLOT_LEGS ||
                    slot == EQUIPMENT_SLOT_FEET ||
                    slot == EQUIPMENT_SLOT_WRISTS ||
                    slot == EQUIPMENT_SLOT_HANDS) &&
                    !ShouldEquipArmorForSpec(clazz, spec, proto))
                    continue;

                // check possible weapon for spec
                if ((proto->Class == ITEM_CLASS_WEAPON || (proto->SubClass == ITEM_SUBCLASS_ARMOR_SHIELD || (proto->SubClass == ITEM_SUBCLASS_ARMOR_MISC && proto->InventoryType == INVTYPE_HOLDABLE))) &&
                    !ShouldEquipWeaponForSpec(clazz, spec, proto))
                    continue;

                //StatWeight statWeight;
                //statWeight.id = m_weightScales[spec].info.id;
                ItemSpecType tempSpec = ITEM_SPEC_NONE;
                uint32 statW = CalculateStatWeight(clazz, spec, proto, tempSpec);
                itemSpec |= tempSpec;
                if (!statW /*&& tempSpec == ITEM_SPEC_NONE*/ && cacheInfo->quality < ITEM_QUALITY_UNCOMMON && cacheInfo->minLevel <= 10)
                    statW = 1;

                // legendary stat x2
                if (proto->Quality >= ITEM_QUALITY_LEGENDARY && statW > 1)
                    statW *= 2;

                if (slot == EQUIPMENT_SLOT_BODY && statW <= 0)
                    statW = 1;

                // warriors only plate >= 40 lvl
                if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && cacheInfo->minLevel >= 40 && clazz == CLASS_WARRIOR)
                    statW = 0;

                // paladin tank/dps only plate >= 40 lvl
                if (proto->SubClass == ITEM_SUBCLASS_ARMOR_MAIL && cacheInfo->minLevel >= 40 && clazz == CLASS_PALADIN && spec != 4)
                    statW = 0;

                // some trinkets have no stats
                if (cacheInfo->slot == EQUIPMENT_SLOT_TRINKET1 ||
                    cacheInfo->slot == EQUIPMENT_SLOT_TRINKET2)
                {
                    if (statW == 0 && proto->AllowableClass == uint32(clazz) && proto->Spells[0].SpellId)
                    {
                        statW = (uint32)(proto->Quality + proto->ItemLevel);
                    }
                }

                // set stat weight = 1 for items that can be equipped but have no proper stats
                //statWeight.weight = statW;
                // save item statWeight into ItemCache
                cacheInfo->weights[spec] = statW;
                sLog.outDetail("Item: %d, weight: %d, class: %d, spec: %s", proto->ItemId, statW, clazz, m_weightScales[spec].info.name.c_str());
            }
        }

        cacheInfo->itemSpec = (ItemSpecType)itemSpec;

        // save cache
        static SqlStatementID delCache;
        static SqlStatementID insertCache;

        SqlStatement stmt = PlayerbotDatabase.CreateStatement(delCache, "DELETE FROM ai_playerbot_item_info_cache WHERE id = ?");
        stmt.PExecute(proto->ItemId);

        stmt = PlayerbotDatabase.CreateStatement(insertCache, "INSERT INTO ai_playerbot_item_info_cache (id, quality, slot, source, sourceId, team, faction, factionRepRank, minLevel, "
            "scale_1, scale_2, scale_3, scale_4, scale_5, scale_6, scale_7, scale_8, scale_9, scale_10, scale_11, scale_12, scale_13, scale_14, scale_15, "
            "scale_16, scale_17, scale_18, scale_19, scale_20, scale_21, scale_22, scale_23, scale_24, scale_25, scale_26, scale_27, scale_28, scale_29, scale_30, scale_31, scale_32)"
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

        stmt.addUInt32(cacheInfo->itemId);
        stmt.addUInt32(cacheInfo->quality);
        stmt.addUInt32(cacheInfo->slot);
        stmt.addUInt32(cacheInfo->source);
        stmt.addUInt32(cacheInfo->sourceId);
        stmt.addUInt32(cacheInfo->team);
        stmt.addUInt32(cacheInfo->repFaction);
        stmt.addUInt32(cacheInfo->repRank);
        stmt.addUInt32(cacheInfo->minLevel);

        for (int i = 1; i <= MAX_STAT_SCALES; ++i)
        {
            if (cacheInfo->weights[i])
                stmt.addUInt32(cacheInfo->weights[i]);
            else
                stmt.addUInt32(0);
        }

        stmt.Execute();

        itemInfoCache[cacheInfo->itemId] = cacheInfo;
    }
}

uint32 RandomItemMgr::CalculateStatWeight(uint8 playerclass, uint8 spec, ItemPrototype const* proto, ItemSpecType& itSpec)
{
    uint32 specType = ITEM_SPEC_NONE;
    uint32 statWeight = 0;
    uint32 spellPower = 0;
    uint32 spellHeal = 0;
    uint32 attackPower = 0;
    bool isCasterItem = false;
    bool isAttackItem = false;
    bool isDpsItem = false;
    bool isTankItem = false;
    bool isHealingItem = false;
    bool isSpellDamageItem = false;
    bool hasInt = false;
#ifdef MANGOSBOT_TWO
    bool noCaster = (Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE || (Classes)playerclass == CLASS_DEATH_KNIGHT || (Classes)playerclass == CLASS_HUNTER || spec == 30 || spec == 32 || spec == 21 || spec == 6;
    bool hasMana = !((Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE || (Classes)playerclass == CLASS_DEATH_KNIGHT);

    if (!proto->IsWeapon() && (proto->SubClass == ITEM_SUBCLASS_ARMOR_LIBRAM || proto->SubClass == ITEM_SUBCLASS_ARMOR_IDOL || proto->SubClass == ITEM_SUBCLASS_ARMOR_TOTEM || proto->SubClass == ITEM_SUBCLASS_ARMOR_SIGIL))
        return (uint32)(proto->Quality + proto->ItemLevel);
#else
    bool noCaster = (Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE || (Classes)playerclass == CLASS_HUNTER || spec == 30 || spec == 32 || spec == 21 || spec == 6;
    bool hasMana = !((Classes)playerclass == CLASS_WARRIOR || (Classes)playerclass == CLASS_ROGUE);
    
    if ((Classes)playerclass == CLASS_HUNTER && proto->SubClass == ITEM_SUBCLASS_WEAPON_THROWN)
        return (uint32)proto->ItemLevel;
#endif

    //check relicts
    if (proto->InventoryType == INVTYPE_RELIC)
    {
        if (playerclass == CLASS_PALADIN && proto->SubClass != ITEM_SUBCLASS_ARMOR_LIBRAM)
            return 0;

        if (playerclass == CLASS_DRUID && proto->SubClass != ITEM_SUBCLASS_ARMOR_IDOL)
            return 0;

        if (playerclass == CLASS_SHAMAN && proto->SubClass != ITEM_SUBCLASS_ARMOR_TOTEM)
            return 0;

        if (playerclass == CLASS_WARRIOR
            || playerclass == CLASS_HUNTER
            || playerclass == CLASS_ROGUE
            || playerclass == CLASS_PRIEST
            || playerclass == CLASS_MAGE
            || playerclass == CLASS_WARLOCK)
            return 0;

        return proto->Quality + proto->ItemLevel;
    }

    bool isWhitelist = false;
    // whitelist
    if (std::find(sPlayerbotAIConfig.randomGearWhitelist.begin(), sPlayerbotAIConfig.randomGearWhitelist.end(), proto->ItemId) != sPlayerbotAIConfig.randomGearWhitelist.end())
        isWhitelist = true;

#ifdef MANGOSBOT_ZERO
    // whitelist pvp items, as thei have wierd stats
    if (proto->RequiredHonorRank)
        isWhitelist = true;

    // whitelist the ONLY feral Off Hand in vanilla
    if ((spec == 30 || spec == 32) && proto->ItemId == 13385)
        isWhitelist = true;
#endif

    // whitelist atiesh
    if (playerclass == CLASS_MAGE && proto->ItemId == 22589)
        isWhitelist = true;
    if (playerclass == CLASS_WARLOCK && proto->ItemId == 22630)
        isWhitelist = true;
    if (playerclass == CLASS_PRIEST && proto->ItemId == 22631)
        isWhitelist = true;
    if (playerclass == CLASS_DRUID && proto->ItemId == 22632)
        isWhitelist = true;

    // check basic item stats
    int32 basicStatsWeight = 0;
    for (int j = 0; j < MAX_ITEM_PROTO_STATS; ++j)
    {
        uint32 statType = 0;
        int32 val = 0;
        string weightName = "";

        //if (j >= proto->StatsCount)
        //    continue;

        statType = proto->ItemStat[j].ItemStatType;
        val = proto->ItemStat[j].ItemStatValue;

        if (val == 0)
            continue;

        for (map<string, uint32 >::iterator i = weightStatLink.begin(); i != weightStatLink.end(); ++i)
        {
            uint32 modd = i->second;
            if (modd == statType)
            {
                weightName = i->first;

#ifndef MANGOSBOT_ZERO
                // mark tank item (skip items with def stats for dps)
                if (modd == ITEM_MOD_DODGE_RATING || modd == ITEM_MOD_PARRY_RATING || modd == ITEM_MOD_BLOCK_RATING || modd == ITEM_MOD_DEFENSE_SKILL_RATING)
                    isTankItem = true;

                if (modd == ITEM_MOD_CRIT_MELEE_RATING || modd == ITEM_MOD_CRIT_RATING)
                    isDpsItem = true;

#ifdef MANGOSBOT_TWO
                if (modd == ITEM_MOD_SPELL_POWER)
                {
                    isCasterItem = true;
                    isHealingItem = true;
                    isSpellDamageItem = true;
                }
#endif
#endif

                break;
            }
        }

        if (weightName.empty())
            continue;

        uint32 singleStat = CalculateSingleStatWeight(playerclass, spec, weightName, val);
        basicStatsWeight += singleStat;

        if (val)
        {
            if (weightName == "int" && !noCaster)
                isCasterItem = true;

            if (weightName == "int")
                hasInt = true;

            if (weightName == "splpwr")
                isCasterItem = true;

            if (weightName == "str")
                isAttackItem = true;

            if (weightName == "agi")
                isAttackItem = true;

            if (weightName == "atkpwr")
                isAttackItem = true;
        }
    }

    // check defensive stats
    uint32 defenseStats = 0;
    defenseStats += CalculateSingleStatWeight(playerclass, spec, "block", proto->Block);
    defenseStats += CalculateSingleStatWeight(playerclass, spec, "armor", proto->Armor);

    // check weapon dps
    if (proto->IsWeapon())
    {
        WeaponAttackType attType = BASE_ATTACK;

        uint32 dps = 0;

        for (int i = 0; i < MAX_ITEM_PROTO_DAMAGES; i++)
        {
            if (proto->Damage[i].DamageMax == 0)
                break;

            dps = (proto->Damage[i].DamageMin + proto->Damage[i].DamageMax) / (float)(proto->Delay / 1000.0f) / 2;
            if (dps)
            {
                if (proto->IsRangedWeapon())
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "rgddps", dps);
                else
                    statWeight += CalculateSingleStatWeight(playerclass, spec, "mledps", dps);
            }
        }
    }

    // check item spells
    uint32 spellDamage = 0;
    uint32 spellHealing = 0;
    uint32 auraStatWeight = 0;
    uint32 auraApStatWeight = 0;
    uint32 auraHealStatWeight = 0;
    uint32 auraDamageStatWeight = 0;
    bool isFeral = false;
    for (const auto& spellData : proto->Spells)
    {
        // no spell
        if (!spellData.SpellId)
            continue;
        
        // apply only at-equip spells for weapons, on use/hit for armor
        if (!(spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_EQUIP || (!proto->IsWeapon() && proto->InventoryType != INVTYPE_HOLDABLE && (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_USE || spellData.SpellTrigger == ITEM_SPELLTRIGGER_CHANCE_ON_HIT))))
            continue;

        // check if it is valid spell
        SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
        if (!spellproto)
            continue;

        bool hasAP = false;

        uint32 effectAuraStatWeight = 0;
        uint32 effectAuraApStatWeight = 0;
        uint32 effectAuraHealStatWeight = 0;
        uint32 effectAuraDamageStatWeight = 0;

        for (uint32 j = 0; j < MAX_EFFECT_INDEX; j++)
        {
            if ((spellproto->Effect[j] == SPELL_EFFECT_APPLY_AURA) &&
                (spellproto->EffectBasePoints[j] >= 0))
            {
#ifdef MANGOSBOT_TWO
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    isSpellDamageItem = true;
                    isCasterItem = true;
                    spellDamage = spellproto->EffectBasePoints[j] + 1;

                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        isHealingItem = true;
                        effectAuraDamageStatWeight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        if (!isWhitelist && !specialDamage && isSpellDamageItem)
                            return 0;

                        effectAuraDamageStatWeight += specialDamage;
                    }
                }
#else
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    spellDamage = spellproto->EffectBasePoints[j] + 1;
                    isSpellDamageItem = true;
                    // generic spell damage
                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        isSpellDamageItem = true;
                        effectAuraDamageStatWeight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellproto->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        if (!isWhitelist && !specialDamage && isSpellDamageItem)
                            return 0;

                        effectAuraDamageStatWeight += specialDamage;
                    }
                }
                // spell healing
                // SPELL_AURA_MOD_HEALING_DONE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE)
                {
                    isHealingItem = true;
                    spellHealing = spellproto->EffectBasePoints[j] + 1;
                    effectAuraHealStatWeight += CalculateSingleStatWeight(playerclass, spec, "splheal", spellproto->EffectBasePoints[j] + 1);
                }
#endif

                // spell hit rating (pre tbc)
                // SPELL_AURA_MOD_SPELL_HIT_CHANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SPELL_HIT_CHANCE)
                {
                    isCasterItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "spellhitrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // spell crit rating (pre tbc)
                // SPELL_AURA_MOD_SPELL_CRIT_CHANCE, SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SPELL_CRIT_CHANCE || spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SPELL_CRIT_CHANCE_SCHOOL)
                {
                    isCasterItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "spellcritstrkrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // spell penetration
                // SPELL_AURA_MOD_TARGET_RESISTANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_TARGET_RESISTANCE)
                {
                    // check if magic type
                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_SPELL)
                        effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "spellpenrtng", abs(spellproto->EffectBasePoints[j] + 1));
                }

                // check attack power
                if (!hasAP && spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_ATTACK_POWER)
                {
                    hasAP = true;
                    isAttackItem = true;
                    string SpellName = spellproto->SpellName[0];
                    if (SpellName.find("Attack Power - Feral") != string::npos)
                        isFeral = true;
#ifdef MANGOSBOT_ZERO
                    if (!isWhitelist && isFeral && (playerclass != CLASS_DRUID && playerclass != CLASS_WARRIOR && playerclass != CLASS_PALADIN && proto->IsWeapon()))
                        return 0;
#else
                    if (!isWhitelist && isFeral && playerclass != CLASS_DRUID)
                        return 0;
#endif

                    effectAuraApStatWeight += CalculateSingleStatWeight(playerclass, spec, isFeral ? "feratkpwr" : "atkpwr", spellproto->EffectBasePoints[j] + 1);
                }

                // check ranged ap
                // SPELL_AURA_MOD_RANGED_ATTACK_POWER
                if (!hasAP && spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_RANGED_ATTACK_POWER)
                {
                    // filter non ranged classes
                    if (playerclass == CLASS_SHAMAN || (!proto->IsRangedWeapon() && playerclass != CLASS_HUNTER))
                        return 0;

                    hasAP = true;
                    isAttackItem = true;
                    effectAuraApStatWeight += CalculateSingleStatWeight(playerclass, spec, "atkpwr", spellproto->EffectBasePoints[j] + 1);
                }

                // check block
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SHIELD_BLOCKVALUE)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "block", spellproto->EffectBasePoints[j] + 1);
                }

                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_PARRY_PERCENT)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "parryrtng", spellproto->EffectBasePoints[j] + 1);
                }

                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_DODGE_PERCENT)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "dodgertng", spellproto->EffectBasePoints[j] + 1);
                }

                // block chance
                // SPELL_AURA_MOD_BLOCK_PERCENT
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_BLOCK_PERCENT)
                {
                    isTankItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "blockrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // armor penetration
                // SPELL_AURA_MOD_TARGET_RESISTANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_TARGET_RESISTANCE)
                {
                    // check if physical type
                    if (spellproto->EffectMiscValue[j] == SPELL_SCHOOL_MASK_NORMAL)
                        effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "armorpenrtng", abs(spellproto->EffectBasePoints[j] + 1));
                }

                // hit rating (pre tbc)
                // SPELL_AURA_MOD_HIT_CHANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_HIT_CHANCE)
                {
                    isAttackItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "hitrtng", spellproto->EffectBasePoints[j] + 1);
                }

                // crit rating (pre tbc)
                // SPELL_AURA_MOD_HIT_CHANCE
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_CRIT_PERCENT)
                {
                    isAttackItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "critstrkrtng", spellproto->EffectBasePoints[j] + 1);
                }

                //check defense SPELL_AURA_MOD_SKILL
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_SKILL)
                {
                    if (spellproto->EffectMiscValue[j] == SKILL_DEFENSE)
                    {
                        isTankItem = true;
                        effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "defrtng", spellproto->EffectBasePoints[j] + 1);
                    }
                }

#ifndef MANGOSBOT_ZERO
                // ratings
                // enum CombatRating
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_RATING)
                {
                    for (uint32 rating = 0; rating < MAX_COMBAT_RATING; ++rating)
                    {
                        if (spellproto->EffectMiscValue[0] & (1 << rating))
                        {
                            int32 val = spellproto->EffectBasePoints[j] + 1;
                            string weightName;

                            for (map<string, uint32 >::iterator i = weightRatingLink.begin(); i != weightRatingLink.end(); ++i)
                            {
                                uint32 modd = i->second;
                                if (modd == rating)
                                {
                                    weightName = i->first;
                                    // mark tank item (skip items with def stats for dps)
                                    if (modd == CR_DODGE || modd == CR_PARRY || modd == CR_BLOCK || modd == CR_DEFENSE_SKILL)
                                        isTankItem = true;

                                    if (modd == CR_CRIT_MELEE || modd == CR_CRIT_RANGED)
                                        isDpsItem = true;

                                    break;
                                }
                            }

                            if (weightName.empty())
                                continue;

                            effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, weightName, val);
                        }
                    }
                }
#endif

                // mana regen
                // SPELL_AURA_MOD_POWER_REGEN
                if (spellproto->EffectApplyAuraName[j] == SPELL_AURA_MOD_POWER_REGEN)
                {
                    isCasterItem = true;
                    effectAuraStatWeight += CalculateSingleStatWeight(playerclass, spec, "manargn", spellproto->EffectBasePoints[j] + 1);
                }
            }
        }

        // different stat weight based on trigger
        float coverage = 1;

        if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
        {
            if (spellData.SpellCooldown != 0)
            {
                int32 spellDuration = GetSpellDuration(spellproto);
                coverage = static_cast<float>(spellDuration) / spellData.SpellCooldown;
            }
            else
            {
                coverage = 0.17f; //Most often trinkets have 20 seconds buff with 2 minute cooldown which means ~17%
            }
        }
        else if (spellData.SpellTrigger == ITEM_SPELLTRIGGER_CHANCE_ON_HIT)
        {
#ifdef MANGOSBOT_ZERO
            float averageItemDelay = 2.43f;
#endif
#ifdef MANGOSBOT_ONE
            float averageItemDelay = 2.09f;
#endif
#ifdef MANGOSBOT_TWO
            float averageItemDelay = 2.39f;
#endif
            coverage = (static_cast<float>(spellproto->procChance) / 100) * (static_cast<float>(GetSpellDuration(spellproto)) / averageItemDelay);

            if (coverage > 0.9f)
                coverage = 0.9f;
        }

        effectAuraStatWeight *= coverage;
        effectAuraHealStatWeight *= coverage;
        effectAuraApStatWeight *= coverage;
        effectAuraDamageStatWeight *= coverage;

        auraStatWeight += effectAuraStatWeight;
        auraHealStatWeight += effectAuraHealStatWeight;
        auraApStatWeight += effectAuraApStatWeight;
        auraDamageStatWeight += effectAuraDamageStatWeight;
    }

#ifdef MANGOSBOT_ZERO
    // skip all 1h Maces for feral druids if they have no feral AP
    if (!isWhitelist && !isFeral && playerclass == CLASS_DRUID && proto->IsWeapon() && proto->SubClass == ITEM_SUBCLASS_WEAPON_MACE && (spec == 30 || spec == 32))
        return 0;
#endif

    statWeight += auraStatWeight;
    spellHeal += auraHealStatWeight;
    spellPower += auraDamageStatWeight;
    attackPower += auraApStatWeight;

    uint32 socketBonus = 0;
#ifndef MANGOSBOT_ZERO
    // check sockets
    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        if (!proto->Socket[i].Color)
            continue;

        if (proto->Socket[i].Color == SOCKET_COLOR_META)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "metasocket", 1);

        if (proto->Socket[i].Color == SOCKET_COLOR_YELLOW)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "yellowsocket", 1);

        if (proto->Socket[i].Color == SOCKET_COLOR_BLUE)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "bluesocket", 1);

        if (proto->Socket[i].Color == SOCKET_COLOR_RED)
            socketBonus += CalculateSingleStatWeight(playerclass, spec, "redsocket", 1);
    }
#endif

#ifdef MANGOSBOT_ONE
    // filter healing items in tbc as they also have spell damage
    if (spellDamage && spellHealing && isSpellDamageItem && isHealingItem && spellHealing > spellDamage)
    {
        isSpellDamageItem = false;
        isHealingItem = true;
    }
#endif
#ifdef MANGOSBOT_TWO
    if (spellHeal || spellPower)
        specType |= ITEM_SPEC_SPELL_HEALING | ITEM_SPEC_SPELL_DAMAGE;
#else
    if (spellHeal > spellPower || isHealingItem)
        specType |= ITEM_SPEC_SPELL_HEALING;

    if (spellPower >= spellHeal)
        specType |= ITEM_SPEC_SPELL_DAMAGE;
#endif

    if (isTankItem && (noCaster || !hasMana || !spellHeal || (!isHealingItem && !isSpellDamageItem)))
        specType |= ITEM_SPEC_TANK;

    if (isAttackItem)
        specType |= ITEM_SPEC_ATTACK;

    if (!noCaster && (isCasterItem || hasInt || isSpellDamageItem))
        specType |= ITEM_SPEC_CASTER;

#ifdef MANGOSBOT_ONE
    // retribution should not use items with spell power in 61+
    if ((spec == 6 || spec == 21) && (isSpellDamageItem || spellDamage || spellHealing || spellHeal))
        return 0;

    // filter tanking items (dodge, defense, parry, block) from dps classes
    if (proto->RequiredLevel > 60 && isTankItem && !(spec == 30 || spec == 3 || spec == 5))
        return 0;

    if (proto->RequiredLevel > 60 && isDpsItem && (spec == 30 || spec == 3 || spec == 5))
        return 0;
#endif
#ifdef MANGOSBOT_TWO
    // retribution should not use items with spell power in 61+
    if ((spec == 6 || spec == 21) && (isSpellDamageItem || spellDamage || spellHealing || spellHeal))
        return 0;

    // filter tanking items (dodge, defense, parry, block) from dps classes
    if (proto->RequiredLevel > 60 && isTankItem && !(spec == 30 || spec == 3 || spec == 5 || spec == 18))
        return 0;

    if (proto->RequiredLevel > 60 && isDpsItem && (spec == 30 || spec == 3 || spec == 5 || spec == 18))
        return 0;
#endif

    // limit speed for tank weapons
    if (!isWhitelist && spec == 3 && proto->IsWeapon() && proto->Delay > 2300)
        return 0;

    if (!isWhitelist && spec == 5 && proto->IsWeapon() && proto->Delay > 2400)
        return 0;

    // check for caster item
    if (isCasterItem || hasInt || spellHeal || spellPower || isSpellDamageItem || isHealingItem)
    {
        if (!isWhitelist && (!hasMana || (noCaster && !(spec == 6 || spec == 30 || spec == 32 || spec == 21))) && (spellHeal || isHealingItem || isSpellDamageItem || spellPower))
            return 0;

        if (!isWhitelist && !hasMana && hasInt)
            return 0;

        if (!isWhitelist && !hasMana && noCaster && (spellPower > attackPower || spellHeal > attackPower))
            return 0;

#ifndef MANGOSBOT_TWO
        if (!isWhitelist && (spec != 6 && spec != 21) && !spellPower && !spellHeal && isSpellDamageItem)
            return 0;

        if (!isWhitelist && /*(spec != 6 && spec != 21) && */!spellHeal && isHealingItem && !isSpellDamageItem)
            return 0;
#endif

        if (!isWhitelist && (spec != 6 && spec != 21) && !noCaster && isSpellDamageItem && !spellPower && !(spellDamage && spellHealing && proto->IsWeapon() && proto->InventoryType == INVTYPE_WEAPONMAINHAND))
            return 0;

        bool playerCaster = false;
        for (vector<WeightScaleStat>::iterator i = m_weightScales[spec].stats.begin(); i != m_weightScales[spec].stats.end(); ++i)
        {
            if (i->stat == "splpwr" || i->stat == "int" || i->stat == "manargn" || i->stat == "splheal" || i->stat == "spellcritstrkrtng" || i->stat == "spellhitrtng")
            {
                playerCaster = true;
            }
        }

        if (!isWhitelist && (spec != 6 && spec != 21 && playerclass != CLASS_HUNTER) && !playerCaster)
            return 0;
    }

    // check for caster item
    if (isAttackItem)
    {
        if (!isWhitelist && hasMana && !noCaster && !(hasInt || spellPower || spellHeal || isHealingItem || isSpellDamageItem))
            return 0;

        bool playerAttacker = false;
        for (vector<WeightScaleStat>::iterator i = m_weightScales[spec].stats.begin(); i != m_weightScales[spec].stats.end(); ++i)
        {
            if (i->stat == "str" || i->stat == "agi" || i->stat == "atkpwr" || i->stat == "mledps" || i->stat == "rgddps" || i->stat == "hitrtng" || i->stat == "critstrkrtng")
            {
                playerAttacker = true;
            }
        }

        if (!isWhitelist && !playerAttacker)
            return 0;
    }
    

    itSpec = (ItemSpecType)specType;

    statWeight += spellPower;
    statWeight += spellHeal;
    statWeight += attackPower;
    statWeight += defenseStats;

    // if stat value consists of only socket bonuses - skip
    if (socketBonus && !(statWeight || basicStatsWeight))
        return 0;

    statWeight += socketBonus;

    // handle negative stats
    if (basicStatsWeight < 0 && ((uint32)(abs(basicStatsWeight)) >= statWeight))
        statWeight = 0;
    else
        statWeight += basicStatsWeight;

    return statWeight;
}

uint32 RandomItemMgr::CalculateEnchantWeight(uint8 playerclass, uint8 spec, uint32 enchantId)
{
    if (!enchantId)
        return 0;

    SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(enchantId);

    if (!pEnchant)
        return 0;

    uint32 weight = 0;

    for (int s = 0; s < 3; ++s)
    {
        switch (pEnchant->type[s])
        {
        case 1: //Proc //TODO add proc values?
            break;
        case 2: //Damage
            if (!pEnchant->amount[s])
                continue;
            weight += CalculateSingleStatWeight(playerclass, spec, "mledps", pEnchant->amount[s]);
            break;
        case 3:
        {
            if (!pEnchant->spellid[s])
                continue;

            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(pEnchant->spellid[s]);

            if (!spellInfo)
                continue;

            for (uint32 j = 0; j < MAX_EFFECT_INDEX; ++j)
            {
                if (spellInfo->Effect[j] != SPELL_EFFECT_APPLY_AURA)
                    continue;

                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_STAT)
                {
                    uint32 stat = spellInfo->EffectMiscValue[j];
                    uint32 value = spellInfo->EffectBasePoints[j] + 1;

                    if (!value)
                        continue;

                    if (ItemStatLink.find(stat) == ItemStatLink.end())
                        continue;

                    weight += CalculateSingleStatWeight(playerclass, spec, ItemStatLink[stat], value);
                }
#ifdef MANGOSBOT_TWO
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {                    
                    uint32 spellDamage = spellInfo->EffectBasePoints[j] + 1;

                    if (spellInfo->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        weight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        if (!specialDamage)
                            return 0;

                        weight += specialDamage;
                    }
                }
#else
                // spell damage
                // SPELL_AURA_MOD_DAMAGE_DONE
                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE)
                {
                    uint32 spellDamage = spellInfo->EffectBasePoints[j] + 1;
                    // generic spell damage
                    if (spellInfo->EffectMiscValue[j] == SPELL_SCHOOL_MASK_MAGIC)
                    {
                        weight += CalculateSingleStatWeight(playerclass, spec, "splpwr", spellDamage);  
                    }
                    else
                    {
                        uint32 specialDamage = 0;
                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_ARCANE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "arcsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FROST) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "frosplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_FIRE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "firsplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_SHADOW) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "shasplpwr", spellDamage);

                        if ((spellInfo->EffectMiscValue[j] & SPELL_SCHOOL_MASK_NATURE) != 0)
                            specialDamage += CalculateSingleStatWeight(playerclass, spec, "natsplpwr", spellDamage);

                        if (!specialDamage)
                            return 0;

                        weight += specialDamage;
                    }
                }
                // spell healing
                // SPELL_AURA_MOD_HEALING_DONE
                if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE)
                {
                    weight += CalculateSingleStatWeight(playerclass, spec, "splheal", spellInfo->EffectBasePoints[j] + 1);
                }
#endif

                
            }
            break;
        }
        case 4: //Armor
            if (!pEnchant->amount[s])
                continue;
            weight += CalculateSingleStatWeight(playerclass, spec, "armor", pEnchant->amount[s]);
            break;
        case 5: //Stat
        {
            for (auto& statLink : weightStatLink)
            {
                if (statLink.second != pEnchant->spellid[s])
                    continue;

                weight += CalculateSingleStatWeight(playerclass, spec, statLink.first, pEnchant->amount[s]);
            }
            break;
        }
        case 6: //Totem
            break;
        case 7: //Use Spell
            break;
        case 8: //Prismatic socket
            break;
        }
    }

    return weight;
}


uint32 RandomItemMgr::CalculateRandomPropertyWeight(uint8 playerclass, uint8 spec, int32 randomPropertyId)
{
    uint32 weight = 0;
    if (randomPropertyId)
    {
        ItemRandomPropertiesEntry const* item_rand = sItemRandomPropertiesStore.LookupEntry(abs(randomPropertyId));
        if (item_rand)
        {
            for (uint32 i = PROP_ENCHANTMENT_SLOT_0; i < PROP_ENCHANTMENT_SLOT_0 + 3; ++i)
            {
                uint32 enchantId = item_rand->enchant_id[i - PROP_ENCHANTMENT_SLOT_0];

                weight += CalculateEnchantWeight(playerclass, spec, enchantId);
            }
        }
    }
    return weight;
}

uint32 RandomItemMgr::CalculateGemWeight(uint8 playerclass, uint8 spec, uint32 gemId)
{
#ifdef MANGOSBOT_ZERO
    return 0;
#else
    if (!gemId)
        return 0;

    return CalculateEnchantWeight(playerclass, spec, gemId);
#endif
}

uint32 RandomItemMgr::CalculateSocketWeight(uint8 playerclass, ItemQualifier& qualifier, uint8 spec)
{
#ifdef MANGOSBOT_ZERO
    return 0;
#else

    vector<uint32> gems = { qualifier.GetGem1() , qualifier.GetGem2(), qualifier.GetGem3(), qualifier.GetGem4() };

    ItemPrototype const* proto = qualifier.GetProto();

    bool hasGem = false;
    for (auto gem : gems)
        if(gem)
            hasGem = true;

    if (!hasGem)
        return 0;

    for (int i = 0; i < MAX_GEM_SOCKETS; ++i)
    {
        uint8 SocketColor = proto->Socket[i].Color;

        if (!SocketColor)
            continue;

        uint8 GemColor = 0;

        if (!gems[i])
            return 0;

        SpellItemEnchantmentEntry const* pEnchant = sSpellItemEnchantmentStore.LookupEntry(gems[i]);

        if (!pEnchant)
            return 0;

        uint32 gemid = pEnchant->GemID;
        if (!gemid)
            return 0;

        ItemPrototype const* gemProto = sItemStorage.LookupEntry<ItemPrototype>(gemid);
        if (!gemProto)
            return 0;
            
        GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gemProto->GemProperties);
        
        if (!gemProperty)
            return 0;

        GemColor = gemProperty->color;

        if (!(GemColor & SocketColor))
            return 0;
    }

    return CalculateEnchantWeight(playerclass, spec, proto->socketBonus);
#endif
}


uint32 RandomItemMgr::ItemStatWeight(Player* player, ItemQualifier& qualifier)
{
    ItemSpecType itSpec;
    uint32 weight = CalculateStatWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetProto(), itSpec);
    if(qualifier.GetEnchantId())
        weight += CalculateEnchantWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetEnchantId());
    if (qualifier.GetRandomPropertyId())
        weight += CalculateRandomPropertyWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetRandomPropertyId());
    if(qualifier.GetGem1())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem1());
    if (qualifier.GetGem2())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem2());
    if (qualifier.GetGem3())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem3());
    if (qualifier.GetGem4())
        weight += CalculateGemWeight(player->getClass(), GetPlayerSpecId(player), qualifier.GetGem4());

    weight += CalculateSocketWeight(player->getClass(), qualifier, GetPlayerSpecId(player));

    return weight;
}

uint32 RandomItemMgr::ItemStatWeight(Player* player, Item* item)
{
    ItemQualifier itemQualifier(item);
    return ItemStatWeight(player, itemQualifier);
}

uint32 RandomItemMgr::CalculateSingleStatWeight(uint8 playerclass, uint8 spec, std::string stat, uint32 value)
{
    uint32 statWeight = 0;
    for (vector<WeightScaleStat>::iterator i = m_weightScales[spec].stats.begin(); i != m_weightScales[spec].stats.end(); ++i)
    {
        if (stat == i->stat)
        {
            statWeight = i->weight * value;
            if (statWeight)
                sLog.outDetail("stat: %s, val: %d, weight: %d, total: %d, class: %d, spec: %s", stat.c_str(), value, i->weight, statWeight, playerclass, m_weightScales[spec].info.name.c_str());
            return statWeight;
        }
    }

    return statWeight;
}

bool CheckItemSpec(uint8 spec, ItemSpecType itSpec)
{
    return false;
}

uint32 RandomItemMgr::GetQuestIdForItem(uint32 itemId)
{
    bool isQuest = false;
    uint32 questId = 0;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewItemId[i])
                continue;

            if (quest->RewItemId[i] == itemId)
            {
                isQuest = true;
                questId = quest->GetQuestId();
                break;
            }
        }

        uint32 rewChocieItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChocieItemCount; ++i)
        {
            if (!quest->RewChoiceItemId[i])
                continue;

            if (quest->RewChoiceItemId[i] == itemId)
            {
                isQuest = true;
                questId = quest->GetQuestId();
                break;
            }
        }
        if (isQuest)
            break;
    }
    return questId;
}

vector<uint32> RandomItemMgr::GetQuestIdsForItem(uint32 itemId)
{
    vector<uint32> questIds;
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        Quest const* quest = i->second;

        uint32 rewItemCount = quest->GetRewItemsCount();
        for (uint32 i = 0; i < rewItemCount; ++i)
        {
            if (!quest->RewItemId[i])
                continue;

            if (quest->RewItemId[i] == itemId)
            {
                questIds.push_back(quest->GetQuestId());
                break;
            }
        }

        uint32 rewChocieItemCount = quest->GetRewChoiceItemsCount();
        for (uint32 i = 0; i < rewChocieItemCount; ++i)
        {
            if (!quest->RewChoiceItemId[i])
                continue;

            if (quest->RewChoiceItemId[i] == itemId)
            {
                questIds.push_back(quest->GetQuestId());
                break;
            }
        }
    }
    return questIds;
}

std::string RandomItemMgr::GetPlayerSpecName(Player* player)
{
    std::string specName;
    int tab = AiFactory::GetPlayerSpecTab(player);
    switch (player->getClass())
    {
    case CLASS_PRIEST:
        if (tab == 2)
            specName = "shadow";
        else if (tab == 1)
            specName = "holy";
        else
            specName = "disc";
        ;        break;
    case CLASS_SHAMAN:
        if (tab == 2)
            specName = "resto";
        else if (tab == 1)
            specName = "enhance";
        else
            specName = "elem";
        break;
    case CLASS_WARRIOR:
        if (tab == 2)
            specName = "prot";
        else if (tab == 1)
            specName = "fury";
        else
            specName = "arms";
        break;
    case CLASS_PALADIN:
        if (tab == 0)
            specName = "holy";
        else if (tab == 1)
            specName = "prot";
        else if (tab == 2)
            specName = "retrib";
        break;
    case CLASS_DRUID:
        if (tab == 0)
            specName = "balance";
        else if (tab == 1)
        {
            specName = "feraltank";
            if (player->GetLevel() > 19 && urand(0, 100) > 50)
                specName = "feraldps";
        }
        else if (tab == 2)
            specName = "resto";
        break;
    case CLASS_ROGUE:
        if (tab == 0)
            specName = "assas";
        else if (tab == 1)
            specName = "combat";
        else if (tab == 2)
            specName = "subtle";
        break;
    case CLASS_HUNTER:
        if (tab == 0)
            specName = "beast";
        else if (tab == 1)
            specName = "marks";
        else if (tab == 2)
            specName = "surv";
        break;
#ifdef MANGOSBOT_TWO
    case CLASS_DEATH_KNIGHT:
        if (tab == 0)
            specName = "blooddps";
        else if (tab == 1)
        {
            specName = "frosttank";
            if (urand(0, 100) > 50)
                specName = "frostdps";
        }
        else if (tab == 2)
            specName = "unholydps";
        break;
#endif
    case CLASS_MAGE:
        if (tab == 0)
            specName = "arcane";
        else if (tab == 1)
            specName = "fire";
        else if (tab == 2)
            specName = "frost";
        break;
    case CLASS_WARLOCK:
        if (tab == 0)
            specName = "afflic";
        else if (tab == 1)
            specName = "demo";
        else if (tab == 2)
            specName = "destro";
        break;
    default:
        break;
    }
    return specName;
}

uint32 RandomItemMgr::GetPlayerSpecId(Player* player)
{
    std::string specName = GetPlayerSpecName(player);
    if (specName.empty())
        return 0;

    for (auto itr : m_weightScales)
    {
        if (itr.second.info.name == specName && itr.second.info.classId == player->getClass())
            return itr.second.info.id;
    }
    return 0;
}

uint32 RandomItemMgr::GetUpgrade(Player* player, std::string spec, uint8 slot, uint32 quality, uint32 itemId)
{
    if (!player)
        return 0;

    // get old item statWeight
    uint32 oldStatWeight = 0;
    uint32 specId = 0;
    uint32 closestUpgrade = 0;
    uint32 closestUpgradeWeight = 0;
    vector<uint32> classspecs;

    for (uint32 specNum = 1; specNum < 5; ++specNum)
    {
        if (!m_weightScales[specNum].info.id)
            continue;

        classspecs.push_back(m_weightScales[specNum].info.id);

        if (m_weightScales[specNum].info.name == spec)
            specId = m_weightScales[specNum].info.id;
    }
    if (!specId)
        return 0;

    if (itemId && itemInfoCache[itemId])
    {
        oldStatWeight = itemInfoCache[itemId]->weights[specId];

        if (oldStatWeight)
            sLog.outString("Old Item: %d, weight: %d", itemId, oldStatWeight);
        else
            sLog.outString("Old item has no stat weight");
    }

    for (map<uint32, ItemInfoEntry*>::iterator i = itemInfoCache.begin(); i != itemInfoCache.end(); ++i)
    {
        ItemInfoEntry* info = i->second;
        if (!info)
            continue;

        // skip useless items
        if (info->weights[specId] == 0)
            continue;

        // skip higher lvl
        if (info->minLevel > player->GetLevel())
            continue;

        // skip too low level
        if (info->minLevel < (player->GetLevel() - 10))
            continue;

        // skip wrong team
        if (info->team && info->team != player->GetTeam())
            continue;

        // skip wrong slot
        if ((EquipmentSlots)info->slot != (EquipmentSlots)slot)
            continue;

        // skip higher quality
        if (quality && info->quality != quality)
            continue;

        // skip worse items
        if (info->weights[specId] <= oldStatWeight)
            continue;

        // skip items that only fit in slot, but not stats
        if (!itemId && info->weights[specId] == 1 && player->GetLevel() > 40)
            continue;

        // skip quest items
        if (info->source == ITEM_SOURCE_QUEST)
        {
            if (player->GetQuestRewardStatus(info->sourceId) != QUEST_STATUS_COMPLETE)
                continue;
        }

        // skip no stats trinkets
        if (info->weights[specId] == 1 &&
            info->slot == EQUIPMENT_SLOT_NECK ||
            info->slot == EQUIPMENT_SLOT_TRINKET1 ||
            info->slot == EQUIPMENT_SLOT_TRINKET2 ||
            info->slot == EQUIPMENT_SLOT_FINGER1 ||
            info->slot == EQUIPMENT_SLOT_FINGER2)
            continue;

        // check if item stat score is the best among class specs
        uint32 bestSpecId = 0;
        uint32 bestSpecScore = 0;
        for (vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
        {
            if (info->weights[*i] > bestSpecScore)
            {
                bestSpecId = *i;
                bestSpecScore = info->weights[specId];
            }
        }

        if (bestSpecId && bestSpecId != specId && player->GetLevel() > 40)
            return 0;

        if (!closestUpgrade)
        {
            closestUpgrade = info->itemId;
            closestUpgradeWeight = info->weights[specId];
        }

        // pick closest upgrade
        if (info->weights[specId] < closestUpgradeWeight)
        {
            closestUpgrade = info->itemId;
            closestUpgradeWeight = info->weights[specId];
        }
    }

    if (closestUpgrade)
        sLog.outString("New Item: %d, weight: %d", closestUpgrade, closestUpgradeWeight);

    return closestUpgrade;
}

vector<uint32> RandomItemMgr::GetUpgradeList(Player* player, uint32 specId, uint8 slot, uint32 quality, uint32 itemId, uint32 amount)
{
    vector<uint32> listItems;
    if (!player)
        return listItems;

    // get old item statWeight
    uint32 oldStatWeight = 0;
    uint32 closestUpgrade = 0;
    uint32 closestUpgradeWeight = 0;
    vector<uint32> classspecs;

    if (itemId && itemInfoCache[itemId])
    {
        oldStatWeight = itemInfoCache[itemId]->weights[specId];

        if (oldStatWeight)
            sLog.outString("Old Item: %d, weight: %d", itemId, oldStatWeight);
        else
            sLog.outString("Old item has no stat weight");
    }

    for (map<uint32, ItemInfoEntry*>::iterator i = itemInfoCache.begin(); i != itemInfoCache.end(); ++i)
    {
        ItemInfoEntry* info = i->second;
        if (!info)
            continue;

        // skip useless items
        if (info->weights[specId] == 0)
            continue;

        // skip higher lvl
        if (info->minLevel > player->GetLevel())
            continue;

        // skip too low level
        if ((int32)info->minLevel < (int32)(player->GetLevel() - 20))
            continue;

        // skip wrong team
        if (info->team && info->team != player->GetTeam())
            continue;

        // skip wrong slot
        if ((EquipmentSlots)info->slot != (EquipmentSlots)slot)
            continue;

        // skip higher quality
        if (quality && info->quality != quality)
            continue;

        // skip worse items
        if (info->weights[specId] <= oldStatWeight)
            continue;

        // skip items that only fit in slot, but not stats
        if (!itemId && info->weights[specId] == 1 && player->GetLevel() > 20)
            continue;

        // skip quest items
        if (info->source == ITEM_SOURCE_QUEST)
        {
            if (player->GetQuestRewardStatus(info->sourceId) != QUEST_STATUS_COMPLETE)
                continue;
        }

        // skip no stats trinkets
        if (info->weights[specId] < 2 && (
            info->slot == EQUIPMENT_SLOT_NECK ||
            info->slot == EQUIPMENT_SLOT_TRINKET1 ||
            info->slot == EQUIPMENT_SLOT_TRINKET2 ||
            info->slot == EQUIPMENT_SLOT_FINGER1 ||
            info->slot == EQUIPMENT_SLOT_FINGER2))
            continue;

        // skip pvp items
        if (info->source == ITEM_SOURCE_PVP)
        {
#ifndef MANGOSBOT_ZERO
            if (!player->GetHonorPoints() && !player->GetArenaPoints())
                continue;
#else
            if (!player->GetHonorRankInfo().rank)
                continue;
#endif
        }

        //if (player->GetLevel() >= 40)
        //{
        //    // check if item stat score is the best among class specs
        //    uint32 bestSpecId = 0;
        //    uint32 bestSpecScore = 0;
        //    for (vector<uint32>::iterator i = classspecs.begin(); i != classspecs.end(); ++i)
        //    {
        //        if (info->weights[*i] > bestSpecScore)
        //        {
        //            bestSpecId = *i;
        //            bestSpecScore = info->weights[specId];
        //        }
        //    }

        //    if (bestSpecId && bestSpecId != specId)
        //        continue;
        //}

        listItems.push_back(info->itemId);
        //continue;

        // pick closest upgrade
        if (info->weights[specId] > closestUpgradeWeight)
        {
            closestUpgrade = info->itemId;
            closestUpgradeWeight = info->weights[specId];
        }
    }

    if (listItems.size())
        sLog.outString("New Items: %d, Old item:%d, New items max: %d", listItems.size(), oldStatWeight, closestUpgradeWeight);

    // sort by stat weight
    sort(listItems.begin(), listItems.end(), [specId](int a, int b) { return sRandomItemMgr.GetStatWeight(a, specId) <= sRandomItemMgr.GetStatWeight(b, specId); });

    return listItems;
}

bool RandomItemMgr::HasStatWeight(uint32 itemId)
{
    return itemInfoCache[itemId] != nullptr;
}

uint32 RandomItemMgr::GetMinLevelFromCache(uint32 itemId)
{
    ItemInfoEntry* info = itemInfoCache[itemId];
    if (!info)
        return 0;

    return info->minLevel;
}

uint32 RandomItemMgr::GetStatWeight(Player* player, uint32 itemId)
{
    if (!player || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    uint32 statWeight = 0;
    uint32 specId = GetPlayerSpecId(player);
    vector<uint32> classspecs;

    if (specId == 0)
        return 0;

    if (!m_weightScales[specId].info.id)
        return 0;

    map<uint32, ItemInfoEntry*>::iterator itr = itemInfoCache.find(itemId);
    if (itr != itemInfoCache.end())
    {
        statWeight = itr->second->weights[specId];
    }

    return statWeight;
}

uint32 RandomItemMgr::GetStatWeight(uint32 itemId, uint32 specId)
{
    if (!specId || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    uint32 statWeight = 0;
    vector<uint32> classspecs;

    if (!m_weightScales[specId].info.id)
        return 0;

    map<uint32, ItemInfoEntry*>::iterator itr = itemInfoCache.find(itemId);
    if (itr != itemInfoCache.end())
    {
        statWeight = itr->second->weights[specId];
    }

    return statWeight;
}

uint32 RandomItemMgr::GetLiveStatWeight(Player* player, uint32 itemId, uint32 specId)
{
    if (!player || !itemId)
        return 0;

    if (!itemInfoCache[itemId])
        return 0;

    uint32 statWeight = 0;
    specId = specId ? specId : GetPlayerSpecId(player);
    if (specId == 0)
        return 0;

    if (!m_weightScales[specId].info.id)
        return 0;

    ItemInfoEntry* info = itemInfoCache[itemId];
    if (!info)
        return 0;

    statWeight = info->weights[specId];

    // skip higher lvl
    if (info->minLevel > player->GetLevel())
        return 0;

    // skip too low level
    //if ((int32)info->minLevel < (int32)(player->GetLevel() - 20))
    //    return 0;

    // skip wrong team
    if (info->team && (Team)info->team != player->GetTeam())
        return 0;

    // skip quest items
    if (info->source == ITEM_SOURCE_QUEST && info->sourceId)
    {
        Quest const* quest = sObjectMgr.GetQuestTemplate(info->sourceId);
        if (quest)
        {
            // only class quests player could do
            if (!player->SatisfyQuestClass(quest, false) || !player->SatisfyQuestRace(quest, false) || !player->SatisfyQuestLevel(quest, false))
                return 0;
        }
        /*if (!player->GetQuestRewardStatus(info->sourceId))
            return 0;*/
    }

    // skip pvp items
    /*if (info->source == ITEM_SOURCE_PVP)
    {
#ifndef MANGOSBOT_ZERO
        if (!player->GetHonorPoints() && !player->GetArenaPoints())
            return 0;
#else
        if (!player->GetHonorRankInfo().rank)
            return 0;
#endif
    }*/

    // skip missing reputation
    if (info->repFaction && uint32(player->GetReputationRank(info->repFaction)) < info->repRank)
        return 0;
    
#ifdef MANGOSBOT_ZERO
    // skip missing pvp ranks
    if (info->pvpRank && player->GetHonorHighestRankInfo().rank < info->pvpRank)
        return 0;
    if (info->pvpRank && info->pvpRank < 16 && player->GetHonorHighestRankInfo().rank == 18)
        return 0;

    // skip non pvp items for some specs
    if (info->pvpRank < 16 && player->GetHonorHighestRankInfo().rank == 18)
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
        if (proto && !(
            info->slot == EQUIPMENT_SLOT_WAIST ||
            info->slot == EQUIPMENT_SLOT_WRISTS ||
            info->slot == EQUIPMENT_SLOT_BACK ||
            info->slot == EQUIPMENT_SLOT_NECK ||
            info->slot == EQUIPMENT_SLOT_TRINKET1 ||
            info->slot == EQUIPMENT_SLOT_TRINKET2 ||
            info->slot == EQUIPMENT_SLOT_FINGER1 ||
            info->slot == EQUIPMENT_SLOT_FINGER2 ||
            proto->SubClass == ITEM_SUBCLASS_ARMOR_TOTEM ||
            proto->SubClass == ITEM_SUBCLASS_ARMOR_LIBRAM ||
            proto->SubClass == ITEM_SUBCLASS_ARMOR_IDOL)
            && !(specId == 3 || specId == 4 || specId == 5 || specId == 14 || specId == 22 || specId == 31))
            return 0;
    }
#else
    // skip missing pvp ranks
    if (info->pvpRank && player->GetHighestPvPRankIndex() < info->pvpRank)
        return 0;
#endif

    // skip missing skills
    if (info->reqSkill && player->GetSkillValue(info->reqSkill) < info->reqSkillRank)

    // skip no stats trinkets
    if (info->weights[specId] == 1 && info->quality > ITEM_QUALITY_RARE && (
        info->slot == EQUIPMENT_SLOT_NECK ||
        info->slot == EQUIPMENT_SLOT_TRINKET1 ||
        info->slot == EQUIPMENT_SLOT_TRINKET2 ||
        info->slot == EQUIPMENT_SLOT_FINGER1 ||
        info->slot == EQUIPMENT_SLOT_FINGER2))
        return 0;

    // skip items that only fit in slot, but not stats
    if (!itemId && info->weights[specId] == 1 && player->GetLevel() > 20)
        return 0;

    // check if item stat score is the best among class specs
    /*uint32 bestSpecId = 0;
    uint32 bestSpecScore = 0;
    for (uint32 spec = 1; spec < MAX_STAT_SCALES; ++spec)
    {
        if (!m_weightScales[spec].info.id)
            continue;

        if (m_weightScales[spec].info.classId != player->getClass())
            continue;

        if (info->weights[spec] > bestSpecScore && info->weights[spec] > 1)
        {
            bestSpecId = spec;
            bestSpecScore = info->weights[spec];
        }
    }*/

    // TODO test
    /*if (bestSpecId && bestSpecId != specId && player->GetLevel() >= 60)
        return 0;*/

#ifdef MANGOSBOT_ZERO
    // increase stat weights for pvp items
    if (info->pvpRank)
        return statWeight * 5;
#endif

    return statWeight;
}

void RandomItemMgr::BuildEquipCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    auto results = PlayerbotDatabase.PQuery("select clazz, spec, lvl, slot, quality, item from ai_playerbot_equip_cache");
    if (results)
    {
        sLog.outString("Loading equipment cache for %d classes, %d levels, %d slots, %d quality from %d items",
                MAX_CLASSES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, sItemStorage.GetMaxEntry());
        int count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 clazz = fields[0].GetUInt32();
            uint32 spec = fields[1].GetUInt32();
            uint32 level = fields[2].GetUInt32();
            uint32 slot = fields[3].GetUInt32();
            uint32 quality = fields[4].GetUInt32();
            uint32 itemId = fields[5].GetUInt32();

            BotEquipKey key(level, clazz, spec, slot, quality);
            equipCache[key].push_back(itemId);
            count++;

        } while (results->NextRow());
        sLog.outString("Equipment cache loaded from %d records", count);
    }
    else
    {
        uint64 total = uint64(MAX_CLASSES * 3 * maxLevel * EQUIPMENT_SLOT_END * ITEM_QUALITY_ARTIFACT);
        sLog.outString("Building equipment cache for %d classes, %d specs, %d levels, %d slots, %d quality from %d items (%zu total)",
                MAX_CLASSES, MAX_STAT_SCALES, maxLevel, EQUIPMENT_SLOT_END, ITEM_QUALITY_ARTIFACT, sItemStorage.GetMaxEntry(), total);

        BarGoLink bar(total);
        RandomItemList tabardsList;
        RandomItemList shirtsList;
        BotEquipKey tabardKey(60, 1, 1, EQUIPMENT_SLOT_TABARD, 1);
        BotEquipKey shirtKey(60, 1, 1, EQUIPMENT_SLOT_BODY, 1);
        
        for (uint8 clazz = CLASS_WARRIOR; clazz < MAX_CLASSES; ++clazz)
        {
            // skip nonexistent classes
            if (!((1 << (clazz - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(clazz))
                continue;

            for (uint32 level = 1; level <= maxLevel; ++level)
            {
                for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
                {
                    for (uint32 spec = 1; spec <= MAX_STAT_SCALES; ++spec)
                    {
                        if (!m_weightScales[spec].info.id)
                            continue;

                        if (m_weightScales[spec].info.classId != clazz)
                            continue;

                        for (uint32 quality = ITEM_QUALITY_POOR; quality <= ITEM_QUALITY_ARTIFACT; ++quality)
                        {
                            BotEquipKey key(level, clazz, spec, slot, quality);

                            RandomItemList items;
                            for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
                            {
                                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                                if (!proto)
                                    continue;

                                if (proto->Quality != key.quality)
                                    continue;

                                // skip not available items
                                if (proto->ExtraFlags & ITEM_EXTRA_NOT_OBTAINABLE)
                                    continue;

                                if ((slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD))
                                {
                                    set<InventoryType> slots = viableSlots[(EquipmentSlots)key.slot];
                                    if (slots.find((InventoryType)proto->InventoryType) == slots.end())
                                        continue;

                                    if (slot == EQUIPMENT_SLOT_BODY && std::find(shirtsList.begin(), shirtsList.end(), itemId) == shirtsList.end())
                                        shirtsList.push_back(itemId);
                                    if (slot == EQUIPMENT_SLOT_TABARD && std::find(tabardsList.begin(), tabardsList.end(), itemId) == tabardsList.end())
                                        tabardsList.push_back(itemId);

                                    PlayerbotDatabase.PExecute("replace into ai_playerbot_equip_cache (id, clazz, spec, lvl, slot, quality, item) values (%u, %u, %u, %u, %u, %u, %u)",
                                        2000000 + itemId, 1, 1, 60, slot, 1, itemId);

                                    continue;
                                }

                                // check stat weight
                                uint32 statWeight = GetStatWeight(itemId, spec);
                                if (statWeight <= 0)
                                    continue;

                                // only accept "useless" items if bot level <= 30
                                if (statWeight == 1 && level > 20)
                                    continue;

                                uint32 minLevel = GetMinLevelFromCache(itemId);
                                // skip higher level (e.g. quest rewards)
                                if (minLevel > level)
                                    continue;

                                if (abs((int)minLevel - (int)level) > 20)
                                    continue;

                                /*if (proto->Class == ITEM_CLASS_WEAPON && abs((int)minLevel - (int)level) > 10)
                                    continue;*/

                                if (proto->Class != ITEM_CLASS_WEAPON &&
                                    proto->Class != ITEM_CLASS_ARMOR &&
                                    proto->Class != ITEM_CLASS_CONTAINER &&
                                    proto->Class != ITEM_CLASS_PROJECTILE)
                                    continue;

                                if (!CanEquipItem(key, proto))
                                    continue;

                                if (proto->Class == ITEM_CLASS_ARMOR && (
                                    slot == EQUIPMENT_SLOT_HEAD ||
                                    slot == EQUIPMENT_SLOT_SHOULDERS ||
                                    slot == EQUIPMENT_SLOT_CHEST ||
                                    slot == EQUIPMENT_SLOT_WAIST ||
                                    slot == EQUIPMENT_SLOT_LEGS ||
                                    slot == EQUIPMENT_SLOT_FEET ||
                                    slot == EQUIPMENT_SLOT_WRISTS ||
                                    slot == EQUIPMENT_SLOT_HANDS) && !CanEquipArmor(key.clazz, key.spec, key.level, proto))
                                    continue;

                                //if (proto->Class == ITEM_CLASS_WEAPON && !CanEquipWeapon(key.clazz, proto))
                                //    continue;

                                if (slot == EQUIPMENT_SLOT_OFFHAND && key.clazz == CLASS_ROGUE && proto->Class != ITEM_CLASS_WEAPON)
                                    continue;

                                items.push_back(itemId);

                                PlayerbotDatabase.PExecute("insert into ai_playerbot_equip_cache (clazz, spec, lvl, slot, quality, item) values (%u, %u, %u, %u, %u, %u)",
                                    clazz, spec, level, slot, quality, itemId);
                            }

                            equipCache[key] = items;
                            bar.step();
                            sLog.outDetail("Equipment cache for class: %d, level %d, slot %d, quality %d: %zu items",
                                clazz, level, slot, quality, items.size());
                        }
                    }
                }
            }
        }
        equipCache[tabardKey] = tabardsList;
        equipCache[shirtKey] = shirtsList;
        sLog.outString("Equipment cache saved to DB");
    }
}

RandomItemList RandomItemMgr::Query(uint32 level, uint8 clazz, uint8 spec, uint8 slot, uint32 quality)
{
    BotEquipKey key(level, clazz, spec, slot, quality);
    return equipCache[key];
}

void RandomItemMgr::BuildAmmoCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building ammo cache for %d levels", maxLevel);
	int counter1 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        for (uint32 subClass = ITEM_SUBCLASS_ARROW; subClass <= ITEM_SUBCLASS_BULLET; subClass++)
        {
            auto results = WorldDatabase.PQuery(
                    "select entry, RequiredLevel from item_template where class = '%u' and subclass = '%u' and RequiredLevel <= '%u' and quality = '%u' order by RequiredLevel desc",
                    ITEM_CLASS_PROJECTILE, subClass, level, ITEM_QUALITY_NORMAL);
            if (!results)
                return;

            Field* fields = results->Fetch();
            if (fields)
            {
                uint32 entry = fields[0].GetUInt32();
                ammoCache[level / 10][subClass] = entry;
				counter1++;
            }
        }

        auto results = WorldDatabase.PQuery(
            "select entry, RequiredLevel from item_template where class = '%u' and subclass = '%u' and RequiredLevel <= '%u' and quality = '%u' order by RequiredLevel desc",
            ITEM_CLASS_WEAPON, ITEM_SUBCLASS_WEAPON_THROWN, level, ITEM_QUALITY_NORMAL);
        if (!results)
            return;

        Field* fields = results->Fetch();
        if (fields)
        {
            uint32 entry = fields[0].GetUInt32();
            ammoCache[level / 10][ITEM_SUBCLASS_WEAPON_THROWN] = entry;
            counter1++;
        }
    }
	sLog.outString("Cached %d types of ammo", counter1); // TEST
}

uint32 RandomItemMgr::GetAmmo(uint32 level, uint32 subClass)
{
    return ammoCache[(level - 1) / 10][subClass];
}


void RandomItemMgr::BuildPotionCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building potion cache for %d levels", maxLevel);
	int counter2 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
        for (int i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];

            for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                    (proto->SubClass != ITEM_SUBCLASS_POTION && proto->SubClass != ITEM_SUBCLASS_FLASK) ||
                    proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                if (sAhBotConfig.ignoreItemIds.find(proto->ItemId) != sAhBotConfig.ignoreItemIds.end())
                    continue;

                for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
                {
                    const SpellEntry* const spellInfo = sServerFacade.LookupSpellInfo(proto->Spells[j].SpellId);
                    if (!spellInfo)
                        continue;

                    for (int i = 0 ; i < 3; i++)
                    {
                        if (spellInfo->Effect[i] == effect)
                        {
                            potionCache[level / 10][effect].push_back(itemId);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 effects[] = { SPELL_EFFECT_HEAL, SPELL_EFFECT_ENERGIZE };
        for (int i = 0; i < 2; ++i)
        {
            uint32 effect = effects[i];
            uint32 size = potionCache[level / 10][effect].size();
			counter2++;
            sLog.outDetail("Potion cache for level=%d, effect=%d: %d items", level, effect, size);
        }
    }
	sLog.outString("Cached %d types of potions", counter2); // TEST
}

void RandomItemMgr::BuildFoodCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building food cache for %d levels", maxLevel);
	int counter3 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 categories[] = { 11, 59 };
        for (int i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];

            for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
            {
                ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
                if (!proto)
                    continue;

                if (proto->Class != ITEM_CLASS_CONSUMABLE ||
                    (proto->SubClass != ITEM_SUBCLASS_FOOD && proto->SubClass != ITEM_SUBCLASS_CONSUMABLE) ||
                    (proto->Spells[0].SpellCategory != category) ||
                    proto->Bonding != NO_BIND)
                    continue;

                if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                    continue;

                if (proto->RequiredSkill)
                    continue;

                if (proto->Area || proto->Map || proto->RequiredCityRank || proto->RequiredHonorRank)
                    continue;

                if (proto->Duration & 0x80000000)
                    continue;

                if (sAhBotConfig.ignoreItemIds.find(proto->ItemId) != sAhBotConfig.ignoreItemIds.end())
                    continue;

                foodCache[level / 10][category].push_back(itemId);
            }
        }
    }

    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 categories[] = { 11, 59 };
        for (int i = 0; i < 2; ++i)
        {
            uint32 category = categories[i];
            uint32 size = foodCache[level / 10][category].size();
			counter3++;
            sLog.outDetail("Food cache for level=%d, category=%d: %d items", level, category, size);
        }
    }
	sLog.outString("Cached %d types of food", counter3);
}

uint32 RandomItemMgr::GetRandomPotion(uint32 level, uint32 effect)
{
    vector<uint32> potions = potionCache[(level - 1) / 10][effect];
    if (potions.empty()) return 0;
    return potions[urand(0, potions.size() - 1)];
}

uint32 RandomItemMgr::GetFood(uint32 level, uint32 category)
{
    vector<uint32> items;
    if (category == 11)
    {
        if (level < 5)
            items = { 787, 117, 4540, 2680 };
        else if (level < 15)
            items = { 2287, 4592, 4541, 21072 };
        else if (level < 25)
            items = { 3770, 16170, 4542, 20074 };
        else if (level < 35)
            items = { 4594, 3771, 1707, 4457 };
        else if (level < 45)
            items = { 4599, 4601, 21552, 17222 /*21030, 16168 */ };
#ifdef MANGOSBOT_ZERO
        else
#else
        else if (level < 55)
#endif
            items = { 8950, 8952, 8957, 21023 /*21033, 21031 */ };
#ifdef MANGOSBOT_ONE
        else if (level < 65)
            items = { 29292, 27859, 30458, 27662 };
        else
            items = { 29450, 29451, 29452 };
#endif
#ifdef MANGOSBOT_TWO
    else if (level < 65)
        items = { 29292, 27859, 30458, 27662 };
    else if (level < 75)
        items = { 29450, 29451, 29452 };
    else
        items = { 35947 };
#endif
    }

    if (category == 59)
    {
        if (level < 5)
            items = { 159, 117 };
        else if (level < 15)
            items = { 1179, 21072 };
        else if (level < 25)
            items = { 1205 };
        else if (level < 35)
            items = { 1708 };
        else if (level < 45)
            items = { 1645 };
#ifdef MANGOSBOT_ZERO
        else
#else
        else if (level < 55)
#endif
            items = { 8766 };
#ifdef MANGOSBOT_ONE
        else if (level < 65)
            items = { 28399 };
        else
            items = { 27860 };
#endif
#ifdef MANGOSBOT_TWO
    else if (level < 65)
        items = { 28399 };
    else if (level < 75)
        items = { 27860 };
    else
        items = { 33445 };
#endif
    }

    if (items.empty()) return 0;
    return items[urand(0, items.size() - 1)];
}

uint32 RandomItemMgr::GetRandomFood(uint32 level, uint32 category)
{
    vector<uint32> food = foodCache[(level - 1) / 10][category];
    if (food.empty()) return 0;
    return food[urand(0, food.size() - 1)];
}

void RandomItemMgr::BuildTradeCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    sLog.outBasic("Building trade cache for %d levels", maxLevel);
	int counter4 = 0;
    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_TRADE_GOODS || proto->Bonding != NO_BIND)
                continue;

            if (proto->ItemLevel < level)
                continue;

            if (proto->RequiredLevel && (proto->RequiredLevel > level || proto->RequiredLevel < level - 10))
                continue;

            if (proto->RequiredSkill)
                continue;

            tradeCache[level / 10].push_back(itemId);
        }
    }

    for (uint32 level = 1; level <= maxLevel+1; level+=10)
    {
        uint32 size = tradeCache[level / 10].size();
        sLog.outDetail("Trade cache for level=%d: %d items", level, size);
		counter4++;
    }
	sLog.outString("Cached %d trade categories", counter4); // TEST
}

uint32 RandomItemMgr::GetRandomTrade(uint32 level)
{
    vector<uint32> trade = tradeCache[(level - 1) / 10];
    if (trade.empty()) return 0;
    return trade[urand(0, trade.size() - 1)];
}

vector<uint32> RandomItemMgr::GetGemsList()
{
    vector<uint32>_gems;

#ifndef MANGOSBOT_ZERO
    if (_gems.empty())
    {
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Class != ITEM_CLASS_GEM)
                continue;

            if (proto->SubClass == ITEM_SUBCLASS_GEM_SIMPLE)
                continue;

            _gems.push_back(proto->ItemId);
        }
    }
#endif
    return _gems;
}

void RandomItemMgr::BuildRarityCache()
{
    auto results = PlayerbotDatabase.PQuery("select item, rarity from ai_playerbot_rarity_cache");
    if (results)
    {
        sLog.outBasic("Loading item rarity cache");
        int count = 0;
        do
        {
            Field* fields = results->Fetch();
            uint32 itemId = fields[0].GetUInt32();
            float rarity = fields[1].GetFloat();

            rarityCache[itemId] = rarity;
            count++;

        } while (results->NextRow());
        sLog.outString("Item rarity cache loaded from %d records", count);
    }
    else
    {
        sLog.outBasic("Building item rarity cache from %u items", sItemStorage.GetMaxEntry());
        BarGoLink bar(sItemStorage.GetMaxEntry());
        for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); ++itemId)
        {
            bar.step();
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);
            if (!proto)
                continue;

            if (proto->Duration & 0x80000000)
                continue;

            if (proto->Quality == ITEM_QUALITY_POOR)
                continue;

            if (sAhBotConfig.ignoreItemIds.find(proto->ItemId) != sAhBotConfig.ignoreItemIds.end())
                continue;

            if (strstri(proto->Name1, "qa") || strstri(proto->Name1, "test") || strstri(proto->Name1, "deprecated"))
                continue;

            if (!proto->ItemLevel/* || proto->RequiredLevel > sAhBotConfig.maxRequiredLevel || proto->ItemLevel > sAhBotConfig.maxItemLevel*/)
                continue;
            auto results = WorldDatabase.PQuery(
                    "select max(q.chance) from ( "
                    // "-- Creature "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from creature_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from creature_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'creature' type "
                    "from creature_loot_template lt "
                    "join creature_template ct on ct.LootId = lt.entry "
                    "join creature c on c.id = ct.entry "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Gameobject "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from gameobject_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from gameobject_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'gameobject' type "
                    "from gameobject_loot_template lt "
                    "join gameobject_template ct on ct.data1 = lt.entry "
                    "join gameobject c on c.id = ct.entry "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Disenchant "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from disenchant_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from disenchant_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'disenchant' type "
                    "from disenchant_loot_template lt "
                    "join item_template ct on ct.DisenchantID = lt.entry "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Fishing "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from fishing_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    / (select count(*) from fishing_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0) "
                    "    end "
                    ") chance, 'fishing' type "
                    "from fishing_loot_template lt "
                    "where lt.item = '%u' "
                    "union all "
                    // "-- Skinning "
                    "select  "
                    "avg ( "
                    "   case  "
                    "    when lt.groupid = 0 then lt.ChanceOrQuestChance  "
                    "    when lt.ChanceOrQuestChance > 0 then lt.ChanceOrQuestChance  "
                    "    else   "
                    "    ifnull(100 - (select sum(ChanceOrQuestChance) from skinning_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance > 0), 100) "
                    "    * ifnull((select 1/count(*) from skinning_loot_template lt1 where lt1.groupid = lt.groupid and lt1.entry = lt.entry and lt1.ChanceOrQuestChance = 0), 1) "
                    "    end "
                    ") chance, 'skinning' type "
                    "from skinning_loot_template lt "
                    "join creature_template ct on ct.SkinningLootId = lt.entry "
                    "join creature c on c.id = ct.entry "
                    "where lt.item = '%u' "
                    ") q; ",
                             itemId,itemId,itemId,itemId,itemId);

            if (results)
            {
                Field* fields = results->Fetch();
                float rarity = fields[0].GetFloat();
                if (rarity > 0.01)
                {
                    rarityCache[itemId] = rarity;

                    PlayerbotDatabase.PExecute("insert into ai_playerbot_rarity_cache (item, rarity) values (%u, %f)",
                            itemId, rarity);
                }
            }
        }
        sLog.outString("Item rarity cache built from %u items", sItemStorage.GetMaxEntry());
    }
}

float RandomItemMgr::GetItemRarity(uint32 itemId)
{
    return rarityCache[itemId];
}
