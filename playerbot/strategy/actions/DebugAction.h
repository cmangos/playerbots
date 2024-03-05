#pragma once
#include "GenericActions.h"

namespace ai
{
    class DebugAction : public ChatCommandAction
    {
    public:
        DebugAction(PlayerbotAI* ai) : ChatCommandAction(ai, "debug") {}
        virtual bool Execute(Event& event) override;
        void FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target = ObjectGuid(), std::list<ObjectGuid> otherTargets = {}, std::list<ObjectGuid> missTargets = {}, WorldPosition source = WorldPosition(), WorldPosition dest = WorldPosition(), bool forceDest = false);
        void addAura(uint32 spellId, Unit* target);
    };
}
