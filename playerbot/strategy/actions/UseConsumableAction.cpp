#include "playerbot/playerbot.h"
#include "UseConsumableAction.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/AiFactory.h"

using namespace ai;

// Flasks vanilla
const std::vector<uint32> UseConsumableAction::knownFlaskAuras = {
    17626,  // Flask of the Titans
    17627,  // Flask of Distilled Wisdom
    17628,  // Flask of Supreme Power
    17629,  // Flask of Chromatic Resistance
    17624,  // Flask of Petrification
};

// Food buffs vanilla
const std::vector<uint32> UseConsumableAction::knownFoodBuffAuras = {
    18230,  // Grilled Squid
    25660,  // Dirge's Kickin' Chimaerok Chops
    24800,  // Smoked Desert Dumplings
    18233,  // Nightfin Soup
    22731,  // Runn Tum Tuber Surprise
    18140,  // Blessed Sunfruit Juice
    18124,  // Blessed Sunfruit
    18191,  // Increased Stamina
};

const std::vector<uint32> UseConsumableAction::alcoholItemIds = {
    21151,
};

// Never use these items 
const std::vector<uint32> UseConsumableAction::excludedItemIds = {
    13455,  // Greater Stoneshield Potion
    13461,  // Mighty Rage Potion
    5634,   // Free Action Potion
    13457,  // Greater Fire Protection Potion
    13456,  // Greater Frost Protection Potion
    13460,  // Greater Holy Protection Potion
    13458,  // Greater Nature Protection Potion
    13459,  // Greater Shadow Protection Potion
    13462,  // Greater Arcane Protection Potion
};

bool UseConsumableAction::IsProtectionConsumable(const ItemPrototype* proto) const
{
    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        uint32 spellId = proto->Spells[i].SpellId;
        if (!spellId)
            continue;

        const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!spellInfo)
            continue;

        for (int j = 0; j < 3; ++j)
        {
            if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_SCHOOL_ABSORB)
                return true;
        }

        bool hasResistance = false;
        bool hasOtherBuff = false;
        for (int j = 0; j < 3; ++j)
        {
            if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_RESISTANCE)
                hasResistance = true;
            else if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_STAT ||
                spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_ATTACK_POWER ||
                spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE ||
                spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HIT_CHANCE ||
                spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_CRIT_PERCENT ||
                spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_POWER_REGEN ||
                spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE)
                hasOtherBuff = true;
        }

        if (hasResistance && !hasOtherBuff)
            return true;
    }
    return false;
}

bool UseConsumableAction::IsPotionLike(const ItemPrototype* proto) const
{
    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        uint32 spellId = proto->Spells[i].SpellId;
        if (!spellId)
            continue;

        if (proto->Spells[i].SpellCategory == 4)
            return true;

        const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (spellInfo && spellInfo->Category == 4)
            return true;
    }
    return false;
}

bool UseConsumableAction::IsFoodOrDrink(const ItemPrototype* proto) const
{
    if (proto->Class != ITEM_CLASS_CONSUMABLE)
        return false;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (proto->Spells[i].SpellId &&
            (proto->Spells[i].SpellCategory == 11 || proto->Spells[i].SpellCategory == 59))
            return true;
    }
    return false;
}

