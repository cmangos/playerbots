
#include "playerbot/playerbot.h"
#include "WanderStrategy.h"

using namespace ai;

void WanderStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wander far",
        NextAction::array(0, new NextAction("check mount state", ACTION_HIGH), new NextAction("flee to master", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "wander medium",
        NextAction::array(0, new NextAction("check mount state", 0.1f), new NextAction("follow", 0.1f), NULL)));

    triggers.push_back(new TriggerNode(
        "wander near",
        NextAction::array(0, new NextAction("stop follow", 0.1f), NULL)));
}

void WanderStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wander far",
        NextAction::array(0, new NextAction("check mount state", ACTION_HIGH), new NextAction("flee to master", ACTION_HIGH), NULL)));

    triggers.push_back(new TriggerNode(
        "wander medium",
        NextAction::array(0, new NextAction("check mount state", 0.1f), new NextAction("follow", 0.1f), NULL)));

    triggers.push_back(new TriggerNode(
        "wander near",
        NextAction::array(0, new NextAction("stop follow", 0.1f), NULL)));
}

void WanderStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    InitNonCombatTriggers(triggers);
}

void WanderStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "stop follow",
        NextAction::array(0, new NextAction("stop follow", ACTION_PASSTROUGH), NULL)));
}

void WanderStrategy::OnStrategyAdded(BotState state)
{
    if (state != BotState::BOT_STATE_REACTION)
    {
        ai->ChangeStrategy("+wander", BotState::BOT_STATE_REACTION);
    }
}

void WanderStrategy::OnStrategyRemoved(BotState state)
{
    if (state == ai->GetState() && ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        ai->StopMoving();
    }

    if (state == BotState::BOT_STATE_REACTION)
        return;

    BotState checkState1, checkState2;

    if (state == BotState::BOT_STATE_COMBAT)
    {
        checkState1 = BotState::BOT_STATE_NON_COMBAT;
        checkState2 = BotState::BOT_STATE_DEAD;
    }
    else if (state == BotState::BOT_STATE_NON_COMBAT)
    {
        checkState1 = BotState::BOT_STATE_COMBAT;
        checkState2 = BotState::BOT_STATE_DEAD;
    }
    else
    {
        checkState1 = BotState::BOT_STATE_NON_COMBAT;
        checkState2 = BotState::BOT_STATE_COMBAT;
    }

    if (!ai->HasStrategy("wander", checkState1) && !ai->HasStrategy("wander", checkState2))
    {
        ai->ChangeStrategy("-wander", BotState::BOT_STATE_REACTION);
    }
}