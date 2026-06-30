#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"

namespace ai
{
    class MechanarEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MechanarEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable mechanar strategy", "+mechanar") {}
    };

    class MechanarDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        MechanarDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable mechanar strategy", "-mechanar") {}
    };

    class NethermancerSepethreaEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        NethermancerSepethreaEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable nethermancer sepethrea fight strategy", "+nethermancer sepethrea") {}
    };

    class NethermancerSepethreaDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        NethermancerSepethreaDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable nethermancer sepethrea fight strategy", "-nethermancer sepethrea") {}
    };

    class RagingFlamesMoveAwayAction : public MoveAwayFromCreature
    {
    public:
        RagingFlamesMoveAwayAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "move away from raging flames", 20481, 20.0f) {}
    };
}