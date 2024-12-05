#include "PlayerbotLoginMgr.h"
#include "Database/DatabaseImpl.h"

class LoginQueryHolder : public SqlQueryHolder
{
private:
    uint32 m_accountId;
    ObjectGuid m_guid;
public:
    LoginQueryHolder(uint32 accountId, ObjectGuid guid)
        : m_accountId(accountId), m_guid(guid) {
    }
    ObjectGuid GetGuid() const { return m_guid; }
    uint32 GetAccountId() const { return m_accountId; }
    bool Initialize();
};

class PlayerbotLoginQueryHolder : public LoginQueryHolder
{
private:
    uint32 masterAccountId;
    PlayerbotHolder* playerbotHolder;

public:
    PlayerbotLoginQueryHolder(PlayerbotHolder* playerbotHolder, uint32 masterAccount, uint32 accountId, uint32 guid)
        : LoginQueryHolder(accountId, ObjectGuid(HIGHGUID_PLAYER, guid)), masterAccountId(masterAccount), playerbotHolder(playerbotHolder) {
    }

public:
    uint32 GetMasterAccountId() const { return masterAccountId; }
    PlayerbotHolder* GetPlayerbotHolder() { return playerbotHolder; }
};

PlayerBotInfo::PlayerBotInfo(uint32 account, uint32 guid, uint8 race, uint8 cls, uint32 level, bool isNew, WorldPosition position) : account(account), guid(guid), race(race), cls(cls), level(level), isNew(isNew), position(position)
{

    holder = new PlayerbotLoginQueryHolder(&sRandomPlayerbotMgr, 0, account, guid);

    PlayerbotLoginQueryHolder* lqh = (PlayerbotLoginQueryHolder*)holder;

    if (!lqh->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return;
    }    
}

uint32 PlayerBotInfo::GetLevel()
{
    if (!isNew || sPlayerbotAIConfig.disableRandomLevels)
        return level;

    uint32 minRandomLevel, maxRandomLevel;

    minRandomLevel = sPlayerbotAIConfig.randomBotMinLevel;

    maxRandomLevel = std::max(sPlayerbotAIConfig.randomBotMinLevel, std::min(sRandomPlayerbotMgr.GetPlayersLevel() + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));

    return (minRandomLevel + maxRandomLevel) / 2;
}

bool PlayerBotInfo::SendHolder()
{
    if (holderState == HolderState::HOLDER_SENT)
        return true;

    if (holderState == HolderState::HOLDER_RECEIVED)
        return false;

    if (loginState == LoginState::BOT_ONLINE)
        return false;

    if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
        return false;

    sRandomPlayerbotMgr.GetValue(guid, "logout");

    holderState = HolderState::HOLDER_SENT;

    if (holder)
        delete holder;

    holder = nullptr; 

    holder = new PlayerbotLoginQueryHolder(&sRandomPlayerbotMgr, 0, account, guid);

    PlayerbotLoginQueryHolder* lqh = (PlayerbotLoginQueryHolder*)holder;

    if (!lqh->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return false;
    }

    CharacterDatabase.DelayQueryHolder(this, &PlayerBotInfo::HandlePlayerBotLoginCallback, holder);

    return true;
}

void PlayerBotInfo::HandlePlayerBotLoginCallback(QueryResult* /*dummy*/, SqlQueryHolder* holder)
{
    if (!holder)
    {
        holderState = HolderState::HOLDER_EMPTY;
        return;
    }
  
    holderState = HolderState::HOLDER_RECEIVED;
}

void PlayerBotInfo::FillLoginSpace(LoginSpace& space, FillStep step)
{
    if (step == FillStep::NOW && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGINQUEUE))
        return;

    if (step == FillStep::NEXT_STEP && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGOUTQUEUE))
        return;

    space.totalSpace--;
    space.classRaceBucket[cls][race]--;
    space.levelBucket[GetLevel()]--;
};

void PlayerBotInfo::EmptyLoginSpace(LoginSpace& space)
{
    space.totalSpace++;
    space.classRaceBucket[cls][race]++;
    space.levelBucket[GetLevel()]++;
};

