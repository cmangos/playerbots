#include "playerbot/TravelMgr.h"
#include <numeric>
#include <iomanip>

#include "playerbot/strategy/values/SharedValueContext.h"
#include "playerbot/strategy/values/TravelValues.h"
#include "MotionGenerators/PathFinder.h"
#include "TravelNode.h"
#include "PlayerbotAI.h"
#include "BotTests.h"
#include "Globals/ObjectAccessor.h"
#include <execution>

using namespace ai;
using namespace MaNGOS;

PlayerTravelInfo::PlayerTravelInfo(Player* player)
{
    PlayerbotAI* ai = player->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    team = player->GetTeam();
    level = player->GetLevel();
    currentSkill[SKILL_MINING] = player->GetSkillValue(SKILL_MINING);
    currentSkill[SKILL_HERBALISM] = player->GetSkillValue(SKILL_HERBALISM);
    currentSkill[SKILL_FISHING] = player->GetSkillValue(SKILL_FISHING);
    currentSkill[SKILL_SKINNING] = player->GetSkillValue(SKILL_SKINNING);

    skillMax[SKILL_MINING] = player->GetSkillMax(SKILL_MINING);
    skillMax[SKILL_HERBALISM] = player->GetSkillMax(SKILL_HERBALISM);
    skillMax[SKILL_FISHING] = player->GetSkillMax(SKILL_FISHING);
    skillMax[SKILL_SKINNING] = player->GetSkillMax(SKILL_SKINNING);

    money = player->GetMoney();

    if (player->GetGroup())
        groupSize = player->GetGroup()->GetMembersCount();

    focusList = AI_VALUE(focusQuestTravelList, "focus travel target");

    for (auto& [valueName, value] : boolValues)
        value = AI_VALUE(bool, valueName);

    for (auto& [valueName, value] : uint8Values)
        value = AI_VALUE(uint8, valueName);
}

WorldPosition* TravelDestination::NearestPoint(const WorldPosition& pos) const {
    return *std::min_element(points.begin(), points.end(), [pos](WorldPosition* i, WorldPosition* j) {return i->distance(pos) < j->distance(pos); });
}

std::vector <WorldPosition*> TravelDestination::NextPoint(const WorldPosition& pos) const {
    return pos.GetNextPoint(GetPoints());
}

std::string QuestTravelDestination::GetTitle() const {
    return ChatHelper::formatQuest(GetQuestTemplate());
}

bool QuestRelationTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    if (!info.GetBoolValue2("has strategy", "rpg quest"))
        return false;

    bool forceThisQuest = info.HasFocusQuest();

    if (forceThisQuest && !info.IsFocusQuest(GetQuestId()))
        return false;

    if (GetRelation() == 0)
    {
        if (!forceThisQuest && (int32)GetQuestTemplate()->GetQuestLevel() >= (int32)info.GetLevel() + (int32)5)
            return false;

        if (GetPoints().front()->getMapId() != info.GetPosition().getMapId()) //CanTakeQuest will check required conditions which will fail on a different map.
            if (GetQuestTemplate()->GetRequiredCondition())          //So we skip this quest for now.
                return false;

        if (!info.HasFocusQuest())
        {
            if (info.GetBoolValue("can fight equal"))
            {
                if (info.GetUint8Value("free quest log slots") < 5)
                    return false;
            }
            else
            {
                if (info.GetUint8Value("free quest log slots") < 10)
                    return false;
            }

            //Do not try to pick up dungeon/elite quests in instances without a group.
            if ((GetQuestTemplate()->GetType() == QUEST_TYPE_ELITE || GetQuestTemplate()->GetType() == QUEST_TYPE_DUNGEON) && !info.GetBoolValue("can fight boss"))
                return false;
        }
    }
    else
    {
        //Do not try to hand-in dungeon/elite quests in instances without a group.
        if ((GetQuestTemplate()->GetType() == QUEST_TYPE_ELITE || GetQuestTemplate()->GetType() == QUEST_TYPE_DUNGEON) && !info.GetBoolValue("can fight boss"))
        {
            if (!this->NearestPoint(info.GetPosition())->isOverworld())
                return false;
        }
    }

    return true;
}

bool QuestRelationTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const {
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if(!IsPossible(info))
        return false;

    bool forceThisQuest = info.HasFocusQuest();

    if (GetRelation() == 0)
    {
        if ((!info.HasFocusQuest() && !bot->GetMap()->IsContinent()) || !bot->CanTakeQuest(GetQuestTemplate(), false))
            return false;

        if (!forceThisQuest)
        {
            if (info.GetBoolValue("can fight equal"))
            {
                if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest npc::" + std::to_string(GetEntry()))) //Noone has yellow exclamation mark.
                    if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(GetEntry()) + ",need quest reward::" + std::to_string(GetQuestId()))) //Noone can do this quest for a usefull reward.
                        return false;
            }
            else
            {
                if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(GetEntry()))) //Noone can pick up this quest for money.
                    return false;
            }
        }
        else
        {
            if (!AI_VALUE2(bool, "can accept quest npc", std::to_string(GetEntry())))
                return false;
        }
    }
    else
    {
        if (!forceThisQuest)
        {
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can turn in quest npc::" + std::to_string(GetEntry())))
                return false;
        }
        else
        {
            if (!AI_VALUE2(bool, "can turn in quest npc", std::to_string(GetEntry())))
                return false;
        }
    }

    if (GetEntry() > 0)
    {
        return !GuidPosition(HIGHGUID_UNIT, GetEntry()).IsHostileTo(bot);
    }

    return true;
}

std::string QuestRelationTravelDestination::GetTitle() const {
    std::ostringstream out;

    if (GetRelation() == 0)
        out << "questgiver ";
    else
        out << "questtaker ";

    out << ChatHelper::formatWorldEntry(GetEntry());
    return out.str();
}

bool QuestObjectiveTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    if (!info.GetBoolValue2("has strategy", "rpg quest"))
        return false;

    bool forceThisQuest = info.HasFocusQuest();

    if (forceThisQuest && !info.IsFocusQuest(GetQuestId()))
        return false;

    if (!forceThisQuest)
    {
        if ((int32)GetQuestTemplate()->GetQuestLevel() > (int32)info.GetLevel() + (int32)1)
            return false;

        if (GetQuestTemplate()->GetQuestLevel() + 5 > (int)info.GetLevel() && !info.GetBoolValue("can fight equal"))
            return false;
    }

    if (info.IsInRaid() != (GetQuestTemplate()->GetType() == QUEST_TYPE_RAID))
        return false;

    bool isVendor = false;

    //Check mob level
    if (GetEntry() > 0)
    {
        CreatureInfo const* cInfo = GetCreatureInfo();

        if (cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR && GetQuestTemplate()->ReqItemId[GetObjective()])
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(GetQuestTemplate()->ReqItemId[GetObjective()]);
            if (GetQuestTemplate()->ReqItemCount[GetObjective()] * proto->BuyPrice > info.GetMoney()) //Need more money.
                return false;

            isVendor = true;
        }

        if (!isVendor && !forceThisQuest)
        {
            if (cInfo && (int)cInfo->MaxLevel - (int)info.GetLevel() > 4)
                return false;

            //Do not try to hand-in dungeon/elite quests in instances without a group.
            if (cInfo->Rank > CREATURE_ELITE_NORMAL)
            {
                if (!this->NearestPoint(info.GetPosition())->isOverworld() && !info.GetBoolValue("can fight boss"))
                    return false;
                else if (!info.GetBoolValue("can fight elite"))
                    return false;
            }
        }
    }

    if (!forceThisQuest)
    {
        if (!isVendor && GetQuestTemplate()->GetType() == QUEST_TYPE_ELITE && !info.GetBoolValue("can fight elite"))
            return false;

        //Do not try to do dungeon/elite quests in instances without a group.
        if ((GetQuestTemplate()->GetType() == QUEST_TYPE_ELITE || GetQuestTemplate()->GetType() == QUEST_TYPE_DUNGEON || GetQuestTemplate()->GetType() == QUEST_TYPE_RAID) && !info.GetBoolValue("can fight boss"))
        {
            if (!this->NearestPoint(info.GetPosition())->isOverworld())
                return false;
        }

        //Do not try to do pvp quests in bg's (no way to travel there). 
        if (GetQuestTemplate()->GetType() == QUEST_TYPE_PVP)
        {
            if (!this->NearestPoint(info.GetPosition())->isOverworld())
                return false;
        }
    }

    return true;
}

