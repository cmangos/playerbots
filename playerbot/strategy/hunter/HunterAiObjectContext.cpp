
#include "playerbot/playerbot.h"
#include "HunterActions.h"
#include "HunterTriggers.h"
#include "HunterAiObjectContext.h"
#include "BeastMasteryHunterStrategy.h"
#include "MarksmanshipHunterStrategy.h"
#include "SurvivalHunterStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    namespace hunter
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = &hunter::StrategyFactoryInternal::aoe;
                creators["buff"] = &hunter::StrategyFactoryInternal::buff;
                creators["pull"] = &hunter::StrategyFactoryInternal::pull;
                creators["cc"] = &hunter::StrategyFactoryInternal::cc;
                creators["boost"] = &hunter::StrategyFactoryInternal::boost;
                creators["pet"] = &hunter::StrategyFactoryInternal::pet;
            }

        private:
            static Strategy* aoe(PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); }
            static Strategy* buff(PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "serpent sting"); }
            static Strategy* cc(PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); }
            static Strategy* pet(PlayerbotAI* ai) { return new HunterPetStrategy(ai); }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe beast mastery pve"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_beast_mastery_pve;
                creators["aoe beast mastery pvp"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_beast_mastery_pvp;
                creators["aoe beast mastery raid"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_beast_mastery_raid;
                creators["aoe marksmanship pve"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_marksmanship_pve;
                creators["aoe marksmanship pvp"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_marksmanship_pvp;
                creators["aoe marksmanship raid"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_marksmanship_raid;
                creators["aoe survival pve"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_survival_pve;
                creators["aoe survival pvp"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_survival_pvp;
                creators["aoe survival raid"] = &hunter::AoeSituationStrategyFactoryInternal::aoe_survival_raid;
            }

        private:
            static Strategy* aoe_beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterAoePveStrategy(ai); }
            static Strategy* aoe_beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterAoePvpStrategy(ai); }
            static Strategy* aoe_beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterAoeRaidStrategy(ai); }
            static Strategy* aoe_marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterAoePveStrategy(ai); }
            static Strategy* aoe_marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterAoePvpStrategy(ai); }
            static Strategy* aoe_marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterAoeRaidStrategy(ai); }
            static Strategy* aoe_survival_pve(PlayerbotAI* ai) { return new SurvivalHunterAoePveStrategy(ai); }
            static Strategy* aoe_survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterAoePvpStrategy(ai); }
            static Strategy* aoe_survival_raid(PlayerbotAI* ai) { return new SurvivalHunterAoeRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff beast mastery pve"] = &hunter::BuffSituationStrategyFactoryInternal::buff_beast_mastery_pve;
                creators["buff beast mastery pvp"] = &hunter::BuffSituationStrategyFactoryInternal::buff_beast_mastery_pvp;
                creators["buff beast mastery raid"] = &hunter::BuffSituationStrategyFactoryInternal::buff_beast_mastery_raid;
                creators["buff marksmanship pve"] = &hunter::BuffSituationStrategyFactoryInternal::buff_marksmanship_pve;
                creators["buff marksmanship pvp"] = &hunter::BuffSituationStrategyFactoryInternal::buff_marksmanship_pvp;
                creators["buff marksmanship raid"] = &hunter::BuffSituationStrategyFactoryInternal::buff_marksmanship_raid;
                creators["buff survival pve"] = &hunter::BuffSituationStrategyFactoryInternal::buff_survival_pve;
                creators["buff survival pvp"] = &hunter::BuffSituationStrategyFactoryInternal::buff_survival_pvp;
                creators["buff survival raid"] = &hunter::BuffSituationStrategyFactoryInternal::buff_survival_raid;
            }

        private:
            static Strategy* buff_beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterBuffPveStrategy(ai); }
            static Strategy* buff_beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterBuffPvpStrategy(ai); }
            static Strategy* buff_beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterBuffRaidStrategy(ai); }
            static Strategy* buff_marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterBuffPveStrategy(ai); }
            static Strategy* buff_marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterBuffPvpStrategy(ai); }
            static Strategy* buff_marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterBuffRaidStrategy(ai); }
            static Strategy* buff_survival_pve(PlayerbotAI* ai) { return new SurvivalHunterBuffPveStrategy(ai); }
            static Strategy* buff_survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterBuffPvpStrategy(ai); }
            static Strategy* buff_survival_raid(PlayerbotAI* ai) { return new SurvivalHunterBuffRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost beast mastery pve"] = &hunter::BoostSituationStrategyFactoryInternal::boost_beast_mastery_pve;
                creators["boost beast mastery pvp"] = &hunter::BoostSituationStrategyFactoryInternal::boost_beast_mastery_pvp;
                creators["boost beast mastery raid"] = &hunter::BoostSituationStrategyFactoryInternal::boost_beast_mastery_raid;
                creators["boost marksmanship pve"] = &hunter::BoostSituationStrategyFactoryInternal::boost_marksmanship_pve;
                creators["boost marksmanship pvp"] = &hunter::BoostSituationStrategyFactoryInternal::boost_marksmanship_pvp;
                creators["boost marksmanship raid"] = &hunter::BoostSituationStrategyFactoryInternal::boost_marksmanship_raid;
                creators["boost survival pve"] = &hunter::BoostSituationStrategyFactoryInternal::boost_survival_pve;
                creators["boost survival pvp"] = &hunter::BoostSituationStrategyFactoryInternal::boost_survival_pvp;
                creators["boost survival raid"] = &hunter::BoostSituationStrategyFactoryInternal::boost_survival_raid;
            }

        private:
            static Strategy* boost_beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterBoostPveStrategy(ai); }
            static Strategy* boost_beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterBoostPvpStrategy(ai); }
            static Strategy* boost_beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterBoostRaidStrategy(ai); }
            static Strategy* boost_marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterBoostPveStrategy(ai); }
            static Strategy* boost_marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterBoostPvpStrategy(ai); }
            static Strategy* boost_marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterBoostRaidStrategy(ai); }
            static Strategy* boost_survival_pve(PlayerbotAI* ai) { return new SurvivalHunterBoostPveStrategy(ai); }
            static Strategy* boost_survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterBoostPvpStrategy(ai); }
            static Strategy* boost_survival_raid(PlayerbotAI* ai) { return new SurvivalHunterBoostRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc marksmanship pve"] = &hunter::CcSituationStrategyFactoryInternal::cc_marksmanship_pve;
                creators["cc marksmanship pvp"] = &hunter::CcSituationStrategyFactoryInternal::cc_marksmanship_pvp;
                creators["cc marksmanship raid"] = &hunter::CcSituationStrategyFactoryInternal::cc_marksmanship_raid;
                creators["cc survival pve"] = &hunter::CcSituationStrategyFactoryInternal::cc_survival_pve;
                creators["cc survival pvp"] = &hunter::CcSituationStrategyFactoryInternal::cc_survival_pvp;
                creators["cc survival raid"] = &hunter::CcSituationStrategyFactoryInternal::cc_survival_raid;
                creators["cc beast mastery pve"] = &hunter::CcSituationStrategyFactoryInternal::cc_beast_mastery_pve;
                creators["cc beast mastery pvp"] = &hunter::CcSituationStrategyFactoryInternal::cc_beast_mastery_pvp;
                creators["cc beast mastery raid"] = &hunter::CcSituationStrategyFactoryInternal::cc_beast_mastery_raid;
            }

        private:
            static Strategy* cc_marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterCcPveStrategy(ai); }
            static Strategy* cc_marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterCcPvpStrategy(ai); }
            static Strategy* cc_marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterCcRaidStrategy(ai); }
            static Strategy* cc_survival_pve(PlayerbotAI* ai) { return new SurvivalHunterCcPveStrategy(ai); }
            static Strategy* cc_survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterCcPvpStrategy(ai); }
            static Strategy* cc_survival_raid(PlayerbotAI* ai) { return new SurvivalHunterCcRaidStrategy(ai); }
            static Strategy* cc_beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterCcPveStrategy(ai); }
            static Strategy* cc_beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterCcPvpStrategy(ai); }
            static Strategy* cc_beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterCcRaidStrategy(ai); }
        };

        class StingManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StingManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["sting"] = &hunter::StingManualStrategyFactoryInternal::sting;
                creators["sting serpent"] = &hunter::StingManualStrategyFactoryInternal::sting_serpent;
                creators["sting scorpid"] = &hunter::StingManualStrategyFactoryInternal::sting_scorpid;
                creators["sting viper"] = &hunter::StingManualStrategyFactoryInternal::sting_viper;
            }

        private:
            static Strategy* sting(PlayerbotAI* ai) { return new HunterStingPlaceholderStrategy(ai); }
            static Strategy* sting_serpent(PlayerbotAI* ai) { return new HunterManualStingStrategy(ai, "sting serpent", "serpent sting", "serpent sting"); }
            static Strategy* sting_scorpid(PlayerbotAI* ai) { return new HunterManualStingStrategy(ai, "sting scorpid", "scorpid sting", "scorpid sting"); }
            static Strategy* sting_viper(PlayerbotAI* ai) { return new HunterManualStingStrategy(ai, "sting viper", "viper sting", "viper sting"); }
        };

        class StingSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StingSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["sting marksmanship pve"] = &hunter::StingSituationStrategyFactoryInternal::sting_marksmanship_pve;
                creators["sting marksmanship pvp"] = &hunter::StingSituationStrategyFactoryInternal::sting_marksmanship_pvp;
                creators["sting marksmanship raid"] = &hunter::StingSituationStrategyFactoryInternal::sting_marksmanship_raid;
                creators["sting survival pve"] = &hunter::StingSituationStrategyFactoryInternal::sting_survival_pve;
                creators["sting survival pvp"] = &hunter::StingSituationStrategyFactoryInternal::sting_survival_pvp;
                creators["sting survival raid"] = &hunter::StingSituationStrategyFactoryInternal::sting_survival_raid;
                creators["sting beast mastery pve"] = &hunter::StingSituationStrategyFactoryInternal::sting_beast_mastery_pve;
                creators["sting beast mastery pvp"] = &hunter::StingSituationStrategyFactoryInternal::sting_beast_mastery_pvp;
                creators["sting beast mastery raid"] = &hunter::StingSituationStrategyFactoryInternal::sting_beast_mastery_raid;
            }

        private:
            static Strategy* sting_marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterStingPveStrategy(ai); }
            static Strategy* sting_marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterStingPvpStrategy(ai); }
            static Strategy* sting_marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterStingRaidStrategy(ai); }
            static Strategy* sting_survival_pve(PlayerbotAI* ai) { return new SurvivalHunterStingPveStrategy(ai); }
            static Strategy* sting_survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterStingPvpStrategy(ai); }
            static Strategy* sting_survival_raid(PlayerbotAI* ai) { return new SurvivalHunterStingRaidStrategy(ai); }
            static Strategy* sting_beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterStingPveStrategy(ai); }
            static Strategy* sting_beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterStingPvpStrategy(ai); }
            static Strategy* sting_beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterStingRaidStrategy(ai); }
        };

        class AspectManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AspectManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aspect"] = &hunter::AspectManualStrategyFactoryInternal::aspect;
                creators["aspect hawk"] = &hunter::AspectManualStrategyFactoryInternal::aspect_hawk;
                creators["aspect monkey"] = &hunter::AspectManualStrategyFactoryInternal::aspect_monkey;
                creators["aspect cheetah"] = &hunter::AspectManualStrategyFactoryInternal::aspect_cheetah;
                creators["aspect pack"] = &hunter::AspectManualStrategyFactoryInternal::aspect_pack;
                creators["aspect beast"] = &hunter::AspectManualStrategyFactoryInternal::aspect_beast;
                creators["aspect wild"] = &hunter::AspectManualStrategyFactoryInternal::aspect_wild;
                creators["aspect viper"] = &hunter::AspectManualStrategyFactoryInternal::aspect_viper;
                creators["aspect dragonhawk"] = &hunter::AspectManualStrategyFactoryInternal::aspect_dragonhawk;
            }

        private:
            static Strategy* aspect(PlayerbotAI* ai) { return new HunterAspectPlaceholderStrategy(ai); }
            static Strategy* aspect_hawk(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect hawk", "aspect of the hawk", "aspect of the hawk"); }
            static Strategy* aspect_monkey(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect monkey", "aspect of the monkey", "aspect of the monkey"); }
            static Strategy* aspect_cheetah(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect cheetah", "aspect of the cheetah", "aspect of the cheetah"); }
            static Strategy* aspect_pack(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect pack", "aspect of the pack", "aspect of the pack"); }
            static Strategy* aspect_beast(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect beast", "aspect of the beast", "aspect of the beast"); }
            static Strategy* aspect_wild(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect wild", "aspect of the wild", "aspect of the wild"); }
            static Strategy* aspect_viper(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect viper", "aspect of the viper", "aspect of the viper"); }
            static Strategy* aspect_dragonhawk(PlayerbotAI* ai) { return new HunterManualAspectStrategy(ai, "aspect dragonhawk", "aspect of the dragonhawk", "aspect of the dragonhawk"); }
        };

        class AspectSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AspectSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aspect marksmanship pve"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_marksmanship_pve;
                creators["aspect marksmanship pvp"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_marksmanship_pvp;
                creators["aspect marksmanship raid"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_marksmanship_raid;
                creators["aspect survival pve"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_survival_pve;
                creators["aspect survival pvp"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_survival_pvp;
                creators["aspect survival raid"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_survival_raid;
                creators["aspect beast mastery pve"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_beast_mastery_pve;
                creators["aspect beast mastery pvp"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_beast_mastery_pvp;
                creators["aspect beast mastery raid"] = &hunter::AspectSituationStrategyFactoryInternal::aspect_beast_mastery_raid;
            }

        private:
            static Strategy* aspect_marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterAspectPveStrategy(ai); }
            static Strategy* aspect_marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterAspectPvpStrategy(ai); }
            static Strategy* aspect_marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterAspectRaidStrategy(ai); }
            static Strategy* aspect_survival_pve(PlayerbotAI* ai) { return new SurvivalHunterAspectPveStrategy(ai); }
            static Strategy* aspect_survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterAspectPvpStrategy(ai); }
            static Strategy* aspect_survival_raid(PlayerbotAI* ai) { return new SurvivalHunterAspectRaidStrategy(ai); }
            static Strategy* aspect_beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterAspectPveStrategy(ai); }
            static Strategy* aspect_beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterAspectPvpStrategy(ai); }
            static Strategy* aspect_beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterAspectRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["marksmanship"] = &hunter::ClassStrategyFactoryInternal::marksmanship;
                creators["survival"] = &hunter::ClassStrategyFactoryInternal::survival;
                creators["beast mastery"] = &hunter::ClassStrategyFactoryInternal::beast_mastery;
            }

        private:
            static Strategy* marksmanship(PlayerbotAI* ai) { return new MarksmanshipHunterPlaceholderStrategy(ai); }
            static Strategy* survival(PlayerbotAI* ai) { return new SurvivalHunterPlaceholderStrategy(ai); }
            static Strategy* beast_mastery(PlayerbotAI* ai) { return new BeastMasteryHunterPlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["beast mastery pvp"] = &hunter::ClassSituationStrategyFactoryInternal::beast_mastery_pvp;
                creators["beast mastery pve"] = &hunter::ClassSituationStrategyFactoryInternal::beast_mastery_pve;
                creators["beast mastery raid"] = &hunter::ClassSituationStrategyFactoryInternal::beast_mastery_raid;
                creators["marksmanship pvp"] = &hunter::ClassSituationStrategyFactoryInternal::marksmanship_pvp;
                creators["marksmanship pve"] = &hunter::ClassSituationStrategyFactoryInternal::marksmanship_pve;
                creators["marksmanship raid"] = &hunter::ClassSituationStrategyFactoryInternal::marksmanship_raid;
                creators["survival pvp"] = &hunter::ClassSituationStrategyFactoryInternal::survival_pvp;
                creators["survival pve"] = &hunter::ClassSituationStrategyFactoryInternal::survival_pve;
                creators["survival raid"] = &hunter::ClassSituationStrategyFactoryInternal::survival_raid;
            }

        private:
            static Strategy* beast_mastery_pvp(PlayerbotAI* ai) { return new BeastMasteryHunterPvpStrategy(ai); }
            static Strategy* beast_mastery_pve(PlayerbotAI* ai) { return new BeastMasteryHunterPveStrategy(ai); }
            static Strategy* beast_mastery_raid(PlayerbotAI* ai) { return new BeastMasteryHunterRaidStrategy(ai); }
            static Strategy* marksmanship_pvp(PlayerbotAI* ai) { return new MarksmanshipHunterPvpStrategy(ai); }
            static Strategy* marksmanship_pve(PlayerbotAI* ai) { return new MarksmanshipHunterPveStrategy(ai); }
            static Strategy* marksmanship_raid(PlayerbotAI* ai) { return new MarksmanshipHunterRaidStrategy(ai); }
            static Strategy* survival_pvp(PlayerbotAI* ai) { return new SurvivalHunterPvpStrategy(ai); }
            static Strategy* survival_pve(PlayerbotAI* ai) { return new SurvivalHunterPveStrategy(ai); }
            static Strategy* survival_raid(PlayerbotAI* ai) { return new SurvivalHunterRaidStrategy(ai); }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["black arrow"] = &TriggerFactoryInternal::black_arrow;
                creators["black arrow on snare target"] = &TriggerFactoryInternal::black_arrow_snare;
                creators["no stings"] = &TriggerFactoryInternal::NoStings;
                creators["hunters pet dead"] = &TriggerFactoryInternal::hunters_pet_dead;
                creators["hunters pet low health"] = &TriggerFactoryInternal::hunters_pet_low_health;
                creators["hunter's mark"] = &TriggerFactoryInternal::hunters_mark;
                creators["freezing trap"] = &TriggerFactoryInternal::freezing_trap;
                creators["frost trap"] = &TriggerFactoryInternal::frost_trap;
                creators["explosive trap"] = &TriggerFactoryInternal::explosive_trap;
                creators["rapid fire"] = &TriggerFactoryInternal::rapid_fire;
                creators["kill command"] = &TriggerFactoryInternal::kill_command;
                creators["aspect of the hawk"] = &TriggerFactoryInternal::aspect_of_the_hawk;
                creators["aspect of the wild"] = &TriggerFactoryInternal::aspect_of_the_wild;
                creators["aspect of the viper"] = &TriggerFactoryInternal::aspect_of_the_viper;
                creators["aspect of the monkey"] = &TriggerFactoryInternal::aspect_of_the_monkey;
                creators["aspect of the beast"] = &TriggerFactoryInternal::aspect_of_the_beast;
                creators["aspect of the cheetah"] = &TriggerFactoryInternal::aspect_of_the_cheetah;
                creators["aspect of the dragonhawk"] = &TriggerFactoryInternal::aspect_of_the_dragonhawk;
                creators["aspect of the pack"] = &TriggerFactoryInternal::aspect_of_the_pack;
                creators["trueshot aura"] = &TriggerFactoryInternal::trueshot_aura;
                creators["serpent sting on attacker"] = &TriggerFactoryInternal::serpent_sting_on_attacker;
                creators["viper sting on attacker"] = &TriggerFactoryInternal::viper_sting_on_attacker;
                creators["pet not happy"] = &TriggerFactoryInternal::pet_not_happy;
                creators["concussive shot on snare target"] = &TriggerFactoryInternal::concussive_shot_on_snare_target;
                creators["scare beast"] = &TriggerFactoryInternal::scare_beast;
                creators["low ammo"] = &TriggerFactoryInternal::low_ammo;
                creators["no ammo"] = &TriggerFactoryInternal::no_ammo;
                creators["has ammo"] = &TriggerFactoryInternal::has_ammo;
                creators["switch to melee"] = &TriggerFactoryInternal::switch_to_melee;
                creators["switch to ranged"] = &TriggerFactoryInternal::switch_to_ranged;
                creators["feign death"] = &TriggerFactoryInternal::feign_death;
                creators["scatter shot on snare target"] = &TriggerFactoryInternal::scatter_shot;
                creators["chimera shot"] = &TriggerFactoryInternal::chimera_shot;
                creators["explosive shot"] = &TriggerFactoryInternal::explosive_shot;
                creators["multi-shot"] = &TriggerFactoryInternal::multi_shot;
                creators["steady shot"] = &TriggerFactoryInternal::steady_shot;
                creators["intimidation on snare target"] = &TriggerFactoryInternal::intimidation;
                creators["counterattack"] = &TriggerFactoryInternal::counterattack;
                creators["wyvern sting"] = &TriggerFactoryInternal::wyvern_sting;
                creators["mongoose bite"] = &TriggerFactoryInternal::mongoose_bite;
                creators["viper sting"] = &TriggerFactoryInternal::viper_sting;
                creators["aimed shot"] = &TriggerFactoryInternal::aimed_shot;
                creators["bestial wrath"] = &TriggerFactoryInternal::bestial_wrath;
                creators["silencing shot interrupt"] = &TriggerFactoryInternal::silencing_shot_interrupt;
                creators["silencing shot on enemy healer"] = &TriggerFactoryInternal::silencing_shot_interrupt_healer;
                creators["no beast"] = &TriggerFactoryInternal::no_beast;
                creators["stealthed nearby"] = &TriggerFactoryInternal::stealthed_nearby;
            }

        private:
            static Trigger* silencing_shot_interrupt_healer(PlayerbotAI* ai) { return new SilencingShotInterruptHealerTrigger(ai); }
            static Trigger* silencing_shot_interrupt(PlayerbotAI* ai) { return new SilencingShotInterruptTrigger(ai); }
            static Trigger* bestial_wrath(PlayerbotAI* ai) { return new BestialWrathBoostTrigger(ai); }
            static Trigger* aimed_shot(PlayerbotAI* ai) { return new AimedShotTrigger(ai); }
            static Trigger* viper_sting(PlayerbotAI* ai) { return new ViperStingTrigger(ai); }
            static Trigger* black_arrow_snare(PlayerbotAI* ai) { return new BlackArrowSnareTrigger(ai); }
            static Trigger* mongoose_bite(PlayerbotAI* ai) { return new MongooseBiteCastTrigger(ai); }
            static Trigger* wyvern_sting(PlayerbotAI* ai) { return new WybernStingSnareTrigger(ai); }
            static Trigger* counterattack(PlayerbotAI* ai) { return new CounterattackCanCastTrigger(ai); }
            static Trigger* intimidation(PlayerbotAI* ai) { return new IntimidationSnareTrigger(ai); }
            static Trigger* chimera_shot(PlayerbotAI* ai) { return new ChimeraShotCanCastTrigger(ai); }
            static Trigger* explosive_shot(PlayerbotAI* ai) { return new ExplosiveShotCanCastTrigger(ai); }
            static Trigger* multi_shot(PlayerbotAI* ai) { return new MultishotCanCastTrigger(ai); }
            static Trigger* steady_shot(PlayerbotAI* ai) { return new SteadyShotCanCastTrigger(ai); }
            static Trigger* scatter_shot(PlayerbotAI* ai) { return new ScatterShotSnareTrigger(ai); }
            static Trigger* scare_beast(PlayerbotAI* ai) { return new ScareBeastTrigger(ai); }
            static Trigger* concussive_shot_on_snare_target(PlayerbotAI* ai) { return new ConsussiveShotSnareTrigger(ai); }
            static Trigger* pet_not_happy(PlayerbotAI* ai) { return new HunterPetNotHappy(ai); }
            static Trigger* serpent_sting_on_attacker(PlayerbotAI* ai) { return new SerpentStingOnAttackerTrigger(ai); }
            static Trigger* viper_sting_on_attacker(PlayerbotAI* ai) { return new ViperStingOnAttackerTrigger(ai); }
            static Trigger* trueshot_aura(PlayerbotAI* ai) { return new TrueshotAuraTrigger(ai); }
            static Trigger* black_arrow(PlayerbotAI* ai) { return new BlackArrowTrigger(ai); }
            static Trigger* NoStings(PlayerbotAI* ai) { return new HunterNoStingsActiveTrigger(ai); }
            static Trigger* hunters_pet_dead(PlayerbotAI* ai) { return new HuntersPetDeadTrigger(ai); }
            static Trigger* hunters_pet_low_health(PlayerbotAI* ai) { return new HuntersPetLowHealthTrigger(ai); }
            static Trigger* hunters_mark(PlayerbotAI* ai) { return new HuntersMarkTrigger(ai); }
            static Trigger* freezing_trap(PlayerbotAI* ai) { return new FreezingTrapTrigger(ai); }
            static Trigger* frost_trap(PlayerbotAI* ai) { return new FrostTrapTrigger(ai); }
            static Trigger* explosive_trap(PlayerbotAI* ai) { return new ExplosiveTrapTrigger(ai); }
            static Trigger* rapid_fire(PlayerbotAI* ai) { return new RapidFireTrigger(ai); }
            static Trigger* kill_command(PlayerbotAI* ai) { return new KillCommandBoostTrigger(ai); }
            static Trigger* low_ammo(PlayerbotAI* ai) { return new HunterLowAmmoTrigger(ai); }
            static Trigger* no_ammo(PlayerbotAI* ai) { return new HunterNoAmmoTrigger(ai); }
            static Trigger* has_ammo(PlayerbotAI* ai) { return new HunterHasAmmoTrigger(ai); }
            static Trigger* switch_to_melee(PlayerbotAI* ai) { return new SwitchToMeleeTrigger(ai); }
            static Trigger* switch_to_ranged(PlayerbotAI* ai) { return new SwitchToRangedTrigger(ai); }
            static Trigger* feign_death(PlayerbotAI* ai) { return new FeignDeathTrigger(ai); }
            static Trigger* no_beast(PlayerbotAI* ai) { return new HunterNoPet(ai); }
            static Trigger* stealthed_nearby(PlayerbotAI* ai) { return new StealthedNearbyTrigger(ai); }
            static Trigger* aspect_of_the_viper(PlayerbotAI* ai) { return new AspectOfTheViperTrigger(ai); }
            static Trigger* aspect_of_the_pack(PlayerbotAI* ai) { return new AspectOfThePackTrigger(ai); }
            static Trigger* aspect_of_the_hawk(PlayerbotAI* ai) { return new AspectOfTheHawkTrigger(ai); }
            static Trigger* aspect_of_the_wild(PlayerbotAI* ai) { return new AspectOfTheWildTrigger(ai); }
            static Trigger* aspect_of_the_monkey(PlayerbotAI* ai) { return new AspectOfTheMonkeyTrigger(ai); }
            static Trigger* aspect_of_the_beast(PlayerbotAI* ai) { return new AspectOfTheBeastTrigger(ai); }
            static Trigger* aspect_of_the_cheetah(PlayerbotAI* ai) { return new AspectOfTheCheetahTrigger(ai); }
            static Trigger* aspect_of_the_dragonhawk(PlayerbotAI* ai) { return new AspectOfTheDragonhawkTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["auto shot"] = &AiObjectContextInternal::auto_shot;
                creators["aimed shot"] = &AiObjectContextInternal::aimed_shot;
                creators["chimera shot"] = &AiObjectContextInternal::chimera_shot;
                creators["explosive shot"] = &AiObjectContextInternal::explosive_shot;
                creators["arcane shot"] = &AiObjectContextInternal::arcane_shot;
                creators["tranquilizing shot"] = &AiObjectContextInternal::tranquilizing_shot;
                creators["concussive shot"] = &AiObjectContextInternal::concussive_shot;
                creators["distracting shot"] = &AiObjectContextInternal::distracting_shot;
                creators["multi-shot"] = &AiObjectContextInternal::multi_shot;
                creators["volley"] = &AiObjectContextInternal::volley;
                creators["serpent sting"] = &AiObjectContextInternal::serpent_sting;
                creators["serpent sting on attacker"] = &AiObjectContextInternal::serpent_sting_on_attacker;
                creators["viper sting on attacker"] = &AiObjectContextInternal::viper_sting_on_attacker;
                creators["wyvern sting"] = &AiObjectContextInternal::wyvern_sting;
                creators["viper sting"] = &AiObjectContextInternal::viper_sting;
                creators["scorpid sting"] = &AiObjectContextInternal::scorpid_sting;
                creators["hunter's mark"] = &AiObjectContextInternal::hunters_mark;
                creators["mend pet"] = &AiObjectContextInternal::mend_pet;
                creators["revive pet"] = &AiObjectContextInternal::revive_pet;
                creators["call pet"] = &AiObjectContextInternal::call_pet;
                creators["black arrow"] = &AiObjectContextInternal::black_arrow;
                creators["rapid fire"] = &AiObjectContextInternal::rapid_fire;
                creators["kill command"] = &AiObjectContextInternal::kill_command;
                creators["boost"] = &AiObjectContextInternal::rapid_fire;
                creators["readiness"] = &AiObjectContextInternal::readiness;
                creators["aspect of the monkey"] = &AiObjectContextInternal::aspect_of_the_monkey;
                creators["aspect of the hawk"] = &AiObjectContextInternal::aspect_of_the_hawk;
                creators["aspect of the wild"] = &AiObjectContextInternal::aspect_of_the_wild;
                creators["aspect of the viper"] = &AiObjectContextInternal::aspect_of_the_viper;
                creators["aspect of the pack"] = &AiObjectContextInternal::aspect_of_the_pack;
                creators["aspect of the cheetah"] = &AiObjectContextInternal::aspect_of_the_cheetah;
                creators["aspect of the beast"] = &AiObjectContextInternal::aspect_of_the_beast;
                creators["aspect of the dragonhawk"] = &AiObjectContextInternal::aspect_of_the_dragonhawk;
                creators["remove aspect of the cheetah"] = &AiObjectContextInternal::remove_aspect_of_the_cheetah;
                creators["trueshot aura"] = &AiObjectContextInternal::trueshot_aura;
                creators["feign death"] = &AiObjectContextInternal::feign_death;
                creators["wing clip"] = &AiObjectContextInternal::wing_clip;
                creators["raptor strike"] = &AiObjectContextInternal::raptor_strike;
                creators["feed pet"] = &AiObjectContextInternal::feed_pet;
                creators["bestial wrath"] = &AiObjectContextInternal::bestial_wrath;
                creators["scare beast"] = &AiObjectContextInternal::scare_beast;
                creators["scare beast on cc"] = &AiObjectContextInternal::scare_beast_on_cc;
                creators["remove feign death"] = &AiObjectContextInternal::remove_feign_death;
                creators["scatter shot"] = &AiObjectContextInternal::scatter_shot;
                creators["scatter shot on closest attacker targeting me"] = &AiObjectContextInternal::scatter_shot_closest_attacker_targeting_me;
                creators["intimidation"] = &AiObjectContextInternal::intimidation;
                creators["deterrence"] = &AiObjectContextInternal::deterrence;
                creators["counterattack"] = &AiObjectContextInternal::counterattack;
                creators["wyvern sting"] = &AiObjectContextInternal::wyvern_sting;
                creators["mongoose bite"] = &AiObjectContextInternal::mongoose_bite;
                creators["black arrow on snare target"] = &AiObjectContextInternal::black_arrow_snare;
                creators["silencing shot"] = &AiObjectContextInternal::silencing_shot;
                creators["silencing shot on enemy healer"] = &AiObjectContextInternal::silencing_shot_healer;
                creators["readiness"] = &AiObjectContextInternal::readiness;
                creators["steady shot"] = &AiObjectContextInternal::steady_shot;
                creators["tame beast"] = &AiObjectContextInternal::tame_beast;
                creators["flare"] = &AiObjectContextInternal::flare;
                creators["immolation trap"] = &AiObjectContextInternal::immolation_trap;
                creators["frost trap"] = &AiObjectContextInternal::frost_trap;
                creators["explosive trap"] = &AiObjectContextInternal::explosive_trap;
                creators["freezing trap"] = &AiObjectContextInternal::freezing_trap;
                creators["immolation trap on target"] = &AiObjectContextInternal::immolation_trap_on_target;
                creators["frost trap on target"] = &AiObjectContextInternal::frost_trap_on_target;
                creators["explosive trap on target"] = &AiObjectContextInternal::explosive_trap_on_target;
                creators["freezing trap on target"] = &AiObjectContextInternal::freezing_trap_on_target;
                creators["freezing trap on cc"] = &AiObjectContextInternal::freezing_trap_on_cc;
                creators["immolation trap in place"] = &AiObjectContextInternal::immolation_trap_in_place;
                creators["frost trap in place"] = &AiObjectContextInternal::frost_trap_in_place;
                creators["explosive trap in place"] = &AiObjectContextInternal::explosive_trap_in_place;
                creators["freezing trap in place"] = &AiObjectContextInternal::freezing_trap_in_place;
                creators["dismiss pet"] = &AiObjectContextInternal::dismiss_pet;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
            }

        private:
            static Action* flare(PlayerbotAI* ai) { return new CastFlareAction(ai); }
            static Action* steady_shot(PlayerbotAI* ai) { return new CastSteadyShotAction(ai); }
            static Action* silencing_shot_healer(PlayerbotAI* ai) { return new CastSilencingShotOnHealerAction(ai); }
            static Action* silencing_shot(PlayerbotAI* ai) { return new CastSilencingShotAction(ai); }
            static Action* black_arrow_snare(PlayerbotAI* ai) { return new CastBlackArrowSnareAction(ai); }
            static Action* mongoose_bite(PlayerbotAI* ai) { return new MongooseBiteAction(ai); }
            static Action* wyvern_sting(PlayerbotAI* ai) { return new WyvernStingSnareAction(ai); }
            static Action* counterattack(PlayerbotAI* ai) { return new CastCounterattackAction(ai); }
            static Action* deterrence(PlayerbotAI* ai) { return new DeterrenceAction(ai); }
            static Action* intimidation(PlayerbotAI* ai) { return new IntimidationAction(ai); }
            static Action* scatter_shot(PlayerbotAI* ai) { return new CastScatterShotAction(ai); }
            static Action* scatter_shot_closest_attacker_targeting_me(PlayerbotAI* ai) { return new CastScatterShotAction(ai); }
            static Action* scare_beast(PlayerbotAI* ai) { return new CastScareBeastAction(ai); }
            static Action* scare_beast_on_cc(PlayerbotAI* ai) { return new CastScareBeastCcAction(ai); }
            static Action* bestial_wrath(PlayerbotAI* ai) { return new CastBestialWrathAction(ai); }
            static Action* feed_pet(PlayerbotAI* ai) { return new FeedPetAction(ai); }
            static Action* feign_death(PlayerbotAI* ai) { return new CastFeignDeathAction(ai); }
            static Action* trueshot_aura(PlayerbotAI* ai) { return new CastTrueshotAuraAction(ai); }
            static Action* auto_shot(PlayerbotAI* ai) { return new CastAutoShotAction(ai); }
            static Action* aimed_shot(PlayerbotAI* ai) { return new CastAimedShotAction(ai); }
            static Action* chimera_shot(PlayerbotAI* ai) { return new CastChimeraShotAction(ai); }
            static Action* explosive_shot(PlayerbotAI* ai) { return new CastExplosiveShotAction(ai); }
            static Action* tranquilizing_shot(PlayerbotAI* ai) { return new CastTranquilizingShotAction(ai); }
            static Action* arcane_shot(PlayerbotAI* ai) { return new CastArcaneShotAction(ai); }
            static Action* concussive_shot(PlayerbotAI* ai) { return new CastConcussiveShotAction(ai); }
            static Action* distracting_shot(PlayerbotAI* ai) { return new CastDistractingShotAction(ai); }
            static Action* multi_shot(PlayerbotAI* ai) { return new CastMultiShotAction(ai); }
            static Action* volley(PlayerbotAI* ai) { return new CastVolleyAction(ai); }
            static Action* serpent_sting(PlayerbotAI* ai) { return new CastSerpentStingAction(ai); }
            static Action* serpent_sting_on_attacker(PlayerbotAI* ai) { return new CastSerpentStingOnAttackerAction(ai); }
            static Action* viper_sting_on_attacker(PlayerbotAI* ai) { return new CastViperStingOnAttackerAction(ai); }
            static Action* viper_sting(PlayerbotAI* ai) { return new CastViperStingAction(ai); }
            static Action* scorpid_sting(PlayerbotAI* ai) { return new CastScorpidStingAction(ai); }
            static Action* hunters_mark(PlayerbotAI* ai) { return new CastHuntersMarkAction(ai); }
            static Action* mend_pet(PlayerbotAI* ai) { return new CastMendPetAction(ai); }
            static Action* revive_pet(PlayerbotAI* ai) { return new CastRevivePetAction(ai); }
            static Action* call_pet(PlayerbotAI* ai) { return new CastCallPetAction(ai); }
            static Action* black_arrow(PlayerbotAI* ai) { return new CastBlackArrow(ai); }
            static Action* rapid_fire(PlayerbotAI* ai) { return new CastRapidFireAction(ai); }
            static Action* kill_command(PlayerbotAI* ai) { return new CastKillCommandAction(ai); }
            static Action* readiness(PlayerbotAI* ai) { return new CastReadinessAction(ai); }
            static Action* aspect_of_the_monkey(PlayerbotAI* ai) { return new CastAspectOfTheMonkeyAction(ai); }
            static Action* aspect_of_the_hawk(PlayerbotAI* ai) { return new CastAspectOfTheHawkAction(ai); }
            static Action* aspect_of_the_wild(PlayerbotAI* ai) { return new CastAspectOfTheWildAction(ai); }
            static Action* aspect_of_the_viper(PlayerbotAI* ai) { return new CastAspectOfTheViperAction(ai); }
            static Action* aspect_of_the_pack(PlayerbotAI* ai) { return new CastAspectOfThePackAction(ai); }
            static Action* aspect_of_the_cheetah(PlayerbotAI* ai) { return new CastAspectOfTheCheetahAction(ai); }
            static Action* aspect_of_the_beast(PlayerbotAI* ai) { return new CastAspectOfTheBeastAction(ai); }
            static Action* aspect_of_the_dragonhawk(PlayerbotAI* ai) { return new CastAspectOfTheDragonhawkAction(ai); }
            static Action* remove_aspect_of_the_cheetah(PlayerbotAI* ai) { return new RemoveBuffAction(ai, "aspect of the cheetah"); }
            static Action* wing_clip(PlayerbotAI* ai) { return new CastWingClipAction(ai); }
            static Action* raptor_strike(PlayerbotAI* ai) { return new CastRaptorStrikeAction(ai); }
            static Action* remove_feign_death(PlayerbotAI* ai) { return new RemoveFeignDeathAction(ai); }
            static Action* tame_beast(PlayerbotAI* ai) { return new TameBeastAction(ai); }
            static Action* immolation_trap(PlayerbotAI* ai) { return new CastImmolationTrapAction(ai); }
            static Action* explosive_trap(PlayerbotAI* ai) { return new CastExplosiveTrapAction(ai); }
            static Action* frost_trap(PlayerbotAI* ai) { return new CastFrostTrapAction(ai); }
            static Action* freezing_trap(PlayerbotAI* ai) { return new CastFreezingTrapAction(ai); }
            static Action* immolation_trap_on_target(PlayerbotAI* ai) { return new CastImmolationTrapOnTargetAction(ai); }
            static Action* explosive_trap_on_target(PlayerbotAI* ai) { return new CastExplosiveTrapOnTargetAction(ai); }
            static Action* frost_trap_on_target(PlayerbotAI* ai) { return new CastFrostTrapOnTargetAction(ai); }
            static Action* freezing_trap_on_target(PlayerbotAI* ai) { return new CastFreezingTrapOnTargetAction(ai); }
            static Action* freezing_trap_on_cc(PlayerbotAI* ai) { return new CastFreezingTrapOnCcAction(ai); }
            static Action* immolation_trap_in_place(PlayerbotAI* ai) { return new CastImmolationTrapInPlaceAction(ai); }
            static Action* explosive_trap_in_place(PlayerbotAI* ai) { return new CastExplosiveTrapInPlaceAction(ai); }
            static Action* frost_trap_in_place(PlayerbotAI* ai) { return new CastFrostTrapInPlaceAction(ai); }
            static Action* freezing_trap_in_place(PlayerbotAI* ai) { return new CastFreezingTrapInPlaceAction(ai); }
            static Action* dismiss_pet(PlayerbotAI* ai) { return new CastDismissPetAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdateHunterPveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdateHunterPvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdateHunterRaidStrategiesAction(ai); }
        };
    };
};

HunterAiObjectContext::HunterAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::hunter::StrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::StingManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::StingSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::AspectManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::hunter::AspectSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::hunter::AiObjectContextInternal());
    triggerContexts.Add(new ai::hunter::TriggerFactoryInternal());
}
