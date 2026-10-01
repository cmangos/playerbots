#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/NamedObjectContext.h"

class GameObject;

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
        std::map<uint32, uint32> required;
        std::map<uint32, uint32> missing;

        bool IsValid() const { return spellId != 0 && skillId != 0; }
        bool HasMaterials() const { return IsValid() && missing.empty(); }
        bool Needs(uint32 reagentId) const { return missing.find(reagentId) != missing.end(); }
        std::map<uint32, uint32> GetMissingReagents(PlayerbotAI* ai) const;
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
        static bool IsAhSearchReady(PlayerbotAI* ai);
        static uint32 GetAhBudget(PlayerbotAI* ai);
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
