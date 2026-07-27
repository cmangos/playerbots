
#include "playerbot/playerbot.h"
#include "WaitForAttackAction.h"
#include "playerbot/strategy/generic/CombatStrategy.h"
#include "playerbot/strategy/values/PositionValue.h"

using namespace ai;

bool WaitForAttackKeepSafeDistanceAction::Execute(Event& event)
{  
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target)
    {
        WorldPosition basePos = WorldPosition(target);
        if (target->GetTarget())
        {
            target = target->GetTarget();
            basePos = WorldPosition(target);
            if (target->IsPlayer())
            {
                // If target's target is a bot, see if we can get the pull back pos
                // else we should just base it off of the player pos
                Player* player = (Player*)target;

                if (PlayerbotAI* pullerAI = player->GetPlayerbotAI())
                {
                    if (pullerAI->HasStrategy("pull back", BotState::BOT_STATE_COMBAT))
                    {
                        PositionMap& posMap = PAI_VALUE(PositionMap&, "position");
                        PositionEntry pullPosition = posMap["pull"];
                        if (pullPosition.valueSet)
                            basePos = pullPosition.Get();
                    }
                }
            }
        }

        if (target->IsAlive())
        {
            const float safeDistance = std::max(float(target->GetAttackDistance(bot) + ATTACK_DISTANCE), WaitForAttackStrategy::GetSafeDistance());
            const float safeDistanceThreshold = WaitForAttackStrategy::GetSafeDistanceThreshold();

            // Find the best point around the target.
            const WorldPosition bestPoint = GetBestPoint(basePos, (safeDistance - safeDistanceThreshold), safeDistance);
            if (bestPoint)
            {
                // Move to the best point
                bool success = MoveTo(bestPoint.getMapId(), bestPoint.getX(), bestPoint.getY(), bestPoint.getZ(), false, false, false, true);
                if (success)
                    WaitForReach(WorldPosition(bot).fDist(bestPoint));
                return success;
            }
        }
    }

    return false;
}

const ai::WorldPosition WaitForAttackKeepSafeDistanceAction::GetBestPoint(const WorldPosition& pos, float minDistance, float maxDistance) const
{
    const WorldPosition botPosition(bot);
    const WorldPosition targetPosition = pos;
    const int8 startDir = urand(0, 1) * 2 - 1;
    const float radiansIncrement = (5.0f / 180.0f) * (M_PI_F);
    const float startAngle = targetPosition.getAngleTo(botPosition) + urand(0.f,radiansIncrement) * startDir;
    const float distance = frand(minDistance, maxDistance);
    const std::list<ObjectGuid> enemies = AI_VALUE(std::list<ObjectGuid>, "possible targets no los");

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
    {
        for (uint32 dist = 0; dist < distance; dist++)
        {
            WorldPosition point = targetPosition + WorldPosition(0, dist * cos(startAngle), dist * sin(startAngle), 1.0f);
            Creature* wpCreature = bot->SummonCreature(1, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 1000.0f + dist * 100.0f);
        }
    }

    std::list<WorldPosition> points;

    for (float tryAngle = 0.0f; tryAngle < M_PI_F; tryAngle += radiansIncrement)
    {
        for (int8 tryDir = -1; tryAngle && tryDir < 1; tryDir += 2)
        {
            float pointAngle = startAngle;
            pointAngle += tryAngle * startDir * tryDir;

            WorldPosition point = targetPosition + WorldPosition(0, distance * cos(pointAngle), distance * sin(pointAngle), 1.0f);

            point.setZ(point.getHeight());

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
            {
                Creature* wpCreature = bot->SummonCreature(1, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f + tryAngle * 1000.0f);
            }

            // Check if the target is visible from the point
            if (!pos.IsInLineOfSight(WorldPosition(point.getMapId(), point.getX(), point.getY(), point.getZ() + bot->GetCollisionHeight())))
                continue;

            // Check if the point is not surrounded by other enemies
            if (IsEnemyClose(point, enemies))
                continue;

            // Check if the bot can move to this point.
            if (!botPosition.canPathTo(point,bot))
                continue;

            if (ai->HasStrategy("debug move", BotState::BOT_STATE_COMBAT))
            {
                Creature* wpCreature = bot->SummonCreature(15631, point.getX(), point.getY(), point.getZ(), 0.0f, TEMPSPAWN_TIMED_DESPAWN, 5000.0f + tryAngle * 1000.0f);
            }

            points.push_back(point);
        }
    }

    // Use a minimal move from where we are to a safe spot. If no points were found just use original pos.
    WorldPosition minimumMove = botPosition;
    if (!points.empty())
    {
        points.sort([botPosition](WorldPosition i, WorldPosition j) { return botPosition.fDist(i) < botPosition.fDist(j); });
        return points.front();
    }
    return minimumMove;
}

bool WaitForAttackKeepSafeDistanceAction::IsEnemyClose(const WorldPosition& point, const std::list<ObjectGuid>& enemies) const
{
    for (const ObjectGuid& enemyGUID : enemies)
    {
        Unit* enemy = ai->GetUnit(enemyGUID);
        if (enemy)
        {
            // If the enemy is visible in the same map
            if (enemy->IsWithinLOSInMap(bot))
            {
                // If the enemy is not neutral
                if (enemy->CanAttackOnSight(bot))
                {
                    const float enemyAttackRange = enemy->GetAttackDistance(bot) + ATTACK_DISTANCE;
                    const float distanceToPoint = WorldPosition(enemy).sqDistance(point);
                    if (distanceToPoint <= (enemyAttackRange * enemyAttackRange))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}
