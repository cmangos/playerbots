#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{

    class MCRuneInSightTrigger : public ValueTrigger
    {
    public:
        MCRuneInSightTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "mc rune in sight", 1)
        {
            qualifier = "and::{"
                "action possible::use id::17333,"
                "has object::go usable filter::go trapped filter::entry filter::{gos in sight,mc runes},"
                "not::has object::entry filter::{gos close,mc runes}"
                "}";
        }
    };

    class MCRuneCloseTrigger : public ValueTrigger
    {
    public:
        MCRuneCloseTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "mc rune close", 1) { qualifier = "has object::go usable filter::entry filter::{gos close,mc runes}"; }
    };


    class MoltenCoreTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        MoltenCoreTriggerContext()
        {
            creators["enter molten core"] = [](PlayerbotAI* ai){ return new EnterDungeonTrigger(ai, "enter molten core", "molten core", 409);};
            creators["leave molten core"] = [](PlayerbotAI* ai){ return new LeaveDungeonTrigger(ai, "leave molten core", "molten core", 409);};

            creators["start lucifron fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start lucifron fight", "lucifron", 12118);};
            creators["end lucifron fight"] = [](PlayerbotAI* ai) {return new EndBossFightTrigger(ai, "end lucifron fight", "lucifron", 12118);};
            creators["start magmadar fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start magmadar fight", "magmadar", 11982);};
            creators["end magmadar fight"] = [](PlayerbotAI* ai) {return new EndBossFightTrigger(ai, "end magmadar fight", "magmadar", 11982);};
            creators["start gehennas fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start gehennas fight", "gehennas", 12259);};
            creators["end gehennas fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end gehennas fight", "gehennas", 12259);};
            creators["start garr fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start garr fight", "garr", 12057);};
            creators["end garr fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end garr fight", "garr", 12057);};
            creators["start baron geddon fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start baron geddon fight", "baron geddon", 12056);};
            creators["end baron geddon fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end baron geddon fight", "baron geddon", 12056);};
            creators["start shazzrah fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start shazzrah fight", "shazzrah", 12264);};
            creators["end shazzrah fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end shazzrah fight", "shazzrah", 12264);};
            creators["start sulfuron harbinger fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start sulfuron harbinger fight", "sulfuron harbinger", 12098);};
            creators["end sulfuron harbinger fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end sulfuron harbinger fight", "sulfuron harbinger", 12098);};
            creators["start golemagg fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start golemagg fight", "golemagg", 11988);};
            creators["end golemagg fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end golemagg fight", "golemagg", 11988);};
            creators["start majordomo fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start majordomo fight", "majordomo", 12018);};
            creators["end majordomo fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end majordomo fight", "majordomo", 12018);};
            creators["start ragnaros fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start ragnaros fight", "ragnaros", 11502);};
            creators["end ragnaros fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end ragnaros fight", "ragnaros", 11502);};


            creators["magmadar lava bomb"] = [](PlayerbotAI* ai) {return new CloseToGameObjectHazardTrigger(ai, "magmadar lava bomb", 177704, 5.0f, 60);};
            creators["magmadar too close"] = [](PlayerbotAI* ai) {return new  CloseToCreatureTrigger(ai, "magmadar too close", 11982, 30.0f);};
            creators["fire protection potion ready"] = [](PlayerbotAI* ai){ return new ItemBuffReadyTrigger(ai, "fire protection potion ready", 13457, 17543);};

            creators["mc rune in sight"] = &MoltenCoreTriggerContext::mc_rune_in_sight;
            creators["mc rune close"] = &MoltenCoreTriggerContext::mc_rune_close;
        }
        private:
        static Trigger* mc_rune_in_sight(PlayerbotAI* ai) { return new MCRuneInSightTrigger(ai); };
        static Trigger* mc_rune_close(PlayerbotAI* ai) { return new MCRuneCloseTrigger(ai); };
    };

  
   

};