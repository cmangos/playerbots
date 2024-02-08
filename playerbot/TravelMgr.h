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
            : pointFrom(pointFrom1), pointTo(pointTo1), portalLength(portalLength1) {}

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
        TravelDestination(float radiusMin1, float radiusMax1) { radiusMin = radiusMin1; radiusMax = radiusMax1; }
        TravelDestination(vector<WorldPosition*> points1, float radiusMin1, float radiusMax1) { points = points1;  radiusMin = radiusMin1; radiusMax = radiusMax1; }

        void addPoint(WorldPosition* pos) { points.push_back(pos); }
        bool hasPoint(WorldPosition* pos) { return std::find(points.begin(), points.end(), pos) != points.end(); }
        void setExpireDelay(uint32 delay) { expireDelay = delay; }
        void setCooldownDelay(uint32 delay) { cooldownDelay = delay; }

        vector<WorldPosition*> getPoints(bool ignoreFull = false);
        uint32 getExpireDelay() { return expireDelay; }
        uint32 getCooldownDelay() { return cooldownDelay; }

        virtual Quest const* GetQuestTemplate() { return nullptr; }

        virtual bool isActive(Player* bot) { return false; }

        virtual string getName() { return "TravelDestination"; }
        virtual int32 getEntry() { return 0; }
        virtual string getTitle() { return "generic travel destination"; }

        WorldPosition* nearestPoint(WorldPosition* pos) {return nearestPoint(*pos);};
        WorldPosition* nearestPoint(WorldPosition pos);
        float distanceTo(WorldPosition* pos) { return nearestPoint(pos)->distance(*pos); }
        float distanceTo(WorldPosition pos) { return nearestPoint(pos)->distance(pos); }
        bool onMap(WorldPosition pos) {return nearestPoint(pos)->getMapId() == pos.getMapId();}
        virtual bool isIn(WorldPosition pos, float radius = 0) { return onMap(pos) && distanceTo(pos) <= (radius? radius : radiusMin); }
        virtual bool isOut(WorldPosition pos, float radius = 0) { return !onMap(pos) || distanceTo(pos) > (radius? radius : radiusMax); }
        float getRadiusMin() { return radiusMin; }

        vector<WorldPosition*> touchingPoints(WorldPosition* pos);
        vector<WorldPosition*> sortedPoints(WorldPosition* pos);
        vector<WorldPosition*> nextPoint(WorldPosition* pos, bool ignoreFull = true);
    protected:
        vector<WorldPosition*> points;
        float radiusMin = 0;
        float radiusMax = 0;

        uint32 expireDelay = 5 * 1000;
        uint32 cooldownDelay = 60 * 1000;
    };

    //A travel target that is always inactive and jumps to cooldown.
    class NullTravelDestination : public TravelDestination
    {
    public:
        NullTravelDestination(uint32 cooldownDelay1 = 5 * 60 * 1000) : TravelDestination() { cooldownDelay = cooldownDelay1;};

        virtual Quest const* GetQuestTemplate() { return nullptr; }

        virtual bool isActive(Player* bot) { return false; }
        
        virtual string getName() { return "NullTravelDestination"; }
        virtual string getTitle() { return "no destination"; }

        virtual bool isIn(WorldPosition* pos) { return true; }
        virtual bool isOut(WorldPosition* pos) { return false; }

    protected:
    };


    //A travel target specifically related to a quest.
    class QuestTravelDestination : public TravelDestination
    {
    public:
        QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1) { questId = questId1; questTemplate = sObjectMgr.GetQuestTemplate(questId); }

        virtual Quest const* GetQuestTemplate() { return questTemplate;  }

        virtual bool isActive(Player* bot) { return bot->IsActiveQuest(questId); }
        
        virtual string getName() { return "QuestTravelDestination"; }
        virtual int32 getEntry() { return 0; }
        virtual string getTitle();
    protected:
        uint32 questId;
        Quest const* questTemplate;
    };

    //A quest giver or taker.
    class QuestRelationTravelDestination : public QuestTravelDestination
    {
    public:
        QuestRelationTravelDestination(uint32 quest_id1, uint32 entry1, uint32 relation1, float radiusMin1, float radiusMax1) :QuestTravelDestination(quest_id1, radiusMin1, radiusMax1) { entry = entry1; relation = relation1; }

        virtual bool isActive(Player* bot);

        virtual CreatureInfo const* getCreatureInfo() { return entry > 0 ? ObjectMgr::GetCreatureTemplate(entry) : nullptr; }

        virtual string getName() { return "QuestRelationTravelDestination"; }
        virtual int32 getEntry() { return entry; }
        virtual string getTitle();
        virtual uint32 getRelation() { return relation; }
    private:
        uint32 relation;
        int32 entry;
    };

    //A quest objective (creature/gameobject to grind/loot)
    class QuestObjectiveTravelDestination : public QuestTravelDestination
    {
    public:
        QuestObjectiveTravelDestination(uint32 quest_id1, uint32 entry1, uint32 objective1, float radiusMin1, float radiusMax1) : QuestTravelDestination(quest_id1, radiusMin1, radiusMax1) {
            objective = objective1; entry = entry1;
        }

        bool isCreature() { return GetQuestTemplate()->ReqCreatureOrGOId[objective] > 0; }

        int32 ReqCreature() {
            return isCreature() ? GetQuestTemplate()->ReqCreatureOrGOId[objective] : 0;
        }
        int32 ReqGOId() {
            return !isCreature() ? abs(GetQuestTemplate()->ReqCreatureOrGOId[objective]) : 0;
        }
        uint32 ReqCount() { return GetQuestTemplate()->ReqCreatureOrGOCount[objective]; }

        virtual bool isActive(Player* bot);

        virtual string getName() { return "QuestObjectiveTravelDestination"; }

        virtual int32 getEntry() { return entry; }

        virtual string getTitle();

        virtual uint32 getObjective() { return objective; }
    private:
        uint32 objective;
        int32 entry;
    };

    //A location with rpg target(s) based on race and level
    class RpgTravelDestination : public TravelDestination
    {
    public:
        RpgTravelDestination(uint32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1) {
            entry = entry1;
        }

        virtual bool isActive(Player* bot);

        virtual CreatureInfo const* getCreatureInfo() { return ObjectMgr::GetCreatureTemplate(entry); }
        virtual GameObjectInfo const* getGoInfo() { return ObjectMgr::GetGameObjectInfo(-1*entry); }
        virtual string getName() { return "RpgTravelDestination"; }
        virtual int32 getEntry() { return entry; }
        virtual string getTitle();
    protected:
        int32 entry;
    };

    //A location with zone exploration target(s) 
    class ExploreTravelDestination : public TravelDestination
    {
    public:
        ExploreTravelDestination(uint32 areaId1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1) {
            areaId = areaId1;
        }

        virtual bool isActive(Player* bot);

        virtual string getName() { return "ExploreTravelDestination"; }
        virtual int32 getEntry() { return 0; }
        virtual string getTitle() { return title; }
        virtual void setTitle(std::string newTitle) { title = newTitle; }
        virtual uint32 getAreaId() { return areaId; }
    protected:
        uint32 areaId;
        std::string title = "";
    };

    //A location with zone exploration target(s) 
    class GrindTravelDestination : public TravelDestination
    {
    public:
        GrindTravelDestination(int32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1) {
            entry = entry1;
        }

        virtual bool isActive(Player* bot);
        virtual CreatureInfo const* getCreatureInfo() { return ObjectMgr::GetCreatureTemplate(entry); }
        virtual string getName() { return "GrindTravelDestination"; }
        virtual int32 getEntry() { return entry; }
        virtual string getTitle();
    protected:
        int32 entry;
    };

    //A location with a boss
    class BossTravelDestination : public TravelDestination
    {
    public:
        BossTravelDestination(int32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1) {
            entry = entry1; cooldownDelay = 1000;
        }

        virtual bool isActive(Player* bot);
        virtual CreatureInfo const* getCreatureInfo() { return ObjectMgr::GetCreatureTemplate(entry); }
        virtual string getName() { return "BossTravelDestination"; }
        virtual int32 getEntry() { return entry; }
        virtual string getTitle();
    protected:
        int32 entry;
    };
   
    //A quest destination container for quick lookup of all destinations related to a quest.
    struct QuestContainer
    {
        vector<QuestTravelDestination*> questGivers;
        vector<QuestTravelDestination*> questTakers;
        vector<QuestTravelDestination*> questObjectives;
    };

    //
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
        TravelTarget(PlayerbotAI* ai, TravelDestination* tDestination1, WorldPosition* wPosition1) : AiObject(ai) { setTarget(tDestination1, wPosition1);}
        ~TravelTarget() = default;
        
        void setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1 = false);
        void setStatus(TravelStatus status);
        void setExpireIn(uint32 expireMs) { statusTime = getExpiredTime() + expireMs; }
        void incRetry(bool isMove) { if (isMove) moveRetryCount+=2; else extendRetryCount+=2; }
        void decRetry(bool isMove) { if (isMove && moveRetryCount > 0) moveRetryCount--; else if (extendRetryCount > 0) extendRetryCount--; }
        void setRetry(bool isMove, uint32 newCount = 0) { if (isMove) moveRetryCount = newCount; else extendRetryCount = newCount; }
        void setGroupCopy(bool isGroupCopy = true) { groupCopy = isGroupCopy; }
        void setForced(bool forced1) { forced = forced1; }
        void setRadius(float radius1) { radius = radius1; }

        void copyTarget(TravelTarget* target);

        float distance(Player* bot) { WorldPosition pos(bot);  return wPosition->distance(pos); };
        WorldPosition* getPosition() { return wPosition; };
        string GetPosStr() { return wPosition->to_string(); }
        TravelDestination* getDestination() { return tDestination; };
        int32 getEntry() { if (!tDestination) return 0; return tDestination->getEntry(); }
        PlayerbotAI* getAi() { return ai; }

        uint32 getExpiredTime() { return WorldTimer::getMSTime() - startTime; }
        uint32 getTimeLeft() { return statusTime - getExpiredTime(); }
        uint32 getMaxTravelTime() { return (1000.0 * distance(bot)) / bot->GetSpeed(MOVE_RUN); }
        uint32 getRetryCount(bool isMove) { return isMove ? moveRetryCount: extendRetryCount; }

        bool isTraveling(); 
        bool isActive();   
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
        WorldPosition* wPosition = nullptr;
    };

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
            //Only shuffle if there is atleast 1 non-zero value.
            W check_weight = first_weight;
            while (check_weight != last_weight && !*check_weight)
                ++check_weight;

            if (check_weight == last_weight)
                return;

            while (first != last && first_weight != last_weight)
            {
                std::discrete_distribution<int> dd(first_weight, last_weight);
                auto i = dd(g);

                if (i)
                {
                    swap(*first, *std::next(first, i));
                    swap(*first_weight, *std::next(first_weight, i));
                }
                ++first;
                ++first_weight;
            }
        }

        vector <WorldPosition*> getNextPoint(WorldPosition* center, vector<WorldPosition*> points, uint32 amount = 1);
        vector <WorldPosition> getNextPoint(WorldPosition center, vector<WorldPosition> points, uint32 amount = 1);
        QuestStatusData* getQuestStatus(Player* bot, uint32 questId);
        bool getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective);
        uint32 getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest);
        vector<TravelDestination*> getQuestTravelDestinations(Player* bot, int32 questId = -1, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000, bool ignoreObjectives = false);
        vector<TravelDestination*> getRpgTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000);
        vector<TravelDestination*> getExploreTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false);
        vector<TravelDestination*> getGrindTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 5000, uint32 maxCheck = 50);
        vector<TravelDestination*> getBossTravelDestinations(Player* bot, bool ignoreFull = false, bool ignoreInactive = false, float maxDistance = 25000);


        void setNullTravelTarget(Player* player);

        void addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance = 0.1f, bool makeShortcuts = true);
        void loadMapTransfers();
        float mapTransDistance(WorldPosition start, WorldPosition end, bool toMap = false);
        float fastMapTransDistance(WorldPosition start, WorldPosition end, bool toMap = false);

        NullTravelDestination* nullTravelDestination = new NullTravelDestination();
        WorldPosition* nullWorldPosition = new WorldPosition();

        void addBadVmap(uint32 mapId, int x, int y) { badVmap.push_back(make_tuple(mapId, x, y)); }
        void addBadMmap(uint32 mapId, int x, int y) { badMmap.push_back(make_tuple(mapId, x, y)); }
        bool isBadVmap(uint32 mapId, int x, int y) { return std::find(badVmap.begin(), badVmap.end(), make_tuple(mapId, x, y)) != badVmap.end(); }
        bool isBadMmap(uint32 mapId, int x, int y) { return std::find(badMmap.begin(), badMmap.end(), make_tuple(mapId, x, y)) != badMmap.end(); }


        void printGrid(uint32 mapId, int x, int y, string type);
        void printObj(WorldObject* obj, string type);

        int32 getAreaLevel(uint32 area_id);
        void loadAreaLevels();
        std::unordered_map<uint32, QuestContainer*> getQuests() { return quests; }
        std::unordered_map<uint32, ExploreTravelDestination*> getExploreLocs() { return exploreLocs; }
    protected:
        void logQuestError(uint32 errorNr, Quest* quest, uint32 objective = 0, uint32 unitId = 0, uint32 itemId = 0);

        vector<uint32> avoidLoaded;

        vector<QuestTravelDestination*> questGivers;
        vector<RpgTravelDestination*> rpgNpcs;
        vector<GrindTravelDestination*> grindMobs;
        vector<BossTravelDestination*> bossMobs;

        std::unordered_map<uint32, ExploreTravelDestination*> exploreLocs;
        std::unordered_map<uint32, QuestContainer*> quests;
        std::unordered_map<uint64, GuidPosition> pointsMap;
        std::unordered_map<uint32, int32> areaLevels;

        vector<tuple<uint32, int, int>> badVmap, badMmap;

        std::unordered_map<pair<uint32, uint32>, vector<mapTransfer>, boost::hash<pair<uint32, uint32>>> mapTransfersMap;
    };
}

#define sTravelMgr MaNGOS::Singleton<TravelMgr>::Instance()