bool QuestObjectiveTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const {
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!IsPossible(info))
        return false;

    bool forceThisQuest = info.HasFocusQuest();

    bool isVendor = false;

    //Check mob level
    if (GetEntry() > 0)
    {
        CreatureInfo const* cInfo = GetCreatureInfo();

        if (cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR && GetQuestTemplate()->ReqItemId[GetObjective()] && !GuidPosition(HIGHGUID_UNIT, GetEntry()).IsHostileTo(bot))
        {
            isVendor = true;
        }
    }

   std::vector<std::string> qualifier = { std::to_string(GetQuestTemplate()->GetQuestId()), std::to_string(GetObjective()) };

    if (!AI_VALUE2(bool, "group or", "following party,near leader,need quest objective::" + Qualified::MultiQualify(qualifier,","))) //Noone needs the quest objective.
        return false;

    WorldPosition botPos(bot);

    if (!isVendor && GetEntry() > 0 && !IsOut(botPos))
    {
        TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();

        //Only look for the target if it is unique or if we are currently working on it.
        if (GetPoints().size() == 1 || (target->GetStatus() == TravelStatus::TRAVEL_STATUS_WORK && target->GetEntry() == GetEntry()))
        {
            std::list<ObjectGuid> targets = AI_VALUE(std::list<ObjectGuid>, "possible targets");

            for (auto& target : targets)
                if (target.GetEntry() == GetEntry() && target.IsCreature() && ai->GetCreature(target) && ai->GetCreature(target)->IsAlive())
                    return true;

            return false;
        }
    }

    return true;
}

std::string QuestObjectiveTravelDestination::GetTitle() const {
    std::ostringstream out;

    out << "objective " << (GetObjective() + 1);

    if (GetQuestTemplate()->ReqItemCount[GetObjective()] > 0)
        out << " loot " << ChatHelper::formatItem(sObjectMgr.GetItemPrototype(GetQuestTemplate()->ReqItemId[GetObjective()]), 0, 0) << " from";
    else if (GetEntry() > 0)
        out << " to kill";
    else
        out << " to use";

    out << " " << ChatHelper::formatWorldEntry(GetEntry());
    return out.str();
}

bool RpgTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    bool isUsefull = false;

    if (GetEntry() > 0)
    {

        CreatureInfo const* cInfo = this->GetCreatureInfo();

        if (!cInfo)
            return false;

        if (cInfo->NpcFlags & UNIT_NPC_FLAG_VENDOR)
        {
            if (info.GetBoolValue2("group or", "should sell,can sell,following party,near leader"))
                isUsefull = true;
            else if (info.GetBoolValue2("has strategy", "free") && info.GetBoolValue("should sell") && info.GetBoolValue("can sell"))
                isUsefull = true;
        }

        if (cInfo->NpcFlags & UNIT_NPC_FLAG_REPAIR)
        {
            if (info.GetBoolValue2("group or", "should repair,can repair,following party,near leader"))
                isUsefull = true;
            else if (info.GetBoolValue2("has strategy", "free") && info.GetBoolValue("should repair") && info.GetBoolValue("can repair"))
                isUsefull = true;
        }

        if (cInfo->NpcFlags & UNIT_NPC_FLAG_AUCTIONEER)
        {
            if (info.GetBoolValue2("group or", "should ah sell,can ah sell,following party,near leader"))
                isUsefull = true;
            else if (info.GetBoolValue2("has strategy", "free") && info.GetBoolValue("should ah sell") && info.GetBoolValue("can ah sell"))
                isUsefull = true;
        }
    }
    else
    {
        GameObjectInfo const* gInfo = this->GetGoInfo();

        if (!gInfo)
            return false;

        if (gInfo->type == GAMEOBJECT_TYPE_MAILBOX)
            if (info.GetBoolValue("can get mail"))
                isUsefull = true;
    }


    if (!isUsefull)
        return false;

    WorldPosition firstPoint = *GetPoints().front();

    //Horde pvp baracks
    if (firstPoint.getMapId() == 450 && info.GetTeam() == ALLIANCE)
        return false;

    //Alliance pvp baracks
    if (firstPoint.getMapId() == 449 && info.GetTeam() == HORDE)
        return false;

    return true;
}

bool RpgTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!IsPossible(info))
        return false;   

    //Once the target rpged with it is added to the ignore list. We can now move on.
    std::set<ObjectGuid>& ignoreList = AI_VALUE(std::set<ObjectGuid>&,"ignore rpg target");

    for (auto& i : ignoreList)
    {
        if (i.GetEntry() == GetEntry())
        {
            return false;
        }
    }    

    WorldPosition firstPoint = *GetPoints().front();

    //City & Pvp baracks
    if (firstPoint.getMapId() == info.GetPosition().getMapId() && firstPoint.HasAreaFlag(AREA_FLAG_CAPITAL) && !firstPoint.HasFaction(info.GetTeam()))
        return false;

    return !GuidPosition(HIGHGUID_UNIT, GetEntry()).IsHostileTo(bot);
}

std::string RpgTravelDestination::GetTitle() const 
{
    std::ostringstream out;


    if(GetEntry() > 0)
        out << "rpg npc ";

    out << ChatHelper::formatWorldEntry(GetEntry());

    return out.str();
}

bool ExploreTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    AreaTableEntry const* area = GetArea();

    if (level && (uint32)level > info.GetLevel() && info.GetLevel() < DEFAULT_MAX_LEVEL)
        return false;

    return true;
}

bool ExploreTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const
{
    if (!IsPossible(info))
        return false;

    AreaTableEntry const* area = GetArea();

    if (area->exploreFlag == 0xffff)
        return false;
    int offset = area->exploreFlag / 32;

    uint32 val = (uint32)(1 << (area->exploreFlag % 32));
    uint32 currFields = bot->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);

    return !(currFields & val);    
}

AreaTableEntry const* ExploreTravelDestination::GetArea() const
{
    for (uint32 areaid = 0; areaid <= sAreaStore.GetNumRows(); ++areaid)
    {
        AreaTableEntry const* areaEntry = sAreaStore.LookupEntry(areaid);
        if (areaEntry && areaEntry->ID == GetEntry())
        {
            return areaEntry;
        }
    }

    return nullptr;
}

bool GrindTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    if (!urand(0, 10) && !info.GetBoolValue("should get money") && !IsOut(info.GetPosition()))
        return false;

    if (info.GetBoolValue("should sell") && (info.GetBoolValue("can sell") || info.GetBoolValue("can ah sell")))
        return false;

    CreatureInfo const* cInfo = GetCreatureInfo();

    int32 botLevel = info.GetLevel();

    uint8 botPowerLevel = info.GetUint8Value("durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = std::max(botLevel * (0.5f + levelMod), botLevel - 5.0f + levelBoost);

    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = std::max(botLevel * (0.4f + levelMod), botLevel - 12.0f + levelBoost);

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;

    if (cInfo->MinLootGold == 0)
        return false;

    if (cInfo->Rank > CREATURE_ELITE_NORMAL && !info.GetBoolValue("can fight elite"))
        return false;

    return true;
}

bool GrindTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!IsPossible(info))
        return false;

    return GuidPosition(bot).IsHostileTo(GuidPosition(HIGHGUID_UNIT, GetEntry()), bot->GetInstanceId());
}

std::string GrindTravelDestination::GetTitle() const
{
    std::ostringstream out;

    out << "grind mob ";

    out << ChatHelper::formatWorldEntry(GetEntry());

    return out.str();
}

