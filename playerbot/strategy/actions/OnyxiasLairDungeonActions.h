#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"

namespace ai
{
    class OnyxiasLairEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        OnyxiasLairEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable onyxias lair strategy", "+onyxias lair") {}
    };

    class OnyxiasLairDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        OnyxiasLairDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable onyxias lair strategy", "-onyxias lair") {}
    };

    class OnyxiaEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        OnyxiaEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable onyxia fight strategy", "+onyxia") {}
    };

    class OnyxiaDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        OnyxiaDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable onyxia fight strategy", "-onyxia") {}
    };
}