LoginQueueReason PlayerBotInfo::AllowedToQueueLogin(LoginSpace& space, bool ignoreBuckets)
{
    if (loginState == LoginState::BOT_ONLINE)
        return LoginQueueReason::ALREADY_ONLINE;
        
    if (loginState == LoginState::BOT_ON_LOGINQUEUE)
        return LoginQueueReason::ALREADY_IN_QUEUE;

    //if (holderState != HolderState::HOLDER_RECEIVED)
    //    return LoginQueueReason::NO_HOLDER;

    if (space.totalSpace <= 0)
        return LoginQueueReason::NO_TOTAL_SPACE;

    if (ignoreBuckets && space.totalSpace <= (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots)
        return LoginQueueReason::NO_SPARE_SPACE;

    if (!ignoreBuckets && space.classRaceBucket[cls][race] <= 0)
        return LoginQueueReason::NO_CLASS_RACE_SPACE;

    if (!ignoreBuckets && space.levelBucket[GetLevel()] <= 0)
        return LoginQueueReason::NO_LEVEL_SPACE;

    if (sPlayerbotAIConfig.randomBotTimedOffline && sRandomPlayerbotMgr.GetValue(guid, "logout"))
        return LoginQueueReason::TIMED_OFFLINE;

    if (!ignoreBuckets && sPlayerbotAIConfig.loginBotsNearPlayerRange && space.playerLocations.size())
    {
        bool inRange = false;
        for (auto& p : space.playerLocations)
        {
            if (p.sqDistance(position) < sPlayerbotAIConfig.loginBotsNearPlayerRange * sPlayerbotAIConfig.loginBotsNearPlayerRange)
            {
                inRange = true;
                break;
            }
        }

        if(!inRange)
            return LoginQueueReason::OUT_OF_RANGE;
    }

    return LoginQueueReason::ALLOWED;
};

LogoutQueueReason PlayerBotInfo::AllowedToQueueLogout(LoginSpace& space, bool ignoreBuckets)
{
    if (loginState == LoginState::BOT_OFFLINE)
        return LogoutQueueReason::ALREADY_OFFLINE;

    if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
        return LogoutQueueReason::ALREADY_IN_QUEUE;

    if (ignoreBuckets && space.totalSpace <= (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots)
    {
        if (space.classRaceBucket[cls][race] <= 0)
            return LogoutQueueReason::MAKE_SPACE_IN_CLASSRACE_BUCKET;

        if (space.levelBucket[GetLevel()] <= 0)
            return LogoutQueueReason::MAKE_SPACE_IN_LEVEL_BUCKET;
    }

    if (loginState == LoginState::BOT_ONLINE && sPlayerbotAIConfig.randomBotTimedLogout)
        if (!sRandomPlayerbotMgr.GetValue(guid, "add"))
            return LogoutQueueReason::TIMED_LOGOUT;

    if (sPlayerbotAIConfig.loginBotsNearPlayerRange && space.playerLocations.size())
    {
        bool inRange = false;
        for (auto& p : space.playerLocations)
        {
            if (p.sqDistance(position) < sPlayerbotAIConfig.loginBotsNearPlayerRange * sPlayerbotAIConfig.loginBotsNearPlayerRange)
            {
                inRange = true;
                break;
            }
        }

        if (inRange)
            return LogoutQueueReason::IN_RANGE;

        if (!ignoreBuckets || space.totalSpace <= (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots)
            return LogoutQueueReason::MAKE_SPACE_OUT_OF_RANGE;
    }

    return LogoutQueueReason::DISALLOWED;
}

bool PlayerBotInfo::QueueLogin(LoginSpace& space)
{
    if (loginState == LoginState::BOT_OFFLINE)
    {
        loginState = LoginState::BOT_ON_LOGINQUEUE;
        FillLoginSpace(space, FillStep::NEXT_STEP);
        return true;
    }
        
    if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
    {
        loginState = LoginState::BOT_ONLINE;
        FillLoginSpace(space, FillStep::NEXT_STEP);
        return false;
    }

    return false;
}

bool PlayerBotInfo::QueueLogout(LoginSpace& space)
{
    if (loginState == LoginState::BOT_ONLINE)
    {
        loginState = LoginState::BOT_ON_LOGOUTQUEUE;
        EmptyLoginSpace(space);
        return true;
    }

    if (loginState == LoginState::BOT_ON_LOGINQUEUE)
    {
        loginState = LoginState::BOT_OFFLINE;
        EmptyLoginSpace(space);
        return false;
    }

    return false;
}

bool PlayerBotInfo::LoginBot()
{
    if (loginState != LoginState::BOT_ON_LOGINQUEUE)
        return false;

    if (holderState != HolderState::HOLDER_RECEIVED)
        return false;

    if (sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false))
    {
        loginState = LoginState::BOT_ONLINE;
        return false;
    }

    sRandomPlayerbotMgr.HandlePlayerBotLoginCallback(nullptr, holder);
    holderState = HolderState::HOLDER_EMPTY;
    holder = nullptr;

    if(sPlayerbotAIConfig.randomBotTimedLogout)
        sRandomPlayerbotMgr.SetValue(guid, "add", 1, "", urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));

    Player* player = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false);

    if (!player)
    {
        loginState = LoginState::BOT_OFFLINE;
        return false;
    }

    loginState = LoginState::BOT_ONLINE;

    Update(player);

    return true;
}

