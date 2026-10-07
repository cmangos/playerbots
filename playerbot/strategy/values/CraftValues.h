#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "ProfessionCraftingFairness.h"
#include "ProfessionProduction.h"

class GameObject;
class Player;
class Item;
struct SpellEntry;
struct ItemPrototype;

namespace ai
{
    struct ProfessionMaterialSources
    {
        std::map<uint32, std::vector<int32>> gatherEntries;
        std::vector<int32> vendorEntries;
        std::vector<uint32> auctionItems;

        bool HasGathering() const { return !gatherEntries.empty(); }
        bool HasVendor() const { return !vendorEntries.empty(); }
        bool HasAuctionHouse() const { return !auctionItems.empty(); }
    };

    struct ProfessionCraftingPlan
    {
        uint32 spellId = 0;
        uint32 skillId = 0;
        uint32 itemId = 0;
        uint32 spellFocusId = 0;
        uint32 craftCount = 0;
        uint32 goalSpellId = 0;
        uint32 processingInputId = 0;
        std::map<uint32, uint32> retained;
        std::map<uint32, uint32> required;
        std::map<uint32, uint32> missing;

        bool IsValid() const { return spellId != 0 && skillId != 0; }
        bool HasMaterials() const { return IsValid() && missing.empty(); }
        bool Needs(uint32 reagentId) const { return missing.find(reagentId) != missing.end(); }
        std::map<uint32, uint32> GetMissingReagents(PlayerbotAI* ai) const;
        std::map<uint32, uint32> GetMissingSupplies(PlayerbotAI* ai) const;
        ProfessionMaterialSources GetMaterialSources(PlayerbotAI* ai) const;
    };

    class CraftData
    {
    public:
        CraftData() : itemId(0) {}

        CraftData(const CraftData& other) : itemId(other.itemId)
        {
            required.insert(other.required.begin(), other.required.end());
            obtained.insert(other.obtained.begin(), other.obtained.end());
        }

        bool IsEmpty() { return itemId == 0; }
        void Reset() { itemId = 0; }
        bool IsRequired(uint32 item) { return required.find(item) != required.end(); }

        bool IsFulfilled()
        {
            for (std::map<uint32, int>::iterator i = required.begin(); i != required.end(); ++i)
            {
                uint32 item = i->first;
                if (obtained[item] < i->second)
                    return false;
            }

            return true;
        }

        void AddObtained(uint32 itemId, uint32 count)
        {
            if (IsRequired(itemId))
            {
                obtained[itemId] += count;
            }
        }

        void Crafted(uint32 count)
        {
            for (std::map<uint32, int>::iterator i = required.begin(); i != required.end(); ++i)
            {
                uint32 item = i->first;
                if (obtained[item] >= required[item] * (int)count)
                {
                    obtained[item] -= required[item] * (int)count;
                }
            }
        }

    public:
        uint32 itemId;
        std::map<uint32, int> required, obtained;
    };

    class CraftValue : public ManualSetValue<CraftData&>
    {
    public:
        CraftValue(PlayerbotAI* ai, std::string name = "craft") : ManualSetValue<CraftData&>(ai, data, name) {}

    private:
        CraftData data;
    };

    class CraftSpellsValue : public CalculatedValue<std::vector<uint32>> //All crafting spells
    {
    public:
        CraftSpellsValue(PlayerbotAI* ai, std::string name = "craft spells", int checkInterval = 10) : CalculatedValue<std::vector<uint32>>(ai, name, checkInterval) {}
        virtual std::vector<uint32> Calculate() override;
    };

    struct CraftToolRequirements
    {
        std::set<uint32> items;
        std::set<uint32> categories;
        bool UsesItem(const ItemPrototype* item) const;
        bool NeedsItem(const ItemPrototype* item, Player* player) const;
    };

    struct ProfessionCraftRequest
    {
        ProfessionCraftingPlan plan;
        ObjectGuid inputGuid;
        bool accepted = false;
    };

    class ProfessionCraftRequestValue : public ManualSetValue<ProfessionCraftRequest&>
    {
    public:
        ProfessionCraftRequestValue(PlayerbotAI* ai) :
            ManualSetValue<ProfessionCraftRequest&>(ai, state, "profession craft request") {}
        virtual void Reset() override { state = {}; }
    private:
        ProfessionCraftRequest state;
    };

    struct ProcessingSource
    {
        uint32 inputId;
        uint32 effect;
    };
    using ProcessingSourceMap = std::map<uint32, std::vector<ProcessingSource>>;

