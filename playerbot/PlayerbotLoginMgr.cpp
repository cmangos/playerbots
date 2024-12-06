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

PlayerBotInfo::PlayerBotInfo(const uint32 account, const uint32 guid, const uint8 race, const uint8 cls, const uint32 level, const bool isNew, const WorldPosition& position) : account(account), guid(guid), race(race), cls(cls), level(level), isNew(isNew), position(position)
{

    holder = new PlayerbotLoginQueryHolder(&sRandomPlayerbotMgr, 0, account, guid);

    PlayerbotLoginQueryHolder* lqh = (PlayerbotLoginQueryHolder*)holder;

    if (!lqh->Initialize())
    {
        delete holder;                                      // delete all unprocessed queries
        return;
    }    
}

uint32 PlayerBotInfo::GetLevel() const
{
    if (!isNew || sPlayerbotAIConfig.disableRandomLevels)
        return level;

    uint32 minRandomLevel, maxRandomLevel;

    minRandomLevel = sPlayerbotAIConfig.randomBotMinLevel;

    maxRandomLevel = std::max(sPlayerbotAIConfig.randomBotMinLevel, std::min(sRandomPlayerbotMgr.GetPlayersLevel() + sPlayerbotAIConfig.syncLevelMaxAbove, sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL)));

    return (minRandomLevel + maxRandomLevel) / 2;
}

bool PlayerBotInfo::IsFarFromPlayer(const LoginSpace& space) const
{
    if (space.playerLocations.empty())
        return false;

    if (isNew && sPlayerbotAIConfig.instantRandomize) //We do not know where the bot will be teleported to on randomisation. 
        return false;

    for (auto& p : space.playerLocations)
    {
        if (p.mapid == position.mapid && p.sqDistance(position) < sPlayerbotAIConfig.loginBotsNearPlayerRange * sPlayerbotAIConfig.loginBotsNearPlayerRange)
        {
            return false;
        }
    }

    return true;
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

void PlayerBotInfo::FillLoginSpace(LoginSpace& space, FillStep step) const
{
    if (step == FillStep::NOW && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGINQUEUE))
        return;

    if (step == FillStep::NEXT_STEP && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGOUTQUEUE))
        return;

    if (loginState == LoginState::BOT_ONLINE)
        space.currentSpace--;

    space.totalSpace--;
    space.classRaceBucket[cls][race]--;
    space.levelBucket[GetLevel()]--;
};

void PlayerBotInfo::EmptyLoginSpace(LoginSpace& space, FillStep step) const
{
    if (step == FillStep::NOW && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGINQUEUE))
        return;

    if (step == FillStep::NEXT_STEP && (loginState == LoginState::BOT_OFFLINE || loginState == LoginState::BOT_ON_LOGOUTQUEUE))
        return;

    if (loginState == LoginState::BOT_ONLINE)
        space.currentSpace++;

    space.totalSpace++;
    space.classRaceBucket[cls][race]++;
    space.levelBucket[GetLevel()]++;
};

bool PlayerBotInfo::AllowedToQueueLogin(const LoginSpace& space) const
{
    if (loginState == LoginState::BOT_ONLINE)
        return false;
        
    if (loginState == LoginState::BOT_ON_LOGINQUEUE)
        return false;

    return true;
};

bool PlayerBotInfo::AllowedToQueueLogout(const LoginSpace& space) const
{
    if (loginState == LoginState::BOT_OFFLINE)
        return false;

    if (loginState == LoginState::BOT_ON_LOGOUTQUEUE)
        return false;

    return true;
}

