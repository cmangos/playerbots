#pragma once

#include "strategy/AiObject.h"
#include <boost/functional/hash.hpp>
#include "GuidPosition.h"

namespace ai
{
	class GuidePosition;

	class mapTransfer
	{
	public:
		mapTransfer(WorldPosition pointFrom1, WorldPosition pointTo1, float portalLength1 = 0.1f)
			: pointFrom(pointFrom1), pointTo(pointTo1), portalLength(portalLength1) {
		}

		bool isFrom(WorldPosition point) { return point.getMapId() == pointFrom.getMapId(); }
		bool isTo(WorldPosition point) { return point.getMapId() == pointTo.getMapId(); }

		WorldPosition* getPointFrom() { return &pointFrom; }
		WorldPosition* getPointTo() { return &pointTo; }

		bool isUseful(WorldPosition point) { return isFrom(point) || isTo(point); }
		float distance(WorldPosition point) { return isUseful(point) ? (isFrom(point) ? point.distance(pointFrom) : point.distance(pointTo)) : 200000; }

		bool isUseful(WorldPosition start, WorldPosition end) { return isFrom(start) && isTo(end); }
		float distance(WorldPosition start, WorldPosition end) { return (isUseful(start, end) ? (start.distance(pointFrom) + portalLength + pointTo.distance(end)) : 200000); }
		float fDist(WorldPosition start, WorldPosition end) { return start.fDist(pointFrom) + portalLength + pointTo.fDist(end); }
	private:
		WorldPosition pointFrom, pointTo;
		float portalLength = 0.1f;
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
		float DistanceTo(const WorldPosition& pos) { return NearestPoint(pos)->distance(pos); }

		float GetRadiusMin() { return radiusMin; }
		bool HasPoint(const WorldPosition* pos) { return std::find(points.begin(), points.end(), pos) != points.end(); }
		std::vector<WorldPosition*> GetPoints() const;

		virtual bool IsActive(const Player* bot) const { return false; }
		virtual int32 GetEntry() const { return 0; }
		virtual uint8 GetSubEntry() const { return 0; }
		WorldPosition* NearestPoint(const WorldPosition& pos) const;
		std::vector<WorldPosition*> NextPoint(const WorldPosition& pos) const;
	protected:
		void SetExpireFast() { expireDelay = 60000; } //1 minute
		void SetCooldownShort() { cooldownDelay = 1000; } //1 second
		void SetCooldownLong() { cooldownDelay = 300000; } //5 minutes
		virtual std::vector<std::string> GetTravelConditions() const { return {}; }

		float DistanceTo(WorldPosition pos) const { return NearestPoint(pos)->distance(pos); }
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

		virtual bool IsActive(Player* bot) const { return false; }

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

		virtual bool IsActive(const Player* bot) const override { return bot->IsActiveQuest(questId); }

		virtual std::string GetTitle() const override;

		virtual bool IsClassQuest() const { return questTemplate->GetRequiredClasses(); }
		virtual uint32 GetQuestId() const { return questId; }
	protected:
		virtual Quest const* GetQuestTemplate() const { return questTemplate; }
		virtual uint8 GetSubEntry() const { return subEntry; }
	private:
		uint32 questId;
		uint8 subEntry;
		Quest const* questTemplate;
	};

	//A quest giver or taker.
	class QuestRelationTravelDestination : public QuestTravelDestination
	{
	public:
		QuestRelationTravelDestination(uint32 questId, int32 entry, uint32 relation) : QuestTravelDestination(questId, entry, relation) {}

		virtual bool IsActive(const Player* bot) const override;
		virtual std::string GetTitle() const override;
	};

	//A quest objective (creature/gameobject to grind/loot)
	class QuestObjectiveTravelDestination : public QuestTravelDestination
	{
	public:
		QuestObjectiveTravelDestination(uint32 questId, int32 entry, uint32 objective) : QuestTravelDestination(questId, entry, objective) { SetExpireFast(); };

		virtual bool IsActive(const Player* bot) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with rpg target(s) based on race and level
	class RpgTravelDestination : public EntryTravelDestination
	{
	public:
		RpgTravelDestination(int32 entry) : EntryTravelDestination(entry) {}

