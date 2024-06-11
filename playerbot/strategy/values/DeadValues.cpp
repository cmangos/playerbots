
#include "playerbot/playerbot.h"
#include "DeadValues.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

GuidPosition GraveyardValue::Calculate()
{
    WorldPosition refPosition = bot, botPos(bot);

    if (getQualifier() == "master" && ai->GetGroupMaster() && ai->IsSafe(ai->GetGroupMaster()))
        refPosition = ai->GetGroupMaster();
    else if (getQualifier() == "travel")
    {
        if(!AI_VALUE(TravelTarget*, "travel target") || !AI_VALUE(TravelTarget*, "travel target")->getPosition())
            return GuidPosition();

        refPosition = *AI_VALUE(TravelTarget*, "travel target")->getPosition();
    }
    else if (getQualifier() == "home bind")
         refPosition = AI_VALUE(WorldPosition, "home bind");
    else if (getQualifier() == "start")
    {
        std::vector<uint32> races;

        if (bot->GetTeam() == ALLIANCE)
            races = { RACE_HUMAN, RACE_DWARF,RACE_GNOME,RACE_NIGHTELF };
        else
            races = { RACE_ORC, RACE_TROLL,RACE_TAUREN,RACE_UNDEAD };

        refPosition = WorldPosition();

        for (auto race : races)
        {
            for (uint32 cls = 0; cls < MAX_CLASSES; cls++)
            {
                PlayerInfo const* info = sObjectMgr.GetPlayerInfo(race, cls);

                if (!info)
                    continue;

                if (refPosition && botPos.fDist(refPosition) < botPos.fDist(info))
                    continue;

                refPosition = info;
            }
        }
    }        

    WorldSafeLocsEntry const* ClosestGrave = bot->GetMap()->GetGraveyardManager().GetClosestGraveYard(refPosition.getX(), refPosition.getY(), refPosition.getZ(), refPosition.getMapId(), bot->GetTeam());

    if (!ClosestGrave)
        return GuidPosition();

    return GuidPosition(0,ClosestGrave);
}

GuidPosition BestGraveyardValue::Calculate()
{
    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
    {
        return GuidPosition();
    }

    //Revive near master.
    if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && ai->GetGroupMaster() && ai->GetGroupMaster() != bot)
        return AI_VALUE2(GuidPosition, "graveyard", "master");

    uint32 deathCount = AI_VALUE(uint32, "death count");

    //Revive nearby.
    if (deathCount < 5)
        return AI_VALUE2(GuidPosition, "graveyard", "self");

    //Revive near travel target if it's far away from last death.
    if (AI_VALUE2(GuidPosition, "graveyard", "travel") && AI_VALUE2(GuidPosition, "graveyard", "travel").fDist(corpse) > sPlayerbotAIConfig.reactDistance)
        return AI_VALUE2(GuidPosition, "graveyard", "travel");

    //Revive near Inn.
    if (deathCount < 15)
        return AI_VALUE2(GuidPosition, "graveyard", "home bind");

    //Revive at spawn.
    return AI_VALUE2(GuidPosition, "graveyard", "start");
}

bool ShouldSpiritHealerValue::Calculate()
{
    uint32 deathCount = AI_VALUE(uint32, "death count");
    uint8 durability = AI_VALUE(uint8, "durability");

    if (ai->HasActivePlayerMaster()) //Only use spirit healers with direct command with active master.
        return false;

    //Nothing to lose
    if (deathCount > 2 && durability < 10 && (ai->HasAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS, bot) || ai->HasCheat(BotCheatMask::repair)))
        return true;

    Corpse* corpse = bot->GetCorpse();
    if (!corpse)
    {
        return false;
    }

    uint32 deadTime = time(nullptr) - corpse->GetGhostTime();
    //We are dead for a long time
    if (deadTime > 10 * MINUTE && deathCount > 1)
        return true;

    //Try to revive nearby.
    if (deathCount < 5)
        return false;

    //Try to get to a safe place 
    if ((deathCount > 10 && durability < 10) || deathCount > 15)
        return true;

    //If there are enemies near grave and corpse we go to corpse first.
    if (AI_VALUE2(bool, "manual bool", "enemies near graveyard"))
        return false;

    //Enemies near corpse so try grave first.
    if (AI_VALUE2(bool, "manual bool", "enemies near corpse"))
        return true;

    GuidPosition graveyard = AI_VALUE(GuidPosition, "best graveyard");

    float corpseDistance = WorldPosition(bot).fDist(corpse);
    float graveYardDistance = WorldPosition(bot).fDist(graveyard);
    bool corpseInSight = corpseDistance < sPlayerbotAIConfig.sightDistance;
    bool graveInSight = graveYardDistance < sPlayerbotAIConfig.sightDistance;
    bool enemiesNear = !AI_VALUE(std::list<ObjectGuid>, "possible targets").empty();

    if (enemiesNear)
    {
        if (graveInSight)
        {
            SET_AI_VALUE2(bool, "manual bool", "enemies near graveyard", true);
            return false;
        }
        if (corpseInSight)
        {
            SET_AI_VALUE2(bool, "manual bool", "enemies near graveyard", true);
            return true;
        }
    }
    
    //If grave is near and no ress sickness go there.
    if (graveInSight && !corpseInSight && ai->HasCheat(BotCheatMask::repair))
        return true;

    //Stick to corpse.
    return false;
}