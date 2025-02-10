#pragma once
#include "GenericSpellActions.h"

namespace ai
{
    class InitializePetAction : public Action
    {
    public:
        InitializePetAction(PlayerbotAI* ai) : Action(ai, "initialize pet") {}
        bool Execute(Event& event) override;
        bool isUseful() override;

    private:
        void InitialFamilySkill(Pet* pet);
    };

    class InitializePetSpellsAction : public Action
    {
    public:
        InitializePetSpellsAction(PlayerbotAI* ai) : Action(ai, "initialize pet spells") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
        HunterPetBuildPath* PickPremadePath(std::vector<HunterPetBuildPath*> paths, bool useProbability);
        HunterPetBuild* GetBestPremadeBuild(int specId);
        HunterPetBuildPath* getPremadePath(int id);
    };

    class SetPetAction : public Action
    {
    public:
        SetPetAction(PlayerbotAI* ai) : Action(ai, "pet") {}
        bool Execute(Event& event) override;
        virtual bool AutoSelectBuild(Player* bot, std::ostringstream* out);

    private:
        std::vector<HunterPetBuildPath*> getPremadePaths(std::string findName);
        std::vector<HunterPetBuildPath*> getPremadePaths(int id);
        void listPremadePaths(std::vector<HunterPetBuildPath*> paths, std::ostringstream* out);
        HunterPetBuildPath* PickPremadePath(std::vector<HunterPetBuildPath*> paths, bool useProbability);
        HunterPetBuild* GetBestPremadeBuild(int specId);
        HunterPetBuildPath* getPremadePath(int id);
    };
}