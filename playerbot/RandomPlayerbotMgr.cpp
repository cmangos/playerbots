#include "Config/Config.h"
#include "../botpch.h"
#include "playerbot.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotFactory.h"
#include "AccountMgr.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "PlayerbotAI.h"
#include "Player.h"
#include "AiFactory.h"
#include "PlayerbotCommandServer.h"
#include "MemoryMonitor.h"

#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "CellImpl.h"
#include "FleeManager.h"
#include "ServerFacade.h"

#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Chat/ChannelMgr.h"

#include "World/WorldState.h"

#ifndef MANGOSBOT_ZERO
#ifdef CMANGOS
#include "Arena/ArenaTeam.h"
#endif
#ifdef MANGOS
#include "ArenaTeam.h"
#endif
#endif

#include "TravelMgr.h"
#include <iomanip>
#include <float.h>

#if PLATFORM == PLATFORM_WINDOWS
#include "windows.h"
#include "psapi.h"
#endif

using namespace ai;
using namespace MaNGOS;

INSTANTIATE_SINGLETON_1(RandomPlayerbotMgr);

#ifdef CMANGOS
#include <boost/thread/thread.hpp>
#endif

#ifdef MANGOS
class PrintStatsThread: public ACE_Task <ACE_MT_SYNCH>
{
public:
    int svc(void) { sRandomPlayerbotMgr.PrintStats(); return 0; }
};
#endif
#ifdef CMANGOS
void PrintStatsThread()
{
    sRandomPlayerbotMgr.PrintStats();
}
#endif

void activatePrintStatsThread()
{
#ifdef MANGOS
    PrintStatsThread *thread = new PrintStatsThread();
    thread->activate();
#endif
#ifdef CMANGOS
    boost::thread t(PrintStatsThread);
    t.detach();
#endif
}

#ifdef MANGOS
class CheckBgQueueThread : public ACE_Task <ACE_MT_SYNCH>
{
public:
    int svc(void) { sRandomPlayerbotMgr.CheckBgQueue(); return 0; }
};
#endif
#ifdef CMANGOS
void CheckBgQueueThread()
{
    sRandomPlayerbotMgr.CheckBgQueue();
}
#endif

void activateCheckBgQueueThread()
{
#ifdef MANGOS
    CheckBgQueueThread *thread = new CheckBgQueueThread();
    thread->activate();
#endif
#ifdef CMANGOS
    boost::thread t(CheckBgQueueThread);
    t.detach();
#endif
}

#ifdef MANGOS
class CheckLfgQueueThread : public ACE_Task <ACE_MT_SYNCH>
{
public:
    int svc(void) { sRandomPlayerbotMgr.CheckLfgQueue(); return 0; }
};
#endif
#ifdef CMANGOS
void CheckLfgQueueThread()
{
    sRandomPlayerbotMgr.CheckLfgQueue();
}
#endif

void activateCheckLfgQueueThread()
{
#ifdef MANGOS
    CheckLfgQueueThread *thread = new CheckLfgQueueThread();
    thread->activate();
#endif
#ifdef CMANGOS
    boost::thread t(CheckLfgQueueThread);
    t.detach();
#endif
}

#ifdef MANGOS
class CheckPlayersThread : public ACE_Task <ACE_MT_SYNCH>
{
public:
    int svc(void) { sRandomPlayerbotMgr.CheckPlayers(); return 0; }
};
#endif
#ifdef CMANGOS
void CheckPlayersThread()
{
    sRandomPlayerbotMgr.CheckPlayers();
}
#endif

void activateCheckPlayersThread()
{
#ifdef MANGOS
    CheckPlayersThread *thread = new CheckPlayersThread();
    thread->activate();
#endif
#ifdef CMANGOS
    boost::thread t(CheckPlayersThread);
    t.detach();
#endif
}

class botPIDImpl
{
public:
    botPIDImpl(double dt, double max, double min, double Kp, double Ki, double Kd);
    ~botPIDImpl();
    double calculate(double setpoint, double pv);
    void adjust(double Kp, double Ki, double Kd) { _Kp = Kp; _Ki = Ki; _Kd = Kd; }
    void reset() { _integral = 0; }

private:
    double _dt;
    double _max;
    double _min;
    double _Kp;
    double _Ki;
    double _Kd;
    double _pre_error;
    double _integral;
};


botPID::botPID(double dt, double max, double min, double Kp, double Ki, double Kd)
{
    pimpl = new botPIDImpl(dt, max, min, Kp, Ki, Kd);
}
void botPID::adjust(double Kp, double Ki, double Kd)
{
    pimpl->adjust(Kp, Ki, Kd);
}
void botPID::reset()
{
    pimpl->reset();
}
double botPID::calculate(double setpoint, double pv)
{
    return pimpl->calculate(setpoint, pv);
}
botPID::~botPID()
{
    delete pimpl;
}


/**
 * Implementation
 */
botPIDImpl::botPIDImpl(double dt, double max, double min, double Kp, double Ki, double Kd) :
    _dt(dt),
    _max(max),
    _min(min),
    _Kp(Kp),
    _Ki(Ki),
    _Kd(Kd),
    _pre_error(0),
    _integral(0)
{
}

double botPIDImpl::calculate(double setpoint, double pv)
{

    // Calculate error
    double error = setpoint - pv;

    // Proportional term
    double Pout = _Kp * error;

    // Integral term
    _integral += error * _dt;
    double Iout = _Ki * _integral;

    // Derivative term
    double derivative = (error - _pre_error) / _dt;
    double Dout = _Kd * derivative;

    // Calculate total output
    double output = Pout + Iout + Dout;

    // Restrict to max/min
    if (output > _max)
    {
        output = _max;
        _integral -= error * _dt; //Stop integral buildup at max
    }
    else if (output < _min)
    {
        output = _min;
        _integral -= error * _dt; //Stop integral buildup at min
    }

    // Save error to previous error
    _pre_error = error;

    return output;
}

botPIDImpl::~botPIDImpl()
{
}

RandomPlayerbotMgr::RandomPlayerbotMgr() : PlayerbotHolder(), processTicks(0), loginProgressBar(NULL)
{
    if (sPlayerbotAIConfig.enabled || sPlayerbotAIConfig.randomBotAutologin)
    {
        sPlayerbotCommandServer.Start();
        PrepareTeleportCache();

        for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
        {
            for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
            {
                BgPlayers[j][i][0] = 0;
                BgPlayers[j][i][1] = 0;
                BgBots[j][i][0] = 0;
                BgBots[j][i][1] = 0;
                ArenaBots[j][i][0][0] = 0;
                ArenaBots[j][i][0][1] = 0;
                ArenaBots[j][i][1][0] = 0;
                ArenaBots[j][i][1][1] = 0;
                NeedBots[j][i][0] = false;
                NeedBots[j][i][1] = false;
            }
        }

        //1) Proportional: Amount activity is adjusted based on diff being above or below wanted diff. (100 wanted diff & 0.1 p = 150 diff = -5% activity)
        //2) Integral: Same as proportional but builds up each tick. (100 wanted diff & 0.01 i = 150 diff = -0.5% activity each tick)
        //3) Derative: Based on speed of diff. (+5 diff last tick & 0.05 d = -0.25% activity)
        pid.adjust(0.05,0.01,0.2);
        BgCheckTimer = 0;
        LfgCheckTimer = 0;
        PlayersCheckTimer = 0;
        EventTimeSyncTimer = 0;
        OfflineGroupBotsTimer = 0;
        guildsDeleted = false;
        arenaTeamsDeleted = false;

        list<uint32> availableBots = GetBots();

        for (auto& bot : availableBots)
        {
            if(GetEventValue(bot,"login"))
                SetEventValue(bot, "login", 0, 0);
        }

#ifndef MANGOSBOT_ZERO
        // load random bot team members
        auto results = CharacterDatabase.PQuery("SELECT guid FROM arena_team_member");
        if (results)
        {
            sLog.outString("Loading arena team bot members...");
            do
            {
                Field* fields = results->Fetch();
                uint32 lowguid = fields[0].GetUInt32();
                arenaTeamMembers.push_back(lowguid);
            } while (results->NextRow());
        }
#endif
        // sync event timers
        SyncEventTimers();

        for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
        {
            if (!sMapStore.LookupEntry(i))
                continue;

            uint32 mapId = sMapStore.LookupEntry(i)->MapID;
            facingFix[mapId] = {};
        }

        showLoginWarning = true;
    }
}

RandomPlayerbotMgr::~RandomPlayerbotMgr()
{
}

int RandomPlayerbotMgr::GetMaxAllowedBotCount()
{
    return GetEventValue(0, "bot_count");
}

inline ostringstream print_path(Unit* bot, vector<pair<int, int>>& log, bool is_sqDist_greater_200 = false) {
    ostringstream out;
    out << bot->GetName() << ",";
    out << std::fixed << std::setprecision(1);
    out << "\"LINESTRING(";
    if (is_sqDist_greater_200) {
        out << log.back().first << " " << log.back().second << ",";
        out << WorldPosition(bot).getDisplayX() << " " << WorldPosition(bot).getDisplayY();
    }
    else {
        for (auto& p : log) {
            out << p.first << " " << p.second << (&p == &log.back() ? "" : ",");
        }
    }
    out << ")\",";
    out << bot->GetOrientation() << ",";
    out << to_string(bot->getRace()) << ",";
    out << to_string(bot->getClass()) << ",";
    out << (is_sqDist_greater_200 ? "1" : "0");
    return out;
}

void RandomPlayerbotMgr::LogPlayerLocation()
{
    if (sPlayerbotAIConfig.hasLog("player_location.csv"))
    {
        activeBots = 0;

        try
        {
            sPlayerbotAIConfig.openLog("player_location.csv", "w");
            if (sPlayerbotAIConfig.randomBotAutologin)
                for (auto i : GetAllBots())
                {
                    Player* bot = i.second;
                    if (!bot)
                        continue;
                    ostringstream out;
                    out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                    out << "RND" << ",";
                    out << bot->GetName() << ",";
                    out << std::fixed << std::setprecision(2);
                    WorldPosition(bot).printWKT(out);
                    out << bot->GetOrientation() << ",";
                    out << to_string(bot->getRace()) << ",";
                    out << to_string(bot->getClass()) << ",";
                    out << bot->GetMapId() << ",";
                    out << bot->GetLevel() << ",";
                    out << bot->GetHealth() << ",";
                    out << bot->GetPowerPercent() << ",";
                    out << bot->GetMoney() << ",";

                    if (bot->GetPlayerbotAI())
                    {
                        out << to_string(uint8(bot->GetPlayerbotAI()->GetGrouperType())) << ",";
                        out << to_string(uint8(bot->GetPlayerbotAI()->GetGuilderType())) << ",";
                        out << (bot->GetPlayerbotAI()->AllowActivity(ALL_ACTIVITY) ? "active" : "inactive") << ",";
                        out << (bot->GetPlayerbotAI()->IsActive() ? "active" : "delay") << ",";
                        out << bot->GetPlayerbotAI()->HandleRemoteCommand("state") << ",";

                        if (bot->GetPlayerbotAI()->AllowActivity(ALL_ACTIVITY))
                            activeBots++;
                    }
                    else
                    {
                        out << 0 << "," << 0 << ",err,err,err,";
                    }

                    out << (bot->IsInCombat() ? "combat" : "safe") << ",";
                    out << (bot->IsDead() ? (bot->GetCorpse() ? "ghost" : "dead") : "alive");


                    sPlayerbotAIConfig.log("player_location.csv", out.str().c_str());

                    if (sPlayerbotAIConfig.hasLog("player_paths.csv"))
                    {
                        float sqDist = (playerBotMoveLog[i.first].empty() ? 1 : (pow(playerBotMoveLog[i.first].back().first - int32(WorldPosition(bot).getDisplayX()), 2) + pow(playerBotMoveLog[i.first].back().second - int32(WorldPosition(bot).getDisplayY()), 2)));
                        if (sqDist <= 200 * 200) {
                            playerBotMoveLog[i.first].push_back(make_pair(WorldPosition(bot).getDisplayX(), WorldPosition(bot).getDisplayY()));
                            if (playerBotMoveLog[i.first].size() > 100) {
                                sPlayerbotAIConfig.log("player_paths.csv", print_path(bot, playerBotMoveLog[i.first]).str().c_str());
                                playerBotMoveLog[i.first].clear();
                            }
                        }
                        else if (sqDist >= 200 * 200) {
                            if (playerBotMoveLog[i.first].size() > 1)
                                sPlayerbotAIConfig.log("player_paths.csv", print_path(bot, playerBotMoveLog[i.first]).str().c_str());
                            sPlayerbotAIConfig.log("player_paths.csv", print_path(bot, playerBotMoveLog[i.first], true).str().c_str());
                            playerBotMoveLog[i.first].clear();
                            playerBotMoveLog[i.first].push_back(make_pair(WorldPosition(bot).getDisplayX(), WorldPosition(bot).getDisplayY()));
                        }
                    }
                }
            for (auto i : GetPlayers())
            {
                Player* bot = i.second;
                if (!bot)
                    continue;
                ostringstream out;
                out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
                out << "PLR" << ",";
                out << bot->GetName() << ",";
                out << std::fixed << std::setprecision(2);
                WorldPosition(bot).printWKT(out);
                out << bot->GetOrientation() << ",";
                out << to_string(bot->getRace()) << ",";
                out << to_string(bot->getClass()) << ",";
                out << bot->GetMapId() << ",";
                out << bot->GetLevel() << ",";
                out << bot->GetHealth() << ",";
                out << bot->GetPowerPercent() << ",";
                out << bot->GetMoney() << ",";
                if (bot->GetPlayerbotAI())
                {
                    out << to_string(uint8(bot->GetPlayerbotAI()->GetGrouperType())) << ",";
                    out << to_string(uint8(bot->GetPlayerbotAI()->GetGuilderType())) << ",";
                    out << (bot->GetPlayerbotAI()->AllowActivity(ALL_ACTIVITY) ? "active" : "inactive") << ",";
                    out << (bot->GetPlayerbotAI()->IsActive() ? "active" : "delay") << ",";
                    out << bot->GetPlayerbotAI()->HandleRemoteCommand("state") << ",";

                    if (bot->GetPlayerbotAI()->AllowActivity(ALL_ACTIVITY))
                        activeBots++;
                }
                else
                {
                    out << 0 << "," << 0 << ",player,player,player,";
                }

                out << (bot->IsInCombat() ? "combat" : "safe") << ",";
                out << (bot->IsDead() ? (bot->GetCorpse() ? "ghost" : "dead") : "alive");

                sPlayerbotAIConfig.log("player_location.csv", out.str().c_str());

                if (sPlayerbotAIConfig.hasLog("player_paths.csv"))
                {
                    float sqDist = (playerBotMoveLog[i.first].empty() ? 1 : (pow(playerBotMoveLog[i.first].back().first - int32(WorldPosition(bot).getDisplayX()), 2) + pow(playerBotMoveLog[i.first].back().second - int32(WorldPosition(bot).getDisplayY()), 2)));
                    if (sqDist <= 200 * 200) {
                        playerBotMoveLog[i.first].push_back(make_pair(WorldPosition(bot).getDisplayX(), WorldPosition(bot).getDisplayY()));
                        if (playerBotMoveLog[i.first].size() > 100) {
                            sPlayerbotAIConfig.log("player_paths.csv", print_path(bot, playerBotMoveLog[i.first]).str().c_str());
                            playerBotMoveLog[i.first].clear();
                        }
                    }
                    else if (sqDist >= 200 * 200) {
                        if (playerBotMoveLog[i.first].size() > 1)
                            sPlayerbotAIConfig.log("player_paths.csv", print_path(bot, playerBotMoveLog[i.first]).str().c_str());
                        sPlayerbotAIConfig.log("player_paths.csv", print_path(bot, playerBotMoveLog[i.first], true).str().c_str());
                        playerBotMoveLog[i.first].clear();
                        playerBotMoveLog[i.first].push_back(make_pair(WorldPosition(bot).getDisplayX(), WorldPosition(bot).getDisplayY()));
                    }
                }
            }
        }
        catch (...)
        {
            return;
            //This is to prevent some thread-unsafeness. Crashes would happen if bots get added or removed.
            //We really don't care here. Just skip a log. Making this thread-safe is not worth the effort.
        }
    }
    else if (sPlayerbotAIConfig.hasLog("activity_pid.csv"))
    {
        activeBots = 0;

        if (sPlayerbotAIConfig.randomBotAutologin)
            for (auto i : GetAllBots())
            {
                Player* bot = i.second;
                if (bot->GetPlayerbotAI())
                    if (bot->GetPlayerbotAI()->AllowActivity(ALL_ACTIVITY))
                        activeBots++;
            }
        for (auto i : GetPlayers())
        {
            Player* bot = i.second;
            if (!bot)
                continue;
            if (bot->GetPlayerbotAI())
                if (bot->GetPlayerbotAI()->AllowActivity(ALL_ACTIVITY))
                    activeBots++;
        }
    }
}

void RandomPlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool minimal)
{
#ifdef MEMORY_MONITOR
    sMemoryMonitor.Print();
    sMemoryMonitor.LogCount(sConfig.GetStringDefault("LogsDir") + "/" + "memory.csv");
#endif

    if (!sPlayerbotAIConfig.randomBotAutologin || !sPlayerbotAIConfig.enabled)
        return;

    ScaleBotActivity();

    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (!maxAllowedBotCount || ((uint32)maxAllowedBotCount < sPlayerbotAIConfig.minRandomBots || (uint32)maxAllowedBotCount > sPlayerbotAIConfig.maxRandomBots))
    {
        maxAllowedBotCount = urand(sPlayerbotAIConfig.minRandomBots, sPlayerbotAIConfig.maxRandomBots);
        SetEventValue(0, "bot_count", maxAllowedBotCount,
            urand(sPlayerbotAIConfig.randomBotCountChangeMinInterval, sPlayerbotAIConfig.randomBotCountChangeMaxInterval));
    }

    list<uint32> availableBots = GetBots();    
    uint32 availableBotCount = availableBots.size();
    uint32 onlineBotCount = playerBots.size();
    
    SetAIInternalUpdateDelay(sPlayerbotAIConfig.randomBotUpdateInterval * 1000);

    PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_TOTAL,
        onlineBotCount < maxAllowedBotCount ? "RandomPlayerbotMgr::Login" : "RandomPlayerbotMgr::UpdateAIInternal");

    if (time(nullptr) > (EventTimeSyncTimer + 30))
        SaveCurTime();

    if (availableBotCount < maxAllowedBotCount && !sWorld.IsShutdowning())
    {
        bool logInAllowed = true;
        if (sPlayerbotAIConfig.randomBotLoginWithPlayer)
        {
            logInAllowed = !players.empty();
        }

        if (logInAllowed)
        {
            AddRandomBots();
        }
    }

    if (sPlayerbotAIConfig.syncLevelWithPlayers && players.size())
    {
        if (time(nullptr) > (PlayersCheckTimer + 60))
            CheckPlayers();
    }

    if (sPlayerbotAIConfig.randomBotJoinLfg && players.size())
    {
        if (time(nullptr) > (LfgCheckTimer + 30))
            CheckLfgQueue();
    }

    if (sPlayerbotAIConfig.randomBotJoinBG/* && players.size()*/)
    {
        if (time(nullptr) > (BgCheckTimer + 30))
            CheckBgQueue();
    }

    if (time(nullptr) > (OfflineGroupBotsTimer + 5) && players.size())
        AddOfflineGroupBots();

    uint32 updateBots = sPlayerbotAIConfig.randomBotsPerInterval;
    uint32 maxNewBots = sPlayerbotAIConfig.randomBotsMaxLoginsPerInterval;
    if (onlineBotCount < sPlayerbotAIConfig.minRandomBots * sPlayerbotAIConfig.loginBoostPercentage / 100)
        maxNewBots *= 2;

    uint32 loginBots = 0;

    if (!availableBots.empty())
    {
        //Update bots
        for (auto bot : availableBots)
        {
            if (!GetPlayerBot(bot))
                continue;

            if (ProcessBot(bot))
                updateBots--;

            if (!updateBots)
                break;
        }

        //Log in bots
        if (sRandomPlayerbotMgr.GetDatabaseDelay("CharacterDatabase") < 10 * IN_MILLISECONDS)
        {
            for (auto bot : availableBots)
            {
                if (GetPlayerBot(bot))
                    continue;   

                if (GetEventValue(bot, "login"))
                    onlineBotCount++;

                if (onlineBotCount + loginBots > maxAllowedBotCount)
                    break;

                if (ProcessBot(bot)) {
                    loginBots++;
                }

                if (loginBots > maxNewBots)
                    break;
            }
        }
    }

    if (pmo) pmo->finish();

    LoginFreeBots();

    //sLog.outString("[char %d, bot %d]", CharacterDatabase.m_threadBody->m_sqlQueue.size(), CharacterDatabase.m_threadBody->m_sqlQueue.size());
   
    LogPlayerLocation();

    DelayedFacingFix();

    //Ping character database.
    //CharacterDatabase.AsyncPQuery(&RandomPlayerbotMgr::DatabasePing, sWorld.GetCurrentMSTime(), string("CharacterDatabase"), "select 1 from dual");
}

void RandomPlayerbotMgr::ScaleBotActivity()
{
    float activityPercentage = getActivityPercentage();

    //if (activityPercentage >= 100.0f || activityPercentage <= 0.0f) pid.reset(); //Stop integer buildup during max/min activity

    //    % increase/decrease                   wanted diff                                         , avg diff
    float activityPercentageMod = pid.calculate(sRandomPlayerbotMgr.GetPlayers().empty() ? sPlayerbotAIConfig.diffEmpty : sPlayerbotAIConfig.diffWithPlayer, sWorld.GetAverageDiff());

    activityPercentage = activityPercentageMod + 50;

    //Cap the percentage between 0 and 100.
    activityPercentage = std::max(0.0f, std::min(100.0f, activityPercentage));

    setActivityPercentage(activityPercentage);

    if (sPlayerbotAIConfig.hasLog("activity_pid.csv"))
    {
        double virtualMemUsedByMe = 0;
#if PLATFORM == PLATFORM_WINDOWS
        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        virtualMemUsedByMe = pmc.PrivateUsage;
#endif

        ostringstream out;
        out << sWorld.GetCurrentMSTime() << ", ";

        out << sWorld.GetCurrentDiff() << ",";
        out << sWorld.GetAverageDiff() << ",";
        out << sWorld.GetMaxDiff() << ",";
        out << virtualMemUsedByMe << ",";
        out << activityPercentage << ",";
        out << activityPercentageMod << ",";
        out << activeBots << ",";
        out << playerBots.size() << ",";

        float level = 0, gold = 0, gearscore = 0;

        if (sPlayerbotAIConfig.randomBotAutologin)
            for (auto i : GetAllBots())
            {
                Player* bot = i.second;
                if (!bot)
                    continue;

                if (!bot->GetPlayerbotAI()->AllowActivity())
                    continue;

                string bracket = "level:" + to_string(bot->GetLevel() / 10);

                float level = ((float)bot->GetLevel() + (bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP) ? ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)) : 0));
                float gold = bot->GetMoney() / 10000;
                float gearscore = bot->GetPlayerbotAI()->GetEquipGearScore(bot, false, false);

                PushMetric(botPerformanceMetrics[bracket], i.first, level);
                PushMetric(botPerformanceMetrics["gold"], i.first, gold);
                PushMetric(botPerformanceMetrics["gearscore"], i.first, gearscore);
            }

        out << std::fixed << std::setprecision(4);

        for (uint8 i = 0; i < (DEFAULT_MAX_LEVEL / 10) + 1; i++)
            out << GetMetricDelta(botPerformanceMetrics["level:" + to_string(i)]) * 12 * 60 << ",";

        out << GetMetricDelta(botPerformanceMetrics["gold"]) * 12 * 60 << ",";
        out << GetMetricDelta(botPerformanceMetrics["gearscore"]) * 12 * 60 << ",";
        //out << CharacterDatabase.m_threadBody->m_sqlQueue.size();

        sPlayerbotAIConfig.log("activity_pid.csv", out.str().c_str());
    }
}

void RandomPlayerbotMgr::LoginFreeBots()
{

    if (!sPlayerbotAIConfig.freeAltBots.empty())
    {
        for (auto bot : sPlayerbotAIConfig.freeAltBots)
        {
            Player* player = GetPlayerBot(bot.second);

            PlayerbotAI* ai = player ? player->GetPlayerbotAI() : NULL;

            if (!player)
            {
                sLog.outDetail("Add player %d", bot.second);
                AddPlayerBot(bot.second, bot.first);
            }
        }
    }
}

void RandomPlayerbotMgr::DelayedFacingFix()
{
    if (!sPlayerbotAIConfig.turnInRpg)
        return;

    for (auto& fMap : facingFix) {
        for (auto obj : fMap.second) {
            if (time(0) - obj.second > 5)
            {
                if (!obj.first.IsCreature())
                    continue;

                GuidPosition guidP(obj.first, WorldPosition(fMap.first, 0, 0, 0));

                Creature* unit = guidP.GetCreature();

                if (!unit)
                    continue;

                CreatureData* data = guidP.GetCreatureData();

                if (!data)
                    continue;

                if (unit->GetOrientation() == data->orientation)
                    continue;

                unit->SetFacingTo(data->orientation);
            }
        }
        facingFix[fMap.first].clear();
    }
}

void RandomPlayerbotMgr::DatabasePing(std::unique_ptr<QueryResult> result, uint32 pingStart, string db)
{
    sRandomPlayerbotMgr.SetDatabaseDelay(db, sWorld.GetCurrentMSTime() - pingStart);
}

uint32 RandomPlayerbotMgr::AddRandomBots()
{
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");    
    uint32 currentAllowedBotCount = maxAllowedBotCount;

    uint32 maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);
    float currentAvgLevel = 0, wantedAvgLevel = 0, randomAvgLevel = 0;

    if (!playersLevel)
        playersLevel = sPlayerbotAIConfig.syncLevelNoPlayer;
  
    if (currentBots.size() < currentAllowedBotCount)
    {
        if (sPlayerbotAIConfig.syncLevelWithPlayers)
        {
            maxLevel = max(sPlayerbotAIConfig.randomBotMinLevel, min(playersLevel + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));

            wantedAvgLevel = maxLevel / 2;
            for (auto bot : playerBots)
                currentAvgLevel += bot.second->GetLevel();

            if(currentAvgLevel)
                currentAvgLevel = currentAvgLevel / playerBots.size();

            randomAvgLevel = (sPlayerbotAIConfig.randomBotMinLevel + max(sPlayerbotAIConfig.randomBotMinLevel, min(playersLevel+ sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)))) / 2;
        }

        currentAllowedBotCount -= currentBots.size();

        int32 neededAddBots = currentAllowedBotCount;

        currentAllowedBotCount = currentAllowedBotCount*2;      

        CharacterDatabase.AllowAsyncTransactions();
        CharacterDatabase.BeginTransaction();

        bool enoughBotsForCriteria = true;

        for (uint32 noCriteria = 0; noCriteria < 3; noCriteria++)
        {
            int32  classRaceAllowed[MAX_CLASSES][MAX_RACES] = { 0 };

            for (uint32 race = 1; race < MAX_RACES; ++race)
            {
                for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
                {
                    if (sPlayerbotAIConfig.classRaceProbability[cls][race])
                        classRaceAllowed[cls][race] = ((sPlayerbotAIConfig.classRaceProbability[cls][race] * maxAllowedBotCount / sPlayerbotAIConfig.classRaceProbabilityTotal) + 1) * (noCriteria+1);
                }
            }

            for (list<uint32>::iterator i = sPlayerbotAIConfig.randomBotAccounts.begin(); i != sPlayerbotAIConfig.randomBotAccounts.end(); i++)
            {
                uint32 accountId = *i;

                std::unique_ptr<QueryResult> result;

                if (noCriteria == 2)
                {
                    result = CharacterDatabase.PQuery("SELECT guid, level, totaltime, race, class FROM characters WHERE account = '%u'", accountId);
                }
                else
                {
                    bool needToIncrease = wantedAvgLevel && currentAvgLevel + 1 < wantedAvgLevel;
                    bool needToLower = wantedAvgLevel && currentAvgLevel > wantedAvgLevel + 1;
                    bool rndCanIncrease = !sPlayerbotAIConfig.disableRandomLevels && randomAvgLevel > currentAvgLevel;
                    bool rndCanLower = !sPlayerbotAIConfig.disableRandomLevels && randomAvgLevel < currentAvgLevel;

                    string query = "SELECT guid, level, totaltime, race, class FROM characters WHERE account = '%u' and level <= %u";
                    string wasRand = sPlayerbotAIConfig.instantRandomize ? "totaltime" : "(level > 1)";

                    if (needToIncrease) //We need more higher level bots.
                    {
                        query += " and (level > %u";
                        if (rndCanIncrease) //Log in higher level bots or bots that will be randomized.
                            query += " or !" + wasRand;
                        query += ")";

                        result = CharacterDatabase.PQuery(query.c_str(), accountId, maxLevel, (uint32)wantedAvgLevel);
                    }
                    else
                    {
                        if (needToLower && !rndCanLower) //Do not load unrandomized if it'll only increase level.
                            query += " and " + wasRand;

                        result = CharacterDatabase.PQuery(query.c_str(), accountId, maxLevel);
                    }
                }

                if (!result)
                    continue;

                do
                {
                    Field* fields = result->Fetch();
                    uint32 guid = fields[0].GetUInt32();
                    uint32 level = fields[1].GetUInt32();
                    uint32 totaltime = fields[2].GetUInt32();
                    uint32 race = fields[3].GetUInt32();
                    uint32 cls = fields[4].GetUInt32();

                    if (GetEventValue(guid, "add"))
                    {
                        if (!noCriteria)
                            classRaceAllowed[cls][race]--;
                        continue;
                    }

                    if (GetEventValue(guid, "logout"))
                        continue;

                    if (GetPlayerBot(guid))
                    {
                        if (!noCriteria)
                            classRaceAllowed[cls][race]--;
                        continue;
                    }

                    if (std::find(currentBots.begin(), currentBots.end(), guid) != currentBots.end())
                    {
                        if (!noCriteria)
                            classRaceAllowed[cls][race]--;
                        continue;
                    }

                    if (classRaceAllowed[cls][race] <= 0)
                        continue;

                    SetEventValue(guid, "add", 1, urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));
                    SetEventValue(guid, "logout", 0, 0);
                    currentBots.push_back(guid);

                    if(!noCriteria)
                        classRaceAllowed[cls][race]--;

                    if (wantedAvgLevel)
                    {
                        if (sPlayerbotAIConfig.instantRandomize ? totaltime : level > 1)
                            currentAvgLevel += (float)level / currentBots.size();
                        else
                            currentAvgLevel += (float)level + randomAvgLevel; //Use predicted randomized level. This will be wrong but avarage out correct.
                    }

                    currentAllowedBotCount--;
                    neededAddBots--;

                    if (!currentAllowedBotCount)
                        break;

                } while (result->NextRow());

                if (!currentAllowedBotCount)
                    break;
            }

            if (!currentAllowedBotCount)
                break;

            if (showLoginWarning && neededAddBots > 0)
            {
                sLog.outError("Not enough accounts to meet selection criteria. A random selection of bots was activated to fill the server.");

                if (sPlayerbotAIConfig.syncLevelWithPlayers)
                    sLog.outError("Only bots between level %d and %d are selected to sync with player level", uint32((currentAvgLevel + 1 < wantedAvgLevel) ? wantedAvgLevel : 1), maxLevel);

                ChatHelper chat(nullptr);

                for (uint32 race = 1; race < MAX_RACES; ++race)
                {
                    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
                    {
                        int32 moreWanted = classRaceAllowed[cls][race];
                        if (moreWanted > 0)
                        {
                            int32 totalWanted = ((sPlayerbotAIConfig.classRaceProbability[cls][race] * maxAllowedBotCount / sPlayerbotAIConfig.classRaceProbabilityTotal) + 1);
                            float percentage = float(sPlayerbotAIConfig.classRaceProbability[cls][race]) * 100.0f / sPlayerbotAIConfig.classRaceProbabilityTotal;
                            sLog.outError("%d %s %ss needed to get %3.2f%% of total but only %d found.", totalWanted, chat.formatRace(race).c_str(), chat.formatClass(cls).c_str(), percentage, totalWanted - moreWanted);
                        }
                    }
                }

                showLoginWarning = false;
            }
        }

        CharacterDatabase.CommitTransaction();

        if (currentAllowedBotCount)
            currentAllowedBotCount = GetEventValue(0, "bot_count") - currentBots.size();          

        if(currentAllowedBotCount)
