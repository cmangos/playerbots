
#include "playerbot/playerbot.h"
#include "WarlockActions.h"
#include "WarlockAiObjectContext.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "WarlockTriggers.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/actions/UseItemAction.h"
#include "AfflictionWarlockStrategy.h"
#include "DestructionWarlockStrategy.h"
#include "DemonologyWarlockStrategy.h"

namespace ai
{
    namespace warlock
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = &warlock::StrategyFactoryInternal::aoe;
                creators["buff"] = &warlock::StrategyFactoryInternal::buff;
                creators["boost"] = &warlock::StrategyFactoryInternal::boost;
                creators["pull"] = &warlock::StrategyFactoryInternal::pull;
                creators["cc"] = &warlock::StrategyFactoryInternal::cc;
            }

        private:
            static Strategy* aoe(PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); }
            static Strategy* buff(PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); }
            static Strategy* cc(PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe destruction pve"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_destruction_pve;
                creators["aoe destruction pvp"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_destruction_pvp;
                creators["aoe destruction raid"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_destruction_raid;
                creators["aoe demonology pve"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_demonology_pve;
                creators["aoe demonology pvp"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_demonology_pvp;
                creators["aoe demonology raid"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_demonology_raid;
                creators["aoe affliction pve"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_affliction_pve;
                creators["aoe affliction pvp"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_affliction_pvp;
                creators["aoe affliction raid"] = &warlock::AoeSituationStrategyFactoryInternal::aoe_affliction_raid;
            }

        private:
            static Strategy* aoe_destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockAoePveStrategy(ai); }
            static Strategy* aoe_destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockAoePvpStrategy(ai); }
            static Strategy* aoe_destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockAoeRaidStrategy(ai); }
            static Strategy* aoe_demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockAoePveStrategy(ai); }
            static Strategy* aoe_demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockAoePvpStrategy(ai); }
            static Strategy* aoe_demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockAoeRaidStrategy(ai); }
            static Strategy* aoe_affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockAoePveStrategy(ai); }
            static Strategy* aoe_affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockAoePvpStrategy(ai); }
            static Strategy* aoe_affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockAoeRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff destruction pve"] = &warlock::BuffSituationStrategyFactoryInternal::buff_destruction_pve;
                creators["buff destruction pvp"] = &warlock::BuffSituationStrategyFactoryInternal::buff_destruction_pvp;
                creators["buff destruction raid"] = &warlock::BuffSituationStrategyFactoryInternal::buff_destruction_raid;
                creators["buff demonology pve"] = &warlock::BuffSituationStrategyFactoryInternal::buff_demonology_pve;
                creators["buff demonology pvp"] = &warlock::BuffSituationStrategyFactoryInternal::buff_demonology_pvp;
                creators["buff demonology raid"] = &warlock::BuffSituationStrategyFactoryInternal::buff_demonology_raid;
                creators["buff affliction pve"] = &warlock::BuffSituationStrategyFactoryInternal::buff_affliction_pve;
                creators["buff affliction pvp"] = &warlock::BuffSituationStrategyFactoryInternal::buff_affliction_pvp;
                creators["buff affliction raid"] = &warlock::BuffSituationStrategyFactoryInternal::buff_affliction_raid;
            }

        private:
            static Strategy* buff_destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockBuffPveStrategy(ai); }
            static Strategy* buff_destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockBuffPvpStrategy(ai); }
            static Strategy* buff_destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockBuffRaidStrategy(ai); }
            static Strategy* buff_demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockBuffPveStrategy(ai); }
            static Strategy* buff_demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockBuffPvpStrategy(ai); }
            static Strategy* buff_demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockBuffRaidStrategy(ai); }
            static Strategy* buff_affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockBuffPveStrategy(ai); }
            static Strategy* buff_affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockBuffPvpStrategy(ai); }
            static Strategy* buff_affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockBuffRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost destruction pve"] = &warlock::BoostSituationStrategyFactoryInternal::boost_destruction_pve;
                creators["boost destruction pvp"] = &warlock::BoostSituationStrategyFactoryInternal::boost_destruction_pvp;
                creators["boost destruction raid"] = &warlock::BoostSituationStrategyFactoryInternal::boost_destruction_raid;
                creators["boost demonology pve"] = &warlock::BoostSituationStrategyFactoryInternal::boost_demonology_pve;
                creators["boost demonology pvp"] = &warlock::BoostSituationStrategyFactoryInternal::boost_demonology_pvp;
                creators["boost demonology raid"] = &warlock::BoostSituationStrategyFactoryInternal::boost_demonology_raid;
                creators["boost affliction pve"] = &warlock::BoostSituationStrategyFactoryInternal::boost_affliction_pve;
                creators["boost affliction pvp"] = &warlock::BoostSituationStrategyFactoryInternal::boost_affliction_pvp;
                creators["boost affliction raid"] = &warlock::BoostSituationStrategyFactoryInternal::boost_affliction_raid;
            }

        private:
            static Strategy* boost_destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockBoostPveStrategy(ai); }
            static Strategy* boost_destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockBoostPvpStrategy(ai); }
            static Strategy* boost_destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockBoostRaidStrategy(ai); }
            static Strategy* boost_demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockBoostPveStrategy(ai); }
            static Strategy* boost_demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockBoostPvpStrategy(ai); }
            static Strategy* boost_demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockBoostRaidStrategy(ai); }
            static Strategy* boost_affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockBoostPveStrategy(ai); }
            static Strategy* boost_affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockBoostPvpStrategy(ai); }
            static Strategy* boost_affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockBoostRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["demonology"] = &warlock::ClassStrategyFactoryInternal::demonology;
                creators["affliction"] = &warlock::ClassStrategyFactoryInternal::affliction;
                creators["destruction"] = &warlock::ClassStrategyFactoryInternal::destruction;
            }

        private:
            static Strategy* demonology(PlayerbotAI* ai) { return new DemonologyWarlockPlaceholderStrategy(ai); }
            static Strategy* affliction(PlayerbotAI* ai) { return new AfflictionWarlockPlaceholderStrategy(ai); }
            static Strategy* destruction(PlayerbotAI* ai) { return new DestructionWarlockPlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["destruction pvp"] = &warlock::ClassSituationStrategyFactoryInternal::destruction_pvp;
                creators["destruction pve"] = &warlock::ClassSituationStrategyFactoryInternal::destruction_pve;
                creators["destruction raid"] = &warlock::ClassSituationStrategyFactoryInternal::destruction_raid;
                creators["demonology pvp"] = &warlock::ClassSituationStrategyFactoryInternal::demonology_pvp;
                creators["demonology pve"] = &warlock::ClassSituationStrategyFactoryInternal::demonology_pve;
                creators["demonology raid"] = &warlock::ClassSituationStrategyFactoryInternal::demonology_raid;
                creators["affliction pvp"] = &warlock::ClassSituationStrategyFactoryInternal::affliction_pvp;
                creators["affliction pve"] = &warlock::ClassSituationStrategyFactoryInternal::affliction_pve;
                creators["affliction raid"] = &warlock::ClassSituationStrategyFactoryInternal::affliction_raid;
            }

        private:
            static Strategy* destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockPvpStrategy(ai); }
            static Strategy* destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockPveStrategy(ai); }
            static Strategy* destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockRaidStrategy(ai); }
            static Strategy* demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockPvpStrategy(ai); }
            static Strategy* demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockPveStrategy(ai); }
            static Strategy* demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockRaidStrategy(ai); }
            static Strategy* affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockPvpStrategy(ai); }
            static Strategy* affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockPveStrategy(ai); }
            static Strategy* affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockRaidStrategy(ai); }
        };

        class PetManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PetManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["pet"] = &warlock::PetManualStrategyFactoryInternal::pet;
                creators["pet imp"] = &warlock::PetManualStrategyFactoryInternal::pet_imp;
                creators["pet voidwalker"] = &warlock::PetManualStrategyFactoryInternal::pet_voidwalker;
                creators["pet succubus"] = &warlock::PetManualStrategyFactoryInternal::pet_succubus;
                creators["pet felhunter"] = &warlock::PetManualStrategyFactoryInternal::pet_felhunter;
                creators["pet felguard"] = &warlock::PetManualStrategyFactoryInternal::pet_felguard;
            }

        private:
            static Strategy* pet(PlayerbotAI* ai) { return new WarlockPetPlaceholderStrategy(ai); }
            static Strategy* pet_imp(PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet imp", "no imp", "summon imp"); }
            static Strategy* pet_voidwalker(PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet voidwalker", "no voidwalker", "summon voidwalker"); }
            static Strategy* pet_succubus(PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet succubus", "no succubus", "summon succubus"); }
            static Strategy* pet_felhunter(PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet felhunter", "no felhunter", "summon felhunter"); }
            static Strategy* pet_felguard(PlayerbotAI* ai) { return new WarlockManualPetStrategy(ai, "pet felguard", "no felguard", "summon felguard"); }
        };

        class PetSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            PetSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["pet destruction pve"] = &warlock::PetSituationStrategyFactoryInternal::pet_destruction_pve;
                creators["pet destruction pvp"] = &warlock::PetSituationStrategyFactoryInternal::pet_destruction_pvp;
                creators["pet destruction raid"] = &warlock::PetSituationStrategyFactoryInternal::pet_destruction_raid;
                creators["pet demonology pve"] = &warlock::PetSituationStrategyFactoryInternal::pet_demonology_pve;
                creators["pet demonology pvp"] = &warlock::PetSituationStrategyFactoryInternal::pet_demonology_pvp;
                creators["pet demonology raid"] = &warlock::PetSituationStrategyFactoryInternal::pet_demonology_raid;
                creators["pet affliction pve"] = &warlock::PetSituationStrategyFactoryInternal::pet_affliction_pve;
                creators["pet affliction pvp"] = &warlock::PetSituationStrategyFactoryInternal::pet_affliction_pvp;
                creators["pet affliction raid"] = &warlock::PetSituationStrategyFactoryInternal::pet_affliction_raid;
            }

        private:
            static Strategy* pet_destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockPetPveStrategy(ai); }
            static Strategy* pet_destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockPetPvpStrategy(ai); }
            static Strategy* pet_destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockPetRaidStrategy(ai); }
            static Strategy* pet_demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockPetPveStrategy(ai); }
            static Strategy* pet_demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockPetPvpStrategy(ai); }
            static Strategy* pet_demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockPetRaidStrategy(ai); }
            static Strategy* pet_affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockPetPveStrategy(ai); }
            static Strategy* pet_affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockPetPvpStrategy(ai); }
            static Strategy* pet_affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockPetRaidStrategy(ai); }
        };

        class CurseManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CurseManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["curse"] = &warlock::CurseManualStrategyFactoryInternal::curse;
                creators["curse agony"] = &warlock::CurseManualStrategyFactoryInternal::curse_agony;
                creators["curse doom"] = &warlock::CurseManualStrategyFactoryInternal::curse_doom;
                creators["curse elements"] = &warlock::CurseManualStrategyFactoryInternal::curse_elements;
                creators["curse recklessness"] = &warlock::CurseManualStrategyFactoryInternal::curse_recklessness;
                creators["curse weakness"] = &warlock::CurseManualStrategyFactoryInternal::curse_weakness;
                creators["curse tongues"] = &warlock::CurseManualStrategyFactoryInternal::curse_tongues;
                creators["curse shadow"] = &warlock::CurseManualStrategyFactoryInternal::curse_shadow;
            }

        private:
            static Strategy* curse(PlayerbotAI* ai) { return new WarlockCursePlaceholderStrategy(ai); }
            static Strategy* curse_agony(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse agony", "curse of agony", "curse of agony"); }
            static Strategy* curse_doom(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse doom", "curse of doom", "curse of doom"); }
            static Strategy* curse_elements(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse elements", "curse of the elements", "curse of the elements"); }
            static Strategy* curse_recklessness(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse recklessness", "curse of recklessness", "curse of recklessness"); }
            static Strategy* curse_weakness(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse weakness", "curse of weakness", "curse of weakness"); }
            static Strategy* curse_tongues(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse tongues", "curse of tongues", "curse of tongues"); }
            static Strategy* curse_shadow(PlayerbotAI* ai) { return new WarlockManualCurseStrategy(ai, "curse shadow", "curse of shadow", "curse of shadow"); }
        };

        class CurseSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CurseSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["curse destruction pve"] = &warlock::CurseSituationStrategyFactoryInternal::curse_destruction_pve;
                creators["curse destruction pvp"] = &warlock::CurseSituationStrategyFactoryInternal::curse_destruction_pvp;
                creators["curse destruction raid"] = &warlock::CurseSituationStrategyFactoryInternal::curse_destruction_raid;
                creators["curse demonology pve"] = &warlock::CurseSituationStrategyFactoryInternal::curse_demonology_pve;
                creators["curse demonology pvp"] = &warlock::CurseSituationStrategyFactoryInternal::curse_demonology_pvp;
                creators["curse demonology raid"] = &warlock::CurseSituationStrategyFactoryInternal::curse_demonology_raid;
                creators["curse affliction pve"] = &warlock::CurseSituationStrategyFactoryInternal::curse_affliction_pve;
                creators["curse affliction pvp"] = &warlock::CurseSituationStrategyFactoryInternal::curse_affliction_pvp;
                creators["curse affliction raid"] = &warlock::CurseSituationStrategyFactoryInternal::curse_affliction_raid;
            }

        private:
            static Strategy* curse_destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockCursesPveStrategy(ai); }
            static Strategy* curse_destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockCursesPvpStrategy(ai); }
            static Strategy* curse_destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockCursesRaidStrategy(ai); }
            static Strategy* curse_demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockCursesPveStrategy(ai); }
            static Strategy* curse_demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockCursesPvpStrategy(ai); }
            static Strategy* curse_demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockCursesRaidStrategy(ai); }
            static Strategy* curse_affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockCursesPveStrategy(ai); }
            static Strategy* curse_affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockCursesPvpStrategy(ai); }
            static Strategy* curse_affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockCursesRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc destruction pve"] = &warlock::CcSituationStrategyFactoryInternal::cc_destruction_pve;
                creators["cc destruction pvp"] = &warlock::CcSituationStrategyFactoryInternal::cc_destruction_pvp;
                creators["cc destruction raid"] = &warlock::CcSituationStrategyFactoryInternal::cc_destruction_raid;
                creators["cc demonology pve"] = &warlock::CcSituationStrategyFactoryInternal::cc_demonology_pve;
                creators["cc demonology pvp"] = &warlock::CcSituationStrategyFactoryInternal::cc_demonology_pvp;
                creators["cc demonology raid"] = &warlock::CcSituationStrategyFactoryInternal::cc_demonology_raid;
                creators["cc affliction pve"] = &warlock::CcSituationStrategyFactoryInternal::cc_affliction_pve;
                creators["cc affliction pvp"] = &warlock::CcSituationStrategyFactoryInternal::cc_affliction_pvp;
                creators["cc affliction raid"] = &warlock::CcSituationStrategyFactoryInternal::cc_affliction_raid;
            }

        private:
            static Strategy* cc_destruction_pve(PlayerbotAI* ai) { return new DestructionWarlockCcPveStrategy(ai); }
            static Strategy* cc_destruction_pvp(PlayerbotAI* ai) { return new DestructionWarlockCcPvpStrategy(ai); }
            static Strategy* cc_destruction_raid(PlayerbotAI* ai) { return new DestructionWarlockCcRaidStrategy(ai); }
            static Strategy* cc_demonology_pve(PlayerbotAI* ai) { return new DemonologyWarlockCcPveStrategy(ai); }
            static Strategy* cc_demonology_pvp(PlayerbotAI* ai) { return new DemonologyWarlockCcPvpStrategy(ai); }
            static Strategy* cc_demonology_raid(PlayerbotAI* ai) { return new DemonologyWarlockCcRaidStrategy(ai); }
            static Strategy* cc_affliction_pve(PlayerbotAI* ai) { return new AfflictionWarlockCcPveStrategy(ai); }
            static Strategy* cc_affliction_pvp(PlayerbotAI* ai) { return new AfflictionWarlockCcPvpStrategy(ai); }
            static Strategy* cc_affliction_raid(PlayerbotAI* ai) { return new AfflictionWarlockCcRaidStrategy(ai); }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["shadow trance"] = &TriggerFactoryInternal::shadow_trance;
                creators["demon armor"] = &TriggerFactoryInternal::demon_armor;
                creators["no healthstone"] = &TriggerFactoryInternal::has_healthstone;
                creators["no firestone"] = &TriggerFactoryInternal::has_firestone;
                creators["no spellstone"] = &TriggerFactoryInternal::has_spellstone;
                creators["corruption"] = &TriggerFactoryInternal::corruption;
                creators["corruption on attacker"] = &TriggerFactoryInternal::corruption_on_attacker;
                creators["curse of agony"] = &TriggerFactoryInternal::curse_of_agony;
                creators["curse of agony on attacker"] = &TriggerFactoryInternal::curse_of_agony_on_attacker;
                creators["curse of doom"] = &TriggerFactoryInternal::curse_of_doom;
                creators["curse of doom on attacker"] = &TriggerFactoryInternal::curse_of_doom_on_attacker;
                creators["curse of the elements"] = &TriggerFactoryInternal::curse_of_the_elements;
                creators["curse of the elements on attacker"] = &TriggerFactoryInternal::curse_of_the_elements_on_attacker;
                creators["curse of recklessness"] = &TriggerFactoryInternal::curse_of_recklessness;
                creators["curse of recklessness on attacker"] = &TriggerFactoryInternal::curse_of_recklessness_on_attacker;
                creators["curse of weakness"] = &TriggerFactoryInternal::curse_of_weakness;
                creators["curse of weakness on attacker"] = &TriggerFactoryInternal::curse_of_weakness_on_attacker;
                creators["curse of tongues"] = &TriggerFactoryInternal::curse_of_tongues;
                creators["curse of tongues on attacker"] = &TriggerFactoryInternal::curse_of_tongues_on_attacker;
                creators["curse of shadow"] = &TriggerFactoryInternal::curse_of_shadow;
                creators["curse of shadow on attacker"] = &TriggerFactoryInternal::curse_of_shadow_on_attacker;
                creators["banish"] = &TriggerFactoryInternal::banish;
                creators["spellstone"] = &TriggerFactoryInternal::spellstone;
                creators["backlash"] = &TriggerFactoryInternal::backlash;
                creators["fear"] = &TriggerFactoryInternal::fear;
                creators["fear pvp"] = &TriggerFactoryInternal::fear_pvp;
                creators["immolate"] = &TriggerFactoryInternal::immolate;
                creators["amplify curse"] = &TriggerFactoryInternal::amplify_curse;
                creators["siphon life"] = &TriggerFactoryInternal::siphon_life;
                creators["siphon life on attacker"] = &TriggerFactoryInternal::siphon_life_on_attacker;
                creators["death coil interrupt"] = &TriggerFactoryInternal::death_coil_interrupt;
                creators["death coil on enemy healer"] = &TriggerFactoryInternal::death_coil_enemy_healer;
                creators["death coil on snare target"] = &TriggerFactoryInternal::death_coil_snare_target;
                creators["inferno"] = &TriggerFactoryInternal::inferno;
                creators["shadowfury interrupt"] = &TriggerFactoryInternal::shadowfury_interrupt;
                creators["shadowfury on snare target"] = &TriggerFactoryInternal::shadowfury_snare;
                creators["unstable affliction"] = &TriggerFactoryInternal::unstable_affliction;
                creators["unstable affliction on attacker"] = &TriggerFactoryInternal::unstable_affliction_on_attacker;
                creators["life tap"] = &TriggerFactoryInternal::life_tap;
                creators["drain soul"] = &TriggerFactoryInternal::drain_soul;
                creators["no curse"] = &TriggerFactoryInternal::no_curse;
                creators["no curse on attacker"] = &TriggerFactoryInternal::no_curse_on_attacker;
                creators["conflagrate"] = &TriggerFactoryInternal::conflagrate;
                creators["demonic sacrifice"] = &TriggerFactoryInternal::demonic_sacrifice;
                creators["no imp"] = &TriggerFactoryInternal::no_imp;
                creators["no voidwalker"] = &TriggerFactoryInternal::no_voidwalker;
                creators["no succubus"] = &TriggerFactoryInternal::no_succubus;
                creators["no felhunter"] = &TriggerFactoryInternal::no_felhunter;
                creators["no felguard"] = &TriggerFactoryInternal::no_felguard;
                creators["spell lock"] = &TriggerFactoryInternal::spell_lock;
                creators["spell lock enemy healer"] = &TriggerFactoryInternal::spell_lock_enemy_healer;
                creators["seed of corruption on attacker"] = &TriggerFactoryInternal::seed_of_corruption_on_attacker;
                creators["soulstone"] = &TriggerFactoryInternal::soulstone;
                creators["shadow ward"] = &TriggerFactoryInternal::shadow_ward;
            }

        private:
            static Trigger* unstable_affliction(PlayerbotAI* ai) { return new UnstableAfflictionTrigger(ai); }
            static Trigger* unstable_affliction_on_attacker(PlayerbotAI* ai) { return new UnstableAfflictionOnAttackerTrigger(ai); }
            static Trigger* shadowfury_snare(PlayerbotAI* ai) { return new ShadowfurySnareTrigger(ai); }
            static Trigger* shadowfury_interrupt(PlayerbotAI* ai) { return new ShadowfuryInterruptTrigger(ai); }
            static Trigger* inferno(PlayerbotAI* ai) { return new InfernoTrigger(ai); }
            static Trigger* death_coil_snare_target(PlayerbotAI* ai) { return new DeathCoilSnareTrigger(ai); }
            static Trigger* death_coil_enemy_healer(PlayerbotAI* ai) { return new DeathCoilInterruptTHealerTrigger(ai); }
            static Trigger* death_coil_interrupt(PlayerbotAI* ai) { return new DeathCoilInterruptTrigger(ai); }
            static Trigger* amplify_curse(PlayerbotAI* ai) { return new AmplifyCurseTrigger(ai); }
            static Trigger* shadow_trance(PlayerbotAI* ai) { return new ShadowTranceTrigger(ai); }
            static Trigger* demon_armor(PlayerbotAI* ai) { return new DemonArmorTrigger(ai); }
            static Trigger* has_healthstone(PlayerbotAI* ai) { return new HasHealthstoneTrigger(ai); }
            static Trigger* has_firestone(PlayerbotAI* ai) { return new HasFirestoneTrigger(ai); }
            static Trigger* has_spellstone(PlayerbotAI* ai) { return new HasSpellstoneTrigger(ai); }
            static Trigger* corruption(PlayerbotAI* ai) { return new CorruptionTrigger(ai); }
            static Trigger* corruption_on_attacker(PlayerbotAI* ai) { return new CorruptionOnAttackerTrigger(ai); }
            static Trigger* siphon_life(PlayerbotAI* ai) { return new SiphonLifeTrigger(ai); }
            static Trigger* siphon_life_on_attacker(PlayerbotAI* ai) { return new SiphonLifeOnAttackerTrigger(ai); }
            static Trigger* curse_of_agony(PlayerbotAI* ai) { return new CurseOfAgonyTrigger(ai); }
            static Trigger* curse_of_agony_on_attacker(PlayerbotAI* ai) { return new CurseOfAgonyOnAttackerTrigger(ai); }
            static Trigger* curse_of_doom(PlayerbotAI* ai) { return new CurseOfDoomTrigger(ai); }
            static Trigger* curse_of_doom_on_attacker(PlayerbotAI* ai) { return new CurseOfDoomOnAttackerTrigger(ai); }
            static Trigger* curse_of_the_elements(PlayerbotAI* ai) { return new CurseOfTheElementsTrigger(ai); }
            static Trigger* curse_of_the_elements_on_attacker(PlayerbotAI* ai) { return new CurseOfTheElementsOnAttackerTrigger(ai); }
            static Trigger* curse_of_recklessness(PlayerbotAI* ai) { return new CurseOfRecklessnessTrigger(ai); }
            static Trigger* curse_of_recklessness_on_attacker(PlayerbotAI* ai) { return new CurseOfRecklessnessOnAttackerTrigger(ai); }
            static Trigger* curse_of_weakness(PlayerbotAI* ai) { return new CurseOfWeaknessTrigger(ai); }
            static Trigger* curse_of_weakness_on_attacker(PlayerbotAI* ai) { return new CurseOfWeaknessOnAttackerTrigger(ai); }
            static Trigger* curse_of_tongues(PlayerbotAI* ai) { return new CurseOfTonguesTrigger(ai); }
            static Trigger* curse_of_tongues_on_attacker(PlayerbotAI* ai) { return new CurseOfTonguesOnAttackerTrigger(ai); }
            static Trigger* curse_of_shadow(PlayerbotAI* ai) { return new CurseOfShadowTrigger(ai); }
            static Trigger* curse_of_shadow_on_attacker(PlayerbotAI* ai) { return new CurseOfShadowOnAttackerTrigger(ai); }
            static Trigger* banish(PlayerbotAI* ai) { return new BanishTrigger(ai); }
            static Trigger* spellstone(PlayerbotAI* ai) { return new SpellstoneTrigger(ai); }
            static Trigger* backlash(PlayerbotAI* ai) { return new BacklashTrigger(ai); }
            static Trigger* fear(PlayerbotAI* ai) { return new FearTrigger(ai); }
            static Trigger* fear_pvp(PlayerbotAI* ai) { return new FearPvpTrigger(ai); }
            static Trigger* immolate(PlayerbotAI* ai) { return new ImmolateTrigger(ai); }
            static Trigger* life_tap(PlayerbotAI* ai) { return new LifeTapTrigger(ai); }
            static Trigger* drain_soul(PlayerbotAI* ai) { return new DrainSoulTrigger(ai); }
            static Trigger* no_curse(PlayerbotAI* ai) { return new NoCurseTrigger(ai); }
            static Trigger* no_curse_on_attacker(PlayerbotAI* ai) { return new NoCurseOnAttackerTrigger(ai); }
            static Trigger* conflagrate(PlayerbotAI* ai) { return new ConflagrateTrigger(ai); }
            static Trigger* demonic_sacrifice(PlayerbotAI* ai) { return new DemonicSacrificeTrigger(ai); }
            static Trigger* soul_link(PlayerbotAI* ai) { return new SoulLinkTrigger(ai); }
            static Trigger* no_imp(PlayerbotAI* ai) { return new NoImpTrigger(ai); }
            static Trigger* no_voidwalker(PlayerbotAI* ai) { return new NoVoidwalkerTrigger(ai); }
            static Trigger* no_succubus(PlayerbotAI* ai) { return new NoSuccubusTrigger(ai); }
            static Trigger* no_felhunter(PlayerbotAI* ai) { return new NoFelhunterTrigger(ai); }
            static Trigger* no_felguard(PlayerbotAI* ai) { return new NoFelguardTrigger(ai); }
            static Trigger* spell_lock(PlayerbotAI* ai) { return new SpellLockTrigger(ai); }
            static Trigger* spell_lock_enemy_healer(PlayerbotAI* ai) { return new SpellLockEnemyHealerTrigger(ai); }
            static Trigger* seed_of_corruption_on_attacker(PlayerbotAI* ai) { return new SeedOfCorruptionOnAttackerTrigger(ai); }
            static Trigger* soulstone(PlayerbotAI* ai) { return new SoulstoneTrigger(ai); }
            static Trigger* shadow_ward(PlayerbotAI* ai) { return new ShadowWardTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["fel armor"] = &AiObjectContextInternal::fel_armor;
                creators["demon armor"] = &AiObjectContextInternal::demon_armor;
                creators["demon skin"] = &AiObjectContextInternal::demon_skin;
                creators["create healthstone"] = &AiObjectContextInternal::create_healthstone;
                creators["create firestone"] = &AiObjectContextInternal::create_firestone;
                creators["create spellstone"] = &AiObjectContextInternal::create_spellstone;
                creators["spellstone"] = &AiObjectContextInternal::spellstone;
                creators["summon voidwalker"] = &AiObjectContextInternal::summon_voidwalker;
                creators["summon succubus"] = &AiObjectContextInternal::summon_succubus;
                creators["summon felhunter"] = &AiObjectContextInternal::summon_felhunter;
                creators["summon imp"] = &AiObjectContextInternal::summon_imp;
                creators["summon felguard"] = &AiObjectContextInternal::summon_felguard;
                creators["summon inferno"] = &AiObjectContextInternal::inferno;
                creators["immolate"] = &AiObjectContextInternal::immolate;
                creators["corruption"] = &AiObjectContextInternal::corruption;
                creators["corruption on attacker"] = &AiObjectContextInternal::corruption_on_attacker;
                creators["seed of corruption on attacker"] = &AiObjectContextInternal::seed_of_corruption_on_attacker;
                creators["siphon life"] = &AiObjectContextInternal::siphon_life;
                creators["siphon life on attacker"] = &AiObjectContextInternal::siphon_life_on_attacker;
                creators["curse of agony"] = &AiObjectContextInternal::curse_of_agony;
                creators["curse of agony on attacker"] = &AiObjectContextInternal::curse_of_agony_on_attacker;
                creators["curse of doom"] = &AiObjectContextInternal::curse_of_doom;
                creators["curse of doom on attacker"] = &AiObjectContextInternal::curse_of_doom_on_attacker;
                creators["curse of the elements"] = &AiObjectContextInternal::curse_of_the_elements;
                creators["curse of the elements on attacker"] = &AiObjectContextInternal::curse_of_the_elements_on_attacker;
                creators["curse of recklessness"] = &AiObjectContextInternal::curse_of_recklessness;
                creators["curse of recklessness on attacker"] = &AiObjectContextInternal::curse_of_recklessness_on_attacker;
                creators["curse of weakness"] = &AiObjectContextInternal::curse_of_weakness;
                creators["curse of weakness on attacker"] = &AiObjectContextInternal::curse_of_weakness_on_attacker;
                creators["curse of tongues"] = &AiObjectContextInternal::curse_of_tongues;
                creators["curse of tongues on attacker"] = &AiObjectContextInternal::curse_of_tongues_on_attacker;
                creators["curse of shadow"] = &AiObjectContextInternal::curse_of_shadow;
                creators["curse of shadow on attacker"] = &AiObjectContextInternal::curse_of_shadow_on_attacker;
                creators["shadow bolt"] = &AiObjectContextInternal::shadow_bolt;
                creators["drain soul"] = &AiObjectContextInternal::drain_soul;
                creators["drain mana"] = &AiObjectContextInternal::drain_mana;
                creators["drain life"] = &AiObjectContextInternal::drain_life;
                creators["banish"] = &AiObjectContextInternal::banish;
                creators["banish on cc"] = &AiObjectContextInternal::banish_on_cc;
                creators["seed of corruption"] = &AiObjectContextInternal::seed_of_corruption;
                creators["rain of fire"] = &AiObjectContextInternal::rain_of_fire;
                creators["shadowfury"] = &AiObjectContextInternal::shadowfury;
                creators["shadowfury on snare target"] = &AiObjectContextInternal::shadowfury_snare;
                creators["life tap"] = &AiObjectContextInternal::life_tap;
                creators["fear"] = &AiObjectContextInternal::fear;
                creators["fear on cc"] = &AiObjectContextInternal::fear_on_cc;
                creators["incinerate"] = &AiObjectContextInternal::incinerate;
                creators["conflagrate"] = &AiObjectContextInternal::conflagrate;
                creators["amplify curse"] = &AiObjectContextInternal::amplify_curse;
                creators["shadowburn"] = &AiObjectContextInternal::shadowburn;
                creators["death coil"] = &AiObjectContextInternal::death_coil;
                creators["death coil on enemy healer"] = &AiObjectContextInternal::death_coil_healer;
                creators["death coil on snare target"] = &AiObjectContextInternal::death_coil_snare;
                creators["dark pact"] = &AiObjectContextInternal::dark_pact;
                creators["howl of terror"] = &AiObjectContextInternal::howl_of_terror;
                creators["unstable affliction"] = &AiObjectContextInternal::unstable_affliction;
                creators["unstable affliction on attacker"] = &AiObjectContextInternal::unstable_affliction_on_attacker;
                creators["soulshatter"] = &AiObjectContextInternal::soulshatter;
                creators["searing pain"] = &AiObjectContextInternal::searing_pain;
                creators["soul fire"] = &AiObjectContextInternal::soul_fire;
                creators["curse of exhaustion"] = &AiObjectContextInternal::curse_of_exhaustion;
                creators["demonic sacrifice"] = &AiObjectContextInternal::demonic_sacrifice;
                creators["soul link"] = &AiObjectContextInternal::soul_link;
                creators["sacrifice"] = &AiObjectContextInternal::sacrifice;
                creators["spell lock"] = &AiObjectContextInternal::spell_lock;
                creators["spell lock on enemy healer"] = &AiObjectContextInternal::spell_lock_on_enemy_healer;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
                creators["soulstone"] = &AiObjectContextInternal::soulstone;
                creators["shadow ward"] = &AiObjectContextInternal::shadow_ward;
            }

        private:
            static Action* searing_pain(PlayerbotAI* ai) { return new CastSearingPainAction(ai); }
            static Action* soulshatter(PlayerbotAI* ai) { return new CastSoulShatterAction(ai); }
            static Action* unstable_affliction(PlayerbotAI* ai) { return new CastUnstableAfflictionAction(ai); }
            static Action* unstable_affliction_on_attacker(PlayerbotAI* ai) { return new CastUnstableAfflictionOnAttackerAction(ai); }
            static Action* howl_of_terror(PlayerbotAI* ai) { return new CastHowlOfTerrorAction(ai); }
            static Action* dark_pact(PlayerbotAI* ai) { return new CastDarkPactAction(ai); }
            static Action* death_coil_snare(PlayerbotAI* ai) { return new CastDeathCoilSnareAction(ai); }
            static Action* death_coil_healer(PlayerbotAI* ai) { return new CastDeathCoilOnHealerAction(ai); }
            static Action* death_coil(PlayerbotAI* ai) { return new CastDeathCoilAction(ai); }
            static Action* amplify_curse(PlayerbotAI* ai) { return new CastAmplifyCurseAction(ai); }
            static Action* shadowburn(PlayerbotAI* ai) { return new CastShadowburnAction(ai); }
            static Action* conflagrate(PlayerbotAI* ai) { return new CastConflagrateAction(ai); }
            static Action* incinerate(PlayerbotAI* ai) { return new CastIncinerateAction(ai); }
            static Action* fear_on_cc(PlayerbotAI* ai) { return new CastFearOnCcAction(ai); }
            static Action* fear(PlayerbotAI* ai) { return new CastFearAction(ai); }
            static Action* immolate(PlayerbotAI* ai) { return new CastImmolateAction(ai); }
            static Action* summon_imp(PlayerbotAI* ai) { return new CastSummonImpAction(ai); }
            static Action* summon_succubus(PlayerbotAI* ai) { return new CastSummonSuccubusAction(ai); }
            static Action* summon_felhunter(PlayerbotAI * ai) { return new CastSummonFelhunterAction(ai); }
            static Action* fel_armor(PlayerbotAI* ai) { return new CastFelArmorAction(ai); }
            static Action* demon_armor(PlayerbotAI* ai) { return new CastDemonArmorAction(ai); }
            static Action* demon_skin(PlayerbotAI* ai) { return new CastDemonSkinAction(ai); }
            static Action* create_healthstone(PlayerbotAI* ai) { return new CastCreateHealthstoneAction(ai); }
            static Action* create_firestone(PlayerbotAI* ai) { return new CastCreateFirestoneAction(ai); }
            static Action* create_spellstone(PlayerbotAI* ai) { return new CastCreateSpellstoneAction(ai); }
            static Action* spellstone(PlayerbotAI* ai) { return new UseSpellItemAction(ai, "spellstone"); }
            static Action* summon_voidwalker(PlayerbotAI* ai) { return new CastSummonVoidwalkerAction(ai); }
            static Action* summon_felguard(PlayerbotAI* ai) { return new CastSummonFelguardAction(ai); }
            static Action* inferno(PlayerbotAI* ai) { return new CastSummonInfernoAction(ai); }
            static Action* corruption(PlayerbotAI* ai) { return new CastCorruptionAction(ai); }
            static Action* corruption_on_attacker(PlayerbotAI* ai) { return new CastCorruptionOnAttackerAction(ai); }
            static Action* seed_of_corruption_on_attacker(PlayerbotAI* ai) { return new CastSeedOfCorruptionOnAttackerAction(ai); }
            static Action* siphon_life(PlayerbotAI* ai) { return new CastSiphonLifeAction(ai); }
            static Action* siphon_life_on_attacker(PlayerbotAI* ai) { return new CastSiphonLifeOnAttackerAction(ai); }
            static Action* curse_of_agony(PlayerbotAI* ai) { return new CastCurseOfAgonyAction(ai); }
            static Action* curse_of_agony_on_attacker(PlayerbotAI* ai) { return new CastCurseOfAgonyOnAttackerAction(ai); }
            static Action* curse_of_doom(PlayerbotAI* ai) { return new CastCurseOfDoomAction(ai); }
            static Action* curse_of_doom_on_attacker(PlayerbotAI* ai) { return new CastCurseOfDoomOnAttackerAction(ai); }
            static Action* curse_of_the_elements(PlayerbotAI* ai) { return new CastCurseOfTheElementsAction(ai); }
            static Action* curse_of_the_elements_on_attacker(PlayerbotAI* ai) { return new CastCurseOfTheElementsOnAttackerAction(ai); }
            static Action* curse_of_recklessness(PlayerbotAI* ai) { return new CastCurseOfRecklessnessAction(ai); }
            static Action* curse_of_recklessness_on_attacker(PlayerbotAI* ai) { return new CastCurseOfRecklessnessOnAttackerAction(ai); }
            static Action* curse_of_weakness(PlayerbotAI* ai) { return new CastCurseOfWeaknessAction(ai); }
            static Action* curse_of_weakness_on_attacker(PlayerbotAI* ai) { return new CastCurseOfWeaknessOnAttackerAction(ai); }
            static Action* curse_of_tongues(PlayerbotAI* ai) { return new CastCurseOfTonguesAction(ai); }
            static Action* curse_of_tongues_on_attacker(PlayerbotAI* ai) { return new CastCurseOfTonguesOnAttackerAction(ai); }
            static Action* curse_of_shadow(PlayerbotAI* ai) { return new CastCurseOfShadowAction(ai); }
            static Action* curse_of_shadow_on_attacker(PlayerbotAI* ai) { return new CastCurseOfShadowOnAttackerAction(ai); }
            static Action* curse_of_exhaustion(PlayerbotAI* ai) { return new CastCurseOfExhaustionAction(ai); }
            static Action* shadow_bolt(PlayerbotAI* ai) { return new CastShadowBoltAction(ai); }
            static Action* drain_soul(PlayerbotAI* ai) { return new CastDrainSoulAction(ai); }
            static Action* drain_mana(PlayerbotAI* ai) { return new CastDrainManaAction(ai); }
            static Action* drain_life(PlayerbotAI* ai) { return new CastDrainLifeAction(ai); }
            static Action* banish(PlayerbotAI* ai) { return new CastBanishAction(ai); }
            static Action* banish_on_cc(PlayerbotAI* ai) { return new CastBanishOnCcAction(ai); }
            static Action* seed_of_corruption(PlayerbotAI* ai) { return new CastSeedOfCorruptionAction(ai); }
            static Action* rain_of_fire(PlayerbotAI* ai) { return new CastRainOfFireAction(ai); }
            static Action* shadowfury(PlayerbotAI* ai) { return new CastShadowfuryAction(ai); }
            static Action* shadowfury_snare(PlayerbotAI* ai) { return new CastShadowfurySnareAction(ai); }
            static Action* life_tap(PlayerbotAI* ai) { return new CastLifeTapAction(ai); }
            static Action* soul_fire(PlayerbotAI* ai) { return new CastSoulFireAction(ai); }
            static Action* demonic_sacrifice(PlayerbotAI* ai) { return new CastDemonicSacrificeAction(ai); }
            static Action* soul_link(PlayerbotAI* ai) { return new CastSoulLinkAction(ai); }
            static Action* sacrifice(PlayerbotAI* ai) { return new CastSacrificeAction(ai); }
            static Action* spell_lock(PlayerbotAI* ai) { return new CastSpellLockAction(ai); }
            static Action* spell_lock_on_enemy_healer(PlayerbotAI* ai) { return new CastSpellLockOnEnemyHealerAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdateWarlockPveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdateWarlockPvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdateWarlockRaidStrategiesAction(ai); }
            static Action* soulstone(PlayerbotAI* ai) { return new CastSoulstoneAction(ai); }
            static Action* shadow_ward(PlayerbotAI* ai) { return new CastShadowWardAction(ai); }
        };
    };
};

WarlockAiObjectContext::WarlockAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warlock::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::PetManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::PetSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CurseSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CurseManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::warlock::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::warlock::AiObjectContextInternal());
    triggerContexts.Add(new ai::warlock::TriggerFactoryInternal());
}
