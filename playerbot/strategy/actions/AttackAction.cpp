
#include "playerbot/playerbot.h"
#include "AttackAction.h"
#include "MotionGenerators/MovementGenerator.h"
#include "AI/BaseAI/CreatureAI.h"
#include "playerbot/LootObjectStack.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/generic/CombatStrategy.h"

using namespace ai;

bool AttackAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    Unit* target = GetTarget();
    if (target && target->IsInWorld() && target->GetMapId() == bot->GetMapId())
    {
        return Attack(requester, target);
    }

    return false;
}

bool AttackMyTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if(requester)
    {
        const ObjectGuid guid = requester->GetSelectionGuid();
        if (guid)
        {
            if (Attack(requester, ai->GetUnit(guid)))
            {
                SET_AI_VALUE(ObjectGuid, "attack target", guid);
                return true;
            }
        }
        else if (verbose)
        {
            ai->TellError(requester, "You have no target");
        }
    }

    return false;
}

bool AttackRTITargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    Unit* rtiTarget = AI_VALUE(Unit*, "rti target");

    if (rtiTarget && rtiTarget->IsInWorld() && rtiTarget->GetMapId() == bot->GetMapId())
    {
        if (Attack(requester, rtiTarget))
        {
            SET_AI_VALUE(ObjectGuid, "attack target", rtiTarget->GetObjectGuid());
            return true;
        }
    }
    else
    {
        ai->TellError(requester, "I dont see my rti attack target");
    }

    return false;
}

bool AttackMyTargetAction::isUseful()
{
    return !ai->ContainsStrategy(STRATEGY_TYPE_HEAL) || ai->HasStrategy("offdps", BotState::BOT_STATE_COMBAT);
}

bool AttackRTITargetAction::isUseful()
{
    return !ai->ContainsStrategy(STRATEGY_TYPE_HEAL) || ai->HasStrategy("offdps", BotState::BOT_STATE_COMBAT);
}

