#pragma once
#include "DungeonTriggers.h"

namespace ai
{
    class OnyxiasLairEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        OnyxiasLairEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter onyxias lair", "onyxias lair", 249) {}
    };

    class OnyxiasLairLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        OnyxiasLairLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave onyxias lair", "onyxias lair", 249) {}
    };

    class OnyxiaStartFightTrigger : public StartBossFightTrigger
    {
    public:
        OnyxiaStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start onyxia fight", "onyxia", 10184) {}
    };

    class OnyxiaEndFightTrigger : public EndBossFightTrigger
    {
    public:
        OnyxiaEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end onyxia fight", "onyxia", 10184) {}
    };

    // Mechanics
    class OnyxiaDeepBreathTrigger : public Trigger
    {
    public:
        OnyxiaDeepBreathTrigger(PlayerbotAI* botAI);
        bool IsActive() override;
    };

    class OnyxiaNearTailTrigger : public Trigger
    {
    public:
        OnyxiaNearTailTrigger(PlayerbotAI* botAI);
        bool IsActive() override;
    };

    class OnyxiaFireballSplashTrigger : public Trigger
    {
    public:
        OnyxiaFireballSplashTrigger(PlayerbotAI* botAI);
        bool IsActive() override;
    };

    class OnyxiaWhelpsSpawnTrigger : public Trigger
    {
    public:
        OnyxiaWhelpsSpawnTrigger(PlayerbotAI* botAI);
        bool IsActive() override;
    };

    class OnyxiaAvoidEggsTrigger : public Trigger
    {
    public:
        OnyxiaAvoidEggsTrigger(PlayerbotAI* botAI);
        bool IsActive() override;
    };
}