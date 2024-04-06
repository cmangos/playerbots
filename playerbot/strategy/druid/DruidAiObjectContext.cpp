
#include "playerbot/playerbot.h"
#include "DruidActions.h"
#include "DruidAiObjectContext.h"
#include "TankFeralDruidStrategy.h"
#include "DpsFeralDruidStrategy.h"
#include "BalanceDruidStrategy.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "DruidTriggers.h"
#include "RestorationDruidStrategy.h"
#include "playerbot/strategy/generic/PullStrategy.h"

namespace ai
{
    namespace druid
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["pull"] = &druid::StrategyFactoryInternal::pull;
                creators["aoe"] = &druid::StrategyFactoryInternal::aoe;
                creators["buff"] = &druid::StrategyFactoryInternal::buff;
                creators["boost"] = &druid::StrategyFactoryInternal::boost;
                creators["stealth"] = &druid::StrategyFactoryInternal::stealth;
                creators["stealthed"] = &druid::StrategyFactoryInternal::stealthed;
                creators["cc"] = &druid::StrategyFactoryInternal::cc;
                creators["cure"] = &druid::StrategyFactoryInternal::cure;
                creators["powershift"] = &druid::StrategyFactoryInternal::powershift;
                creators["offheal"] = &druid::StrategyFactoryInternal::offheal;
            }

        private:
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "faerie fire", "dire bear form"); }
            static Strategy* aoe(PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); }
            static Strategy* buff(PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); }
            static Strategy* stealth(PlayerbotAI* ai) { return new StealthPlaceholderStrategy(ai); }
            static Strategy* stealthed(PlayerbotAI* ai) { return new DpsFeralDruidStealthedStrategy(ai); }
            static Strategy* cc(PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); }
            static Strategy* cure(PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); }
            static Strategy* powershift(PlayerbotAI* ai) { return new DpsFeralDruidPowershiftStrategy(ai); }
            static Strategy* offheal(PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); }
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe tank feral pve"] = &druid::AoeSituationStrategyFactoryInternal::aoe_tank_feral_pve;
                creators["aoe tank feral pvp"] = &druid::AoeSituationStrategyFactoryInternal::aoe_tank_feral_pvp;
                creators["aoe tank feral raid"] = &druid::AoeSituationStrategyFactoryInternal::aoe_tank_feral_raid;
                creators["aoe dps feral pve"] = &druid::AoeSituationStrategyFactoryInternal::aoe_dps_feral_pve;
                creators["aoe dps feral pvp"] = &druid::AoeSituationStrategyFactoryInternal::aoe_dps_feral_pvp;
                creators["aoe dps feral raid"] = &druid::AoeSituationStrategyFactoryInternal::aoe_dps_feral_raid;
                creators["aoe restoration pve"] = &druid::AoeSituationStrategyFactoryInternal::aoe_restoration_pve;
                creators["aoe restoration pvp"] = &druid::AoeSituationStrategyFactoryInternal::aoe_restoration_pvp;
                creators["aoe restoration raid"] = &druid::AoeSituationStrategyFactoryInternal::aoe_restoration_raid;
                creators["aoe balance pve"] = &druid::AoeSituationStrategyFactoryInternal::aoe_balance_pve;
                creators["aoe balance pvp"] = &druid::AoeSituationStrategyFactoryInternal::aoe_balance_pvp;
                creators["aoe balance raid"] = &druid::AoeSituationStrategyFactoryInternal::aoe_balance_raid;
            }

        private:
            static Strategy* aoe_tank_feral_pve(PlayerbotAI* ai) { return new TankFeralDruidAoePveStrategy(ai); }
            static Strategy* aoe_tank_feral_pvp(PlayerbotAI* ai) { return new TankFeralDruidAoePvpStrategy(ai); }
            static Strategy* aoe_tank_feral_raid(PlayerbotAI* ai) { return new TankFeralDruidAoeRaidStrategy(ai); }
            static Strategy* aoe_dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidAoePveStrategy(ai); }
            static Strategy* aoe_dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidAoePvpStrategy(ai); }
            static Strategy* aoe_dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidAoeRaidStrategy(ai); }
            static Strategy* aoe_restoration_pve(PlayerbotAI* ai) { return new RestorationDruidAoePveStrategy(ai); }
            static Strategy* aoe_restoration_pvp(PlayerbotAI* ai) { return new RestorationDruidAoePvpStrategy(ai); }
            static Strategy* aoe_restoration_raid(PlayerbotAI* ai) { return new RestorationDruidAoeRaidStrategy(ai); }
            static Strategy* aoe_balance_pve(PlayerbotAI* ai) { return new BalanceDruidAoePveStrategy(ai); }
            static Strategy* aoe_balance_pvp(PlayerbotAI* ai) { return new BalanceDruidAoePvpStrategy(ai); }
            static Strategy* aoe_balance_raid(PlayerbotAI* ai) { return new BalanceDruidAoeRaidStrategy(ai); }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure tank feral pve"] = &druid::CureSituationStrategyFactoryInternal::cure_tank_feral_pve;
                creators["cure tank feral pvp"] = &druid::CureSituationStrategyFactoryInternal::cure_tank_feral_pvp;
                creators["cure tank feral raid"] = &druid::CureSituationStrategyFactoryInternal::cure_tank_feral_raid;
                creators["cure dps feral pve"] = &druid::CureSituationStrategyFactoryInternal::cure_dps_feral_pve;
                creators["cure dps feral pvp"] = &druid::CureSituationStrategyFactoryInternal::cure_dps_feral_pvp;
                creators["cure dps feral raid"] = &druid::CureSituationStrategyFactoryInternal::cure_dps_feral_raid;
                creators["cure restoration pve"] = &druid::CureSituationStrategyFactoryInternal::cure_restoration_pve;
                creators["cure restoration pvp"] = &druid::CureSituationStrategyFactoryInternal::cure_restoration_pvp;
                creators["cure restoration raid"] = &druid::CureSituationStrategyFactoryInternal::cure_restoration_raid;
                creators["cure balance pve"] = &druid::CureSituationStrategyFactoryInternal::cure_balance_pve;
                creators["cure balance pvp"] = &druid::CureSituationStrategyFactoryInternal::cure_balance_pvp;
                creators["cure balance raid"] = &druid::CureSituationStrategyFactoryInternal::cure_balance_raid;
            }

        private:
            static Strategy* cure_tank_feral_pve(PlayerbotAI* ai) { return new TankFeralDruidCurePveStrategy(ai); }
            static Strategy* cure_tank_feral_pvp(PlayerbotAI* ai) { return new TankFeralDruidCurePvpStrategy(ai); }
            static Strategy* cure_tank_feral_raid(PlayerbotAI* ai) { return new TankFeralDruidCureRaidStrategy(ai); }
            static Strategy* cure_dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidCurePveStrategy(ai); }
            static Strategy* cure_dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidCurePvpStrategy(ai); }
            static Strategy* cure_dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidCureRaidStrategy(ai); }
            static Strategy* cure_restoration_pve(PlayerbotAI* ai) { return new RestorationDruidCurePveStrategy(ai); }
            static Strategy* cure_restoration_pvp(PlayerbotAI* ai) { return new RestorationDruidCurePvpStrategy(ai); }
            static Strategy* cure_restoration_raid(PlayerbotAI* ai) { return new RestorationDruidCureRaidStrategy(ai); }
            static Strategy* cure_balance_pve(PlayerbotAI* ai) { return new BalanceDruidCurePveStrategy(ai); }
            static Strategy* cure_balance_pvp(PlayerbotAI* ai) { return new BalanceDruidCurePvpStrategy(ai); }
            static Strategy* cure_balance_raid(PlayerbotAI* ai) { return new BalanceDruidCureRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff tank feral pve"] = &druid::BuffSituationStrategyFactoryInternal::buff_tank_feral_pve;
                creators["buff tank feral pvp"] = &druid::BuffSituationStrategyFactoryInternal::buff_tank_feral_pvp;
                creators["buff tank feral raid"] = &druid::BuffSituationStrategyFactoryInternal::buff_tank_feral_raid;
                creators["buff dps feral pve"] = &druid::BuffSituationStrategyFactoryInternal::buff_dps_feral_pve;
                creators["buff dps feral pvp"] = &druid::BuffSituationStrategyFactoryInternal::buff_dps_feral_pvp;
                creators["buff dps feral raid"] = &druid::BuffSituationStrategyFactoryInternal::buff_dps_feral_raid;
                creators["buff restoration pve"] = &druid::BuffSituationStrategyFactoryInternal::buff_restoration_pve;
                creators["buff restoration pvp"] = &druid::BuffSituationStrategyFactoryInternal::buff_restoration_pvp;
                creators["buff restoration raid"] = &druid::BuffSituationStrategyFactoryInternal::buff_restoration_raid;
                creators["buff balance pve"] = &druid::BuffSituationStrategyFactoryInternal::buff_balance_pve;
                creators["buff balance pvp"] = &druid::BuffSituationStrategyFactoryInternal::buff_balance_pvp;
                creators["buff balance raid"] = &druid::BuffSituationStrategyFactoryInternal::buff_balance_raid;
            }

        private:
            static Strategy* buff_tank_feral_pve(PlayerbotAI* ai) { return new TankFeralDruidBuffPveStrategy(ai); }
            static Strategy* buff_tank_feral_pvp(PlayerbotAI* ai) { return new TankFeralDruidBuffPvpStrategy(ai); }
            static Strategy* buff_tank_feral_raid(PlayerbotAI* ai) { return new TankFeralDruidBuffRaidStrategy(ai); }
            static Strategy* buff_dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidBuffPveStrategy(ai); }
            static Strategy* buff_dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidBuffPvpStrategy(ai); }
            static Strategy* buff_dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidBuffRaidStrategy(ai); }
            static Strategy* buff_restoration_pve(PlayerbotAI* ai) { return new RestorationDruidBuffPveStrategy(ai); }
            static Strategy* buff_restoration_pvp(PlayerbotAI* ai) { return new RestorationDruidBuffPvpStrategy(ai); }
            static Strategy* buff_restoration_raid(PlayerbotAI* ai) { return new RestorationDruidBuffRaidStrategy(ai); }
            static Strategy* buff_balance_pve(PlayerbotAI* ai) { return new BalanceDruidBuffPveStrategy(ai); }
            static Strategy* buff_balance_pvp(PlayerbotAI* ai) { return new BalanceDruidBuffPvpStrategy(ai); }
            static Strategy* buff_balance_raid(PlayerbotAI* ai) { return new BalanceDruidBuffRaidStrategy(ai); }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = &druid::OffhealSituationStrategyFactoryInternal::offheal_pve;
                creators["offheal pvp"] = &druid::OffhealSituationStrategyFactoryInternal::offheal_pvp;
                creators["offheal raid"] = &druid::OffhealSituationStrategyFactoryInternal::offheal_raid;
            }

        private:
            static Strategy* offheal_pve(PlayerbotAI* ai) { return new DruidOffhealPveStrategy(ai); }
            static Strategy* offheal_pvp(PlayerbotAI* ai) { return new DruidOffhealPvpStrategy(ai); }
            static Strategy* offheal_raid(PlayerbotAI* ai) { return new DruidOffhealRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost tank feral pve"] = &druid::BoostSituationStrategyFactoryInternal::boost_tank_feral_pve;
                creators["boost tank feral pvp"] = &druid::BoostSituationStrategyFactoryInternal::boost_tank_feral_pvp;
                creators["boost tank feral raid"] = &druid::BoostSituationStrategyFactoryInternal::boost_tank_feral_raid;
                creators["boost dps feral pve"] = &druid::BoostSituationStrategyFactoryInternal::boost_dps_feral_pve;
                creators["boost dps feral pvp"] = &druid::BoostSituationStrategyFactoryInternal::boost_dps_feral_pvp;
                creators["boost dps feral raid"] = &druid::BoostSituationStrategyFactoryInternal::boost_dps_feral_raid;
                creators["boost restoration pve"] = &druid::BoostSituationStrategyFactoryInternal::boost_restoration_pve;
                creators["boost restoration pvp"] = &druid::BoostSituationStrategyFactoryInternal::boost_restoration_pvp;
                creators["boost restoration raid"] = &druid::BoostSituationStrategyFactoryInternal::boost_restoration_raid;
                creators["boost balance pve"] = &druid::BoostSituationStrategyFactoryInternal::boost_balance_pve;
                creators["boost balance pvp"] = &druid::BoostSituationStrategyFactoryInternal::boost_balance_pvp;
                creators["boost balance raid"] = &druid::BoostSituationStrategyFactoryInternal::boost_balance_raid;
            }

        private:
            static Strategy* boost_tank_feral_pve(PlayerbotAI* ai) { return new TankFeralDruidBoostPveStrategy(ai); }
            static Strategy* boost_tank_feral_pvp(PlayerbotAI* ai) { return new TankFeralDruidBoostPvpStrategy(ai); }
            static Strategy* boost_tank_feral_raid(PlayerbotAI* ai) { return new TankFeralDruidBoostRaidStrategy(ai); }
            static Strategy* boost_dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidBoostPveStrategy(ai); }
            static Strategy* boost_dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidBoostPvpStrategy(ai); }
            static Strategy* boost_dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidBoostRaidStrategy(ai); }
            static Strategy* boost_restoration_pve(PlayerbotAI* ai) { return new RestorationDruidBoostPveStrategy(ai); }
            static Strategy* boost_restoration_pvp(PlayerbotAI* ai) { return new RestorationDruidBoostPvpStrategy(ai); }
            static Strategy* boost_restoration_raid(PlayerbotAI* ai) { return new RestorationDruidBoostRaidStrategy(ai); }
            static Strategy* boost_balance_pve(PlayerbotAI* ai) { return new BalanceDruidBoostPveStrategy(ai); }
            static Strategy* boost_balance_pvp(PlayerbotAI* ai) { return new BalanceDruidBoostPvpStrategy(ai); }
            static Strategy* boost_balance_raid(PlayerbotAI* ai) { return new BalanceDruidBoostRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc tank feral pve"] = &druid::CcSituationStrategyFactoryInternal::cc_tank_feral_pve;
                creators["cc tank feral pvp"] = &druid::CcSituationStrategyFactoryInternal::cc_tank_feral_pvp;
                creators["cc tank feral raid"] = &druid::CcSituationStrategyFactoryInternal::cc_tank_feral_raid;
                creators["cc dps feral pve"] = &druid::CcSituationStrategyFactoryInternal::cc_dps_feral_pve;
                creators["cc dps feral pvp"] = &druid::CcSituationStrategyFactoryInternal::cc_dps_feral_pvp;
                creators["cc dps feral raid"] = &druid::CcSituationStrategyFactoryInternal::cc_dps_feral_raid;
                creators["cc restoration pve"] = &druid::CcSituationStrategyFactoryInternal::cc_restoration_pve;
                creators["cc restoration pvp"] = &druid::CcSituationStrategyFactoryInternal::cc_restoration_pvp;
                creators["cc restoration raid"] = &druid::CcSituationStrategyFactoryInternal::cc_restoration_raid;
                creators["cc balance pve"] = &druid::CcSituationStrategyFactoryInternal::cc_balance_pve;
                creators["cc balance pvp"] = &druid::CcSituationStrategyFactoryInternal::cc_balance_pvp;
                creators["cc balance raid"] = &druid::CcSituationStrategyFactoryInternal::cc_balance_raid;
            }

        private:
            static Strategy* cc_tank_feral_pve(PlayerbotAI* ai) { return new TankFeralDruidCcPveStrategy(ai); }
            static Strategy* cc_tank_feral_pvp(PlayerbotAI* ai) { return new TankFeralDruidCcPvpStrategy(ai); }
            static Strategy* cc_tank_feral_raid(PlayerbotAI* ai) { return new TankFeralDruidCcRaidStrategy(ai); }
            static Strategy* cc_dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidCcPveStrategy(ai); }
            static Strategy* cc_dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidCcPvpStrategy(ai); }
            static Strategy* cc_dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidCcRaidStrategy(ai); }
            static Strategy* cc_restoration_pve(PlayerbotAI* ai) { return new RestorationDruidCcPveStrategy(ai); }
            static Strategy* cc_restoration_pvp(PlayerbotAI* ai) { return new RestorationDruidCcPvpStrategy(ai); }
            static Strategy* cc_restoration_raid(PlayerbotAI* ai) { return new RestorationDruidCcRaidStrategy(ai); }
            static Strategy* cc_balance_pve(PlayerbotAI* ai) { return new BalanceDruidCcPveStrategy(ai); }
            static Strategy* cc_balance_pvp(PlayerbotAI* ai) { return new BalanceDruidCcPvpStrategy(ai); }
            static Strategy* cc_balance_raid(PlayerbotAI* ai) { return new BalanceDruidCcRaidStrategy(ai); }
        };

        class StealthSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StealthSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["stealth dps feral pve"] = &druid::StealthSituationStrategyFactoryInternal::stealth_dps_feral_pve;
                creators["stealth dps feral pvp"] = &druid::StealthSituationStrategyFactoryInternal::stealth_dps_feral_pvp;
                creators["stealth dps feral raid"] = &druid::StealthSituationStrategyFactoryInternal::stealth_dps_feral_raid;
            }

        private:
            static Strategy* stealth_dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidStealthPveStrategy(ai); }
            static Strategy* stealth_dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidStealthPvpStrategy(ai); }
            static Strategy* stealth_dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidStealthRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["bear"] = &druid::ClassStrategyFactoryInternal::tank_feral;
                creators["tank"] = &druid::ClassStrategyFactoryInternal::tank_feral;
                creators["tank feral"] = &druid::ClassStrategyFactoryInternal::tank_feral;
                creators["cat"] = &druid::ClassStrategyFactoryInternal::dps_feral;
                creators["dps feral"] = &druid::ClassStrategyFactoryInternal::dps_feral;
                creators["balance"] = &druid::ClassStrategyFactoryInternal::balance;
                creators["restoration"] = &druid::ClassStrategyFactoryInternal::restoration;
                creators["heal"] = &druid::ClassStrategyFactoryInternal::restoration;
            }

        private:
            static Strategy* tank_feral(PlayerbotAI* ai) { return new TankFeralDruidPlaceholderStrategy(ai); }
            static Strategy* dps_feral(PlayerbotAI* ai) { return new DpsFeralDruidPlaceholderStrategy(ai); }
            static Strategy* balance(PlayerbotAI* ai) { return new BalanceDruidPlaceholderStrategy(ai); }
            static Strategy* restoration(PlayerbotAI* ai) { return new RestorationDruidPlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["tank feral pvp"] = &druid::ClassSituationStrategyFactoryInternal::tank_feral_pvp;
                creators["tank feral pve"] = &druid::ClassSituationStrategyFactoryInternal::tank_feral_pve;
                creators["tank feral raid"] = &druid::ClassSituationStrategyFactoryInternal::tank_feral_raid;
                creators["dps feral pvp"] = &druid::ClassSituationStrategyFactoryInternal::dps_feral_pvp;
                creators["dps feral pve"] = &druid::ClassSituationStrategyFactoryInternal::dps_feral_pve;
                creators["dps feral raid"] = &druid::ClassSituationStrategyFactoryInternal::dps_feral_raid;
                creators["restoration pvp"] = &druid::ClassSituationStrategyFactoryInternal::restoration_pvp;
                creators["restoration pve"] = &druid::ClassSituationStrategyFactoryInternal::restoration_pve;
                creators["restoration raid"] = &druid::ClassSituationStrategyFactoryInternal::restoration_raid;
                creators["balance pvp"] = &druid::ClassSituationStrategyFactoryInternal::balance_pvp;
                creators["balance pve"] = &druid::ClassSituationStrategyFactoryInternal::balance_pve;
                creators["balance raid"] = &druid::ClassSituationStrategyFactoryInternal::balance_raid;
            }

        private:
            static Strategy* tank_feral_pvp(PlayerbotAI* ai) { return new TankFeralDruidPvpStrategy(ai); }
            static Strategy* tank_feral_pve(PlayerbotAI* ai) { return new TankFeralDruidPveStrategy(ai); }
            static Strategy* tank_feral_raid(PlayerbotAI* ai) { return new TankFeralDruidRaidStrategy(ai); }
            static Strategy* dps_feral_pvp(PlayerbotAI* ai) { return new DpsFeralDruidPvpStrategy(ai); }
            static Strategy* dps_feral_pve(PlayerbotAI* ai) { return new DpsFeralDruidPveStrategy(ai); }
            static Strategy* dps_feral_raid(PlayerbotAI* ai) { return new DpsFeralDruidRaidStrategy(ai); }
            static Strategy* restoration_pvp(PlayerbotAI* ai) { return new RestorationDruidPvpStrategy(ai); }
            static Strategy* restoration_pve(PlayerbotAI* ai) { return new RestorationDruidPveStrategy(ai); }
            static Strategy* restoration_raid(PlayerbotAI* ai) { return new RestorationDruidRaidStrategy(ai); }
            static Strategy* balance_pvp(PlayerbotAI* ai) { return new BalanceDruidPvpStrategy(ai); }
            static Strategy* balance_pve(PlayerbotAI* ai) { return new BalanceDruidPveStrategy(ai); }
            static Strategy* balance_raid(PlayerbotAI* ai) { return new BalanceDruidRaidStrategy(ai); }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["omen of clarity"] = &TriggerFactoryInternal::omen_of_clarity;
                creators["thorns"] = &TriggerFactoryInternal::thorns;
                creators["thorns on party"] = &TriggerFactoryInternal::thorns_on_party;
                creators["bash"] = &TriggerFactoryInternal::bash;
                creators["faerie fire (feral)"] = &TriggerFactoryInternal::faerie_fire_feral;
                creators["faerie fire"] = &TriggerFactoryInternal::faerie_fire;
                creators["insect swarm"] = &TriggerFactoryInternal::insect_swarm;
                creators["moonfire"] = &TriggerFactoryInternal::moonfire;
                creators["nature's grasp"] = &TriggerFactoryInternal::natures_grasp;
                creators["tiger's fury"] = &TriggerFactoryInternal::tigers_fury;
                creators["rake"] = &TriggerFactoryInternal::rake;
                creators["mark of the wild"] = &TriggerFactoryInternal::mark_of_the_wild;
                creators["mark of the wild on party"] = &TriggerFactoryInternal::mark_of_the_wild_on_party;
                creators["gift of the wild on party"] = &TriggerFactoryInternal::gift_of_the_wild_on_party;
                creators["cure poison"] = &TriggerFactoryInternal::cure_poison;
                creators["party member cure poison"] = &TriggerFactoryInternal::party_member_cure_poison;
                creators["remove curse"] = &TriggerFactoryInternal::remove_curse;
                creators["party member remove curse"] = &TriggerFactoryInternal::party_member_remove_curse;
                creators["entangling roots"] = &TriggerFactoryInternal::entangling_roots;
                creators["entangling roots kite"] = &TriggerFactoryInternal::entangling_roots_kite;
                creators["entangling roots snare"] = &TriggerFactoryInternal::entangling_roots_snare;
                creators["hibernate"] = &TriggerFactoryInternal::hibernate;
                creators["bear form"] = &TriggerFactoryInternal::bear_form;
                creators["cat form"] = &TriggerFactoryInternal::cat_form;
                creators["tree form"] = &TriggerFactoryInternal::tree_form;
                creators["moonkin form"] = &TriggerFactoryInternal::moonkin_form;
                creators["eclipse (solar)"] = &TriggerFactoryInternal::eclipse_solar;
                creators["eclipse (lunar)"] = &TriggerFactoryInternal::eclipse_lunar;
                creators["bash on enemy healer"] = &TriggerFactoryInternal::bash_on_enemy_healer;
                creators["nature's swiftness"] = &TriggerFactoryInternal::natures_swiftness;
                creators["ferocious bite"] = &TriggerFactoryInternal::ferocious_bite;
                creators["claw"] = &TriggerFactoryInternal::claw;
                creators["rip"] = &TriggerFactoryInternal::rip;
                creators["enrage"] = &TriggerFactoryInternal::enrage;
                creators["lacerate"] = &TriggerFactoryInternal::lacerate;
                creators["unstealth"] = &TriggerFactoryInternal::unstealth;
                creators["in stealth"] = &TriggerFactoryInternal::in_stealth;
                creators["no stealth"] = &TriggerFactoryInternal::no_stealth;
                creators["stealth"] = &TriggerFactoryInternal::stealth;
                creators["powershift"] = &TriggerFactoryInternal::powershift;
                creators["rebirth"] = &TriggerFactoryInternal::rebirth;
                creators["innervate"] = &TriggerFactoryInternal::innervate;
                creators["lifebloom"] = &TriggerFactoryInternal::lifebloom;
                creators["clearcasting"] = &TriggerFactoryInternal::clearcasting;
            }

        private:
            static Trigger* rip(PlayerbotAI* ai) { return new RipTrigger(ai, 3); }
            static Trigger* ferocious_bite(PlayerbotAI* ai) { return new FerociousBiteTrigger(ai); }
            static Trigger* natures_swiftness(PlayerbotAI* ai) { return new NaturesSwiftnessTrigger(ai); }
            static Trigger* eclipse_solar(PlayerbotAI* ai) { return new EclipseSolarTrigger(ai); }
            static Trigger* eclipse_lunar(PlayerbotAI* ai) { return new EclipseLunarTrigger(ai); }
            static Trigger* thorns(PlayerbotAI* ai) { return new ThornsTrigger(ai); }
            static Trigger* thorns_on_party(PlayerbotAI* ai) { return new ThornsOnPartyTrigger(ai); }
            static Trigger* bash(PlayerbotAI* ai) { return new BashInterruptSpellTrigger(ai); }
            static Trigger* faerie_fire_feral(PlayerbotAI* ai) { return new FaerieFireFeralTrigger(ai); }
            static Trigger* insect_swarm(PlayerbotAI* ai) { return new InsectSwarmTrigger(ai); }
            static Trigger* moonfire(PlayerbotAI* ai) { return new MoonfireTrigger(ai); }
            static Trigger* faerie_fire(PlayerbotAI* ai) { return new FaerieFireTrigger(ai); }
            static Trigger* natures_grasp(PlayerbotAI* ai) { return new NaturesGraspTrigger(ai); }
            static Trigger* tigers_fury(PlayerbotAI* ai) { return new TigersFuryTrigger(ai); }
            static Trigger* rake(PlayerbotAI* ai) { return new RakeTrigger(ai); }
            static Trigger* claw(PlayerbotAI* ai) { return new SpellCanBeCastedTrigger(ai, "claw"); }
            static Trigger* mark_of_the_wild(PlayerbotAI* ai) { return new MarkOfTheWildTrigger(ai); }
            static Trigger* mark_of_the_wild_on_party(PlayerbotAI* ai) { return new MarkOfTheWildOnPartyTrigger(ai); }
            static Trigger* gift_of_the_wild_on_party(PlayerbotAI* ai) { return new GiftOfTheWildOnPartyTrigger(ai); }
            static Trigger* cure_poison(PlayerbotAI* ai) { return new CurePoisonTrigger(ai); }
            static Trigger* party_member_cure_poison(PlayerbotAI* ai) { return new PartyMemberCurePoisonTrigger(ai); }
            static Trigger* remove_curse(PlayerbotAI* ai) { return new RemoveCurseTrigger(ai); }
            static Trigger* party_member_remove_curse(PlayerbotAI* ai) { return new RemoveCurseOnPartyTrigger(ai); }
            static Trigger* entangling_roots(PlayerbotAI* ai) { return new EntanglingRootsTrigger(ai); }
            static Trigger* entangling_roots_kite(PlayerbotAI* ai) { return new EntanglingRootsKiteTrigger(ai); }
            static Trigger* entangling_roots_snare(PlayerbotAI* ai) { return new EntanglingRootsSnareTrigger(ai); }
            static Trigger* hibernate(PlayerbotAI* ai) { return new HibernateTrigger(ai); }
            static Trigger* bear_form(PlayerbotAI* ai) { return new BearFormTrigger(ai); }
            static Trigger* cat_form(PlayerbotAI* ai) { return new CatFormTrigger(ai); }
            static Trigger* tree_form(PlayerbotAI* ai) { return new TreeFormTrigger(ai); }
            static Trigger* moonkin_form(PlayerbotAI* ai) { return new MoonkinFormTrigger(ai); }
            static Trigger* bash_on_enemy_healer(PlayerbotAI* ai) { return new BashInterruptEnemyHealerSpellTrigger(ai); }
            static Trigger* omen_of_clarity(PlayerbotAI* ai) { return new OmenOfClarityTrigger(ai); }
            static Trigger* enrage(PlayerbotAI* ai) { return new EnrageTrigger(ai); }
            static Trigger* lacerate(PlayerbotAI* ai) { return new LacerateTrigger(ai); }
            static Trigger* unstealth(PlayerbotAI* ai) { return new DruidUnstealthTrigger(ai); }
            static Trigger* in_stealth(PlayerbotAI* ai) { return new InStealthTrigger(ai); }
            static Trigger* no_stealth(PlayerbotAI* ai) { return new NoStealthTrigger(ai); }
            static Trigger* stealth(PlayerbotAI* ai) { return new StealthTrigger(ai); }
            static Trigger* powershift(PlayerbotAI* ai) { return new PowershiftTrigger(ai); }
            static Trigger* rebirth(PlayerbotAI* ai) { return new RebirthTrigger(ai); }
            static Trigger* innervate(PlayerbotAI* ai) { return new InnervateTrigger(ai); }
            static Trigger* lifebloom(PlayerbotAI* ai) { return new LifebloomTankTrigger(ai); }
            static Trigger* clearcasting(PlayerbotAI* ai) { return new ClearcastingTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["feral charge - bear"] = &AiObjectContextInternal::feral_charge_bear;
                creators["feral charge - cat"] = &AiObjectContextInternal::feral_charge_cat;
                creators["swipe (bear)"] = &AiObjectContextInternal::swipe_bear;
                creators["faerie fire (feral)"] = &AiObjectContextInternal::faerie_fire_feral;
                creators["faerie fire"] = &AiObjectContextInternal::faerie_fire;
                creators["bear form"] = &AiObjectContextInternal::bear_form;
                creators["dire bear form"] = &AiObjectContextInternal::dire_bear_form;
                creators["moonkin form"] = &AiObjectContextInternal::moonkin_form;
                creators["cat form"] = &AiObjectContextInternal::cat_form;
                creators["tree form"] = &AiObjectContextInternal::tree_form;
                creators["travel form"] = &AiObjectContextInternal::travel_form;
                creators["aquatic form"] = &AiObjectContextInternal::aquatic_form;
                creators["caster form"] = &AiObjectContextInternal::caster_form;
                creators["mangle (bear)"] = &AiObjectContextInternal::mangle_bear;
                creators["maul"] = &AiObjectContextInternal::maul;
                creators["bash"] = &AiObjectContextInternal::bash;
                creators["swipe"] = &AiObjectContextInternal::swipe;
                creators["growl"] = &AiObjectContextInternal::growl;
                creators["demoralizing roar"] = &AiObjectContextInternal::demoralizing_roar;
                creators["hibernate"] = &AiObjectContextInternal::hibernate;
                creators["entangling roots"] = &AiObjectContextInternal::entangling_roots;
                creators["entangling roots on cc"] = &AiObjectContextInternal::entangling_roots_on_cc;
                creators["entangling roots on snare"] = &AiObjectContextInternal::entangling_roots_on_snare;
                creators["hibernate"] = &AiObjectContextInternal::hibernate;
                creators["hibernate on cc"] = &AiObjectContextInternal::hibernate_on_cc;
                creators["wrath"] = &AiObjectContextInternal::wrath;
                creators["starfall"] = &AiObjectContextInternal::starfall;
                creators["insect swarm"] = &AiObjectContextInternal::insect_swarm;
                creators["moonfire"] = &AiObjectContextInternal::moonfire;
                creators["starfire"] = &AiObjectContextInternal::starfire;
                creators["nature's grasp"] = &AiObjectContextInternal::natures_grasp;
                creators["claw"] = &AiObjectContextInternal::claw;
                creators["mangle (cat)"] = &AiObjectContextInternal::mangle_cat;
                creators["swipe (cat)"] = &AiObjectContextInternal::swipe_cat;
                creators["rake"] = &AiObjectContextInternal::rake;
                creators["ferocious bite"] = &AiObjectContextInternal::ferocious_bite;
                creators["rip"] = &AiObjectContextInternal::rip;
                creators["cower"] = &AiObjectContextInternal::cower;
                creators["survival instincts"] = &AiObjectContextInternal::survival_instincts;
                creators["thorns"] = &AiObjectContextInternal::thorns;
                creators["thorns on party"] = &AiObjectContextInternal::thorns_on_party;
                creators["cure poison"] = &AiObjectContextInternal::cure_poison;
                creators["cure poison on party"] = &AiObjectContextInternal::cure_poison_on_party;
                creators["abolish poison"] = &AiObjectContextInternal::abolish_poison;
                creators["abolish poison on party"] = &AiObjectContextInternal::abolish_poison_on_party;
                creators["remove curse"] = &AiObjectContextInternal::remove_curse;
                creators["remove curse on party"] = &AiObjectContextInternal::remove_curse_on_party;
                creators["berserk"] = &AiObjectContextInternal::berserk;
                creators["tiger's fury"] = &AiObjectContextInternal::tigers_fury;
                creators["mark of the wild"] = &AiObjectContextInternal::mark_of_the_wild;
                creators["mark of the wild on party"] = &AiObjectContextInternal::mark_of_the_wild_on_party;
                creators["gift of the wild on party"] = &AiObjectContextInternal::gift_of_the_wild_on_party;
                creators["regrowth"] = &AiObjectContextInternal::regrowth;
                creators["rejuvenation"] = &AiObjectContextInternal::rejuvenation;
                creators["healing touch"] = &AiObjectContextInternal::healing_touch;
                creators["regrowth on party"] = &AiObjectContextInternal::regrowth_on_party;
                creators["rejuvenation on party"] = &AiObjectContextInternal::rejuvenation_on_party;
                creators["healing touch on party"] = &AiObjectContextInternal::healing_touch_on_party;
                creators["rebirth"] = &AiObjectContextInternal::rebirth;
                creators["revive"] = &AiObjectContextInternal::revive;
                creators["barskin"] = &AiObjectContextInternal::barskin;
                creators["lacerate"] = &AiObjectContextInternal::lacerate;
                creators["hurricane"] = &AiObjectContextInternal::hurricane;
                creators["innervate"] = &AiObjectContextInternal::innervate;
                creators["tranquility"] = &AiObjectContextInternal::tranquility;
                creators["bash on enemy healer"] = &AiObjectContextInternal::bash_on_enemy_healer;
                creators["omen of clarity"] = &AiObjectContextInternal::omen_of_clarity;
                creators["nature's swiftness"] = &AiObjectContextInternal::natures_swiftness;
                creators["dash"] = &AiObjectContextInternal::dash;
                creators["shred"] = &AiObjectContextInternal::shred;
                creators["ravage"] = &AiObjectContextInternal::ravage;
                creators["pounce"] = &AiObjectContextInternal::pounce;
                creators["frenzied regeneration"] = &AiObjectContextInternal::frenzied_regeneration;
                creators["challenging roar"] = &AiObjectContextInternal::challenging_roar;
                creators["enrage"] = &AiObjectContextInternal::enrage;
                creators["stealth"] = &AiObjectContextInternal::stealth;
                creators["unstealth"] = &AiObjectContextInternal::unstealth;
                creators["check stealth"] = &AiObjectContextInternal::check_stealth;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
                creators["lifebloom"] = &AiObjectContextInternal::lifebloom;
            }

        private:
            static Action* ravage(PlayerbotAI* ai) { return new CastRavageAction(ai); }
            static Action* pounce(PlayerbotAI* ai) { return new CastPounceAction(ai); }
            static Action* dash(PlayerbotAI* ai) { return new CastDashAction(ai); }
            static Action* shred(PlayerbotAI* ai) { return new CastShredAction(ai); }
            static Action* natures_swiftness(PlayerbotAI* ai) { return new CastNaturesSwiftnessAction(ai); }
            static Action* omen_of_clarity(PlayerbotAI* ai) { return new CastOmenOfClarityAction(ai); }
            static Action* tranquility(PlayerbotAI* ai) { return new CastTranquilityAction(ai); }
            static Action* feral_charge_bear(PlayerbotAI* ai) { return new CastFeralChargeBearAction(ai); }
            static Action* feral_charge_cat(PlayerbotAI* ai) { return new CastFeralChargeCatAction(ai); }
            static Action* swipe_bear(PlayerbotAI* ai) { return new CastSwipeBearAction(ai); }
            static Action* faerie_fire_feral(PlayerbotAI* ai) { return new CastFaerieFireFeralAction(ai); }
            static Action* faerie_fire(PlayerbotAI* ai) { return new CastFaerieFireAction(ai); }
            static Action* bear_form(PlayerbotAI* ai) { return new CastBearFormAction(ai); }
            static Action* dire_bear_form(PlayerbotAI* ai) { return new CastDireBearFormAction(ai); }
            static Action* cat_form(PlayerbotAI* ai) { return new CastCatFormAction(ai); }
            static Action* tree_form(PlayerbotAI* ai) { return new CastTreeFormAction(ai); }
            static Action* travel_form(PlayerbotAI* ai) { return new CastTravelFormAction(ai); }
            static Action* aquatic_form(PlayerbotAI* ai) { return new CastAquaticFormAction(ai); }
            static Action* caster_form(PlayerbotAI* ai) { return new CastCasterFormAction(ai); }
            static Action* mangle_bear(PlayerbotAI* ai) { return new CastMangleBearAction(ai); }
            static Action* maul(PlayerbotAI* ai) { return new CastMaulAction(ai); }
            static Action* bash(PlayerbotAI* ai) { return new CastBashAction(ai); }
            static Action* swipe(PlayerbotAI* ai) { return new CastSwipeAction(ai); }
            static Action* growl(PlayerbotAI* ai) { return new CastGrowlAction(ai); }
            static Action* demoralizing_roar(PlayerbotAI* ai) { return new CastDemoralizingRoarAction(ai); }
            static Action* moonkin_form(PlayerbotAI* ai) { return new CastMoonkinFormAction(ai); }
            static Action* hibernate(PlayerbotAI* ai) { return new CastHibernateAction(ai); }
            static Action* entangling_roots(PlayerbotAI* ai) { return new CastEntanglingRootsAction(ai); }
            static Action* entangling_roots_on_cc(PlayerbotAI* ai) { return new CastEntanglingRootsCcAction(ai); }
            static Action* entangling_roots_on_snare(PlayerbotAI* ai) { return new CastEntanglingRootsSnareAction(ai); }
            static Action* hibernate_on_cc(PlayerbotAI* ai) { return new CastHibernateCcAction(ai); }
            static Action* wrath(PlayerbotAI* ai) { return new CastWrathAction(ai); }
            static Action* starfall(PlayerbotAI* ai) { return new CastStarfallAction(ai); }
            static Action* insect_swarm(PlayerbotAI* ai) { return new CastInsectSwarmAction(ai); }
            static Action* moonfire(PlayerbotAI* ai) { return new CastMoonfireAction(ai); }
            static Action* starfire(PlayerbotAI* ai) { return new CastStarfireAction(ai); }
            static Action* natures_grasp(PlayerbotAI* ai) { return new CastNaturesGraspAction(ai); }
            static Action* claw(PlayerbotAI* ai) { return new CastClawAction(ai); }
            static Action* mangle_cat(PlayerbotAI* ai) { return new CastMangleCatAction(ai); }
            static Action* swipe_cat(PlayerbotAI* ai) { return new CastSwipeCatAction(ai); }
            static Action* rake(PlayerbotAI* ai) { return new CastRakeAction(ai); }
            static Action* ferocious_bite(PlayerbotAI* ai) { return new CastFerociousBiteAction(ai); }
            static Action* rip(PlayerbotAI* ai) { return new CastRipAction(ai); }
            static Action* cower(PlayerbotAI* ai) { return new CastCowerAction(ai); }
            static Action* survival_instincts(PlayerbotAI* ai) { return new CastSurvivalInstinctsAction(ai); }
            static Action* thorns(PlayerbotAI* ai) { return new CastThornsAction(ai); }
            static Action* thorns_on_party(PlayerbotAI* ai) { return new CastThornsOnPartyAction(ai); }
            static Action* cure_poison(PlayerbotAI* ai) { return new CastCurePoisonAction(ai); }
            static Action* cure_poison_on_party(PlayerbotAI* ai) { return new CastCurePoisonOnPartyAction(ai); }
            static Action* abolish_poison(PlayerbotAI* ai) { return new CastAbolishPoisonAction(ai); }
            static Action* abolish_poison_on_party(PlayerbotAI* ai) { return new CastAbolishPoisonOnPartyAction(ai); }
            static Action* remove_curse(PlayerbotAI* ai) { return new CastRemoveCurseAction(ai); }
            static Action* remove_curse_on_party(PlayerbotAI* ai) { return new CastRemoveCurseOnPartyAction(ai); }
            static Action* berserk(PlayerbotAI* ai) { return new CastBerserkAction(ai); }
            static Action* tigers_fury(PlayerbotAI* ai) { return new CastTigersFuryAction(ai); }
            static Action* mark_of_the_wild(PlayerbotAI* ai) { return new CastMarkOfTheWildAction(ai); }
            static Action* mark_of_the_wild_on_party(PlayerbotAI* ai) { return new CastMarkOfTheWildOnPartyAction(ai); }
            static Action* gift_of_the_wild_on_party(PlayerbotAI* ai) { return new CastGiftOfTheWildOnPartyAction(ai); }
            static Action* regrowth(PlayerbotAI* ai) { return new CastRegrowthAction(ai); }
            static Action* rejuvenation(PlayerbotAI* ai) { return new CastRejuvenationAction(ai); }
            static Action* healing_touch(PlayerbotAI* ai) { return new CastHealingTouchAction(ai); }
            static Action* regrowth_on_party(PlayerbotAI* ai) { return new CastRegrowthOnPartyAction(ai); }
            static Action* rejuvenation_on_party(PlayerbotAI* ai) { return new CastRejuvenationOnPartyAction(ai); }
            static Action* healing_touch_on_party(PlayerbotAI* ai) { return new CastHealingTouchOnPartyAction(ai); }
            static Action* rebirth(PlayerbotAI* ai) { return new CastRebirthAction(ai); }
            static Action* revive(PlayerbotAI* ai) { return new CastReviveAction(ai); }
            static Action* barskin(PlayerbotAI* ai) { return new CastBarskinAction(ai); }
            static Action* lacerate(PlayerbotAI* ai) { return new CastLacerateAction(ai); }
            static Action* hurricane(PlayerbotAI* ai) { return new CastHurricaneAction(ai); }
            static Action* innervate(PlayerbotAI* ai) { return new CastInnervateAction(ai); }
            static Action* bash_on_enemy_healer(PlayerbotAI* ai) { return new CastBashOnEnemyHealerAction(ai); }
            static Action* frenzied_regeneration(PlayerbotAI* ai) { return new CastFrenziedRegenerationAction(ai); }
            static Action* challenging_roar(PlayerbotAI* ai) { return new CastChallengingRoarAction(ai); }
            static Action* enrage(PlayerbotAI* ai) { return new CastEnrageAction(ai); }
            static Action* stealth(PlayerbotAI* ai) { return new CastProwlAction(ai); }
            static Action* check_stealth(PlayerbotAI* ai) { return new CheckStealthAction(ai); }
            static Action* unstealth(PlayerbotAI* ai) { return new DruidUnstealthAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdateDruidPveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdateDruidPvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdateDruidRaidStrategiesAction(ai); }
            static Action* lifebloom(PlayerbotAI* ai) { return new CastLifebloomAction(ai); }
        };
    };
};

DruidAiObjectContext::DruidAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::druid::StrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::StealthSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::druid::ClassSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::druid::AiObjectContextInternal());
    triggerContexts.Add(new ai::druid::TriggerFactoryInternal());
}