    class ProcessingSourcesValue : public SingleCalculatedValue<ProcessingSourceMap*>
    {
    public:
        ProcessingSourcesValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "processing sources") {}
        virtual ProcessingSourceMap* Calculate() override;
    };

    using CraftToolItemMap = std::map<uint32, std::vector<uint32>>;

    // Shared metadata only; do not scan all items once per bot/category.
    class CraftToolItemsValue : public SingleCalculatedValue<CraftToolItemMap*>
    {
    public:
        CraftToolItemsValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "craft tool items") {}
        virtual CraftToolItemMap* Calculate() override;
    };

    class ProfessionToolPurchasesValue : public CalculatedValue<std::set<uint32>>
    {
    public:
        ProfessionToolPurchasesValue(PlayerbotAI* ai) :
            CalculatedValue<std::set<uint32>>(ai, "profession tool purchases", 30) {}
        virtual std::set<uint32> Calculate() override;
    };

    class CraftToolRequirementsValue : public CalculatedValue<CraftToolRequirements>
    {
    public:
        CraftToolRequirementsValue(PlayerbotAI* ai) :
            CalculatedValue<CraftToolRequirements>(ai, "craft tool requirements", 10) {}
        virtual CraftToolRequirements Calculate() override;
    };

    class ProfessionCraftingFairnessValue : public ManualSetValue<profession::CraftingFairness&>
    {
    public:
        ProfessionCraftingFairnessValue(PlayerbotAI* ai) :
            ManualSetValue<profession::CraftingFairness&>(ai, state, "profession fairness") {}
        virtual void Reset() override { state = {}; }
    private:
        profession::CraftingFairness state;
    };

    class EnchantSpellsValue : public CalculatedValue<std::vector<uint32>> //All enchanting spells
    {
    public:
        EnchantSpellsValue(PlayerbotAI* ai, std::string name = "enchant spells", int checkInterval = 10) : CalculatedValue<std::vector<uint32>>(ai, name, checkInterval) {}
        virtual std::vector<uint32> Calculate() override;
    };

    class HasReagentsForValue : public Uint32CalculatedValue, public Qualified //Does the bot have reagents to cast this craft spell?
    {
    public:
        HasReagentsForValue(PlayerbotAI* ai, std::string name = "has reagents for", int checkInterval = 1) : Uint32CalculatedValue(ai, name, checkInterval), Qualified() {}
        virtual uint32 Calculate() override;
    };

    class CanCraftSpellValue : public BoolCalculatedValue, public Qualified
    {
    public:
        CanCraftSpellValue(PlayerbotAI* ai, std::string name = "can craft spell", int checkInterval = 10) : BoolCalculatedValue(ai, name, checkInterval), Qualified() {}
        virtual bool Calculate() override;
        static bool HasRequiredTools(const SpellEntry* spell, Player* player);
    };

    class ShouldCraftSpellValue : public BoolCalculatedValue, public Qualified
    {
    public:
        ShouldCraftSpellValue(PlayerbotAI* ai, std::string name = "should craft spell", int checkInterval = 10) : BoolCalculatedValue(ai, name, checkInterval), Qualified() {}
        virtual bool Calculate() override;
        static bool SpellGivesSkillUp(uint32 spellId, Player* bot);
    };

    class ProfessionCraftingPlanValue : public CalculatedValue<ProfessionCraftingPlan>
    {
    public:
        ProfessionCraftingPlanValue(PlayerbotAI* ai);
        virtual ProfessionCraftingPlan Calculate() override;

        static bool IsEnabledFor(PlayerbotAI* ai);
        static bool IsCraftCooldownReady(PlayerbotAI* ai);
        static bool HasPendingCraft(PlayerbotAI* ai);
        static void QueuePendingCraft(PlayerbotAI* ai, uint32 spellId, bool acceptedCast = false);
        static void ClearPendingCraft(PlayerbotAI* ai, uint32 spellId);
        static void QueuePlan(PlayerbotAI* ai, const ProfessionCraftingPlan& plan, Item* input);
        static Item* GetProcessingTarget(PlayerbotAI* ai, const ProfessionCraftingPlan& plan,
            ObjectGuid ownedGuid = ObjectGuid());
        static void CompleteProcessingLoot(PlayerbotAI* ai, ObjectGuid inputGuid);
        static bool IsAhSearchReady(PlayerbotAI* ai);
        static uint32 GetAhBudget(PlayerbotAI* ai);
        static std::map<uint32, uint32> GetMissingTools(PlayerbotAI* ai);
        static bool ShouldTravelForGathering(PlayerbotAI* ai, const ProfessionCraftingPlan& plan);
        static bool ShouldTravelToVendor(PlayerbotAI* ai, const ProfessionCraftingPlan& plan);
        static bool ShouldTravelToAuctionHouse(PlayerbotAI* ai, const ProfessionCraftingPlan& plan);
        static bool ShouldTravelToSpellFocus(PlayerbotAI* ai, const ProfessionCraftingPlan& plan);
        static GameObject* GetCurrentSpellFocus(PlayerbotAI* ai, const ProfessionCraftingPlan& plan);
    };

    class CanCraftProfessionValue : public BoolCalculatedValue
    {
    public:
        CanCraftProfessionValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can craft profession", 10) {}
        virtual bool Calculate() override;
    };

    class ProfessionMaterialSourcesValue : public CalculatedValue<ProfessionMaterialSources>
    {
    public:
        ProfessionMaterialSourcesValue(PlayerbotAI* ai) :
            CalculatedValue<ProfessionMaterialSources>(ai, "profession material sources", 15) {}
        virtual ProfessionMaterialSources Calculate() override;
    };
}
