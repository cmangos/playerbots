#pragma once
#include "playerbot/strategy/Action.h"
#include "MovementActions.h"
#include "UseMeetingStoneAction.h"
#include "playerbot/strategy/generic/WorldBuffTravelStrategy.h"

namespace ai
{
    class WorldBuffTravelApplyAction : public Action
    {
    public:
        WorldBuffTravelApplyAction(PlayerbotAI* ai)
            : Action(ai, "world buff travel apply") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override { return true; }

    private:
        void ApplyBuffsForStep(WorldBuffTravelStep step);
        void ApplyBuffToSelfAndRealPlayers(uint32 spellId);
        bool TakeFlightFromMaster(uint32 npcEntry, uint32 destTaxiNode);
        void AdvanceStep();

        bool TrySummonFarAwayMembers(WorldBuffTravelStep step);
    };

    class WorldBuffTravelSetTargetAction : public MovementAction
    {
    public:
        WorldBuffTravelSetTargetAction(PlayerbotAI* ai)
            : MovementAction(ai, "world buff travel set target") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
    };

    class WorldBuffTravelSongflowerAction : public MovementAction
    {
    public:
        WorldBuffTravelSongflowerAction(PlayerbotAI* ai)
            : MovementAction(ai, "world buff travel songflower") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override { return true; }

    private:
        void ApplyBuffToSelfAndRealPlayers(uint32 spellId);
    };

    class WorldBuffTravelDMBuffedAction : public Action
    {
    public:
        WorldBuffTravelDMBuffedAction(PlayerbotAI* ai)
            : Action(ai, "world buff travel dm buffed") {
        }

        bool Execute(Event& event) override;
    };

    class WorldBuffTravelDMExitedAction : public Action
    {
    public:
        WorldBuffTravelDMExitedAction(PlayerbotAI* ai)
            : Action(ai, "world buff travel dm exited") {
        }

        bool Execute(Event& event) override;
    };

    class WorldBuffTravelDMCastPortalAction : public Action
    {
    public:
        WorldBuffTravelDMCastPortalAction(PlayerbotAI* ai)
            : Action(ai, "world buff travel dm cast portal") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    class WorldBuffTravelDMTakePortalAction : public MovementAction
    {
    public:
        WorldBuffTravelDMTakePortalAction(PlayerbotAI* ai)
            : MovementAction(ai, "world buff travel dm take portal") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
    };

    class WorldBuffTravelCastPortalAction : public Action
    {
    public:
        WorldBuffTravelCastPortalAction(PlayerbotAI* ai)
            : Action(ai, "world buff travel cast portal") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override;
    };

    class WorldBuffTravelTakePortalAction : public MovementAction
    {
    public:
        WorldBuffTravelTakePortalAction(PlayerbotAI* ai)
            : MovementAction(ai, "world buff travel take portal") {
        }

        bool Execute(Event& event) override;
        bool isUseful() override { return true; }
    };

    class WorldBuffTravelFinishAction : public Action
    {
    public:
        WorldBuffTravelFinishAction(PlayerbotAI* ai)
            : Action(ai, "world buff travel finish") {
        }

        bool Execute(Event& event) override;
    };
}