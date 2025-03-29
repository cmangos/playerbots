#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/playerbot.h"
#include "GlyphAction.h"
#include "playerbot/RandomItemMgr.h"

using namespace ai;

bool SetGlyphAction::Execute(Event& event)
{
#ifdef MANGOSBOT_TWO
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    uint8 glyphSlot = MAX_GLYPH_SLOT_INDEX;
    const ItemPrototype* glyphProto = nullptr;
    std::string glyphName = "Glyph of " + qualifier;

    if (!event.getParam().empty())
    {
        std::string glyphName = event.getParam();

        ItemIds ids = ChatHelper::parseItems(glyphName);

        if (!ids.empty())
        {
            glyphProto = sObjectMgr.GetItemPrototype(*ids.begin());

            if (glyphProto)
            {
                if (glyphName.find("|r ") != std::string::npos)
                {
                    if (isValidNumberString(glyphName.substr(glyphName.find("|r ") + 3)))
                        glyphSlot = stoi(glyphName.substr(glyphName.find("|r ") + 3));
                }

                glyphName = glyphProto->Name1;
            }
        }
    }

    if (!glyphProto)
       glyphProto = GetGlyphProtoFromName(glyphName, bot->getClass());
    
    if (!glyphProto)
    {
        ai->TellError(requester, "Glyph " + glyphName + " does not exists");
        return false;
    }

    uint32 glyphId = GetGlyphIdFromProto(glyphProto);

    if (glyphProto->Class != ITEM_CLASS_GLYPH || !glyphId)
    {
        ai->TellError(requester, glyphName + " is not a valid glyph");
        return false;
    }

    if (glyphProto->AllowableClass != bot->getClass())
    {
        ai->TellError(requester, glyphName + " is not a valid glyph for this class");
        return false;
    }

    Item* glyphItem = ItemUsageValue::CurrentItem(glyphProto, bot);

    if (!glyphItem && !ai->HasCheat(BotCheatMask::item))
    {
        ai->TellError(requester, "Glyph " + glyphName + " not found in inventory");
        return false;
    }

    glyphSlot = GetBestSlotForGlyph(glyphId, bot, glyphSlot);

    if (glyphSlot == MAX_GLYPH_SLOT_INDEX)
    {
        ai->TellError(requester, "No proper slot found for Glyph " + glyphName);
        return false;
    }

    if (!isGlyphSlotEnabled(glyphSlot, bot->GetLevel()))
    {
        ai->TellError(requester, "Slot " + std::to_string(glyphSlot) + " is not enabled");
        return false;
    }

    if (isGlyphAlreadySet(glyphId, bot))
    {
        ai->TellError(requester, "Glyph " + glyphName + " is already active");
        return false; //Bot already has glyph applied.
    }

    //Refactor to use packet/use item action later.
    bot->ApplyGlyph(glyphSlot, false);
    bot->SetGlyph(glyphSlot, glyphId);
    bot->ApplyGlyph(glyphSlot, true);
    bot->SendTalentsInfoData(false);

    uint32 count = 1;

    if (!ai->HasCheat(BotCheatMask::item))
        bot->DestroyItemCount(glyphItem, count, true);

    ai->TellPlayerNoFacing(requester, "Applied " + glyphName, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
#endif;
    return true;
}

bool SetGlyphAction::isUseful()
{
    return true;
}

const ItemPrototype* SetGlyphAction::GetGlyphProtoFromName(std::string glyphName, uint32 classId)
{
    //Glyph of Molten Armor
    for (auto& itemId : sRandomItemMgr.GetGlyphs(1 << (classId - 1)))
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

        if (!proto)
            continue;

        if (proto->Name1 == glyphName)
            return proto;
    }

    return nullptr;
}

uint32 SetGlyphAction::GetGlyphIdFromProto(const ItemPrototype* glyphProto)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(glyphProto->Spells[0].SpellId);

    if (!spellInfo)
        return 0;

    return spellInfo->EffectMiscValue[0];
}

const ItemPrototype* SetGlyphAction::GetGlyphProtoFromGlyphId(uint32 glyphId, uint32 classId)
{
    //Glyph of Molten Armor
    for (auto& itemId : sRandomItemMgr.GetGlyphs(1<<(classId - 1)))
    {
        ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

        if (!proto)
            continue;

        if (GetGlyphIdFromProto(proto) == glyphId)
            return proto;
    }

    return nullptr;
}

bool SetGlyphAction::isGlyphSlotEnabled(uint8 slot, uint32 level)
{
    if (level >= 15 && (slot == 0 || slot == 1))
        return true;
    if (level >= 30 && slot == 3)
        return true;
    if (level >= 50 && slot == 2)
        return true;
    if (level >= 70 && slot == 4)
        return true;
    if (level >= 80 && slot == 5)
        return true;

    return false;
}

bool SetGlyphAction::isGlyphAlreadySet(uint32 glyphId, Player* bot)
{
#ifdef MANGOSBOT_TWO
    for (uint32 glyphIndex = 0; glyphIndex < MAX_GLYPH_SLOT_INDEX; ++glyphIndex)
        if (GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(bot->GetGlyphSlot(glyphIndex)))
            if (bot->GetGlyph(glyphIndex) == glyphId)
                return true;
#endif;
    return false;
}