#ifdef MANGOSBOT_TWO
            sLog.outError("Not enough random bot accounts available. Need %d more!!", (uint32)ceil(currentAllowedBotCount / 10));
#else
            sLog.outError("Not enough random bot accounts available. Need %d more!!", (uint32)ceil(currentAllowedBotCount / 9));
#endif
      
    }

    return currentBots.size();
}

void RandomPlayerbotMgr::LoadBattleMastersCache()
{
    BattleMastersCache.clear();

    sLog.outString("---------------------------------------");
    sLog.outString("          Loading BattleMasters Cache  ");
    sLog.outString("---------------------------------------");
    sLog.outString();

    auto result = WorldDatabase.Query("SELECT `entry`,`bg_template` FROM `battlemaster_entry`");

    uint32 count = 0;

    if (!result)
    {
        sLog.outString(">> Loaded 0 battlemaster entries - table is empty!");
        sLog.outString();
        return;
    }

    do
    {
        ++count;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        uint32 bgTypeId = fields[1].GetUInt32();

        CreatureInfo const* bmaster = sObjectMgr.GetCreatureTemplate(entry);
        if (!bmaster)
            continue;

#ifdef MANGOS
        FactionTemplateEntry const* bmFaction = sFactionTemplateStore.LookupEntry(bmaster->FactionAlliance);
#endif
#ifdef CMANGOS
        FactionTemplateEntry const* bmFaction = sFactionTemplateStore.LookupEntry(bmaster->Faction);
#endif
        uint32 bmFactionId = bmFaction->faction;
#ifdef MANGOS
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry(bmFactionId);
#endif
#ifdef CMANGOS
#ifdef MANGOSBOT_ONE
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry<FactionEntry>(bmFactionId);
#else
        FactionEntry const* bmParentFaction = sFactionStore.LookupEntry(bmFactionId);
#endif
#endif
        uint32 bmParentTeam = bmParentFaction->team;
        Team bmTeam = TEAM_BOTH_ALLOWED;
        if (bmParentTeam == 891)
            bmTeam = ALLIANCE;
        if (bmFactionId == 189)
            bmTeam = ALLIANCE;
        if (bmParentTeam == 892)
            bmTeam = HORDE;
        if (bmFactionId == 66)
            bmTeam = HORDE;

        BattleMastersCache[bmTeam][BattleGroundTypeId(bgTypeId)].insert(BattleMastersCache[bmTeam][BattleGroundTypeId(bgTypeId)].end(), entry);
        sLog.outDetail("Cached Battmemaster #%d for BG Type %d (%s)", entry, bgTypeId, bmTeam == ALLIANCE ? "Alliance" : bmTeam == HORDE ? "Horde" : "Neutral");

    } while (result->NextRow());

    sLog.outString(">> Loaded %u battlemaster entries", count);
    sLog.outString();
}

void RandomPlayerbotMgr::CheckBgQueue()
{
    if (!BgCheckTimer)
        BgCheckTimer = time(nullptr);

    if (time(nullptr) < (BgCheckTimer + 30))
    {
        return;
    }
    else
    {
        BgCheckTimer = time(nullptr);
    }

    sLog.outBasic("Checking BG Queue...");

    for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BgPlayers[j][i][0] = 0;
            BgPlayers[j][i][1] = 0;
            BgBots[j][i][0] = 0;
            BgBots[j][i][1] = 0;
            ArenaBots[j][i][0][0] = 0;
            ArenaBots[j][i][0][1] = 0;
            ArenaBots[j][i][1][0] = 0;
            ArenaBots[j][i][1][1] = 0;
            NeedBots[j][i][0] = false;
            NeedBots[j][i][1] = false;
        }
    }

    for (auto i : players)
    {
        Player* player = i.second;

        if (!player || !player->IsInWorld())
            continue;

        if (!player->InBattleGroundQueue())
            continue;

        if (player->InBattleGround() && player->GetBattleGround()->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        for (int i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            BattleGroundQueueTypeId queueTypeId = player->GetBattleGroundQueueTypeId(i);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            uint32 TeamId = player->GetTeam() == ALLIANCE ? 0 : 1;

            BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
#ifdef MANGOS
            BattleGroundBracketId bracketId = player->GetBattleGroundBracketIdFromLevel(bgTypeId);
#endif
#ifdef CMANGOS
#ifdef MANGOSBOT_TWO
            BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, player->GetLevel());
            if (!pvpDiff)
                continue;

            BattleGroundBracketId bracketId = pvpDiff->GetBracketId();
#else
            BattleGroundBracketId bracketId = player->GetBattleGroundBracketIdFromLevel(bgTypeId);
#endif
#endif
#ifndef MANGOSBOT_ZERO
            if (ArenaType arenaType = sServerFacade.BgArenaType(queueTypeId))
            {
#ifdef MANGOS
                BattleGroundQueue& bgQueue = sServerFacade.bgQueue(queueTypeId);
                GroupQueueInfo ginfo;
                uint32 tempT = TeamId;

                if (bgQueue.GetPlayerGroupInfoData(player->GetObjectGuid(), &ginfo))
                {
                    if (ginfo.IsRated)
                    {
                        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                        {
                            uint32 arena_team_id = player->GetArenaTeamId(arena_slot);
                            ArenaTeam* arenateam = sObjectMgr.GetArenaTeamById(arena_team_id);
                            if (!arenateam)
                                continue;
                            if (arenateam->GetType() != arenaType)
                                continue;

                            Rating[queueTypeId][bracketId][1] = arenateam->GetRating();
                        }
                    }
                    TeamId = ginfo.IsRated ? 1 : 0;
                }
                if (player->InArena())
                {
                    if (player->GetBattleGround()->isRated() && (ginfo.IsRated && ginfo.ArenaTeamId && ginfo.ArenaTeamRating && ginfo.OpponentsTeamRating))
                        TeamId = 1;
                    else
                        TeamId = 0;
        }
#endif
#ifdef CMANGOS
                BattleGroundQueue& bgQueue = sServerFacade.bgQueue(queueTypeId);
                GroupQueueInfo ginfo;
                uint32 tempT = TeamId;

                if (bgQueue.GetPlayerGroupInfoData(player->GetObjectGuid(), &ginfo))
                {
                    if (ginfo.isRated)
                    {
                        for (uint32 arena_slot = 0; arena_slot < MAX_ARENA_SLOT; ++arena_slot)
                        {
                            uint32 arena_team_id = player->GetArenaTeamId(arena_slot);
                            ArenaTeam* arenateam = sObjectMgr.GetArenaTeamById(arena_team_id);
                            if (!arenateam)
                                continue;
                            if (arenateam->GetType() != arenaType)
                                continue;

                            Rating[queueTypeId][bracketId][1] = arenateam->GetRating();
                        }
                    }
                    TeamId = ginfo.isRated ? 1 : 0;
                }
                if (player->InArena())
                {
                    if (player->GetBattleGround()->IsRated()/* && (ginfo.isRated && ginfo.arenaTeamId && ginfo.arenaTeamRating && ginfo.opponentsTeamRating)*/)
                        TeamId = 1;
                    else
                        TeamId = 0;
                }
#endif
                ArenaBots[queueTypeId][bracketId][TeamId][tempT]++;
    }
#endif

            if (player->GetPlayerbotAI())
                BgBots[queueTypeId][bracketId][TeamId]++;
            else
                BgPlayers[queueTypeId][bracketId][TeamId]++;

            if (!player->IsInvitedForBattleGroundQueueType(queueTypeId) && (!player->InBattleGround() || player->GetBattleGround()->GetTypeId() != sServerFacade.BgTemplateId(queueTypeId)))
            {
#ifndef MANGOSBOT_ZERO
                if (ArenaType arenaType = sServerFacade.BgArenaType(queueTypeId))
                {
                    NeedBots[queueTypeId][bracketId][TeamId] = true;
                }
                else
                {
                    NeedBots[queueTypeId][bracketId][0] = true;
                    NeedBots[queueTypeId][bracketId][1] = true;
                }
#else
                NeedBots[queueTypeId][bracketId][0] = true;
                NeedBots[queueTypeId][bracketId][1] = true;
#endif
            }
        }
    }

    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;

        if (!bot || !bot->IsInWorld())
            continue;

        if (!bot->InBattleGroundQueue())
            continue;

        if (!IsFreeBot(bot))
            continue;

        if (bot->InBattleGround() && bot->GetBattleGround()->GetStatus() == STATUS_WAIT_LEAVE)
            continue;

        for (int i = 0; i < PLAYER_MAX_BATTLEGROUND_QUEUES; ++i)
        {
            BattleGroundQueueTypeId queueTypeId = bot->GetBattleGroundQueueTypeId(i);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            uint32 TeamId = bot->GetTeam() == ALLIANCE ? 0 : 1;

            BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
#ifdef MANGOS
            BattleGroundBracketId bracketId = bot->GetBattleGroundBracketIdFromLevel(bgTypeId);
#endif
#ifdef CMANGOS
#ifdef MANGOSBOT_TWO
            BattleGround* bg = sBattleGroundMgr.GetBattleGroundTemplate(bgTypeId);
            uint32 mapId = bg->GetMapId();
            PvPDifficultyEntry const* pvpDiff = GetBattlegroundBracketByLevel(mapId, bot->GetLevel());
            if (!pvpDiff)
                continue;

            BattleGroundBracketId bracketId = pvpDiff->GetBracketId();
#else
            BattleGroundBracketId bracketId = bot->GetBattleGroundBracketIdFromLevel(bgTypeId);
#endif
#endif

#ifndef MANGOSBOT_ZERO
            ArenaType arenaType = sServerFacade.BgArenaType(queueTypeId);
            if (arenaType != ARENA_TYPE_NONE)
            {
                BattleGroundQueue& bgQueue = sServerFacade.bgQueue(queueTypeId);
                GroupQueueInfo ginfo;
                uint32 tempT = TeamId;
#ifdef MANGOS
                if (bgQueue.GetPlayerGroupInfoData(bot->GetObjectGuid(), &ginfo))
                {
                    TeamId = ginfo.IsRated ? 1 : 0;
                }
                if (bot->InArena())
                {
                    if (bot->GetBattleGround()->isRated() && (ginfo.IsRated && ginfo.ArenaTeamId && ginfo.ArenaTeamRating && ginfo.OpponentsTeamRating))
                        TeamId = 1;
                    else
                        TeamId = 0;
                }
#endif
#ifdef CMANGOS
                if (bgQueue.GetPlayerGroupInfoData(bot->GetObjectGuid(), &ginfo))
                {
                    TeamId = ginfo.isRated ? 1 : 0;
                }
                if (bot->InArena())
                {
                    if (bot->GetBattleGround()->IsRated()/* && (ginfo.isRated && ginfo.arenaTeamId && ginfo.arenaTeamRating && ginfo.opponentsTeamRating)*/)
                        TeamId = 1;
                    else
                        TeamId = 0;
                }
#endif
                ArenaBots[queueTypeId][bracketId][TeamId][tempT]++;
            }
#endif
            BgBots[queueTypeId][bracketId][TeamId]++;
        }
    }

    for (int i = BG_BRACKET_ID_FIRST; i < MAX_BATTLEGROUND_BRACKETS; ++i)
    {
        for (int j = BATTLEGROUND_QUEUE_AV; j < MAX_BATTLEGROUND_QUEUE_TYPES; ++j)
        {
            BattleGroundQueueTypeId queueTypeId = BattleGroundQueueTypeId(j);

            if ((BgPlayers[j][i][0] + BgBots[j][i][0] + BgPlayers[j][i][1] + BgBots[j][i][1]) == 0)
                continue;

#ifndef MANGOSBOT_ZERO
            if (ArenaType type = sServerFacade.BgArenaType(queueTypeId))
            {
                sLog.outBasic("ARENA:%s %s: Plr (Skirmish:%d, Rated:%d) Bot (Skirmish:%d, Rated:%d) Total (Skirmish:%d Rated:%d)",
                    type == ARENA_TYPE_2v2 ? "2v2" : type == ARENA_TYPE_3v3 ? "3v3" : "5v5",
                    i == 0 ? "10-19" : i == 1 ? "20-29" : i == 2 ? "30-39" : i == 3 ? "40-49" : i == 4 ? "50-59" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 6) ? "60" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 7) ? "60-69" : i == 6 ? (i == 6 && MAX_BATTLEGROUND_BRACKETS == 16) ? "70-79" : "70" : "80",
                    BgPlayers[j][i][0],
                    BgPlayers[j][i][1],
                    BgBots[j][i][0],
                    BgBots[j][i][1],
                    BgPlayers[j][i][0] + BgBots[j][i][0],
                    BgPlayers[j][i][1] + BgBots[j][i][1]
                );
                continue;
            }
#endif
            BattleGroundTypeId bgTypeId = sServerFacade.BgTemplateId(queueTypeId);
            string _bgType;
            switch (bgTypeId)
            {
            case BATTLEGROUND_AV:
                _bgType = "AV";
                break;
            case BATTLEGROUND_WS:
                _bgType = "WSG";
                break;
            case BATTLEGROUND_AB:
                _bgType = "AB";
                break;
#ifndef MANGOSBOT_ZERO
            case BATTLEGROUND_EY:
                _bgType = "EotS";
                break;
#endif
#ifdef MANGOSBOT_TWO
            case BATTLEGROUND_RB:
                _bgType = "Random";
                break;
            case BATTLEGROUND_SA:
                _bgType = "SotA";
                break;
            case BATTLEGROUND_IC:
                _bgType = "IoC";
                break;
#endif
            default:
                _bgType = "Other";
                break;
            }
            sLog.outBasic("BG:%s %s: Plr (%d:%d) Bot (%d:%d) Total (A:%d H:%d)",
                _bgType.c_str(),
                i == 0 ? "10-19" : i == 1 ? "20-29" : i == 2 ? "30-39" : i == 3 ? "40-49" : i == 4 ? "50-59" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 6) ? "60" : (i == 5 && MAX_BATTLEGROUND_BRACKETS == 7) ? "60-69" : i == 6 ? (i == 6 && MAX_BATTLEGROUND_BRACKETS == 16) ? "70-79" : "70" : "80",
                BgPlayers[j][i][0],
                BgPlayers[j][i][1],
                BgBots[j][i][0],
                BgBots[j][i][1],
                BgPlayers[j][i][0] + BgBots[j][i][0],
                BgPlayers[j][i][1] + BgBots[j][i][1]
            );
        }
    }

    sLog.outBasic("BG Queue check finished");
    return;
}

