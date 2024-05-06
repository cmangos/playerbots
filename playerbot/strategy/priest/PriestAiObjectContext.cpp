#include "playerbot/playerbot.h"
#include "PriestActions.h"
#include "PriestTriggers.h"
#include "PriestAiObjectContext.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "DisciplinePriestStrategy.h"
#include "ShadowPriestStrategy.h"
#include "HolyPriestStrategy.h"

namespace ai
{
    namespace priest
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = &priest::StrategyFactoryInternal::aoe;
                creators["cure"] = &priest::StrategyFactoryInternal::cure;
                creators["buff"] = &priest::StrategyFactoryInternal::buff;
                creators["pull"] = &priest::StrategyFactoryInternal::pull;
                creators["cc"] = &priest::StrategyFactoryInternal::cc;
                creators["offheal"] = &priest::StrategyFactoryInternal::offheal;
                creators["boost"] = &priest::StrategyFactoryInternal::boost;
                creators["offdps"] = &priest::StrategyFactoryInternal::offdps;
            }

        private:
            static Strategy* aoe(PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); }
            static Strategy* cure(PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); }
            static Strategy* buff(PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); }
            static Strategy* cc(PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); }
            static Strategy* offheal(PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); }
            static Strategy* offdps(PlayerbotAI* ai) { return new OffdpsPlaceholderStrategy(ai); }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe discipline pve"] = &priest::AoeSituationStrategyFactoryInternal::aoe_discipline_pve;
                creators["aoe discipline pvp"] = &priest::AoeSituationStrategyFactoryInternal::aoe_discipline_pvp;
                creators["aoe discipline raid"] = &priest::AoeSituationStrategyFactoryInternal::aoe_discipline_raid;
                creators["aoe holy pve"] = &priest::AoeSituationStrategyFactoryInternal::aoe_holy_pve;
                creators["aoe holy pvp"] = &priest::AoeSituationStrategyFactoryInternal::aoe_holy_pvp;
                creators["aoe holy raid"] = &priest::AoeSituationStrategyFactoryInternal::aoe_holy_raid;
                creators["aoe shadow pve"] = &priest::AoeSituationStrategyFactoryInternal::aoe_shadow_pve;
                creators["aoe shadow pvp"] = &priest::AoeSituationStrategyFactoryInternal::aoe_shadow_pvp;
                creators["aoe shadow raid"] = &priest::AoeSituationStrategyFactoryInternal::aoe_shadow_raid;
            }

        private:
            static Strategy* aoe_discipline_pve(PlayerbotAI* ai) { return new DisciplinePriestAoePveStrategy(ai); }
            static Strategy* aoe_discipline_pvp(PlayerbotAI* ai) { return new DisciplinePriestAoePvpStrategy(ai); }
            static Strategy* aoe_discipline_raid(PlayerbotAI* ai) { return new DisciplinePriestAoeRaidStrategy(ai); }
            static Strategy* aoe_holy_pve(PlayerbotAI* ai) { return new HolyPriestAoePveStrategy(ai); }
            static Strategy* aoe_holy_pvp(PlayerbotAI* ai) { return new HolyPriestAoePvpStrategy(ai); }
            static Strategy* aoe_holy_raid(PlayerbotAI* ai) { return new HolyPriestAoeRaidStrategy(ai); }
            static Strategy* aoe_shadow_pve(PlayerbotAI* ai) { return new ShadowPriestAoePveStrategy(ai); }
            static Strategy* aoe_shadow_pvp(PlayerbotAI* ai) { return new ShadowPriestAoePvpStrategy(ai); }
            static Strategy* aoe_shadow_raid(PlayerbotAI* ai) { return new ShadowPriestAoeRaidStrategy(ai); }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure discipline pve"] = &priest::CureSituationStrategyFactoryInternal::cure_discipline_pve;
                creators["cure discipline pvp"] = &priest::CureSituationStrategyFactoryInternal::cure_discipline_pvp;
                creators["cure discipline raid"] = &priest::CureSituationStrategyFactoryInternal::cure_discipline_raid;
                creators["cure holy pve"] = &priest::CureSituationStrategyFactoryInternal::cure_holy_pve;
                creators["cure holy pvp"] = &priest::CureSituationStrategyFactoryInternal::cure_holy_pvp;
                creators["cure holy raid"] = &priest::CureSituationStrategyFactoryInternal::cure_holy_raid;
                creators["cure shadow pve"] = &priest::CureSituationStrategyFactoryInternal::cure_shadow_pve;
                creators["cure shadow pvp"] = &priest::CureSituationStrategyFactoryInternal::cure_shadow_pvp;
                creators["cure shadow raid"] = &priest::CureSituationStrategyFactoryInternal::cure_shadow_raid;
            }

        private:
            static Strategy* cure_discipline_pve(PlayerbotAI* ai) { return new DisciplinePriestCurePveStrategy(ai); }
            static Strategy* cure_discipline_pvp(PlayerbotAI* ai) { return new DisciplinePriestCurePvpStrategy(ai); }
            static Strategy* cure_discipline_raid(PlayerbotAI* ai) { return new DisciplinePriestCureRaidStrategy(ai); }
            static Strategy* cure_holy_pve(PlayerbotAI* ai) { return new HolyPriestCurePveStrategy(ai); }
            static Strategy* cure_holy_pvp(PlayerbotAI* ai) { return new HolyPriestCurePvpStrategy(ai); }
            static Strategy* cure_holy_raid(PlayerbotAI* ai) { return new HolyPriestCureRaidStrategy(ai); }
            static Strategy* cure_shadow_pve(PlayerbotAI* ai) { return new ShadowPriestCurePveStrategy(ai); }
            static Strategy* cure_shadow_pvp(PlayerbotAI* ai) { return new ShadowPriestCurePvpStrategy(ai); }
            static Strategy* cure_shadow_raid(PlayerbotAI* ai) { return new ShadowPriestCureRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff discipline pve"] = &priest::BuffSituationStrategyFactoryInternal::buff_discipline_pve;
                creators["buff discipline pvp"] = &priest::BuffSituationStrategyFactoryInternal::buff_discipline_pvp;
                creators["buff discipline raid"] = &priest::BuffSituationStrategyFactoryInternal::buff_discipline_raid;
                creators["buff holy pve"] = &priest::BuffSituationStrategyFactoryInternal::buff_holy_pve;
                creators["buff holy pvp"] = &priest::BuffSituationStrategyFactoryInternal::buff_holy_pvp;
                creators["buff holy raid"] = &priest::BuffSituationStrategyFactoryInternal::buff_holy_raid;
                creators["buff shadow pve"] = &priest::BuffSituationStrategyFactoryInternal::buff_shadow_pve;
                creators["buff shadow pvp"] = &priest::BuffSituationStrategyFactoryInternal::buff_shadow_pvp;
                creators["buff shadow raid"] = &priest::BuffSituationStrategyFactoryInternal::buff_shadow_raid;
            }

        private:
            static Strategy* buff_discipline_pve(PlayerbotAI* ai) { return new DisciplinePriestBuffPveStrategy(ai); }
            static Strategy* buff_discipline_pvp(PlayerbotAI* ai) { return new DisciplinePriestBuffPvpStrategy(ai); }
            static Strategy* buff_discipline_raid(PlayerbotAI* ai) { return new DisciplinePriestBuffRaidStrategy(ai); }
            static Strategy* buff_holy_pve(PlayerbotAI* ai) { return new HolyPriestBuffPveStrategy(ai); }
            static Strategy* buff_holy_pvp(PlayerbotAI* ai) { return new HolyPriestBuffPvpStrategy(ai); }
            static Strategy* buff_holy_raid(PlayerbotAI* ai) { return new HolyPriestBuffRaidStrategy(ai); }
            static Strategy* buff_shadow_pve(PlayerbotAI* ai) { return new ShadowPriestBuffPveStrategy(ai); }
            static Strategy* buff_shadow_pvp(PlayerbotAI* ai) { return new ShadowPriestBuffPvpStrategy(ai); }
            static Strategy* buff_shadow_raid(PlayerbotAI* ai) { return new ShadowPriestBuffRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost discipline pve"] = &priest::BoostSituationStrategyFactoryInternal::boost_discipline_pve;
                creators["boost discipline pvp"] = &priest::BoostSituationStrategyFactoryInternal::boost_discipline_pvp;
                creators["boost discipline raid"] = &priest::BoostSituationStrategyFactoryInternal::boost_discipline_raid;
                creators["boost holy pve"] = &priest::BoostSituationStrategyFactoryInternal::boost_holy_pve;
                creators["boost holy pvp"] = &priest::BoostSituationStrategyFactoryInternal::boost_holy_pvp;
                creators["boost holy raid"] = &priest::BoostSituationStrategyFactoryInternal::boost_holy_raid;
                creators["boost shadow pve"] = &priest::BoostSituationStrategyFactoryInternal::boost_shadow_pve;
                creators["boost shadow pvp"] = &priest::BoostSituationStrategyFactoryInternal::boost_shadow_pvp;
                creators["boost shadow raid"] = &priest::BoostSituationStrategyFactoryInternal::boost_shadow_raid;
            }

        private:
            static Strategy* boost_discipline_pve(PlayerbotAI* ai) { return new DisciplinePriestBoostPveStrategy(ai); }
            static Strategy* boost_discipline_pvp(PlayerbotAI* ai) { return new DisciplinePriestBoostPvpStrategy(ai); }
            static Strategy* boost_discipline_raid(PlayerbotAI* ai) { return new DisciplinePriestBoostRaidStrategy(ai); }
            static Strategy* boost_holy_pve(PlayerbotAI* ai) { return new HolyPriestBoostPveStrategy(ai); }
            static Strategy* boost_holy_pvp(PlayerbotAI* ai) { return new HolyPriestBoostPvpStrategy(ai); }
            static Strategy* boost_holy_raid(PlayerbotAI* ai) { return new HolyPriestBoostRaidStrategy(ai); }
            static Strategy* boost_shadow_pve(PlayerbotAI* ai) { return new ShadowPriestBoostPveStrategy(ai); }
            static Strategy* boost_shadow_pvp(PlayerbotAI* ai) { return new ShadowPriestBoostPvpStrategy(ai); }
            static Strategy* boost_shadow_raid(PlayerbotAI* ai) { return new ShadowPriestBoostRaidStrategy(ai); }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = &priest::OffhealSituationStrategyFactoryInternal::offheal_pve;
                creators["offheal pvp"] = &priest::OffhealSituationStrategyFactoryInternal::offheal_pvp;
                creators["offheal raid"] = &priest::OffhealSituationStrategyFactoryInternal::offheal_raid;
            }

        private:
            static Strategy* offheal_pve(PlayerbotAI* ai) { return new PriestOffhealPveStrategy(ai); }
            static Strategy* offheal_pvp(PlayerbotAI* ai) { return new PriestOffhealPvpStrategy(ai); }
            static Strategy* offheal_raid(PlayerbotAI* ai) { return new PriestOffhealRaidStrategy(ai); }
        };

        class OffdpsSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffdpsSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offdps pve"] = &priest::OffdpsSituationStrategyFactoryInternal::offdps_pve;
                creators["offdps pvp"] = &priest::OffdpsSituationStrategyFactoryInternal::offdps_pvp;
                creators["offdps raid"] = &priest::OffdpsSituationStrategyFactoryInternal::offdps_raid;
            }

        private:
            static Strategy* offdps_pve(PlayerbotAI* ai) { return new PriestOffdpsPveStrategy(ai); }
            static Strategy* offdps_pvp(PlayerbotAI* ai) { return new PriestOffdpsPvpStrategy(ai); }
            static Strategy* offdps_raid(PlayerbotAI* ai) { return new PriestOffdpsRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc holy pve"] = &priest::CcSituationStrategyFactoryInternal::cc_holy_pve;
                creators["cc holy pvp"] = &priest::CcSituationStrategyFactoryInternal::cc_holy_pvp;
                creators["cc holy raid"] = &priest::CcSituationStrategyFactoryInternal::cc_holy_raid;
                creators["cc shadow pve"] = &priest::CcSituationStrategyFactoryInternal::cc_shadow_pve;
                creators["cc shadow pvp"] = &priest::CcSituationStrategyFactoryInternal::cc_shadow_pvp;
                creators["cc shadow raid"] = &priest::CcSituationStrategyFactoryInternal::cc_shadow_raid;
                creators["cc discipline pve"] = &priest::CcSituationStrategyFactoryInternal::cc_discipline_pve;
                creators["cc discipline pvp"] = &priest::CcSituationStrategyFactoryInternal::cc_discipline_pvp;
                creators["cc discipline raid"] = &priest::CcSituationStrategyFactoryInternal::cc_discipline_raid;
            }

        private:
            static Strategy* cc_holy_pve(PlayerbotAI* ai) { return new HolyPriestCcPveStrategy(ai); }
            static Strategy* cc_holy_pvp(PlayerbotAI* ai) { return new HolyPriestCcPvpStrategy(ai); }
            static Strategy* cc_holy_raid(PlayerbotAI* ai) { return new HolyPriestCcRaidStrategy(ai); }
            static Strategy* cc_shadow_pve(PlayerbotAI* ai) { return new ShadowPriestCcPveStrategy(ai); }
            static Strategy* cc_shadow_pvp(PlayerbotAI* ai) { return new ShadowPriestCcPvpStrategy(ai); }
            static Strategy* cc_shadow_raid(PlayerbotAI* ai) { return new ShadowPriestCcRaidStrategy(ai); }
            static Strategy* cc_discipline_pve(PlayerbotAI* ai) { return new DisciplinePriestCcPveStrategy(ai); }
            static Strategy* cc_discipline_pvp(PlayerbotAI* ai) { return new DisciplinePriestCcPvpStrategy(ai); }
            static Strategy* cc_discipline_raid(PlayerbotAI* ai) { return new DisciplinePriestCcRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["holy"] = &priest::ClassStrategyFactoryInternal::holy;
                creators["shadow"] = &priest::ClassStrategyFactoryInternal::shadow;
                creators["discipline"] = &priest::ClassStrategyFactoryInternal::discipline;
                creators["heal"] = &priest::ClassStrategyFactoryInternal::holy;
                creators["dps"] = &priest::ClassStrategyFactoryInternal::shadow;
            }

        private:
            static Strategy* holy(PlayerbotAI* ai) { return new HolyPriestPlaceholderStrategy(ai); }
            static Strategy* shadow(PlayerbotAI* ai) { return new ShadowPriestPlaceholderStrategy(ai); }
            static Strategy* discipline(PlayerbotAI* ai) { return new DisciplinePriestPlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["discipline pvp"] = &priest::ClassSituationStrategyFactoryInternal::discipline_pvp;
                creators["discipline pve"] = &priest::ClassSituationStrategyFactoryInternal::discipline_pve;
                creators["discipline raid"] = &priest::ClassSituationStrategyFactoryInternal::discipline_raid;
                creators["holy pvp"] = &priest::ClassSituationStrategyFactoryInternal::holy_pvp;
                creators["holy pve"] = &priest::ClassSituationStrategyFactoryInternal::holy_pve;
                creators["holy raid"] = &priest::ClassSituationStrategyFactoryInternal::holy_raid;
                creators["shadow pvp"] = &priest::ClassSituationStrategyFactoryInternal::shadow_pvp;
                creators["shadow pve"] = &priest::ClassSituationStrategyFactoryInternal::shadow_pve;
                creators["shadow raid"] = &priest::ClassSituationStrategyFactoryInternal::shadow_raid;
            }

        private:
            static Strategy* discipline_pvp(PlayerbotAI* ai) { return new DisciplinePriestPvpStrategy(ai); }
            static Strategy* discipline_pve(PlayerbotAI* ai) { return new DisciplinePriestPveStrategy(ai); }
            static Strategy* discipline_raid(PlayerbotAI* ai) { return new DisciplinePriestRaidStrategy(ai); }
            static Strategy* holy_pvp(PlayerbotAI* ai) { return new HolyPriestPvpStrategy(ai); }
            static Strategy* holy_pve(PlayerbotAI* ai) { return new HolyPriestPveStrategy(ai); }
            static Strategy* holy_raid(PlayerbotAI* ai) { return new HolyPriestRaidStrategy(ai); }
            static Strategy* shadow_pvp(PlayerbotAI* ai) { return new ShadowPriestPvpStrategy(ai); }
            static Strategy* shadow_pve(PlayerbotAI* ai) { return new ShadowPriestPveStrategy(ai); }
            static Strategy* shadow_raid(PlayerbotAI* ai) { return new ShadowPriestRaidStrategy(ai); }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["devouring plague"] = &TriggerFactoryInternal::devouring_plague;
                creators["shadow word: pain"] = &TriggerFactoryInternal::shadow_word_pain;
                creators["shadow word: pain on attacker"] = &TriggerFactoryInternal::shadow_word_pain_on_attacker;
                creators["dispel magic"] = &TriggerFactoryInternal::dispel_magic;
                creators["dispel magic on party"] = &TriggerFactoryInternal::dispel_magic_party_member;
                creators["cure disease"] = &TriggerFactoryInternal::cure_disease;
                creators["party member cure disease"] = &TriggerFactoryInternal::party_member_cure_disease;
                creators["power word: fortitude"] = &TriggerFactoryInternal::power_word_fortitude;
                creators["power word: fortitude on party"] = &TriggerFactoryInternal::power_word_fortitude_on_party;
                creators["divine spirit"] = &TriggerFactoryInternal::divine_spirit;
                creators["divine spirit on party"] = &TriggerFactoryInternal::divine_spirit_on_party;
                creators["inner fire"] = &TriggerFactoryInternal::inner_fire;
                creators["vampiric touch"] = &TriggerFactoryInternal::vampiric_touch;
                creators["vampiric touch on attacker"] = &TriggerFactoryInternal::vampiric_touch_on_attacker;
                creators["vampiric embrace"] = &TriggerFactoryInternal::vampiric_embrace;
                creators["shadowform"] = &TriggerFactoryInternal::shadowform;
                creators["power infusion"] = &TriggerFactoryInternal::power_infusion;
                creators["inner focus"] = &TriggerFactoryInternal::inner_focus;
                creators["shadow protection"] = &TriggerFactoryInternal::shadow_protection;
                creators["shadow protection on party"] = &TriggerFactoryInternal::shadow_protection_on_party;
                creators["prayer of shadow protection on party"] = &TriggerFactoryInternal::prayer_of_shadow_protection_on_party;
                creators["prayer of fortitude on party"] = &TriggerFactoryInternal::prayer_of_fortitude_on_party;
                creators["prayer of spirit on party"] = &TriggerFactoryInternal::prayer_of_spirit_on_party;
                creators["shackle undead"] = &TriggerFactoryInternal::shackle_undead;
                creators["discipline fire"] = &TriggerFactoryInternal::discipline_fire;
                creators["touch of weakness"] = &TriggerFactoryInternal::touch_of_weakness;
                creators["hex of weakness"] = &TriggerFactoryInternal::hex_of_weakness;
                creators["shadowguard"] = &TriggerFactoryInternal::shadowguard;
                creators["starshards"] = &TriggerFactoryInternal::starshards;
                creators["fear ward"] = &TriggerFactoryInternal::fear_ward;
                creators["feedback"] = &TriggerFactoryInternal::feedback;
                creators["binding heal"] = &TriggerFactoryInternal::binding_heal;
                creators["chastise"] = &TriggerFactoryInternal::chastise;
                creators["silence"] = &TriggerFactoryInternal::silence;
                creators["silence on enemy healer"] = &TriggerFactoryInternal::silence_on_enemy_healer;
                creators["shadowfiend"] = &TriggerFactoryInternal::shadowfiend;
                creators["mind blast"] = &TriggerFactoryInternal::mind_blast;
                creators["smite"] = &TriggerFactoryInternal::smite;
                creators["holy fire"] = &TriggerFactoryInternal::holy_fire;
            }

        private:
            static Trigger* shadowfiend(PlayerbotAI* ai) { return new ShadowfiendTrigger(ai); }
            static Trigger* silence_on_enemy_healer(PlayerbotAI* ai) { return new SilenceEnemyHealerTrigger(ai); }
            static Trigger* silence(PlayerbotAI* ai) { return new SilenceTrigger(ai); }
            static Trigger* chastise(PlayerbotAI* ai) { return new ChastiseTrigger(ai); }
            static Trigger* binding_heal(PlayerbotAI* ai) { return new BindingHealTrigger(ai); }
            static Trigger* feedback(PlayerbotAI* ai) { return new FeedbackTrigger(ai); }
            static Trigger* fear_ward(PlayerbotAI* ai) { return new FearWardTrigger(ai); }
            static Trigger* shadowguard(PlayerbotAI* ai) { return new ShadowguardTrigger(ai); }
            static Trigger* starshards(PlayerbotAI* ai) { return new StarshardsTrigger(ai); }
            static Trigger* hex_of_weakness(PlayerbotAI* ai) { return new HexOfWeaknessTrigger(ai); }
            static Trigger* touch_of_weakness(PlayerbotAI* ai) { return new TouchOfWeaknessTrigger(ai); }
            static Trigger* discipline_fire(PlayerbotAI* ai) { return new HolyFireTrigger(ai); }
            static Trigger* shadowform(PlayerbotAI* ai) { return new ShadowformTrigger(ai); }
            static Trigger* vampiric_embrace(PlayerbotAI* ai) { return new VampiricEmbraceTrigger(ai); }
            static Trigger* vampiric_touch(PlayerbotAI* ai) { return new VampiricTouchTrigger(ai); }
            static Trigger* vampiric_touch_on_attacker(PlayerbotAI* ai) { return new VampiricTouchOnAttackerTrigger(ai); }
            static Trigger* devouring_plague(PlayerbotAI* ai) { return new DevouringPlagueTrigger(ai); }
            static Trigger* shadow_word_pain(PlayerbotAI* ai) { return new PowerWordPainTrigger(ai); }
            static Trigger* shadow_word_pain_on_attacker(PlayerbotAI* ai) { return new PowerWordPainOnAttackerTrigger(ai); }
            static Trigger* dispel_magic(PlayerbotAI* ai) { return new DispelMagicTrigger(ai); }
            static Trigger* dispel_magic_party_member(PlayerbotAI* ai) { return new DispelMagicPartyMemberTrigger(ai); }
            static Trigger* cure_disease(PlayerbotAI* ai) { return new CureDiseaseTrigger(ai); }
            static Trigger* party_member_cure_disease(PlayerbotAI* ai) { return new PartyMemberCureDiseaseTrigger(ai); }
            static Trigger* power_word_fortitude(PlayerbotAI* ai) { return new PowerWordFortitudeTrigger(ai); }
            static Trigger* power_word_fortitude_on_party(PlayerbotAI* ai) { return new PowerWordFortitudeOnPartyTrigger(ai); }
            static Trigger* divine_spirit(PlayerbotAI* ai) { return new DivineSpiritTrigger(ai); }
            static Trigger* divine_spirit_on_party(PlayerbotAI* ai) { return new DivineSpiritOnPartyTrigger(ai); }
            static Trigger* inner_fire(PlayerbotAI* ai) { return new InnerFireTrigger(ai); }
            static Trigger* power_infusion(PlayerbotAI* ai) { return new PowerInfusionTrigger(ai); }
            static Trigger* inner_focus(PlayerbotAI* ai) { return new InnerFocusTrigger(ai); }
            static Trigger* shadow_protection_on_party(PlayerbotAI* ai) { return new ShadowProtectionOnPartyTrigger(ai); }
            static Trigger* shadow_protection(PlayerbotAI* ai) { return new ShadowProtectionTrigger(ai); }
            static Trigger* shackle_undead(PlayerbotAI* ai) { return new ShackleUndeadTrigger(ai); }
            static Trigger* prayer_of_fortitude_on_party(PlayerbotAI* ai) { return new PrayerOfFortitudeOnPartyTrigger(ai); }
            static Trigger* prayer_of_spirit_on_party(PlayerbotAI* ai) { return new PrayerOfSpiritOnPartyTrigger(ai); }
            static Trigger* prayer_of_shadow_protection_on_party(PlayerbotAI* ai) { return new PrayerOfShadowProtectionOnPartyTrigger(ai); }
            static Trigger* mind_blast(PlayerbotAI* ai) { return new MindBlastTrigger(ai); }
            static Trigger* smite(PlayerbotAI* ai) { return new SmiteTrigger(ai); }
            static Trigger* holy_fire(PlayerbotAI* ai) { return new HolyFireTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["power infusion"] = &AiObjectContextInternal::power_infusion;
                creators["inner focus"] = &AiObjectContextInternal::inner_focus;
                creators["shadow word: pain"] = &AiObjectContextInternal::shadow_word_pain;
                creators["shadow word: pain on attacker"] = &AiObjectContextInternal::shadow_word_pain_on_attacker;
                creators["devouring plague"] = &AiObjectContextInternal::devouring_plague;
                creators["mind flay"] = &AiObjectContextInternal::mind_flay;
                creators["discipline fire"] = &AiObjectContextInternal::discipline_fire;
                creators["smite"] = &AiObjectContextInternal::smite;
                creators["mind blast"] = &AiObjectContextInternal::mind_blast;
                creators["shadowform"] = &AiObjectContextInternal::shadowform;
                creators["remove shadowform"] = &AiObjectContextInternal::remove_shadowform;
                creators["discipline nova"] = &AiObjectContextInternal::discipline_nova;
                creators["power word: fortitude"] = &AiObjectContextInternal::power_word_fortitude;
                creators["power word: fortitude on party"] = &AiObjectContextInternal::power_word_fortitude_on_party;
                creators["divine spirit"] = &AiObjectContextInternal::divine_spirit;
                creators["divine spirit on party"] = &AiObjectContextInternal::divine_spirit_on_party;
                creators["power word: shield"] = &AiObjectContextInternal::power_word_shield;
                creators["power word: shield on party"] = &AiObjectContextInternal::power_word_shield_on_party;
                creators["renew"] = &AiObjectContextInternal::renew;
                creators["renew on party"] = &AiObjectContextInternal::renew_on_party;
                creators["greater heal"] = &AiObjectContextInternal::greater_heal;
                creators["greater heal on party"] = &AiObjectContextInternal::greater_heal_on_party;
                creators["heal"] = &AiObjectContextInternal::heal;
                creators["heal on party"] = &AiObjectContextInternal::heal_on_party;
                creators["lesser heal"] = &AiObjectContextInternal::lesser_heal;
                creators["lesser heal on party"] = &AiObjectContextInternal::lesser_heal_on_party;
                creators["flash heal"] = &AiObjectContextInternal::flash_heal;
                creators["flash heal on party"] = &AiObjectContextInternal::flash_heal_on_party;
                creators["dispel magic"] = &AiObjectContextInternal::dispel_magic;
                creators["dispel magic on party"] = &AiObjectContextInternal::dispel_magic_on_party;
                creators["dispel magic on target"] = &AiObjectContextInternal::dispel_magic_on_target;
                creators["cure disease"] = &AiObjectContextInternal::cure_disease;
                creators["cure disease on party"] = &AiObjectContextInternal::cure_disease_on_party;
                creators["abolish disease"] = &AiObjectContextInternal::abolish_disease;
                creators["abolish disease on party"] = &AiObjectContextInternal::abolish_disease_on_party;
                creators["fade"] = &AiObjectContextInternal::fade;
                creators["inner fire"] = &AiObjectContextInternal::inner_fire;
                creators["resurrection"] = &AiObjectContextInternal::resurrection;
                creators["circle of healing"] = &AiObjectContextInternal::circle_of_healing;
                creators["psychic scream"] = &AiObjectContextInternal::psychic_scream;
                creators["vampiric touch"] = &AiObjectContextInternal::vampiric_touch;
                creators["vampiric touch on attacker"] = &AiObjectContextInternal::vampiric_touch_on_attacker;
                creators["vampiric embrace"] = &AiObjectContextInternal::vampiric_embrace;
                //creators["dispersion"] = &AiObjectContextInternal::dispersion;
                creators["shadow protection"] = &AiObjectContextInternal::shadow_protection;
                creators["shadow protection on party"] = &AiObjectContextInternal::shadow_protection_on_party;
                creators["shackle undead"] = &AiObjectContextInternal::shackle_undead;
                creators["prayer of fortitude on party"] = &AiObjectContextInternal::prayer_of_fortitude_on_party;
                creators["prayer of spirit on party"] = &AiObjectContextInternal::prayer_of_spirit_on_party;
                creators["prayer of shadow protection on party"] = &AiObjectContextInternal::prayer_of_shadow_protection_on_party;
                creators["silence"] = &AiObjectContextInternal::silence;
                creators["silence on enemy healer"] = &AiObjectContextInternal::silence_on_enemy_healer;
                creators["mana burn"] = &AiObjectContextInternal::mana_burn;
                creators["levitate"] = &AiObjectContextInternal::levitate;
                creators["prayer of healing"] = &AiObjectContextInternal::prayer_of_healing;
                creators["lightwell"] = &AiObjectContextInternal::lightwell;
                creators["mind soothe"] = &AiObjectContextInternal::mind_soothe;
                creators["touch of weakness"] = &AiObjectContextInternal::touch_of_weakness;
                creators["hex of weakness"] = &AiObjectContextInternal::hex_of_weakness;
                creators["shadowguard"] = &AiObjectContextInternal::shadowguard;
                creators["desperate prayer"] = &AiObjectContextInternal::desperate_prayer;
                creators["fear ward"] = &AiObjectContextInternal::fear_ward;
                creators["starshards"] = &AiObjectContextInternal::starshards;
                creators["elune's grace"] = &AiObjectContextInternal::elunes_grace;
                creators["feedback"] = &AiObjectContextInternal::feedback;
                creators["symbol of hope"] = &AiObjectContextInternal::symbol_of_hope;
                creators["consume magic"] = &AiObjectContextInternal::consume_magic;
                creators["chastise"] = &AiObjectContextInternal::chastise;
                creators["shadow word: death"] = &AiObjectContextInternal::shadow_word_death;
                creators["shadowfiend"] = &AiObjectContextInternal::shadowfiend;
                creators["mass dispel"] = &AiObjectContextInternal::mass_dispel;
                creators["pain suppression"] = &AiObjectContextInternal::pain_suppression;
                creators["pain suppression on party"] = &AiObjectContextInternal::pain_suppression_on_party;
                creators["prayer of mending"] = &AiObjectContextInternal::prayer_of_mending;
                creators["binding heal"] = &AiObjectContextInternal::binding_heal;
                creators["holy fire"] = &AiObjectContextInternal::holy_fire;
                creators["holy nova"] = &AiObjectContextInternal::holy_nova;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
            }

        private:
            static Action* binding_heal(PlayerbotAI* ai) { return new CastBindingHealAction(ai); }
            static Action* prayer_of_mending(PlayerbotAI* ai) { return new CastPrayerOfMendingAction(ai); }
            static Action* pain_suppression_on_party(PlayerbotAI* ai) { return new CastPainSuppressionProtectAction(ai); }
            static Action* pain_suppression(PlayerbotAI* ai) { return new CastPainSuppressionAction(ai); }
            static Action* mass_dispel(PlayerbotAI* ai) { return new CastMassDispelAction(ai); }
            static Action* shadowfiend(PlayerbotAI* ai) { return new CastShadowfiendAction(ai); }
            static Action* shadow_word_death(PlayerbotAI* ai) { return new CastShadowWordDeathAction(ai); }
            static Action* chastise(PlayerbotAI* ai) { return new CastChastiseAction(ai); }
            static Action* consume_magic(PlayerbotAI* ai) { return new CastConsumeMagicAction(ai); }
            static Action* symbol_of_hope(PlayerbotAI* ai) { return new CastSymbolOfHopeAction(ai); }
            static Action* feedback(PlayerbotAI* ai) { return new CastFeedbackAction(ai); }
            static Action* elunes_grace(PlayerbotAI* ai) { return new CastElunesGraceAction(ai); }
            static Action* starshards(PlayerbotAI* ai) { return new CastStarshardsAction(ai); }
            static Action* fear_ward(PlayerbotAI* ai) { return new CastFearWardAction(ai); }
            static Action* desperate_prayer(PlayerbotAI* ai) { return new CastDesperatePrayerAction(ai); }
            static Action* shadowguard(PlayerbotAI* ai) { return new CastShadowguardAction(ai); }
            static Action* hex_of_weakness(PlayerbotAI* ai) { return new CastHexOfWeaknessAction(ai); }
            static Action* touch_of_weakness(PlayerbotAI* ai) { return new CastTouchOfWeaknessAction(ai); }
            static Action* mind_soothe(PlayerbotAI* ai) { return new CastMindSootheAction(ai); }
            static Action* lightwell(PlayerbotAI* ai) { return new CastLightwellAction(ai); }
            static Action* prayer_of_healing(PlayerbotAI* ai) { return new CastPrayerOfHealingAction(ai); }
            static Action* levitate(PlayerbotAI* ai) { return new CastLevitateAction(ai); }
            static Action* mana_burn(PlayerbotAI* ai) { return new CastManaBurnAction(ai); }
            static Action* silence_on_enemy_healer(PlayerbotAI* ai) { return new CastSilenceOnEnemyHealerAction(ai); }
            static Action* silence(PlayerbotAI* ai) { return new CastSilenceAction(ai); }
            static Action* prayer_of_shadow_protection_on_party(PlayerbotAI* ai) { return new CastPrayerOfShadowProtectionAction(ai); }
            static Action* prayer_of_spirit_on_party(PlayerbotAI* ai) { return new CastPrayerOfSpiritOnPartyAction(ai); }
            static Action* prayer_of_fortitude_on_party(PlayerbotAI* ai) { return new CastPrayerOfFortitudeOnPartyAction(ai); }
            static Action* shackle_undead(PlayerbotAI* ai) { return new CastShackleUndeadAction(ai); }
            static Action* shadow_protection_on_party(PlayerbotAI* ai) { return new CastShadowProtectionOnPartyAction(ai); }
            static Action* shadow_protection(PlayerbotAI* ai) { return new CastShadowProtectionAction(ai); }
            static Action* power_infusion(PlayerbotAI* ai) { return new CastPowerInfusionAction(ai); }
            static Action* inner_focus(PlayerbotAI* ai) { return new CastInnerFocusAction(ai); }
            //static Action* dispersion(PlayerbotAI* ai) { return new CastDispersionAction(ai); }
            static Action* vampiric_embrace(PlayerbotAI* ai) { return new CastVampiricEmbraceAction(ai); }
            static Action* vampiric_touch(PlayerbotAI* ai) { return new CastVampiricTouchAction(ai); }
            static Action* vampiric_touch_on_attacker(PlayerbotAI* ai) { return new CastVampiricTouchActionOnAttacker(ai); }
            static Action* psychic_scream(PlayerbotAI* ai) { return new CastPsychicScreamAction(ai); }
            static Action* circle_of_healing(PlayerbotAI* ai) { return new CastCircleOfHealingAction(ai); }
            static Action* resurrection(PlayerbotAI* ai) { return new CastResurrectionAction(ai); }
            static Action* shadow_word_pain(PlayerbotAI* ai) { return new CastPowerWordPainAction(ai); }
            static Action* shadow_word_pain_on_attacker(PlayerbotAI* ai) { return new CastPowerWordPainOnAttackerAction(ai); }
            static Action* devouring_plague(PlayerbotAI* ai) { return new CastDevouringPlagueAction(ai); }
            static Action* mind_flay(PlayerbotAI* ai) { return new CastMindFlayAction(ai); }
            static Action* discipline_fire(PlayerbotAI* ai) { return new CastHolyFireAction(ai); }
            static Action* smite(PlayerbotAI* ai) { return new CastSmiteAction(ai); }
            static Action* mind_blast(PlayerbotAI* ai) { return new CastMindBlastAction(ai); }
            static Action* shadowform(PlayerbotAI* ai) { return new CastShadowformAction(ai); }
            static Action* remove_shadowform(PlayerbotAI* ai) { return new CastRemoveShadowformAction(ai); }
            static Action* discipline_nova(PlayerbotAI* ai) { return new CastHolyNovaAction(ai); }
            static Action* power_word_fortitude(PlayerbotAI* ai) { return new CastPowerWordFortitudeAction(ai); }
            static Action* power_word_fortitude_on_party(PlayerbotAI* ai) { return new CastPowerWordFortitudeOnPartyAction(ai); }
            static Action* divine_spirit(PlayerbotAI* ai) { return new CastDivineSpiritAction(ai); }
            static Action* divine_spirit_on_party(PlayerbotAI* ai) { return new CastDivineSpiritOnPartyAction(ai); }
            static Action* power_word_shield(PlayerbotAI* ai) { return new CastPowerWordShieldAction(ai); }
            static Action* power_word_shield_on_party(PlayerbotAI* ai) { return new CastPowerWordShieldOnPartyAction(ai); }
            static Action* renew(PlayerbotAI* ai) { return new CastRenewAction(ai); }
            static Action* renew_on_party(PlayerbotAI* ai) { return new CastRenewOnPartyAction(ai); }
            static Action* greater_heal(PlayerbotAI* ai) { return new CastGreaterHealAction(ai); }
            static Action* greater_heal_on_party(PlayerbotAI* ai) { return new CastGreaterHealOnPartyAction(ai); }
            static Action* heal(PlayerbotAI* ai) { return new CastHealAction(ai); }
            static Action* heal_on_party(PlayerbotAI* ai) { return new CastHealOnPartyAction(ai); }
            static Action* lesser_heal(PlayerbotAI* ai) { return new CastLesserHealAction(ai); }
            static Action* lesser_heal_on_party(PlayerbotAI* ai) { return new CastLesserHealOnPartyAction(ai); }
            static Action* flash_heal(PlayerbotAI* ai) { return new CastFlashHealAction(ai); }
            static Action* flash_heal_on_party(PlayerbotAI* ai) { return new CastFlashHealOnPartyAction(ai); }
            static Action* dispel_magic(PlayerbotAI* ai) { return new CastDispelMagicAction(ai); }
            static Action* dispel_magic_on_party(PlayerbotAI* ai) { return new CastDispelMagicOnPartyAction(ai); }
            static Action* dispel_magic_on_target(PlayerbotAI* ai) { return new CastDispelMagicOnTargetAction(ai); }
            static Action* cure_disease(PlayerbotAI* ai) { return new CastCureDiseaseAction(ai); }
            static Action* cure_disease_on_party(PlayerbotAI* ai) { return new CastCureDiseaseOnPartyAction(ai); }
            static Action* abolish_disease(PlayerbotAI* ai) { return new CastAbolishDiseaseAction(ai); }
            static Action* abolish_disease_on_party(PlayerbotAI* ai) { return new CastAbolishDiseaseOnPartyAction(ai); }
            static Action* fade(PlayerbotAI* ai) { return new CastFadeAction(ai); }
            static Action* inner_fire(PlayerbotAI* ai) { return new CastInnerFireAction(ai); }
            static Action* holy_fire(PlayerbotAI* ai) { return new CastHolyFireAction(ai); }
            static Action* holy_nova(PlayerbotAI* ai) { return new CastHolyNovaAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdatePriestPveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdatePriestPvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdatePriestRaidStrategiesAction(ai); }
        };
    };
};

PriestAiObjectContext::PriestAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::priest::StrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::OffdpsSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::priest::AiObjectContextInternal());
    triggerContexts.Add(new ai::priest::TriggerFactoryInternal());
}