bool UseConsumableAction::IsStatBuffSpell(uint32 spellId) const
{
    const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!spellInfo)
        return false;

    for (int j = 0; j < 3; ++j)
    {
        if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_STAT ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_ATTACK_POWER ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HIT_CHANCE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_CRIT_PERCENT ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_POWER_REGEN ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_RESISTANCE)
        {
            if (spellInfo->SpellName[0])
            {
                std::string name = spellInfo->SpellName[0];
                if (name == "Food" || name == "Drink")
                    continue;
            }
            return true;
        }

        if (spellInfo->EffectTriggerSpell[j])
        {
            const SpellEntry* triggered = sServerFacade.LookupSpellInfo(spellInfo->EffectTriggerSpell[j]);
            if (triggered)
            {
                if (triggered->SpellName[0])
                {
                    std::string tname = triggered->SpellName[0];
                    if (tname == "Food" || tname == "Drink")
                        continue;
                }

                for (int k = 0; k < 3; ++k)
                {
                    if (triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_STAT ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_ATTACK_POWER ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_DAMAGE_DONE ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_HIT_CHANCE ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_CRIT_PERCENT ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_POWER_REGEN ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_HEALING_DONE ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_RESISTANCE)
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

uint32 UseConsumableAction::ResolveBuffAuraSpellId(uint32 spellId) const
{
    const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!spellInfo)
        return spellId;

    for (int j = 0; j < 3; ++j)
    {
        if (spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_STAT ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_ATTACK_POWER ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_DAMAGE_DONE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HIT_CHANCE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_CRIT_PERCENT ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_POWER_REGEN ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_HEALING_DONE ||
            spellInfo->EffectApplyAuraName[j] == SPELL_AURA_MOD_RESISTANCE)
        {
            return spellId;
        }

        if (spellInfo->EffectTriggerSpell[j])
        {
            const SpellEntry* triggered = sServerFacade.LookupSpellInfo(spellInfo->EffectTriggerSpell[j]);
            if (triggered)
            {
                for (int k = 0; k < 3; ++k)
                {
                    if (triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_STAT ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_ATTACK_POWER ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_DAMAGE_DONE ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_HIT_CHANCE ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_CRIT_PERCENT ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_POWER_REGEN ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_HEALING_DONE ||
                        triggered->EffectApplyAuraName[k] == SPELL_AURA_MOD_RESISTANCE)
                    {
                        return spellInfo->EffectTriggerSpell[j];
                    }
                }
            }
        }
    }

    return spellId;
}

void UseConsumableAction::CollectPossibleAuraIds(uint32 spellId, std::vector<uint32>& outIds) const
{
    outIds.push_back(spellId);

    const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!spellInfo)
        return;

    for (int j = 0; j < 3; ++j)
    {
        if (spellInfo->EffectTriggerSpell[j] && spellInfo->EffectTriggerSpell[j] != spellId)
            outIds.push_back(spellInfo->EffectTriggerSpell[j]);
    }
}

bool UseConsumableAction::ClassifyConsumable(const ItemPrototype* proto, ConsumableType& outType, uint32& outSpellId) const
{
    if (!proto || proto->Class != ITEM_CLASS_CONSUMABLE)
        return false;

    if (proto->SubClass == ITEM_SUBCLASS_POTION)
        return false;

    for (uint32 id : excludedItemIds)
    {
        if (proto->ItemId == id)
            return false;
    }

    for (uint32 id : alcoholItemIds)
    {
        if (proto->ItemId == id)
        {
            for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                if (proto->Spells[i].SpellId)
                {
                    outSpellId = proto->Spells[i].SpellId;
                    outType = ConsumableType::Alcohol;
                    return true;
                }
            }
            return false;
        }
    }

    if (proto->SubClass == ITEM_SUBCLASS_FLASK)
    {
        bool isRealFlask = false;
        uint32 firstSpellId = 0;
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            if (!firstSpellId)
                firstSpellId = spellId;

            std::vector<uint32> possibleAuras;
            CollectPossibleAuraIds(spellId, possibleAuras);
            for (uint32 auraId : possibleAuras)
            {
                for (uint32 flaskAura : knownFlaskAuras)
                {
                    if (auraId == flaskAura)
                    {
                        isRealFlask = true;
                        break;
                    }
                }
                if (isRealFlask)
                    break;
            }
            if (isRealFlask)
                break;
        }

        if (!firstSpellId)
            return false;

        if (isRealFlask)
        {
            outSpellId = firstSpellId;
            outType = ConsumableType::Flask;
            return true;
        }

        if (IsPotionLike(proto))
            return false;

        if (IsProtectionConsumable(proto))
            return false;

        outSpellId = firstSpellId;
        outType = ConsumableType::Elixir;
        return true;
    }

    if (proto->SubClass == ITEM_SUBCLASS_ELIXIR)
    {
        if (IsProtectionConsumable(proto))
            return false;

        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            outSpellId = spellId;
            outType = ConsumableType::Elixir;
            return true;
        }
        return false;
    }

    if (proto->SubClass == ITEM_SUBCLASS_FOOD ||
        proto->SubClass == ITEM_SUBCLASS_CONSUMABLE ||
        proto->SubClass == ITEM_SUBCLASS_CONSUMABLE_OTHER)
    {
        if (IsProtectionConsumable(proto))
            return false;

        if (IsPotionLike(proto))
            return false;

        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
            if (!spellInfo)
                continue;

            if (proto->Spells[i].SpellCategory == 11 || proto->Spells[i].SpellCategory == 59)
            {
                if (IsStatBuffSpell(spellId))
                {
                    outSpellId = ResolveBuffAuraSpellId(spellId);
                    outType = ConsumableType::FoodBuff;
                    return true;
                }
                continue;
            }

            if (IsStatBuffSpell(spellId))
            {
                outSpellId = ResolveBuffAuraSpellId(spellId);
                outType = ConsumableType::FoodBuff;
                return true;
            }
        }
        return false;
    }

    if (proto->SubClass == ITEM_SUBCLASS_SCROLL)
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (proto->Spells[i].SpellId)
            {
                outSpellId = proto->Spells[i].SpellId;
                outType = ConsumableType::Scroll;
                return true;
            }
        }
        return false;
    }

    return false;
}

