#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "MovementActions.h"
#include "UseItemAction.h"
#include "playerbot/strategy/values/GuidPositionValues.h"

namespace ai
{
    const uint32 SPELL_DISARM_TRAP = 1842;

    class BlackwingLairEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        BlackwingLairEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable blackwing lair strategy", "+blackwing lair") {}
    };

    class BlackwingLairDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        BlackwingLairDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable blackwing lair strategy", "-blackwing lair") {}
    };

    class MoveToSuppressionDeviceAction : public MovementAction
    {
    public:
        MoveToSuppressionDeviceAction(PlayerbotAI* ai) : MovementAction(ai, "move to suppression device") {}

        bool Execute(Event& event) override
        {
            std::list<GuidPosition> gos = AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos in sight,suppression devices}");
            
            if (gos.empty())
                return false;

            WorldPosition botPos(bot);
            GuidPosition closest;
            float closestDist = FLT_MAX;

            for (const GuidPosition& gp : gos)
            {
                float dist = botPos.distance(gp);
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closest = gp;
                }
            }

            if (!closest)
                return false;
            
            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            {
                ai->TellPlayerNoFacing(GetMaster(), "Moving to Suppression Device at " + std::to_string((int)closestDist) + " yards");
            }

            return MoveTo(closest.getMapId(), closest.getX(), closest.getY(), closest.getZ());
        }

        bool isPossible() override
        {
            return ai->CanMove();
        }

        bool isUseful() override
        {
            std::list<GuidPosition> gos = AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos in sight,suppression devices}");
            return !gos.empty();
        }
    };

    class StealthForSuppressionDeviceAction : public Action
    {
    public:
        StealthForSuppressionDeviceAction(PlayerbotAI* ai) : Action(ai, "stealth for suppression device") {}

        bool Execute(Event& event) override
        {
            if (bot->getClass() != CLASS_ROGUE)
                return false;

            if (ai->HasAura("stealth", bot))
                return false;

            if (ai->CastSpell("stealth", bot))
            {
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("+stealthed", BotState::BOT_STATE_NON_COMBAT);
                bot->InterruptSpell(CURRENT_MELEE_SPELL);
                return true;
            }

            return false;
        }

        bool isPossible() override
        {
            return bot->getClass() == CLASS_ROGUE && !ai->HasAura("stealth", bot);
        }

        bool isUseful() override
        {
            if (ai->HasAura("stealth", bot))
                return false;

            // Core rogue stealth logic had some WSG/EYE flag checks, added in here too just in case
            return !ai->HasAura(23333, bot) && !ai->HasAura(23335, bot) && !ai->HasAura(34976, bot);
        }
    };

    class DeactivateSuppressionDeviceAction : public Action
    {
    public:
        DeactivateSuppressionDeviceAction(PlayerbotAI* ai) : Action(ai, "deactivate suppression device") {}

        bool Execute(Event& event) override
        {
            std::list<GuidPosition> gos = AI_VALUE(std::list<GuidPosition>, "entry filter::{gos close,suppression devices}");
            
            if (gos.empty())
                return false;

            for (const GuidPosition& guidPos : gos)
            {
                GameObject* go = ai->GetGameObject(guidPos);
                if (!go)
                    continue;

                if (go->GetLootState() != GO_READY)
                    continue;

                if (!bot->GetGameObjectIfCanInteractWith(go->GetObjectGuid(), GAMEOBJECT_TYPE_TRAP))
                    continue;

                std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
                *packet << go->GetObjectGuid();
                bot->GetSession()->QueuePacket(std::move(packet));

                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    ai->TellPlayerNoFacing(GetMaster(), "Deactivating Suppression Device");
                }

                return true;
            }

            return false;
        }

        bool isPossible() override
        {
            return ai->CanMove();
        }
    };

    class DisarmSuppressionDeviceAction : public Action
    {
    public:
        DisarmSuppressionDeviceAction(PlayerbotAI* ai) : Action(ai, "disarm suppression device") {}

        bool Execute(Event& event) override
        {
            if (bot->getClass() != CLASS_ROGUE)
                return false;

            if (!bot->HasSpell(SPELL_DISARM_TRAP))
                return false;

            std::list<GuidPosition> gos = AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos close,suppression devices}");
            
            if (gos.empty())
                return false;

            WorldPosition botPos(bot);
            GameObject* closestGo = nullptr;
            float closestDist = FLT_MAX;

            for (const GuidPosition& guidPos : gos)
            {
                GameObject* go = ai->GetGameObject(guidPos);
                if (!go)
                    continue;

                if (go->GetLootState() != GO_READY)
                    continue;

                float dist = botPos.distance(WorldPosition(go));
                if (dist < closestDist)
                {
                    closestDist = dist;
                    closestGo = go;
                }
            }

            if (!closestGo)
                return false;

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
            {
                ai->TellPlayerNoFacing(GetMaster(), "Casting Disarm Trap on Suppression Device");
            }

            return ai->CastSpell(SPELL_DISARM_TRAP, closestGo);
        }

        bool isPossible() override
        {
            return bot->getClass() == CLASS_ROGUE && 
                   bot->HasSpell(SPELL_DISARM_TRAP) && 
                   ai->CanMove();
        }

        bool isUseful() override
        {
            std::list<GuidPosition> gos = AI_VALUE(std::list<GuidPosition>, "go usable filter::go trapped filter::entry filter::{gos close,suppression devices}");
            return !gos.empty();
        }
    };
}