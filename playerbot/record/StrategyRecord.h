#pragma once

#include "TriggerRecord.h"
#include <vector>

enum StrategyType : unsigned int
{
    STRATEGY_TYPE_GENERIC = 0,
    STRATEGY_TYPE_COMBAT = 1,
    STRATEGY_TYPE_NONCOMBAT = 2,
    STRATEGY_TYPE_TANK = 4,
    STRATEGY_TYPE_DPS = 8,
    STRATEGY_TYPE_HEAL = 16,
    STRATEGY_TYPE_RANGED = 32,
    STRATEGY_TYPE_MELEE = 64,
    STRATEGY_TYPE_REACTION = 128,
    VALID_STRATEGY_TYPE = STRATEGY_TYPE_GENERIC | STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_NONCOMBAT |
        STRATEGY_TYPE_TANK | STRATEGY_TYPE_DPS | STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED |
        STRATEGY_TYPE_MELEE | STRATEGY_TYPE_REACTION,
};

struct StrategyRecord
{
    const StrategyType Type;
    const std::string Name;
    const std::string Description;
    const std::vector<std::string> RelatedStrategies;
    const std::vector<TriggerRecord> Triggers;

    StrategyRecord(StrategyType type, std::string const& name, std::string const& description,
        std::vector<std::string> const& relatedStrategies, std::vector<TriggerRecord> const& triggers) :
            Type(type), Name(name), Description(description), RelatedStrategies(relatedStrategies), Triggers(triggers) {}
};
