#pragma once
#include "playerbot/strategy/Value.h"
#include "TargetValue.h"

namespace ai
{
    class FindLeastHpTargetStrategy : public FindNonCcTargetStrategy
    {
    public:
        FindLeastHpTargetStrategy(PlayerbotAI* ai) : FindNonCcTargetStrategy(ai)
        {
            minHealth = 0;
        }
    public:
        virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) override
        {
            // do not use this logic for pvp
            if (attacker->IsPlayer())
                return;

            if (IsCcTarget(attacker))
                return;

            if (!result || result->GetHealth() > attacker->GetHealth())
                result = attacker;
        }
    protected:
        float minHealth;
    };

    class LeastHpTargetValue : public TargetValue
	{
	public:
        LeastHpTargetValue(PlayerbotAI* ai, std::string name = "least hp target") : TargetValue(ai, name) {}

    public:
        ObjectGuid Calculate() override;
    };
}
