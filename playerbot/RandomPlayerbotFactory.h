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
        
        enum class NameRaceAndGender : uint8
        {
            // Generic is the category used for human & undead
            GenericMale = 0,
            GenericFemale,
            GnomeMale,
            GnomeFemale,
            DwarfMale,
            DwarfFemale,
            NightelfMale,
            NightelfFemale,
            DraeneiMale,
            DraeneiFemale,
            OrcMale,
            OrcFemale,
            TrollMale,
            TrollFemale,
            TaurenMale,
            TaurenFemale,
            BloodelfMale,
            BloodelfFemale
        };

        static constexpr NameRaceAndGender CombineRaceAndGender(uint8 gender, uint8 race);

        RandomPlayerbotFactory(uint32 accountId);
		virtual ~RandomPlayerbotFactory() {}

	public:
        bool CreateRandomBot(uint8 cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& names);
        static void CreateRandomBots();
        static void CreateRandomGuilds();
        static void CreateRandomArenaTeams();
        static std::string CreateRandomGuildName();
        static bool isAvailableRace(uint8 cls, uint8 race);
        static void CheckCaptainAvailability(Player* player, std::vector<ObjectGuid>& availableCaptains, ObjectGuid captain, const std::vector<ArenaType>& arenaTypes);
	private:
        std::string CreateRandomBotName(NameRaceAndGender raceAndGender);
        static std::string CreateRandomArenaTeamName(ArenaType randomType);

        uint8 GetRandomClass();
        uint8 GetRandomRace(uint8 cls);
    private:
        uint32 accountId;
        static std::map<uint8, std::vector<uint8> > availableRaces;
};

#endif
