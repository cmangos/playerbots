#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"
#include "RtiTargetValue.h"

namespace ai
{
    // Tank target in three tiers: loose mobs (not attacking the bot) nearest first, then mobs in melee range,
    // then the rest. Within the melee and the rest tiers the mob with the lowest own threat wins, as in
    // mod-playerbots FindTankTargetSmartStrategy. The files are picked up by the values/*.cpp glob in CMakeLists.txt.
    class SmartTankTargetValue : public RtiTargetValue
    {
    public:
        SmartTankTargetValue(PlayerbotAI* ai, std::string type = "rti", std::string name = "tank target") : RtiTargetValue(ai, type, name) {}

    public:
        ObjectGuid Calculate() override;
    };
}
