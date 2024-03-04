#ifndef _RandomPlayerbotMgr_H
#define _RandomPlayerbotMgr_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include "PlayerbotMgr.h"
#include "playerbot/PlayerbotAIConfig.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

using namespace std;

class CachedEvent
{
public:
    CachedEvent() : value(0), lastChangeTime(0), validIn(0), data("") {}
    CachedEvent(const CachedEvent& other) : value(other.value), lastChangeTime(other.lastChangeTime), validIn(other.validIn), data(other.data) {}
    CachedEvent(uint32 value, uint32 lastChangeTime, uint32 validIn, string data = "") : value(value), lastChangeTime(lastChangeTime), validIn(validIn), data(data) {}

public:
    bool IsEmpty() { return !lastChangeTime; }

public:
    uint32 value, lastChangeTime, validIn;
    string data;
};

class PerformanceMonitorOperation;

//https://gist.github.com/bradley219/5373998

class botPIDImpl;
class botPID
{
public:
    // Kp -  proportional gain
    // Ki -  Integral gain
    // Kd -  derivative gain
    // dt -  loop interval time
    // max - maximum value of manipulated variable
    // min - minimum value of manipulated variable
    botPID(double dt, double max, double min, double Kp, double Ki, double Kd);
    void adjust(double Kp, double Ki, double Kd);
    void reset();
   
    double calculate(double setpoint, double pv);
    ~botPID();

private:
    botPIDImpl* pimpl;
};

class RandomPlayerbotMgr : public PlayerbotHolder
{
    public:
        RandomPlayerbotMgr();
        virtual ~RandomPlayerbotMgr();
        static RandomPlayerbotMgr& instance()
        {
            static RandomPlayerbotMgr instance;
            return instance;
        }

        virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false);
private:
        void ScaleBotActivity();
        void LogPlayerLocation();
        void DelayedFacingFix();
        void LoginFreeBots();
