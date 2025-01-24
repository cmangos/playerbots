#pragma once

#include "playerbot/strategy/actions/GenericActions.h"

namespace ai
{
    class ChangePetBuildAction : public ChatCommandAction
    {
    public:
        ChangePetBuildAction(PlayerbotAI* ai, std::string name = "pet-build") : ChatCommandAction(ai, name) {}

    public:
        virtual bool Execute(Event& event) override;
        virtual bool isUseful();

    private:
        std::vector<HunterPetBuildPath*> getPremadePaths(std::string findName);
        void listPremadePaths(std::vector<HunterPetBuildPath*> paths, std::ostringstream* out);
    };
}

