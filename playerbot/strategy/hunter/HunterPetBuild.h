#pragma once

#include "Entities/Player.h"

class HunterPetBuild
{
    public:
        struct HunterPetBuildSpellEntity
        {
            uint8 Rank;
            uint8 Level;
            uint8 TPCost;
            uint32 SpellId;
            uint32 TrainingSpellId;

            HunterPetBuildSpellEntity(uint8 rank, uint8 level, uint8 tpCost, uint32 spellId, uint32 trainingSpellId)
            {
                Rank = rank;
                Level = level;
                TPCost = tpCost;
                SpellId = spellId;
                TrainingSpellId = trainingSpellId;
            }

            HunterPetBuildSpellEntity()
            {
                Rank = 0;
                Level = 1;
                TPCost = 0;
                SpellId = 0;
                TrainingSpellId = 0;
            }
        };

        struct HunterPetBuildSpell
        {
            std::string Name;
            int Position;
            bool IsActiveAbility;
            bool IsPetLearned;
            std::vector<uint32> FamilyIds;
            std::map<uint8, HunterPetBuildSpellEntity> Spells;

            HunterPetBuildSpell(std::string name, uint8 position, bool isActiveAbility, bool isPetLearned, std::vector<uint32> familyIds, std::map<uint8, HunterPetBuildSpellEntity> spells)
            {
                Name = name;
                Position = position;
                IsActiveAbility = isActiveAbility;
                FamilyIds = familyIds;
                Spells = spells;
                IsPetLearned = isPetLearned;
            }
            HunterPetBuildSpell()
            {
                Name = "Bad Entry";
                Position = -1;
                IsActiveAbility = true;
                IsPetLearned = false;
            }
        };

        std::map<int, HunterPetBuildSpell> spells;
        uint32 tpCost;

        HunterPetBuild() { InitHunterPetBuildSpellEntityList(); };
        HunterPetBuild(uint32 familyId);
        HunterPetBuild(HunterPetBuild* build, std::string buildLink);
        HunterPetBuild(Player* bot);
        HunterPetBuild(std::string buildLink);

        virtual bool CheckBuild(uint32 tpPoints, std::ostringstream* out);

        bool CheckBuildLink(std::string buildLink, uint32 familyId, std::ostringstream* out);

        std::string GetBuildLink();

        void ReadSpells(std::string buildLink);
        void ReadSpells(Player* bot);
        void ApplyBuild(Player* bot, std::vector<std::string> out);
        void UnlearnCurrentSpells(Player* bot);

        void InitializeStartingPetSpells(Player* bot, uint32 petLevel, uint32 petFamily);

        uint32 CalculateTrainingPoints(Player* bot);

        uint16 GetTPCostOfBuild();

    private:
        uint16 TPCost = 0;
        uint8 MaxRank = 0;
        std::string BuildLink;
        std::map<uint8, HunterPetBuildSpell> spellRankEntityMapping;

        void InitHunterPetBuildSpellEntityList();
      
};

class HunterPetBuildPath {
    public: 
        HunterPetBuildPath(int pathId, std::string pathName, int pathProbability) { id = pathId; name = pathName; probability = pathProbability; };
        HunterPetBuildPath() {};
        int id = -1;
        std::string name = "";
        int probability = 100;

        std::vector<HunterPetBuild> hunterPetBuild = {} ;
};

class FamilyPetBuilds {
public:
    FamilyPetBuilds(uint32 familyBuildId) { familyId = familyBuildId; baseBuild = HunterPetBuild(familyBuildId); };
    FamilyPetBuilds() { };


    uint32 familyId = 0;
    HunterPetBuild baseBuild = HunterPetBuild();

    std::vector<HunterPetBuildPath> hunterPetBuildPaths = {};

    HunterPetBuildPath GetBuildPath(int buildNo);
};