
#include "playerbot/playerbot.h"
#include "PriestActions.h"
#include "PriestAiObjectContext.h"
#include "PriestNonCombatStrategy.h"
#include "PriestReactionStrategy.h"
#include "ShadowPriestStrategy.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "PriestTriggers.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "HolyPriestStrategy.h"

using namespace ai;


namespace ai
{
    namespace priest
    {
        using namespace ai;

        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["nc"] = &priest::StrategyFactoryInternal::nc;
                creators["react"] = &priest::StrategyFactoryInternal::react;
                creators["pull"] = &priest::StrategyFactoryInternal::pull;
                creators["aoe"] = &priest::StrategyFactoryInternal::shadow_aoe;
                creators["shadow aoe"] = &priest::StrategyFactoryInternal::shadow_aoe;
                creators["dps debuff"] = &priest::StrategyFactoryInternal::shadow_debuff;
                creators["shadow debuff"] = &priest::StrategyFactoryInternal::shadow_debuff;
                creators["cure"] = &priest::StrategyFactoryInternal::cure;
                creators["buff"] = &priest::StrategyFactoryInternal::buff;
                creators["boost"] = &priest::StrategyFactoryInternal::boost;
                creators["rshadow"] = &priest::StrategyFactoryInternal::rshadow;
                creators["cc"] = &priest::StrategyFactoryInternal::cc;
            }

        private:
            static Strategy* cc(PlayerbotAI* ai) { return new PriestCcStrategy(ai); }
            static Strategy* rshadow(PlayerbotAI* ai) { return new PriestShadowResistanceStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new PriestBoostStrategy(ai); }
            static Strategy* buff(PlayerbotAI* ai) { return new PriestBuffStrategy(ai); }
            static Strategy* nc(PlayerbotAI* ai) { return new PriestNonCombatStrategy(ai); }
            static Strategy* react(PlayerbotAI* ai) { return new PriestReactionStrategy(ai); }
            static Strategy* shadow_aoe(PlayerbotAI* ai) { return new ShadowPriestAoeStrategy(ai); }
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "shoot"); }
            static Strategy* shadow_debuff(PlayerbotAI* ai) { return new ShadowPriestDebuffStrategy(ai); }
            static Strategy* cure(PlayerbotAI* ai) { return new PriestCureStrategy(ai); }
        };

        class CombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["heal"] = &priest::CombatStrategyFactoryInternal::heal;
                creators["shadow"] = &priest::CombatStrategyFactoryInternal::dps;
                creators["dps"] = &priest::CombatStrategyFactoryInternal::dps;
                creators["holy"] = &priest::CombatStrategyFactoryInternal::holy;
            }

        private:
            static Strategy* heal(PlayerbotAI* ai) { return new HealPriestStrategy(ai); }
            static Strategy* dps(PlayerbotAI* ai) { return new ShadowPriestStrategy(ai); }
            static Strategy* holy(PlayerbotAI* ai) { return new HolyPriestStrategy(ai); }
        };
    };
};

namespace ai
{
    namespace priest
    {
        using namespace ai;

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
                creators["holy fire"] = &TriggerFactoryInternal::holy_fire;
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
            static Trigger* holy_fire(PlayerbotAI* ai) { return new HolyFireTrigger(ai); }
            static Trigger* shadowform(PlayerbotAI* ai) { return new ShadowformTrigger(ai); }
            static Trigger* vampiric_embrace(PlayerbotAI* ai) { return new VampiricEmbraceTrigger(ai); }
            static Trigger* vampiric_touch(PlayerbotAI* ai) { return new VampiricTouchTrigger(ai); }
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
        };
    };
};



namespace ai
{
    namespace priest
    {
        using namespace ai;

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["power infusion"] = &AiObjectContextInternal::power_infusion;
                creators["power infusion on party"] = &AiObjectContextInternal::power_infusion_on_party;
                creators["inner focus"] = &AiObjectContextInternal::inner_focus;
                creators["shadow word: pain"] = &AiObjectContextInternal::shadow_word_pain;
                creators["shadow word: pain on attacker"] = &AiObjectContextInternal::shadow_word_pain_on_attacker;
                creators["devouring plague"] = &AiObjectContextInternal::devouring_plague;
                creators["mind flay"] = &AiObjectContextInternal::mind_flay;
                creators["holy fire"] = &AiObjectContextInternal::holy_fire;
                creators["smite"] = &AiObjectContextInternal::smite;
                creators["mind blast"] = &AiObjectContextInternal::mind_blast;
                creators["shadowform"] = &AiObjectContextInternal::shadowform;
                creators["remove shadowform"] = &AiObjectContextInternal::remove_shadowform;
                creators["holy nova"] = &AiObjectContextInternal::holy_nova;
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
                creators["fear ward on party"] = &AiObjectContextInternal::fear_ward_on_party;
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
            static Action* fear_ward_on_party(PlayerbotAI* ai) { return new CastFearWardOnPartyAction(ai); }
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
            static Action* power_infusion_on_party(PlayerbotAI* ai) { return new CastPowerInfusionOnPartyAction(ai); }
            static Action* inner_focus(PlayerbotAI* ai) { return new CastInnerFocusAction(ai); }
            //static Action* dispersion(PlayerbotAI* ai) { return new CastDispersionAction(ai); }
            static Action* vampiric_embrace(PlayerbotAI* ai) { return new CastVampiricEmbraceAction(ai); }
            static Action* vampiric_touch(PlayerbotAI* ai) { return new CastVampiricTouchAction(ai); }
            static Action* psychic_scream(PlayerbotAI* ai) { return new CastPsychicScreamAction(ai); }
            static Action* circle_of_healing(PlayerbotAI* ai) { return new CastCircleOfHealingAction(ai); }
            static Action* resurrection(PlayerbotAI* ai) { return new CastResurrectionAction(ai); }
            static Action* shadow_word_pain(PlayerbotAI* ai) { return new CastPowerWordPainAction(ai); }
            static Action* shadow_word_pain_on_attacker(PlayerbotAI* ai) { return new CastPowerWordPainOnAttackerAction(ai); }
            static Action* devouring_plague(PlayerbotAI* ai) { return new CastDevouringPlagueAction(ai); }
            static Action* mind_flay(PlayerbotAI* ai) { return new CastMindFlayAction(ai); }
            static Action* holy_fire(PlayerbotAI* ai) { return new CastHolyFireAction(ai); }
            static Action* smite(PlayerbotAI* ai) { return new CastSmiteAction(ai); }
            static Action* mind_blast(PlayerbotAI* ai) { return new CastMindBlastAction(ai); }
            static Action* shadowform(PlayerbotAI* ai) { return new CastShadowformAction(ai); }
            static Action* remove_shadowform(PlayerbotAI* ai) { return new CastRemoveShadowformAction(ai); }
            static Action* holy_nova(PlayerbotAI* ai) { return new CastHolyNovaAction(ai); }
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
        };
    };
};

PriestAiObjectContext::PriestAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::priest::StrategyFactoryInternal());
    strategyContexts.Add(new ai::priest::CombatStrategyFactoryInternal());
    actionContexts.Add(new ai::priest::AiObjectContextInternal());
    triggerContexts.Add(new ai::priest::TriggerFactoryInternal());
}
