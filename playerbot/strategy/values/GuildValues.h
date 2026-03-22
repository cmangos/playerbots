#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    enum class GuildOrderType : uint8
    {
        None = 0,
        Farm,
        Kill,
        Explore,
        Craft
    };

    struct GuildOrder
    {
        GuildOrderType type = GuildOrderType::None;
        std::string target;
        uint32 amount = 0; // Used for Craft and Farm orders

        bool IsValid() const { return type != GuildOrderType::None && !target.empty(); }
        bool IsTravelOrder() const { return type == GuildOrderType::Farm || type == GuildOrderType::Kill || type == GuildOrderType::Explore; }
        bool IsCraftOrder() const { return type == GuildOrderType::Craft; }

        std::string GetTypeName() const
        {
            switch (type)
            {
            case GuildOrderType::Farm:    return "Farm";
            case GuildOrderType::Kill:    return "Kill";
            case GuildOrderType::Explore: return "Explore";
            case GuildOrderType::Craft:   return "Craft";
            default:                      return "None";
            }
        }
    };

    class GuildOrderValue : public CalculatedValue<GuildOrder>
    {
    public:
        GuildOrderValue(PlayerbotAI* ai) : CalculatedValue<GuildOrder>(ai, "guild order", 60) {}

        GuildOrder Calculate() override;

        static uint32 FindItemByName(const std::string& name);
    private:
        static std::string TrimWhitespace(const std::string& str);
        static bool ParseOrderPrefix(const std::string& note, const std::string& prefix, std::string& outBody);
    };

    class HasGuildOrderValue : public BoolCalculatedValue
    {
    public:
        HasGuildOrderValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has guild order", 10) {}

        bool Calculate() override { return AI_VALUE(GuildOrder, "guild order").IsValid(); }
    };

    class HasGuildTravelOrderValue : public BoolCalculatedValue
    {
    public:
        HasGuildTravelOrderValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has guild travel order", 10) {}

        bool Calculate() override
        {
            GuildOrder order = AI_VALUE(GuildOrder, "guild order");
            if (!order.IsTravelOrder())
                return false;

            // If a Farm order has an amount, check inventory count.
            if (order.type == GuildOrderType::Farm && order.amount > 0)
            {
                uint32 itemId = GuildOrderValue::FindItemByName(order.target);
                if (itemId)
                {
                    uint32 currentCount = ai->GetInventoryItemsCountWithId(itemId);
                    if (currentCount >= order.amount)
                        return false;
                }
            }

            return true;
        }
    };

    class HasGuildCraftOrderValue : public BoolCalculatedValue
    {
    public:
        HasGuildCraftOrderValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has guild craft order", 10) {}

        bool Calculate() override { return AI_VALUE(GuildOrder, "guild order").IsCraftOrder(); }
    };

    class NeedsProfessionReagentsValue : public BoolCalculatedValue
    {
    public:
        NeedsProfessionReagentsValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "needs profession reagents", 60) {}

        bool Calculate() override;

        static std::vector<uint32> GetMissingReagents(PlayerbotAI* ai);
    };

    enum class GuildShareFilter : uint8
    {
        FILTER_CLASS = 0,
        FILTER_ALL,
        FILTER_MELEE,
        FILTER_RANGED,
        FILTER_TANK,
        FILTER_DPS,
        FILTER_HEAL,
    };

    struct GuildShareItemEntry
    {
        GuildShareFilter filter = GuildShareFilter::FILTER_CLASS;
        uint8 playerClass = 0;
        uint32 itemId = 0;
        uint32 amount = 0;

        bool MatchesPlayer(Player* player) const;
    };

    uint32 CountGuildFinishedItemDeficit(Player* bot, uint32 itemId, const std::vector<GuildShareItemEntry>& shareList);

    class GuildShareListValue : public CalculatedValue<std::vector<GuildShareItemEntry>>
    {
    public:
        GuildShareListValue(PlayerbotAI* ai) : CalculatedValue<std::vector<GuildShareItemEntry>>(ai, "guild share list", 30) {}

        std::vector<GuildShareItemEntry> Calculate() override;

        static uint8 ParseClassName(const std::string& name);
        static GuildShareFilter ParseRoleFilter(const std::string& name);
    };

    // Represents an item a nearby guild member needs, paired with the receiver.
    struct GuildShareTarget
    {
        Player* receiver = nullptr;
        uint32 itemId = 0;
        uint32 amount = 0;

        bool IsValid() const { return receiver && itemId; }
    };

    class GuildShareTargetValue : public CalculatedValue<GuildShareTarget>
    {
    public:
        GuildShareTargetValue(PlayerbotAI* ai) : CalculatedValue<GuildShareTarget>(ai, "guild share target", 5) {}

        GuildShareTarget Calculate() override;
    };

    // Returns true if there is a valid guild share target nearby.
    class HasGuildShareTargetValue : public BoolCalculatedValue
    {
    public:
        HasGuildShareTargetValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has guild share target", 5) {}

        bool Calculate() override { return AI_VALUE(GuildShareTarget, "guild share target").IsValid(); }
    };

    class GuildShareCraftOrderValue : public CalculatedValue<GuildOrder>
    {
    public:
        GuildShareCraftOrderValue(PlayerbotAI* ai) : CalculatedValue<GuildOrder>(ai, "guild share craft order", 60) {}

        GuildOrder Calculate() override;
    };

    class GuildShareFarmOrderValue : public CalculatedValue<GuildOrder>
    {
    public:
        GuildShareFarmOrderValue(PlayerbotAI* ai) : CalculatedValue<GuildOrder>(ai, "guild share farm order", 60) {}

        GuildOrder Calculate() override;
    };

    class PetitionSignsValue : public SingleCalculatedValue<uint8>
    {
    public:
        PetitionSignsValue(PlayerbotAI* ai) : SingleCalculatedValue<uint8>(ai, "petition signs") {}

        uint8 Calculate() override;
    };

    class CanHandInPetitionValue : public CalculatedValue<bool>
    {
    public:
        CanHandInPetitionValue(PlayerbotAI* ai) : CalculatedValue<bool>(ai, "can hand in guild petition") {}

        bool Calculate() override { return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", chat->formatQItem(5863)) && AI_VALUE(uint8, "petition signs") >= sWorld.getConfig(CONFIG_UINT32_MIN_PETITION_SIGNS); };
    };

    class CanBuyTabard : public CalculatedValue<bool>
    {
    public:
        CanBuyTabard(PlayerbotAI* ai) : CalculatedValue<bool>(ai, "can buy tabard") {}

        bool Calculate() override;
    };
}