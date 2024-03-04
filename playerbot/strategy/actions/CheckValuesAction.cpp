
#include "playerbot/playerbot.h"
#include "CheckValuesAction.h"

#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

#include "playerbot/TravelMgr.h"
#include "../../TravelNode.h"
#include "playerbot/strategy/values/LastMovementValue.h"
using namespace ai;

CheckValuesAction::CheckValuesAction(PlayerbotAI* ai) : Action(ai, "check values")
{
}

bool CheckValuesAction::Execute(Event& event)
{
    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        ai->Ping(bot->GetPositionX() - 7.5, bot->GetPositionY() + 7.5);

        LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");

        if (lastMove.lastMoveShort)
            ai->Ping(lastMove.lastMoveShort.getX() - 7.5, lastMove.lastMoveShort.getY() + 7.5);
    }

    if (ai->HasStrategy("map", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("map full", BotState::BOT_STATE_NON_COMBAT))
    {
        sTravelNodeMap.manageNodes(bot, ai->HasStrategy("map full", BotState::BOT_STATE_NON_COMBAT));
    }

    list<ObjectGuid> possible_targets = AI_VALUE(list<ObjectGuid>, "possible targets");
    list<ObjectGuid> all_targets = AI_VALUE(list<ObjectGuid>, "all targets");
    list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
    list<ObjectGuid> corpses = AI_VALUE(list<ObjectGuid>, "nearest corpses");
    list<ObjectGuid> gos = AI_VALUE(list<ObjectGuid>, "nearest game objects");
    list<ObjectGuid> nfp = AI_VALUE(list<ObjectGuid>, "nearest friendly players");
    //if (!ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
    //    context->ClearExpiredValues();
    return true;
}
