#pragma once

#include "strategy/AiObject.h"
#include <boost/functional/hash.hpp>
#include "GuidPosition.h"
#include "strategy/values/TravelValues.h"

namespace ai
{
	class GuidePosition;

	class MapTransfer
	{
	public:
		MapTransfer(const WorldPosition& pointFrom, const WorldPosition& pointTo, float portalLength = 0.1f)
			: pointFrom(pointFrom), pointTo(pointTo), portalLength(portalLength) {
		}
		float Distance(const WorldPosition& start, const WorldPosition& end) const { return (isUseful(start, end) ? (start.distance(pointFrom) + portalLength + pointTo.distance(end)) : 200000); }

		float FDist(const WorldPosition& start, const WorldPosition& end) const { return start.fDist(pointFrom) + portalLength + pointTo.fDist(end); }

		WorldPosition GetPointFrom() const { return pointFrom; }
		WorldPosition GetPointTo() const { return pointTo; }
	private:
		bool isFrom(const WorldPosition& point) const { return point.getMapId() == pointFrom.getMapId(); }
		bool isTo(const WorldPosition& point) const { return point.getMapId() == pointTo.getMapId(); }

		bool isUseful(const WorldPosition& point) const { return isFrom(point) || isTo(point); }
		bool isUseful(const WorldPosition& start, const WorldPosition& end) const { return isFrom(start) && isTo(end); }

		float Distance(const WorldPosition& point) const { return isUseful(point) ? (isFrom(point) ? point.distance(pointFrom) : point.distance(pointTo)) : 200000; }

		WorldPosition pointFrom, pointTo;
		float portalLength = 0.1f;
	};

	class PlayerTravelInfo 
	{
	public:
		PlayerTravelInfo() {};
		PlayerTravelInfo(Player* player);
		
		WorldPosition GetPosition() const { return position; }
		Team GetTeam() const { return team; }
		uint32 GetLevel() const  { return level; }
		uint16 GetCurrentSkill(SkillType skillType) const  { return currentSkill[skillType]; }
		uint16 GetSkillMax(SkillType skillType) const { return skillMax[skillType]; }
		bool IsInRaid() const { return groupSize > 5; }
		bool IsInGroup() const { return groupSize > 1 && groupSize <= 5; }
		uint32 GetMoney() const { return money; }

		bool HasFocusQuest() const {return !focusList.empty();}
		bool IsFocusQuest(uint32 questId) const { return focusList.find(questId) != focusList.end(); }

		bool GetBoolValue(const std::string& valueName) const {return boolValues.at(valueName);};
		bool GetBoolValue2(const std::string& valueName, const std::string& qualifier) const { return boolValues.at(valueName + "::" + qualifier); };
		uint8 GetUint8Value(const std::string& valueName) const { return uint8Values.at(valueName);};

	private:
		WorldPosition position;
		Team team = TEAM_NONE;
		uint32 level = 0;
		uint16 currentSkill[MAX_SKILL_TYPE] = {0};
		uint16 skillMax[MAX_SKILL_TYPE] = { 0 };
		uint8 groupSize = 0;
		uint32 money = 0;

		focusQuestTravelList focusList = {};

		std::unordered_map<std::string, bool> boolValues = 
		{
			{"group or::should sell,can sell,following party,near leader", false},
			{"group or::should repair,can repair,following party,near leader", false},
			{"group or::should ah sell,can ah sell,following party,near leader", false},
			{"should get money", false},
			{"should sell", false},
			{"can sell", false},
			{"should repair", false},
			{"can repair", false},
			{"should ah sell", false},
			{"can ah sell", false},
			{"can get mail", false },
			{"has strategy::free", false},
			{"has strategy::rpg quest", false},
			{"can fight equal", false},
			{"can fight elite", false},
			{"can fight boss", false},
		};

		std::unordered_map<std::string, uint8> uint8Values =
		{
			{"free quest log slots", 0},
			{"durability",0},
		};
	};

	//A destination for a bot to travel to and do something.
	class TravelDestination
	{
	public:
		TravelDestination() {}

		void AddPoint(WorldPosition* pos) { points.push_back(pos); }

		virtual std::string GetTitle() const { return "generic travel destination"; }

		uint32 GetExpireDelay() { return expireDelay; }
		uint32 GetCooldownDelay() { return cooldownDelay; }

		virtual bool IsIn(const WorldPosition& pos, float radius = 0) const { return OnMap(pos) && DistanceTo(pos) <= (radius ? radius : radiusMin); }
		float DistanceTo(const WorldPosition& pos) const { return NearestPoint(pos)->distance(pos); }

		float GetRadiusMin() { return radiusMin; }
		bool HasPoint(const WorldPosition* pos) { return std::find(points.begin(), points.end(), pos) != points.end(); }
		const std::vector<WorldPosition*>& GetPoints() const {return points;};

