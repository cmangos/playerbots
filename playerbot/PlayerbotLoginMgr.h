#include "WorldPosition.h"
#include <shared_mutex>

struct LoginSpace
{
	int32 totalSpace;
	int32 classRaceBucket[MAX_CLASSES][MAX_RACES];
	int32 levelBucket[DEFAULT_MAX_LEVEL];
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

enum class Logoutreason : uint8
{
	KEEK_ONLINE = 0,
	NEXT_STEP = 1,
	ALL_POTENTIAL = 2
};

enum class LoginQueueReason : uint8
{
	ALLOWED = 0,
	ALREADY_ONLINE = 1,
	ALREADY_IN_QUEUE = 2,
	NO_HOLDER = 3,
	NO_TOTAL_SPACE = 4,
	NO_SPARE_SPACE = 5,
	NO_CLASS_RACE_SPACE = 6,
	NO_LEVEL_SPACE = 7,
	TIMED_OFFLINE = 8,
	OUT_OF_RANGE = 9
};

enum class LogoutQueueReason : uint8
{
	DISALLOWED = 0,
	ALREADY_OFFLINE = 1,
	ALREADY_IN_QUEUE = 2,
	MAKE_SPACE_IN_CLASSRACE_BUCKET = 3,
	MAKE_SPACE_IN_LEVEL_BUCKET = 4,
	TIMED_LOGOUT = 5,
	IN_RANGE = 6,
	MAKE_SPACE_OUT_OF_RANGE = 7
};

class PlayerBotInfo
{
public:
	PlayerBotInfo(uint32 account, uint32 guid, uint8 race, uint8 cls, uint32 level, bool isNew, WorldPosition position);

	ObjectGuid GetGuid() { return ObjectGuid(HIGHGUID_PLAYER, guid); }
	uint32 GetLevel();

	bool SendHolder();
	void HandlePlayerBotLoginCallback(QueryResult* /*dummy*/, SqlQueryHolder* holder);

	void FillLoginSpace(LoginSpace& space, FillStep step = FillStep::NOW);
	void EmptyLoginSpace(LoginSpace& space);

	LoginQueueReason AllowedToQueueLogin(LoginSpace& space, bool ignoreBuckets = false);
	LogoutQueueReason AllowedToQueueLogout(LoginSpace& space, bool ignoreBuckets = false);

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
