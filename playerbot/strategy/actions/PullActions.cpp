
#include "playerbot/playerbot.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "playerbot/strategy/values/AttackersValue.h"
#include "PullActions.h"
#include "playerbot/strategy/values/PositionValue.h"

using namespace ai;

static bool HasRequiredPullWeapon(Player* bot, PullStrategy* strategy)
{
    const bool needsRanged = bot->getClass() == CLASS_HUNTER || strategy->GetPullActionName() == "shoot";
    return !needsRanged || bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
}

bool PullCheckAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    PullStrategy* strategy = PullStrategy::Get(ai);
    const bool strategyActive = strategy != nullptr;
    if (!strategy)
        strategy = dynamic_cast<PullStrategy*>(ai->GetAiObjectContext()->GetStrategy("pull"));

    if (!strategy)
    {
        ai->TellPlayerNoFacing(requester, "My class has no pull ability.");
        return true;
    }

    const std::string spellName = strategy->GetSpellName();

    if (getName() == "pull capability")
    {
        if (strategyActive)
        {
            bool canPull = strategy->CanDoPullAction(nullptr);
            ai->TellPlayerNoFacing(requester, canPull ? "I am capable of pulling with " + spellName + "." : "I don't have a usable pull action.");
            return true;
        }

        if (!HasRequiredPullWeapon(bot, strategy))
        {
            ai->TellPlayerNoFacing(requester, "I can't pull: I have no ranged weapon equipped.");
            return true;
        }

        if (!ai->HasSpell(spellName))
        {
            ai->TellPlayerNoFacing(requester, "I can't pull: I don't know " + spellName + ".");
            return true;
        }

        ai->TellPlayerNoFacing(requester, "I can pull with " + spellName + ".");
        return true;
    }

    Unit* target = ai->GetUnit(requester->GetSelectionGuid());
    if (!target)
    {
        ai->TellPlayerNoFacing(requester, "Select a target to check pull readiness.");
        return true;
    }
    if (target->GetDistance(bot) > sPlayerbotAIConfig.reactDistance * 3)
    {
        ai->TellPlayerNoFacing(requester, "The target is too far away.");
        return true;
    }
    if (!AttackersValue::IsValid(target, bot, nullptr, false))
    {
        ai->TellPlayerNoFacing(requester, "That target can't be pulled.");
        return true;
    }
    if (!bot->IsWithinLOSInMap(target))
    {
        ai->TellPlayerNoFacing(requester, "I can't see that target.");
        return true;
    }

    if (!strategyActive)
    {
        const bool ready = HasRequiredPullWeapon(bot, strategy) && ai->HasSpell(spellName) && ai->CanCastSpell(spellName, target, true, nullptr, true);
        ai->TellPlayerNoFacing(requester, ready ? "I am ready to pull that target with " + spellName + "." : "I can't pull that target right now.");
        return true;
    }

    bool canPull = strategy->CanDoPullAction(target);
    if (canPull)
    {
        ObjectGuid previousTarget = AI_VALUE(ObjectGuid, "pull target");
        SET_AI_VALUE(ObjectGuid, "pull target", target->GetObjectGuid());
        canPull = ai->CanDoSpecificAction("pull action", true, true);
        SET_AI_VALUE(ObjectGuid, "pull target", previousTarget);
    }
    ai->TellPlayerNoFacing(requester, canPull ? "I am ready to pull that target." : "I can't pull that target right now.");
    return true;
}

bool PullRequestAction::Execute(Event& event)
{
    PullStrategy* strategy = PullStrategy::Get(ai);
    if (!strategy)
    {
        return false;
    }

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    Unit* target = GetTarget(event);
    if (!target)
    {
        ai->TellPlayerNoFacing(requester, "You have no target");
        return false;
    }

    const float maxPullDistance = sPlayerbotAIConfig.reactDistance * 3;
    const float distanceToPullTarget = target->GetDistance(ai->GetBot());
    if (distanceToPullTarget > maxPullDistance)
    {
        ai->TellPlayerNoFacing(requester, "The target is too far away");
        return false;
    }

    if (!AttackersValue::IsValid(target, bot, nullptr, false))
    {
        ai->TellPlayerNoFacing(requester, "The target can't be pulled");
        return false;
    }

    if (!strategy->CanDoPullAction(target))
    {
        std::ostringstream out; out << "Can't perform pull action '" << strategy->GetPullActionName() << "'";
        ai->TellPlayerNoFacing(requester, out.str());
        return false;
    }

    //Set position to return to after pulling.
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionEntry pullPosition = posMap["pull"];

    pullPosition.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
    posMap["pull"] = pullPosition;

    strategy->RequestPull(target);

    // Force change combat state to have a faster reaction time
    ai->OnCombatStarted();

    return true;
}