bool PlayerBotInfo::LogoutBot()
{
    if (loginState != LoginState::BOT_ON_LOGOUTQUEUE)
        return false;

    Player* player = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false);

    if (!player)
    {
        loginState = LoginState::BOT_OFFLINE;
        return false;
    }

    Update(player);

    sRandomPlayerbotMgr.SetValue(guid, "add", 0, "", 0);

    sRandomPlayerbotMgr.LogoutPlayerBot(guid);

    if (sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid), false))
        return false;

    loginState = LoginState::BOT_OFFLINE;

    if (sPlayerbotAIConfig.randomBotTimedOffline)
        sRandomPlayerbotMgr.SetValue(guid, "logout", 1, "", urand(sPlayerbotAIConfig.minRandomBotInWorldTime, sPlayerbotAIConfig.maxRandomBotInWorldTime));

    return true;
}

void PlayerBotInfo::Update(Player* player)
{
    level = player->GetLevel();
    position = WorldPosition(player);
    isNew = ((level > 1) ? false : (player->GetTotalPlayedTime() == 0));
}

void PlayerBotLoginMgr::LoadBotsFromDb()
{
    StopThread();

    loginMutex.lock();
    botPool.clear();
    std::set<uint32> accounts;
    std::string prefixString = sPlayerbotAIConfig.randomBotAccountPrefix + "%";
    auto result = LoginDatabase.PQuery("SELECT id FROM account where UPPER(username) like UPPER('%s')", prefixString.c_str());
    if (!result)
    {
        loginMutex.unlock();
        return;
    }

    do
    {
    Field* fields = result->Fetch();
    uint32 accountId = fields[0].GetUInt32();
    accounts.insert(accountId);
    } while (result->NextRow());

    sLog.outDebug("PlayerbotLoginMgr: %d accounts found.", uint32(accounts.size()));

    result = CharacterDatabase.PQuery("SELECT account, guid, race, class, level, totaltime, map, position_x, position_y, position_z, orientation FROM characters");
         
    do
    {
        Field* fields = result->Fetch();
        uint32 account = fields[0].GetUInt32();

        if (accounts.find(account) == accounts.end())
            continue;

        uint32 guid = fields[1].GetUInt32();
        uint32 race = fields[2].GetUInt8();
        uint32 cls = fields[3].GetUInt8();
        uint32 level = fields[4].GetUInt32();
        bool isNew = sPlayerbotAIConfig.instantRandomize ? (fields[5].GetUInt32() == 0) : level == 1;
        WorldPosition position(fields[6].GetFloat(), fields[7].GetFloat(), fields[8].GetFloat(), fields[9].GetFloat(), fields[10].GetFloat());
        botPool.insert(std::make_pair(guid, PlayerBotInfo(account, guid, race, cls, level, isNew, position)));;
    } while (result->NextRow());

    sLog.outDebug("PlayerbotLoginMgr: %d bots found.", uint32(botPool.size()));

    loginMutex.unlock();

    StartThread();
}

void PlayerBotLoginMgr::StopThread()
{
    if (!stopThread)
    {
        stopThread = true;
        loginThread.join();
    }
}

void PlayerBotLoginMgr::StartThread()
{
    if (stopThread)
    {
        stopThread = false;
        loginThread = std::thread(&PlayerBotLoginMgr::Update, this);
        loginThread.detach();
    }
}

void PlayerBotLoginMgr::Update()
{
    while (!stopThread)
    {
        FillLoginLogoutQueue();
        SendHolders(sPlayerbotAIConfig.randomBotsMaxLoginsPerInterval);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (showSpace)
        {
            ShowSpace();
            showSpace = false;
        }
    }

    stopThread = false;
}

void PlayerBotLoginMgr::SetPlayerLocations(std::map<uint32, Player*> players)
{
    std::vector<WorldPosition> positions;
    for (auto& [guid, player] : players)
        positions.push_back(player);

    playerMutex.lock();
    playerLocations = positions;
    playerMutex.unlock();
}