bool AttackAction::Attack(Player* requester, Unit* target)
{
    MotionMaster &mm = *bot->GetMotionMaster();
	if (mm.GetCurrentMovementGeneratorType() == TAXI_MOTION_TYPE || (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f))
    {
        if (verbose)
        {
            ai->TellPlayerNoFacing(requester, "I cannot attack in flight");
        }

        return false;
    }

    if (IsTargetValid(requester, target))
    {
        if (bot->IsMounted() && (sServerFacade.GetDistance2d(bot, target) < 40.0f || bot->IsFlying()))
        {
            ai->Unmount();
            
            if (bot->IsFlying())
            {
                return true;
            }
        }

        ObjectGuid guid = target->GetObjectGuid();
        bot->SetSelectionGuid(target->GetObjectGuid());

        Unit* oldTarget = AI_VALUE(Unit*, "current target");
        if(oldTarget)
        {
            SET_AI_VALUE(Unit*, "old target", oldTarget);
        }

        SET_AI_VALUE(Unit*, "current target", target);
        AI_VALUE(LootObjectStack*, "available loot")->Add(guid);

        WaitForAttackStrategy* strategy = WaitForAttackStrategy::Get(ai);
        bool isWaitingForAttack = false;
        if (strategy)
        {
            isWaitingForAttack = strategy->ShouldWait(ai);
            Pet* pet = bot->GetPet();
            if (pet)
            {
                UnitAI* creatureAI = ((Creature*)pet)->AI();
                if (creatureAI)
                {
                    // Don't send the pet to attack if the bot is waiting for attack
                    if (!isWaitingForAttack && (!ai->HasStrategy("stay", BotState::BOT_STATE_COMBAT) || AI_VALUE2(float, "distance", "current target") < ai->GetRange("spell")))
                    {
                        // Reset the pet state if no master
                        if (creatureAI->GetReactState() == REACT_PASSIVE && !ai->GetMaster())
                        {
                            creatureAI->SetReactState(REACT_DEFENSIVE);
                        }
                        else 
                        {
                            // If we're done waiting to attack and there's mobs to cc, we can't use defensive/aggressive
                            // because non passive pets will ignore our cc
                            // Therefore, we'll keep passive so we can only attack the current target specifically
                            // In other words, pet only attacks what owner can attack
                            Unit* ccTarget = AI_VALUE(Unit*, "rti cc target");
                            if (!ccTarget || (ccTarget && target->GetObjectGuid() != ccTarget->GetObjectGuid()))
                            {
                                constexpr uint32 PET_IMP = 416;
                                constexpr uint32 PHASE_SHIFT = 4511;
                                if (!(bot->getClass() == CLASS_WARLOCK &&
                                    pet->AI() && pet->AI()->HasReactState(REACT_PASSIVE) &&
                                    pet->GetEntry() == PET_IMP && pet->HasAura(PHASE_SHIFT)))
                                {
                                    // Send pet action packet
                                    const ObjectGuid& petGuid = pet->GetObjectGuid();
                                    const ObjectGuid& targetGuid = target->GetObjectGuid();
                                    const uint8 flag = ACT_COMMAND;
                                    const uint32 spellId = COMMAND_ATTACK;
                                    const uint32 command = (flag << 24) | spellId;

                                    WorldPacket data(CMSG_PET_ACTION);
                                    data << petGuid;
                                    data << command;
                                    data << targetGuid;
                                    bot->GetSession()->HandlePetAction(data);
                                }
                            }
                        }
                    }
                    else
                    {
                        strategy->SetPetReactState(creatureAI->GetReactState() != REACT_PASSIVE ? creatureAI->GetReactState() : REACT_PASSIVE);
                        creatureAI->SetReactState(REACT_PASSIVE);

                        // Send pet action packet
                        const ObjectGuid& petGuid = pet->GetObjectGuid();
                        const uint8 flag = ACT_REACTION;
                        const uint32 spellId = REACT_PASSIVE;
                        const uint32 data = (flag << 24) | spellId;

                        WorldPacket packet(CMSG_PET_ACTION);
                        packet << petGuid;
                        packet << data;
                        packet << uint64(0);
                        bot->GetSession()->HandlePetAction(packet);
                        bot->PetSpellInitialize();
                    }
                }
            }
        }

        if (ai->CanMove() && !sServerFacade.IsInFront(bot, target, sPlayerbotAIConfig.sightDistance, CAST_ANGLE_IN_FRONT))
        {
            sServerFacade.SetFacingTo(bot, target);
        }

        bool result = true;

        // Don't attack target if it is waiting for attack or in stealth
        if (!ai->HasStrategy("stealthed", BotState::BOT_STATE_COMBAT) && !isWaitingForAttack)
        {
            ai->PlayAttackEmote(1);
            result = bot->Attack(target, !ai->IsRanged(bot) || (sServerFacade.GetDistance2d(bot, target) < 5.0f));
        }

        if (result)
        {
            // Force change combat state to have a faster reaction time
            ai->OnCombatStarted();
        }

        return result;
    }

    return false;
}

bool AttackAction::IsTargetValid(Player* requester, Unit* target)
{
    if (!target)
    {
        if (verbose) 
        {
            ai->TellPlayerNoFacing(requester, "I have no target");
        }

        return false;
    }
    else if (sServerFacade.IsFriendlyTo(bot, target))
    {
        if (verbose)
        {
            std::ostringstream msg;
            msg << target->GetName();
            msg << " is friendly to me";
            ai->TellPlayerNoFacing(requester, msg.str());
        }

        return false;
    }
    else if (sServerFacade.UnitIsDead(target))
    {
        if (verbose)
        {
            std::ostringstream msg;
            msg << target->GetName();
            msg << " is dead";
            ai->TellPlayerNoFacing(requester, msg.str());
        }

        return false;
    }
    else if (sServerFacade.GetDistance2d(bot, target) > sPlayerbotAIConfig.sightDistance)
    {
        if (verbose)
        {
            std::ostringstream msg;
            msg << target->GetName();
            msg << " is too far away";
            ai->TellPlayerNoFacing(requester, msg.str());
        }

        return false;
    }

    return true;
}

bool AttackDuelOpponentAction::isUseful()
{
    return AI_VALUE(Unit*, "duel target");
}

bool AttackDuelOpponentAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    return Attack(requester, AI_VALUE(Unit*, "duel target"));
}
