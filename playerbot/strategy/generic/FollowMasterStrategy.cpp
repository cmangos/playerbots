
#include "playerbot/playerbot.h"
#include "FollowMasterStrategy.h"

using namespace ai;

void FollowMasterStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "out of free move range",
        NextAction::array(0, new NextAction("check mount state", ACTION_HIGH), new NextAction("follow", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "update follow",
        NextAction::array(0, new NextAction("follow", ACTION_IDLE), NULL)));
}

void FollowMasterStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void FollowMasterStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void FollowMasterStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "stop follow",
        NextAction::array(0, new NextAction("stop follow", ACTION_PASSTROUGH), NULL)));
}

void FollowMasterStrategy::OnStrategyAdded(BotState state)
{
    if (state != BotState::BOT_STATE_REACTION)
    {
        ai->ChangeStrategy("+" + getName(), BotState::BOT_STATE_REACTION);
    }
}

void FollowMasterStrategy::OnStrategyRemoved(BotState state)
{
    if (state == ai->GetState() && ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        ai->StopMoving();
    }

    if (state == BotState::BOT_STATE_REACTION)
        return;

    bool hasFollow = false;

    for (uint8 checkState = (uint8)BotState::BOT_STATE_COMBAT; checkState < (uint8)BotState::BOT_STATE_REACTION; checkState++)
    {
        if (ai->HasStrategy(getName(), BotState(checkState)))
        {
            hasFollow = true;
            break;
        }
    }

    if (!hasFollow)
    {
        ai->ChangeStrategy("-" + getName(), BotState::BOT_STATE_REACTION);
    }
}