void PlayerBotLoginMgr::SendHolders(uint32 amount)
{
    uint32 sent = 0;

    CharacterDatabase.AsyncPQuery(&RandomPlayerbotMgr::DatabasePing, sWorld.GetCurrentMSTime(), std::string("CharacterDatabase"), "select 1 from dual");

    if (sRandomPlayerbotMgr.GetDatabaseDelay("CharacterDatabase") > 100)
        return;

    if (!loginMutex.try_lock())
        return;

    std::queue<PlayerBotInfo*> queue = loginQueue;

    loginMutex.unlock();

    while (!queue.empty() && sent < amount && !stopThread) {
        if (queue.front()->SendHolder())
            sent++;
            queue.pop();
    }

    /*
    for (auto& [guid, botInfo] : botPool)
    {
        sent += botInfo.SendHolder();

        if (sent > amount || stopThread)
            break;
    }
    */

    sLog.outDebug("PlayerbotLoginMgr: %d botinfos sent.", sent);
}

void PlayerBotLoginMgr::ShowSpace()
{
    LoginSpace onlineSpace, offlineSpace;

    FillLoginSpace(onlineSpace, FillStep::NOW);
    FillLoginSpace(offlineSpace, FillStep::ALL_POTENTIAL);

    int32 currentOnline = GetMaxOnlineBotCount() - onlineSpace.totalSpace;
    int32 currentOffline = GetMaxOnlineBotCount() - offlineSpace.totalSpace;

    sLog.outError("Bots online %d/%d (%d generated). Log in:%d out:%d", currentOnline, GetMaxOnlineBotCount(), currentOffline, uint32(loginQueue.size()), uint32(logoutQueue.size()));

    if (onlineSpace.totalSpace < 0)
        sLog.outError("Too many bots online (%d/%d).", currentOnline, GetMaxOnlineBotCount());

    if (offlineSpace.totalSpace > 0)
        sLog.outError("Too few bots generated (%d/%d).", currentOffline, GetMaxOnlineBotCount());

    for (uint32 level = 1; level < GetMaxLevel(); ++level)
    {
        if (onlineSpace.levelBucket[level] < 0)
        {
            int32 current = GetLevelBucketSize(level) - onlineSpace.levelBucket[level];
            sLog.outError("Too many level %d bots online (%d/%d).", level, current, GetLevelBucketSize(level));
        }

        if (offlineSpace.levelBucket[level] > 0)
        {
            int32 current = GetLevelBucketSize(level) - offlineSpace.levelBucket[level];
            sLog.outError("Too few level %d bots generated (%d/%d).", level, current, GetLevelBucketSize(level));
        }
    }

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            if (onlineSpace.classRaceBucket[cls][race] < 0)
            {
                int32 current = GetClassRaceBucketSize(cls, race) - onlineSpace.classRaceBucket[cls][race];
                sLog.outError("Too many %s %s bots online (%d/%d).", ChatHelper::formatRace(race).c_str(), ChatHelper::formatClass(race).c_str(), current, GetClassRaceBucketSize(cls, race));
            }

            if (offlineSpace.classRaceBucket[cls][race] > 0)
            {
                int32 current = GetClassRaceBucketSize(cls, race) - offlineSpace.classRaceBucket[cls][race];
                sLog.outError("Too few %s %s bots generated (%d/%d).", ChatHelper::formatRace(race).c_str(), ChatHelper::formatClass(race).c_str(), current, GetClassRaceBucketSize(cls, race));
            }
        }
    }
}

void PlayerBotLoginMgr::FillLoginSpace(LoginSpace& space, FillStep step)
{
    space.totalSpace = GetMaxOnlineBotCount();
    playerMutex.lock();
    space.playerLocations = playerLocations;
    playerMutex.unlock();

    for (uint32 level = 1; level < GetMaxLevel(); ++level)
    {
        space.levelBucket[level] = GetLevelBucketSize(level);
    }

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            space.classRaceBucket[cls][race] = GetClassRaceBucketSize(cls, race);
        }
    }

    for (auto& [guid, botInfo] : botPool)
    {
        botInfo.FillLoginSpace(space, FillStep::NEXT_STEP);
    }   
}

