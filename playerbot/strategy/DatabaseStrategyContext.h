#pragma once

#include "NamedObjectContext.h"
#include "Strategy.h"

namespace ai
{
    class DatabaseStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        DatabaseStrategyContext();
    };
};
