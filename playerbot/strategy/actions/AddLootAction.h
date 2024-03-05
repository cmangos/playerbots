#pragma once
#include "GenericActions.h"

namespace ai
{
    class AddLootAction : public ChatCommandAction
    {
    public:
        AddLootAction(PlayerbotAI* ai) : ChatCommandAction(ai, "add loot") {}
        virtual bool isUseful();

    private:
        bool Execute(Event& event) override;
    };

    class AddAllLootAction : public ChatCommandAction
    {
    public:
        AddAllLootAction(PlayerbotAI* ai, std::string name = "add all loot") : ChatCommandAction(ai, name) {}
        virtual bool isUseful();

    protected:
        virtual bool AddLoot(Player* requester, ObjectGuid guid);
        virtual bool Execute(Event& event) override;
    };

    class AddGatheringLootAction : public AddAllLootAction
    {
    public:
        AddGatheringLootAction(PlayerbotAI* ai) : AddAllLootAction(ai, "add gathering loot") {}

    private:
        bool AddLoot(Player* requester, ObjectGuid guid) override;
    };
}