bool BossTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    if (!info.GetBoolValue("can fight boss"))
        return false;

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(GetEntry());

    if ((int32)cInfo->MaxLevel > info.GetLevel() + 3)
        return false;

    WorldPosition firstPoint = *GetPoints().front();

    if (info.IsInGroup())
    {
        if (info.IsInRaid())
        {
#ifndef MANGOSBOT_TWO
            if (firstPoint.getMapEntry() && firstPoint.getMapEntry()->IsNonRaidDungeon())
#else
            if (firstPoint.getMapEntry() && firstPoint.getMapEntry()->IsNonRaidDungeon())
#endif
                return false;
        }
        else if (firstPoint.getMapEntry() && firstPoint.getMapEntry()->IsRaid())
            return false;
    }

    //Ragefire casm
    if (firstPoint.getMapId() == 389 && info.GetTeam() == ALLIANCE)
        return false;

    //Stockades
    if (firstPoint.getMapId() == 34 && info.GetTeam() == HORDE)
        return false;

    //Do not move to overworld bosses/uniques that are far away.
    if (firstPoint.isOverworld() && DistanceTo(info.GetPosition()) > 2000.0f)
        return false;

    return true;
}

bool BossTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!IsPossible(info))
        return false;

    if (!GuidPosition(bot).IsHostileTo(GuidPosition(HIGHGUID_UNIT, GetEntry()), bot->GetInstanceId()))
        return false;

    WorldPosition firstPoint = *GetPoints().front();   

    WorldPosition botPos(bot);

    if (!IsOut(botPos))
    {
        std::list<ObjectGuid> targets = AI_VALUE(std::list<ObjectGuid>, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == GetEntry() && target.IsCreature() && ai->GetCreature(target) && ai->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    if (!AI_VALUE2(bool, "has upgrade",  GetEntry()))
        return false;

    return true;
}

std::string BossTravelDestination::GetTitle() const
{
    std::ostringstream out;

    out << "boss mob ";

    out << ChatHelper::formatWorldEntry(GetEntry());

    return out.str();
}

bool GatherTravelDestination::IsPossible(const PlayerTravelInfo& info) const
{
    uint32 skillId = SKILL_NONE;
    uint32 reqSkillValue = 0;

    if (GetEntry() > 0)
    {
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(GetEntry());

        if (!cInfo)
            return false;

        skillId = cInfo->GetRequiredLootSkill();
        uint32 targetLevel = cInfo->MaxLevel;
        reqSkillValue = targetLevel < 10 ? 1 : targetLevel < 20 ? (targetLevel - 10) * 10 : targetLevel * 5;
    }
    else
    {
        GameObjectInfo const* goInfo = ObjectMgr::GetGameObjectInfo(GetEntry());

        if (!goInfo)
            return false;

        uint32 lockId = goInfo->GetLockId();
        LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
        if (!lockInfo)
            return false;

        for (int i = 0; i < 8; ++i)
        {
            if (lockInfo->Type[i] == LOCK_KEY_SKILL)
                if (SkillByLockType(LockType(lockInfo->Index[i])) > 0)
                {
                    skillId = SkillByLockType(LockType(lockInfo->Index[i]));
                    reqSkillValue = std::max((uint32)1, lockInfo->Skill[i]);
                    break;
                }
        }
    }

    if (!info.GetCurrentSkill((SkillType)skillId))
        return false;

    uint32 skillValue = uint32(info.GetCurrentSkill((SkillType)skillId));
    if (reqSkillValue > skillValue)
        return false;

    if (info.GetLevel() * 5 <= skillValue) //Not able to increase skill.
        return false;

    if (info.GetSkillMax((SkillType)skillId) <= skillValue) //Not able to increase skill.
        return false;

    if (reqSkillValue + 100 < skillValue) //Gray level = no skillup
        return false;

    return true;
}

bool GatherTravelDestination::IsActive(Player* bot, const PlayerTravelInfo& info) const
{
    PlayerbotAI* ai = bot->GetPlayerbotAI();
    AiObjectContext* context = ai->GetAiObjectContext();

    if (!IsPossible(info))
        return false;   

    return true;
}

std::string GatherTravelDestination::GetTitle() const {
    std::ostringstream out;

    out << "gathering location ";

    out << ChatHelper::formatWorldEntry(GetEntry());

    return out.str();
}

TravelTarget::TravelTarget(PlayerbotAI* ai) : AiObject(ai)
{
    sTravelMgr.SetNullTravelTarget(this);
}

void TravelTarget::SetTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1) {
    wPosition = wPosition1;
    tDestination = tDestination1;
    groupCopy = groupCopy1;
    forced = false;
    radius = 0;

    SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);
}

void TravelTarget::CopyTarget(TravelTarget* const target) {
    SetTarget(target->tDestination, target->wPosition);
    groupCopy = target->IsGroupCopy();
    forced = target->forced;
    extendRetryCount = target->extendRetryCount;
}

void TravelTarget::SetStatus(TravelStatus status) {
    m_status = status;
    startTime = WorldTimer::getMSTime();

    switch (m_status) {
    case TravelStatus::TRAVEL_STATUS_NONE:
    case TravelStatus::TRAVEL_STATUS_PREPARE:
    case TravelStatus::TRAVEL_STATUS_EXPIRED:
        statusTime = 1;
        break;
    case TravelStatus::TRAVEL_STATUS_TRAVEL:
        statusTime = GetMaxTravelTime() * 2 + sPlayerbotAIConfig.maxWaitForMove;
        break;
    case TravelStatus::TRAVEL_STATUS_WORK:
        statusTime = tDestination->GetExpireDelay();
        break;
    case TravelStatus::TRAVEL_STATUS_COOLDOWN:
        statusTime = tDestination->GetCooldownDelay();
    default: break;
    }
}

bool TravelTarget::IsActive() {
    if (m_status == TravelStatus::TRAVEL_STATUS_NONE || m_status == TravelStatus::TRAVEL_STATUS_EXPIRED || m_status == TravelStatus::TRAVEL_STATUS_PREPARE)
        return false;

    if (forced && IsTraveling())
        return true;

    if ((statusTime > 0 && startTime + statusTime < WorldTimer::getMSTime()))
    {
        SetStatus(TravelStatus::TRAVEL_STATUS_EXPIRED);
        return false;
    }

    if (m_status == TravelStatus::TRAVEL_STATUS_COOLDOWN)
        return true;

    if (IsTraveling())
        return true;

    if (IsWorking())
        return true;   

    if (!tDestination->IsActive(bot, PlayerTravelInfo(bot))) //Target has become invalid. Stop.
    {
        SetStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);
        return true;
    }

    return true;
};

bool TravelTarget::IsTraveling() {
    if (m_status != TravelStatus::TRAVEL_STATUS_TRAVEL)
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT))
        {
            SetStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);
            return false;
        }

    if (!tDestination->IsActive(bot, PlayerTravelInfo(bot)) && !forced) //Target has become invalid. Stop.
    {
        SetStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    bool HasArrived = tDestination->IsIn(pos, radius);

    if (HasArrived)
    {
        SetStatus(TravelStatus::TRAVEL_STATUS_WORK);
        return false;
    }

    if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT) && !ai->HasStrategy("travel once", BotState::BOT_STATE_NON_COMBAT))
    {
        sTravelMgr.SetNullTravelTarget(this);
        return false;
    }

    return true;
}

