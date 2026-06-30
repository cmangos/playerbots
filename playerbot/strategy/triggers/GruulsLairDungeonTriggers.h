#pragma once
#include "DungeonTriggers.h"
#include "playerbot/strategy/generic/GruulsLairDungeonStrategies.h"

namespace ai
{
    class GruulsLairEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        GruulsLairEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter gruul's lair", "gruul's lair", 565) {}
    };

    class GruulsLairLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        GruulsLairLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave gruul's lair", "gruul's lair", 565) {}
    };

    class GruulStartFightTrigger : public StartBossFightTrigger
    {
    public:
        GruulStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start gruul fight", "gruul the dragonkiller", GL_NPC_GRUUL) {}
    };

    class GruulEndFightTrigger : public EndBossFightTrigger
    {
    public:
        GruulEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end gruul fight", "gruul the dragonkiller", GL_NPC_GRUUL) {}
    };

    // Gruul Ground Slam -> Shatter: bot has Ground Slam debuff
    class GruulIncomingShatterTrigger : public Trigger
    {
    public:
        GruulIncomingShatterTrigger(PlayerbotAI* ai) : Trigger(ai, "gruul incoming shatter", 1) {}
        bool IsActive() override
        {
            return bot->HasAura(GL_SPELL_GROUND_SLAM) || bot->HasAura(GL_SPELL_GROUND_SLAM_2);
        }
    };
}
