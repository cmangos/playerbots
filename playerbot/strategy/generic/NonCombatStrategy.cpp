
#include "playerbot/playerbot.h"
#include "NonCombatStrategy.h"
#include "playerbot/strategy/Value.h"

using namespace ai;

void NonCombatStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("check mount state", 1.0f), new NextAction("check values", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "near dark portal",
        NextAction::array(0, new NextAction("move to dark portal", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal azeroth",
        NextAction::array(0, new NextAction("use dark portal azeroth", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "at dark portal outland",
        NextAction::array(0, new NextAction("move from dark portal", 1.0f), NULL)));

    /*
    triggers.push_back(new TriggerNode(
        "vehicle near",
        NextAction::array(0, new NextAction("enter vehicle", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0, new NextAction("use lightwell", 80.0f), NULL)));
    */
}

void CollisionStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    triggers.push_back(new TriggerNode(
        "collision",
        NextAction::array(0, new NextAction("move out of collision", 2.0f), NULL)));
}

void MountStrategy::InitNonCombatTriggers(std::list<TriggerNode*> &triggers)
{
    /*triggers.push_back(new TriggerNode(
        "no possible targets",
        NextAction::array(0, new NextAction("mount", 1.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "no rpg target",
        NextAction::array(0, new NextAction("mount", 1.0f), NULL)));*/

    /*triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("mount", 4.0f), NULL)));*/
}

void WorldBuffStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "need world buff",
        NextAction::array(0, new NextAction("world buff", 1.0f), NULL)));
}

void WorldBuffStrategy::OnStrategyRemoved(BotState state)
{
    // Remove world buffs
    Player* bot = ai->GetBot();
    if (bot)
    {
        for (auto& wb : sPlayerbotAIConfig.worldBuffs)
        {
            if (bot->HasAura(wb.spellId))
            {
                bot->RemoveAurasDueToSpell(wb.spellId);
            }
        }
    }
}

void NoWarStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "at war",
        NextAction::array(0, new NextAction("faction", 1.0f), NULL)));
}

void GlyphStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "apply glyphs",
        NextAction::array(0, new NextAction("auto set glyph", 1.0f), NULL)));
}

