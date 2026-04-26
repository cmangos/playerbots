#ifndef _RandomPlayerbotFactory_H
#define _RandomPlayerbotFactory_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include <mutex>

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

        static constexpr NameRaceAndGender CombineRaceAndGender(uint8 gender, uint8 race)
        {
            switch (race)
            {
                case RACE_HUMAN: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
                case RACE_ORC: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::OrcMale) + gender);
                case RACE_DWARF: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::DwarfMale) + gender);
                case RACE_NIGHTELF: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::NightelfMale) + gender);
                case RACE_UNDEAD: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
                case RACE_TAUREN: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::TaurenMale) + gender);
                case RACE_GNOME: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GnomeMale) + gender);
                case RACE_TROLL: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::TrollMale) + gender);
#ifndef MANGOSBOT_ZERO
                case RACE_DRAENEI: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::DraeneiMale) + gender);
                case RACE_BLOODELF: return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::BloodelfMale) + gender);
#endif
                default:
                    return static_cast<NameRaceAndGender>(static_cast<uint8>(NameRaceAndGender::GenericMale) + gender);
            }
        }

        RandomPlayerbotFactory(uint32 accountId);
		virtual ~RandomPlayerbotFactory() {}

	public:
        bool CreateRandomBot(uint8 cls, uint8 inputRace = 0);
        static void CreateRandomBots();
        static void CreateRandomGuilds();
        static void CreateRandomArenaTeams();
        static std::string CreateRandomGuildName();
        static bool isAvailableRace(uint8 cls, uint8 race);
        static bool isAvailableRole(uint8 cls, BotRoles role = BotRoles::BOT_ROLE_NONE);
        uint8 GetRandomClass(uint8 race = 0, BotRoles role = BotRoles::BOT_ROLE_NONE);
        static bool isRaceForTeam(uint8 race, Team team = Team::TEAM_BOTH_ALLOWED);
        uint8 GetRandomRace(uint8 cls, Team team = Team::TEAM_BOTH_ALLOWED);
        static std::string CreateRandomBotName(NameRaceAndGender raceAndGender);
        static void EnsureNamesInitialized();
    private:
        static std::string CreateRandomArenaTeamName();
        static std::unordered_map<NameRaceAndGender, std::vector<std::string>> freeNames;
        static std::unordered_map<NameRaceAndGender, std::vector<std::string>> allNames;
        static std::mutex nameMutex;
        static bool namesInitialized;

        uint32 accountId;
        static std::map<uint8, std::vector<uint8> > availableRaces;
};

#endif
