#ifndef _PLAYERBOTMGR_H
#define _PLAYERBOTMGR_H

#include "Common.h"
#include "PlayerbotAIBase.h"


class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

typedef map<uint32, Player*> PlayerBotMap;
typedef map<string, set<string> > PlayerBotErrorMap;

class PlayerbotHolder : public PlayerbotAIBase
{
public:
    PlayerbotHolder();
    virtual ~PlayerbotHolder();

    void AddPlayerBot(uint32 guid, uint32 masterAccountId);
	void HandlePlayerBotLoginCallback(QueryResult * dummy, SqlQueryHolder * holder);

    void LogoutPlayerBot(uint32 guid);
    void DisablePlayerBot(uint32 guid);
    Player* GetPlayerBot (uint32 guid) const;
    PlayerBotMap::const_iterator GetPlayerBotsBegin() const { return playerBots.begin(); }
    PlayerBotMap::const_iterator GetPlayerBotsEnd()   const { return playerBots.end();   }

    virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false);
    void UpdateSessions(uint32 elapsed);

    void LogoutAllBots();
    void OnBotLogin(Player* bot);
    void MovePlayerBot(uint32 guid, PlayerbotHolder* newHolder) { auto botptr = playerBots.find(guid); if (botptr == playerBots.end()) return;  newHolder->OnBotLogin(botptr->second); playerBots.erase(guid); }

    list<string> HandlePlayerbotCommand(char const* args, Player* master = NULL);
    string ProcessBotCommand(string cmd, ObjectGuid guid, ObjectGuid masterguid, bool admin, uint32 masterAccountId, uint32 masterGuildId);
    uint32 GetAccountId(string name);
    string ListBots(Player* master);

protected:
    virtual void OnBotLoginInternal(Player * const bot) = 0;

protected:
    PlayerBotMap playerBots;
};

class PlayerbotMgr : public PlayerbotHolder
{
public:
    PlayerbotMgr(Player* const master);
    virtual ~PlayerbotMgr();

    static bool HandlePlayerbotMgrCommand(ChatHandler* handler, char const* args);
    void HandleMasterIncomingPacket(const WorldPacket& packet);
    void HandleMasterOutgoingPacket(const WorldPacket& packet);
    void HandleCommand(uint32 type, const string& text, uint32 lang = LANG_UNIVERSAL);
    void OnPlayerLogin(Player* player);
    void CancelLogout();

    virtual void UpdateAIInternal(uint32 elapsed, bool minimal = false);
    void TellError(string botName, string text);

    Player* GetMaster() const { return master; };

    void SaveToDB();

protected:
    virtual void OnBotLoginInternal(Player * const bot);
    void CheckTellErrors(uint32 elapsed);

private:
    Player* const master;
    PlayerBotErrorMap errors;
    time_t lastErrorTell;
};

#endif