LoginCriterionFailType PlayerBotInfo::MatchNoCriteria(const LoginSpace& space, const LoginCriteria& criteria) const
{
    for (auto& [criterionfail, criterion] : criteria)
        if (criterion(*this, space))
            return criterionfail;

    return LoginCriterionFailType::LOGIN_OK;
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
        EmptyLoginSpace(space, FillStep::NEXT_STEP);
        loginState = LoginState::BOT_ON_LOGOUTQUEUE;
        return true;
    }

    if (loginState == LoginState::BOT_ON_LOGINQUEUE)
    {
        EmptyLoginSpace(space, FillStep::NEXT_STEP);
        loginState = LoginState::BOT_OFFLINE;
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
    if (!stopThread && loginThread.joinable())
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

    for (uint32 level = 1; level <= GetMaxLevel(); ++level)
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

#define ADD_CRITERIA(type, condition) criteria.push_back(std::make_pair(LoginCriterionFailType::type, []( const PlayerBotInfo& info, const LoginSpace& space) {return condition;}))

LoginCriteria PlayerBotLoginMgr::GetLoginCriteria(const uint8 attempt) const
{
    LoginCriteria criteria;
    
    ADD_CRITERIA(MAX_BOTS, space.totalSpace <= int(0));

    if(attempt == 0)
        ADD_CRITERIA(LOGOUT_NOK, info.GetLoginState() == LoginState::BOT_ONLINE && space.currentSpace > (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots);
    else
        ADD_CRITERIA(SPARE_ROOM, info.GetLoginState() == LoginState::BOT_OFFLINE && space.totalSpace <= (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots);

    if (sPlayerbotAIConfig.randomBotTimedLogout)
        ADD_CRITERIA(RANDOM_TIMED_LOGOUT, info.GetLoginState() == LoginState::BOT_ONLINE && !sRandomPlayerbotMgr.GetValue(info.GetId(), "add"));

    if (sPlayerbotAIConfig.randomBotTimedOffline)
        ADD_CRITERIA(RANDOM_TIMED_OFFLINE, info.GetLoginState() == LoginState::BOT_OFFLINE && sRandomPlayerbotMgr.GetValue(info.GetId(), "logout"));

    std::vector<std::string> configCriteria = sPlayerbotAIConfig.loginCriteria;

    for (uint8 i = 0; i < std::min(uint8(configCriteria.size() - attempt), uint8(configCriteria.size())); i++)
    {
        if (configCriteria[i] == "classrace")
            ADD_CRITERIA(CLASSRACE, space.classRaceBucket[info.GetClass()][info.GetRace()] <= 0);
        if (configCriteria[i] == "level")
            ADD_CRITERIA(LEVEL, space.classRaceBucket[info.GetLevel()] <= 0);
        if (configCriteria[i] == "range")
            ADD_CRITERIA(RANGE, info.IsFarFromPlayer(space));
    }

    return criteria;
}

void PlayerBotLoginMgr::FillLoginSpace(LoginSpace& space, FillStep step)
{
    space.totalSpace = GetMaxOnlineBotCount();
    space.currentSpace = GetMaxOnlineBotCount();
    playerMutex.lock();
    space.playerLocations = playerLocations;
    playerMutex.unlock();

    for (uint32 level = 1; level <= GetMaxLevel(); ++level)
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

    std::unordered_map<uint8, std::unordered_map<LoginCriterionFailType, uint32>> loginFails;

    std::vector<PlayerBotInfo*> logins, logouts;

    for (uint8 attempt = 0; attempt < sPlayerbotAIConfig.loginCriteria.size(); attempt++)
    {
        LoginCriteria criteria = GetLoginCriteria(attempt);

        for (auto& [guid, botInfo] : botPool)
        {
            botInfo.EmptyLoginSpace(loginSpace, FillStep::NEXT_STEP); //Pretend the bot isn't logged in for a moment.
            LoginCriterionFailType LoginFail = botInfo.MatchNoCriteria(loginSpace, criteria);
            botInfo.FillLoginSpace(loginSpace, FillStep::NEXT_STEP);

            bool wantedOnline = (LoginFail == LoginCriterionFailType::LOGIN_OK || LoginFail == LoginCriterionFailType::LOGOUT_NOK);

            if (wantedOnline)
            {
                if (botInfo.AllowedToQueueLogin(loginSpace))
                    if (botInfo.QueueLogin(loginSpace))
                        logins.push_back(&botInfo);
            }
            else if (attempt == 0)
            {
                if (botInfo.AllowedToQueueLogout(loginSpace))
                    if (botInfo.QueueLogout(loginSpace))
                        logouts.push_back(&botInfo);
            }

            loginFails[attempt][LoginFail]++;

            if (attempt > 0 && loginSpace.totalSpace < (int32)sPlayerbotAIConfig.freeRoomForNonSpareBots)
                loginSpace.totalSpace = 0;
        }

        if (loginSpace.totalSpace <= 0)
            break;
    }

    std::lock_guard<std::mutex> guard(loginMutex);

    for (auto& logout : logouts)
        logoutQueue.push(logout);

    for (auto& login : logins)
        loginQueue.push(login);

    for (auto& [attempt, fails] : loginFails)
    {
        for (auto& [fail, nr] : fails)
        {
            sLog.outDebug("PlayerbotLoginMgr attempt: %d fail %s: %d", attempt, failName.find(fail)->second.c_str(), nr);
        }
    }

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
        else if(loginQueue.front()->GetLoginState() == LoginState::BOT_ON_LOGINQUEUE)
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

