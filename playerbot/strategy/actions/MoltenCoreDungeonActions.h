#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"

namespace ai
{
    class MoveToMCRuneAction : public MoveToAction
    {
    public:
        MoveToMCRuneAction(PlayerbotAI* ai) : MoveToAction(ai, "move to mc rune") { qualifier = "entry filter::{gos in sight,mc runes}"; }
    };

    class DouseMCRuneAction : public UseItemIdAction
    {
    public:
        DouseMCRuneAction(PlayerbotAI* ai) : UseItemIdAction(ai, "douse mc rune") { qualifier = "{17333,entry filter::{gos close,mc runes}}"; }
    };


    class MoltenCoreActionContext : public NamedObjectContext<Action>
    {
    public:
        MoltenCoreActionContext()
        {
            creators["enable molten core strategy"] = [](PlayerbotAI* ai){return new ChangeAllStrategyAction(ai, "enable molten core strategy", "+molten core");};
            creators["disable molten core strategy"] = [](PlayerbotAI* ai){return new ChangeAllStrategyAction(ai, "disable molten core strategy", "-molten core");};

            creators["enable lucifron fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable lucifron fight strategy", "+lucifron");};
            creators["disable lucifron fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable lucifron fight strategy", "-lucifron");};
            creators["enable magmadar fight strategy"] = [](PlayerbotAI* ai){return new ChangeAllStrategyAction(ai, "enable magmadar fight strategy", "+magmadar");};
            creators["disable magmadar fight strategy"] = [](PlayerbotAI* ai){return new ChangeAllStrategyAction(ai, "disable magmadar fight strategy", "-magmadar");};
            creators["enable gehennas fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable gehennas fight strategy", "+gehennas");};
            creators["disable gehennas fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable gehennas fight strategy", "-gehennas");};
            creators["enable garr fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable garr fight strategy", "+garr");};
            creators["disable garr fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable garr fight strategy", "-garr");};
            creators["enable baron geddon fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable baron geddon fight strategy", "+baron geddon");};
            creators["disable baron geddon fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable baron geddon fight strategy", "-baron geddon");};
            creators["enable shazzrah fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable shazzrah fight strategy", "+shazzrah");};
            creators["disable shazzrah fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable shazzrah fight strategy", "-shazzrah");};
            creators["enable sulfuron fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable sulfuron fight strategy", "+sulfuron harbinger");};
            creators["disable sulfuron fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable sulfuron fight strategy", "-sulfuron harbinger");};
            creators["enable golemagg fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable golemagg fight strategy", "+golemagg");};
            creators["disable golemagg fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable golemagg fight strategy", "-golemagg");};
            creators["enable majordomo fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable majordomo fight strategy", "+majordomo");};
            creators["disable majordomo fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable majordomo fight strategy", "-majordomo");};
            creators["enable ragnaros fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable ragnaros fight strategy", "+ragnaros");};
            creators["disable ragnaros fight strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable ragnaros fight strategy", "-ragnaros");};

            creators["move away from magmadar"] = [](PlayerbotAI* ai){ return new MoveAwayFromCreature(ai, "move away from magmadar", 11982, 31.0f);};

            creators["move away from hazard"] = [](PlayerbotAI* ai){return new MoveAwayFromHazard(ai, "move away from magmadar lava bomb");};
            creators["move to mc rune"] = &MoltenCoreActionContext::move_to_mc_rune;
            creators["douse mc rune"] = &MoltenCoreActionContext::douse_mc_rune;
        }
    
      
        private:
        static Action* move_to_mc_rune(PlayerbotAI* ai) { return new MoveToMCRuneAction(ai); }
        static Action* douse_mc_rune(PlayerbotAI* ai) { return new DouseMCRuneAction(ai); }
    };
}