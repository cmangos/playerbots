
#include "playerbot/playerbot.h"
#include "GruulsLairDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

// Gruul Ground Slam -> Shatter: disable all actions when stunned
class GruulGroundSlamMultiplier : public Multiplier
{
public:
    GruulGroundSlamMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gruul ground slam") {}

    float GetValue(Action* action) override
    {
        if (!action)
            return 1.0f;

        // When knocked up by Ground Slam, disable everything except flee
        if (bot->HasAura(GL_SPELL_GROUND_SLAM) || bot->HasAura(GL_SPELL_GROUND_SLAM_2))
        {
            const std::string& name = action->getName();
            if (name == "gruul shatter spread" || name == "flee")
                return 1.0f;
            return 0.0f;
        }

        return 1.0f;
    }
};

void GruulsLairDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start gruul fight",
        NextAction::array(0, new NextAction("enable gruul fight strategy", 100.0f), NULL)));
}

void GruulFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Shatter: spread out when Ground Slam is cast
    triggers.push_back(new TriggerNode(
        "gruul incoming shatter",
        NextAction::array(0, new NextAction("gruul shatter spread", 100.0f), NULL)));
}

void GruulFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end gruul fight",
        NextAction::array(0, new NextAction("disable gruul fight strategy", 100.0f), NULL)));
}

void GruulFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end gruul fight",
        NextAction::array(0, new NextAction("disable gruul fight strategy", 100.0f), NULL)));
}

void GruulFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new GruulGroundSlamMultiplier(ai));
}
