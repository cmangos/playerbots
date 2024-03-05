#pragma once
#include "GenericActions.h"

namespace ai
{
    class EquipAction : public ChatCommandAction
    {
    public:
        EquipAction(PlayerbotAI* ai, std::string name = "equip") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        void EquipItems(Player* requester, ItemIds ids);
        void EquipItem(Player* requester, Item* item);

    private:
        void EquipItem(Player* requester, FindItemVisitor* visitor);
        void ListItems(Player* requester);
        uint8 GetSmallestBagSlot();
    };

    class EquipUpgradesAction : public EquipAction
    {
    public:
        EquipUpgradesAction(PlayerbotAI* ai, std::string name = "equip upgrades") : EquipAction(ai, name) {}
        virtual bool Execute(Event& event) override;
    };
}
