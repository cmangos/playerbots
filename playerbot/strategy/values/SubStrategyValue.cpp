
#include "SubStrategyValue.h"

using namespace ai;

void SubStrategyValue::Set(string newValue)
{
	SetValues(value, newValue, allowedValues);
}


void SubStrategyValue::SetValues(string& currentValue, const string newValue, const string allowedValues)
{
	vector<string> currentValues = StrSplit(currentValue, ",");
	vector<string> newValues = StrSplit(newValue, ",");
	vector<string> allowedVals = StrSplit(allowedValues, ",");

	for (string& newVal : newValues)
	{
		char operation = newVal[0];

		if (operation == '-' || operation == '+' || operation == '~')
			newVal = newVal.substr(1);
		else
			operation = '+';

		if (allowedValues.size() && std::find(allowedVals.begin(), allowedVals.end(), newVal) == allowedVals.end())
			continue;

		auto found = std::find(currentValues.begin(), currentValues.end(), newVal);

		if (found == currentValues.end())
		{
			if (operation != '-')
				currentValues.push_back(newVal);
		}
		else
		{
			if (operation != '+')
				currentValues.erase(found);
		}
	}

	currentValue = "";

	for (string value : currentValues)
		currentValue += value + ",";

	if(!currentValues.empty())
		currentValue.pop_back();
}

bool SubStrategyValue::Load(string value)
{
    this->value = "";
    Set(value);
    return true;
}