bool UseConsumableAction::HasActiveFlask() const
{
    for (uint32 id : knownFlaskAuras)
        if (ai->HasAura(id, bot))
            return true;
    return false;
}

bool UseConsumableAction::HasActiveFoodBuff() const
{
    for (uint32 id : knownFoodBuffAuras)
        if (ai->HasAura(id, bot))
            return true;

    // Well-fed
    if (ai->HasAura(19705, bot))
        return true;

    if (ai->HasAura("well fed", bot))
        return true;

    if (bot->IsNonMeleeSpellCasted(false, false, true))
        return true;

    return false;
}

bool UseConsumableAction::HasAnyCandidateAura(const ConsumableCandidate& candidate) const
{
    std::vector<uint32> auraIds;
    CollectPossibleAuraIds(candidate.spellId, auraIds);

    const ItemPrototype* proto = candidate.item->GetProto();
    if (proto)
    {
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            uint32 itemSpellId = proto->Spells[i].SpellId;
            if (itemSpellId)
            {
                CollectPossibleAuraIds(itemSpellId, auraIds);
            }
        }
    }

    for (uint32 id : auraIds)
    {
        if (ai->HasAura(id, bot))
            return true;
    }
    return false;
}

BotConsumableRole UseConsumableAction::GetBotRole() const
{
    const uint8 cls = bot->getClass();
    const int tab = AiFactory::GetPlayerSpecTab(bot);

    switch (cls)
    {
        // --- Warrior -----------------------------------------------------------
    case CLASS_WARRIOR:
        if (tab == 2)                           // Protection
            return BotConsumableRole::Tank;
        return BotConsumableRole::MeleeDps;     // Arms / Fury

        // --- Paladin -----------------------------------------------------------
    case CLASS_PALADIN:
        if (tab == 0)                           // Holy
            return BotConsumableRole::Healer;
        if (tab == 1)                           // Protection
            return BotConsumableRole::Tank;
        return BotConsumableRole::MeleeDps;     // Retribution

        // --- Hunter ------------------------------------------------------------
    case CLASS_HUNTER:
        return BotConsumableRole::RangedPhysDps; // All specs

        // --- Rogue -------------------------------------------------------------
    case CLASS_ROGUE:
        return BotConsumableRole::MeleeDps;     // All specs

        // --- Priest ------------------------------------------------------------
    case CLASS_PRIEST:
        if (tab == 2)                           // Shadow
            return BotConsumableRole::CasterDps;
        return BotConsumableRole::Healer;       // Discipline / Holy

        // --- Shaman ------------------------------------------------------------
    case CLASS_SHAMAN:
        if (tab == 0)                           // Elemental
            return BotConsumableRole::CasterDps;
        if (tab == 1)                           // Enhancement
            return BotConsumableRole::MeleeDps;
        return BotConsumableRole::Healer;       // Restoration

        // --- Mage --------------------------------------------------------------
    case CLASS_MAGE:
        return BotConsumableRole::CasterDps;    // All specs

        // --- Warlock -----------------------------------------------------------
    case CLASS_WARLOCK:
        return BotConsumableRole::CasterDps;    // All specs

        // --- Druid -------------------------------------------------------------
    case CLASS_DRUID:
        if (tab == 0)                           // Balance
            return BotConsumableRole::CasterDps;
        if (tab == 1)                           // Feral
        {
            // Thick Hide / feral tank talents → tank; otherwise melee DPS
            if (ai->IsTank(bot))
                return BotConsumableRole::Tank;
            return BotConsumableRole::MeleeDps;
        }
        return BotConsumableRole::Healer;       // Restoration

    default:
        return BotConsumableRole::MeleeDps;
    }
}

