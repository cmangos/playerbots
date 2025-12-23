#pragma once
#include "Config/Config.h"
#include "Category.h"


namespace ahbot
{
    class Alchemy : public Consumable
    {
    public:
        Alchemy() : Consumable() {}

    public:
        virtual bool Contains(ItemPrototype const* proto) override
        {
            return Consumable::Contains(proto) &&
                    (proto->SubClass == ITEM_SUBCLASS_POTION ||
                    proto->SubClass == ITEM_SUBCLASS_ELIXIR ||
                    proto->SubClass == ITEM_SUBCLASS_FLASK);
        }

        virtual std::string GetName() override { return "consumables.alchemy"; }
        virtual std::string GetLabel() override { return "elixirs and potions"; }
    };

    class Scroll : public Consumable
    {
    public:
        Scroll() : Consumable() {}

    public:
        virtual bool Contains(ItemPrototype const* proto) override
        {
            return Consumable::Contains(proto) &&
                    (proto->SubClass == ITEM_SUBCLASS_SCROLL ||
                    proto->SubClass == ITEM_SUBCLASS_ITEM_ENHANCEMENT);
        }

        virtual std::string GetName() override { return "consumables.scroll"; }
        virtual std::string GetLabel() override { return "scrolls"; }
    };

    class Food : public Consumable
    {
    public:
        Food() : Consumable() {}

    public:
        virtual bool Contains(ItemPrototype const* proto) override
        {
            return Consumable::Contains(proto) &&
                    (proto->SubClass == ITEM_SUBCLASS_FOOD
#ifdef MANGOSBOT_ZERO
                    || proto->SubClass == ITEM_SUBCLASS_CONSUMABLE
#endif
                    );
        }

        virtual std::string GetName() override { return "consumables.food"; }
        virtual std::string GetLabel() override { return "food and drink"; }
    };

    class Bandage : public Consumable
    {
    public:
        Bandage() : Consumable() {}

    public:
        virtual bool Contains(ItemPrototype const* proto) override
        {
            return Consumable::Contains(proto) &&
                    proto->SubClass == ITEM_SUBCLASS_BANDAGE;
        }

        virtual std::string GetName() override { return "consumables.bandage"; }
        virtual std::string GetLabel() override { return "bandages"; }
    };

    class ItemEnchant : public Consumable
    {
    public:
        ItemEnchant() : Consumable() {}

    public:
        virtual bool Contains(ItemPrototype const* proto) override
        {
            return Consumable::Contains(proto) &&
                    proto->SubClass == ITEM_SUBCLASS_CONSUMABLE_OTHER;
        }

        virtual std::string GetName() override { return "consumables.enchant"; }
        virtual std::string GetLabel() override { return "item enchants"; }
    };
};
