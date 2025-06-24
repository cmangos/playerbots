#pragma once
#include "GenericActions.h"

namespace ai
{
    class DebugAction : public ChatCommandAction
    {
    public:
        DebugAction(PlayerbotAI* ai) : ChatCommandAction(ai, "debug") {}
        virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "debug"; }
        virtual std::string GetHelpDescription()
        {
            return "This command enables debug functionalities for testing bot behaviors.\n"
                   "Usage: cdebug [subcommand]\n"
                   "Please refer to the source code for specific uses.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 

        void FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target = ObjectGuid(), std::list<ObjectGuid> otherTargets = {}, std::list<ObjectGuid> missTargets = {}, WorldPosition source = WorldPosition(), WorldPosition dest = WorldPosition(), bool forceDest = false);
        void addAura(uint32 spellId, Unit* target);
    };
}