public:
        static void DatabasePing(std::unique_ptr<QueryResult> result, uint32 pingStart, string db);
        void SetDatabaseDelay(string db, uint32 delay) {databaseDelay[db] = delay;}
        uint32 GetDatabaseDelay(string db) {if(databaseDelay.find(db) == databaseDelay.end()) return 0; return databaseDelay[db];}

        static bool HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args);
        bool IsRandomBot(Player* bot);
        bool IsRandomBot(uint32 bot);
        bool IsFreeBot(Player* bot) { return IsRandomBot(bot) || sPlayerbotAIConfig.IsFreeAltBot(bot); }
        bool IsFreeBot(uint32 bot) { return IsRandomBot(bot) || sPlayerbotAIConfig.IsFreeAltBot(bot); }
        void InstaRandomize(Player* bot);
        void Randomize(Player* bot);
        void RandomizeFirst(Player* bot);
        void UpdateGearSpells(Player* bot);
        void ScheduleTeleport(uint32 bot, uint32 time = 0);
        void ScheduleChangeStrategy(uint32 bot, uint32 time = 0);
        void HandleCommand(uint32 type, const string& text, Player& fromPlayer, string channelName = "", Team team = TEAM_BOTH_ALLOWED, uint32 lang = LANG_UNIVERSAL);
        string HandleRemoteCommand(string request);
        void OnPlayerLogout(Player* player);
        void OnPlayerLogin(Player* player);
        void OnPlayerLoginError(uint32 bot);
        Player* GetRandomPlayer();
        PlayerBotMap& GetPlayers() { return players; };
        Player* GetPlayer(uint32 playerGuid);
        PlayerBotMap& GetAllBots() { return playerBots; };
        void PrintStats();
        double GetBuyMultiplier(Player* bot);
        double GetSellMultiplier(Player* bot);
        void AddTradeDiscount(Player* bot, Player* master, int32 value);
        void SetTradeDiscount(Player* bot, Player* master, uint32 value);
        uint32 GetTradeDiscount(Player* bot, Player* master);
        void Refresh(Player* bot);
        void RandomTeleportForLevel(Player* bot, bool activeOnly);
        void RandomTeleportForLevel(Player* bot) { return RandomTeleportForLevel(bot, true); }
        void RandomTeleportForRpg(Player* bot, bool activeOnly);
        void RandomTeleportForRpg(Player* bot) { return RandomTeleportForRpg(bot, true); }
        int GetMaxAllowedBotCount();
        bool ProcessBot(Player* player);
        void Revive(Player* player);
        void ChangeStrategy(Player* player);
        void ChangeStrategyOnce(Player* player);
        uint32 GetValue(Player* bot, string type);
        uint32 GetValue(uint32 bot, string type);
        int32 GetValueValidTime(uint32 bot, string event);
        string GetData(uint32 bot, string type);
        void SetValue(uint32 bot, string type, uint32 value, string data = "");
        void SetValue(Player* bot, string type, uint32 value, string data = "");
        void Remove(Player* bot);
        void Hotfix(Player* player, uint32 version);
        uint32 GetBattleMasterEntry(Player* bot, BattleGroundTypeId bgTypeId, bool fake = false);
        const CreatureDataPair* GetCreatureDataByEntry(uint32 entry);
        uint32 GetCreatureGuidByEntry(uint32 entry);
        void LoadBattleMastersCache();
        map<uint32, map<uint32, map<uint32, bool> > > NeedBots;
        map<uint32, map<uint32, map<uint32, uint32> > > BgBots;
        map<uint32, map<uint32, map<uint32, uint32> > > VisualBots;
        map<uint32, map<uint32, map<uint32, uint32> > > BgPlayers;
        map<uint32, map<uint32, map<uint32, map<uint32, uint32> > > > ArenaBots;
        map<uint32, map<uint32, map<uint32, uint32> > > Rating;
        map<uint32, map<uint32, map<uint32, uint32> > > Supporters;
        map<Team, vector<uint32>> LfgDungeons;
        void CheckBgQueue();
        void CheckLfgQueue();
        void CheckPlayers();
        void SaveCurTime();
        void SyncEventTimers();
        void AddOfflineGroupBots();
        static Item* CreateTempItem(uint32 item, uint32 count, Player const* player, uint32 randomPropertyId = 0);

        bool AddRandomBot(uint32 bot);

        map<Team, map<BattleGroundTypeId, list<uint32> > > getBattleMastersCache() { return BattleMastersCache; }

        float getActivityMod() { return activityMod; }
        float getActivityPercentage() { return activityMod * 100.0f; }
        void setActivityPercentage(float percentage) { activityMod = percentage / 100.0f; }

        void PrintTeleportCache();

        void AddFacingFix(uint32 mapId, ObjectGuid guid) { facingFix[mapId].push_back(make_pair(guid,time(0))); }

        bool arenaTeamsDeleted, guildsDeleted = false;

        std::mutex m_ahActionMutex;
	protected:
	    virtual void OnBotLoginInternal(Player * const bot);

    private:
        //pid values are set in constructor
        botPID pid = botPID(1, 50, -50, 0, 0, 0);
        float activityMod = 0.25;
        map<string, uint32> databaseDelay;
        uint32 GetEventValue(uint32 bot, string event);
        string GetEventData(uint32 bot, string event);
        uint32 SetEventValue(uint32 bot, string event, uint32 value, uint32 validIn, string data = "");
        list<uint32> GetBots();
        list<uint32> GetBgBots(uint32 bracket);
        time_t BgCheckTimer;
        time_t LfgCheckTimer;
        time_t PlayersCheckTimer;
        time_t EventTimeSyncTimer;
        time_t OfflineGroupBotsTimer;
        uint32 AddRandomBots();
        bool ProcessBot(uint32 bot);
        void ScheduleRandomize(uint32 bot, uint32 time);
        void RandomTeleport(Player* bot);
        void RandomTeleport(Player* bot, vector<WorldLocation> &locs, bool hearth = false, bool activeOnly = false);
        uint32 GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ);
        void PrepareTeleportCache();
        typedef void (RandomPlayerbotMgr::*ConsoleCommandHandler) (Player*);
    private:
        PlayerBotMap players;
        int processTicks;
        map<uint8, vector<WorldLocation> > locsPerLevelCache;
        map<uint32, vector<WorldLocation> > rpgLocsCache;
		map<uint32, map<uint32, vector<WorldLocation> > > rpgLocsCacheLevel;
        map<Team, map<BattleGroundTypeId, list<uint32> > > BattleMastersCache;
        map<uint32, map<string, CachedEvent> > eventCache;
        BarGoLink* loginProgressBar;
        list<uint32> currentBots;
        list<uint32> arenaTeamMembers;
        uint32 bgBotsCount;
        uint32 playersLevel = 0;
        uint32 activeBots = 0;

        std::unordered_map<uint32, std::vector<std::pair<int32,int32>>> playerBotMoveLog;
        typedef std::unordered_map <uint32, list<float>> botPerformanceMetric;
        std::unordered_map<string, botPerformanceMetric> botPerformanceMetrics;
        
        std::vector<std::pair<uint32, uint32>> RpgLocationsNear(WorldLocation pos, uint32 areaId = 0, uint32 radius = 2000);
        void PushMetric(botPerformanceMetric& metric, const uint32 bot, const float value, const uint32 maxNum = 60) const;
        float GetMetricDelta(botPerformanceMetric& metric) const;

        bool showLoginWarning;
        std::unordered_map<uint32, std::vector<pair<ObjectGuid, time_t>>> facingFix;
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif
