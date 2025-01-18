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
    for (auto& entry : spells)
    {
        if (entry.Level > level)
        {
            *out << "Build is for a higher level.";
            return false;
        }
    }
    return true;
}

bool HunterPetBuild::CheckBuildLink(std::string buildLink, std::ostringstream* out)
{
#ifdef MANGOSBOT_ZERO
    int buildLinkLength = 27;
#endif
#ifdef MANGOSBOT_ONE
    int buildLinkLength = 35;
#endif
    for (int ii = 0; ii < buildLinkLength; ii++)
    {
        // if 4th digit skip if '-' other wise invalid format so reject build link.
        if ((ii + 1) % 4 == 0)
            if (buildLink[ii] == '-')
                continue;
            else
                return false;

    }
}

void HunterPetBuild::ReadSpells(std::string buildLink)
{

}

void HunterPetBuild::GetSpells(uint32 familyId)
{
}
