#pragma once

#include "playerbot/playerbot.h"
#include "../Trigger.h"

namespace ai
{
    class LearnGlyphTrigger : public Trigger
    {
    public:
        LearnGlyphTrigger(PlayerbotAI* ai, string triggerName, int glyphId, int requiredSpellId = -1, int requiredLevel = -1, string requiredCombatStrategy = "") : Trigger(ai, triggerName, 60) {
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
        string requiredCombatStrategy;
    };

    class RemoveGlyphTrigger : public Trigger
    {
    public:
        RemoveGlyphTrigger(PlayerbotAI* ai, string triggerName, int glyphId, int requiredSpellId = -1, int requiredLevel = -1, string requiredCombatStrategy = "") : Trigger(ai, triggerName, 60) {
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
        string requiredCombatStrategy;
    };
}