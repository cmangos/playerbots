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

        HunterPetBuild() {};
        HunterPetBuild(uint32 familyId);
        HunterPetBuild(HunterPetBuild* build, std::string buildLink);

        virtual bool CheckBuild(uint32 level, std::ostringstream* out);
        bool CheckBuildLink(std::string buildLink, std::ostringstream* out);

        void ReadSpells(std::string buildLink);
        void GetSpells(uint32 familyId);

        uint16 GetTPCostOfBuild();

    private:
        uint16 TPCost = 0;
      // Position in buildLink
        std::map<uint8, 
          // Rank of Spell
            std::map<uint8, HunterPetBuildSpellEntity>> SpellRankEntityMapping;

#pragma region HunterPetBuildSpellEntities
        // Arcane Resistance
        HunterPetBuildSpellEntity rank1ArcaneResistance = HunterPetBuildSpellEntity(1, 20, 5,   24493, 24495);
        HunterPetBuildSpellEntity rank2ArcaneResistance = HunterPetBuildSpellEntity(2, 30, 15,  24497, 24508);
        HunterPetBuildSpellEntity rank3ArcaneResistance = HunterPetBuildSpellEntity(3, 40, 45,  24500, 24509);
        HunterPetBuildSpellEntity rank4ArcaneResistance = HunterPetBuildSpellEntity(4, 50, 90,  24501, 24510);
        HunterPetBuildSpellEntity rank5ArcaneResistance = HunterPetBuildSpellEntity(5, 60, 105, 27052, 27350); // TBC

        // Avoidance
        HunterPetBuildSpellEntity rank1Avoidance = HunterPetBuildSpellEntity(1, 30, 15, 35694, 35699); // TBC
        HunterPetBuildSpellEntity rank2Avoidance = HunterPetBuildSpellEntity(2, 60, 25, 35698, 35700); // TBC
        
        // Bite
        HunterPetBuildSpellEntity rank1Bite = HunterPetBuildSpellEntity(1, 1,  1,  17253, 17254);
        HunterPetBuildSpellEntity rank2Bite = HunterPetBuildSpellEntity(2, 8,  4,  17255, 17262);
        HunterPetBuildSpellEntity rank3Bite = HunterPetBuildSpellEntity(3, 16, 7,  17256, 17263);
        HunterPetBuildSpellEntity rank4Bite = HunterPetBuildSpellEntity(4, 24, 10, 17257, 17264);
        HunterPetBuildSpellEntity rank5Bite = HunterPetBuildSpellEntity(5, 32, 13, 17258, 17265);
        HunterPetBuildSpellEntity rank6Bite = HunterPetBuildSpellEntity(6, 40, 17, 17259, 17266);
        HunterPetBuildSpellEntity rank7Bite = HunterPetBuildSpellEntity(7, 48, 21, 17260, 17267);
        HunterPetBuildSpellEntity rank8Bite = HunterPetBuildSpellEntity(8, 56, 25, 17261, 17268);
        HunterPetBuildSpellEntity rank9Bite = HunterPetBuildSpellEntity(9, 64, 29, 27050, 27348);

        // Charge
        HunterPetBuildSpellEntity rank1Charge = HunterPetBuildSpellEntity(1, 1,  5,  7371,  7370);
        HunterPetBuildSpellEntity rank2Charge = HunterPetBuildSpellEntity(2, 12, 9,  26177, 26184);
        HunterPetBuildSpellEntity rank3Charge = HunterPetBuildSpellEntity(3, 24, 13, 26178, 26185);
        HunterPetBuildSpellEntity rank4Charge = HunterPetBuildSpellEntity(4, 36, 17, 26179, 26186);
        HunterPetBuildSpellEntity rank5Charge = HunterPetBuildSpellEntity(5, 48, 25, 26201, 26202);
        HunterPetBuildSpellEntity rank6Charge = HunterPetBuildSpellEntity(6, 60, 29, 27685, 28343);

        // Claw
        HunterPetBuildSpellEntity rank1Claw = HunterPetBuildSpellEntity(1, 1,  1,  16827, 2980);
        HunterPetBuildSpellEntity rank2Claw = HunterPetBuildSpellEntity(2, 8,  4,  16828, 2981);
        HunterPetBuildSpellEntity rank3Claw = HunterPetBuildSpellEntity(3, 16, 7,  16829, 2982);
        HunterPetBuildSpellEntity rank4Claw = HunterPetBuildSpellEntity(4, 24, 10, 16830, 3667);
        HunterPetBuildSpellEntity rank5Claw = HunterPetBuildSpellEntity(5, 32, 13, 16831, 2975);
        HunterPetBuildSpellEntity rank6Claw = HunterPetBuildSpellEntity(6, 40, 17, 16832, 2976);
        HunterPetBuildSpellEntity rank7Claw = HunterPetBuildSpellEntity(7, 48, 21, 3010,  2977);
        HunterPetBuildSpellEntity rank8Claw = HunterPetBuildSpellEntity(8, 56, 25, 3009,  3666);
        HunterPetBuildSpellEntity rank9Claw = HunterPetBuildSpellEntity(9, 64, 29, 27049, 27347); // TBC

        // Cobra Reflexes
        HunterPetBuildSpellEntity rank1CobraReflexes = HunterPetBuildSpellEntity(1, 30, 15, 25076, 25077); // TBC

        // Cower
        HunterPetBuildSpellEntity rank1Cower = HunterPetBuildSpellEntity(1, 5,  8,  1742,  1747);
        HunterPetBuildSpellEntity rank2Cower = HunterPetBuildSpellEntity(2, 15, 10, 1753,  1748);
        HunterPetBuildSpellEntity rank3Cower = HunterPetBuildSpellEntity(3, 25, 12, 1754,  1749);
        HunterPetBuildSpellEntity rank4Cower = HunterPetBuildSpellEntity(4, 35, 14, 1755,  1750);
        HunterPetBuildSpellEntity rank5Cower = HunterPetBuildSpellEntity(5, 45, 16, 1756,  1751);
        HunterPetBuildSpellEntity rank6Cower = HunterPetBuildSpellEntity(6, 55, 18, 16997, 16998);
        HunterPetBuildSpellEntity rank7Cower = HunterPetBuildSpellEntity(7, 65, 21, 27048, 27346); // TBC

        // Dash
        HunterPetBuildSpellEntity rank1Dash = HunterPetBuildSpellEntity(1, 30, 15, 23099, 23100);
        HunterPetBuildSpellEntity rank2Dash = HunterPetBuildSpellEntity(2, 40, 20, 23109, 23111);
        HunterPetBuildSpellEntity rank3Dash = HunterPetBuildSpellEntity(3, 50, 25, 23110, 23112);

        // Dive
        HunterPetBuildSpellEntity rank1Dive = HunterPetBuildSpellEntity(1, 30, 15, 23145, 23146);
        HunterPetBuildSpellEntity rank2Dive = HunterPetBuildSpellEntity(2, 40, 20, 23147, 23149);
        HunterPetBuildSpellEntity rank3Dive = HunterPetBuildSpellEntity(3, 50, 25, 23148, 23150);

        // Fire Breath
        HunterPetBuildSpellEntity rank1FireBreath = HunterPetBuildSpellEntity(1, 1,  5,  34889, 34890);
        HunterPetBuildSpellEntity rank2FireBreath = HunterPetBuildSpellEntity(2, 60, 25, 35323, 35324);

        // Fire Resistance
        HunterPetBuildSpellEntity rank1FireResistance = HunterPetBuildSpellEntity(1, 20, 5,   23992, 24440);
        HunterPetBuildSpellEntity rank2FireResistance = HunterPetBuildSpellEntity(2, 30, 15,  24439, 24441);
        HunterPetBuildSpellEntity rank3FireResistance = HunterPetBuildSpellEntity(3, 40, 45,  24444, 24463);
        HunterPetBuildSpellEntity rank4FireResistance = HunterPetBuildSpellEntity(4, 50, 90,  24445, 24464);
        HunterPetBuildSpellEntity rank5FireResistance = HunterPetBuildSpellEntity(5, 60, 105, 27053, 27351); // TBC

        // Frost Resistance
        HunterPetBuildSpellEntity rank1FrostResistance = HunterPetBuildSpellEntity(1, 20, 5,   24446, 24475);
        HunterPetBuildSpellEntity rank2FrostResistance = HunterPetBuildSpellEntity(2, 30, 15,  24447, 24476);
        HunterPetBuildSpellEntity rank3FrostResistance = HunterPetBuildSpellEntity(3, 40, 45,  24448, 24477);
        HunterPetBuildSpellEntity rank4FrostResistance = HunterPetBuildSpellEntity(4, 50, 90,  24449, 24478);
        HunterPetBuildSpellEntity rank5FrostResistance = HunterPetBuildSpellEntity(5, 60, 105, 27054, 27352); // TBC
        
#pragma endregion
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