void RandomPlayerbotMgr::CheckLfgQueue()
{
    if (!LfgCheckTimer || time(NULL) > (LfgCheckTimer + 30))
        LfgCheckTimer = time(NULL);

    sLog.outBasic("Checking LFG Queue...");

    // Clear LFG list
    LfgDungeons[HORDE].clear();
    LfgDungeons[ALLIANCE].clear();

    for (auto i : players)
    {
        Player* player = i.second;

        if (!player || !player->IsInWorld())
            continue;

        bool isLFG = false;

#ifdef MANGOSBOT_ZERO
        WorldSafeLocsEntry const* ClosestGrave = nullptr;
        ClosestGrave = player->GetMap()->GetGraveyardManager().GetClosestGraveYard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam());
        uint32 zoneId = 0;
        if (ClosestGrave)
            zoneId = ClosestGrave->ID;

        Group* group = player->GetGroup();
        if (group)
        {
            if (sWorld.GetLFGQueue().IsGroupInQueue(group->GetId()))
            {
                isLFG = true;
                LFGGroupQueueInfo lfgInfo;
                sWorld.GetLFGQueue().GetGroupQueueInfo(&lfgInfo, group->GetId());
                uint32 lfgType = (zoneId << 16) | lfgInfo.areaId;
                LfgDungeons[player->GetTeam()].push_back(lfgType);
            }
        }
        else
        {
            if (sWorld.GetLFGQueue().IsPlayerInQueue(player->GetObjectGuid()))
            {
                isLFG = true;
                LFGPlayerQueueInfo lfgInfo;
                sWorld.GetLFGQueue().GetPlayerQueueInfo(&lfgInfo, player->GetObjectGuid());
                uint32 lfgType = (zoneId << 16) | lfgInfo.areaId;
                LfgDungeons[player->GetTeam()].push_back(lfgType);
            }
        }
#endif
#ifdef MANGOSBOT_ONE
        WorldSafeLocsEntry const* ClosestGrave = nullptr;
        ClosestGrave = player->GetMap()->GetGraveyardManager().GetClosestGraveYard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam());
        uint32 zoneId = 0;
        if (ClosestGrave)
            zoneId = ClosestGrave->ID;

        Group* group = player->GetGroup();
        if (group && !group->IsFull())
        {
            if (group->IsLeader(player->GetObjectGuid()))
            {
                if (!player->m_lookingForGroup.more.empty() && player->GetSession()->LookingForGroup_auto_add && player->m_lookingForGroup.more.isAuto())
                {
                    uint32 lfgType = (zoneId << 16) | ((1 << 8) | uint8(player->m_lookingForGroup.more.entry));
                    LfgDungeons[player->GetTeam()].push_back(lfgType);
                    isLFG = true;
                }
            }
        }
        else if (!group)
        {
            for (int i = 0; i < MAX_LOOKING_FOR_GROUP_SLOT; ++i)
                if (!player->m_lookingForGroup.group[i].empty() && player->GetSession()->LookingForGroup_auto_join && player->m_lookingForGroup.group[i].isAuto())
                {
                    isLFG = true;
                    uint32 lfgType = (zoneId << 16) | ((0 << 8) | uint8(player->m_lookingForGroup.group[i].entry));
                    LfgDungeons[player->GetTeam()].push_back(lfgType);
                }

            if (!player->m_lookingForGroup.more.empty() && player->GetSession()->LookingForGroup_auto_add && player->m_lookingForGroup.more.isAuto())
            {
                uint32 lfgType = (zoneId << 16) | ((1 << 8) | uint8(player->m_lookingForGroup.more.entry));
                LfgDungeons[player->GetTeam()].push_back(lfgType);
                isLFG = true;
            }
        }
#endif
#ifdef MANGOSBOT_TWO
        Group* group = player->GetGroup();
        if (group)
        {
            if (group->IsLFGGroup())
            {
                isLFG = true;
                LFGQueueData& lfgData = sWorld.GetLFGQueue().GetQueueData(group->GetObjectGuid());
                if (lfgData.GetState() != LFG_STATE_NONE && lfgData.GetState() < LFG_STATE_DUNGEON)
                {
                    LfgDungeonSet dList = lfgData.GetDungeons();
                    for (auto dungeon : dList)
                    {
                        LfgDungeons[player->GetTeam()].push_back(dungeon);
                    }
                }
            }
        }
        else
        {
            if (player->GetLfgData().GetState() != LFG_STATE_NONE)
            {
                LFGQueueData& lfgData = sWorld.GetLFGQueue().GetQueueData(player->GetObjectGuid());
                isLFG = true;
                if (lfgData.GetState() < LFG_STATE_DUNGEON)
                {
                    LfgDungeonSet dList = lfgData.GetDungeons();
                    for (auto dungeon : dList)
                    {
                        LfgDungeons[player->GetTeam()].push_back(dungeon);
                    }
                }
            }
        }
#endif
    }
#ifdef MANGOSBOT_ONE
    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;

        if (!bot || !bot->IsInWorld())
            continue;

        if (LfgDungeons[bot->GetTeam()].empty())
            continue;

        WorldSafeLocsEntry const* ClosestGrave = nullptr;
        ClosestGrave = bot->GetMap()->GetGraveyardManager().GetClosestGraveYard(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(), bot->GetTeam());
        uint32 zoneId = 0;
        if (ClosestGrave)
            zoneId = ClosestGrave->ID;

        Group* group = bot->GetGroup();
        if (group && !group->IsFull())
        {
            if (group->IsLeader(bot->GetObjectGuid()))
            {
                if (!bot->m_lookingForGroup.more.empty() && bot->GetSession()->LookingForGroup_auto_add && bot->m_lookingForGroup.more.isAuto())
                {
                    uint32 lfgType = (zoneId << 16) | ((1 << 8) | uint8(bot->m_lookingForGroup.more.entry));
                    LfgDungeons[bot->GetTeam()].push_back(lfgType);
                }
            }
        }
        else if (!group)
        {
            if (!bot->m_lookingForGroup.more.empty() && bot->GetSession()->LookingForGroup_auto_add && bot->m_lookingForGroup.more.isAuto())
            {
                uint32 lfgType = (zoneId << 16) | ((1 << 8) | uint8(bot->m_lookingForGroup.more.entry));
                LfgDungeons[bot->GetTeam()].push_back(lfgType);
            }
        }
    }
#endif
    if (LfgDungeons[ALLIANCE].size() || LfgDungeons[HORDE].size())
        sLog.outBasic("LFG Queue check finished. There are real players in queue.");
    else
        sLog.outBasic("LFG Queue check finished. No real players in queue.");
    return;
}

void RandomPlayerbotMgr::AddOfflineGroupBots()
{
    if (!OfflineGroupBotsTimer || time(NULL) > (OfflineGroupBotsTimer + 5))
        OfflineGroupBotsTimer = time(NULL);

    uint32 totalCounter = 0;
    for (const auto& i : players)
    {
        Player* player = i.second;

        if (!player || !player->IsInWorld() || !player->GetGroup())
            continue;

        Group* group = player->GetGroup();
        if (group && group->IsLeader(player->GetObjectGuid()))
        {
            vector<uint32> botsToAdd;
            Group::MemberSlotList const& slots = group->GetMemberSlots();
            for (Group::MemberSlotList::const_iterator i = slots.begin(); i != slots.end(); ++i)
            {
                ObjectGuid member = i->guid;
                if (member == player->GetObjectGuid())
                    continue;

                if (!IsFreeBot(member.GetCounter()))
                    continue;

                if (sObjectMgr.GetPlayer(member))
                    continue;

                if (GetPlayerBot(member))
                    continue;

                botsToAdd.push_back(member.GetCounter());
            }

            if (botsToAdd.empty())
                return;

            uint32 maxToAdd = urand(1, 5);
            uint32 counter = 0;
            for (auto& guid : botsToAdd)
            {
                if (counter >= maxToAdd)
                    break;

                if (sPlayerbotAIConfig.IsFreeAltBot(guid))
                {
                    for (auto& bot : sPlayerbotAIConfig.freeAltBots)
                    {
                        if (bot.second == guid)
                        {
                            Player* player = GetPlayerBot(bot.second);
                            if (!player)
                            {
                                AddPlayerBot(bot.second, bot.first);
                            }
                        }
                    }
                }
                else
                    AddRandomBot(guid);

                counter++;
                totalCounter++;
            }
        }
    }

    if (totalCounter)
        sLog.outBasic("Added %u offline bots from groups", totalCounter);
}

Item* RandomPlayerbotMgr::CreateTempItem(uint32 item, uint32 count, Player const* player, uint32 randomPropertyId)
{
    if (count < 1)
        return nullptr;                                        // don't create item at zero count

    if (ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(item))
    {
        if (count > pProto->GetMaxStackSize())
            count = pProto->GetMaxStackSize();

        MANGOS_ASSERT(count != 0 && "pProto->Stackable == 0 but checked at loading already");

        Item* pItem = NewItemOrBag(pProto);
        if (pItem->Create(0, item, player))
        {
            pItem->SetCount(count);
            if (int32 randId = randomPropertyId ? randomPropertyId : Item::GenerateItemRandomPropertyId(item))
                pItem->SetItemRandomProperties(randId);

            return pItem;
        }
        delete pItem;
    }
    return nullptr;
}

void RandomPlayerbotMgr::SaveCurTime()
{
    if (!EventTimeSyncTimer || time(NULL) > (EventTimeSyncTimer + 60))
        EventTimeSyncTimer = time(NULL);

    SetValue(uint32(0), "current_time", uint32(time(nullptr)));
}

void RandomPlayerbotMgr::SyncEventTimers()
{
    uint32 oldTime = GetValue(uint32(0), "current_time");
    if (oldTime)
    {
        uint32 curTime = time(nullptr);
        uint32 timeDiff = curTime - oldTime;
        CharacterDatabase.PExecute("UPDATE ai_playerbot_random_bots SET time = time + %u WHERE owner = 0 and bot <> 0", timeDiff);
    }
}

void RandomPlayerbotMgr::CheckPlayers()
{
    if (!PlayersCheckTimer || time(NULL) > (PlayersCheckTimer + 60))
        PlayersCheckTimer = time(NULL);

    sLog.outBasic("Checking Players...");

    for (auto i : players)
    {
        Player* player = i.second;

        if (player->IsGameMaster())
            continue;

        //if (player->GetSession()->GetSecurity() > SEC_PLAYER)
        //    continue;

        if (player->GetLevel() > playersLevel)
            playersLevel = player->GetLevel();
    }
    sLog.outBasic("Max player level is %d, max bot level set to %d", playersLevel, playersLevel);
    return;
}

void RandomPlayerbotMgr::ScheduleRandomize(uint32 bot, uint32 time)
{
    SetEventValue(bot, "randomize", 1, time);
}

void RandomPlayerbotMgr::ScheduleTeleport(uint32 bot, uint32 time)
{
    if (!time)
        time = 60 + urand(sPlayerbotAIConfig.randomBotUpdateInterval, sPlayerbotAIConfig.randomBotUpdateInterval * 3);
    SetEventValue(bot, "teleport", 1, time);
}

void RandomPlayerbotMgr::ScheduleChangeStrategy(uint32 bot, uint32 time)
{
    if (!time)
        time = urand(sPlayerbotAIConfig.minRandomBotChangeStrategyTime, sPlayerbotAIConfig.maxRandomBotChangeStrategyTime);
    SetEventValue(bot, "change_strategy", 1, time);
}

bool RandomPlayerbotMgr::AddRandomBot(uint32 bot)
{
    Player* player = GetPlayerBot(bot);
    if (player)
        return true;

    uint32 accountId = sObjectMgr.GetPlayerAccountIdByGUID(ObjectGuid(HIGHGUID_PLAYER, bot));

    if (!sPlayerbotAIConfig.IsInRandomAccountList(accountId))
    {
        sLog.outError("Bot #%d login fail: Not random bot!", bot);
        return false;
    }

    AddPlayerBot(bot, 0);
    SetEventValue(bot, "add", 1, urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));
    SetEventValue(bot, "logout", 0, 0);
    SetEventValue(bot, "login", 1, sPlayerbotAIConfig.randomBotUpdateInterval);
    uint32 randomTime = urand(sPlayerbotAIConfig.minRandomBotReviveTime, sPlayerbotAIConfig.maxRandomBotReviveTime);
    SetEventValue(bot, "update", 1, randomTime);
    currentBots.push_back(bot);
    sLog.outBasic("Random bot added #%d", bot);
    return true;

    player = GetPlayerBot(bot);

    if (player)
    {
        sLog.outError("Random bot added #%d", bot);
        return true;
    }
    else
    {
        sLog.outError("Failed to add random bot #%d", bot);
        return false;
    }

    return false;
}

bool RandomPlayerbotMgr::ProcessBot(uint32 bot)
{
    Player* player = GetPlayerBot(bot);
    if (player && sPlayerbotAIConfig.IsFreeAltBot(player))
    {
        return false;
    }

    PlayerbotAI* ai = player ? player->GetPlayerbotAI() : NULL;

    bool botsAllowedInWorld = !sPlayerbotAIConfig.randomBotLoginWithPlayer || (!players.empty() && sWorld.GetActiveSessionCount() > 0);

    bool isValid = true;
   
    if (sPlayerbotAIConfig.randomBotTimedLogout && !GetEventValue(bot, "add")) // RandomBotInWorldTime is expired.
        isValid = false;
    else if(!botsAllowedInWorld)                                               // Logout if all players logged out
        isValid = false;

    //Log out bot
    if (!isValid)
    {
        if (botsAllowedInWorld && player && player->GetGroup())
        {
            SetEventValue(bot, "add", 1, 120);                                 // Delay logout for 2 minutes while in group.
            return false;
        }

        if (!player || !player->IsInWorld())
            sLog.outDetail("Bot #%d: log out", bot);
        else
            sLog.outDetail("Bot #%d %s:%d <%s>: log out", bot, IsAlliance(player->getRace()) ? "A" : "H", player->GetLevel(), player->GetName());

        currentBots.remove(bot);
        SetEventValue(bot, "add", 0, 0);

        if (!player)
        {
            return false;
        }    

        LogoutPlayerBot(bot);

        if (sPlayerbotAIConfig.randomBotTimedOffline)
        {
            uint32 logout = GetEventValue(bot, "logout");

            if (!logout)
                SetEventValue(bot, "logout", 1, urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));
        }

        return false;
    }

    //Log in bot (Added in AddRandomBots)
    if (!player)
    {
        if (!botsAllowedInWorld)
            return false;

        AddPlayerBot(bot, 0);
        SetEventValue(bot, "login", 1, sPlayerbotAIConfig.randomBotUpdateInterval * 100);
        uint32 randomTime = urand(sPlayerbotAIConfig.minRandomBotReviveTime, sPlayerbotAIConfig.maxRandomBotReviveTime);
        SetEventValue(bot, "update", 1, randomTime);
        return true;
    }

    if (!player->IsInWorld() || player->IsBeingTeleported() || player->GetSession()->isLogingOut()) //Skip bots that are in limbo.
        return false;

    if(GetEventValue(bot, "login"))
        SetEventValue(bot, "login", 0, 0); //Bot is no longer loggin in.

    uint32 update = GetEventValue(bot, "update");
    //Update the bot
    if (!update)
    {
        //Clean up expired values
        if (ai && !ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
            ai->GetAiObjectContext()->ClearExpiredValues();

        //Randomize/teleport bot
        if (!sPlayerbotAIConfig.disableRandomLevels)
        {
            if (player->GetGroup() || player->IsTaxiFlying())
                return false;

            bool update = true;
            if (ai)
            {
                if (!sRandomPlayerbotMgr.IsRandomBot(player))
                    update = false;

                if (player->GetGroup() && ai->GetGroupMaster() && (!ai->GetGroupMaster()->GetPlayerbotAI() || ai->GetGroupMaster()->GetPlayerbotAI()->IsRealPlayer()))
                    update = false;

                if (ai->HasPlayerNearby())
                    update = false;
            }
            if (update)
                ProcessBot(player);
        }

        uint32 randomTime = urand(sPlayerbotAIConfig.minRandomBotReviveTime, sPlayerbotAIConfig.maxRandomBotReviveTime * 5);
        SetEventValue(bot, "update", 1, randomTime);
        return true;
    }

    return false;
}