bool TravelTarget::IsWorking() {
    if (m_status != TravelStatus::TRAVEL_STATUS_WORK)
        return false;

    if (!tDestination->IsActive(bot, PlayerTravelInfo(bot))) //Target has become invalid. Stop.
    {
        SetStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    if (!ai->HasStrategy("travel", BotState::BOT_STATE_NON_COMBAT) && !ai->HasStrategy("travel once", BotState::BOT_STATE_NON_COMBAT))
    {
        sTravelMgr.SetNullTravelTarget(this);
        return false;
    }

    return true;
}

bool TravelTarget::IsPreparing() {
    if (m_status != TravelStatus::TRAVEL_STATUS_PREPARE)
        return false;

    return true;
}

TravelState TravelTarget::GetTravelState() {
    if (!tDestination || typeid(*tDestination) == typeid(NullTravelDestination))
        return TravelState::TRAVEL_STATE_IDLE;

    if (typeid(*tDestination) == typeid(QuestRelationTravelDestination))
    {
        if (((QuestRelationTravelDestination*)tDestination)->GetRelation() == 0)
        {
            if (IsTraveling() || IsPreparing())
                return TravelState::TRAVEL_STATE_TRAVEL_PICK_UP_QUEST;
            if (IsWorking())
                return TravelState::TRAVEL_STATE_WORK_PICK_UP_QUEST;
        }
        else
        {
            if (IsTraveling() || IsPreparing())
                return TravelState::TRAVEL_STATE_TRAVEL_HAND_IN_QUEST;
            if (IsWorking())
                return TravelState::TRAVEL_STATE_WORK_HAND_IN_QUEST;
        }
    }
    else if (typeid(*tDestination) == typeid(QuestObjectiveTravelDestination))
    {
        if (IsTraveling() || IsPreparing())
            return TravelState::TRAVEL_STATE_TRAVEL_DO_QUEST;
        if (IsWorking())
            return TravelState::TRAVEL_STATE_WORK_DO_QUEST;
    }
    else if (typeid(*tDestination) == typeid(RpgTravelDestination))
    {
        return TravelState::TRAVEL_STATE_TRAVEL_RPG;
    }
    else if (typeid(*tDestination) == typeid(ExploreTravelDestination))
    {
        return TravelState::TRAVEL_STATE_TRAVEL_EXPLORE;
    }

    return TravelState::TRAVEL_STATE_IDLE;
}

void TravelMgr::Clear()
{
#ifdef MANGOS
    sObjectAccessor.DoForAllPlayers([this](Player* plr) { TravelMgr::SetNullTravelTarget(plr); });
#endif
#ifdef CMANGOS
#ifndef MANGOSBOT_ZERO
    sObjectAccessor.ExecuteOnAllPlayers([this](Player* plr) { TravelMgr::SetNullTravelTarget(plr); });
#else
    HashMapHolder<Player>::ReadGuard g(HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        TravelMgr::SetNullTravelTarget(itr->second);
#endif
#endif
    for (auto& [type, dests] : destinationMap)
        for (auto& dest : dests)
            delete dest;
    destinationMap.clear();
    pointsMap.clear();
}

int32 TravelMgr::GetAreaLevel(uint32 area_id)
{
    auto lev = areaLevels.find(area_id);

    if (lev != areaLevels.end())
        return lev->second;

    AreaTableEntry const* area = GetAreaEntryByAreaID(area_id);

    if (!area)
    {
        areaLevels[area_id] = -2;
        return -2;
    }

    //Get exploration level
    if (area->area_level) 
    {
        areaLevels[area_id] = area->area_level;
        return area->area_level;
    }


    int32 level = 0;
    uint32 cnt = 0;

    //Get sub-area's
    for (uint32 i = 0; i <= sAreaStore.GetNumRows(); i++)
    {
        AreaTableEntry const* subArea = GetAreaEntryByAreaID(i);

        if (!subArea || subArea->zone != area->ID)
            continue;

        int32 subLevel = GetAreaLevel(subArea->ID);

        if (!subLevel)
            continue;

        level += subLevel;

        cnt++;
    }

    if (cnt)
    {
        areaLevels[area_id] = std::max(uint32(1), level / cnt);
        return areaLevels[area_id];
    }

    //Get units avarage
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        if (WorldPosition(creaturePair).GetArea() != area)
            continue;

        CreatureData const cData = creaturePair->second;
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

        if (!cInfo)
            continue;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, factionEntry);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, factionEntry);

        if (reactionHum > REP_NEUTRAL || reactionOrc > REP_NEUTRAL)
            continue;

        level += cInfo->MaxLevel;
        cnt++;
    }

    if (cnt)
    {
        areaLevels[area_id] = std::max(uint32(1),level / cnt);
        return areaLevels[area_id];
    }

    //Use parent zone value.
    if (area->zone)
    {
        areaLevels[area_id] = 0; //Set a temporary value so it wont be counted.
        level = GetAreaLevel(area->zone);
        areaLevels[area_id] = level;        
        return areaLevels[area_id];
    }

    areaLevels[area_id] = -1;

    return areaLevels[area_id];
}

void TravelMgr::LoadAreaLevels()
{
    if (!areaLevels.empty())
        return;

    WorldDatabase.PExecute("CREATE TABLE IF NOT EXISTS `ai_playerbot_zone_level` (`id` bigint(20) NOT NULL ,`level` bigint(20) NOT NULL,PRIMARY KEY(`id`))");

    std::string query = "SELECT id, level FROM ai_playerbot_zone_level";

    {
        auto result = WorldDatabase.PQuery(query.c_str());

        std::vector<uint32> loadedAreas;

        if (result)
        {
            BarGoLink bar(result->GetRowCount());            

            do
            {
                Field* fields = result->Fetch();
                bar.step();

                areaLevels[fields[0].GetUInt32()] = fields[1].GetInt32();

                loadedAreas.push_back(fields[0].GetUInt32());
            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " area levels.", areaLevels.size());
        }

        BarGoLink bar(sAreaStore.GetNumRows());
        WorldDatabase.BeginTransaction();
        for (uint32 i = 0; i < sAreaStore.GetNumRows(); ++i)    // areaflag numbered from 0
        {
            bar.step();
            if (AreaTableEntry const* area = sAreaStore.LookupEntry(i))
            {
                if (std::find(loadedAreas.begin(), loadedAreas.end(), area->ID) == loadedAreas.end())
                {
                    int32 level = sTravelMgr.GetAreaLevel(area->ID);

                    WorldDatabase.PExecute("INSERT INTO `ai_playerbot_zone_level` (`id`, `level`) VALUES ('%d', '%d')", area->ID, level);
                }
            }
        }
        WorldDatabase.CommitTransaction();
        if(areaLevels.size() > loadedAreas.size())
            sLog.outString(">> Generated " SIZEFMTD " areas.", areaLevels.size()- loadedAreas.size());
    }
}

void TravelMgr::SetMobAvoidArea()
{
    sLog.outString("-Apply mob avoidance maps");

    std::vector<std::future<void>> calculations;

    BarGoLink bar(sMapStore.GetNumRows());

    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        if (!sMapStore.LookupEntry(i))
            continue;
        
        uint32 mapId = sMapStore.LookupEntry(i)->MapID;
        calculations.push_back(std::async([this, mapId] { SetMobAvoidAreaMap(mapId); }));
        bar.step();
    }

    BarGoLink bar2(calculations.size());
    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
        bar2.step();
    }

    sLog.outString(">> Modified navmap areas for %d maps.", sMapStore.GetNumRows());
}

void TravelMgr::SetMobAvoidAreaMap(uint32 mapId) 
{
    PathFinder path(mapId, 0);
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    std::vector<CreatureDataPair const*> creatures = WorldPosition(mapId, 1,1).getCreaturesNear();

    for (auto& creaturePair : creatures)
    {
        CreatureData const cData = creaturePair->second;
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

        if (!cInfo)
            continue;

        WorldPosition point = WorldPosition(cData.mapid, cData.posX, cData.posY, cData.posZ, cData.orientation);

        if (cInfo->NpcFlags > 0)
            continue;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, factionEntry);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, factionEntry);

        if (reactionHum >= REP_NEUTRAL || reactionOrc >= REP_NEUTRAL)
            continue;

        if (!point.getTerrain())
            continue;

        if (!point.loadMapAndVMap(0))
            continue;

        path.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 12, 50.0f);
        path.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 13, 20.0f);
    }
}

