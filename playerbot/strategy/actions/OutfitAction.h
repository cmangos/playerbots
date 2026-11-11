#pragma once
#include "playerbot/LootObjectStack.h"
#include "EquipAction.h"

namespace ai
{
    class OutfitAction : public EquipAction 
    {
    public:
        OutfitAction(PlayerbotAI* ai, std::string name = "outfit") : EquipAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    protected:
        void List(Player* requester);
        void Save(std::string name, ItemIds outfit);
        void Update(std::string name);
    };

    class ResistAction : public OutfitAction
    {
    public:
        ResistAction(PlayerbotAI* ai) : OutfitAction(ai, "resist") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        std::string FindResistOutfit(const std::string& school);
        bool WearsResistOutfit();
        void ChangeResistStrategy(const std::string& previousSchool, const std::string& school);
    };
}
