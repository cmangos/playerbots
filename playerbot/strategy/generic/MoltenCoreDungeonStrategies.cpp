
#include "playerbot/playerbot.h"
#include "MoltenCoreDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

// ----- Garr AOE Disable Multiplier -----
// Prevents DPS bots from using AOE abilities during the Garr fight
// to avoid killing Firesworn adds prematurely
class GarrDisableDpsAoeMultiplier : public Multiplier
{
public:
    GarrDisableDpsAoeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "garr disable dps aoe") {}

    float GetValue(Action* action) override
    {
        if (!action)
            return 1.0f;

        Unit* garr = AI_VALUE2(Unit*, "find target", "garr");
        if (!garr)
            return 1.0f;

        // Check if this is an AOE action from a DPS bot
        Player* bot = ai->GetBot();
        if (!ai->IsTank(bot) && !ai->IsHeal(bot))
        {
            if (action->getThreatType() == ActionThreatType::ACTION_THREAT_AOE)
                return 0.0f;
        }

        return 1.0f;
    }
};

// ----- Baron Geddon Ability Multiplier -----
// Disables movement/casting actions when Living Bomb or Inferno is active
class BaronGeddonAbilityMultiplier : public Multiplier
{
public:
    BaronGeddonAbilityMultiplier(PlayerbotAI* ai) : Multiplier(ai, "baron geddon ability") {}

    float GetValue(Action* action) override
    {
        if (!action)
            return 1.0f;

        bool hasBomb = bot->HasAura(MC_SPELL_LIVING_BOMB);
        bool infernoActive = false;

        if (Unit* boss = AI_VALUE2(Unit*, "find target", "baron geddon"))
            infernoActive = boss->HasAura(MC_SPELL_INFERNO);

        if (hasBomb || infernoActive)
        {
            // Allow only dungeon-specific movement actions
            const std::string& name = action->getName();
            if (name == "mc move from group" || name == "mc move from baron geddon" ||
                name == "move away from hazard" || name == "follow" || name == "stay" ||
                name == "flee")
                return 1.0f;

            // Block other movement and casting
            return 0.0f;
        }

        return 1.0f;
    }
};

// ----- Golemagg Multiplier -----
// Manages Core Rager tanking and prevents AOE
class GolemaggMultiplier : public Multiplier
{
public:
    GolemaggMultiplier(PlayerbotAI* ai) : Multiplier(ai, "golemagg") {}

    float GetValue(Action* action) override
    {
        if (!action)
            return 1.0f;

        Unit* golemagg = AI_VALUE2(Unit*, "find target", "golemagg the incinerator");
        if (!golemagg)
            return 1.0f;

        // Prevent AOE from DPS bots (Core Ragers shouldn't be killed near Golemagg)
        Player* bot = ai->GetBot();
        if (!ai->IsTank(bot) && !ai->IsHeal(bot))
        {
            if (action->getThreatType() == ActionThreatType::ACTION_THREAT_AOE)
                return 0.0f;
        }

        return 1.0f;
    }
};

void MoltenCoreDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start magmadar fight",
        NextAction::array(0, new NextAction("enable magmadar fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start baron geddon fight",
        NextAction::array(0, new NextAction("enable baron geddon fight strategy", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "start golemagg fight",
        NextAction::array(0, new NextAction("enable golemagg fight strategy", 100.0f), NULL)));
}

void MoltenCoreDungeonStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new GarrDisableDpsAoeMultiplier(ai));
}

void MoltenCoreDungeonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /*
    triggers.push_back(new TriggerNode(
        "val::and::{"
        "action possible::use id::17333,"
        "has object::go usable filter::go trapped filter::entry filter::{gos in sight,mc runes},"
        "not::has object::entry filter::{gos close,mc runes}"
        "}",
        NextAction::array(0, new NextAction("move to::entry filter::{gos in sight,mc runes}", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "val::has object::go usable filter::entry filter::{gos close,mc runes}",
        NextAction::array(0, new NextAction("use id::{17333,entry filter::{gos close,mc runes}}", 1.0f), NULL)));
        */

    triggers.push_back(new TriggerNode(
        "mc rune in sight",
        NextAction::array(0, new NextAction("move to mc rune", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "mc rune close",
        NextAction::array(0,
            new NextAction("douse mc rune eternal", 2.0f),
            new NextAction("douse mc rune aqual", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    Player* bot = ai->GetBot();
    if (ai->IsRanged(bot) || ai->IsHeal(bot))
    {
        triggers.push_back(new TriggerNode(
            "magmadar too close",
            NextAction::array(0, new NextAction("move away from magmadar", 100.0f), NULL)));
    }

    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end magmadar fight",
        NextAction::array(0, new NextAction("disable magmadar fight strategy", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end magmadar fight",
        NextAction::array(0, new NextAction("disable magmadar fight strategy", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "magmadar lava bomb",
        NextAction::array(0, new NextAction("move away from hazard", 100.0f), NULL)));
}

void MagmadarFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    Player* bot = ai->GetBot();
    if (ai->IsRanged(bot) || ai->IsHeal(bot))
    {
        multipliers.push_back(new PreventMoveAwayFromCreatureOnReachToCastMultiplier(ai));
    }
}

// ----- Baron Geddon Fight Strategy -----

void BaronGeddonFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // Living Bomb: move 20 yards away from group
    triggers.push_back(new TriggerNode(
        "mc living bomb debuff",
        NextAction::array(0, new NextAction("mc move from group", 100.0f), NULL)));

    // Inferno: ranged/healers move 20 yards away from boss
    triggers.push_back(new TriggerNode(
        "mc baron geddon inferno",
        NextAction::array(0, new NextAction("mc move from baron geddon", 100.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void BaronGeddonFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end baron geddon fight",
        NextAction::array(0, new NextAction("disable baron geddon fight strategy", 100.0f), NULL)));
}

void BaronGeddonFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end baron geddon fight",
        NextAction::array(0, new NextAction("disable baron geddon fight strategy", 100.0f), NULL)));
}

void BaronGeddonFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new BaronGeddonAbilityMultiplier(ai));
}

// ----- Golemagg Fight Strategy -----

void GolemaggFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void GolemaggFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end golemagg fight",
        NextAction::array(0, new NextAction("disable golemagg fight strategy", 100.0f), NULL)));
}

void GolemaggFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end golemagg fight",
        NextAction::array(0, new NextAction("disable golemagg fight strategy", 100.0f), NULL)));
}

void GolemaggFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new GolemaggMultiplier(ai));
}