float UseConsumableAction::GetStatWeight(uint32 statType) const
{
    const BotConsumableRole role = GetBotRole();

    //                              STR   AGI   STA   INT   SPI
    // MeleeDps  — wants str/agi  1.0   1.0   0.3   0.1   0.05
    // RangedPhysDps — agi focus  0.1   1.0   0.3   0.3   0.05
    // CasterDps — int/sp         0.05  0.05  0.3   1.0   0.4
    // Healer    — int/spi/+heal  0.05  0.05  0.3   1.0   0.6
    // Tank      — sta/str/agi    0.6   0.8   1.0   0.1   0.05

    switch (statType)
    {
    case 0: // Strength
        switch (role)
        {
        case BotConsumableRole::MeleeDps:       return 1.0f;
        case BotConsumableRole::RangedPhysDps:  return 0.1f;
        case BotConsumableRole::CasterDps:      return 0.05f;
        case BotConsumableRole::Healer:         return 0.05f;
        case BotConsumableRole::Tank:           return 0.6f;
        }
        break;

    case 1: // Agility
        switch (role)
        {
        case BotConsumableRole::MeleeDps:       return 1.0f;
        case BotConsumableRole::RangedPhysDps:  return 1.0f;
        case BotConsumableRole::CasterDps:      return 0.05f;
        case BotConsumableRole::Healer:         return 0.05f;
        case BotConsumableRole::Tank:           return 0.8f;
        }
        break;

    case 2: // Stamina
        switch (role)
        {
        case BotConsumableRole::MeleeDps:       return 0.3f;
        case BotConsumableRole::RangedPhysDps:  return 0.3f;
        case BotConsumableRole::CasterDps:      return 0.3f;
        case BotConsumableRole::Healer:         return 0.3f;
        case BotConsumableRole::Tank:           return 1.0f;
        }
        break;

    case 3: // Intellect
        switch (role)
        {
        case BotConsumableRole::MeleeDps:       return 0.1f;
        case BotConsumableRole::RangedPhysDps:  return 0.3f;
        case BotConsumableRole::CasterDps:      return 1.0f;
        case BotConsumableRole::Healer:         return 1.0f;
        case BotConsumableRole::Tank:           return 0.1f;
        }
        break;

    case 4: // Spirit
        switch (role)
        {
        case BotConsumableRole::MeleeDps:       return 0.05f;
        case BotConsumableRole::RangedPhysDps:  return 0.05f;
        case BotConsumableRole::CasterDps:      return 0.4f;
        case BotConsumableRole::Healer:         return 0.6f;
        case BotConsumableRole::Tank:           return 0.05f;
        }
        break;
    }

    return 0.1f;
}

float UseConsumableAction::ScoreSpellStats(uint32 spellId) const
{
    const SpellEntry* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!spellInfo)
        return 0.0f;

    const BotConsumableRole role = GetBotRole();
    float score = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        float amount = (float)spellInfo->EffectBasePoints[i];
        if (amount <= 0)
            continue;

        switch (spellInfo->EffectApplyAuraName[i])
        {
        case SPELL_AURA_MOD_STAT:
        {
            int32 stat = spellInfo->EffectMiscValue[i];
            if (stat == -1) // all stats
            {
                for (uint32 s = 0; s < 5; ++s)
                    score += amount * GetStatWeight(s);
            }
            else if (stat >= 0 && stat <= 4)
            {
                score += amount * GetStatWeight((uint32)stat);
            }
            break;
        }
        case SPELL_AURA_MOD_ATTACK_POWER:
        {
            float w;
            switch (role)
            {
            case BotConsumableRole::MeleeDps:       w = 1.0f;  break;
            case BotConsumableRole::RangedPhysDps:  w = 0.8f;  break;
            case BotConsumableRole::Tank:           w = 0.5f;  break;
            default:                                w = 0.05f; break;
            }
            score += amount * w;
            break;
        }
        case SPELL_AURA_MOD_DAMAGE_DONE:
        {
            float w;
            switch (role)
            {
            case BotConsumableRole::CasterDps:      w = 1.0f;  break;
            case BotConsumableRole::Healer:         w = 0.3f;  break;
            default:                                w = 0.1f;  break;
            }
            score += amount * w;
            break;
        }
        case SPELL_AURA_MOD_HIT_CHANCE:
        case SPELL_AURA_MOD_CRIT_PERCENT:
        {
            score += amount * 10.0f;
            break;
        }
        case SPELL_AURA_MOD_HEALING_DONE:
        {
            float w = (role == BotConsumableRole::Healer) ? 1.0f : 0.05f;
            score += amount * w;
            break;
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            float w;
            switch (role)
            {
            case BotConsumableRole::Healer:         w = 3.0f; break;
            case BotConsumableRole::CasterDps:      w = 2.0f; break;
            default:                                w = 0.5f; break;
            }
            score += amount * w;
            break;
        }
        case SPELL_AURA_MOD_RESISTANCE:
        {
            score += amount * 0.2f;
            break;
        }
        default:
            score += amount * 0.1f;
            break;
        }
    }

    return score;
}

