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
        };
        uint16 GetTPCostOfBuild();
        std::vector<HunterPetBuildSpellEntity> spells;
        HunterPetBuild() {};
        HunterPetBuild(uint32 familyId) { GetSpells(familyId); };

        void GetSpells(uint32 familyId);

    private:
        uint16 TPCost = 0;
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