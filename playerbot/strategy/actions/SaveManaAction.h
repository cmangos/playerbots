#pragma once
#include "GenericActions.h"

namespace ai
{
    class SaveManaAction : public ChatCommandAction
    {
    public:
        SaveManaAction(PlayerbotAI* ai) : ChatCommandAction(ai, "save mana") {}
        virtual bool Execute(Event& event) override;

    private:
        std::string format(double value);
    };
}