		virtual bool IsActive(const Player* bot) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with zone exploration target(s) 
	class ExploreTravelDestination : public EntryTravelDestination
	{
	public:
		ExploreTravelDestination(int32 areaId) : EntryTravelDestination(areaId) { SetExpireFast(); SetCooldownShort(); title = sAreaStore.LookupEntry(areaId)->area_name[0]; }

		virtual bool IsActive(const Player* bot) const override;
		virtual std::string GetTitle() const override { return title; }
	protected:
		std::string title = "";
	};

	//A location with zone exploration target(s) 
	class GrindTravelDestination : public EntryTravelDestination
	{
	public:
		GrindTravelDestination(int32 entry) : EntryTravelDestination(entry) {}

		virtual bool IsActive(const Player* bot) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with a boss
	class BossTravelDestination : public EntryTravelDestination
	{
	public:
		BossTravelDestination(int32 entry) : EntryTravelDestination(entry) { SetCooldownShort(); }

		virtual bool IsActive(const Player* bot) const override;
		virtual std::string GetTitle() const override;
	};

	//A location with a object that can be gathered
	class GatherTravelDestination : public EntryTravelDestination
	{
	public:
		GatherTravelDestination(int32 entry) : EntryTravelDestination(entry) {}

		virtual bool IsActive(const Player* bot) const override;
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
		TravelTarget(PlayerbotAI* ai) : AiObject(ai) {};
		TravelTarget(PlayerbotAI* ai, TravelDestination* tDestination1, WorldPosition* wPosition1) : AiObject(ai) { setTarget(tDestination1, wPosition1); }
		~TravelTarget() = default;

		void setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1 = false);
		void setStatus(TravelStatus status);
		void setExpireIn(uint32 expireMs) { statusTime = getExpiredTime() + expireMs; }
		void incRetry(bool isMove) { if (isMove) moveRetryCount += 2; else extendRetryCount += 2; }
		void decRetry(bool isMove) { if (isMove && moveRetryCount > 0) moveRetryCount--; else if (extendRetryCount > 0) extendRetryCount--; }
		void setRetry(bool isMove, uint32 newCount = 0) { if (isMove) moveRetryCount = newCount; else extendRetryCount = newCount; }
		void setGroupCopy(bool isGroupCopy = true) { groupCopy = isGroupCopy; }
		void setForced(bool forced1) { forced = forced1; }
		void setRadius(float radius1) { radius = radius1; }

		void copyTarget(TravelTarget* target);

		float distance(Player* bot) { WorldPosition pos(bot);  return wPosition->distance(pos); };
		WorldPosition* getPosition() { return wPosition; };
		std::string GetPosStr() { return wPosition->to_string(); }
		TravelDestination* getDestination() { return tDestination; };
		int32 GetEntry() { if (!tDestination) return 0; return tDestination->GetEntry(); }
		PlayerbotAI* getAi() { return ai; }

		uint32 getExpiredTime() { return WorldTimer::getMSTime() - startTime; }
		uint32 getTimeLeft() { return statusTime - getExpiredTime(); }
		uint32 getMaxTravelTime() { return (1000.0 * distance(bot)) / bot->GetSpeed(MOVE_RUN); }
		uint32 getRetryCount(bool isMove) { return isMove ? moveRetryCount : extendRetryCount; }

		bool isTraveling();
		bool IsActive();
		bool isWorking();
		bool isPreparing();
		bool isMaxRetry(bool isMove) { return isMove ? (moveRetryCount > 10) : (extendRetryCount > 5); }
		TravelStatus getStatus() { return m_status; }

		TravelState getTravelState();

		bool isGroupCopy() { return groupCopy; }
		bool isForced() { return forced; }
	protected:
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
	typedef std::unordered_map<int32, DestinationList> DestinationMap;
	typedef std::unordered_map<std::type_index, DestinationMap> TypedDestinationMap;

	//General container for all travel destinations.
	class TravelMgr
	{
	public:
		TravelMgr() {};
		void Clear();

		void SetMobAvoidArea();
		void SetMobAvoidAreaMap(uint32 mapId);

		void LoadQuestTravelTable();

