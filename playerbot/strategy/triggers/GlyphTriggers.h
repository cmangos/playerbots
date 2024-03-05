#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/strategy/Trigger.h"

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
}