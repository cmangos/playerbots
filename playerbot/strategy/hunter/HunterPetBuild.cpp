#include "HunterPetBuild.h"

uint16 HunterPetBuild::GetTPCostOfBuild()
{
    for (HunterPetBuildSpellEntity spell : spells)
    {
        TPCost += spell.TPCost;
    }
}
