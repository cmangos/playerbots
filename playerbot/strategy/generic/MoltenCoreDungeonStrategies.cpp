
#include "playerbot/playerbot.h"
#include "MoltenCoreDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

void MoltenCoreDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start lucifron fight",
        NextAction::array(0, new NextAction("enable lucifron fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start magmadar fight",
        NextAction::array(0, new NextAction("enable magmadar fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start gehennas fight",
        NextAction::array(0, new NextAction("enable gehennas fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start garr fight",
        NextAction::array(0, new NextAction("enable garr fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start baron geddon fight",
        NextAction::array(0, new NextAction("enable baron geddon fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start shazzrah fight",
        NextAction::array(0, new NextAction("enable shazzrah fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start sulfuron harbinger fight",
        NextAction::array(0, new NextAction("enable sulfuron harbinger fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start golemagg fight",
        NextAction::array(0, new NextAction("enable golemagg fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start majordomo executus fight",
        NextAction::array(0, new NextAction("enable majordomo executus fight strategy", 100.0f), NULL)));
    
    triggers.push_back(new TriggerNode(
        "start ragnaros fight",
        NextAction::array(0, new NextAction("enable ragnaros fight strategy", 100.0f), NULL)));
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
        NextAction::array(0, new NextAction("douse mc rune", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "fire protection potion ready",
        NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

// Lucifron Fight Strategy
void LucifronFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
}

void LucifronFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end lucifron fight",
        NextAction::array(0, new NextAction("disable lucifron fight strategy", 100.0f), NULL)));
}

void LucifronFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end lucifron fight",
        NextAction::array(0, new NextAction("disable lucifron fight strategy", 100.0f), NULL)));
}

void LucifronFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void LucifronFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
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
    

// Gehennas Fight Strategy
void GehennasFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
}

void GehennasFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end gehennas fight",
        NextAction::array(0, new NextAction("disable gehennas fight strategy", 100.0f), NULL)));
}

void GehennasFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end gehennas fight",
        NextAction::array(0, new NextAction("disable gehennas fight strategy", 100.0f), NULL)));
}

void GehennasFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void GehennasFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}

// Garr Fight Strategy
void GarrFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
}

void GarrFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end garr fight",
        NextAction::array(0, new NextAction("disable garr fight strategy", 100.0f), NULL)));
}

void GarrFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end garr fight",
        NextAction::array(0, new NextAction("disable garr fight strategy", 100.0f), NULL)));
}

void GarrFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void GarrFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}

// Baron Geddon Fight Strategy
void BaronGeddonFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
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

void BaronGeddonFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void BaronGeddonFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}

// Shazzrah Fight Strategy
void ShazzrahFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
}

void ShazzrahFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end shazzrah fight",
        NextAction::array(0, new NextAction("disable shazzrah fight strategy", 100.0f), NULL)));
}

void ShazzrahFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end shazzrah fight",
        NextAction::array(0, new NextAction("disable shazzrah fight strategy", 100.0f), NULL)));
}

void ShazzrahFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void ShazzrahFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}

// Sulfuron Harbinger Fight Strategy
void SulfuronHarbingerFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
}

void SulfuronHarbingerFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end sulfuron fight",
        NextAction::array(0, new NextAction("disable sulfuron fight strategy", 100.0f), NULL)));
}

void SulfuronHarbingerFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end sulfuron fight",
        NextAction::array(0, new NextAction("disable sulfuron fight strategy", 100.0f), NULL)));
}

void SulfuronHarbingerFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void SulfuronHarbingerFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}

// Golemagg Fight Strategy
void GolemaggFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
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

void GolemaggFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void GolemaggFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}

// Ragnaros Fight Strategy
void RagnarosFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific combat triggers
}

void RagnarosFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end ragnaros fight",
        NextAction::array(0, new NextAction("disable ragnaros fight strategy", 100.0f), NULL)));
}

void RagnarosFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end ragnaros fight",
        NextAction::array(0, new NextAction("disable ragnaros fight strategy", 100.0f), NULL)));
}

void RagnarosFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
    // No specific reaction triggers
}

void RagnarosFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    // No specific combat multipliers
}