
#include "playerbot/playerbot.h"
#include "DuelTargetValue.h"

using namespace ai;

ObjectGuid DuelTargetValue::Calculate()
{
    if (bot->duel)
    {
        Player* player = bot->duel->opponent;
        if (player) return player->GetObjectGuid();
    }
    
    return NULL;
}
