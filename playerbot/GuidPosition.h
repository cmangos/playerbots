#pragma once
#include "WorldPosition.h"

namespace ai
{
    class GuidPosition : public ObjectGuid, public WorldPosition
    {
    public:
        GuidPosition() : ObjectGuid(), WorldPosition() {}
        GuidPosition(ObjectGuid guid, WorldPosition pos) : ObjectGuid(guid), WorldPosition(pos) {};
        GuidPosition(ObjectGuid guid, const uint32 mapId) : ObjectGuid(guid) { WorldPosition::set(guid, mapId); }
        GuidPosition(uint64 const& guid, WorldPosition const& pos) : ObjectGuid(guid), WorldPosition(pos) {};
        //template<class T>
        //GuidPosition(ObjectGuid guid, T) : ObjectGuid(guid) {WorldPosition::set(WorldPosition(T))};
        GuidPosition(CreatureDataPair const* dataPair) : ObjectGuid(HIGHGUID_UNIT, dataPair->second.id, dataPair->first), WorldPosition(dataPair) {};
        GuidPosition(GameObjectDataPair const* dataPair) : ObjectGuid(HIGHGUID_GAMEOBJECT, dataPair->second.id, dataPair->first), WorldPosition(dataPair) {};
        GuidPosition(WorldObject* wo) : WorldPosition(wo) { ObjectGuid::Set(wo->GetObjectGuid()); };
        GuidPosition(HighGuid hi, uint32 entry, uint32 counter = 1, WorldPosition pos = WorldPosition()) : ObjectGuid(hi, entry, counter), WorldPosition(pos) {};
        GuidPosition(std::string qualifier);

        virtual std::string to_string() const override;

        CreatureData* GetCreatureData() const { return IsCreature() ? sObjectMgr.GetCreatureData(GetCounter()) : nullptr; }
        CreatureInfo const* GetCreatureTemplate() const {return IsCreature() ? sObjectMgr.GetCreatureTemplate(GetEntry()) : nullptr; };

        GameObjectInfo const* GetGameObjectInfo() const { return IsGameObject() ? sObjectMgr.GetGameObjectInfo(GetEntry()) : nullptr; };

        WorldObject* GetWorldObject() const { return getMap() ? getMap()->GetWorldObject(*this) : nullptr;}
        Creature* GetCreature() const;
        Unit* GetUnit() const;
        GameObject* GetGameObject() const;
        Player* GetPlayer() const;

        void updatePosition() {WorldObject* wo = GetWorldObject(); if (wo) WorldPosition::set(wo); }

        bool HasNpcFlag(NPCFlags flag) { return IsCreature() && GetCreatureTemplate()->NpcFlags & flag; }
        bool isGoType(GameobjectTypes type) { return IsGameObject() && GetGameObjectInfo()->type == type; }

        const FactionTemplateEntry* GetFactionTemplateEntry() const;
        const ReputationRank GetReactionTo(const GuidPosition& other);
        bool IsFriendlyTo(const GuidPosition& other) { return (GetFactionTemplateEntry() && other.GetFactionTemplateEntry()) ? (GetReactionTo(other) > REP_NEUTRAL) : false; }
        bool IsHostileTo(const GuidPosition& other) { return (GetFactionTemplateEntry() && other.GetFactionTemplateEntry()) ? (GetReactionTo(other) < REP_NEUTRAL) : false; }

        bool isDead(); //For loaded grids check if the unit/object is unloaded/dead.

        uint16 IsPartOfAPool();
        uint16 GetGameEventId();
        bool IsEventUnspawned();

        virtual std::string print();

        operator bool() const { return getX() != 0 || getY() != 0 || getZ() != 0 || !IsEmpty(); }
        bool operator!() const { return getX() == 0 && getY() == 0 && getZ() == 0 && IsEmpty(); }
        bool operator== (ObjectGuid const& guid) const { return GetRawValue() == guid.GetRawValue(); }
        bool operator!= (ObjectGuid const& guid) const { return GetRawValue() != guid.GetRawValue(); }
        bool operator< (ObjectGuid const& guid) const { return GetRawValue() < guid.GetRawValue(); }
    };

    inline ByteBuffer& operator<<(ByteBuffer& b, GuidPosition& guidP)
    {
        b << (ObjectGuid)guidP;
        b << (WorldPosition)guidP;

        return b;
    }
        
    inline ByteBuffer& operator>>(ByteBuffer& b, GuidPosition& g)
    {
        ObjectGuid guid;
        WorldPosition pos;

        b >> guid;
        b >> pos;

        g = GuidPosition(guid, pos);

        return b;
    }
}