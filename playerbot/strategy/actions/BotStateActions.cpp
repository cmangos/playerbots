
#include "playerbot/playerbot.h"
#include "BotStateActions.h"
#include "playerbot/strategy/values/PositionValue.h"

using namespace ai;

bool SetCombatStateAction::Execute(Event& event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    ai->OnCombatStarted();

    //Set stay positon on location when combat starts.
    if(ai->HasStrategy("stay", BotState::BOT_STATE_COMBAT) && !ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT))
        SET_AI_VALUE2(PositionEntry, "pos", "stay", PositionEntry(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),bot->GetMapId()));
    
    //Stop following on combat start.
    if (!ai->HasStrategy("follow", BotState::BOT_STATE_COMBAT) && ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
    {
        ai->StopMoving();
    }

    return true;
}

bool SetNonCombatStateAction::Execute(Event& event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    if (ai->IsStateActive(BotState::BOT_STATE_DEAD))
    {
        ai->OnResurrected();
    }
    else
    {
        ai->OnCombatEnded();
    }

    //Stop following on combat end.
    if (ai->HasStrategy("follow", BotState::BOT_STATE_COMBAT) && !ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
    {
        ai->StopMoving();
    }

    return true;
}

bool SetDeadStateAction::Execute(Event& event)
{
    SetDuration(sPlayerbotAIConfig.reactDelay);
    ai->OnDeath();
    return true;
}
