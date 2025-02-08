#pragma once

#include "ActionRecord.h"
#include <vector>

enum TriggerFlags : unsigned int
{
    TRIGGER_FLAG_NON_COMBAT = 1 << 0,
    TRIGGER_FLAG_COMBAT = 1 << 1,
    TRIGGER_FLAG_DEAD = 1 << 2,
    TRIGGER_FLAG_REACTION = 1 << 3,
    VALID_TRIGGER_FLAGS = TRIGGER_FLAG_NON_COMBAT | TRIGGER_FLAG_COMBAT |
        TRIGGER_FLAG_DEAD | TRIGGER_FLAG_REACTION,
};

struct TriggerRecord
{
    const std::string Name;
    const TriggerFlags Flags;
    const std::vector<ActionRecord> Actions;
};
