
#include "playerbot/playerbot.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "WarriorActions.h"
#include "WarriorTriggers.h"
#include "WarriorAiObjectContext.h"
#include "ProtectionWarriorStrategy.h"
#include "ArmsWarriorStrategy.h"
#include "FuryWarriorStrategy.h"

namespace ai
{
    namespace warrior
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = &warrior::StrategyFactoryInternal::aoe;
                creators["buff"] = &warrior::StrategyFactoryInternal::buff;
                creators["boost"] = &warrior::StrategyFactoryInternal::boost;
                creators["pull"] = &warrior::StrategyFactoryInternal::pull;
                creators["cc"] = &warrior::StrategyFactoryInternal::cc;
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
                creators["aoe arms pve"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_arms_pve;
                creators["aoe arms pvp"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_arms_pvp;
                creators["aoe arms raid"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_arms_raid;
                creators["aoe fury pve"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_fury_pve;
                creators["aoe fury pvp"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_fury_pvp;
                creators["aoe fury raid"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_fury_raid;
                creators["aoe protection pve"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_protection_pve;
                creators["aoe protection pvp"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_protection_pvp;
                creators["aoe protection raid"] = &warrior::AoeSituationStrategyFactoryInternal::aoe_protection_raid;
            }

        private:
            static Strategy* aoe_arms_pve(PlayerbotAI* ai) { return new ArmsWarriorAoePveStrategy(ai); }
            static Strategy* aoe_arms_pvp(PlayerbotAI* ai) { return new ArmsWarriorAoePvpStrategy(ai); }
            static Strategy* aoe_arms_raid(PlayerbotAI* ai) { return new ArmsWarriorAoeRaidStrategy(ai); }
            static Strategy* aoe_fury_pve(PlayerbotAI* ai) { return new FuryWarriorAoePveStrategy(ai); }
            static Strategy* aoe_fury_pvp(PlayerbotAI* ai) { return new FuryWarriorAoePvpStrategy(ai); }
            static Strategy* aoe_fury_raid(PlayerbotAI* ai) { return new FuryWarriorAoeRaidStrategy(ai); }
            static Strategy* aoe_protection_pve(PlayerbotAI* ai) { return new ProtectionWarriorAoePveStrategy(ai); }
            static Strategy* aoe_protection_pvp(PlayerbotAI* ai) { return new ProtectionWarriorAoePvpStrategy(ai); }
            static Strategy* aoe_protection_raid(PlayerbotAI* ai) { return new ProtectionWarriorAoeRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff arms pve"] = &warrior::BuffSituationStrategyFactoryInternal::buff_arms_pve;
                creators["buff arms pvp"] = &warrior::BuffSituationStrategyFactoryInternal::buff_arms_pvp;
                creators["buff arms raid"] = &warrior::BuffSituationStrategyFactoryInternal::buff_arms_raid;
                creators["buff fury pve"] = &warrior::BuffSituationStrategyFactoryInternal::buff_fury_pve;
                creators["buff fury pvp"] = &warrior::BuffSituationStrategyFactoryInternal::buff_fury_pvp;
                creators["buff fury raid"] = &warrior::BuffSituationStrategyFactoryInternal::buff_fury_raid;
                creators["buff protection pve"] = &warrior::BuffSituationStrategyFactoryInternal::buff_protection_pve;
                creators["buff protection pvp"] = &warrior::BuffSituationStrategyFactoryInternal::buff_protection_pvp;
                creators["buff protection raid"] = &warrior::BuffSituationStrategyFactoryInternal::buff_protection_raid;
            }

        private:
            static Strategy* buff_arms_pve(PlayerbotAI* ai) { return new ArmsWarriorBuffPveStrategy(ai); }
            static Strategy* buff_arms_pvp(PlayerbotAI* ai) { return new ArmsWarriorBuffPvpStrategy(ai); }
            static Strategy* buff_arms_raid(PlayerbotAI* ai) { return new ArmsWarriorBuffRaidStrategy(ai); }
            static Strategy* buff_fury_pve(PlayerbotAI* ai) { return new FuryWarriorBuffPveStrategy(ai); }
            static Strategy* buff_fury_pvp(PlayerbotAI* ai) { return new FuryWarriorBuffPvpStrategy(ai); }
            static Strategy* buff_fury_raid(PlayerbotAI* ai) { return new FuryWarriorBuffRaidStrategy(ai); }
            static Strategy* buff_protection_pve(PlayerbotAI* ai) { return new ProtectionWarriorBuffPveStrategy(ai); }
            static Strategy* buff_protection_pvp(PlayerbotAI* ai) { return new ProtectionWarriorBuffPvpStrategy(ai); }
            static Strategy* buff_protection_raid(PlayerbotAI* ai) { return new ProtectionWarriorBuffRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost arms pve"] = &warrior::BoostSituationStrategyFactoryInternal::boost_arms_pve;
                creators["boost arms pvp"] = &warrior::BoostSituationStrategyFactoryInternal::boost_arms_pvp;
                creators["boost arms raid"] = &warrior::BoostSituationStrategyFactoryInternal::boost_arms_raid;
                creators["boost fury pve"] = &warrior::BoostSituationStrategyFactoryInternal::boost_fury_pve;
                creators["boost fury pvp"] = &warrior::BoostSituationStrategyFactoryInternal::boost_fury_pvp;
                creators["boost fury raid"] = &warrior::BoostSituationStrategyFactoryInternal::boost_fury_raid;
                creators["boost protection pve"] = &warrior::BoostSituationStrategyFactoryInternal::boost_protection_pve;
                creators["boost protection pvp"] = &warrior::BoostSituationStrategyFactoryInternal::boost_protection_pvp;
                creators["boost protection raid"] = &warrior::BoostSituationStrategyFactoryInternal::boost_protection_raid;
            }

        private:
            static Strategy* boost_arms_pve(PlayerbotAI* ai) { return new ArmsWarriorBoostPveStrategy(ai); }
            static Strategy* boost_arms_pvp(PlayerbotAI* ai) { return new ArmsWarriorBoostPvpStrategy(ai); }
            static Strategy* boost_arms_raid(PlayerbotAI* ai) { return new ArmsWarriorBoostRaidStrategy(ai); }
            static Strategy* boost_fury_pve(PlayerbotAI* ai) { return new FuryWarriorBoostPveStrategy(ai); }
            static Strategy* boost_fury_pvp(PlayerbotAI* ai) { return new FuryWarriorBoostPvpStrategy(ai); }
            static Strategy* boost_fury_raid(PlayerbotAI* ai) { return new FuryWarriorBoostRaidStrategy(ai); }
            static Strategy* boost_protection_pve(PlayerbotAI* ai) { return new ProtectionWarriorBoostPveStrategy(ai); }
            static Strategy* boost_protection_pvp(PlayerbotAI* ai) { return new ProtectionWarriorBoostPvpStrategy(ai); }
            static Strategy* boost_protection_raid(PlayerbotAI* ai) { return new ProtectionWarriorBoostRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc fury pve"] = &warrior::CcSituationStrategyFactoryInternal::cc_fury_pve;
                creators["cc fury pvp"] = &warrior::CcSituationStrategyFactoryInternal::cc_fury_pvp;
                creators["cc fury raid"] = &warrior::CcSituationStrategyFactoryInternal::cc_fury_raid;
                creators["cc protection pve"] = &warrior::CcSituationStrategyFactoryInternal::cc_protection_pve;
                creators["cc protection pvp"] = &warrior::CcSituationStrategyFactoryInternal::cc_protection_pvp;
                creators["cc protection raid"] = &warrior::CcSituationStrategyFactoryInternal::cc_protection_raid;
                creators["cc arms pve"] = &warrior::CcSituationStrategyFactoryInternal::cc_arms_pve;
                creators["cc arms pvp"] = &warrior::CcSituationStrategyFactoryInternal::cc_arms_pvp;
                creators["cc arms raid"] = &warrior::CcSituationStrategyFactoryInternal::cc_arms_raid;
            }

        private:
            static Strategy* cc_fury_pve(PlayerbotAI* ai) { return new FuryWarriorCcPveStrategy(ai); }
            static Strategy* cc_fury_pvp(PlayerbotAI* ai) { return new FuryWarriorCcPvpStrategy(ai); }
            static Strategy* cc_fury_raid(PlayerbotAI* ai) { return new FuryWarriorCcRaidStrategy(ai); }
            static Strategy* cc_protection_pve(PlayerbotAI* ai) { return new ProtectionWarriorCcPveStrategy(ai); }
            static Strategy* cc_protection_pvp(PlayerbotAI* ai) { return new ProtectionWarriorCcPvpStrategy(ai); }
            static Strategy* cc_protection_raid(PlayerbotAI* ai) { return new ProtectionWarriorCcRaidStrategy(ai); }
            static Strategy* cc_arms_pve(PlayerbotAI* ai) { return new ArmsWarriorCcPveStrategy(ai); }
            static Strategy* cc_arms_pvp(PlayerbotAI* ai) { return new ArmsWarriorCcPvpStrategy(ai); }
            static Strategy* cc_arms_raid(PlayerbotAI* ai) { return new ArmsWarriorCcRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["fury"] = &warrior::ClassStrategyFactoryInternal::fury;
                creators["protection"] = &warrior::ClassStrategyFactoryInternal::protection;
                creators["tank"] = &warrior::ClassStrategyFactoryInternal::protection;
                creators["arms"] = &warrior::ClassStrategyFactoryInternal::arms;
            }

        private:
            static Strategy* fury(PlayerbotAI* ai) { return new FuryWarriorPlaceholderStrategy(ai); }
            static Strategy* protection(PlayerbotAI* ai) { return new ProtectionWarriorPlaceholderStrategy(ai); }
            static Strategy* arms(PlayerbotAI* ai) { return new ArmsWarriorPlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["arms pvp"] = &warrior::ClassSituationStrategyFactoryInternal::arms_pvp;
                creators["arms pve"] = &warrior::ClassSituationStrategyFactoryInternal::arms_pve;
                creators["arms raid"] = &warrior::ClassSituationStrategyFactoryInternal::arms_raid;
                creators["fury pvp"] = &warrior::ClassSituationStrategyFactoryInternal::fury_pvp;
                creators["fury pve"] = &warrior::ClassSituationStrategyFactoryInternal::fury_pve;
                creators["fury raid"] = &warrior::ClassSituationStrategyFactoryInternal::fury_raid;
                creators["protection pvp"] = &warrior::ClassSituationStrategyFactoryInternal::protection_pvp;
                creators["protection pve"] = &warrior::ClassSituationStrategyFactoryInternal::protection_pve;
                creators["protection raid"] = &warrior::ClassSituationStrategyFactoryInternal::protection_raid;
            }

        private:
            static Strategy* arms_pvp(PlayerbotAI* ai) { return new ArmsWarriorPvpStrategy(ai); }
            static Strategy* arms_pve(PlayerbotAI* ai) { return new ArmsWarriorPveStrategy(ai); }
            static Strategy* arms_raid(PlayerbotAI* ai) { return new ArmsWarriorRaidStrategy(ai); }
            static Strategy* fury_pvp(PlayerbotAI* ai) { return new FuryWarriorPvpStrategy(ai); }
            static Strategy* fury_pve(PlayerbotAI* ai) { return new FuryWarriorPveStrategy(ai); }
            static Strategy* fury_raid(PlayerbotAI* ai) { return new FuryWarriorRaidStrategy(ai); }
            static Strategy* protection_pvp(PlayerbotAI* ai) { return new ProtectionWarriorPvpStrategy(ai); }
            static Strategy* protection_pve(PlayerbotAI* ai) { return new ProtectionWarriorPveStrategy(ai); }
            static Strategy* protection_raid(PlayerbotAI* ai) { return new ProtectionWarriorRaidStrategy(ai); }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["hamstring"] = &TriggerFactoryInternal::hamstring;
                creators["victory rush"] = &TriggerFactoryInternal::victory_rush;
                creators["death wish"] = &TriggerFactoryInternal::death_wish;
                creators["battle shout"] = &TriggerFactoryInternal::battle_shout;
                creators["rend"] = &TriggerFactoryInternal::rend;
                creators["rend on attacker"] = &TriggerFactoryInternal::rend_on_attacker;
                creators["bloodrage"] = &TriggerFactoryInternal::bloodrage;
                creators["shield bash"] = &TriggerFactoryInternal::shield_bash;
                creators["disarm"] = &TriggerFactoryInternal::disarm;
                creators["concussion blow"] = &TriggerFactoryInternal::concussion_blow;
                creators["sword and board"] = &TriggerFactoryInternal::SwordAndBoard;
                creators["shield bash on enemy healer"] = &TriggerFactoryInternal::shield_bash_on_enemy_healer;
                creators["battle stance"] = &TriggerFactoryInternal::battle_stance;
                creators["defensive stance"] = &TriggerFactoryInternal::defensive_stance;
                creators["berserker stance"] = &TriggerFactoryInternal::berserker_stance;
                creators["shield block"] = &TriggerFactoryInternal::shield_block;
                creators["sunder armor"] = &TriggerFactoryInternal::sunder_armor;
                creators["demoralizing shout"] = &TriggerFactoryInternal::demoralizing_shout;
                creators["devastate"] = &TriggerFactoryInternal::devastate;
                creators["revenge"] = &TriggerFactoryInternal::revenge;
                creators["overpower"] = &TriggerFactoryInternal::overpower;
                creators["mocking blow"] = &TriggerFactoryInternal::mocking_blow;
                creators["rampage"] = &TriggerFactoryInternal::rampage;
                creators["mortal strike"] = &TriggerFactoryInternal::mortal_strike;
                creators["thunder clap on snare target"] = &TriggerFactoryInternal::thunder_clap_on_snare_target;
                creators["thunder clap"] = &TriggerFactoryInternal::thunder_clap;
                creators["bloodthirst"] = &TriggerFactoryInternal::bloodthirst;
                creators["whirlwind"] = &TriggerFactoryInternal::whirlwind;
                creators["heroic strike"] = &TriggerFactoryInternal::heroic_strike;
                creators["berserker rage"] = &TriggerFactoryInternal::berserker_rage;
                creators["pummel on enemy healer"] = &TriggerFactoryInternal::pummel_on_enemy_healer;
                creators["pummel"] = &TriggerFactoryInternal::pummel;
                creators["intercept on enemy healer"] = &TriggerFactoryInternal::intercept_on_enemy_healer;
                creators["intercept"] = &TriggerFactoryInternal::intercept;
                creators["taunt on snare target"] = &TriggerFactoryInternal::taunt_on_snare_target;
                creators["commanding shout"] = &TriggerFactoryInternal::commanding_shout;
                creators["intercept on snare target"] = &TriggerFactoryInternal::intercept_on_snare_target;
                creators["spell reflection"] = &TriggerFactoryInternal::spell_reflection;
                creators["sudden death"] = &TriggerFactoryInternal::sudden_death;
                creators["instant slam"] = &TriggerFactoryInternal::instant_slam;
                creators["shockwave"] = &TriggerFactoryInternal::shockwave;
                creators["shockwave on snare target"] = &TriggerFactoryInternal::shockwave_on_snare_target;
                creators["taste for blood"] = &TriggerFactoryInternal::taste_for_blood;
                creators["thunder clap and rage"] = &TriggerFactoryInternal::thunderclap_and_rage;
                creators["thunder clap and rage and aoe"] = &TriggerFactoryInternal::thunderclap_and_rage_and_aoe;
                creators["intercept can cast"] = &TriggerFactoryInternal::intercept_can_cast;
                creators["intercept and far enemy"] = &TriggerFactoryInternal::intercept_and_far_enemy;
                creators["intercept and rage"] = &TriggerFactoryInternal::intercept_and_rage;
                creators["recklessness"] = &TriggerFactoryInternal::recklessness;
                creators["slam"] = &TriggerFactoryInternal::slam;
            }

        private:
            static Trigger* taste_for_blood(PlayerbotAI* ai) { return new TasteForBloodTrigger(ai); }
            static Trigger* shockwave_on_snare_target(PlayerbotAI* ai) { return new ShockwaveSnareTrigger(ai); }
            static Trigger* shockwave(PlayerbotAI* ai) { return new ShockwaveTrigger(ai); }
            static Trigger* instant_slam(PlayerbotAI* ai) { return new SlamInstantTrigger(ai); }
            static Trigger* sudden_death(PlayerbotAI* ai) { return new SuddenDeathTrigger(ai); }
            static Trigger* spell_reflection(PlayerbotAI* ai) { return new SpellReflectionTrigger(ai); }
            static Trigger* intercept_on_snare_target(PlayerbotAI* ai) { return new InterceptSnareTrigger(ai); }
            static Trigger* commanding_shout(PlayerbotAI* ai) { return new CommandingShoutTrigger(ai); }
            static Trigger* taunt_on_snare_target(PlayerbotAI* ai) { return new TauntSnareTrigger(ai); }
            static Trigger* intercept(PlayerbotAI* ai) { return new InterceptInterruptSpellTrigger(ai); }
            static Trigger* intercept_on_enemy_healer(PlayerbotAI* ai) { return new InterceptInterruptEnemyHealerSpellTrigger(ai); }
            static Trigger* pummel(PlayerbotAI* ai) { return new PummelInterruptSpellTrigger(ai); }
            static Trigger* pummel_on_enemy_healer(PlayerbotAI* ai) { return new PummelInterruptEnemyHealerSpellTrigger(ai); }
            static Trigger* berserker_rage(PlayerbotAI* ai) { return new BerserkerRageBuffTrigger(ai); }
            static Trigger* bloodthirst(PlayerbotAI* ai) { return new BloodthirstBuffTrigger(ai); }
            static Trigger* whirlwind(PlayerbotAI* ai) { return new WhirlwindTrigger(ai); }
            static Trigger* heroic_strike(PlayerbotAI* ai) { return new HeroicStrikeTrigger(ai); }
            static Trigger* thunder_clap_on_snare_target(PlayerbotAI* ai) { return new ThunderClapSnareTrigger(ai); }
            static Trigger* thunder_clap(PlayerbotAI* ai) { return new ThunderClapTrigger(ai); }
            static Trigger* mortal_strike(PlayerbotAI* ai) { return new MortalStrikeDebuffTrigger(ai); }
            static Trigger* rampage(PlayerbotAI* ai) { return new RampageAvailableTrigger(ai); }
            static Trigger* mocking_blow(PlayerbotAI* ai) { return new MockingBlowTrigger(ai); }
            static Trigger* overpower(PlayerbotAI* ai) { return new OverpowerAvailableTrigger(ai); }
            static Trigger* devastate(PlayerbotAI* ai) { return new DevastateAvailableTrigger(ai); }
            static Trigger* revenge(PlayerbotAI* ai) { return new RevengeAvailableTrigger(ai); }
            static Trigger* sunder_armor(PlayerbotAI* ai) { return new SunderArmorDebuffTrigger(ai); }
            static Trigger* demoralizing_shout(PlayerbotAI* ai) { return new DemoralizingShoutDebuffTrigger(ai); }
            static Trigger* shield_block(PlayerbotAI* ai) { return new ShieldBlockTrigger(ai); }
            static Trigger* berserker_stance(PlayerbotAI* ai) { return new BerserkerStanceTrigger(ai); }
            static Trigger* defensive_stance(PlayerbotAI* ai) { return new DefensiveStanceTrigger(ai); }
            static Trigger* battle_stance(PlayerbotAI* ai) { return new BattleStanceTrigger(ai); }
            static Trigger* hamstring(PlayerbotAI* ai) { return new HamstringTrigger(ai); }
            static Trigger* victory_rush(PlayerbotAI* ai) { return new VictoryRushTrigger(ai); }
            static Trigger* death_wish(PlayerbotAI* ai) { return new DeathWishTrigger(ai); }
            static Trigger* battle_shout(PlayerbotAI* ai) { return new BattleShoutTrigger(ai); }
            static Trigger* rend(PlayerbotAI* ai) { return new RendDebuffTrigger(ai); }
            static Trigger* rend_on_attacker(PlayerbotAI* ai) { return new RendDebuffOnAttackerTrigger(ai); }
            static Trigger* bloodrage(PlayerbotAI* ai) { return new BloodrageBuffTrigger(ai); }
            static Trigger* shield_bash(PlayerbotAI* ai) { return new ShieldBashInterruptSpellTrigger(ai); }
            static Trigger* disarm(PlayerbotAI* ai) { return new DisarmDebuffTrigger(ai); }
            static Trigger* concussion_blow(PlayerbotAI* ai) { return new ConcussionBlowTrigger(ai); }
            static Trigger* SwordAndBoard(PlayerbotAI* ai) { return new SwordAndBoardTrigger(ai); }
            static Trigger* shield_bash_on_enemy_healer(PlayerbotAI* ai) { return new ShieldBashInterruptEnemyHealerSpellTrigger(ai); }
            static Trigger* thunderclap_and_rage(PlayerbotAI* ai) { return new TwoTriggers(ai, "thunder clap", "medium rage available"); }
            static Trigger* thunderclap_and_rage_and_aoe(PlayerbotAI* ai) { return new TwoTriggers(ai, "thunder clap and rage", "ranged light aoe"); }
            static Trigger* intercept_can_cast(PlayerbotAI* ai) { return new InterceptCanCastTrigger(ai); }
            static Trigger* intercept_and_far_enemy(PlayerbotAI* ai) { return new TwoTriggers(ai, "enemy is out of melee", "intercept can cast"); }
            static Trigger* intercept_and_rage(PlayerbotAI* ai) { return new TwoTriggers(ai, "intercept and far enemy", "light rage available"); }
            static Trigger* recklessness(PlayerbotAI* ai) { return new RecklessnessTrigger(ai); }
            static Trigger* slam(PlayerbotAI* ai) { return new SlamTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["devastate"] = &AiObjectContextInternal::devastate;
                creators["overpower"] = &AiObjectContextInternal::overpower;
                creators["charge"] = &AiObjectContextInternal::charge;
                creators["bloodthirst"] = &AiObjectContextInternal::bloodthirst;
                creators["rend"] = &AiObjectContextInternal::rend;
                creators["rend on attacker"] = &AiObjectContextInternal::rend_on_attacker;
                creators["mocking blow"] = &AiObjectContextInternal::mocking_blow;
                creators["death wish"] = &AiObjectContextInternal::death_wish;
                creators["berserker rage"] = &AiObjectContextInternal::berserker_rage;
                creators["victory rush"] = &AiObjectContextInternal::victory_rush;
                creators["execute"] = &AiObjectContextInternal::execute;
                creators["defensive stance"] = &AiObjectContextInternal::defensive_stance;
                creators["hamstring"] = &AiObjectContextInternal::hamstring;
                creators["shield bash"] = &AiObjectContextInternal::shield_bash;
                creators["shield block"] = &AiObjectContextInternal::shield_block;
                creators["bloodrage"] = &AiObjectContextInternal::bloodrage;
                creators["battle stance"] = &AiObjectContextInternal::battle_stance;
                creators["heroic strike"] = &AiObjectContextInternal::heroic_strike;
                creators["intimidating shout"] = &AiObjectContextInternal::intimidating_shout;
                creators["demoralizing shout"] = &AiObjectContextInternal::demoralizing_shout;
                creators["challenging shout"] = &AiObjectContextInternal::challenging_shout;
                creators["shield wall"] = &AiObjectContextInternal::shield_wall;
                creators["battle shout"] = &AiObjectContextInternal::battle_shout;
                creators["battle shout taunt"] = &AiObjectContextInternal::battle_shout_taunt;
                creators["thunder clap"] = &AiObjectContextInternal::thunder_clap;
                creators["taunt"] = &AiObjectContextInternal::taunt;
                creators["revenge"] = &AiObjectContextInternal::revenge;
                creators["slam"] = &AiObjectContextInternal::slam;
                creators["shield slam"] = &AiObjectContextInternal::shield_slam;
                creators["disarm"] = &AiObjectContextInternal::disarm;
                creators["sunder armor"] = &AiObjectContextInternal::sunder_armor;
                creators["last stand"] = &AiObjectContextInternal::last_stand;
                creators["shockwave"] = &AiObjectContextInternal::shockwave;
                creators["shockwave on snare target"] = &AiObjectContextInternal::shockwave_on_snare_target;
                creators["cleave"] = &AiObjectContextInternal::cleave;
                creators["concussion blow"] = &AiObjectContextInternal::concussion_blow;
                creators["shield bash on enemy healer"] = &AiObjectContextInternal::shield_bash_on_enemy_healer;
                creators["berserker stance"] = &AiObjectContextInternal::berserker_stance;
                creators["commanding shout"] = &AiObjectContextInternal::commanding_shout;
                creators["retaliation"] = &AiObjectContextInternal::retaliation;
                creators["mortal strike"] = &AiObjectContextInternal::mortal_strike;
                creators["sweeping strikes"] = &AiObjectContextInternal::sweeping_strikes;
                creators["intercept"] = &AiObjectContextInternal::intercept;
                creators["whirlwind"] = &AiObjectContextInternal::whirlwind;
                creators["pummel"] = &AiObjectContextInternal::pummel;
                creators["pummel on enemy healer"] = &AiObjectContextInternal::pummel_on_enemy_healer;
                creators["recklessness"] = &AiObjectContextInternal::recklessness;
                creators["piercing howl"] = &AiObjectContextInternal::piercing_howl;
                creators["rampage"] = &AiObjectContextInternal::rampage;
                creators["intervene"] = &AiObjectContextInternal::intervene;
                creators["spell reflection"] = &AiObjectContextInternal::spell_reflection;
                creators["thunder clap on snare target"] = &AiObjectContextInternal::thunder_clap_on_snare_target;
                creators["taunt on snare target"] = &AiObjectContextInternal::taunt_on_snare_target;
                creators["intercept on enemy healer"] = &AiObjectContextInternal::intercept_on_enemy_healer;
                creators["intercept on snare target"] = &AiObjectContextInternal::intercept_on_snare_target;
                creators["bladestorm"] = &AiObjectContextInternal::bladestorm;
                creators["heroic throw"] = &AiObjectContextInternal::heroic_throw;
                creators["heroic throw on snare target"] = &AiObjectContextInternal::heroic_throw_on_snare_target;
                creators["shattering throw"] = &AiObjectContextInternal::shattering_throw;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
            }

        private:
            static Action* shattering_throw(PlayerbotAI* ai) { return new CastShatteringThrowAction(ai); }
            static Action* heroic_throw_on_snare_target(PlayerbotAI* ai) { return new CastHeroicThrowSnareAction(ai); }
            static Action* heroic_throw(PlayerbotAI* ai) { return new CastHeroicThrowAction(ai); }
            static Action* bladestorm(PlayerbotAI* ai) { return new CastBladestormAction(ai); }
            static Action* intercept_on_snare_target(PlayerbotAI* ai) { return new CastInterceptOnSnareTargetAction(ai); }
            static Action* intercept_on_enemy_healer(PlayerbotAI* ai) { return new CastInterceptOnEnemyHealerAction(ai); }
            static Action* taunt_on_snare_target(PlayerbotAI* ai) { return new CastTauntOnSnareTargetAction(ai); }
            static Action* thunder_clap_on_snare_target(PlayerbotAI* ai) { return new CastThunderClapSnareAction(ai); }
            static Action* berserker_stance(PlayerbotAI* ai) { return new CastBerserkerStanceAction(ai); }
            static Action* commanding_shout(PlayerbotAI* ai) { return new CastCommandingShoutAction(ai); }
            static Action* retaliation(PlayerbotAI* ai) { return new CastRetaliationAction(ai); }
            static Action* mortal_strike(PlayerbotAI* ai) { return new CastMortalStrikeAction(ai); }
            static Action* sweeping_strikes(PlayerbotAI* ai) { return new CastSweepingStrikesAction(ai); }
            static Action* intercept(PlayerbotAI* ai) { return new CastInterceptAction(ai); }
            static Action* whirlwind(PlayerbotAI* ai) { return new CastWhirlwindAction(ai); }
            static Action* pummel(PlayerbotAI* ai) { return new CastPummelAction (ai); }
            static Action* pummel_on_enemy_healer(PlayerbotAI* ai) { return new CastPummelOnEnemyHealerAction(ai); }
            static Action* recklessness(PlayerbotAI* ai) { return new CastRecklessnessAction(ai); }
            static Action* piercing_howl(PlayerbotAI* ai) { return new CastPiercingHowlAction(ai); }
            static Action* rampage(PlayerbotAI* ai) { return new CastRampageAction(ai); }
            static Action* intervene(PlayerbotAI* ai) { return new CastInterveneAction(ai); }
            static Action* spell_reflection(PlayerbotAI* ai) { return new CastSpellReflectionAction(ai); }
            static Action* devastate(PlayerbotAI* ai) { return new CastDevastateAction(ai); }
            static Action* last_stand(PlayerbotAI* ai) { return new CastLastStandAction(ai); }
            static Action* shockwave(PlayerbotAI* ai) { return new CastShockwaveAction(ai); }
            static Action* shockwave_on_snare_target(PlayerbotAI* ai) { return new CastShockwaveSnareAction(ai); }
            static Action* cleave(PlayerbotAI* ai) { return new CastCleaveAction(ai); }
            static Action* concussion_blow(PlayerbotAI* ai) { return new CastConcussionBlowAction(ai); }
            static Action* taunt(PlayerbotAI* ai) { return new CastTauntAction(ai); }
            static Action* revenge(PlayerbotAI* ai) { return new CastRevengeAction(ai); }
            static Action* slam(PlayerbotAI* ai) { return new CastSlamAction(ai); }
            static Action* shield_slam(PlayerbotAI* ai) { return new CastShieldSlamAction(ai); }
            static Action* disarm(PlayerbotAI* ai) { return new CastDisarmAction(ai); }
            static Action* sunder_armor(PlayerbotAI* ai) { return new CastSunderArmorAction(ai); }
            static Action* overpower(PlayerbotAI* ai) { return new CastOverpowerAction(ai); }
            static Action* charge(PlayerbotAI* ai) { return new CastChargeAction(ai); }
            static Action* bloodthirst(PlayerbotAI* ai) { return new CastBloodthirstAction(ai); }
            static Action* rend(PlayerbotAI* ai) { return new CastRendAction(ai); }
            static Action* rend_on_attacker(PlayerbotAI* ai) { return new CastRendOnAttackerAction(ai); }
            static Action* mocking_blow(PlayerbotAI* ai) { return new CastMockingBlowAction(ai); }
            static Action* death_wish(PlayerbotAI* ai) { return new CastDeathWishAction(ai); }
            static Action* berserker_rage(PlayerbotAI* ai) { return new CastBerserkerRageAction(ai); }
            static Action* victory_rush(PlayerbotAI* ai) { return new CastVictoryRushAction(ai); }
            static Action* execute(PlayerbotAI* ai) { return new CastExecuteAction(ai); }
            static Action* defensive_stance(PlayerbotAI* ai) { return new CastDefensiveStanceAction(ai); }
            static Action* hamstring(PlayerbotAI* ai) { return new CastHamstringAction(ai); }
            static Action* shield_bash(PlayerbotAI* ai) { return new CastShieldBashAction(ai); }
            static Action* shield_block(PlayerbotAI* ai) { return new CastShieldBlockAction(ai); }
            static Action* bloodrage(PlayerbotAI* ai) { return new CastBloodrageAction(ai); }
            static Action* battle_stance(PlayerbotAI* ai) { return new CastBattleStanceAction(ai); }
            static Action* heroic_strike(PlayerbotAI* ai) { return new CastHeroicStrikeAction(ai); }
            static Action* intimidating_shout(PlayerbotAI* ai) { return new CastIntimidatingShoutAction(ai); }
            static Action* demoralizing_shout(PlayerbotAI* ai) { return new CastDemoralizingShoutAction(ai); }
            static Action* challenging_shout(PlayerbotAI* ai) { return new CastChallengingShoutAction(ai); }
            static Action* shield_wall(PlayerbotAI* ai) { return new CastShieldWallAction(ai); }
            static Action* battle_shout(PlayerbotAI* ai) { return new CastBattleShoutAction(ai); }
            static Action* battle_shout_taunt(PlayerbotAI* ai) { return new CastBattleShoutTauntAction(ai); }
            static Action* thunder_clap(PlayerbotAI* ai) { return new CastThunderClapAction(ai); }
            static Action* shield_bash_on_enemy_healer(PlayerbotAI* ai) { return new CastShieldBashOnEnemyHealerAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdateWarriorPveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdateWarriorPvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdateWarriorRaidStrategiesAction(ai); }
        };
    };
};

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::warrior::StrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::warrior::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::warrior::AiObjectContextInternal());
    triggerContexts.Add(new ai::warrior::TriggerFactoryInternal());
}
