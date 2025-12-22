#pragma once
#include "playerbot/strategy/Trigger.h"

namespace ai
{
    class EnemyPlayerNear : public Trigger
    {
    public:
        EnemyPlayerNear(PlayerbotAI* ai) : Trigger(ai, "enemy player near") {}

    public:
        virtual bool IsActive() override;
    };

    class PlayerHasNoFlag : public Trigger
    {
    public:
        PlayerHasNoFlag(PlayerbotAI* ai) : Trigger(ai, "player has no flag") {}

    public:
        virtual bool IsActive() override;
    };

    class PlayerHasFlag : public Trigger
    {
    public:
        PlayerHasFlag(PlayerbotAI* ai) : Trigger(ai, "player has flag") {}

    public:
        virtual bool IsActive() override;
    };

    class EnemyFlagCarrierNear : public Trigger
    {
    public:
        EnemyFlagCarrierNear(PlayerbotAI* ai) : Trigger(ai, "enemy flagcarrier near") {}

    public:
        virtual bool IsActive() override;
    };

    class TeamFlagCarrierNear : public Trigger
    {
    public:
        TeamFlagCarrierNear(PlayerbotAI* ai) : Trigger(ai, "team flagcarrier near") {}

    public:
        virtual bool IsActive() override;
    };

    class TeamHasFlag : public Trigger
    {
    public:
        TeamHasFlag(PlayerbotAI* ai) : Trigger(ai, "team has flag") {}

    public:
        virtual bool IsActive() override;
    };

    class EnemyTeamHasFlag : public Trigger
    {
    public:
        EnemyTeamHasFlag(PlayerbotAI* ai) : Trigger(ai, "enemy team has flag") {}

    public:
        virtual bool IsActive() override;
    };

    class PlayerIsInBattleground : public Trigger
    {
    public:
        PlayerIsInBattleground(PlayerbotAI* ai) : Trigger(ai, "in battleground") {}

    public:
        virtual bool IsActive() override;
    };

    class BgWaitingTrigger : public Trigger
    {
    public:
        BgWaitingTrigger(PlayerbotAI* ai) : Trigger(ai, "bg waiting", 30) {}

    public:
        virtual bool IsActive() override;
    };

    class BgActiveTrigger : public Trigger
    {
    public:
        BgActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "bg active", 1) {}

    public:
        virtual bool IsActive() override;
    };

    class BgInviteActiveTrigger : public Trigger
    {
    public:
        BgInviteActiveTrigger(PlayerbotAI* ai) : Trigger(ai, "bg invite active", 10) {}

    public:
        virtual bool IsActive() override;
    };

    class BgEndedTrigger : public Trigger
    {
    public:
        BgEndedTrigger(PlayerbotAI* ai) : Trigger(ai, "bg ended", 10) {}

    public:
        virtual bool IsActive() override;
    };

    class PlayerIsInBattlegroundWithoutFlag : public Trigger
    {
    public:
        PlayerIsInBattlegroundWithoutFlag(PlayerbotAI* ai) : Trigger(ai, "in battleground without flag") {}

    public:
        virtual bool IsActive() override;
    };

    class PlayerWantsInBattlegroundTrigger : public Trigger
    {
    public:
        PlayerWantsInBattlegroundTrigger(PlayerbotAI* ai) : Trigger(ai, "wants in bg") {}

    public:
        virtual bool IsActive() override;
    };

    class VehicleNearTrigger : public Trigger
    {
    public:
        VehicleNearTrigger(PlayerbotAI* ai) : Trigger(ai, "vehicle near", 10) {}

    public:
        virtual bool IsActive() override;
    };

    class InVehicleTrigger : public Trigger, public Qualified
    {
    public:
        InVehicleTrigger(PlayerbotAI* ai) : Trigger(ai, "in vehicle"), Qualified() {}

    public:
        virtual bool IsActive() override;
    };
}
