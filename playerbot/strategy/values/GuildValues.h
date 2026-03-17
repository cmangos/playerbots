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
        uint32 amount = 0; // Used for Craft orders

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
        GuildOrderValue(PlayerbotAI* ai) : CalculatedValue<GuildOrder>(ai, "guild order", 10) {}

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

        bool Calculate() override { return AI_VALUE(GuildOrder, "guild order").IsTravelOrder(); }
    };

    class HasGuildCraftOrderValue : public BoolCalculatedValue
    {
    public:
        HasGuildCraftOrderValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has guild craft order", 10) {}

        bool Calculate() override { return AI_VALUE(GuildOrder, "guild order").IsCraftOrder(); }
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