void PlayerBotLoginMgr::FillLoginLogoutQueue()
{
    LoginSpace loginSpace;
    FillLoginSpace(loginSpace, FillStep::NEXT_STEP);

    std::vector<PlayerBotInfo*> logins, logouts;

    for (auto& [guid, botInfo] : botPool)
    {
        LogoutQueueReason outReason = botInfo.AllowedToQueueLogout(loginSpace);

        if (outReason == LogoutQueueReason::MAKE_SPACE_IN_CLASSRACE_BUCKET || outReason == LogoutQueueReason::MAKE_SPACE_IN_LEVEL_BUCKET || outReason == LogoutQueueReason::TIMED_LOGOUT || outReason == LogoutQueueReason::MAKE_SPACE_OUT_OF_RANGE)
        {
            if (botInfo.QueueLogout(loginSpace))
                logouts.push_back(&botInfo);
        }

        LoginQueueReason inReason = botInfo.AllowedToQueueLogin(loginSpace, true);

        if (inReason == LoginQueueReason::ALLOWED)
        {
            if (botInfo.QueueLogin(loginSpace))
                logins.push_back(&botInfo);
        }
    }


    if (sPlayerbotAIConfig.freeRoomForNonSpareBots)
    {
        for (auto& [guid, botInfo] : botPool)
        {
            LogoutQueueReason outReason = botInfo.AllowedToQueueLogout(loginSpace, true);



            if (outReason == LogoutQueueReason::MAKE_SPACE_IN_CLASSRACE_BUCKET || outReason == LogoutQueueReason::MAKE_SPACE_IN_LEVEL_BUCKET || outReason == LogoutQueueReason::TIMED_LOGOUT || outReason == LogoutQueueReason::MAKE_SPACE_OUT_OF_RANGE)
            {
                if (botInfo.QueueLogout(loginSpace))
                    logouts.push_back(&botInfo);
            }

            LoginQueueReason inReason = botInfo.AllowedToQueueLogin(loginSpace, true);

            if (inReason == LoginQueueReason::ALLOWED)
            {
                if (botInfo.QueueLogin(loginSpace))
                    logins.push_back(&botInfo);
            }
        }
    }

    std::lock_guard<std::mutex> guard(loginMutex);

    for (auto& logout : logouts)
        logoutQueue.push(logout);

    for (auto& login : logins)
        loginQueue.push(login);

    sLog.outDebug("PlayerbotLoginMgr: Queued to log in: %d, out: %d", uint32(loginQueue.size()), uint32(logoutQueue.size()));
}

void PlayerBotLoginMgr::LogoutBots(uint32 maxLogouts)
{    
    std::queue<PlayerBotInfo*> queue;

    if (!loginMutex.try_lock())
        return;  

    uint32 sent = 0;
    while (!logoutQueue.empty() && sent < maxLogouts) {
        if (logoutQueue.front()->LogoutBot())
            sent++;
        logoutQueue.pop();
    }

    loginMutex.unlock();
}

void PlayerBotLoginMgr::LoginBots(uint32 maxLogins)
{
    std::queue<PlayerBotInfo*> retryQueue;

    if (!loginMutex.try_lock())
        return;

    uint32 sent = 0;
    while (!loginQueue.empty() && sent < maxLogins) {
        if (loginQueue.front()->LoginBot())
            sent++;
        else
            retryQueue.push(loginQueue.front());
        loginQueue.pop();
    }

    while (!loginQueue.empty())
    {
        retryQueue.push(loginQueue.front());
        loginQueue.pop();
    }
    std::swap(retryQueue, loginQueue);

    loginMutex.unlock();
}

uint32 PlayerBotLoginMgr::GetMaxLevel() const
{
    return std::max(sPlayerbotAIConfig.randomBotMinLevel, std::min(sRandomPlayerbotMgr.GetPlayersLevel() + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));
}

uint32 PlayerBotLoginMgr::GetMaxOnlineBotCount() const
{
    return sRandomPlayerbotMgr.GetValue(uint32(0), "bot_count");
}

uint32 PlayerBotLoginMgr::GetClassRaceBucketSize(uint8 cls, uint8 race) const
{
    uint32 prob = sPlayerbotAIConfig.classRaceProbability[cls][race];

    if (prob == 0)
        return 0;

    return GetMaxOnlineBotCount() * sPlayerbotAIConfig.classRaceProbability[cls][race] / sPlayerbotAIConfig.classRaceProbabilityTotal;
}

uint32 PlayerBotLoginMgr::GetLevelBucketSize(uint32 level) const
{
    uint32 prob = sPlayerbotAIConfig.levelProbability[level];

    if (prob == 0 || level > GetMaxLevel())
        return 0;

    uint32 levelProbabilityTotal = 0;
    for (uint32 level = 1; level < GetMaxLevel(); ++level)
    {
        levelProbabilityTotal += sPlayerbotAIConfig.levelProbability[level];
    }

    return GetMaxOnlineBotCount() * sPlayerbotAIConfig.levelProbability[level] / levelProbabilityTotal;
}

