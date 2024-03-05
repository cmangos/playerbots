#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class MountValue : AiObject
    {
    public:
        MountValue(PlayerbotAI* ai, uint32 spellId) : AiObject(ai), spellId(spellId) {};
        MountValue(PlayerbotAI* ai, Item* item) : AiObject(ai), itemGuid(item->GetObjectGuid()) { spellId = GetMountSpell(item->GetProto()->ItemId); };

        bool IsItem() { return itemGuid; }
        Item* GetItem() { return bot->GetItemByGuid(itemGuid); }
        uint32 GetSpellId() { return spellId; }
        uint32 GetSpeed(bool canFly) {return GetSpeed(spellId, canFly);}
        static uint32 IsMountSpell(uint32 spellId) { return GetSpeed(spellId, false) || GetSpeed(spellId, true); }
        static uint32 GetSpeed(uint32 spellId, bool canFly);
        static uint32 GetSpeed(uint32 spellId) { return std::max(GetSpeed(spellId, false), GetSpeed(spellId, true)); };
        static uint32 GetMountSpell(uint32 itemId);
        bool IsValidLocation();

    private:
        ObjectGuid itemGuid = ObjectGuid();
        uint32 spellId = 0;
    };

    class CurrentMountSpeedValue : public Uint32CalculatedValue, public Qualified
    {
    public:
        CurrentMountSpeedValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "current mount speed", 1), Qualified() {}
        virtual uint32 Calculate();
    };

    class MountListValue : public CalculatedValue<std::vector<MountValue>>
    {
    public:
        MountListValue(PlayerbotAI* ai) : CalculatedValue<std::vector<MountValue>>(ai, "mount list", 10) {}
        virtual std::vector<MountValue> Calculate();
        virtual std::string Format();
    };        
}