		virtual bool IsPossible(const PlayerTravelInfo& info) const { return false; }
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const { return false; }

		virtual int32 GetEntry() const { return 0; }
		virtual uint8 GetSubEntry() const { return 0; }
		WorldPosition* NearestPoint(const WorldPosition& pos) const;
		std::vector<WorldPosition*> NextPoint(const WorldPosition& pos) const;
	protected:
		void SetExpireFast() { expireDelay = 60000; } //1 minute
		void SetCooldownShort() { cooldownDelay = 1000; } //1 second
		void SetCooldownLong() { cooldownDelay = 300000; } //5 minutes
		virtual std::vector<std::string> GetTravelConditions() const { return {}; }

		virtual bool IsOut(const WorldPosition& pos, float radius = 0) const { return !OnMap(pos) || DistanceTo(pos) > (radius ? radius : radiusMax); }
	private:
		bool OnMap(const WorldPosition& pos) const { return NearestPoint(pos)->getMapId() == pos.getMapId(); }

		std::vector<WorldPosition*> points;
		float radiusMin = sPlayerbotAIConfig.tooCloseDistance;
		float radiusMax = sPlayerbotAIConfig.sightDistance;

		uint32 expireDelay = 300000; //5 minutes
		uint32 cooldownDelay = 60000; //1 minute
	};

	//A travel target that is always inactive and jumps to cooldown.
	class NullTravelDestination : public TravelDestination
	{
	public:
		NullTravelDestination() : TravelDestination() { SetCooldownLong(); };

		virtual std::string GetTitle() const override { return "no destination"; }

		virtual bool IsIn(const WorldPosition& pos, float radius = 0) const override { return true; }
	protected:
		virtual bool IsOut(const WorldPosition& pos, float radius = 0) const override { return false; }
	};

	class EntryTravelDestination : public TravelDestination
	{
	public:
		EntryTravelDestination(int32 entry) : TravelDestination(), entry(entry) { if (entry > 0) creatureInfo = ObjectMgr::GetCreatureTemplate(entry); else goInfo = ObjectMgr::GetGameObjectInfo(-1 * entry); }
		virtual int32 GetEntry() const override { return entry; }
		virtual GameObjectInfo const* GetGoInfo() const { return goInfo; }
		virtual CreatureInfo const* GetCreatureInfo() const { return creatureInfo; }
	private:
		CreatureInfo const* creatureInfo = nullptr;
		GameObjectInfo const* goInfo = nullptr;
		int32 entry = 0;
	};

	//A travel target specifically related to a quest.
	class QuestTravelDestination : public EntryTravelDestination
	{
	public:
		QuestTravelDestination(uint32 questId, int32 entry, uint8 subEntry) : EntryTravelDestination(entry), questId(questId), subEntry(subEntry) { questTemplate = sObjectMgr.GetQuestTemplate(questId); };

		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override { return bot->IsActiveQuest(questId); }

		virtual std::string GetTitle() const override;

		virtual bool IsClassQuest() const { return questTemplate->GetRequiredClasses(); }
		virtual int32 GetQuestId() const { return questId; }
	protected:
		virtual Quest const* GetQuestTemplate() const { return questTemplate; }
		virtual uint8 GetSubEntry() const override { return subEntry; }
	private:
		uint8 subEntry;
		uint32 questId;
		Quest const* questTemplate;
	};

	enum class QuestTravelSubEntry : uint8
	{
		ALL = 0,
		OBJECTIVE1 = 1,
		OBJECTIVE2 = 2,
		OBJECTIVE3 = 3,
		OBJECTIVE4 = 4,
		QUESTGIVER = 5,
		QUESTTAKER = 6,
		NONE = 7
	};

	//A quest giver or taker.
	class QuestRelationTravelDestination : public QuestTravelDestination
	{
	public:
		QuestRelationTravelDestination(uint32 questId, int32 entry, uint8 relation) : QuestTravelDestination(questId, entry, relation + (uint8)QuestTravelSubEntry::QUESTGIVER) {}

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override;

		virtual uint8 GetRelation() const { return GetSubEntry() - (uint8)QuestTravelSubEntry::QUESTGIVER; }
	};

	//A quest objective (creature/gameobject to grind/loot)
	class QuestObjectiveTravelDestination : public QuestTravelDestination
	{
	public:
		QuestObjectiveTravelDestination(uint32 questId, int32 entry, uint8 objective) : QuestTravelDestination(questId, entry, objective + (uint8)QuestTravelSubEntry::OBJECTIVE1) { SetExpireFast(); };

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override;