bool RandomPlayerbotMgr::ProcessBot(Player* player)
{
    if (!player || !player->IsInWorld() || player->IsBeingTeleported() || player->GetSession()->isLogingOut())
        return false;

    uint32 bot = player->GetGUIDLow();

    if (player->InBattleGround())
        return false;

    if (player->InBattleGroundQueue())
        return false;

    // only teleport idle bots
    bool idleBot = false;
    TravelTarget* target = player->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
    if (target)
    {
        if (target->getTravelState() == TravelState::TRAVEL_STATE_IDLE)
            idleBot = true;
    }
    else
        idleBot = true;

    if (idleBot)
    {
        uint32 randomize = GetEventValue(bot, "randomize");
        if (!randomize)
        {
            bool randomiser = true;
            if (player->GetGuildId())
            {
                Guild* guild = sGuildMgr.GetGuildById(player->GetGuildId());
                uint32 accountId = sObjectMgr.GetPlayerAccountIdByGUID(guild->GetLeaderGuid());
                if (!sPlayerbotAIConfig.IsInRandomAccountList(accountId))
                {
                    int32 rank = guild->GetRank(player->GetObjectGuid());
                    randomiser = rank < 4 ? false : true;
                }
            }

            if (randomiser)
            {
                Randomize(player);
                return true;
            }
        }

        uint32 changeStrategy = GetEventValue(bot, "change_strategy");
        if (!changeStrategy)
        {
            sLog.outDetail("Changing strategy for bot #%d %s:%d <%s>", bot, player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName());
            ChangeStrategy(player);
            ScheduleChangeStrategy(bot);
            return true;
        }

        uint32 teleport = GetEventValue(bot, "teleport");
        if (!teleport && players.size())
        {
            sLog.outBasic("Bot #%d %s:%d <%s>: sent to grind", bot, player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName());
            RandomTeleportForLevel(player, true);
            ScheduleTeleport(bot);
            return true;
        }
    }

    return false;
}

void RandomPlayerbotMgr::Revive(Player* player)
{
    uint32 bot = player->GetGUIDLow();

    //sLog.outString("Bot %d revived", bot);
    SetEventValue(bot, "dead", 0, 0);
    SetEventValue(bot, "revive", 0, 0);

    if (sServerFacade.GetDeathState(player) == CORPSE)
    {
        RandomTeleport(player);
    }
    else
    {
        RandomTeleportForLevel(player, false);
    }
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot, vector<WorldLocation> &locs, bool hearth, bool activeOnly)
{
    if (bot->IsBeingTeleported())
        return;

    if (bot->InBattleGround())
        return;

    if (bot->InBattleGroundQueue())
        return;

	if (bot->GetLevel() < 5)
		return;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        return;

    if (bot->IsTaxiFlying() && bot->GetPlayerbotAI()->HasPlayerNearby())
        return;

    if (sPlayerbotAIConfig.randomBotRpgChance < 0)
        return;

    if (locs.empty())
    {
        sLog.outError("Cannot teleport bot %s - no locations available", bot->GetName());
        return;
    }

    vector<WorldPosition> tlocs;

    for (auto& loc : locs)
    {
        tlocs.push_back(WorldPosition(loc));
    }

    //Do not teleport to maps disabled in config
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [](const WorldPosition& l) {vector<uint32>::iterator i = find(sPlayerbotAIConfig.randomBotMaps.begin(), sPlayerbotAIConfig.randomBotMaps.end(), l.getMapId()); return i == sPlayerbotAIConfig.randomBotMaps.end(); }), tlocs.end());

    //Random shuffle based on distance. Closer distances are more likely (but not exclusively) to be at the begin of the list.
    tlocs = sTravelMgr.getNextPoint(WorldPosition(bot), tlocs, 0);

    //5% + 0.1% per level chance node on different map in selection.
    //tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](WorldLocation const& l) {return l.mapid != bot->GetMapId() && urand(1, 100) > 0.5 * bot->GetLevel(); }), tlocs.end());

    //Continent is about 20.000 large
    //Bot will travel 0-5000 units + 75-150 units per level.
    //tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](WorldLocation const& l) {return l.mapid == bot->GetMapId() && sServerFacade.GetDistance2d(bot, l.coord_x, l.coord_y) > urand(0, 5000) + bot->GetLevel() * 15 * urand(5, 10); }), tlocs.end());

    // teleport to active areas only
    if (sPlayerbotAIConfig.randomBotTeleportNearPlayer && activeOnly)
    {
        tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [this](const WorldPosition& l)
        {
            uint32 mapId = l.getMapId();
            Map* tMap = sMapMgr.FindMap(mapId, 0);
            if (tMap && tMap->IsContinent() && tMap->HasActiveZones())
            {
                uint32 zoneId = sTerrainMgr.GetZoneId(mapId, l.coord_x, l.coord_y, l.coord_z);
                if (tMap->HasActiveZone(zoneId))
                {
                    if (sPlayerbotAIConfig.randomBotTeleportNearPlayerMaxAmount > 0 && sPlayerbotAIConfig.randomBotTeleportNearPlayerMaxAmountRadius > 0.0f)
                    {
                        uint32 botsNearTeleportPoint = 0;
                        for (auto& pair : GetAllBots())
                        {
                            // Only check the bots that are on the same zone
                            Player* otherBot = pair.second;
                            if (otherBot && !otherBot->IsBeingTeleported() && zoneId == otherBot->GetZoneId())
                            {
                                if (l.fDist(WorldPosition(otherBot)) <= sPlayerbotAIConfig.randomBotTeleportNearPlayerMaxAmountRadius)
                                {
                                    botsNearTeleportPoint++;
                                }
                            }
                        }

                        return botsNearTeleportPoint >= sPlayerbotAIConfig.randomBotTeleportNearPlayerMaxAmount;
                    }
                    else
                    {
                        return false;
                    }
                }
            }

            return true;
        }), 
        tlocs.end());

        /*if (!tlocs.empty())
        {
            tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](const WorldPosition& l)
            {
                uint32 mapId = l.getMapId();
                Map* tMap = sMapMgr.FindMap(mapId, 0);
                if (!tMap || !tMap->IsContinent())
                        return true;

                if (!tMap->HasActiveAreas())
                    return true;

                AreaTableEntry const* area = l.getArea();
                if (area)
                {
                    if (!tMap->HasActiveZone(area->zone ? area->zone : area->ID))
                        return true;
                }
            }), tlocs.end());
        }*/
    }

    // filter starter zones
    tlocs.erase(std::remove_if(tlocs.begin(), tlocs.end(), [bot](const WorldPosition& l)
    {
        uint32 mapId = l.getMapId();
        uint32 zoneId, areaId;
        sTerrainMgr.GetZoneAndAreaId(zoneId, areaId, mapId, l.coord_x, l.coord_y, l.coord_z);
        AreaTableEntry const* area = GetAreaEntryByAreaID(areaId);
        if (zoneId && zoneId != areaId)
        {
            AreaTableEntry const* zone = GetAreaEntryByAreaID(zoneId);
            if (!zone)
                return true;

            bool isEnemyZone = false;
            switch (zone->team)
            {
            case AREATEAM_ALLY:
                isEnemyZone = bot->GetTeam() != ALLIANCE;
                break;
            case AREATEAM_HORDE:
                isEnemyZone = bot->GetTeam() != HORDE;
                break;
            default:
                isEnemyZone = false;
                break;
            }
            if (isEnemyZone && (bot->GetLevel() < 21 || (zone->flags & AREA_FLAG_CAPITAL)))
                return true;

            // filter other races zones
            if (bot->GetLevel() < 30)
            {
                if ((zoneId == 12 || zoneId == 40) && bot->getRace() != RACE_HUMAN)
                    return true;
                if ((zoneId == 1 || zoneId == 38) && bot->getRace() != RACE_DWARF)
                    return true;
                if ((zoneId == 85 || zoneId == 130) && bot->getRace() != RACE_UNDEAD)
                    return true;
                if ((zoneId == 141 || zoneId == 148) && bot->getRace() != RACE_NIGHTELF)
                    return true;
                if ((zoneId == 14 || zoneId == 17) && !(bot->getRace() == RACE_ORC || bot->getRace() == RACE_TROLL))
                    return true;
                if ((zoneId == 215) && bot->getRace() != RACE_TAUREN)
                    return true;
#ifndef MANGOSBOT_ZERO
                if ((zoneId == 3524 || zoneId == 3525) && bot->getRace() != RACE_DRAENEI)
                    return true;
                if ((zoneId == 3430 || zoneId == 3433) && bot->getRace() != RACE_BLOODELF)
                    return true;
#endif
            }
        }

        if (!area)
            return true;

        bool isEnemyZone = false;
        switch (area->team)
        {
        case AREATEAM_ALLY:
            isEnemyZone = bot->GetTeam() != ALLIANCE;
            break;
        case AREATEAM_HORDE:
            isEnemyZone = bot->GetTeam() != HORDE;
            break;
        default:
            isEnemyZone = false;
            break;
        }
        return isEnemyZone && bot->GetLevel() < 21;

    }), tlocs.end());

    if (tlocs.empty())
    {
        if (activeOnly)
        {
            if (hearth)
                return RandomTeleportForRpg(bot, false);
            else
                return RandomTeleportForLevel(bot, false);
        }

        sLog.outError("Cannot teleport bot %s - no locations available", bot->GetName());

        return;
    }

    PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "RandomTeleportByLocations");

    int index = 0;

    for (int i = 0; i < tlocs.size(); i++)
    {
        for (int attemtps = 0; attemtps < 3; ++attemtps)
        {
            WorldLocation loc = tlocs[i];

#ifdef MANGOSBOT_ONE
            // Teleport to Dark Portal area if event is in progress
            if (sWorldState.GetExpansion() == EXPANSION_NONE && bot->GetLevel() > 54 && urand(0, 100) > 20)
            {
                if (urand(0, 1))
                    loc = WorldLocation(uint32(0), -11772.43f, -3272.84f, -17.9f, 3.32447f);
                else
                    loc = WorldLocation(uint32(0), -11741.70f, -3130.3f, -11.7936f, 3.32447f);
            }
#endif

            float x = loc.coord_x + (attemtps > 0 ? urand(0, sPlayerbotAIConfig.grindDistance) - sPlayerbotAIConfig.grindDistance / 2 : 0);
            float y = loc.coord_y + (attemtps > 0 ? urand(0, sPlayerbotAIConfig.grindDistance) - sPlayerbotAIConfig.grindDistance / 2 : 0);
            float z = loc.coord_z;

            Map* map = sMapMgr.FindMap(loc.mapid, 0);
            if (!map)
                continue;

            uint32 areaId = sTerrainMgr.GetAreaId(loc.mapid, x, y, z);
            AreaTableEntry const* area = GetAreaEntryByAreaID(areaId);
            if (!area)
                continue;

#ifndef MANGOSBOT_ZERO
            // Do not teleport to outland before portal opening (allow new races zones)
            if (sWorldState.GetExpansion() == EXPANSION_NONE && (loc.mapid == 571 || (loc.mapid == 530 && area->team != 2 && area->team != 4)))
                continue;
#endif

#ifdef MANGOSBOT_TWO
            float ground = map->GetHeight(bot->GetPhaseMask(), x, y, z + 0.5f);
#else
            float ground = map->GetHeight(x, y, z + 0.5f);
#endif
            if (ground <= INVALID_HEIGHT)
                continue;

            z = 0.05f + ground;
            sLog.outDetail("Random teleporting bot %s to %s %f,%f,%f (%u/%zu locations)",
                bot->GetName(), area->area_name[0], x, y, z, attemtps, tlocs.size());

            if (bot->IsTaxiFlying())          
                bot->GetMotionMaster()->MovementExpired();

            if (hearth)
                bot->SetHomebindToLocation(loc, area->ID);

            bot->GetMotionMaster()->Clear();
            bot->TeleportTo(loc.mapid, x, y, z, 0);
            bot->SendHeartBeat();
            bot->GetPlayerbotAI()->Reset(true);

            if (bot->GetGroup())
            {
                for (GroupReference* gref = bot->GetGroup()->GetFirstMember(); gref; gref = gref->next())
                {
                    Player* member = gref->getSource();
                    PlayerbotAI* ai = bot->GetPlayerbotAI();
                    if (ai && bot != member)
                    {
                        if (member->IsTaxiFlying())
                            member->GetMotionMaster()->MovementExpired();
                        if (hearth)
                            member->SetHomebindToLocation(loc, area->ID);

                        member->GetMotionMaster()->Clear();
                        member->TeleportTo(loc.mapid, x, y, z, 0);
                        member->SendHeartBeat();
                        member->GetPlayerbotAI()->Reset(true);
                    }

                }
            }
            if (pmo) pmo->finish();
            return;
        }
    }

    if (pmo) pmo->finish();
    sLog.outError("Cannot teleport bot %s - no locations available", bot->GetName());
}

std::vector<std::pair<uint32, uint32>> RandomPlayerbotMgr::RpgLocationsNear(WorldLocation pos, uint32 areaId, uint32 radius)
{
    vector<std::pair<uint32, uint32>> results;
    float minDist = FLT_MAX;
    string hasZone = "-", wantZone = WorldPosition(pos).getAreaName(true, true);
    for (uint32 level = 1; level < sPlayerbotAIConfig.randomBotMaxLevel + 1; level++)
    {
        for (uint32 r = 1; r < MAX_RACES; r++)
        {
            for (auto p : rpgLocsCacheLevel[r][level])
            {
                string currentZone = WorldPosition(p).getAreaName(true, true);

                if (currentZone != wantZone && hasZone == wantZone) //If we already have the right id but this location isn't in the right id. Skip it.
                    continue;

                if (currentZone == wantZone && hasZone != wantZone) //If this is the first spot with a good area id use this now.
                    minDist = FLT_MAX;

                float dist = WorldPosition(pos).fDist(p);

                if (dist > radius || dist > minDist)
                    continue;

                if (dist < minDist)
                    results.clear();

                results.push_back(make_pair(r, level));

                hasZone = currentZone;

                minDist = dist;
            }
        }
    }

    return results;
}

