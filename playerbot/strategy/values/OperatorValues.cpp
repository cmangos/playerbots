#include "../../../botpch.h"
#include "playerbot/playerbot.h"
#include "../Value.h"
#include "OperatorValues.h"

namespace ai
{
    bool BoolAndValue::Calculate()
    {
        vector<string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (!AI_VALUE(bool, value))
                return false;
        }

        return true;
    }

    bool NotValue::Calculate()
    {
        vector<string> values = getMultiQualifiers(getQualifier(), ",");

        for (auto value : values)
        {
            if (AI_VALUE(bool, value))
                return false;
        }

        return true;
    }
}