		virtual uint8 GetObjective() const { return GetSubEntry() - (uint8)QuestTravelSubEntry::OBJECTIVE1; }
	};

	//A location with rpg target(s) based on race and level
	class RpgTravelDestination : public EntryTravelDestination
	{
	public:
		RpgTravelDestination(int32 entry) : EntryTravelDestination(entry) {}

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with zone exploration target(s) 
	class ExploreTravelDestination : public EntryTravelDestination
	{
	public:
		ExploreTravelDestination(int32 areaId) : EntryTravelDestination(areaId) {
			SetExpireFast(); SetCooldownShort(); if (auto area = GetArea()) { title = area->area_name[0]; level = area->area_level; }}

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override { return title; }
	private:
		AreaTableEntry const* GetArea() const;
		std::string title = "";
		int32 level = 0;
	};

	//A location with zone exploration target(s) 
	class GrindTravelDestination : public EntryTravelDestination
	{
	public:
		GrindTravelDestination(int32 entry) : EntryTravelDestination(entry) {}

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with a boss
	class BossTravelDestination : public EntryTravelDestination
	{
	public:
		BossTravelDestination(int32 entry) : EntryTravelDestination(entry) { SetCooldownShort(); }

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with a object that can be gathered
	class GatherTravelDestination : public EntryTravelDestination
	{
	public:
		GatherTravelDestination(int32 entry) : EntryTravelDestination(entry) {}

		virtual bool IsPossible(const PlayerTravelInfo& info) const override;
		virtual bool IsActive(Player* bot, const PlayerTravelInfo& info) const override;
		virtual std::string GetTitle() const override;
	};

	enum class TravelState : uint8
	{
		TRAVEL_STATE_IDLE = 0,
		TRAVEL_STATE_TRAVEL_PICK_UP_QUEST = 1,
		TRAVEL_STATE_WORK_PICK_UP_QUEST = 2,
		TRAVEL_STATE_TRAVEL_DO_QUEST = 3,
		TRAVEL_STATE_WORK_DO_QUEST = 4,
		TRAVEL_STATE_TRAVEL_HAND_IN_QUEST = 5,
		TRAVEL_STATE_WORK_HAND_IN_QUEST = 6,
		TRAVEL_STATE_TRAVEL_RPG = 7,
		TRAVEL_STATE_TRAVEL_EXPLORE = 8,
		MAX_TRAVEL_STATE
	};

	enum class TravelStatus : uint8
	{
		TRAVEL_STATUS_NONE = 0,
		TRAVEL_STATUS_PREPARE = 1,
		TRAVEL_STATUS_TRAVEL = 2,
		TRAVEL_STATUS_WORK = 3,
		TRAVEL_STATUS_COOLDOWN = 4,
		TRAVEL_STATUS_EXPIRED = 5,
		MAX_TRAVEL_STATUS
	};

	//Current target and location for the bot to travel to.
	//The flow is as follows:
	//PREPARE   (wait until no loot is near)
	//TRAVEL    (move towards target until close enough) (rpg and grind is disabled)
	//WORK      (grind/rpg until the target is no longer active) (rpg and grind is enabled on quest mobs)
	//COOLDOWN  (wait some time free to do what the bot wants)
	//EXPIRE    (if any of the above actions take too long pick a new target)
	class TravelTarget : AiObject
	{
	public:
		TravelTarget(PlayerbotAI* ai);
		TravelTarget(PlayerbotAI* ai, TravelDestination* tDestination1, WorldPosition* wPosition1) : AiObject(ai) { SetTarget(tDestination1, wPosition1); }
		~TravelTarget() = default;

		float Distance(Player* bot) const { WorldPosition pos(bot);  return wPosition->distance(pos); };
		TravelDestination* GetDestination() const { return tDestination; };
		WorldPosition* GetPosition() const { return wPosition; };
		std::string GetPosStr() const { return wPosition->to_string(); }

		int32 GetEntry() const { if (!tDestination) return 0; return tDestination->GetEntry(); }
		TravelStatus GetStatus() const { return m_status; }
		TravelState GetTravelState();

		bool IsGroupCopy() const { return groupCopy; }
		bool IsForced() const { return forced; }

		bool IsActive();
		bool IsTraveling();
		bool IsWorking();

		uint32 GetRetryCount(bool isMove) const { return isMove ? moveRetryCount : extendRetryCount; }
		uint32 GetTimeLeft() const { return statusTime - GetExpiredTime(); }
		uint32 GetExpiredTime() const { return WorldTimer::getMSTime() - startTime; }

		void SetRetry(bool isMove, uint32 newCount = 0) { if (isMove) moveRetryCount = newCount; else extendRetryCount = newCount; }
		bool IsMaxRetry(bool isMove) { return isMove ? (moveRetryCount > 10) : (extendRetryCount > 5); }