void RandomPlayerbotMgr::PrepareTeleportCache()
{
    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    auto results = CharacterDatabase.PQuery("select map_id, x, y, z, level from ai_playerbot_tele_cache");
    if (results)
    {
        sLog.outString("Loading random teleport caches for %d levels...", maxLevel);
        do
        {
            Field* fields = results->Fetch();
            uint16 mapId = fields[0].GetUInt16();
            float x = fields[1].GetFloat();
            float y = fields[2].GetFloat();
            float z = fields[3].GetFloat();
            uint16 level = fields[4].GetUInt16();
            WorldLocation loc(mapId, x, y, z, 0);
            locsPerLevelCache[level].push_back(loc);
        } while (results->NextRow());
    }
    else
    {
        sLog.outString("Preparing random teleport caches for %d levels...", maxLevel);
        BarGoLink bar(maxLevel);
        for (uint8 level = 1; level <= maxLevel; level++)
        {
            auto results = WorldDatabase.PQuery("select map, position_x, position_y, position_z "
                "from (select map, avg(position_x) position_x, avg(position_y) position_y, avg(position_z) position_z, avg(t.maxlevel), avg(t.minlevel), "
                "%u - (avg(t.maxlevel) + avg(t.minlevel)) / 2 delta "
                "from creature c inner join creature_template t on c.id = t.entry where t.NpcFlags = 0 and NOT (extraFlags & 1024 OR extraflags & 64 OR unitFlags & 256 OR unitFlags & 512) and t.lootid != 0 group by t.entry, c.map having count(*) > 1) q "
                "where delta >= 0 and delta <= %u and map in (%s) and not exists ( "
                "select map, position_x, position_y, position_z from "
                "("
                "select map, avg(position_x) position_x, avg(position_y) position_y, avg(position_z) position_z, avg(t.maxlevel), avg(t.minlevel), "
                "%u - (avg(t.maxlevel) + avg(t.minlevel)) / 2 delta "
                "from creature c "
                "inner join creature_template t on c.id = t.entry where t.NpcFlags = 0 and NOT (extraFlags & 1024 OR extraflags & 64 OR unitFlags & 256 OR unitFlags & 512) and t.lootid != 0 group by t.entry, c.map "
                ") q1 "
                "where abs(delta) > %u and q1.map = q.map "
                "and sqrt("
                "(q1.position_x - q.position_x)*(q1.position_x - q.position_x) +"
                "(q1.position_y - q.position_y)*(q1.position_y - q.position_y) +"
                "(q1.position_z - q.position_z)*(q1.position_z - q.position_z)"
                ") < %u)",
                level,
                sPlayerbotAIConfig.randomBotTeleLevel,
                sPlayerbotAIConfig.randomBotMapsAsString.c_str(),
                level,
                sPlayerbotAIConfig.randomBotTeleLevel,
                (uint32)sPlayerbotAIConfig.sightDistance
            );
            if (results)
            {
                do
                {
                    Field* fields = results->Fetch();
                    uint16 mapId = fields[0].GetUInt16();
                    float x = fields[1].GetFloat();
                    float y = fields[2].GetFloat();
                    float z = fields[3].GetFloat();
                    WorldLocation loc(mapId, x, y, z, 0);
                    locsPerLevelCache[level].push_back(loc);

                    CharacterDatabase.PExecute("insert into ai_playerbot_tele_cache (level, map_id, x, y, z) values (%u, %u, %f, %f, %f)",
                        level, mapId, x, y, z);
                } while (results->NextRow());
            }
            bar.step();
        }
    }

    sLog.outString("Preparing RPG teleport caches for %d factions...", sFactionTemplateStore.GetNumRows());

    results = WorldDatabase.PQuery("SELECT map, position_x, position_y, position_z, "
        "r.race, r.minl, r.maxl "
        "from creature c inner join ai_playerbot_rpg_races r on c.id = r.entry "
        "where r.race < 15");

    if (results)
    {
        do
        {
            for (uint32 level = 1; level < sPlayerbotAIConfig.randomBotMaxLevel + 1; level++)
            {
                Field* fields = results->Fetch();
                uint16 mapId = fields[0].GetUInt16();
                float x = fields[1].GetFloat();
                float y = fields[2].GetFloat();
                float z = fields[3].GetFloat();
                //uint32 faction = fields[4].GetUInt32();
                //string name = fields[5].GetCppString();
                uint32 race = fields[4].GetUInt32();
                uint32 minl = fields[5].GetUInt32();
                uint32 maxl = fields[6].GetUInt32();

                if (level > maxl || level < minl) continue;

                WorldLocation loc(mapId, x, y, z, 0);
                for (uint32 r = 1; r < MAX_RACES; r++)
                {
                    if (race == r || race == 0) rpgLocsCacheLevel[r][level].push_back(loc);
                }
            }
            //bar.step();
        } while (results->NextRow());
    }

    sLog.outString("Enhancing RPG teleport cache");

    std::vector<std::pair<std::pair<uint32, uint32>, WorldPosition>> newPoints;

    //Static portals.
    for (auto& goData : WorldPosition().getGameObjectsNear(0, 0))
    {
        GuidPosition go(goData);

        auto data = sGOStorage.LookupEntry<GameObjectInfo>(go.GetEntry());

        if (!data)
            continue;

        if (data->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(data->spellcaster.spellId);

        if (pSpellInfo->EffectTriggerSpell[0])
            pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        SpellTargetPosition const* pos = sSpellMgr.GetSpellTargetPosition(pSpellInfo->Id);

        if (!pos)
            continue;

        std::vector<std::pair<uint32, uint32>> ranges = RpgLocationsNear(WorldPosition(pos));

        for (auto& range : ranges)
            newPoints.push_back(make_pair(make_pair(range.first, range.second), pos));
    }

    //Creatures.
    for (auto& creatureData : WorldPosition().getCreaturesNear(0, 0))
    {
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creatureData->second.id);

        if (!cInfo)
            continue;

        if (cInfo->ExtraFlags & CREATURE_EXTRA_FLAG_INVISIBLE)
            continue;

        vector<uint32> allowedNpcFlags;

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BATTLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

        for (auto flag : allowedNpcFlags)
        {
            if ((cInfo->NpcFlags & flag) != 0)
            {
                std::vector<std::pair<uint32, uint32>> ranges = RpgLocationsNear(WorldPosition(creatureData));
                
                for (auto& range : ranges)
                    newPoints.push_back(make_pair(make_pair(range.first, range.second), creatureData));
                break;
            }
        }
    }

    for (auto newPoint : newPoints)
        rpgLocsCacheLevel[newPoint.first.first][newPoint.first.second].push_back(newPoint.second);
}

void RandomPlayerbotMgr::PrintTeleportCache()
{
    sPlayerbotAIConfig.openLog("telecache.csv", "w");

    for (auto& l : sRandomPlayerbotMgr.locsPerLevelCache)
    {
        uint32 level = l.first;
        for (auto& p : l.second)
        {
            ostringstream out;
            out << level << ",";
            WorldPosition(p).printWKT(out);
            out << "LEVEL" << ",0," << WorldPosition(p).getAreaName(true, true);
            sPlayerbotAIConfig.log("telecache.csv", out.str().c_str());
        }
    }

    for (auto r : sRandomPlayerbotMgr.rpgLocsCacheLevel)
    {
        uint32 race =  r.first;
        for (auto& l : r.second)
        {
            uint32 level = l.first;
            for (auto& p : l.second)
            {
                ostringstream out;
                out << level << ",";
                WorldPosition(p).printWKT(out);
                out << "RPG" << "," << race << "," << WorldPosition(p).getAreaName(true, true);
                sPlayerbotAIConfig.log("telecache.csv", out.str().c_str());
            }
        }
    }
}

void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot, bool activeOnly)
{
    if (bot->InBattleGround())
        return;

    sLog.outDetail("Preparing location to random teleporting bot %s for level %u", bot->GetName(), bot->GetLevel());
    RandomTeleport(bot, locsPerLevelCache[bot->GetLevel()], false, activeOnly);
    Refresh(bot);
}

void RandomPlayerbotMgr::RandomTeleport(Player* bot)
{
    if (bot->InBattleGround())
        return;

    PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "RandomTeleport");
    vector<WorldLocation> locs;

    list<Unit*> targets;
    float range = sPlayerbotAIConfig.randomBotTeleportDistance;
    MaNGOS::AnyUnitInObjectRangeCheck u_check(bot, range);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    if (!targets.empty())
    {
        for (list<Unit *>::iterator i = targets.begin(); i != targets.end(); ++i)
        {
            Unit* unit = *i;
            bot->SetPosition(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ(), 0);
            FleeManager manager(bot, sPlayerbotAIConfig.sightDistance, 0, true);
            float rx, ry, rz;
            if (manager.CalculateDestination(&rx, &ry, &rz))
            {
                WorldLocation loc(bot->GetMapId(), rx, ry, rz);
                locs.push_back(loc);
            }
        }
    }
    else
    {
        RandomTeleportForLevel(bot, true);
    }

    if (pmo) pmo->finish();

    Refresh(bot);
}

void RandomPlayerbotMgr::InstaRandomize(Player* bot)
{
    sRandomPlayerbotMgr.Randomize(bot);

    if(bot->GetLevel() > sWorld.getConfig(CONFIG_UINT32_START_PLAYER_LEVEL))
        sRandomPlayerbotMgr.RandomTeleportForLevel(bot, false);
}

void RandomPlayerbotMgr::Randomize(Player* bot)
{
    if (!bot || !bot->IsInWorld() || bot->IsBeingTeleported() || bot->GetSession()->isLogingOut())
        return;

    bool initialRandom = false;
    if (bot->GetLevel() <= sPlayerbotAIConfig.randombotStartingLevel)
        initialRandom = true;
#ifdef MANGOSBOT_TWO
    else if (bot->GetLevel() < 60 && bot->getClass() == CLASS_DEATH_KNIGHT)
        initialRandom = true;
#endif

    // give bot random level if is above or below level sync
    if (!initialRandom && players.size() && sPlayerbotAIConfig.syncLevelWithPlayers)
    {
        uint32 maxLevel = max(sPlayerbotAIConfig.randomBotMinLevel, min(playersLevel + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));
        if (bot->GetLevel() > maxLevel || (bot->GetLevel() + sPlayerbotAIConfig.syncLevelMaxAbove) < playersLevel)
            initialRandom = true;
    }

    if (initialRandom)
    {
        RandomizeFirst(bot);
        sLog.outBasic("Bot #%d %s:%d <%s>: gear/level randomised", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
    }
    else if (sPlayerbotAIConfig.randomGearUpgradeEnabled)
    {
        UpdateGearSpells(bot);
        sLog.outBasic("Bot #%d %s:%d <%s>: gear upgraded", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName());
    }
    else
    {
        // schedule randomise
        uint32 randomTime = urand(sPlayerbotAIConfig.minRandomBotRandomizeTime, sPlayerbotAIConfig.maxRandomBotRandomizeTime);
        SetEventValue(bot->GetGUIDLow(), "randomize", 1, randomTime);
    }

    //SetValue(bot, "version", MANGOSBOT_VERSION);
}

void RandomPlayerbotMgr::UpdateGearSpells(Player* bot)
{
    PerformanceMonitorOperation* pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "UpgradeGear");

    uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
    if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
        maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    uint32 lastLevel = GetValue(bot, "level");
    uint32 level = bot->GetLevel();
    PlayerbotFactory factory(bot, level);
    factory.Randomize(true, false);

    if (lastLevel != level)
        SetValue(bot, "level", level);

    // schedule randomise
    uint32 randomTime = urand(sPlayerbotAIConfig.minRandomBotRandomizeTime, sPlayerbotAIConfig.maxRandomBotRandomizeTime);
    SetEventValue(bot->GetGUIDLow(), "randomize", 1, randomTime);

    if (pmo) pmo->finish();
}

void RandomPlayerbotMgr::RandomizeFirst(Player* bot)
{
	uint32 maxLevel = sPlayerbotAIConfig.randomBotMaxLevel;
	if (maxLevel > sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL))
		maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

    // if lvl sync is enabled, max level is limited by online players lvl
    if (sPlayerbotAIConfig.syncLevelWithPlayers)
        maxLevel = max(sPlayerbotAIConfig.randomBotMinLevel, min(playersLevel+ sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));

	PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "RandomizeFirst");
    uint32 level = urand(std::max(uint32(sWorld.getConfig(CONFIG_UINT32_START_PLAYER_LEVEL)), sPlayerbotAIConfig.randomBotMinLevel), maxLevel);

#ifdef MANGOSBOT_TWO
    if (bot->getClass() == CLASS_DEATH_KNIGHT)
        level = urand(max(bot->GetLevel(), sWorld.getConfig(CONFIG_UINT32_START_HEROIC_PLAYER_LEVEL)), max(sWorld.getConfig(CONFIG_UINT32_START_HEROIC_PLAYER_LEVEL), maxLevel));
#endif

    if (urand(0, 100) < 100 * sPlayerbotAIConfig.randomBotMaxLevelChance && level < maxLevel)
        level = maxLevel;

#ifndef MANGOSBOT_ZERO
    if (sWorldState.GetExpansion() == EXPANSION_NONE && level > 60)
        level = 60;
#endif

#ifdef MANGOSBOT_TWO
    // do not allow level down death knights
    if (bot->getClass() == CLASS_DEATH_KNIGHT && level < sWorld.getConfig(CONFIG_UINT32_START_HEROIC_PLAYER_LEVEL))
        return;

    // only randomise death knights to min lvl 60
    if (bot->getClass() == CLASS_DEATH_KNIGHT && level < 60)
        level = 60;
#endif

    if (level == sWorld.getConfig(CONFIG_UINT32_START_PLAYER_LEVEL))
        return;

    SetValue(bot, "level", level);
    PlayerbotFactory factory(bot, level);
    factory.Randomize(false, false);
	
    // schedule randomise
    uint32 randomTime = urand(sPlayerbotAIConfig.minRandomBotRandomizeTime, sPlayerbotAIConfig.maxRandomBotRandomizeTime);
    SetEventValue(bot->GetGUIDLow(), "randomize", 1, randomTime);

    bool hasPlayer = bot->GetPlayerbotAI()->HasRealPlayerMaster();
    bot->GetPlayerbotAI()->Reset(!hasPlayer);

    if (bot->GetGroup() && !hasPlayer)
        bot->RemoveFromGroup();

	if (pmo) pmo->finish();
}

uint32 RandomPlayerbotMgr::GetZoneLevel(uint16 mapId, float teleX, float teleY, float teleZ)
{
	uint32 maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);

	uint32 level;
    auto results = WorldDatabase.PQuery("select avg(t.minlevel) minlevel, avg(t.maxlevel) maxlevel from creature c "
            "inner join creature_template t on c.id = t.entry "
            "where map = '%u' and minlevel > 1 and abs(position_x - '%f') < '%u' and abs(position_y - '%f') < '%u'",
            mapId, teleX, sPlayerbotAIConfig.randomBotTeleportDistance / 2, teleY, sPlayerbotAIConfig.randomBotTeleportDistance / 2);

    if (results)
    {
        Field* fields = results->Fetch();
        uint8 minLevel = fields[0].GetUInt8();
        uint8 maxLevel = fields[1].GetUInt8();
        level = urand(minLevel, maxLevel);
        if (level > maxLevel)
            level = maxLevel;
    }
    else
    {
        level = urand(1, maxLevel);
    }

    return level;
}

void RandomPlayerbotMgr::Refresh(Player* bot)
{
    if (bot->IsBeingTeleportedFar() || !bot->IsInWorld())
        return;

    if (sServerFacade.UnitIsDead(bot))
    {
        bot->ResurrectPlayer(1.0f);
        bot->SpawnCorpseBones();
        bot->GetPlayerbotAI()->ResetStrategies();
    }

    if (sPlayerbotAIConfig.disableRandomLevels)
        return;

    if (bot->InBattleGround())
        return;

    sLog.outDetail("Refreshing bot #%d <%s>", bot->GetGUIDLow(), bot->GetName());
    PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_RNDBOT, "Refresh");

    bot->GetPlayerbotAI()->Reset();

    bot->DurabilityRepairAll(false, 1.0f
#ifndef MANGOSBOT_ZERO
        , false
#endif
    );
	bot->SetHealthPercent(100);
	bot->SetPvP(true);

    PlayerbotFactory factory(bot, bot->GetLevel());
    factory.Refresh();

    if (bot->GetMaxPower(POWER_MANA) > 0)
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    if (bot->GetMaxPower(POWER_ENERGY) > 0)
        bot->SetPower(POWER_ENERGY, bot->GetMaxPower(POWER_ENERGY));

    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 500 * sqrt(urand(1, bot->GetLevel() * 5)));

    if (pmo) pmo->finish();
}

bool RandomPlayerbotMgr::IsRandomBot(Player* bot)
{
    if (bot && bot->GetPlayerbotAI())
    {
        if (bot->GetPlayerbotAI()->IsRealPlayer())
            return false;
    }
    if (bot)
    {
        if (sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()))
            return true;

        return IsRandomBot(bot->GetGUIDLow());
    }

    return false;
}

bool RandomPlayerbotMgr::IsRandomBot(uint32 bot)
{
    ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, bot);
    if (sPlayerbotAIConfig.IsInRandomAccountList(sObjectMgr.GetPlayerAccountIdByGUID(guid)))
        return true;

    return GetEventValue(bot, "add");
}