Unit* PullMyTargetAction::GetTarget(Event& event)
{
    Unit* target = nullptr;

    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (event.getSource() == "attack anything")
    {
        ObjectGuid guid = event.getObject();
        target = ai->GetCreature(guid);
    }
    else if (requester)
    {
        target = ai->GetUnit(requester->GetSelectionGuid());
    }

    return target;
}

Unit* PullRTITargetAction::GetTarget(Event& event)
{
    return ai->GetUnit(AI_VALUE(ObjectGuid, "rti target"));
}

bool PullStartAction::Execute(Event& event)
{
    bool result = false;
    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy)
    {
        Unit* target = strategy->GetTarget();
        if (target)
        {
            if (strategy->GetPreActionName().empty())
                result = true;
            else
            {
                result = ai->DoSpecificAction(strategy->GetPreActionName(), event, true);
                if(result)
                    SetDuration(3000);
            }

            // Set the pet on passive mode during the pull
            Pet* pet = bot->GetPet();
            if (pet)
            {
                UnitAI* creatureAI = ((Creature*)pet)->AI();
                if (creatureAI)
                {
                    strategy->SetPetReactState(creatureAI->GetReactState());
                    creatureAI->SetReactState(REACT_PASSIVE);
                }
            }

            strategy->OnPullStarted();
        }
    }

    return result;
}


PullAction::PullAction(PlayerbotAI* ai, std::string name) : CastSpellAction(ai, name)
{
    InitPullAction();
}

bool PullAction::Execute(Event& event)
{
    InitPullAction();

    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy)
    {
        Unit* target = strategy->GetTarget();
        if (target)
        {
            // Check if we are on pull range
            const float distanceToTarget = target->GetDistance(bot);
            if (distanceToTarget <= strategy->GetRange())
            {
                if (sServerFacade.isMoving(bot))
                {
                    // Force stop
                    ai->StopMoving();
                    strategy->RequestPull(target, false);
                    return false;
                }

                std::string actionName = strategy->GetPullActionName();

                // Execute the pull action
                SET_AI_VALUE(ObjectGuid, "current target", GetTarget()->GetObjectGuid());
                if (ai->DoSpecificAction(actionName, event, true))
                {
                    strategy->RequestPull(target); //extend pull timer to walk back.
                    return true;
                }
                else
                    return false;
            }
            else
            {
                // Retry the reach pull action
                strategy->RequestPull(target, false);
            }
        }
    }

    return false;
}

bool PullAction::isPossible()
{
    InitPullAction();

    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy)
    {
        std::string spellName = strategy->GetSpellName();
        Unit* target = strategy->GetTarget();
        if (!spellName.empty() && target)
        {
            if (!ai->CanCastSpell(spellName, target, true, nullptr, true))
            {
                return false;
            }
        }
    }

    return true;
}

void PullAction::InitPullAction()
{
    // Get the pull action spell name from the strategy
    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy)
    {
        std::string spellName = strategy->GetSpellName();
        if (!spellName.empty())
        {
            SetSpellName(spellName);

            float spellRange;
            if (ai->GetSpellRange(spellName, &spellRange))
            {
                range = spellRange;
            }
        }
    }
}

bool PullEndAction::Execute(Event& event)
{
    PullStrategy* strategy = PullStrategy::Get(ai);
    if (strategy)
    {
        // Restore the pet react state
        Pet* pet = bot->GetPet();
        if (pet)
        {
            UnitAI* creatureAI = ((Creature*)pet)->AI();
            if (creatureAI)
            {
                creatureAI->SetReactState(strategy->GetPetReactState());
                Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, "current target"));
                if (creatureAI->GetReactState() != REACT_PASSIVE && target)
                    creatureAI->AttackStart(target);
            }
        }

        // Remove the saved pull position
        AiObjectContext* context = ai->GetAiObjectContext();
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry stayPosition = posMap["pull"];
        if (stayPosition.isSet())
        {
            posMap.erase("pull");
        }

        strategy->OnPullEnded();
        return true;
    }

    return false;
}