		void SetTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1 = false);

		void SetStatus(TravelStatus status);
		void SetExpireIn(uint32 expireMs) { statusTime = GetExpiredTime() + expireMs; }
		void SetForced(bool forced1) { forced = forced1; }
		void SetGroupCopy(bool isGroupCopy = true) { groupCopy = isGroupCopy; }
		void SetRadius(float radius1) { radius = radius1; }

		void IncRetry(bool isMove) { if (isMove) moveRetryCount += 2; else extendRetryCount += 2; }
		void DecRetry(bool isMove) { if (isMove && moveRetryCount > 0) moveRetryCount--; else if (extendRetryCount > 0) extendRetryCount--; }

		void CopyTarget(TravelTarget* const target);
	private:
		uint32 GetMaxTravelTime() const { return (1000.0 * Distance(bot)) / bot->GetSpeed(MOVE_RUN); }

 		bool IsPreparing();

		TravelStatus m_status = TravelStatus::TRAVEL_STATUS_NONE;

		uint32 startTime = WorldTimer::getMSTime();
		uint32 statusTime = 0;

		bool forced = false;
		float radius = 0;
		bool groupCopy = false;
		bool visitor = true;

		uint32 extendRetryCount = 0;
		uint32 moveRetryCount = 0;

		TravelDestination* tDestination = nullptr;
		std::vector<std::string> travelConditions = {};
		WorldPosition* wPosition = nullptr;
	};


	typedef std::vector<TravelDestination*> DestinationList;
	//typedef std::unordered_map<int32, DestinationList> DestinationMap;
	typedef std::unordered_map<std::type_index, DestinationList> TypedDestinationMap;

	//General container for all travel destinations.
	class TravelMgr
	{
	public:
		TravelMgr() {};
		void LoadQuestTravelTable();
		std::vector<TravelDestination*> GetExploreLocs() const {return destinationMap.at(typeid(ExploreTravelDestination));}
		void SetMobAvoidArea();

		std::vector<TravelDestination*> GetDestinations(const PlayerTravelInfo& info, std::type_index type, int32 entry = 0, int32 subEntry1 = 0, int32 subEntry2 = 0, bool onlyPossible = true, float maxDistance = 5000) const;

		void SetNullTravelTarget(TravelTarget* target) const;

		void LoadMapTransfers();
		float MapTransDistance(const WorldPosition& start, const WorldPosition& end, bool toMap = false) const;
		float FastMapTransDistance(const WorldPosition& start, const WorldPosition& end, bool toMap = false) const;

		void AddBadMmap(uint32 mapId, int x, int y) { badMmap.push_back(std::make_tuple(mapId, x, y)); }
		bool IsBadMmap(uint32 mapId, int x, int y) const { return std::find(badMmap.begin(), badMmap.end(), std::make_tuple(mapId, x, y)) != badMmap.end(); }

		int32 GetAreaLevel(uint32 area_id);
		void LoadAreaLevels();
	private:
		void Clear();
		void SetMobAvoidAreaMap(uint32 mapId);

		void SetNullTravelTarget(Player* player) const;
		void AddMapTransfer(WorldPosition start, WorldPosition end, float portalDistance = 0.1f, bool makeShortcuts = true);

		template<class T>
		T* AddDestination(int32 entry) {
			for (auto& dest : destinationMap[typeid(T)])
				if (dest->GetEntry() == entry)
					return (T*)dest;

			destinationMap[typeid(T)].push_back(new T(entry));

			return (T*)destinationMap[typeid(T)].back();
		}

		template<class T>
		T* AddQuestDestination(int32 questId, uint32 entry, uint32 subEntry) {
			for (auto& dest : destinationMap[typeid(QuestTravelDestination)])
				if (((QuestTravelDestination*)dest)->GetQuestId() == questId && dest->GetEntry() == entry && dest->GetSubEntry() == subEntry)
					return (T*)dest;

			destinationMap[typeid(QuestTravelDestination)].push_back(new T(questId, entry, subEntry));

			return (T*)destinationMap[typeid(QuestTravelDestination)].back();
		}

		NullTravelDestination* nullTravelDestination = new NullTravelDestination();
		WorldPosition* nullWorldPosition = new WorldPosition();
		TypedDestinationMap destinationMap;

		std::unordered_map<uint64, GuidPosition> pointsMap;
		std::unordered_map<uint32, int32> areaLevels;

		std::vector<std::tuple<uint32, int, int>> badMmap;

		std::unordered_map<std::pair<uint32, uint32>, std::vector<MapTransfer>, boost::hash<std::pair<uint32, uint32>>> mapTransfersMap;
	};
}

#define sTravelMgr MaNGOS::Singleton<TravelMgr>::Instance()

