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
    for (auto& itemId : sRandomItemMgr.GetGlyphs(classId))
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
    for (auto& itemId : sRandomItemMgr.GetGlyphs(classId))
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
    for (uint32 i = 0; i < sGlyphSlotStore.GetNumRows(); ++i)
        if (GlyphSlotEntry const* gs = sGlyphSlotStore.LookupEntry(i))
            if (bot->GetGlyph(gs->Id) == glyphId)
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

            if(!WantsGlyphFromStrategy(currentGlyphId, bot)) //Strategy doesn't need this glyph so we can replace it.
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

bool SetGlyphAction::WantsGlyphFromStrategy(uint32 glyphId, Player* bot)
{
    std::list<TriggerNode*> triggerNodes;

    const ItemPrototype* currentGlyphProto = GetGlyphProtoFromGlyphId(glyphId, bot->getClass());

    bool foundTrigger = false;

    PlayerbotAI* ai = bot->GetPlayerbotAI();

    for (auto& strategy : ai->GetAiObjectContext()->GetSupportedStrategies())
    {
        if (!ai->HasStrategy(strategy, BotState::BOT_STATE_COMBAT))
            continue;

        Strategy* combatStrategy = ai->GetAiObjectContext()->GetStrategy(strategy);

        combatStrategy->InitTriggers(triggerNodes, BotState::BOT_STATE_NON_COMBAT);

        //Loop over all triggers of this strategy.
        for (auto& triggerNode : triggerNodes)
        {
            if (triggerNode->getName().find("set glyph::") != 0)
                continue;

            std::string triggerQualifier = triggerNode->getName().substr(11);

            Tokens tokens = getMultiQualifiers(triggerQualifier, ",");

            std::string glyphName = "glyph of " + tokens[0];

            if (currentGlyphProto->Name1 != glyphName)
                continue;

            if (tokens.size() > 1)
            {
                uint32 requiredTalentSpellId = stoi(tokens[1]);

                if (!bot->HasSpell(requiredTalentSpellId))
                    continue;
            }

            foundTrigger = true;
            break;
        }

        for (std::list<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
        {
            TriggerNode* trigger = *i;
            delete trigger;
        }

        triggerNodes.clear();

        if (foundTrigger)
            break;
    }

    return foundTrigger;
}