uint8 SetGlyphAction::GetBestSlotForGlyph(uint32 glyphId, Player* bot, uint8 wantedSlot)
{
#ifdef MANGOSBOT_TWO
    uint32 minLevel = 9999;
    uint8 bestSlot = MAX_GLYPH_SLOT_INDEX;
    std::map<uint32, uint8> currentLevelSlot;

    GlyphPropertiesEntry const* gp = sGlyphPropertiesStore.LookupEntry(glyphId);

    for (uint32 glyphIndex = 0; glyphIndex < MAX_GLYPH_SLOT_INDEX; ++glyphIndex)
    {
        if (GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(bot->GetGlyphSlot(glyphIndex)))
        {
            if (wantedSlot != MAX_GLYPH_SLOT_INDEX && wantedSlot != glyphIndex)
                continue;

            if (gp->TypeFlags != gs->TypeFlags) //Glyph doesn't fit in this slot.
                continue;

            if (!isGlyphSlotEnabled(glyphIndex, bot->GetLevel())) //Glyphslot isn't enabled
                continue;

            uint32 currentGlyphId = bot->GetGlyph(glyphIndex);

            const ItemPrototype* currentGlyphProto = GetGlyphProtoFromGlyphId(currentGlyphId, bot->getClass());

            if (!currentGlyphProto) //Empty slot put it here.
                return glyphIndex;

            if(!WantsGlyphFromConfig(currentGlyphId, bot)) //Strategy doesn't need this glyph so we can replace it.
                return glyphIndex;

            if (currentGlyphProto->RequiredLevel >= minLevel)
                continue;

            minLevel = currentGlyphProto->RequiredLevel;
            bestSlot = glyphIndex;
        }
    }
    return bestSlot;
#else
    return 0;
#endif;
}

GlyphPriorityList SetGlyphAction::GetGlyphPriorityList(Player* bot)
{
    GlyphPriorityList glyphPriorityList;

    uint32 specNo = sRandomPlayerbotMgr.GetValue(bot->GetGUIDLow(), "specNo");

    if (!specNo)
        return glyphPriorityList;

    uint32 specId = specNo ? specNo - 1 : 0;

    uint32 listLevel = 999;

    for (auto& [level, glyphList] : sPlayerbotAIConfig.glyphPriorityMap[bot->getClass()][specId])
    {
        if (level < listLevel && level >= bot->GetLevel()) //Find the lowest level list that is above the bots current level.
        {
            glyphPriorityList = glyphList;
            listLevel = level;
        }
    }

    return glyphPriorityList;
}

bool SetGlyphAction::WantsGlyphFromConfig(uint32 glyphId, Player* bot)
{
    for (auto [glyphConfigName, reqTalentSpellId] : GetGlyphPriorityList(bot))
    {
        std::string glyphName = "Glyph of " + glyphConfigName;

        if (reqTalentSpellId && !bot->HasSpell(reqTalentSpellId))
            continue;

        const ItemPrototype* glyphProto = GetGlyphProtoFromName(glyphName, bot->getClass());

        if (!glyphProto)
        {
            sLog.outError("%s is not found for class %d", glyphName.c_str(), bot->getClass());
            continue;
        }

        uint32 wantGlyphId = GetGlyphIdFromProto(glyphProto);

        if (wantGlyphId == glyphId)
            return true;
    }

    return false;
}


bool AutoSetGlyphAction::Execute(Event& event)
{
    bool didGlyph = false;
#ifdef MANGOSBOT_TWO

    GlyphPriorityList glyphPriorityList = GetGlyphPriorityList(bot);

    std::reverse(glyphPriorityList.begin(), glyphPriorityList.end()); //Try last glyphs first.

    std::vector<uint32> betterGlyphs;

    for (auto& [glyphConfigName, reqTalentSpellId] : glyphPriorityList)
    {
        std::string glyphName = "Glyph of " + glyphConfigName;

        //We do not have the talent which is configured to be needed for this glyph.
        if (reqTalentSpellId && !bot->HasSpell(reqTalentSpellId))
            continue;

        const ItemPrototype* glyphProto = GetGlyphProtoFromName(glyphName, bot->getClass());

        if (!glyphProto)
        {
            sLog.outError("%s is not found for class %d", glyphName.c_str(), bot->getClass());
            continue;
        }

        //We can not use this glyph.
        if (glyphProto->RequiredLevel > bot->GetLevel())
            continue;

        uint32 glyphId = GetGlyphIdFromProto(glyphProto);

        betterGlyphs.push_back(glyphId);

        uint32 bestSlot = GetBestSlotForGlyph(glyphId, bot, MAX_GLYPH_SLOT_INDEX);

        uint32 currentGlyphId = bot->GetGlyph(bestSlot);

        //A earlier glyph is already in this slot.
        if (std::find(betterGlyphs.begin(), betterGlyphs.end(), currentGlyphId) != betterGlyphs.end())
            continue;

        qualifier = glyphConfigName;

        didGlyph = didGlyph || SetGlyphAction::Execute(event);
    }

#endif;
    return didGlyph;
}