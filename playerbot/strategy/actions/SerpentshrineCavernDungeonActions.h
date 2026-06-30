#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "playerbot/strategy/generic/SerpentshrineCavernDungeonStrategies.h"

namespace ai
{
    class SSCEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        SSCEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable serpentshrine cavern strategy", "+serpentshrine cavern") {}
    };

    class SSCDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        SSCDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable serpentshrine cavern strategy", "-serpentshrine cavern") {}
    };

    class LurkerBelowEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        LurkerBelowEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable lurker below fight strategy", "+lurker below") {}
    };

    class LurkerBelowDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        LurkerBelowDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable lurker below fight strategy", "-lurker below") {}
    };

    class LeotherasEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        LeotherasEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable leotheras fight strategy", "+leotheras") {}
    };

    class LeotherasDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        LeotherasDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable leotheras fight strategy", "-leotheras") {}
    };

    class VashjEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        VashjEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable vashj fight strategy", "+vashj") {}
    };

    class VashjDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        VashjDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable vashj fight strategy", "-vashj") {}
    };

    // Lurker Below: move behind boss during Spout
    class SSCLurkerMoveBehindBossAction : public MoveAwayFromCreature
    {
    public:
        SSCLurkerMoveBehindBossAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "ssc lurker move behind boss", SSC_NPC_LURKER_BELOW, 30.0f) {}
    };

    // Leotheras: flee from Whirlwind
    class SSCLeotherasFleeWhirlwindAction : public MoveAwayFromCreature
    {
    public:
        SSCLeotherasFleeWhirlwindAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "ssc leotheras flee whirlwind", SSC_NPC_LEOTHERAS, 25.0f) {}
    };

    // Vashj: move from group when having Static Charge
    class SSCVashjMoveFromGroupAction : public MovementAction
    {
    public:
        SSCVashjMoveFromGroupAction(PlayerbotAI* ai) : MovementAction(ai, "ssc vashj move from group") {}
        bool Execute(Event& event) override
        {
            Unit* target = AI_VALUE(Unit*, "master target");
            if (!target)
                target = bot;
            return Flee(target);
        }
    };
}
