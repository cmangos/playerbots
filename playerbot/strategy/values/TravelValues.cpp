
#include "playerbot/playerbot.h"
#include "TravelValues.h"
#include "QuestValues.h"
#include "SharedValueContext.h"

using namespace ai;

EntryGuidps EntryGuidpsValue::Calculate()
{
    EntryTravelPurposeMap entryMap = GAI_VALUE(EntryTravelPurposeMap, "entry travel purpose");

    EntryGuidps guidps;

    std::vector<CreatureDataPair const*> creatures = WorldPosition().getCreaturesNear();
    std::vector<GameObjectDataPair const*> gos = WorldPosition().getGameObjectsNear();

    for (auto& [entry, purpose] : entryMap)
    {
        if (entry > 0)
        {
            for (auto& creatureDataPair : creatures)
                if(creatureDataPair->second.id == entry)
                    if (GuidPosition(creatureDataPair).isValid())                    
                        guidps[entry].push_back(creatureDataPair);
        }
        else
        {
            for (auto& goDataPair : gos)
                if (goDataPair->second.id == (entry * -1))
                    if (GuidPosition(goDataPair).isValid())
                        guidps[entry].push_back(goDataPair);
        }
    }

    return guidps;
}

EntryTravelPurposeMap EntryTravelPurposeMapValue::Calculate()
{
    EntryQuestRelationMap relationMap = GAI_VALUE(EntryQuestRelationMap, "entry quest relation");

    EntryTravelPurposeMap entryPurposeMap;

    std::vector<NPCFlags> allowedNpcFlags;

    allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);

    allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
    allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

    for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); ++entry)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);

        if (!cInfo)
            continue;

        if (cInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        DestinationPurose purpose = 0;

        if(relationMap.find(entry) != relationMap.end())
            purpose |= (uint32)TravelDestinationPurposeFlag::QUEST;

        for (auto flag : allowedNpcFlags)
        {
            if ((cInfo->NpcFlags & flag) != 0)
            {
                purpose |= (uint32)TravelDestinationPurposeFlag::RPG;
                break;
            }
        }

        if (cInfo->MinLootGold > 0)
        {
            purpose |= (uint32)TravelDestinationPurposeFlag::GRIND;
        }

        if (cInfo->Rank == 3 || cInfo->Rank == 4 || cInfo->Rank == 1)
        {
            if (cInfo->Rank == 1)
            {
                std::vector<CreatureDataPair const*> creatures = WorldPosition().getCreaturesNear(0, entry);
                if (creatures.size() == 1)
                    if (WorldPosition(creatures[0]).isOverworld())
                        purpose |= (uint32)TravelDestinationPurposeFlag::BOSS;
            }
            else
                purpose |= (uint32)TravelDestinationPurposeFlag::BOSS;
        }

        if (cInfo->SkinningLootId && cInfo->GetRequiredLootSkill() == SKILL_SKINNING)
        {
            purpose |= (uint32)TravelDestinationPurposeFlag::GATHER;
        }

        if (uint32 skillId = SkillIdToGatherEntry(entry))
        {
            if (skillId == SKILL_SKINNING || skillId == SKILL_MINING || skillId == SKILL_HERBALISM)
                purpose |= (uint32)TravelDestinationPurposeFlag::GATHER;
        }

        if (purpose > 0)
            entryPurposeMap[entry] = purpose;
    }

    for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); ++entry)
    {
        GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(entry);

        if (!gInfo)
            continue;

        if (gInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        uint32 purpose = 0;

        DestinationEntry goEntry = entry * -1;

        if (relationMap.find(goEntry) != relationMap.end())
            purpose |= (uint32)TravelDestinationPurposeFlag::QUEST;

        std::vector<GameobjectTypes> allowedGoTypes;

        allowedGoTypes.push_back(GAMEOBJECT_TYPE_MAILBOX);

        for (auto type : allowedGoTypes)
        {
            if (gInfo->type == type)
            {
                purpose |= (uint32)TravelDestinationPurposeFlag::RPG;
                break;
            }
        }

        if (uint32 skillId = SkillIdToGatherEntry(goEntry))
        {
            if (skillId == SKILL_LOCKPICKING || skillId == SKILL_MINING || skillId == SKILL_HERBALISM || skillId == SKILL_FISHING)
                purpose |= (uint32)TravelDestinationPurposeFlag::GATHER;
        }

        if (purpose > 0)
            entryPurposeMap[goEntry] = purpose;
    }

    return entryPurposeMap;
}


uint32 EntryTravelPurposeMapValue::SkillIdToGatherEntry(int32 entry)
{
    if (entry > 0)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo>(entry);

        if (!cInfo->SkinningLootId)
            return 0;

        return cInfo->GetRequiredLootSkill();
    }
    else
    {
        GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(entry * -1);

        if (uint32 lockId = gInfo->GetLockId())
        {
            LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
            if (lockInfo)
            {
                uint32 skillId = SKILL_NONE;

                for (int i = 0; i < 8; ++i)
                {
                    if (lockInfo->Type[i] != LOCK_KEY_SKILL)
                        continue;

                    if (SkillByLockType(LockType(lockInfo->Index[i])) == 0)
                        continue;

                    return SkillByLockType(LockType(lockInfo->Index[i]));
                }
            }
        }
    }

    return 0;
}

bool QuestStageActiveValue::Calculate()
{
    uint32 questId = getMultiQualifierInt(getQualifier(), 0, ",");
    uint32 stage = getMultiQualifierInt(getQualifier(), 1, ",");
    uint32 objective = 0;
    if (stage == 1)
        objective = getMultiQualifierInt(getQualifier(), 2, ",");

    if (stage == 0 && bot->HasQuest(questId))
        return false;

    if (stage == 1 && !AI_VALUE2(bool, "need quest objective", objective))
        return false;

    if (!bot->CanCompleteQuest(questId))
        return false;

    return true;
}