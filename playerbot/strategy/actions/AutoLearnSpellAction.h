#pragma once


#include "playerbot/playerbot.h"
#include "playerbot/strategy/Action.h"

namespace ai
{
    class AutoLearnSpellAction : public Action {
    public:
        AutoLearnSpellAction(PlayerbotAI* ai, string name = "auto learn spell") : Action(ai, name) {}

    public:
        virtual bool Execute(Event& event);

    private: 
        void LearnSpells(ostringstream* out);
        void LearnTrainerSpells(ostringstream* out);
        void LearnQuestSpells(ostringstream* out);
        void LearnSpell(uint32 spellId, ostringstream* out);
    };
}
