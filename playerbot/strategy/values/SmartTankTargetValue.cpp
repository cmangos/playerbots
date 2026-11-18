#include "playerbot/playerbot.h"
#include "SmartTankTargetValue.h"
#include "PossibleAttackTargetsValue.h"
#include <algorithm>

using namespace ai;

class FindTankTargetSmartStrategy : public FindNonCcTargetStrategy
{
public:
    FindTankTargetSmartStrategy(PlayerbotAI* ai) : FindNonCcTargetStrategy(ai) {}

public:
    // The filter of the old TankTargetValue: no crowd-controlled mob, and a valid target or one of the possible attack targets
    bool IsAllowed(Unit* creature)
    {
        Player* bot = ai->GetBot();
        AiObjectContext* context = ai->GetAiObjectContext();

        if (IsCcTarget(creature)) return false;

        if (!PossibleAttackTargetsValue::IsValid(creature, bot))
        {
            std::list<ObjectGuid> attackers = AI_VALUE(std::list<ObjectGuid>, "possible attack targets");
            if (std::find(attackers.begin(), attackers.end(), creature->GetObjectGuid()) == attackers.end())
                return false;
        }

        return true;
    }

    virtual void CheckAttacker(Unit* creature, ThreatManager* threatManager) override
    {
        Player* bot = ai->GetBot();

        if (!IsAllowed(creature)) return;

        // A mob another tank (bot or player) already holds stays theirs, raid icon or not
        if (IsHeldByOtherTank(creature)) return;

        if (!result || IsBetter(creature, threatManager, result))
        {
            result = creature;
            resultThreat = threatManager->getThreat(bot);
        }
    }

private:
    bool IsHeldByOtherTank(Unit* creature)
    {
        Unit* victim = creature->GetVictim();
        if (!victim || victim == ai->GetBot() || !victim->IsPlayer())
            return false;

        return PlayerbotAI::IsTank(static_cast<Player*>(victim));
    }

    bool IsBetter(Unit* creature, ThreatManager* threatManager, Unit* current)
    {
        Player* bot = ai->GetBot();

        // Loose mobs first, then mobs in melee range, then the rest
        const int newLevel = GetTier(creature);
        const int oldLevel = GetTier(current);
        if (newLevel != oldLevel)
            return newLevel > oldLevel;

        // Loose mobs: nearest first
        if (newLevel == 2)
            return bot->GetDistance(creature) < bot->GetDistance(current);

        // Small hysteresis against the target flipping between near-equal threats, as in TankTargetValue
        return resultThreat - threatManager->getThreat(bot) > 0.1f;
    }

    int GetTier(Unit* creature)
    {
        Player* bot = ai->GetBot();
        if (creature->GetVictim() != bot)
            return 2;

        if (bot->CanReachWithMeleeAttack(creature))
            return 1;

        return 0;
    }

    float resultThreat = 0.0f;
};

ObjectGuid SmartTankTargetValue::Calculate()
{
    Player* bot = ai->GetBot();
    ObjectGuid rti = RtiTargetValue::Calculate();
    if (rti)
    {
        Unit* rtiTarget = ai->GetUnit(rti);
        Unit* victim = rtiTarget ? rtiTarget->GetVictim() : nullptr;
        FindTankTargetSmartStrategy filter(ai);

        // Differs from mod-playerbots: the icon target is returned only while a non-tank player is its victim.
        // A mob another tank holds is left to that tank (the search skips it); with no victim, the bot as
        // victim, or a pet as victim the icon mob goes through the tier search, so loose mobs come first.
        if (victim && victim != bot && victim->IsPlayer() && !PlayerbotAI::IsTank(static_cast<Player*>(victim)) &&
            filter.IsAllowed(rtiTarget))
            return rti;
    }

    FindTankTargetSmartStrategy strategy(ai);
    Unit* target = FindTarget(&strategy);
    return target ? target->GetObjectGuid() : ObjectGuid();
}
