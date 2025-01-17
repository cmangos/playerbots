#include "HunterPetBuild.h"

uint16 HunterPetBuild::GetTPCostOfBuild()
{
    int tpCost = 0;
    for (HunterPetBuildSpellEntity spell : spells)
    {
        tpCost += spell.TPCost;
    }

    return tpCost;
}

HunterPetBuild::HunterPetBuild(uint32 familyId)
{
    GetSpells(familyId);
}

HunterPetBuild::HunterPetBuild(HunterPetBuild* build, std::string buildLink)
{
    spells = build->spells;
    ReadSpells(buildLink);
}

// Checks if the spells listed are valid for the level. As TP varies due to level 
// and loyalty of a given pet that check will be done separately.
bool HunterPetBuild::CheckBuild(uint32 level, std::ostringstream* out)
{
    return false;
}

bool HunterPetBuild::CheckBuildLink(std::string buildLink, std::ostringstream* out)
{
    return false;
}

void HunterPetBuild::ReadSpells(std::string buildLink)
{

}

void HunterPetBuild::GetSpells(uint32 familyId)
{
}
