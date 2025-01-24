#include "GenericSpellActions.h"

namespace ai
{
    class InitializePetAction : public Action
    {
    public:
        InitializePetAction(PlayerbotAI* ai) : Action(ai, "initialize pet") {}
        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    class SetPetAction : public Action
    {
    public:
        SetPetAction(PlayerbotAI* ai) : Action(ai, "pet") {}
        bool Execute(Event& event) override;

    private:
        std::vector<HunterPetBuildPath*> getPremadePaths(std::string findName);
        void listPremadePaths(std::vector<HunterPetBuildPath*> paths, std::ostringstream* out);
    };
}