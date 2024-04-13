#include "botpch.h"
#include "../../playerbot.h"
#include "AQRuinsDungeonStrategies.h"
#include "DungeonMultipliers.h"

using namespace ai;

void AQRuinsDungeonStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "start ossirian fight",
        NextAction::array(0, new NextAction("enable ossirian fight strategy", 100.0f), NULL)));
}

void AQRuinsDungeonStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
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

//     triggers.push_back(new TriggerNode(
//         "mc rune in sight",
//         NextAction::array(0, new NextAction("move to mc rune", 1.0f), NULL)));
// 
//     triggers.push_back(new TriggerNode(
//         "mc rune close",
//         NextAction::array(0, new NextAction("douse mc rune", 1.0f), NULL)));

//     triggers.push_back(new TriggerNode(
//         "fire protection potion ready",
//         NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
   Player* bot = ai->GetBot();
   if (ai->IsRanged(bot) || ai->IsHeal(bot))
   {
      triggers.push_back(new TriggerNode(
         "magmadar too close",
         NextAction::array(0, new NextAction("move away from kurinnaxx", 100.0f), NULL)));
   }

   //     triggers.push_back(new TriggerNode(
   //         "fire protection potion ready",
   //         NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
   triggers.push_back(new TriggerNode(
      "end kurinnaxx fight",
      NextAction::array(0, new NextAction("disable kurinnaxx fight strategy", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
   triggers.push_back(new TriggerNode(
      "end kurinnaxx fight",
      NextAction::array(0, new NextAction("disable kurinnaxx fight strategy", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
   //     triggers.push_back(new TriggerNode(
   //         "magmadar lava bomb",
   //         NextAction::array(0, new NextAction("move away from hazard", 100.0f), NULL)));
}

void KurinnaxxFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
   Player* bot = ai->GetBot();
   if (ai->IsRanged(bot) || ai->IsHeal(bot))
   {
      multipliers.push_back(new PreventMoveAwayFromCreatureOnReachToCastMultiplier(ai));
   }
}

void OssirianFightStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
    Player* bot = ai->GetBot();
    if (ai->IsRanged(bot) || ai->IsHeal(bot))
    {
        triggers.push_back(new TriggerNode(
            "magmadar too close",
            NextAction::array(0, new NextAction("move away from ossirian", 100.0f), NULL)));
    }

//     triggers.push_back(new TriggerNode(
//         "fire protection potion ready",
//         NextAction::array(0, new NextAction("fire protection potion", 100.0f), NULL)));
}

void OssirianFightStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end ossirian fight",
        NextAction::array(0, new NextAction("disable ossirian fight strategy", 100.0f), NULL)));
}

void OssirianFightStrategy::InitDeadTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "end ossirian fight",
        NextAction::array(0, new NextAction("disable ossirian fight strategy", 100.0f), NULL)));
}

void OssirianFightStrategy::InitReactionTriggers(std::list<TriggerNode*>& triggers)
{
//     triggers.push_back(new TriggerNode(
//         "magmadar lava bomb",
//         NextAction::array(0, new NextAction("move away from hazard", 100.0f), NULL)));
}

void OssirianFightStrategy::InitCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    Player* bot = ai->GetBot();
    if (ai->IsRanged(bot) || ai->IsHeal(bot))
    {
        multipliers.push_back(new PreventMoveAwayFromCreatureOnReachToCastMultiplier(ai));
    }
}
