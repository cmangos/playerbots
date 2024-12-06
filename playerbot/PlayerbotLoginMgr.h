#include "WorldPosition.h"

struct LoginSpace
{
	int32 currentSpace;
	int32 totalSpace;
	int32 classRaceBucket[MAX_CLASSES][MAX_RACES];
	int32 levelBucket[DEFAULT_MAX_LEVEL + 1];
	std::vector<WorldPosition> playerLocations;
};

enum class HolderState : uint8
{
	HOLDER_EMPTY = 0,
	HOLDER_SENT = 1,
	HOLDER_RECEIVED = 2
};

enum class LoginState : uint8
{
	BOT_OFFLINE = 0,
	BOT_ON_LOGINQUEUE = 1,
	BOT_ONLINE = 2,
	BOT_ON_LOGOUTQUEUE = 3
};

enum class FillStep : uint8
{
	NOW = 0,
	NEXT_STEP = 1,
	ALL_POTENTIAL = 2
};

enum class LoginCriterionFailType : uint8
{
	LOGIN_OK = 0,
	LOGOUT_NOK = 1,
	MAX_BOTS = 2,
	SPARE_ROOM = 3,
	RANDOM_TIMED_LOGOUT = 4,
	RANDOM_TIMED_OFFLINE = 5,
	CLASSRACE = 6,
	LEVEL = 7,
	RANGE = 8
};

static const std::unordered_map<LoginCriterionFailType, std::string> failName = {
 	 {LoginCriterionFailType::LOGIN_OK, "LOGIN_OK"}
	,{LoginCriterionFailType::LOGOUT_NOK, "LOGOUT_NOK" }
    ,{LoginCriterionFailType::MAX_BOTS, "MAX_BOTS"}
	,{LoginCriterionFailType::SPARE_ROOM, "SPARE_ROOM"}
	,{LoginCriterionFailType::RANDOM_TIMED_LOGOUT, "RANDOM_TIMED_LOGOUT"}
	,{LoginCriterionFailType::RANDOM_TIMED_OFFLINE , "RANDOM_TIMED_OFFLINE"}
	,{LoginCriterionFailType::CLASSRACE, "CLASSRACE"}
	,{LoginCriterionFailType::LEVEL, "LEVEL"}
	,{LoginCriterionFailType::RANGE , "RANGE"} };

class PlayerBotInfo;

typedef std::vector <std::pair<LoginCriterionFailType, std::function<bool(const PlayerBotInfo&, const LoginSpace&)>>> LoginCriteria;

class PlayerBotInfo
{
public:
	PlayerBotInfo(const uint32 account, const uint32 guid, const uint8 race, const uint8 cls, const uint32 level, const bool isNew, const WorldPosition& position);

	ObjectGuid GetGuid() const { return ObjectGuid(HIGHGUID_PLAYER, guid); }
	uint32 GetId() const { return guid; }
	uint8 GetRace() const { return race; }
	uint8 GetClass() const { return cls; }
	uint32 GetLevel() const;
	bool IsFarFromPlayer(const LoginSpace& space) const;
	LoginState GetLoginState() const { return loginState; }

	bool SendHolder();
	void HandlePlayerBotLoginCallback(QueryResult* /*dummy*/, SqlQueryHolder* holder);

	void FillLoginSpace(LoginSpace& space, FillStep step = FillStep::NOW) const;
	void EmptyLoginSpace(LoginSpace& space, FillStep step = FillStep::NOW) const;

	bool AllowedToQueueLogin(const LoginSpace& space) const;
	bool AllowedToQueueLogout(const LoginSpace& space) const;
	LoginCriterionFailType MatchNoCriteria(const LoginSpace& space, const LoginCriteria& criteria) const;

	bool QueueLogin(LoginSpace& space);
	bool QueueLogout(LoginSpace& space);

	bool LoginBot();
	bool LogoutBot();
private:
	void Update(Player* player);

	uint32 account;
	uint32 guid;
	uint8 race;
	uint8 cls;
	uint32 level;
	bool isNew = false;
	WorldPosition position;

	SqlQueryHolder* holder;
	HolderState holderState = HolderState::HOLDER_EMPTY;
	LoginState loginState = LoginState::BOT_OFFLINE;
};

class PlayerBotLoginMgr
{
public:	
	~PlayerBotLoginMgr() { StopThread(); }
	void LoadBotsFromDb();
	void StopThread();
	void StartThread();

	void Update();

	void SetShowSpace() { showSpace = true; }
	void SetPlayerLocations(std::map<uint32, Player*> players);

	void LogoutBots(uint32 maxLogouts);
	void LoginBots(uint32 maxLogins);

	void SendHolders(uint32 amount);
private:
	void ShowSpace();
	void FillLoginLogoutQueue();

	LoginCriteria GetLoginCriteria(const uint8 attempt) const;
	void FillLoginSpace(LoginSpace& space, FillStep step);
	uint32 GetMaxOnlineBotCount() const;
	uint32 GetMaxLevel() const;
	uint32 GetClassRaceBucketSize(uint8 cls, uint8 race) const;
	uint32 GetLevelBucketSize(uint32 level) const;

	std::thread loginThread;
	bool stopThread = true;
	bool showSpace = false;

	std::mutex playerMutex;
	std::vector<WorldPosition> playerLocations;

	std::mutex loginMutex;
	std::queue<PlayerBotInfo*> loginQueue;
	std::queue<PlayerBotInfo*> logoutQueue;
	std::map<uint32, PlayerBotInfo> botPool;
};

#define sPlayerBotLoginMgr MaNGOS::Singleton<PlayerBotLoginMgr>::Instance()
