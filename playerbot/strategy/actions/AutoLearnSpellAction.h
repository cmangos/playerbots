#pragma once


#include "playerbot/playerbot.h"
#include "playerbot/strategy/Action.h"

namespace ai
{
    class AutoLearnSpellAction : public Action {
    public:
        AutoLearnSpellAction(PlayerbotAI* ai, std::string name = "auto learn spell") : Action(ai, name) {}
        
    public:
        virtual bool Execute(Event& event);

    private: 
        void LearnSpells(std::ostringstream* out);
        void LearnTrainerSpells(std::ostringstream* out);
        void LearnQuestSpells(std::ostringstream* out);
        void GetClassQuestItem(Quest const* quest, std::ostringstream* out);
        bool LearnSpell(uint32 spellId, std::ostringstream* out);
    };
}