		template <class D, class W, class URBG>
		void weighted_shuffle
		(D first, D last
			, W first_weight, W last_weight
			, URBG&& g)
		{
			while (first != last && first_weight != last_weight)
			{
				std::discrete_distribution<int> dd(first_weight, last_weight);
				auto i = dd(g);

				if (i)
				{
					std::swap(*first, *std::next(first, i));
					std::swap(*first_weight, *std::next(first_weight, i));
				}
				++first;
				++first_weight;
			}
		}

		std::vector <WorldPosition*> getNextPoint(const WorldPosition& center, std::vector<WorldPosition*> points, uint32 amount = 1);
		std::vector <WorldPosition> getNextPoint(const WorldPosition& center, std::vector<WorldPosition> points, uint32 amount = 1);
		QuestStatusData* getQuestStatus(const Player* bot, uint32 questId);
		bool getObjectiveStatus(const Player* bot, Quest const* pQuest, uint32 objective);
		uint32 getDialogStatus(const Player* pPlayer, int32 questgiver, Quest const* pQuest);
		std::vector<TravelDestination*> getQuestTravelDestinations(Player* bot, int32 questId = -1, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000, bool ignoreObjectives = false);
		std::vector<TravelDestination*> getRpgTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000);
		std::vector<TravelDestination*> getExploreTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false);
		std::vector<TravelDestination*> getGrindTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000, uint32 maxCheck = 50);
		std::vector<TravelDestination*> getBossTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 25000);


		void setNullTravelTarget(Player* player);

		void addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance = 0.1f, bool makeShortcuts = true);
		void loadMapTransfers();
		float mapTransDistance(WorldPosition start, WorldPosition end, bool toMap = false);
		float fastMapTransDistance(WorldPosition start, WorldPosition end, bool toMap = false);

		NullTravelDestination* nullTravelDestination = new NullTravelDestination();
		WorldPosition* nullWorldPosition = new WorldPosition();

		void addBadVmap(uint32 mapId, int x, int y) { badVmap.push_back(std::make_tuple(mapId, x, y)); }
		void addBadMmap(uint32 mapId, int x, int y) { badMmap.push_back(std::make_tuple(mapId, x, y)); }
		bool isBadVmap(uint32 mapId, int x, int y) { return std::find(badVmap.begin(), badVmap.end(), std::make_tuple(mapId, x, y)) != badVmap.end(); }
		bool isBadMmap(uint32 mapId, int x, int y) { return std::find(badMmap.begin(), badMmap.end(), std::make_tuple(mapId, x, y)) != badMmap.end(); }


		void printGrid(uint32 mapId, int x, int y, std::string type);
		void printObj(WorldObject* obj, std::string type);

		int32 getAreaLevel(uint32 area_id);
		void loadAreaLevels();

		template<class T>
		T* AddDestination(int32 entry) {
			if (destinationMap[typeid(T)].find(entry) == destinationMap[typeid(T)].end())
				destinationMap[typeid(T)][entry].push_back(new T(entry));

			return (T*)destinationMap[typeid(T)][entry].back();
		}

		template<class T>
		T* AddQuestDestination(int32 questId, uint32 entry, uint32 subEntry) {
			for (auto& dest : destinationMap[typeid(T)][questId])
				if (dest->GetEntry() == entry && dest->GetSubEntry() == subEntry)
					return (T*)dest;

			destinationMap[typeid(T)][questId].push_back(new T(questId, entry, subEntry));

			return (T*)destinationMap[typeid(T)][questId].back();
		}


		std::vector<TravelDestination*> getExploreLocs();
	protected:
		void logQuestError(uint32 errorNr, Quest* quest, uint32 objective = 0, uint32 unitId = 0, uint32 itemId = 0);

		std::vector<uint32> avoidLoaded;

		TypedDestinationMap destinationMap;

		std::unordered_map<uint64, GuidPosition> pointsMap;
		std::unordered_map<uint32, int32> areaLevels;

		std::vector<std::tuple<uint32, int, int>> badVmap, badMmap;

		std::unordered_map<std::pair<uint32, uint32>, std::vector<mapTransfer>, boost::hash<std::pair<uint32, uint32>>> mapTransfersMap;
	};
}

#define sTravelMgr MaNGOS::Singleton<TravelMgr>::Instance()