float UseConsumableAction::ScoreCandidate(const ConsumableCandidate& candidate) const
{
    float base = ScoreSpellStats(candidate.spellId);

    // Priority: Flask -> Elixir -> Scroll -> Food -> Alcohol
    switch (candidate.type)
    {
    case ConsumableType::Flask:    base += 1000.0f; break;
    case ConsumableType::Elixir:   base += 500.0f;  break;
    case ConsumableType::Scroll:   base += 300.0f;  break;
    case ConsumableType::FoodBuff: base += 200.0f;  break;
    case ConsumableType::Alcohol:  base += 100.0f;  break;
    default: break;
    }

    return base;
}

std::vector<ConsumableCandidate> UseConsumableAction::CollectCandidates()
{
    std::vector<ConsumableCandidate> results;

    for (int bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (!pBag)
            continue;

        for (uint32 slot = 0; slot < pBag->GetBagSize(); ++slot)
        {
            Item* item = bot->GetItemByPos(bag, slot);
            if (!item)
                continue;

            const ItemPrototype* proto = item->GetProto();
            if (!proto || proto->Class != ITEM_CLASS_CONSUMABLE)
                continue;

            if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
                continue;

            ConsumableType type;
            uint32 spellId;
            if (!ClassifyConsumable(proto, type, spellId))
                continue;

            ConsumableCandidate c;
            c.item = item;
            c.spellId = spellId;
            c.type = type;
            c.score = 0.0f;
            results.push_back(c);
        }
    }

    for (uint32 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot);
        if (!item)
            continue;

        const ItemPrototype* proto = item->GetProto();
        if (!proto || proto->Class != ITEM_CLASS_CONSUMABLE)
            continue;

        if (bot->CanUseItem(proto) != EQUIP_ERR_OK)
            continue;

        ConsumableType type;
        uint32 spellId;
        if (!ClassifyConsumable(proto, type, spellId))
            continue;

        ConsumableCandidate c;
        c.item = item;
        c.spellId = spellId;
        c.type = type;
        c.score = 0.0f;
        results.push_back(c);
    }

    return results;
}

bool UseConsumableAction::isPossible()
{
    return !bot->InBattleGround() && !bot->IsTaxiFlying();
}

bool UseConsumableAction::isUseful()
{
    if (bot->IsInCombat())
        return false;

    if (ai->HasCheat(BotCheatMask::item))
        return false;

    if (bot->IsTaxiFlying() || bot->InBattleGround())
        return false;

    if (bot->GetLevel() < 5)
        return false;

    return true;
}

bool UseConsumableAction::Execute(Event& event)
{
    selectedItemId = 0;

    if (bot->IsNonMeleeSpellCasted(false, false, true))
        return false;

    std::vector<ConsumableCandidate> candidates = CollectCandidates();
    if (candidates.empty())
        return false;

    for (auto& c : candidates)
        c.score = ScoreCandidate(c);

    std::sort(candidates.begin(), candidates.end(), [](const ConsumableCandidate& a, const ConsumableCandidate& b)
        {
            return a.score > b.score;
        });

    bool hasFlask = HasActiveFlask();
    bool hasFoodBuff = HasActiveFoodBuff();

    for (const auto& c : candidates)
    {
        if (HasAnyCandidateAura(c))
            continue;

        switch (c.type)
        {
        case ConsumableType::Flask:
        {
            if (hasFlask)
                continue;

            selectedItemId = c.item->GetProto()->ItemId;
            break;
        }
        case ConsumableType::Elixir:
        {
            selectedItemId = c.item->GetProto()->ItemId;
            break;
        }
        case ConsumableType::Alcohol:
        {
            selectedItemId = c.item->GetProto()->ItemId;
            break;
        }
        case ConsumableType::FoodBuff:
        {
            if (hasFoodBuff)
                continue;

            if (IsFoodOrDrink(c.item->GetProto()))
                ai->StopMoving();

            selectedItemId = c.item->GetProto()->ItemId;
            break;
        }
        case ConsumableType::Scroll:
        {
            selectedItemId = c.item->GetProto()->ItemId;
            break;
        }
        default:
            continue;
        }

        if (selectedItemId)
            break;
    }

    if (!selectedItemId)
        return false;

    return UseItemIdAction::Execute(event);
}