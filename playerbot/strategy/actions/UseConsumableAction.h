#pragma once
#include "UseItemAction.h"
#include "playerbot/strategy/ItemVisitors.h"

namespace ai
{
    enum class ConsumableType : uint8
    {
        Flask = 0,
        Elixir,
        FoodBuff,
        Alcohol,
        Scroll,
        Count
    };

    enum class BotConsumableRole : uint8
    {
        MeleeDps,
        RangedPhysDps,
        CasterDps,
        Healer,
        Tank,
    };

    struct ConsumableCandidate
    {
        Item* item = nullptr;
        uint32 spellId = 0;
        ConsumableType type = ConsumableType::Count;
        float score = 0.0f;
    };

    class UseConsumableAction : public UseItemIdAction
    {
    public:
        UseConsumableAction(PlayerbotAI* ai) : UseItemIdAction(ai, "use consumable") {}

        bool Execute(Event& event) override;
        bool isPossible() override;
        bool isUseful() override;

    protected:
        uint32 GetItemId() override { return selectedItemId; }

    private:

        std::vector<ConsumableCandidate> CollectCandidates();

        bool ClassifyConsumable(const ItemPrototype* proto, ConsumableType& outType, uint32& outSpellId) const;

        bool IsProtectionConsumable(const ItemPrototype* proto) const;

        bool IsPotionLike(const ItemPrototype* proto) const;

        bool IsFoodOrDrink(const ItemPrototype* proto) const;

        bool IsStatBuffSpell(uint32 spellId) const;

        uint32 ResolveBuffAuraSpellId(uint32 spellId) const;

        void CollectPossibleAuraIds(uint32 spellId, std::vector<uint32>& outIds) const;

        bool HasActiveFlask() const;

        bool HasActiveFoodBuff() const;

        bool HasAnyCandidateAura(const ConsumableCandidate& candidate) const;

        float ScoreCandidate(const ConsumableCandidate& candidate) const;

        float ScoreSpellStats(uint32 spellId) const;

        BotConsumableRole GetBotRole() const;

        float GetStatWeight(uint32 statType) const;

        uint32 selectedItemId = 0;

        static const std::vector<uint32> knownFlaskAuras;
        static const std::vector<uint32> knownFoodBuffAuras;
        static const std::vector<uint32> alcoholItemIds;
        static const std::vector<uint32> excludedItemIds;
    };
}