list<uint32> RandomPlayerbotMgr::GetBots()
{
    if (!currentBots.empty()) return currentBots;

    auto results = CharacterDatabase.Query(
            "select bot from ai_playerbot_random_bots where owner = 0 and event = 'add'");

    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            currentBots.push_back(bot);
        } while (results->NextRow());
    }

    return currentBots;
}

list<uint32> RandomPlayerbotMgr::GetBgBots(uint32 bracket)
{
    //if (!currentBgBots.empty()) return currentBgBots;

    auto results = CharacterDatabase.PQuery(
        "select bot from ai_playerbot_random_bots where event = 'bg' AND value = '%d'", bracket);
    list<uint32> BgBots;
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 bot = fields[0].GetUInt32();
            BgBots.push_back(bot);
        } while (results->NextRow());
    }

    return BgBots;
}

uint32 RandomPlayerbotMgr::GetEventValue(uint32 bot, string event)
{
    // load all events at once on first event load
    if (eventCache[bot].empty())
    {
        auto results = CharacterDatabase.PQuery("SELECT `event`, `value`, `time`, validIn, `data` from ai_playerbot_random_bots where owner = 0 and bot = '%u'", bot);
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                string eventName = fields[0].GetString();
                CachedEvent e;
                e.value = fields[1].GetUInt32();
                e.lastChangeTime = fields[2].GetUInt32();
                e.validIn = fields[3].GetUInt32();
                e.data = fields[4].GetString();
                eventCache[bot][eventName] = e;
            } while (results->NextRow());
        }
    }
    CachedEvent e = eventCache[bot][event];

    if ((time(0) - e.lastChangeTime) >= e.validIn && event != "specNo" && event != "specLink" && event != "init" && event != "current_time" && event != "always" && event != "selfbot")
        e.value = 0;

    return e.value;
}

int32 RandomPlayerbotMgr::GetValueValidTime(uint32 bot, string event)
{
    if (eventCache.find(bot) == eventCache.end())
        return 0;

    if (eventCache[bot].find(event) == eventCache[bot].end())
        return 0;

    CachedEvent e = eventCache[bot][event];

    return e.validIn-(time(0) - e.lastChangeTime);
}

string RandomPlayerbotMgr::GetEventData(uint32 bot, string event)
{
    string data = "";
    if (GetEventValue(bot, event))
    {
        CachedEvent e = eventCache[bot][event];
        data = e.data;
    }
    return data;
}

uint32 RandomPlayerbotMgr::SetEventValue(uint32 bot, string event, uint32 value, uint32 validIn, string data)
{
    CharacterDatabase.PExecute("delete from ai_playerbot_random_bots where owner = 0 and bot = '%u' and event = '%s'",
            bot, event.c_str());
    if (value)
    {
        if (data != "")
        {
            CharacterDatabase.PExecute(
                "insert into ai_playerbot_random_bots (owner, bot, `time`, validIn, event, `value`, `data`) values ('%u', '%u', '%u', '%u', '%s', '%u', '%s')",
                0, bot, (uint32)time(0), validIn, event.c_str(), value, data.c_str());
        }
        else
        {
            CharacterDatabase.PExecute(
                "insert into ai_playerbot_random_bots (owner, bot, `time`, validIn, event, `value`) values ('%u', '%u', '%u', '%u', '%s', '%u')",
                0, bot, (uint32)time(0), validIn, event.c_str(), value);
        }
    }

    CachedEvent e(value, (uint32)time(0), validIn, data);
    eventCache[bot][event] = e;
    return value;
}

uint32 RandomPlayerbotMgr::GetValue(uint32 bot, string type)
{
    return GetEventValue(bot, type);
}

uint32 RandomPlayerbotMgr::GetValue(Player* bot, string type)
{
    return GetValue(bot->GetObjectGuid().GetCounter(), type);
}

string RandomPlayerbotMgr::GetData(uint32 bot, string type)
{
    return GetEventData(bot, type);
}

void RandomPlayerbotMgr::SetValue(uint32 bot, string type, uint32 value, string data)
{
    SetEventValue(bot, type, value, 15*24*3600, data);
}

void RandomPlayerbotMgr::SetValue(Player* bot, string type, uint32 value, string data)
{
    SetValue(bot->GetObjectGuid().GetCounter(), type, value, data);
}

bool RandomPlayerbotMgr::HandlePlayerbotConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig.enabled)
    {
        sLog.outError("Playerbot system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        sLog.outError("Usage: rndbot stats/update/reset/init/refresh/add/remove");
        return false;
    }

    string cmd = args;

    if (cmd == "reset")
    {
        CharacterDatabase.PExecute("delete from ai_playerbot_random_bots");
        sRandomPlayerbotMgr.eventCache.clear();
        sLog.outString("Random bots were reset for all players. Please restart the Server.");
        return true;
    }

    if (cmd == "stats")
    {
        activatePrintStatsThread();
        return true;
    }

    if (cmd == "reload")
    {
        sPlayerbotAIConfig.Initialize();
        return true;
    }

    if (cmd == "update")
    {
        sRandomPlayerbotMgr.UpdateAIInternal(0);
        return true;
    }

    if (cmd.find("pid ") != std::string::npos)
    {
        string pids = cmd.substr(4);
        vector<string> pid = Qualified::getMultiQualifiers(pids, " ");

        if (pid.size() == 0)
            pid.push_back("0");
        if (pid.size() == 1)
            pid.push_back("0");
        if (pid.size() == 2)
            pid.push_back("0");
        sRandomPlayerbotMgr.pid.adjust(stof(pid[0]), stof(pid[1]), stof(pid[2]));

        sLog.outString("Pid set to p:%f i:%f d:%f", stof(pid[0]), stof(pid[1]), stof(pid[2]));

        return true;
    }


    if (cmd.find("diff ") != std::string::npos)
    {
        string diffs = cmd.substr(5);
        vector<string> diff = Qualified::getMultiQualifiers(diffs, " ");
        if (diff.size() == 0)
            diff.push_back("100");
        if (diff.size() == 1)
            diff.push_back(diff[0]);
        sPlayerbotAIConfig.diffWithPlayer = stoi(diff[0]);
        sPlayerbotAIConfig.diffEmpty = stoi(diff[1]);

        sLog.outString("Diff set to %d (player), %d (empty)", stoi(diff[0]), stoi(diff[1]));

        return true;
    }
    if (cmd.find("clean map") == 0)
    {
        for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
        {
            if (!sMapStore.LookupEntry(i))
                continue;

            uint32 mapId = sMapStore.LookupEntry(i)->MapID;
            boost::thread t([mapId]() {WorldPosition::unloadMapAndVMaps(mapId); });
            t.detach();
        }

        return true;
    }
 


    map<string, ConsoleCommandHandler> handlers;
    handlers["init"] = &RandomPlayerbotMgr::RandomizeFirst;
    handlers["upgrade"] = &RandomPlayerbotMgr::UpdateGearSpells;
    handlers["refresh"] = &RandomPlayerbotMgr::Refresh;
    handlers["teleport"] = &RandomPlayerbotMgr::RandomTeleportForLevel;
    handlers["rpg"] = &RandomPlayerbotMgr::RandomTeleportForRpg;
    handlers["revive"] = &RandomPlayerbotMgr::Revive;
    handlers["grind"] = &RandomPlayerbotMgr::RandomTeleport;
    handlers["change_strategy"] = &RandomPlayerbotMgr::ChangeStrategy;

    for (map<string, ConsoleCommandHandler>::iterator j = handlers.begin(); j != handlers.end(); ++j)
    {
        string prefix = j->first;
        if (cmd.find(prefix) != 0) continue;
        string name = cmd.size() > prefix.size() + 1 ? cmd.substr(1 + prefix.size()) : "%";

        list<uint32> botIds;
        for (list<uint32>::iterator i = sPlayerbotAIConfig.randomBotAccounts.begin(); i != sPlayerbotAIConfig.randomBotAccounts.end(); ++i)
        {
            uint32 account = *i;
            if (auto results = CharacterDatabase.PQuery("SELECT guid FROM characters where account = '%u' and name like '%s'",
                    account, name.c_str()))
            {
                do
                {
                    Field* fields = results->Fetch();

                    uint32 botId = fields[0].GetUInt32();
                    ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, botId);
                    Player* bot = sObjectMgr.GetPlayer(guid);
                    if (!bot)
                        continue;

                    botIds.push_back(botId);
                } while (results->NextRow());
			}
        }

        if (botIds.empty())
        {
            sLog.outString("Nothing to do");
            return false;
        }

        int processed = 0;
        for (list<uint32>::iterator i = botIds.begin(); i != botIds.end(); ++i)
        {
            ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, *i);
            Player* bot = sObjectMgr.GetPlayer(guid);
            if (!bot)
                continue;

            sLog.outString("[%u/%zu] Processing command '%s' for bot '%s'",
                    processed++, botIds.size(), cmd.c_str(), bot->GetName());

            ConsoleCommandHandler handler = j->second;
            (sRandomPlayerbotMgr.*handler)(bot);
        }
        return true;
    }

    list<string> messages = sRandomPlayerbotMgr.HandlePlayerbotCommand(args, NULL);
    for (list<string>::iterator i = messages.begin(); i != messages.end(); ++i)
    {
        sLog.outString("%s",i->c_str());
    }
    return true;
}

void RandomPlayerbotMgr::HandleCommand(uint32 type, const string& text, Player& fromPlayer, string channelName, Team team, uint32 lang)
{
    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (!bot)
            continue;

        if (type == CHAT_MSG_SAY)
            if (bot->GetMapId() != fromPlayer.GetMapId() || sServerFacade.GetDistance2d(bot, &fromPlayer) > 25)
                continue;

        if (type == CHAT_MSG_YELL)
            if (bot->GetMapId() != fromPlayer.GetMapId() || sServerFacade.GetDistance2d(bot, &fromPlayer) > 300)
                continue;

        if (team != TEAM_BOTH_ALLOWED && bot->GetTeam() != team)
            continue;

        if (type == CHAT_MSG_GUILD && bot->GetGuildId() != fromPlayer.GetGuildId())
            continue;

        if (!channelName.empty())
        {
            if (ChannelMgr* cMgr = channelMgr(bot->GetTeam()))
            {
                Channel* chn = cMgr->GetChannel(channelName, bot);
                if (!chn)
                    continue;
            }
        }

        bot->GetPlayerbotAI()->HandleCommand(type, text, fromPlayer, lang);
    }
}

void RandomPlayerbotMgr::OnPlayerLogout(Player* player)
{
     DisablePlayerBot(player->GetGUIDLow());

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (player == ai->GetMaster())
        {
            ai->SetMaster(NULL);
            if (!bot->InBattleGround())
            {
                ai->ResetStrategies();
            }
        }
    }

    players.erase(player->GetGUIDLow());
}

void RandomPlayerbotMgr::OnBotLoginInternal(Player * const bot)
{
    sLog.outDetail("%lu/%d Bot %s logged in", playerBots.size(), sRandomPlayerbotMgr.GetMaxAllowedBotCount(), bot->GetName());
	//if (loginProgressBar && playerBots.size() < sRandomPlayerbotMgr.GetMaxAllowedBotCount()) { loginProgressBar->step(); }
	//if (loginProgressBar && playerBots.size() >= sRandomPlayerbotMgr.GetMaxAllowedBotCount() - 1) {
    //if (loginProgressBar && playerBots.size() + 1 >= sRandomPlayerbotMgr.GetMaxAllowedBotCount()) {
	//	sLog.outString("All bots logged in");
    //    delete loginProgressBar;
	//}
}

void RandomPlayerbotMgr::OnPlayerLogin(Player* player)
{
    uint32 botsNearby = 0;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (player == bot/* || player->GetPlayerbotAI()*/) // TEST
            continue;

        //if (player->GetCurrentCell() == bot->GetCurrentCell())
        //    botsNearby++;

        Group* group = bot->GetGroup();
        if (!group)
            continue;

        for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->getSource();
            PlayerbotAI* ai = bot->GetPlayerbotAI();
            if (member == player && (!ai->GetMaster() || ai->GetMaster()->GetPlayerbotAI()))
            {
                if (!bot->InBattleGround())
                {
                    ai->SetMaster(player);
                    ai->ResetStrategies();
                    ai->TellPlayer(ai->GetMaster(), BOT_TEXT("hello"));
                }
                break;
            }
        }
    }

    if (botsNearby > 100 && false)
    {
        WorldPosition botPos(player);

        //botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);
        //botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);
        //botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);

        //player->Relocate(botPos.getX(), botPos.getY(), botPos.getZ(), botPos.getO());
        //player->Relocate(botPos.coord_x, botPos.coord_y, botPos.coord_z, botPos.orientation);
        
        if (!player->GetFactionTemplateEntry())
        {
            botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);
            botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);
            botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig.reactDistance * 2, true);
        }
        else
        {
            vector<TravelDestination*> dests = sTravelMgr.getRpgTravelDestinations(player, true, true, 200000.0f);

            do
            {
                RpgTravelDestination* dest = (RpgTravelDestination*)dests[urand(0, dests.size() - 1)];
                CreatureInfo const* cInfo = dest->getCreatureInfo();
                if (!cInfo)
                    continue;

                FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
                ReputationRank reaction = PlayerbotAI::GetFactionReaction(player->GetFactionTemplateEntry(), factionEntry);

                if (reaction > REP_NEUTRAL && dest->nearestPoint(botPos)->mapid == player->GetMapId())
                {
                    botPos = *dest->nearestPoint(botPos);
                    break;
                }
            } while (true);
        }

        player->TeleportTo(botPos);
        //player->Relocate(botPos.getX(), botPos.getY(), botPos.getZ(), botPos.getO());
        
    }

    if (IsFreeBot(player))
    {
        uint32 guid = player->GetGUIDLow();
        if (sPlayerbotAIConfig.IsFreeAltBot(player))
            SetEventValue(guid, "always", 1, 0);
        else
           SetEventValue(guid, "login", 0, 0);
    }
    else
    {
        players[player->GetGUIDLow()] = player;
        sLog.outDebug("Including non-random bot player %s into random bot update", player->GetName());
    }
}

void RandomPlayerbotMgr::OnPlayerLoginError(uint32 bot)
{
    SetEventValue(bot, "add", 0, 0);
    SetEventValue(bot, "login", 0, 0);
    currentBots.remove(bot);
}

Player* RandomPlayerbotMgr::GetRandomPlayer()
{
    if (players.empty())
        return NULL;

    uint32 index = urand(0, players.size() - 1);
    return players[index];
}

Player* RandomPlayerbotMgr::GetPlayer(uint32 playerGuid)
{
    PlayerBotMap::const_iterator it = players.find(playerGuid);
    return (it == players.end()) ? nullptr : it->second ? it->second : nullptr;
}

