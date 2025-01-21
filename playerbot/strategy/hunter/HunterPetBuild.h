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
        };
        std::vector<HunterPetBuildSpellEntity> spells;

        HunterPetBuild() { InitHunterPetBuildSpellEntityList(); };
        HunterPetBuild(uint32 familyId);
        HunterPetBuild(HunterPetBuild* build, std::string buildLink);

        virtual bool CheckBuild(uint32 level, std::ostringstream* out);
        bool CheckBuildLink(std::string buildLink, std::ostringstream* out);

        void ReadSpells(std::string buildLink);
        void GetSpells(uint32 familyId);

        uint16 GetTPCostOfBuild();

    private:
        uint16 TPCost = 0;
        std::map<uint8, std::map<uint8, HunterPetBuildSpellEntity>> spellRankEntityMapping;

        void InitHunterPetBuildSpellEntityList();
      
};

class HunterPetBuildPath {
    public: 
        HunterPetBuildPath(int pathId, std::string pathName, int pathProbability) { id = pathId; name = pathName; probability = pathProbability; };

        int id = 0;
        std::string name = "";
        int probability = 100;

        std::vector<HunterPetBuild> hunterPetBuild;
};

class FamilyPetBuilds {
public:
    FamilyPetBuilds() {};
    FamilyPetBuilds(uint32 familyBuildId) { familyId = familyBuildId; baseBuild = HunterPetBuild(familyBuildId); };

    uint32 familyId = 0;
    HunterPetBuild baseBuild;

    std::vector<HunterPetBuildPath> hunterPetBuildPaths;
};