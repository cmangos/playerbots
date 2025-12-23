#pragma once
#include "Category.h"

struct ItemPrototype;
struct SpellEntry;

namespace ahbot
{
    class TradeSkill : public Trade
    {
    public:
        TradeSkill(uint32 skill, bool reagent) : Trade(), skill(skill), reagent(reagent) {}

    public:
        virtual bool Contains(ItemPrototype const* proto) override;
        virtual std::string GetName() override;
        virtual std::string GetMainName();
        virtual std::string GetLabel() override;
        virtual uint32 GetSkillId() override { return skill; }

        virtual uint32 GetMaxAllowedAuctionCount() override
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(GetMainName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5)));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto) override
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(GetMainName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType)));
        }
    protected:
        std::string typeName = "trade";
        int32 defaultMaxType = 5;

    private:
        bool ContainsInternal(ItemPrototype const* proto);
        bool IsCraftedBySpell(ItemPrototype const* proto, uint32 spellId);
        bool IsCraftedBy(ItemPrototype const* proto, uint32 craftId);
        bool IsCraftedBySpell(ItemPrototype const* proto, SpellEntry const *entry);
        uint32 skill;
        std::set<uint32> itemCache;
        bool reagent;
    };

};
