// AhnQirajDungeonActions.h
#pragma once
#include "DungeonActions.h"      // For MoveAwayFromHazard (generic dungeon action type)
#include "CureAction.h"          // For CurePartyMemberAction
#include "TargetAction.h"        // For Taunt/Tank related actions


namespace ai
{
    // Action to move away from Kurinnaxx's Sand Trap
    class MoveAwayFromKurinaxxSandTrapAction : public MoveAwayFromHazard
    {
    public:
        MoveAwayFromKurinaxxSandTrapAction(PlayerbotAI* ai) : MoveAwayFromHazard(ai, "move away from kurinnaxx sand trap", 180630) {}
    };

    // Action to cure the Toxic Volley poison
    class CureKurinaxxToxicVolleyAction : public CurePartyMemberAction
    {
    public:
        // Targets party members who have the Toxic Volley aura (Spell ID 26056).
        // Assumes the bot has a poison dispel spell.
        CureKurinaxxToxicVolleyAction(PlayerbotAI* ai) : CurePartyMemberAction(ai, "cure toxic volley poison", DISPEL_POISON) {
            qualifier = "party member has aura::26056"; // Explicitly target players with this aura
        }
    };

    // Action to signal a tank to taunt/swap on Kurinnaxx
    // This action would typically be used by an off-tank bot to taunt.
    // The actual taunt spell would be class-specific (e.g., Warriors: Taunt, Druids: Growl).
    // The 'qualifier' here ensures it targets Kurinnaxx specifically.
    class TauntKurinaxxAction : public TauntAction
    {
    public:
        TauntKurinaxxAction(PlayerbotAI* ai) : TauntAction(ai, "taunt kurinnaxx") {
            // Only taunt if the target is Kurinnaxx (ID 15339)
            qualifier = "creature id::15339";
        }
    };

    // Action for the current tank to retreat/stop tanking for a swap
    // This is a generic 'flee' or 'move out' action triggered when Mortal Wound stacks are high.
    class KurinaxxTankRetreatAction : public FleeAction // Or MoveAwayFromCreature if specific distance is needed
    {
    public:
        KurinaxxTankRetreatAction(PlayerbotAI* ai) : FleeAction(ai, "kurinnaxx tank retreat") {
            // Flee from Kurinnaxx
            qualifier = "creature id::15339";
        }
    };

    class AhnQirajActionContext : public NamedObjectContext<Action>
    {
    public:
        AhnQirajActionContext()
        {
            // General Ahn'Qiraj Dungeon Strategy actions
            creators["enable ahn'qiraj strategy"] = [](PlayerbotAI* ai){return new ChangeAllStrategyAction(ai, "enable ahn'qiraj strategy", "+ahn'qiraj");};
            creators["disable ahn'qiraj strategy"] = [](PlayerbotAI* ai){return new ChangeAllStrategyAction(ai, "disable ahn'qiraj strategy", "-ahn'qiraj");};

            // Kurinnaxx Fight Strategy actions
            creators["enable kurinnaxx strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable kurinnaxx strategy", "+kurinnaxx");};
            creators["disable kurinnaxx strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable kurinnaxx strategy", "-kurinnaxx");};

            // Specific Kurinnaxx Encounter actions
            creators["move away from kurinnaxx sand trap"] = &AhnQirajActionContext::move_away_from_kurinnaxx_sand_trap;
            creators["cure toxic volley poison"] = &AhnQirajActionContext::cure_toxic_volley_poison;
            creators["taunt kurinnaxx"] = &AhnQirajActionContext::taunt_kurinnaxx;
            creators["kurinnaxx tank retreat"] = &AhnQirajActionContext::kurinnaxx_tank_retreat;

            // Add other AQ20 boss strategy actions here as you implement them, e.g.:
            // creators["enable rajaxx strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "enable rajaxx strategy", "+rajaxx");};
            // creators["disable rajaxx strategy"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai, "disable rajaxx strategy", "-rajaxx");};
        }

        private:
        // Private static helper functions for creating Kurinnaxx-specific actions
        static Action* move_away_from_kurinnaxx_sand_trap(PlayerbotAI* ai) { return new MoveAwayFromKurinaxxSandTrapAction(ai); }
        static Action* cure_toxic_volley_poison(PlayerbotAI* ai) { return new CureKurinaxxToxicVolleyAction(ai); }
        static Action* taunt_kurinnaxx(PlayerbotAI* ai) { return new TauntKurinaxxAction(ai); }
        static Action* kurinnaxx_tank_retreat(PlayerbotAI* ai) { return new KurinaxxTankRetreatAction(ai); }
    };
}