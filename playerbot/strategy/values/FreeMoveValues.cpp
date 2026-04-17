
#include "playerbot/playerbot.h"
#include "FreeMoveValues.h"
#include "PositionValue.h"
#include "Formations.h"


using namespace ai;

GuidPosition FreeMoveCenterValue::Calculate()
{       
    if (ai->HasStrategy("follow", ai->GetState()) ||
        ai->HasStrategy("wander", ai->GetState()))
    {
        Unit* followTarget = AI_VALUE(Unit*, "follow target");

        if (!followTarget)
            return bot;

        //Use bot as center when follow target is on a different map.
        if (followTarget->GetMapId() != bot->GetMapId()) 
            return bot;

        Player* player = dynamic_cast<Player*>(followTarget);

        //Use bot as cetner when follow target is being teleported.
        if (player && player->IsBeingTeleported())
            return bot;

        //Use formation location as reference point.
        Formation* formation = AI_VALUE(Formation*, "formation");
        GuidPosition loc(followTarget->GetObjectGuid(),formation->GetLocation());

        if (Formation::IsNullLocation(loc) || loc.mapid == -1)
            return followTarget;

        //Move the location to a location around follow targets destination.
        if (player && player->GetPlayerbotAI() && ai->IsSafe(player) && PAI_VALUE(WorldPosition, "last long move"))
            loc += (PAI_VALUE(WorldPosition, "last long move") - player);

        return loc;
    }

    PositionEntry pos;

    if (ai->HasStrategy("stay", ai->GetState()) && (pos = AI_VALUE2(PositionEntry, "pos", "stay")).isSet())
        return GuidPosition(bot->GetObjectGuid(), pos.Get());

    if (ai->HasStrategy("guard", ai->GetState()) && (pos = AI_VALUE2(PositionEntry, "pos", "guard")).isSet())
        return GuidPosition(bot->GetObjectGuid(), pos.Get());

    return bot;
}

float FreeMoveRangeValue::Calculate()
{
    if (ai->HasStrategy("stay", ai->GetState()))
        return INTERACTION_DISTANCE;

    Unit* followTarget = AI_VALUE(Unit*, "follow target");

    if (!followTarget || followTarget == bot)
        return 0;

   if (ai->HasStrategy("wander", ai->GetState()))
   {
       return ai->GetRange("wandermax");
   }
   if (ai->HasStrategy("follow", ai->GetState()))
   {
       return ai->GetRange("follow");
   }
   if (ai->HasStrategy("guard", ai->GetState()))
   {
       return ai->GetRange("guard");
   }
    
   return 0;
}

bool CanFreeMoveValue::CanFreeMove(PlayerbotAI* ai, WorldPosition dest, float range)
{
    if (!dest)
        return true;

    if (!range)
        return true;

    AiObjectContext* context = ai->GetAiObjectContext();

    GuidPosition center = AI_VALUE(GuidPosition, "free move center");
    return center.distance(dest) < range;
}

bool CanFreeMoveValue::CanFreeMoveTo(PlayerbotAI* ai, WorldPosition dest)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    return CanFreeMove(ai, dest, AI_VALUE(float, "free move range"));
}

bool CanFreeMoveValue::CanFreeTarget(PlayerbotAI* ai, WorldPosition dest)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    float range = ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) ? std::min(ai->GetRange("spell"), AI_VALUE(float, "free move range")) : AI_VALUE(float, "free move range");

    return CanFreeMove(ai, dest, range);
}

bool CanFreeMoveValue::CanFreeAttack(PlayerbotAI* ai, WorldPosition dest)
{
    return CanFreeMove(ai, dest, ai->GetRange("attack"));
}

bool CanFreeMoveValue::Calculate()
{
    float range = 0.0;

    if (qualifier.empty())
        range = AI_VALUE(float, "free move range");
    else
        range = ai->GetRange(qualifier);

    return CanFreeMove(ai, bot, range);
}