void RandomPlayerbotMgr::PrintStats()
{
    sLog.outString("%lu Random Bots online", playerBots.size());

    map<uint32, int> alliance, horde;
    for (uint32 i = 0; i < 10; ++i)
    {
        alliance[i] = 0;
        horde[i] = 0;
    }

    map<uint8, int> perRace, perClass;
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        perRace[race] = 0;
    }
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        perClass[cls] = 0;
    }

    int dps = 0, heal = 0, tank = 0, active = 0, update = 0, randomize = 0, teleport = 0, changeStrategy = 0, dead = 0, combat = 0, revive = 0, taxi = 0, moving = 0, mounted = 0, afk = 0;
    int stateCount[(uint8)TravelState::MAX_TRAVEL_STATE + 1] = { 0 };
    vector<pair<Quest const*, int32>> questCount;
    for (PlayerBotMap::iterator i = playerBots.begin(); i != playerBots.end(); ++i)
    {
        Player* bot = i->second;
        if (IsAlliance(bot->getRace()))
            alliance[bot->GetLevel() / 10]++;
        else
            horde[bot->GetLevel() / 10]++;

        perRace[bot->getRace()]++;
        perClass[bot->getClass()]++;

        if (bot->GetPlayerbotAI()->AllowActivity())
            active++;

        if (bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<bool>("random bot update")->Get())
            update++;

        uint32 botId = bot->GetGUIDLow();
        if (!GetEventValue(botId, "randomize"))
            randomize++;

        if (!GetEventValue(botId, "teleport"))
            teleport++;

        if (!GetEventValue(botId, "change_strategy"))
            changeStrategy++;

        if (bot->IsTaxiFlying())
            taxi++;

        if (bot->IsMoving() && !bot->IsTaxiFlying() && !bot->IsFlying())
            moving++;

        if (bot->IsMounted() && !bot->IsTaxiFlying())
            mounted++;

        if (bot->IsInCombat())
            combat++;

        if (bot->isAFK())
            afk++;

        if (sServerFacade.UnitIsDead(bot))
        {
            dead++;
            //if (!GetEventValue(botId, "dead"))
            //    revive++;
        }

        int spec = AiFactory::GetPlayerSpecTab(bot);
        switch (bot->getClass())
        {
        case CLASS_DRUID:
            if (spec == 2)
                heal++;
            else
                dps++;
            break;
        case CLASS_PALADIN:
            if (spec == 1)
                tank++;
            else if (spec == 0)
                heal++;
            else
                dps++;
            break;
        case CLASS_PRIEST:
            if (spec != 2)
                heal++;
            else
                dps++;
            break;
        case CLASS_SHAMAN:
            if (spec == 2)
                heal++;
            else
                dps++;
            break;
        case CLASS_WARRIOR:
            if (spec == 2)
                tank++;
            else
                dps++;
            break;
#ifdef MANGOSBOT_TWO
        case CLASS_DEATH_KNIGHT:
            if (spec == 0)
                tank++;
            else
                dps++;
            break;
#endif
        default:
            dps++;
            break;
        }

        TravelTarget* target = bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
        if (target)
        {
            TravelState state = target->getTravelState();
            stateCount[(uint8)state]++;

            const Quest* quest = nullptr;
            if (target->getDestination())
            {
                quest = target->getDestination()->GetQuestTemplate();
            }

            if (quest)
            {
                bool found = false;
                for (auto& q : questCount)
                {
                    if (q.first != quest)
                    {
                        continue;
                    }

                    q.second++;
                    found = true;
                }

                if (!found)
                {
                    questCount.push_back(make_pair(quest, 1));
                }
            }
        }
    }

    sLog.outString("Bots level:");
	uint32 maxLevel = sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL);
	for (uint32 i = 0; i < 10; ++i)
    {
        if (!alliance[i] && !horde[i])
            continue;

        uint32 from = i*10;
        uint32 to = min(from + 9, maxLevel);
        if (!from) from = 1;
        sLog.outString("    %d..%d: %d alliance, %d horde", from, to, alliance[i], horde[i]);
    }
    sLog.outString("Bots race:");
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        if (perRace[race])
            sLog.outString("    %s: %d", ChatHelper::formatRace(race).c_str(), perRace[race]);
    }
    sLog.outString("Bots class:");
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        if (perClass[cls])
            sLog.outString("    %s: %d", ChatHelper::formatClass(cls).c_str(), perClass[cls]);
    }
    sLog.outString("Bots role:");
    sLog.outString("    tank: %d, heal: %d, dps: %d", tank, heal, dps);

    sLog.outString("Bots status:");
    sLog.outString("    Active: %d", active);
    sLog.outString("    Moving: %d", moving);
    
    //sLog.outString("Bots to:");
    //sLog.outString("    update: %d", update);
    //sLog.outString("    randomize: %d", randomize);
    //sLog.outString("    teleport: %d", teleport);
    //sLog.outString("    change_strategy: %d", changeStrategy);
    //sLog.outString("    revive: %d", revive);

    sLog.outString("    On taxi: %d", taxi);
    sLog.outString("    On mount: %d", mounted);
    sLog.outString("    In combat: %d", combat);
    sLog.outString("    Dead: %d", dead);
    sLog.outString("    AFK: %d", afk);

    sLog.outString("Bots questing:");
    sLog.outString("    Picking quests: %d", stateCount[(uint8)TravelState::TRAVEL_STATE_TRAVEL_PICK_UP_QUEST] + stateCount[(uint8)TravelState::TRAVEL_STATE_WORK_PICK_UP_QUEST]);
    sLog.outString("    Doing quests: %d", stateCount[(uint8)TravelState::TRAVEL_STATE_TRAVEL_DO_QUEST] + stateCount[(uint8)TravelState::TRAVEL_STATE_WORK_DO_QUEST]);
    sLog.outString("    Completing quests: %d", stateCount[(uint8)TravelState::TRAVEL_STATE_TRAVEL_HAND_IN_QUEST] + stateCount[(uint8)TravelState::TRAVEL_STATE_WORK_HAND_IN_QUEST]);
    sLog.outString("    Idling: %d", stateCount[(uint8)TravelState::TRAVEL_STATE_IDLE]);

    /*sort(questCount.begin(), questCount.end(), [](pair<Quest const*, int32> i, pair<Quest const*, int32> j) {return i.second > j.second; });

    sLog.outString("Bots top quests:");

    int cnt = 0;
    for (auto& quest : questCount)
    {
        sLog.outString("    [%d]: %s (%d)", quest.second, quest.first->GetTitle().c_str(), quest.first->GetQuestLevel());
        cnt++;
        if (cnt > 25)
            break;
    }*/
}

double RandomPlayerbotMgr::GetBuyMultiplier(Player* bot)
{
    uint32 id = bot->GetGUIDLow();
    uint32 value = GetEventValue(id, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        uint32 validIn = urand(sPlayerbotAIConfig.minRandomBotsPriceChangeInterval, sPlayerbotAIConfig.maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "buymultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

double RandomPlayerbotMgr::GetSellMultiplier(Player* bot)
{
    uint32 id = bot->GetGUIDLow();
    uint32 value = GetEventValue(id, "sellmultiplier");
    if (!value)
    {
        value = urand(80, 250);
        uint32 validIn = urand(sPlayerbotAIConfig.minRandomBotsPriceChangeInterval, sPlayerbotAIConfig.maxRandomBotsPriceChangeInterval);
        SetEventValue(id, "sellmultiplier", value, validIn);
    }

    return (double)value / 100.0;
}

void RandomPlayerbotMgr::AddTradeDiscount(Player* bot, Player* master, int32 value)
{
    if (!master) return;
    uint32 discount = GetTradeDiscount(bot, master);
    int32 result = (int32)discount + value;
    discount = (result < 0 ? 0 : result);

    SetTradeDiscount(bot, master, discount);
}

void RandomPlayerbotMgr::SetTradeDiscount(Player* bot, Player* master, uint32 value)
{
    if (!master) return;
    uint32 botId =  bot->GetGUIDLow();
    uint32 masterId =  master->GetGUIDLow();
    ostringstream name; name << "trade_discount_" << masterId;
    SetEventValue(botId, name.str(), value, sPlayerbotAIConfig.maxRandomBotInWorldTime);
}

uint32 RandomPlayerbotMgr::GetTradeDiscount(Player* bot, Player* master)
{
    if (!master) return 0;
    uint32 botId =  bot->GetGUIDLow();
    uint32 masterId = master->GetGUIDLow();
    ostringstream name; name << "trade_discount_" << masterId;
    return GetEventValue(botId, name.str());
}

string RandomPlayerbotMgr::HandleRemoteCommand(string request)
{
    string::iterator pos = find(request.begin(), request.end(), ',');
    if (pos == request.end())
    {
        ostringstream out; out << "invalid request: " << request;
        return out.str();
    }

    string command = string(request.begin(), pos);
    uint32 guid = atoi(string(pos + 1, request.end()).c_str());
    Player* bot = GetPlayerBot(guid);
    if (!bot)
        return "invalid guid";

    PlayerbotAI *ai = bot->GetPlayerbotAI();
    if (!ai)
        return "invalid guid";

    return ai->HandleRemoteCommand(command);
}

void RandomPlayerbotMgr::ChangeStrategy(Player* player)
{
    uint32 bot = player->GetGUIDLow();

    if (urand(0, 100) > 100 * sPlayerbotAIConfig.randomBotRpgChance) // select grind / pvp
    {
        sLog.outBasic("Bot #%d %s:%d <%s>: sent to grind spot", bot, player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName());
        // teleport in different places only if players are online
        RandomTeleportForLevel(player, players.size());
        ScheduleTeleport(bot);
    }
    else
    {
		sLog.outBasic("Bot #%d %s:%d <%s>: sent to inn", bot, player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName());
        RandomTeleportForRpg(player, players.size());
        ScheduleTeleport(bot);
    }
}

void RandomPlayerbotMgr::ChangeStrategyOnce(Player* player)
{
    uint32 bot = player->GetGUIDLow();

    if (urand(0, 100) > 100 * sPlayerbotAIConfig.randomBotRpgChance && players.size()) // select grind / pvp
    {
        sLog.outBasic("Bot #%d %s:%d <%s>: sent to grind spot", bot, player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName());
        // teleport in different places only if players are online
        RandomTeleportForLevel(player, true);
    }
    else
    {
        sLog.outBasic("Bot #%d %s:%d <%s>: sent to inn", bot, player->GetTeam() == ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName());
        RandomTeleportForRpg(player, players.size());
    }
}

void RandomPlayerbotMgr::RandomTeleportForRpg(Player* bot, bool activeOnly)
{
    uint32 race = bot->getRace();
    uint32 level = bot->GetLevel();
    sLog.outDetail("Random teleporting bot %s for RPG (%zu locations available)", bot->GetName(), rpgLocsCacheLevel[race][level].size());
    RandomTeleport(bot, rpgLocsCacheLevel[race][level], true, activeOnly);
    Refresh(bot);

    //Travel cooldown for 10 minutes.
    if (bot->GetPlayerbotAI())
    {
        AiObjectContext* context = bot->GetPlayerbotAI()->GetAiObjectContext();
        TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");

        travelTarget->setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition, true);
        travelTarget->setStatus(TravelStatus::TRAVEL_STATUS_COOLDOWN);
        travelTarget->setExpireIn(10 * MINUTE * IN_MILLISECONDS);
    }
}

void RandomPlayerbotMgr::Remove(Player* bot)
{
    uint32 owner = bot->GetGUIDLow();
    CharacterDatabase.PExecute("delete from ai_playerbot_random_bots where owner = 0 and bot = '%d'", owner);
    eventCache[owner].clear();

    LogoutPlayerBot(owner);
}

const CreatureDataPair* RandomPlayerbotMgr::GetCreatureDataByEntry(uint32 entry)
{
    if (entry != 0 && ObjectMgr::GetCreatureTemplate(entry))
    {
        FindCreatureData worker(entry, NULL);
        sObjectMgr.DoCreatureData(worker);
        CreatureDataPair const* dataPair = worker.GetResult();
        return dataPair;
    }
    return NULL;
}

uint32 RandomPlayerbotMgr::GetCreatureGuidByEntry(uint32 entry)
{
    uint32 guid = 0;

    CreatureDataPair const* dataPair = sRandomPlayerbotMgr.GetCreatureDataByEntry(entry);
    guid = dataPair->first;

    return guid;
}

uint32 RandomPlayerbotMgr::GetBattleMasterEntry(Player* bot, BattleGroundTypeId bgTypeId, bool fake)
{
    Team team = bot->GetTeam();
    uint32 entry = 0;
    vector<uint32> Bms;

    for (auto i = begin(BattleMastersCache[team][bgTypeId]); i != end(BattleMastersCache[team][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    for (auto i = begin(BattleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId]); i != end(BattleMastersCache[TEAM_BOTH_ALLOWED][bgTypeId]); ++i)
    {
        Bms.insert(Bms.end(), *i);
    }

    if (Bms.empty())
        return entry;

    float dist1 = FLT_MAX;

    for (auto i = begin(Bms); i != end(Bms); ++i)
    {
        CreatureDataPair const* dataPair = sRandomPlayerbotMgr.GetCreatureDataByEntry(*i);
        if (!dataPair)
            continue;

        CreatureData const* data = &dataPair->second;

        Unit* Bm = sMapMgr.FindMap((uint32)data->mapid)->GetUnit(ObjectGuid(HIGHGUID_UNIT, *i, dataPair->first));
        if (!Bm)
            continue;

        if (bot->GetMapId() != Bm->GetMapId())
            continue;

        // return first available guid on map if queue from anywhere
        if (fake)
        {
            entry = *i;
            break;
        }

        AreaTableEntry const* area = GetAreaEntryByAreaID(sServerFacade.GetAreaId(Bm));
        if (!area)
            continue;

        if (area->team == 4 && bot->GetTeam() == ALLIANCE)
            continue;
        if (area->team == 2 && bot->GetTeam() == HORDE)
            continue;

        if (Bm->GetDeathState() == DEAD)
            continue;

        float dist2 = sServerFacade.GetDistance2d(bot, data->posX, data->posY);
        if (dist2 < dist1)
        {
            dist1 = dist2;
            entry = *i;
        }
    }

    return entry;
}

void RandomPlayerbotMgr::Hotfix(Player* bot, uint32 version)
{
    PlayerbotFactory factory(bot, bot->GetLevel());
    uint32 exp = bot->GetUInt32Value(PLAYER_XP);
    uint32 level = bot->GetLevel();
    uint32 id = bot->GetGUIDLow();

    for (int fix = version; fix <= MANGOSBOT_VERSION; fix++)
    {
        int count = 0;
        switch (fix)
        {
            case 1: // Apply class quests to previously made random bots

                if (level < 10)
                {
                    break;
                }

                for (list<uint32>::iterator i = factory.classQuestIds.begin(); i != factory.classQuestIds.end(); ++i)
                {
                    uint32 questId = *i;
                    Quest const *quest = sObjectMgr.GetQuestTemplate(questId);

                    if (!bot->SatisfyQuestClass(quest, false) ||
                        quest->GetMinLevel() > bot->GetLevel() ||
                        !bot->SatisfyQuestRace(quest, false) || bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
                        continue;

                    bot->SetQuestStatus(questId, QUEST_STATUS_COMPLETE);
                    bot->RewardQuest(quest, 0, bot, false);
                    bot->SetLevel(level);
                    bot->SetUInt32Value(PLAYER_XP, exp);
                    sLog.outDetail("Bot %d rewarded quest %d",
                        bot->GetGUIDLow(), questId);
                    count++;
                }

                if (count > 0)
                {
                    sLog.outDetail("Bot %d hotfix (Class Quests), %d quests rewarded",
                        bot->GetGUIDLow(), count);
                    count = 0;
                }
                break;
            case 2: // Init Riding skill fix

                if (level < 20)
                {
                    break;
                }
                factory.InitSkills();
                sLog.outDetail("Bot %d hotfix (Riding Skill) applied",
                    bot->GetGUIDLow());
                break;

            default:
                break;
        }
    }
    SetValue(bot, "version", MANGOSBOT_VERSION);
    sLog.outBasic("Bot %d hotfix v%d applied",
        bot->GetGUIDLow(), MANGOSBOT_VERSION);
}

typedef std::unordered_map <uint32, list<float>> botPerformanceMetric;
std::unordered_map<string, botPerformanceMetric> botPerformanceMetrics;

void RandomPlayerbotMgr::PushMetric(botPerformanceMetric& metric, const uint32 bot, const float value, uint32 maxNum) const
{
    metric[bot].push_back(value);

    if (metric[bot].size() > maxNum)
        metric[bot].pop_front();
}

float RandomPlayerbotMgr::GetMetricDelta(botPerformanceMetric& metric) const
{
    float deltaMetric = 0;
    for (auto& botMetric : metric)
    {
        if (botMetric.second.size() > 1)
            deltaMetric += (botMetric.second.back() - botMetric.second.front()) / botMetric.second.size();
    }

    if (metric.empty())
        return 0;

    return deltaMetric / metric.size();
}