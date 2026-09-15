
#include "playerbot/playerbot.h"
#include "DpsTargetValue.h"
#include "LeastHpTargetValue.h"

using namespace ai;


ObjectGuid DpsTargetValue::Calculate()
{
    ObjectGuid rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindLeastHpTargetStrategy strategy(ai);
    Unit* target = TargetValue::FindTarget(&strategy);
    return target ? target->GetObjectGuid() : ObjectGuid();
}

class FindMaxHpTargetStrategy : public FindTargetStrategy
{
public:
    FindMaxHpTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai)
    {
        maxHealth = 0;
    }

public:
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) override
    {
        Group* group = ai->GetBot()->GetGroup();
        if (group)
        {
            uint64 guid = group->GetTargetIcon(4);
            if (guid && attacker->GetObjectGuid() == ObjectGuid(guid))
                return;
        }
        if (!result || result->GetHealth() < attacker->GetHealth())
            result = attacker;
    }

protected:
    float maxHealth;
};

ObjectGuid DpsAoeTargetValue::Calculate()
{
    ObjectGuid rti = RtiTargetValue::Calculate();
    if (rti) return rti;

    FindMaxHpTargetStrategy strategy(ai);
    Unit* target = TargetValue::FindTarget(&strategy);
    return target ? target->GetObjectGuid() : ObjectGuid();
}
