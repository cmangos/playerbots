#ifndef _RandomPlayerbotFactory_H
#define _RandomPlayerbotFactory_H

#include "Common.h"
#include "PlayerbotAIBase.h"

class WorldPacket;
class Player;
class Unit;
class Object;
class Item;

class RandomPlayerbotFactory
{
    public:
        RandomPlayerbotFactory(uint32 accountId);
		virtual ~RandomPlayerbotFactory() {}

	public:
        bool CreateRandomBot(uint8 cls, std::unordered_map<uint8, std::vector<std::string>>& names);
        static void CreateRandomBots();
        static void CreateRandomGuilds();
        static void CreateRandomArenaTeams();
        static std::string CreateRandomGuildName();
        static bool isAvailableRace(uint8 cls, uint8 race);
	private:
        std::string CreateRandomBotName(uint8 gender);
        static std::string CreateRandomArenaTeamName();

        uint8 GetRandomClass();
        uint8 GetRandomRace(uint8 cls);
    private:
        uint32 accountId;
        static std::map<uint8, std::vector<uint8> > availableRaces;
};

#endif