void TravelMgr::LoadQuestTravelTable()
{
    if (!sTravelMgr.destinationMap.empty())
        return;

    // Clearing store (for reloading case)
    Clear();

    struct unit { uint64 guid; uint32 type; uint32 entry; uint32 map; float  x; float  y; float  z;  float  o; uint32 c; } t_unit;
    std::vector<unit> units;

    sLog.outString("Loading trainable spells.");
    if (GAI_VALUE(trainableSpellMap*, "trainable spell map")->empty())
    {

    }

    ObjectMgr::QuestMap const& questMap = sObjectMgr.GetQuestTemplates();
    std::vector<uint32> questIds;

    std::unordered_map <uint32, uint32> entryCount;

    for (auto& quest : questMap)
        questIds.push_back(quest.first);

    std::sort(questIds.begin(), questIds.end());

    sLog.outString("Loading units locations.");
    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        t_unit.type = 0;
        t_unit.guid = ObjectGuid(HIGHGUID_UNIT, creaturePair->second.id, creaturePair->first).GetRawValue();
        t_unit.entry = creaturePair->second.id;
        t_unit.map = creaturePair->second.mapid;
        t_unit.x = creaturePair->second.posX;
        t_unit.y = creaturePair->second.posY;
        t_unit.z = creaturePair->second.posZ;
        t_unit.o = creaturePair->second.orientation;

        entryCount[creaturePair->second.id]++;

        units.push_back(t_unit);
    }

    for (auto& unit : units)
    {
        unit.c = entryCount[unit.entry];
    }

    sLog.outString("Loading game object locations.");
    for (auto& goPair : WorldPosition().getGameObjectsNear())
    {
        t_unit.type = 1;
        t_unit.guid = ObjectGuid(HIGHGUID_GAMEOBJECT, goPair->second.id, goPair->first).GetRawValue();
        t_unit.entry = goPair->second.id;
        t_unit.map = goPair->second.mapid;
        t_unit.x = goPair->second.posX;
        t_unit.y = goPair->second.posY;
        t_unit.z = goPair->second.posZ;
        t_unit.o = goPair->second.orientation;
        t_unit.c = 1;

        units.push_back(t_unit);
    } 

    sLog.outString("Loading quest data.");

    bool loadQuestData = true;

    if (loadQuestData)
    {
        questGuidpMap questMap = GAI_VALUE(questGuidpMap, "quest guidp map");

        for (auto& q : questMap)
        {
            uint32 questId = q.first;

            for (auto& r : q.second)
            {
                uint32 flag = r.first;

                for (auto& e : r.second)
                {
                    int32 entry = e.first;

                    QuestTravelDestination* loc;
                    std::vector<QuestTravelDestination*> locs;

                    if (flag & (uint32)QuestRelationFlag::questGiver)
                    {
                        loc = AddQuestDestination<QuestRelationTravelDestination>(questId, entry, 0);
                        locs.push_back(loc);
                    }
                    if (flag & (uint32)QuestRelationFlag::questTaker)
                    {
                        loc = AddQuestDestination<QuestRelationTravelDestination>(questId, entry, 1);
                        locs.push_back(loc);
                    }
                    if(flag & ((uint32)QuestRelationFlag::objective1 | (uint32)QuestRelationFlag::objective2 | (uint32)QuestRelationFlag::objective3 | (uint32)QuestRelationFlag::objective4))
                    {
                        uint32 objective;
                        if (flag & (uint32)QuestRelationFlag::objective1)
                            objective = 0;
                        else if (flag & (uint32)QuestRelationFlag::objective2)
                            objective = 1;
                        else if (flag & (uint32)QuestRelationFlag::objective3)
                            objective = 2;
                        else if (flag & (uint32)QuestRelationFlag::objective4)
                            objective = 3;

                        loc = AddQuestDestination<QuestObjectiveTravelDestination>(questId, entry, objective);
                        locs.push_back(loc);
                    }

                    for (auto& guidP : e.second)
                    {
                        if (!guidP.isValid())
                            continue; 

                        pointsMap.insert(std::make_pair(guidP.GetRawValue(), guidP));

                        for (auto tLoc : locs)
                        {
                            tLoc->AddPoint(&pointsMap.at(guidP.GetRawValue()));
                        }
                    }
                }
            }
        }
    }

    sLog.outString("Loading Rpg, Grind and Boss locations.");

    GuidPosition point;

    //Rpg locations
    for (auto& u : units)
    {
        RpgTravelDestination* rLoc;
        GrindTravelDestination* gLoc;
        BossTravelDestination* bLoc;
        TravelDestination* tLoc;

        if (u.type == 0)
        {
            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(u.entry);

            if (!cInfo)
                continue;

            if (cInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
                continue;

            std::vector<uint32> allowedNpcFlags;

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

            point = GuidPosition(u.guid, WorldPosition(u.map, u.x, u.y, u.z, u.o));

            if (!point.isValid())
                continue;

            for (auto flag : allowedNpcFlags)
            {
                if ((cInfo->NpcFlags & flag) != 0)
                {
                    rLoc = AddDestination<RpgTravelDestination>(u.entry);

                    pointsMap.insert_or_assign(u.guid, point);
                    rLoc->AddPoint(&pointsMap.at(u.guid));
                    break;
                }
            }

            if (cInfo->MinLootGold > 0)
            {
                gLoc = AddDestination<GrindTravelDestination>(u.entry);

                point = GuidPosition(u.guid, WorldPosition(u.map, u.x, u.y, u.z, u.o));
                pointsMap.insert_or_assign(u.guid, point);
                gLoc->AddPoint(&pointsMap.at(u.guid));
            }

            if (cInfo->Rank == 3 || cInfo->Rank == 4 || (cInfo->Rank == 1 && !point.isOverworld() && u.c == 1))
            {
                bLoc = AddDestination<BossTravelDestination>(u.entry);

                pointsMap.insert_or_assign(u.guid, point);
                bLoc->AddPoint(&pointsMap.at(u.guid));
            }

            if (cInfo->SkinningLootId && cInfo->GetRequiredLootSkill() == SKILL_SKINNING)
            {
                tLoc = AddDestination<GatherTravelDestination>(u.entry);

                pointsMap.insert_or_assign(u.guid, point);
                tLoc->AddPoint(&pointsMap.at(u.guid));                
            }
        }
        else
        {
            GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(u.entry);

            if (!gInfo)
                continue;

            if (gInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
                continue;

            std::vector<uint32> allowedGoTypes;

            allowedGoTypes.push_back(GAMEOBJECT_TYPE_MAILBOX);

            point = GuidPosition(u.guid, WorldPosition(u.map, u.x, u.y, u.z, u.o));

            if (!point.isValid())
                continue;

            uint32 entry = u.entry * -1;

            for (auto type : allowedGoTypes)
            {
                if (gInfo->type == type)
                {
                    rLoc = AddDestination<RpgTravelDestination>(entry);

                    pointsMap.insert_or_assign(u.guid, point);
                    rLoc->AddPoint(&pointsMap.at(u.guid));
                    break;
                }
            }

            if (uint32 lockId = gInfo->GetLockId())
            {
                LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
                if (lockInfo)
                {
                    uint32 skillId = SKILL_NONE;

                    for (int i = 0; i < 8; ++i)
                    {
                        if (lockInfo->Type[i] == LOCK_KEY_SKILL)
                            if (SkillByLockType(LockType(lockInfo->Index[i])) > 0)
                            {
                                skillId = SkillByLockType(LockType(lockInfo->Index[i]));
                                break;
                            }
                    }
            
                    if (skillId == SKILL_LOCKPICKING || skillId == SKILL_MINING || skillId == SKILL_HERBALISM || skillId == SKILL_FISHING)
                    {
                        tLoc = AddDestination<GatherTravelDestination>(entry);

                        pointsMap.insert_or_assign(u.guid, point);
                        tLoc->AddPoint(&pointsMap.at(u.guid));
                    }
                }
            }
        }
    }

    sLog.outString("Loading Explore locations.");

    //Explore points
    for (auto& u : units)
    {
        ExploreTravelDestination* loc;

        GuidPosition point = GuidPosition(u.guid, WorldPosition(u.map, u.x, u.y, u.z, u.o));

        if (!point.isValid())
            continue;

        AreaTableEntry const* area = point.GetArea();
        
        if (!area)
            continue;

        if (!area->exploreFlag)
            continue;

        if (u.type == 1)
            continue;

        int32 guid = u.type == 0 ? u.guid : u.guid * -1;

        pointsMap.insert_or_assign(guid, point);

        loc = AddDestination<ExploreTravelDestination>(area->ID);
        loc->AddPoint(&pointsMap.at(guid));
    }

    //Analyse log files
    if (sPlayerbotAIConfig.hasLog("log_analysis.csv"))
    {
        sLog.outString("Running analysis.");
        LogAnalysis::RunAnalysis();
    }

    sLog.outString("Clearing log files.");

     //Clear these logs files
    sPlayerbotAIConfig.openLog("zones.csv", "w");
    sPlayerbotAIConfig.openLog("creatures.csv", "w");
    sPlayerbotAIConfig.openLog("gos.csv", "w");
    sPlayerbotAIConfig.openLog("bot_movement.csv", "w");
    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_attempt.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_attempt_point.csv", "w");
    sPlayerbotAIConfig.openLog("pathfind_result.csv", "w");
    sPlayerbotAIConfig.openLog("load_map_grid.csv", "w");
    sPlayerbotAIConfig.openLog("strategy.csv", "w");

    sPlayerbotAIConfig.openLog("unload_grid.csv", "w");
    sPlayerbotAIConfig.openLog("unload_obj.csv", "w");
    sPlayerbotAIConfig.openLog("bot_events.csv", "w");
    sPlayerbotAIConfig.openLog("travel_map.csv", "w");
    sPlayerbotAIConfig.openLog("quest_map.csv", "w");
    sPlayerbotAIConfig.openLog("activity_pid.csv", "w");
    sPlayerbotAIConfig.openLog("deaths.csv", "w");
    sPlayerbotAIConfig.openLog("player_paths.csv", "w");

    if (sPlayerbotAIConfig.hasLog("activity_pid.csv"))
    {
        std::ostringstream out;
        out << "Timestamp,";

        out << "sWorld.GetCurrentDiff(),";
        out << "sWorld.GetAverageDiff(),";
        out << "sWorld.GetMaxDiff(),";
        out << "virtualMemUsedByMe" << ",";
        out << "activityPercentage,";
        out << "activityPercentageMod,";
        out << "activeBots,";
        out << "playerBots.size(),";
        out << "totalLevel,";
        out << "avarageLevel1-9,";
        out << "avarageLevel10-19,";
        out << "avarageLevel20-29,";
        out << "avarageLevel30-39,";
        out << "avarageLevel40-49,";
        out << "avarageLevel50-59,";
#ifdef MANGOSBOT_ZERO
        out << "avarageLevel60,";
#else
        out << "avarageLevel60-69,";
#ifdef MANGOSBOT_ONE
        out << "avarageLevel70,";
#else
        out << "avarageLevel70-79,";
        out << "avarageLevel80,";
#endif
#endif

        out << "totalGold,";
        out << "avarageGold,";
        out << "totalavarageGearScore,";
        out << "avarageGearScore";

        sPlayerbotAIConfig.log("activity_pid.csv", out.str().c_str());
    }

#ifdef IKE_PATHFINDER
    bool mmapAvoidMobMod = true;

    if (mmapAvoidMobMod)
    {
        //Mob avoidance
        SetMobAvoidArea();
    }
#endif

    sLog.outString("Loading travel nodes.");

    sTravelNodeMap.loadNodeStore();

    sTravelNodeMap.generateAll();

    sTravelNodeMap.printMap();
    sTravelNodeMap.printNodeStore();
    sTravelNodeMap.saveNodeStore();

    //Creature/gos/zone export.
    if (sPlayerbotAIConfig.hasLog("creatures.csv"))
    {
        sLog.outString("Create creature overlay exports.");

        for (auto& creaturePair : WorldPosition().getCreaturesNear())
        {
            CreatureData const cData = creaturePair->second;
            CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

            if (!cInfo)
                continue;

            WorldPosition point = WorldPosition(cData.mapid, cData.posX, cData.posY, cData.posZ, cData.orientation);

            std::string name = cInfo->Name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << cInfo->MaxLevel << ",";
            out << cInfo->Rank << ",";
            out << cInfo->Faction << ",";
            out << cInfo->NpcFlags << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig.log("creatures.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig.hasLog("vmangoslines.csv"))
    {

        uint32 mapId = 0;
        std::vector<WorldPosition> pos;

            static float const topNorthSouthLimit[] = {
                2032.048340f, -6927.750000f,
                1634.863403f, -6157.505371f,
                1109.519775f, -5181.036133f,
                1315.204712f, -4096.020508f,
                1073.089233f, -3372.571533f,
                 825.833191f, -3125.778809f,
                 657.343994f, -2314.813232f,
                 424.736145f, -1888.283691f,
                 744.395813f, -1647.935425f,
                1424.160645f,  -654.948181f,
                1447.065308f,  -169.751358f,
                1208.715454f,   189.748703f,
                1596.240356f,   998.616699f,
                1577.923706f,  1293.419922f,
                1458.520264f,  1727.373291f,
                1591.916138f,  3728.139404f
            };

            pos.clear();

# define my_sizeof(type) ((char *)(&type+1)-(char*)(&type))

            int size = my_sizeof(topNorthSouthLimit) / my_sizeof(topNorthSouthLimit[0]);

            for (int32 i = 0; i < size-1; i=i+2)
            {
                if (topNorthSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, topNorthSouthLimit[i], topNorthSouthLimit[i + 1], 0));
            }

            std::ostringstream out;
            out << "topNorthSouthLimit" << ",";
            WorldPosition().printWKT(pos,out,1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const ironforgeAreaSouthLimit[] = {
                -7491.33f,  3093.74f,
                -7472.04f,  -391.88f,
                -6366.68f,  -730.10f,
                -6063.96f, -1411.76f,
                -6087.62f, -2190.21f,
                -6349.54f, -2533.66f,
                -6308.63f, -3049.32f,
                -6107.82f, -3345.30f,
                -6008.49f, -3590.52f,
                -5989.37f, -4312.29f,
                -5806.26f, -5864.11f
            };

            pos.clear();

            size = my_sizeof(ironforgeAreaSouthLimit) / my_sizeof(ironforgeAreaSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (ironforgeAreaSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, ironforgeAreaSouthLimit[i], ironforgeAreaSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();
            
            out << "ironforgeAreaSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const stormwindAreaNorthLimit[] = {
                 -8004.25f,  3714.11f,
                 -8075.00f, -179.00f,
                 -8638.00f, 169.00f,
                 -9044.00f, 35.00f,
                 -9068.00f, -125.00f,
                 -9094.00f, -147.00f,
                 -9206.00f, -290.00f,
                 -9097.00f, -510.00f,
                 -8739.00f, -501.00f,
                 -8725.50f, -1618.45f,
                 -9810.40f, -1698.41f,
                -10049.60f, -1740.40f,
                -10670.61f, -1692.51f,
                -10908.48f, -1563.87f,
                -13006.40f, -1622.80f,
                -12863.23f, -4798.42f
            };

            pos.clear();

            size = my_sizeof(stormwindAreaNorthLimit) / my_sizeof(stormwindAreaNorthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (stormwindAreaNorthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, stormwindAreaNorthLimit[i], stormwindAreaNorthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "stormwindAreaNorthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const stormwindAreaSouthLimit[] = {
                 -8725.337891f,  3535.624023f,
                 -9525.699219f,   910.132568f,
                 -9796.953125f,   839.069580f,
                 -9946.341797f,   743.102844f,
                -10287.361328f,   760.076477f,
                -10083.828125f,   380.389893f,
                -10148.072266f,    80.056450f,
                -10014.583984f,  -161.638519f,
                 -9978.146484f,  -361.638031f,
                 -9877.489258f,  -563.304871f,
                 -9980.967773f, -1128.510498f,
                 -9991.717773f, -1428.793213f,
                 -9887.579102f, -1618.514038f,
                -10169.600586f, -1801.582031f,
                 -9966.274414f, -2227.197754f,
                 -9861.309570f, -2989.841064f,
                 -9944.026367f, -3205.886963f,
                 -9610.209961f, -3648.369385f,
                 -7949.329590f, -4081.389404f,
                 -7910.859375f, -5855.578125f
            };

            pos.clear();

            size = my_sizeof(stormwindAreaSouthLimit) / my_sizeof(stormwindAreaSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (stormwindAreaSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, stormwindAreaSouthLimit[i], stormwindAreaSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "stormwindAreaSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());
            
            mapId = 1;

            static float const teldrassilSouthLimit[] = {
            7916.0f,   3475.0f,
            7916.0f,   1000.0f,
            8283.0f,   -501.0f,
            8804.0f,   -1098.0f
            };

            pos.clear();

            size = my_sizeof(teldrassilSouthLimit) / my_sizeof(teldrassilSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (teldrassilSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, teldrassilSouthLimit[i], teldrassilSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "teldrassilSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());
       
            static float const northMiddleLimit[] = {
                  -2280.00f,  4054.00f,
                  -2401.00f,  2365.00f,
                  -2432.00f,  1338.00f,
                  -2286.00f,   769.00f,
                  -2137.00f,   662.00f,
                  -2044.54f,   489.86f,
                  -1808.52f,   436.39f,
                  -1754.85f,   504.55f,
                  -1094.55f,   651.75f,
                   -747.46f,   647.73f,
                   -685.55f,   408.43f,
                   -311.38f,   114.43f,
                   -358.40f,  -587.42f,
                   -377.92f,  -748.70f,
                   -512.57f,  -919.49f,
                   -280.65f, -1008.87f,
                    -81.29f,  -930.89f,
                    284.31f, -1105.39f,
                    568.86f,  -892.28f,
                   1211.09f, -1135.55f,
                    879.60f, -2110.18f,
                    788.96f, -2276.02f,
                    899.68f, -2625.56f,
                   1281.54f, -2689.42f,
                   1521.82f, -3047.85f,
                   1424.22f, -3365.69f,
                   1694.11f, -3615.20f,
                   2373.78f, -4019.96f,
                   2388.13f, -5124.35f,
                   2193.79f, -5484.38f,
                   1703.57f, -5510.53f,
                   1497.59f, -6376.56f,
                   1368.00f, -8530.00f
            };

            pos.clear();

            size = my_sizeof(northMiddleLimit) / my_sizeof(northMiddleLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (northMiddleLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, northMiddleLimit[i], northMiddleLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "northMiddleLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const durotarSouthLimit[] = {
                    2755.00f, -3766.00f,
                    2225.00f, -3596.00f,
                    1762.00f, -3746.00f,
                    1564.00f, -3943.00f,
                    1184.00f, -3915.00f,
                     737.00f, -3782.00f,
                     -75.00f, -3742.00f,
                    -263.00f, -3836.00f,
                    -173.00f, -4064.00f,
                     -81.00f, -4091.00f,
                     -49.00f, -4089.00f,
                     -16.00f, -4187.00f,
                      -5.00f, -4192.00f,
                     -14.00f, -4551.00f,
                    -397.00f, -4601.00f,
                    -522.00f, -4583.00f,
                    -668.00f, -4539.00f,
                    -790.00f, -4502.00f,
                   -1176.00f, -4213.00f,
                   -1387.00f, -4674.00f,
                   -2243.00f, -6046.00f
            };

            pos.clear();

            size = my_sizeof(durotarSouthLimit) / my_sizeof(durotarSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (durotarSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, durotarSouthLimit[i], durotarSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "durotarSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const valleyoftrialsSouthLimit[] = {
                    -324.00f, -3869.00f,
                    -774.00f, -3992.00f,
                    -965.00f, -4290.00f,
                    -932.00f, -4349.00f,
                    -828.00f, -4414.00f,
                    -661.00f, -4541.00f,
                    -521.00f, -4582.00f
            };

            pos.clear();

            size = my_sizeof(valleyoftrialsSouthLimit) / my_sizeof(valleyoftrialsSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (valleyoftrialsSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, valleyoftrialsSouthLimit[i], valleyoftrialsSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "valleyoftrialsSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const middleToSouthLimit[] = {
                        -2402.01f,      4255.70f,
                    -2475.933105f,  3199.568359f, // Desolace
                    -2344.124023f,  1756.164307f,
                    -2826.438965f,   403.824738f, // Mulgore
                    -3472.819580f,   182.522476f, // Feralas
                    -4365.006836f, -1602.575439f, // the Barrens
                    -4515.219727f, -1681.356079f,
                    -4543.093750f, -1882.869385f, // Thousand Needles
                        -4824.16f,     -2310.11f,
                    -5102.913574f, -2647.062744f,
                    -5248.286621f, -3034.536377f,
                    -5246.920898f, -3339.139893f,
                    -5459.449707f, -4920.155273f, // Tanaris
                        -5437.00f,     -5863.00f
            };

            pos.clear();

            size = my_sizeof(middleToSouthLimit) / my_sizeof(middleToSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (middleToSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, middleToSouthLimit[i], middleToSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "middleToSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const orgrimmarSouthLimit[] = {
                    2132.5076f, -3912.2478f,
                    1944.4298f, -3855.2583f,
                    1735.6906f, -3834.2417f,
                    1654.3671f, -3380.9902f,
                    1593.9861f, -3975.5413f,
                    1439.2548f, -4249.6923f,
                    1436.3106f, -4007.8950f,
                    1393.3199f, -4196.0625f,
                    1445.2428f, -4373.9052f,
                    1407.2349f, -4429.4145f,
                    1464.7142f, -4545.2875f,
                    1584.1331f, -4596.8764f,
                    1716.8065f, -4601.1323f,
                    1875.8312f, -4788.7187f,
                    1979.7647f, -4883.4585f,
                    2219.1562f, -4854.3330f
            };

            pos.clear();

            size = my_sizeof(orgrimmarSouthLimit) / my_sizeof(orgrimmarSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (orgrimmarSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, orgrimmarSouthLimit[i], orgrimmarSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "orgrimmarSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const feralasThousandNeedlesSouthLimit[] = {
                    -6495.4995f, -4711.981f,
                    -6674.9995f, -4515.0019f,
                    -6769.5717f, -4122.4272f,
                    -6838.2651f, -3874.2792f,
                    -6851.1314f, -3659.1179f,
                    -6624.6845f, -3063.3843f,
                    -6416.9067f, -2570.1301f,
                    -5959.8466f, -2287.2634f,
                    -5947.9135f, -1866.5028f,
                    -5947.9135f,  -820.4881f,
                    -5876.7114f,    -3.5138f,
                    -5876.7114f,   917.6407f,
                    -6099.3603f,  1153.2884f,
                    -6021.8989f,  1638.1809f,
                    -6091.6176f,  2335.8892f,
                    -6744.9946f,  2393.4855f,
                    -6973.8608f,  3077.0281f,
                    -7068.7241f,  4376.2304f,
                    -7142.1211f,  4808.4331f
            };


            pos.clear();

            size = my_sizeof(feralasThousandNeedlesSouthLimit) / my_sizeof(feralasThousandNeedlesSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (feralasThousandNeedlesSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, feralasThousandNeedlesSouthLimit[i], feralasThousandNeedlesSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "feralasThousandNeedlesSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            mapId = 530;

            static float const ShattrathAreaSouthLimit[] = {
            -2493.8823f,  5761.6894f,
            -2593.7438f,  4768.7978f,
            -1831.5280f,  3383.5705f
            };

            pos.clear();

            size = my_sizeof(ShattrathAreaSouthLimit) / my_sizeof(ShattrathAreaSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (ShattrathAreaSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, ShattrathAreaSouthLimit[i], ShattrathAreaSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "ShattrathAreaSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const HellfireZangarSouthLimit[] = {

            -531.47265f,  8697.5830f,
            -514.56945f,  7291.2763f,
            -404.92804f,  6976.7958f,
            -593.56475f,  6646.0634f,
            -856.75695f,  6318.5507f,
            -1166.2729f,  5799.7817f,
            -1007.9321f,  4761.1352f,
            -1831.5280f,  3383.5705f,
            -2135.1586f,  2335.4426f,
            -2179.3974f,  896.0285f,
            };

            pos.clear();

            size = my_sizeof(HellfireZangarSouthLimit) / my_sizeof(HellfireZangarSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (HellfireZangarSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, HellfireZangarSouthLimit[i], HellfireZangarSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "HellfireZangarSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());

            static float const BladeEdgeNetherSouthLimit[] = {
            2074.6831f,  8216.6113f,
            1248.3884f,  7472.7592f,
            1118.4877f,  6972.6821f,
            1212.2004f,  6106.2861f,
            1175.4729f,  5633.375f,
            1543.8314f,  3961.8886f,
            };

            pos.clear();

            size = my_sizeof(BladeEdgeNetherSouthLimit) / my_sizeof(BladeEdgeNetherSouthLimit[0]);

            for (int32 i = 0; i < size - 1; i = i + 2)
            {
                if (BladeEdgeNetherSouthLimit[i] == 0)
                    break;
                pos.push_back(WorldPosition(mapId, BladeEdgeNetherSouthLimit[i], BladeEdgeNetherSouthLimit[i + 1], 0));
            }

            out.str("");
            out.clear();

            out << "BladeEdgeNetherSouthLimit" << ",";
            WorldPosition().printWKT(pos, out, 1);
            out << std::fixed;

            sPlayerbotAIConfig.log("vmangoslines.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig.hasLog("gos.csv"))
    {
        sLog.outString("Create go overlay exports.");
        for (auto& gameObjectPair : WorldPosition().getGameObjectsNear())
        {
            GameObjectData const gData = gameObjectPair->second;
            auto data = sGOStorage.LookupEntry<GameObjectInfo>(gData.id);

            if (!data)
                continue;

            WorldPosition point = WorldPosition(gData.mapid, gData.posX, gData.posY, gData.posZ, gData.orientation);

            std::string name = data->name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << data->type << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig.log("gos.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig.hasLog("zones.csv"))
    {
        sLog.outString("Create zone overlay exports.");

        std::unordered_map<std::string, std::vector<WorldPosition>> zoneLocs;

        std::vector<WorldPosition> Locs = {};
        
        for (auto& u : units)
        {
            WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            std::string name = std::to_string(u.map) + point.getAreaName();

            if (zoneLocs.find(name) == zoneLocs.end())
                zoneLocs.insert_or_assign(name, Locs);

            zoneLocs.at(name).push_back(point);            
        }        

        for (auto& loc : zoneLocs)
        {
            if (loc.second.empty())
                continue;

            if (!sTravelNodeMap.getMapOffset(loc.second.front().getMapId()) && loc.second.front().getMapId() != 0)
                continue;

            std::vector<WorldPosition> points = loc.second;;
           
            std::ostringstream out; 

            WorldPosition pos = WorldPosition(points, WP_MEAN_CENTROID);

            out << "\"center\"" << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            pos.printWKT(out);

            if(points.begin()->GetArea())
                out << std::to_string(points.begin()->getAreaLevel());
            else
                out << std::to_string(-1);

            out << "\n";
            
            out << "\"area\"" << ",";
            out << points.begin()->getMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            point.printWKT(points, out, 0);

            if (points.begin()->GetArea())
                out << std::to_string(points.begin()->getAreaLevel());
            else
                out << std::to_string(-1);

            sPlayerbotAIConfig.log("zones.csv", out.str().c_str());
        }
    }    

    if (sPlayerbotAIConfig.hasLog("telecache.csv"))
    {
        sLog.outString("Create telecache overlay exports.");

        sRandomPlayerbotMgr.PrintTeleportCache();
    }

#ifndef MANGOSBOT_TWO    
    sTerrainMgr.Update(60 * 60 * 24);
#else
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        if (!sMapStore.LookupEntry(i))
            continue;

        uint32 mapId = sMapStore.LookupEntry(i)->MapID;

        if (WorldPosition(mapId, 0, 0).getMap(0))
            continue;

        WorldPosition::unloadMapAndVMaps(mapId);
    }
#endif     
}

std::vector<TravelDestination*> TravelMgr::GetDestinations(const PlayerTravelInfo& info, std::type_index type, int32 entry, int32 subEntry1, int32 subEntry2, bool onlyPossible, float maxDistance) const
{
    WorldPosition center = info.GetPosition();
    std::vector<TravelDestination*> retTravelLocations;
    
    std::mutex resultMutex;

    std::for_each(
        std::execution::par,
        destinationMap.at(type).begin(),
        destinationMap.at(type).end(),
        [&](TravelDestination* dest) {
            if (type == typeid(QuestTravelDestination))
            {
                if (entry && ((QuestTravelDestination*)dest)->GetQuestId() != entry)
                    return;
            }
            else
            {
                if (entry && dest->GetEntry() != entry)
                    return;
            }

            if (subEntry1 && dest->GetSubEntry() != subEntry1 && dest->GetSubEntry() != subEntry2)
                return;

            if (onlyPossible && !dest->IsPossible(info))
                return;

            if (maxDistance > 0 && dest->DistanceTo(center) > maxDistance)
                return;

            std::lock_guard<std::mutex> guard(resultMutex);
            retTravelLocations.push_back(dest);
        });            

    return retTravelLocations;
}

void TravelMgr::SetNullTravelTarget(TravelTarget* target) const
{
    if (target)
        target->SetTarget(nullTravelDestination, nullWorldPosition);
}

void TravelMgr::SetNullTravelTarget(Player* player) const
{
    if (!player)
        return;

    if (!player->GetPlayerbotAI())
        return;

    TravelTarget* target = player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

    SetNullTravelTarget(target);
}

void TravelMgr::AddMapTransfer(WorldPosition start, WorldPosition end, float portalDistance, bool makeShortcuts)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return;
    
    //Calculate shortcuts.
    if(makeShortcuts)
        for (auto& mapTransfers : mapTransfersMap)
        {
            uint32 sMapt = mapTransfers.first.first;
            uint32 eMapt = mapTransfers.first.second;

            for (auto& mapTransfer : mapTransfers.second)
            {
                if (eMapt == sMap && sMapt != eMap) // [S1 >MT> E1 -> S2] >THIS> E2
                {
                    float newDistToEnd = MapTransDistance(mapTransfer.GetPointFrom(), start) + portalDistance;
                    if (MapTransDistance(mapTransfer.GetPointFrom(), end) > newDistToEnd)
                        AddMapTransfer(mapTransfer.GetPointFrom(), end, newDistToEnd, false);
                }

                if (sMapt == eMap && eMapt != sMap) // S1 >THIS> [E1 -> S2 >MT> E2]
                {
                    float newDistToEnd = portalDistance + MapTransDistance(end, mapTransfer.GetPointTo());
                    if (MapTransDistance(start, mapTransfer.GetPointTo()) > newDistToEnd)
                        AddMapTransfer(start, mapTransfer.GetPointTo(), newDistToEnd, false);
                }
            }
        }

    //Add actual transfer.
    auto mapTransfers = mapTransfersMap.find(std::make_pair(start.getMapId(), end.getMapId()));
    
    if (mapTransfers == mapTransfersMap.end())
        mapTransfersMap.insert({ { sMap, eMap }, {MapTransfer(start, end, portalDistance)} });
    else
        mapTransfers->second.push_back(MapTransfer(start, end, portalDistance));        
};

void TravelMgr::LoadMapTransfers()
{
    for (auto& node : sTravelNodeMap.getNodes())
    {
        for (auto& [node, path] : *node->getLinks())
        {
            AddMapTransfer(*node->getPosition(), *node->getPosition(), path->getDistance());
        }
    }
}

float TravelMgr::MapTransDistance(const WorldPosition& start, const WorldPosition& end, bool toMap) const
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.distance(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({ sMap, eMap });
    
    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        WorldPosition realEnd = end;
        if (toMap) realEnd = mapTrans.GetPointTo();

        float dist = mapTrans.Distance(start, realEnd);

        if (dist < minDist)
            minDist = dist;
    }    

    return minDist;
}

float TravelMgr::FastMapTransDistance(const WorldPosition& start, const WorldPosition& end, bool toMap) const
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.fDist(end);

    float minDist = 200000;

    auto mapTransfers = mapTransfersMap.find({ sMap, eMap });

    if (mapTransfers == mapTransfersMap.end())
        return minDist;

    for (auto& mapTrans : mapTransfers->second)
    {
        WorldPosition realEnd = end;
        if (toMap) realEnd = mapTrans.GetPointTo();

        float dist = mapTrans.FDist(start, end);

        if (dist < minDist)
            minDist = dist;
    }

    return minDist;
}