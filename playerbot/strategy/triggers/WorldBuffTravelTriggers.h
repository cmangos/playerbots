#pragma once
#include "playerbot/strategy/Trigger.h"
#include "playerbot/strategy/generic/WorldBuffTravelStrategy.h"

namespace ai
{
    class WorldBuffTravelZoneReachedTrigger : public Trigger
    {
    public:
        WorldBuffTravelZoneReachedTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel zone reached", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelNeedMoveTrigger : public Trigger
    {
    public:
        WorldBuffTravelNeedMoveTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel need move", 3) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelDMBuffedTrigger : public Trigger
    {
    public:
        WorldBuffTravelDMBuffedTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel dm buffed", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelDMExitedTrigger : public Trigger
    {
    public:
        WorldBuffTravelDMExitedTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel dm exited", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelDMPortalCastTrigger : public Trigger
    {
    public:
        WorldBuffTravelDMPortalCastTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel dm portal cast", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelDMPortalUseTrigger : public Trigger
    {
    public:
        WorldBuffTravelDMPortalUseTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel dm portal use", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelSongflowerTrigger : public Trigger
    {
    public:
        WorldBuffTravelSongflowerTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel songflower", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelPortalStepTrigger : public Trigger
    {
    public:
        WorldBuffTravelPortalStepTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel portal step", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelUsePortalTrigger : public Trigger
    {
    public:
        WorldBuffTravelUsePortalTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel use portal", 2) {
        }

        bool IsActive() override;
    };

    class WorldBuffTravelDoneTrigger : public Trigger
    {
    public:
        WorldBuffTravelDoneTrigger(PlayerbotAI* ai)
            : Trigger(ai, "world buff travel done", 2) {
        }

        bool IsActive() override;
    };
}