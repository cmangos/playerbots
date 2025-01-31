#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class MountValue : AiObject
    {
    public:
        MountValue(PlayerbotAI* ai, uint32 spellId) : AiObject(ai), spellId(spellId) {};
        MountValue(PlayerbotAI* ai, const ItemPrototype* proto) : AiObject(ai), proto(proto) { spellId = GetMountSpell(proto->ItemId); };
        MountValue(PlayerbotAI* ai, Item* item) : AiObject(ai), itemGuid(item->GetObjectGuid()) { spellId = GetMountSpell(item->GetProto()->ItemId); proto = item->GetProto(); };
        MountValue(PlayerbotAI* ai, MountValue mount) : AiObject(ai), itemGuid(mount.itemGuid), spellId(mount.spellId), proto(proto){};

        bool IsItem() { return itemGuid || proto; }
        Item* GetItem() { return bot->GetItemByGuid(itemGuid); }
        const ItemPrototype* GetItemProto() { return proto; }
        uint32 GetSpellId() { return spellId; }
        uint32 GetSpeed(bool canFly) {return GetSpeed(spellId, canFly);}
        static uint32 IsMountSpell(uint32 spellId) { return GetSpeed(spellId, false) || GetSpeed(spellId, true); }
        static uint32 GetSpeed(uint32 spellId, bool canFly);
        static uint32 GetSpeed(uint32 spellId) { return std::max(GetSpeed(spellId, false), GetSpeed(spellId, true)); };
        static uint32 GetMountSpell(uint32 itemId);
        bool IsValidLocation();

    private:
        const ItemPrototype* proto = nullptr;
        ObjectGuid itemGuid = ObjectGuid();
        uint32 spellId = 0;
    };

    class CurrentMountSpeedValue : public Uint32CalculatedValue, public Qualified
    {
    public:
        CurrentMountSpeedValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "current mount speed", 1), Qualified() {}
        virtual uint32 Calculate();
    };

    class FullMountListValue : public SingleCalculatedValue<std::vector<MountValue>>
    {
    public:
        FullMountListValue(PlayerbotAI* ai) : SingleCalculatedValue<std::vector<MountValue>>(ai, "full mount list") {}
        virtual std::vector<MountValue> Calculate();
    };

    class MountListValue : public CalculatedValue<std::vector<MountValue>>
    {
    public:
        MountListValue(PlayerbotAI* ai) : CalculatedValue<std::vector<MountValue>>(ai, "mount list", 10) {}
        virtual std::vector<MountValue> Calculate();
        virtual std::string Format();
    };   

    class MountSkillTypeValue : public CalculatedValue<uint32>
    {
    public:
        MountSkillTypeValue(PlayerbotAI* ai) : CalculatedValue<uint32>(ai, "mount skilltype", 10) {}

        virtual uint32 Calculate() override;
    };

    class AvailableMountVendors : public CalculatedValue<std::vector<int32>>
    {
    public:
        AvailableMountVendors(PlayerbotAI* ai) : CalculatedValue<std::vector<int32>>(ai, "available mount vendors", 10) {}

        virtual std::vector<int32> Calculate() override;
    };

    class CanTrainMountValue : public BoolCalculatedValue
    { 
    public:
        CanTrainMountValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can train mount", 10) {}

        virtual bool Calculate() override;
    };

    class CanBuyMountValue : public BoolCalculatedValue
    {
    public:
        CanBuyMountValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can buy mount", 10) {}

        virtual bool Calculate() override;
    };
}
