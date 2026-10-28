#include "playerbot/playerbot.h"
#include "UnstuckAction.h"

bool UnstuckAction::Execute(Event& event)
{
    std::string source = event.getSource();
    Player* bot = ai->GetBot();
    Player* master = ai->GetMaster();

    // Default action if no specific source is matched
    if (source.empty())
    {
        ai->TellDebug(master, "Unstuck: No specific source, resetting.", "debug unstuck");
        return ai->DoSpecificAction("reset", event, true);
    }

    // Handle move stuck scenarios
    if (source.find("move stuck") != std::string::npos)
    {
        ai->TellDebug(master, "Unstuck: Move stuck detected, resetting.", "debug unstuck");
        return ai->DoSpecificAction("reset", event, true);
    }

    // Handle long move stuck scenarios
    if (source.find("move long stuck") != std::string::npos)
    {
        ai->TellDebug(master, "Unstuck: Long move stuck detected, attempting hearthstone or repop.", "debug unstuck");
        if (AI_VALUE2(bool, "action useful", "hearthstone") && bot->IsAlive())
        {
            return ai->DoSpecificAction("hearthstone", event, true);
        }
        else
        {
            return ai->DoSpecificAction("repop", event, true);
        }
    }

    // Handle combat stuck scenarios
    if (source.find("combat stuck") != std::string::npos)
    {
        ai->TellDebug(master, "Unstuck: Combat stuck detected, resetting position.", "debug unstuck");
        return ai->DoSpecificAction("reset", event, true);
    }

    // Handle long combat stuck scenarios
    if (source.find("combat long stuck") != std::string::npos)
    {
        ai->TellDebug(master, "Unstuck: Long combat stuck detected, attempting hearthstone or repop.", "debug unstuck");
        if (AI_VALUE2(bool, "action useful", "hearthstone") && bot->IsAlive())
        {
            return ai->DoSpecificAction("hearthstone", event, true);
        }
        else
        {
            return ai->DoSpecificAction("repop", event, true);
        }
    }

    // Fallback to reset if no specific condition is met
    ai->TellDebug(master, "Unstuck: Fallback to reset action.", "debug unstuck");
    return ai->DoSpecificAction("reset", event, true);
}

bool DropStandoffAction::Execute(Event& event)
{
    ai->TellDebug(ai->GetMaster(), "Standoff: no damage exchanged while stationary, dropping combat.", "debug unstuck");

    bot->AttackStop();
    bot->SetSelectionGuid(ObjectGuid());
    SET_AI_VALUE(ObjectGuid, "current target", ObjectGuid());
    SET_AI_VALUE(ObjectGuid, "old target", ObjectGuid());
    SET_AI_VALUE(ObjectGuid, "attack target", ObjectGuid());
    bot->CombatStop();

    return true;
}

bool ReleaseStuckMovementAction::Execute(Event& event)
{
    Unit* currentTarget = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));

    if (currentTarget)
    {
        ai->TellDebug(ai->GetMaster(), "Stuck movement: dropping unreachable target.", "debug unstuck");

        SET_AI_VALUE(ObjectGuid, "old target", currentTarget->GetObjectGuid());
        SET_AI_VALUE(ObjectGuid, "current target", ObjectGuid());
        if (AI_VALUE(ObjectGuid, "attack target") == currentTarget->GetObjectGuid())
            SET_AI_VALUE(ObjectGuid, "attack target", ObjectGuid());

        bot->SetSelectionGuid(ObjectGuid());
        bot->AttackStop();
    }

    RESET_AI_VALUE(GuidPosition, "rpg target");

    return true;
}
