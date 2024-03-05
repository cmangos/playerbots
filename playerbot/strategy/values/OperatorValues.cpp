#include "playerbot/playerbot.h"
#include "playerbot/strategy/Value.h"
#include "OperatorValues.h"

namespace ai
{
    bool BoolAndValue::Calculate()
    {
        std::vector<std::string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (!AI_VALUE(bool, value))
                return false;
        }

        return true;
    }

    bool NotValue::Calculate()
    {
        std::vector<std::string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (AI_VALUE(bool, value))
                return false;
        }

        return true;
    }
}
