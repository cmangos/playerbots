// AhnQirajDungeonTriggers.h
#pragma once
#include "DungeonTriggers.h" // For CloseToGameObjectHazardTrigger
#include "GenericTriggers.h" // For HasAuraTrigger and ValueTrigger (for Mortal Wound)
#include "HealthTriggers.h"  // For checking tank health/buffs


namespace ai
{
    // Trigger for Kurinnaxx's Sand Trap
    class KurinaxxSandTrapTrigger : public CloseToGameObjectHazardTrigger
    {
    public:
        // IMPORTANT: Verify GameObject ID 180630 for Kurinnaxx's Sand Trap on your server!
        // This is a common placeholder for similar effects, but actual ID might differ.
        KurinaxxSandTrapTrigger(PlayerbotAI* ai) : CloseToGameObjectHazardTrigger(ai, "kurinnaxx sand trap close", 180630, 8.0f, 60) {}
    };

    // Trigger for Kurinnaxx's Toxic Volley poison debuff
    class KurinaxxToxicVolleyTrigger : public HasAuraTrigger
    {
    public:
        // Spell ID 26056 is 'Toxic Volley' poison aura. Checks party members.
        KurinaxxToxicVolleyTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "toxic volley poison aura", 26056, AURA_CHECK_PASSIVE, 0, AURA_DETECTION_PARTY) {}
    };

    // Trigger for Kurinnaxx's Mortal Wound debuff (for tank swapping)
    class KurinaxxMortalWoundTrigger : public ValueTrigger
    {
    public:
        KurinaxxMortalWoundTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "kurinnaxx mortal wound high", 1)
        {
            // Trigger when current tank has 3 or more stacks of Mortal Wound (Spell ID 26038)
            // This is a common threshold for tank swaps in Classic WoW.
            qualifier = "and::{"
                "has aura::tank,26038,3," // Tank has Mortal Wound (26038) with at least 3 stacks
                "not::has aura::tank,26038,5" // And not at max stacks (5) to give some buffer
                "}";
        }
    };

    class AhnQirajTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        AhnQirajTriggerContext()
        {
            // Dungeon Entry/Exit Triggers for Ahn'Qiraj, The Ruined Kingdom (AQ20)
            creators["enter ahn'qiraj"] = [](PlayerbotAI* ai){ return new EnterDungeonTrigger(ai, "enter ahn'qiraj", "ahn'qiraj", 531);};
            creators["leave ahn'qiraj"] = [](PlayerbotAI* ai){ return new LeaveDungeonTrigger(ai, "leave ahn'qiraj", "ahn'qiraj", 531);};

            // Boss Fight Triggers for Kurinnaxx (AQ20)
            creators["start kurinnaxx fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start kurinnaxx fight", "kurinnaxx", 15339);};
            creators["end kurinnaxx fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end kurinnaxx fight", "kurinnaxx", 15339);};

            // Specific Kurinnaxx Encounter Triggers
            creators["kurinnaxx sand trap close"] = &AhnQirajTriggerContext::kurinnaxx_sand_trap_close;
            creators["toxic volley poison aura"] = &AhnQirajTriggerContext::toxic_volley_poison_aura;
            creators["kurinnaxx mortal wound high"] = &AhnQirajTriggerContext::kurinnaxx_mortal_wound_high;

            // Add other AQ20 boss triggers here as you implement them, e.g.:
            // creators["start rajaxx fight"] = [](PlayerbotAI* ai) { return new StartBossFightTrigger(ai, "start rajaxx fight", "rajaxx", 15338);};
            // creators["end rajaxx fight"] = [](PlayerbotAI* ai) { return new EndBossFightTrigger(ai, "end rajaxx fight", "rajaxx", 15338);};
        }
        private:
        // Private static helper functions for creating Kurinnaxx-specific triggers
        static Trigger* kurinnaxx_sand_trap_close(PlayerbotAI* ai) { return new KurinaxxSandTrapTrigger(ai); };
        static Trigger* toxic_volley_poison_aura(PlayerbotAI* ai) { return new KurinaxxToxicVolleyTrigger(ai); };
        static Trigger* kurinnaxx_mortal_wound_high(PlayerbotAI* ai) { return new KurinaxxMortalWoundTrigger(ai); };
    };
}