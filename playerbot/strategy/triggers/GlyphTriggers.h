#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/strategy/Trigger.h"
#include "playerbot/strategy/actions/GlyphAction.h"

namespace ai
{
    class LearnGlyphTrigger : public Trigger
    {
    public:
        LearnGlyphTrigger(PlayerbotAI* ai, std::string triggerName, int glyphId, int requiredSpellId = -1, int requiredLevel = -1, std::string requiredCombatStrategy = "") : Trigger(ai, triggerName, 60) {
            this->glyphId = glyphId;
            this->requiredSpellId = requiredSpellId;
            this->requiredLevel = requiredLevel;
            this->requiredCombatStrategy = requiredCombatStrategy;
        }
        virtual bool IsActive()
        {
            if (glyphId == 0 || bot->HasSpell(glyphId))
                return false;

            if (requiredSpellId != -1 && !bot->HasSpell(requiredSpellId))
                return false;

            if (requiredLevel != -1 && (int)bot->GetLevel() < requiredLevel)
                return false;

            if (!requiredCombatStrategy.empty() && !ai->HasStrategy(requiredCombatStrategy, BotState::BOT_STATE_COMBAT))
                return false;

            return true;
        }

    private:
        int glyphId, requiredSpellId, requiredLevel;
        std::string requiredCombatStrategy;
    };

    class RemoveGlyphTrigger : public Trigger
    {
    public:
        RemoveGlyphTrigger(PlayerbotAI* ai, std::string triggerName, int glyphId, int requiredSpellId = -1, int requiredLevel = -1, std::string requiredCombatStrategy = "") : Trigger(ai, triggerName, 60) {
            this->glyphId = glyphId;
            this->requiredSpellId = requiredSpellId;
            this->requiredLevel = requiredLevel;
            this->requiredCombatStrategy = requiredCombatStrategy;
        }
        virtual bool IsActive()
        {
            if (glyphId == 0 || !bot->HasSpell(glyphId))
                return false;

            if (requiredSpellId != -1 && !bot->HasSpell(requiredSpellId))
                return true;

            if (requiredLevel != -1 && (int)bot->GetLevel() < requiredLevel)
                return true;

            if (!requiredCombatStrategy.empty() && !ai->HasStrategy(requiredCombatStrategy, BotState::BOT_STATE_COMBAT))
                return true;

            return false;
        }

    private:
        int glyphId, requiredSpellId, requiredLevel;
        std::string requiredCombatStrategy;
    };

    class SetGlyphTrigger : public Trigger, public Qualified
    {
    public:
        SetGlyphTrigger(PlayerbotAI* ai, std::string triggerName = "set glyph") : Trigger(ai, triggerName, 60) {}
        virtual bool IsActive()
        {
#ifdef MANGOSBOT_TWO
            Tokens tokens = getMultiQualifiers(qualifier, ",");

            std::string glyphName = "Glyph of " + tokens[0];

            if (tokens.size() > 1)
            {
                uint32 requiredTalentSpellId = stoi(tokens[1]);

                if (!bot->HasSpell(requiredTalentSpellId))
                    return false;
            }

            const ItemPrototype* glyphProto = SetGlyphAction::GetGlyphProtoFromName(glyphName, bot->getClass());

            if (!glyphProto)
                return false;

            if (glyphProto->RequiredLevel > bot->GetLevel())
                return false;

            uint32 glyphId = SetGlyphAction::GetGlyphIdFromProto(glyphProto);

            if (SetGlyphAction::isGlyphAlreadySet(glyphId, bot))
                return false;

            if (SetGlyphAction::GetBestSlotForGlyph(glyphId, bot, MAX_GLYPH_SLOT_INDEX) == MAX_GLYPH_SLOT_INDEX)
                return false;
#endif;
            return true;
        }
    };    
}