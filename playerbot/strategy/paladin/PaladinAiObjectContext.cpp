#include "playerbot/playerbot.h"
#include "PaladinActions.h"
#include "PaladinTriggers.h"
#include "PaladinAiObjectContext.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "HolyPaladinStrategy.h"
#include "ProtectionPaladinStrategy.h"
#include "RetributionPaladinStrategy.h"

namespace ai
{
    namespace paladin
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = &paladin::StrategyFactoryInternal::aoe;
                creators["cure"] = &paladin::StrategyFactoryInternal::cure;
                creators["buff"] = &paladin::StrategyFactoryInternal::buff;
                creators["pull"] = &paladin::StrategyFactoryInternal::pull;
                creators["cc"] = &paladin::StrategyFactoryInternal::cc;
                creators["offheal"] = &paladin::StrategyFactoryInternal::offheal;
                creators["boost"] = &paladin::StrategyFactoryInternal::boost;
            }

        private:
            static Strategy* aoe(PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); }
            static Strategy* cure(PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); }
            static Strategy* buff(PlayerbotAI* ai) { return new BuffPlaceholderStrategy(ai); }
            static Strategy* cc(PlayerbotAI* ai) { return new CcPlaceholderStrategy(ai); }
            static Strategy* offheal(PlayerbotAI* ai) { return new OffhealPlaceholderStrategy(ai); }
            static Strategy* boost(PlayerbotAI* ai) { return new BoostPlaceholderStrategy(ai); }
#ifdef MANGOSBOT_TWO
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "judgement of light", "seal of righteousness"); }
#else
            static Strategy* pull(PlayerbotAI* ai) { return new PullStrategy(ai, "judgement", "seal of righteousness"); }
#endif
        };

        class AoeSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AoeSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aoe holy pve"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_holy_pve;
                creators["aoe holy pvp"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_holy_pvp;
                creators["aoe holy raid"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_holy_raid;
                creators["aoe retribution pve"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_retribution_pve;
                creators["aoe retribution pvp"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_retribution_pvp;
                creators["aoe retribution raid"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_retribution_raid;
                creators["aoe protection pve"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_protection_pve;
                creators["aoe protection pvp"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_protection_pvp;
                creators["aoe protection raid"] = &paladin::AoeSituationStrategyFactoryInternal::aoe_protection_raid;
            }

        private:
            static Strategy* aoe_holy_pve(PlayerbotAI* ai) { return new HolyPaladinAoePveStrategy(ai); }
            static Strategy* aoe_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinAoePvpStrategy(ai); }
            static Strategy* aoe_holy_raid(PlayerbotAI* ai) { return new HolyPaladinAoeRaidStrategy(ai); }
            static Strategy* aoe_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinAoePveStrategy(ai); }
            static Strategy* aoe_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinAoePvpStrategy(ai); }
            static Strategy* aoe_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinAoeRaidStrategy(ai); }
            static Strategy* aoe_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinAoePveStrategy(ai); }
            static Strategy* aoe_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinAoePvpStrategy(ai); }
            static Strategy* aoe_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinAoeRaidStrategy(ai); }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure holy pve"] = &paladin::CureSituationStrategyFactoryInternal::cure_holy_pve;
                creators["cure holy pvp"] = &paladin::CureSituationStrategyFactoryInternal::cure_holy_pvp;
                creators["cure holy raid"] = &paladin::CureSituationStrategyFactoryInternal::cure_holy_raid;
                creators["cure retribution pve"] = &paladin::CureSituationStrategyFactoryInternal::cure_retribution_pve;
                creators["cure retribution pvp"] = &paladin::CureSituationStrategyFactoryInternal::cure_retribution_pvp;
                creators["cure retribution raid"] = &paladin::CureSituationStrategyFactoryInternal::cure_retribution_raid;
                creators["cure protection pve"] = &paladin::CureSituationStrategyFactoryInternal::cure_protection_pve;
                creators["cure protection pvp"] = &paladin::CureSituationStrategyFactoryInternal::cure_protection_pvp;
                creators["cure protection raid"] = &paladin::CureSituationStrategyFactoryInternal::cure_protection_raid;
            }

        private:
            static Strategy* cure_holy_pve(PlayerbotAI* ai) { return new HolyPaladinCurePveStrategy(ai); }
            static Strategy* cure_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinCurePvpStrategy(ai); }
            static Strategy* cure_holy_raid(PlayerbotAI* ai) { return new HolyPaladinCureRaidStrategy(ai); }
            static Strategy* cure_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinCurePveStrategy(ai); }
            static Strategy* cure_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinCurePvpStrategy(ai); }
            static Strategy* cure_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinCureRaidStrategy(ai); }
            static Strategy* cure_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinCurePveStrategy(ai); }
            static Strategy* cure_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinCurePvpStrategy(ai); }
            static Strategy* cure_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinCureRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff holy pve"] = &paladin::BuffSituationStrategyFactoryInternal::buff_holy_pve;
                creators["buff holy pvp"] = &paladin::BuffSituationStrategyFactoryInternal::buff_holy_pvp;
                creators["buff holy raid"] = &paladin::BuffSituationStrategyFactoryInternal::buff_holy_raid;
                creators["buff retribution pve"] = &paladin::BuffSituationStrategyFactoryInternal::buff_retribution_pve;
                creators["buff retribution pvp"] = &paladin::BuffSituationStrategyFactoryInternal::buff_retribution_pvp;
                creators["buff retribution raid"] = &paladin::BuffSituationStrategyFactoryInternal::buff_retribution_raid;
                creators["buff protection pve"] = &paladin::BuffSituationStrategyFactoryInternal::buff_protection_pve;
                creators["buff protection pvp"] = &paladin::BuffSituationStrategyFactoryInternal::buff_protection_pvp;
                creators["buff protection raid"] = &paladin::BuffSituationStrategyFactoryInternal::buff_protection_raid;
            }

        private:
            static Strategy* buff_holy_pve(PlayerbotAI* ai) { return new HolyPaladinBuffPveStrategy(ai); }
            static Strategy* buff_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinBuffPvpStrategy(ai); }
            static Strategy* buff_holy_raid(PlayerbotAI* ai) { return new HolyPaladinBuffRaidStrategy(ai); }
            static Strategy* buff_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinBuffPveStrategy(ai); }
            static Strategy* buff_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinBuffPvpStrategy(ai); }
            static Strategy* buff_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinBuffRaidStrategy(ai); }
            static Strategy* buff_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinBuffPveStrategy(ai); }
            static Strategy* buff_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinBuffPvpStrategy(ai); }
            static Strategy* buff_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinBuffRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost holy pve"] = &paladin::BoostSituationStrategyFactoryInternal::boost_holy_pve;
                creators["boost holy pvp"] = &paladin::BoostSituationStrategyFactoryInternal::boost_holy_pvp;
                creators["boost holy raid"] = &paladin::BoostSituationStrategyFactoryInternal::boost_holy_raid;
                creators["boost retribution pve"] = &paladin::BoostSituationStrategyFactoryInternal::boost_retribution_pve;
                creators["boost retribution pvp"] = &paladin::BoostSituationStrategyFactoryInternal::boost_retribution_pvp;
                creators["boost retribution raid"] = &paladin::BoostSituationStrategyFactoryInternal::boost_retribution_raid;
                creators["boost protection pve"] = &paladin::BoostSituationStrategyFactoryInternal::boost_protection_pve;
                creators["boost protection pvp"] = &paladin::BoostSituationStrategyFactoryInternal::boost_protection_pvp;
                creators["boost protection raid"] = &paladin::BoostSituationStrategyFactoryInternal::boost_protection_raid;
            }

        private:
            static Strategy* boost_holy_pve(PlayerbotAI* ai) { return new HolyPaladinBoostPveStrategy(ai); }
            static Strategy* boost_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinBoostPvpStrategy(ai); }
            static Strategy* boost_holy_raid(PlayerbotAI* ai) { return new HolyPaladinBoostRaidStrategy(ai); }
            static Strategy* boost_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinBoostPveStrategy(ai); }
            static Strategy* boost_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinBoostPvpStrategy(ai); }
            static Strategy* boost_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinBoostRaidStrategy(ai); }
            static Strategy* boost_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinBoostPveStrategy(ai); }
            static Strategy* boost_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinBoostPvpStrategy(ai); }
            static Strategy* boost_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinBoostRaidStrategy(ai); }
        };

        class OffhealSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            OffhealSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["offheal pve"] = &paladin::OffhealSituationStrategyFactoryInternal::offheal_pve;
                creators["offheal pvp"] = &paladin::OffhealSituationStrategyFactoryInternal::offheal_pvp;
                creators["offheal raid"] = &paladin::OffhealSituationStrategyFactoryInternal::offheal_raid;
            }

        private:
            static Strategy* offheal_pve(PlayerbotAI* ai) { return new PaladinOffhealPveStrategy(ai); }
            static Strategy* offheal_pvp(PlayerbotAI* ai) { return new PaladinOffhealPvpStrategy(ai); }
            static Strategy* offheal_raid(PlayerbotAI* ai) { return new PaladinOffhealRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc retribution pve"] = &paladin::CcSituationStrategyFactoryInternal::cc_retribution_pve;
                creators["cc retribution pvp"] = &paladin::CcSituationStrategyFactoryInternal::cc_retribution_pvp;
                creators["cc retribution raid"] = &paladin::CcSituationStrategyFactoryInternal::cc_retribution_raid;
                creators["cc protection pve"] = &paladin::CcSituationStrategyFactoryInternal::cc_protection_pve;
                creators["cc protection pvp"] = &paladin::CcSituationStrategyFactoryInternal::cc_protection_pvp;
                creators["cc protection raid"] = &paladin::CcSituationStrategyFactoryInternal::cc_protection_raid;
                creators["cc holy pve"] = &paladin::CcSituationStrategyFactoryInternal::cc_holy_pve;
                creators["cc holy pvp"] = &paladin::CcSituationStrategyFactoryInternal::cc_holy_pvp;
                creators["cc holy raid"] = &paladin::CcSituationStrategyFactoryInternal::cc_holy_raid;
            }

        private:
            static Strategy* cc_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinCcPveStrategy(ai); }
            static Strategy* cc_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinCcPvpStrategy(ai); }
            static Strategy* cc_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinCcRaidStrategy(ai); }
            static Strategy* cc_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinCcPveStrategy(ai); }
            static Strategy* cc_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinCcPvpStrategy(ai); }
            static Strategy* cc_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinCcRaidStrategy(ai); }
            static Strategy* cc_holy_pve(PlayerbotAI* ai) { return new HolyPaladinCcPveStrategy(ai); }
            static Strategy* cc_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinCcPvpStrategy(ai); }
            static Strategy* cc_holy_raid(PlayerbotAI* ai) { return new HolyPaladinCcRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["retribution"] = &paladin::ClassStrategyFactoryInternal::retribution;
                creators["protection"] = &paladin::ClassStrategyFactoryInternal::protection;
                creators["tank"] = &paladin::ClassStrategyFactoryInternal::protection;
                creators["holy"] = &paladin::ClassStrategyFactoryInternal::holy;
                creators["heal"] = &paladin::ClassStrategyFactoryInternal::holy;
            }

        private:
            static Strategy* retribution(PlayerbotAI* ai) { return new RetributionPaladinPlaceholderStrategy(ai); }
            static Strategy* protection(PlayerbotAI* ai) { return new ProtectionPaladinPlaceholderStrategy(ai); }
            static Strategy* holy(PlayerbotAI* ai) { return new HolyPaladinPlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["holy pvp"] = &paladin::ClassSituationStrategyFactoryInternal::holy_pvp;
                creators["holy pve"] = &paladin::ClassSituationStrategyFactoryInternal::holy_pve;
                creators["holy raid"] = &paladin::ClassSituationStrategyFactoryInternal::holy_raid;
                creators["retribution pvp"] = &paladin::ClassSituationStrategyFactoryInternal::retribution_pvp;
                creators["retribution pve"] = &paladin::ClassSituationStrategyFactoryInternal::retribution_pve;
                creators["retribution raid"] = &paladin::ClassSituationStrategyFactoryInternal::retribution_raid;
                creators["protection pvp"] = &paladin::ClassSituationStrategyFactoryInternal::protection_pvp;
                creators["protection pve"] = &paladin::ClassSituationStrategyFactoryInternal::protection_pve;
                creators["protection raid"] = &paladin::ClassSituationStrategyFactoryInternal::protection_raid;
            }

        private:
            static Strategy* holy_pvp(PlayerbotAI* ai) { return new HolyPaladinPvpStrategy(ai); }
            static Strategy* holy_pve(PlayerbotAI* ai) { return new HolyPaladinPveStrategy(ai); }
            static Strategy* holy_raid(PlayerbotAI* ai) { return new HolyPaladinRaidStrategy(ai); }
            static Strategy* retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinPvpStrategy(ai); }
            static Strategy* retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinPveStrategy(ai); }
            static Strategy* retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinRaidStrategy(ai); }
            static Strategy* protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinPvpStrategy(ai); }
            static Strategy* protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinPveStrategy(ai); }
            static Strategy* protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinRaidStrategy(ai); }
        };

        class AuraManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AuraManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aura"] = &paladin::AuraManualStrategyFactoryInternal::aura;
                creators["aura devotion"] = &paladin::AuraManualStrategyFactoryInternal::aura_devotion;
                creators["aura retribution"] = &paladin::AuraManualStrategyFactoryInternal::aura_retribution;
                creators["aura concentration"] = &paladin::AuraManualStrategyFactoryInternal::aura_concentration;
                creators["aura shadow"] = &paladin::AuraManualStrategyFactoryInternal::aura_shadow;
                creators["aura frost"] = &paladin::AuraManualStrategyFactoryInternal::aura_frost;
                creators["aura fire"] = &paladin::AuraManualStrategyFactoryInternal::aura_fire;
                creators["aura crusader"] = &paladin::AuraManualStrategyFactoryInternal::aura_crusader;
                creators["aura sanctity"] = &paladin::AuraManualStrategyFactoryInternal::aura_sanctity;
            }

        private:
            static Strategy* aura(PlayerbotAI* ai) { return new PaladinAuraPlaceholderStrategy(ai); }
            static Strategy* aura_devotion(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura devotion", "devotion aura", "devotion aura"); }
            static Strategy* aura_retribution(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura retribution", "retribution aura", "retribution aura"); }
            static Strategy* aura_concentration(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura concentration", "concentration aura", "concentration aura"); }
            static Strategy* aura_shadow(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura shadow", "shadow resistance aura", "shadow resistance aura"); }
            static Strategy* aura_frost(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura frost", "frost resistance aura", "frost resistance aura"); }
            static Strategy* aura_fire(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura fire", "fire resistance aura", "fire resistance aura"); }
            static Strategy* aura_crusader(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura crusader", "crusader aura", "crusader aura"); }
            static Strategy* aura_sanctity(PlayerbotAI* ai) { return new PaladinManualAuraStrategy(ai, "aura sanctity", "sanctity aura", "sanctity aura"); }
        };

        class AuraSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            AuraSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["aura retribution pve"] = &paladin::AuraSituationStrategyFactoryInternal::aura_retribution_pve;
                creators["aura retribution pvp"] = &paladin::AuraSituationStrategyFactoryInternal::aura_retribution_pvp;
                creators["aura retribution raid"] = &paladin::AuraSituationStrategyFactoryInternal::aura_retribution_raid;
                creators["aura protection pve"] = &paladin::AuraSituationStrategyFactoryInternal::aura_protection_pve;
                creators["aura protection pvp"] = &paladin::AuraSituationStrategyFactoryInternal::aura_protection_pvp;
                creators["aura protection raid"] = &paladin::AuraSituationStrategyFactoryInternal::aura_protection_raid;
                creators["aura holy pve"] = &paladin::AuraSituationStrategyFactoryInternal::aura_holy_pve;
                creators["aura holy pvp"] = &paladin::AuraSituationStrategyFactoryInternal::aura_holy_pvp;
                creators["aura holy raid"] = &paladin::AuraSituationStrategyFactoryInternal::aura_holy_raid;
            }

        private:
            static Strategy* aura_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinAuraPveStrategy(ai); }
            static Strategy* aura_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinAuraPvpStrategy(ai); }
            static Strategy* aura_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinAuraRaidStrategy(ai); }
            static Strategy* aura_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinAuraPveStrategy(ai); }
            static Strategy* aura_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinAuraPvpStrategy(ai); }
            static Strategy* aura_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinAuraRaidStrategy(ai); }
            static Strategy* aura_holy_pve(PlayerbotAI* ai) { return new HolyPaladinAuraPveStrategy(ai); }
            static Strategy* aura_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinAuraPvpStrategy(ai); }
            static Strategy* aura_holy_raid(PlayerbotAI* ai) { return new HolyPaladinAuraRaidStrategy(ai); }
        };

        class BlessingManualStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BlessingManualStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["blessing"] = &paladin::BlessingManualStrategyFactoryInternal::blessing;
                creators["blessing might"] = &paladin::BlessingManualStrategyFactoryInternal::blessing_might;
                creators["blessing wisdom"] = &paladin::BlessingManualStrategyFactoryInternal::blessing_wisdom;
                creators["blessing kings"] = &paladin::BlessingManualStrategyFactoryInternal::blessing_kings;
                creators["blessing sanctuary"] = &paladin::BlessingManualStrategyFactoryInternal::blessing_sanctuary;
                creators["blessing light"] = &paladin::BlessingManualStrategyFactoryInternal::blessing_light;
                creators["blessing salvation"] = &paladin::BlessingManualStrategyFactoryInternal::blessing_salvation;
            }

        private:
            static Strategy* blessing(PlayerbotAI* ai) { return new PaladinBlessingPlaceholderStrategy(ai); }
            static Strategy* blessing_might(PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing might", "blessing of might", "blessing of might"); }
            static Strategy* blessing_wisdom(PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing wisdom", "blessing of wisdom", "blessing of wisdom"); }
            static Strategy* blessing_kings(PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing kings", "blessing of kings", "blessing of kings"); }
            static Strategy* blessing_sanctuary(PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing sanctuary", "blessing of sanctuary", "blessing of sanctuary"); }
            static Strategy* blessing_light(PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing light", "blessing of light", "blessing of light"); }
            static Strategy* blessing_salvation(PlayerbotAI* ai) { return new PaladinManualBlessingStrategy(ai, "blessing salvation", "blessing of salvation", "blessing of salvation"); }
        };

        class BlessingSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BlessingSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["blessing retribution pve"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_retribution_pve;
                creators["blessing retribution pvp"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_retribution_pvp;
                creators["blessing retribution raid"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_retribution_raid;
                creators["blessing protection pve"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_protection_pve;
                creators["blessing protection pvp"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_protection_pvp;
                creators["blessing protection raid"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_protection_raid;
                creators["blessing holy pve"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_holy_pve;
                creators["blessing holy pvp"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_holy_pvp;
                creators["blessing holy raid"] = &paladin::BlessingSituationStrategyFactoryInternal::blessing_holy_raid;
            }

        private:
            static Strategy* blessing_retribution_pve(PlayerbotAI* ai) { return new RetributionPaladinBlessingPveStrategy(ai); }
            static Strategy* blessing_retribution_pvp(PlayerbotAI* ai) { return new RetributionPaladinBlessingPvpStrategy(ai); }
            static Strategy* blessing_retribution_raid(PlayerbotAI* ai) { return new RetributionPaladinBlessingRaidStrategy(ai); }
            static Strategy* blessing_protection_pve(PlayerbotAI* ai) { return new ProtectionPaladinBlessingPveStrategy(ai); }
            static Strategy* blessing_protection_pvp(PlayerbotAI* ai) { return new ProtectionPaladinBlessingPvpStrategy(ai); }
            static Strategy* blessing_protection_raid(PlayerbotAI* ai) { return new ProtectionPaladinBlessingRaidStrategy(ai); }
            static Strategy* blessing_holy_pve(PlayerbotAI* ai) { return new HolyPaladinBlessingPveStrategy(ai); }
            static Strategy* blessing_holy_pvp(PlayerbotAI* ai) { return new HolyPaladinBlessingPvpStrategy(ai); }
            static Strategy* blessing_holy_raid(PlayerbotAI* ai) { return new HolyPaladinBlessingRaidStrategy(ai); }
        };

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["judgement"] = &TriggerFactoryInternal::judgement;
                creators["judgement of wisdom"] = &TriggerFactoryInternal::judgement_of_wisdom;
                creators["judgement of light"] = &TriggerFactoryInternal::judgement_of_light;
                creators["seal"] = &TriggerFactoryInternal::seal;
                creators["art of war"] = &TriggerFactoryInternal::art_of_war;
                creators["blessing"] = &TriggerFactoryInternal::blessing;
                creators["greater blessing"] = &TriggerFactoryInternal::greater_blessing;
                creators["blessing of might"] = &TriggerFactoryInternal::blessing_of_might;
                creators["blessing of wisdom"] = &TriggerFactoryInternal::blessing_of_wisdom;
                creators["blessing of kings"] = &TriggerFactoryInternal::blessing_of_kings;
                creators["blessing of sanctuary"] = &TriggerFactoryInternal::blessing_of_sanctuary;
                creators["blessing of light"] = &TriggerFactoryInternal::blessing_of_light;
                creators["blessing of salvation"] = &TriggerFactoryInternal::blessing_of_salvation;
                creators["greater blessing of might"] = &TriggerFactoryInternal::greater_blessing_of_might;
                creators["greater blessing of wisdom"] = &TriggerFactoryInternal::greater_blessing_of_wisdom;
                creators["greater blessing of kings"] = &TriggerFactoryInternal::greater_blessing_of_kings;
                creators["greater blessing of sanctuary"] = &TriggerFactoryInternal::greater_blessing_of_sanctuary;
                creators["greater blessing of light"] = &TriggerFactoryInternal::greater_blessing_of_light;
                creators["greater blessing of salvation"] = &TriggerFactoryInternal::greater_blessing_of_salvation;
                creators["blessing on party"] = &TriggerFactoryInternal::blessing_on_party;
                creators["greater blessing on party"] = &TriggerFactoryInternal::greater_blessing_on_party;
                creators["blessing of might on party"] = &TriggerFactoryInternal::blessing_of_might_on_party;
                creators["blessing of wisdom on party"] = &TriggerFactoryInternal::blessing_of_wisdom_on_party;
                creators["blessing of kings on party"] = &TriggerFactoryInternal::blessing_of_kings_on_party;
                creators["blessing of sanctuary on party"] = &TriggerFactoryInternal::blessing_of_sanctuary_on_party;
                creators["blessing of light on party"] = &TriggerFactoryInternal::blessing_of_light_on_party;
                creators["blessing of salvation on party"] = &TriggerFactoryInternal::blessing_of_salvation_on_party;
                creators["greater blessing of might on party"] = &TriggerFactoryInternal::greater_blessing_of_might_on_party;
                creators["greater blessing of wisdom on party"] = &TriggerFactoryInternal::greater_blessing_of_wisdom_on_party;
                creators["greater blessing of kings on party"] = &TriggerFactoryInternal::greater_blessing_of_kings_on_party;
                creators["greater blessing of sanctuary on party"] = &TriggerFactoryInternal::greater_blessing_of_sanctuary_on_party;
                creators["greater blessing of light on party"] = &TriggerFactoryInternal::greater_blessing_of_light_on_party;
                creators["greater blessing of salvation on party"] = &TriggerFactoryInternal::greater_blessing_of_salvation_on_party;
                creators["no paladin aura"] = &TriggerFactoryInternal::no_paladin_aura;
                creators["crusader aura"] = &TriggerFactoryInternal::crusader_aura;
                creators["retribution aura"] = &TriggerFactoryInternal::retribution_aura;
                creators["devotion aura"] = &TriggerFactoryInternal::devotion_aura;
                creators["sanctity aura"] = &TriggerFactoryInternal::sanctity_aura;
                creators["concentration aura"] = &TriggerFactoryInternal::concentration_aura;
                creators["shadow resistance aura"] = &TriggerFactoryInternal::shadow_resistance_aura;
                creators["fire resistance aura"] = &TriggerFactoryInternal::fire_resistance_aura;
                creators["frost resistance aura"] = &TriggerFactoryInternal::frost_resistance_aura;
                creators["hammer of justice snare"] = &TriggerFactoryInternal::hammer_of_justice_snare;
                creators["hammer of justice interrupt"] = &TriggerFactoryInternal::hammer_of_justice_interrupt;
                creators["cleanse cure disease"] = &TriggerFactoryInternal::CleanseCureDisease;
                creators["cleanse party member cure disease"] = &TriggerFactoryInternal::CleanseCurePartyMemberDisease;
                creators["cleanse cure poison"] = &TriggerFactoryInternal::CleanseCurePoison;
                creators["cleanse party member cure poison"] = &TriggerFactoryInternal::CleanseCurePartyMemberPoison;
                creators["cleanse cure magic"] = &TriggerFactoryInternal::CleanseCureMagic;
                creators["cleanse party member cure magic"] = &TriggerFactoryInternal::CleanseCurePartyMemberMagic;
                creators["righteous fury"] = &TriggerFactoryInternal::righteous_fury;
                creators["holy shield"] = &TriggerFactoryInternal::holy_shield;
                creators["hammer of justice on enemy healer"] = &TriggerFactoryInternal::hammer_of_justice_on_enemy_target;
                creators["hammer of justice on snare target"] = &TriggerFactoryInternal::hammer_of_justice_on_snare_target;
                creators["divine favor"] = &TriggerFactoryInternal::divine_favor;
                creators["turn undead"] = &TriggerFactoryInternal::turn_undead;
                creators["avenger's shield"] = &TriggerFactoryInternal::avenger_shield;
                creators["consecration"] = &TriggerFactoryInternal::consecration;
                creators["exorcism"] = &TriggerFactoryInternal::exorcism;
                creators["repentance on enemy healer"] = &TriggerFactoryInternal::repentance_on_enemy_healer;
                creators["repentance on snare target"] = &TriggerFactoryInternal::repentance_on_snare_target;
                creators["repentance interrupt"] = &TriggerFactoryInternal::repentance_interrupt;
                creators["hammer of justice on enemy"] = &TriggerFactoryInternal::hammer_on_enemy;
                creators["hand of sacrifice"] = &TriggerFactoryInternal::hand_of_sacrifice;
                creators["blessing of sacrifice"] = &TriggerFactoryInternal::blessing_of_sacrifice;
                creators["crusader strike"] = &TriggerFactoryInternal::crusader_strike;
            }

        private:
            static Trigger* divine_illumination(PlayerbotAI* ai) { return new DivineIlluminationBoostTrigger(ai); }
            static Trigger* hammer_on_enemy(PlayerbotAI* ai) { return new HammerOfJusticeOnEnemyTrigger(ai); }
            static Trigger* turn_undead(PlayerbotAI* ai) { return new TurnUndeadTrigger(ai); }
            static Trigger* divine_favor(PlayerbotAI* ai) { return new DivineFavorTrigger(ai); }
            static Trigger* holy_shield(PlayerbotAI* ai) { return new HolyShieldTrigger(ai); }
            static Trigger* righteous_fury(PlayerbotAI* ai) { return new RighteousFuryTrigger(ai); }
            static Trigger* judgement(PlayerbotAI* ai) { return new JudgementTrigger(ai); }
            static Trigger* judgement_of_wisdom(PlayerbotAI* ai) { return new JudgementOfWisdomTrigger(ai); }
            static Trigger* judgement_of_light(PlayerbotAI* ai) { return new JudgementOfLightTrigger(ai); }
            static Trigger* seal(PlayerbotAI* ai) { return new SealTrigger(ai); }
            static Trigger* art_of_war(PlayerbotAI* ai) { return new ArtOfWarTrigger(ai); }
            static Trigger* blessing(PlayerbotAI* ai) { return new BlessingTrigger(ai); }
            static Trigger* greater_blessing(PlayerbotAI* ai) { return new GreaterBlessingTrigger(ai); }
            static Trigger* blessing_of_might(PlayerbotAI* ai) { return new BlessingOfMightTrigger(ai); }
            static Trigger* greater_blessing_of_might(PlayerbotAI* ai) { return new GreaterBlessingOfMightTrigger(ai); }
            static Trigger* blessing_of_wisdom(PlayerbotAI* ai) { return new BlessingOfWisdomTrigger(ai); }
            static Trigger* greater_blessing_of_wisdom(PlayerbotAI* ai) { return new GreaterBlessingOfWisdomTrigger(ai); }
            static Trigger* blessing_of_kings(PlayerbotAI* ai) { return new BlessingOfKingsTrigger(ai); }
            static Trigger* greater_blessing_of_kings(PlayerbotAI* ai) { return new GreaterBlessingOfKingsTrigger(ai); }
            static Trigger* blessing_of_light(PlayerbotAI* ai) { return new BlessingOfLightTrigger(ai); }
            static Trigger* greater_blessing_of_light(PlayerbotAI* ai) { return new GreaterBlessingOfLightTrigger(ai); }
            static Trigger* blessing_of_salvation(PlayerbotAI* ai) { return new BlessingOfSalvationTrigger(ai); }
            static Trigger* greater_blessing_of_salvation(PlayerbotAI* ai) { return new GreaterBlessingOfSalvationTrigger(ai); }
            static Trigger* blessing_of_sanctuary(PlayerbotAI* ai) { return new BlessingOfSanctuaryTrigger(ai); }
            static Trigger* greater_blessing_of_sanctuary(PlayerbotAI* ai) { return new GreaterBlessingOfSanctuaryTrigger(ai); }
            static Trigger* blessing_on_party(PlayerbotAI* ai) { return new BlessingOnPartyTrigger(ai); }
            static Trigger* greater_blessing_on_party(PlayerbotAI* ai) { return new GreaterBlessingOnPartyTrigger(ai); }
            static Trigger* blessing_of_might_on_party(PlayerbotAI* ai) { return new BlessingOfMightOnPartyTrigger(ai); }
            static Trigger* greater_blessing_of_might_on_party(PlayerbotAI* ai) { return new GreaterBlessingOfMightOnPartyTrigger(ai); }
            static Trigger* blessing_of_wisdom_on_party(PlayerbotAI* ai) { return new BlessingOfWisdomOnPartyTrigger(ai); }
            static Trigger* greater_blessing_of_wisdom_on_party(PlayerbotAI* ai) { return new GreaterBlessingOfWisdomOnPartyTrigger(ai); }
            static Trigger* blessing_of_kings_on_party(PlayerbotAI* ai) { return new BlessingOfKingsOnPartyTrigger(ai); }
            static Trigger* greater_blessing_of_kings_on_party(PlayerbotAI* ai) { return new GreaterBlessingOfKingsOnPartyTrigger(ai); }
            static Trigger* blessing_of_light_on_party(PlayerbotAI* ai) { return new BlessingOfLightOnPartyTrigger(ai); }
            static Trigger* greater_blessing_of_light_on_party(PlayerbotAI* ai) { return new GreaterBlessingOfLightOnPartyTrigger(ai); }
            static Trigger* blessing_of_salvation_on_party(PlayerbotAI* ai) { return new BlessingOfSalvationOnPartyTrigger(ai); }
            static Trigger* greater_blessing_of_salvation_on_party(PlayerbotAI* ai) { return new GreaterBlessingOfSalvationOnPartyTrigger(ai); }
            static Trigger* blessing_of_sanctuary_on_party(PlayerbotAI* ai) { return new BlessingOfSanctuaryOnPartyTrigger(ai); }
            static Trigger* greater_blessing_of_sanctuary_on_party(PlayerbotAI* ai) { return new GreaterBlessingOfSanctuaryOnPartyTrigger(ai); }
            static Trigger* no_paladin_aura(PlayerbotAI* ai) { return new NoPaladinAuraTrigger(ai); }
            static Trigger* crusader_aura(PlayerbotAI* ai) { return new CrusaderAuraTrigger(ai); }
            static Trigger* retribution_aura(PlayerbotAI* ai) { return new RetributionAuraTrigger(ai); }
            static Trigger* devotion_aura(PlayerbotAI* ai) { return new DevotionAuraTrigger(ai); }
            static Trigger* sanctity_aura(PlayerbotAI* ai) { return new SanctityAuraTrigger(ai); }
            static Trigger* concentration_aura(PlayerbotAI* ai) { return new ConcentrationAuraTrigger(ai); }
            static Trigger* shadow_resistance_aura(PlayerbotAI* ai) { return new ShadowResistanceAuraTrigger(ai); }
            static Trigger* fire_resistance_aura(PlayerbotAI* ai) { return new FireResistanceAuraTrigger(ai); }
            static Trigger* frost_resistance_aura(PlayerbotAI* ai) { return new FrostResistanceAuraTrigger(ai); }
            static Trigger* hammer_of_justice_snare(PlayerbotAI* ai) { return new HammerOfJusticeSnareTrigger(ai); }
            static Trigger* hammer_of_justice_interrupt(PlayerbotAI* ai) { return new HammerOfJusticeInterruptSpellTrigger(ai); }
            static Trigger* CleanseCureDisease(PlayerbotAI* ai) { return new CleanseCureDiseaseTrigger(ai); }
            static Trigger* CleanseCurePartyMemberDisease(PlayerbotAI* ai) { return new CleanseCurePartyMemberDiseaseTrigger(ai); }
            static Trigger* CleanseCurePoison(PlayerbotAI* ai) { return new CleanseCurePoisonTrigger(ai); }
            static Trigger* CleanseCurePartyMemberPoison(PlayerbotAI* ai) { return new CleanseCurePartyMemberPoisonTrigger(ai); }
            static Trigger* CleanseCureMagic(PlayerbotAI* ai) { return new CleanseCureMagicTrigger(ai); }
            static Trigger* CleanseCurePartyMemberMagic(PlayerbotAI* ai) { return new CleanseCurePartyMemberMagicTrigger(ai); }
            static Trigger* hammer_of_justice_on_enemy_target(PlayerbotAI* ai) { return new HammerOfJusticeEnemyHealerTrigger(ai); }
            static Trigger* hammer_of_justice_on_snare_target(PlayerbotAI* ai) { return new HammerOfJusticeSnareTrigger(ai); }
            static Trigger* avenger_shield(PlayerbotAI* ai) { return new AvengerShieldTrigger(ai); }
            static Trigger* consecration(PlayerbotAI* ai) { return new ConsecrationTrigger(ai); }
            static Trigger* exorcism(PlayerbotAI* ai) { return new ExorcismTrigger(ai); }
            static Trigger* repentance_on_enemy_healer(PlayerbotAI* ai) { return new RepentanceOnHealerTrigger(ai); }
            static Trigger* repentance_on_snare_target(PlayerbotAI* ai) { return new RepentanceSnareTrigger(ai); }
            static Trigger* repentance_interrupt(PlayerbotAI* ai) { return new RepentanceInterruptTrigger(ai); }
            static Trigger* hand_of_sacrifice(PlayerbotAI* ai) { return new HandOfSacrificeTrigger(ai); }
            static Trigger* blessing_of_sacrifice(PlayerbotAI* ai) { return new BlessingOfSacrificeTrigger(ai); }
            static Trigger* crusader_strike(PlayerbotAI* ai) { return new CrusaderStrikeTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["seal of command"] = &AiObjectContextInternal::seal_of_command;
                creators["seal of vengeance"] = &AiObjectContextInternal::seal_of_vengeance;
                creators["pve blessing"] = &AiObjectContextInternal::pve_blessing;
                creators["pve greater blessing"] = &AiObjectContextInternal::pve_greater_blessing;
                creators["pvp blessing"] = &AiObjectContextInternal::pvp_blessing;
                creators["pvp greater blessing"] = &AiObjectContextInternal::pvp_greater_blessing;
                creators["raid blessing"] = &AiObjectContextInternal::raid_blessing;
                creators["raid greater blessing"] = &AiObjectContextInternal::raid_greater_blessing_on_party;
                creators["pve blessing on party"] = &AiObjectContextInternal::pve_blessing_on_party;
                creators["pve greater blessing on party"] = &AiObjectContextInternal::pve_greater_blessing_on_party;
                creators["pvp blessing on party"] = &AiObjectContextInternal::pvp_blessing_on_party;
                creators["pvp greater blessing on party"] = &AiObjectContextInternal::pvp_greater_blessing_on_party;
                creators["raid blessing on party"] = &AiObjectContextInternal::raid_blessing_on_party;
                creators["raid greater blessing on party"] = &AiObjectContextInternal::raid_greater_blessing_on_party;
                creators["blessing of might"] = &AiObjectContextInternal::blessing_of_might;
                creators["greater blessing of might"] = &AiObjectContextInternal::greater_blessing_of_might;
                creators["blessing of might on party"] = &AiObjectContextInternal::blessing_of_might_on_party;
                creators["greater blessing of might on party"] = &AiObjectContextInternal::greater_blessing_of_might_on_party;
                creators["blessing of wisdom"] = &AiObjectContextInternal::blessing_of_wisdom;
                creators["greater blessing of wisdom"] = &AiObjectContextInternal::greater_blessing_of_wisdom;
                creators["blessing of wisdom on party"] = &AiObjectContextInternal::blessing_of_wisdom_on_party;
                creators["greater blessing of wisdom on party"] = &AiObjectContextInternal::greater_blessing_of_wisdom_on_party;
                creators["blessing of kings"] = &AiObjectContextInternal::blessing_of_kings;
                creators["greater blessing of kings"] = &AiObjectContextInternal::greater_blessing_of_kings;
                creators["blessing of kings on party"] = &AiObjectContextInternal::blessing_of_kings_on_party;
                creators["greater blessing of kings on party"] = &AiObjectContextInternal::greater_blessing_of_kings_on_party;
                creators["blessing of sanctuary"] = &AiObjectContextInternal::blessing_of_sanctuary;
                creators["greater blessing of sanctuary"] = &AiObjectContextInternal::greater_blessing_of_sanctuary;
                creators["blessing of sanctuary on party"] = &AiObjectContextInternal::blessing_of_sanctuary_on_party;
                creators["greater blessing of sanctuary on party"] = &AiObjectContextInternal::greater_blessing_of_sanctuary_on_party;
                creators["blessing of light"] = &AiObjectContextInternal::blessing_of_light;
                creators["greater blessing of light"] = &AiObjectContextInternal::greater_blessing_of_light;
                creators["blessing of light on party"] = &AiObjectContextInternal::blessing_of_light_on_party;
                creators["greater blessing of light on party"] = &AiObjectContextInternal::greater_blessing_of_light_on_party;
                creators["blessing of salvation"] = &AiObjectContextInternal::blessing_of_salvation;
                creators["greater blessing of salvation"] = &AiObjectContextInternal::greater_blessing_of_salvation;
                creators["blessing of salvation on party"] = &AiObjectContextInternal::blessing_of_salvation_on_party;
                creators["greater blessing of salvation on party"] = &AiObjectContextInternal::greater_blessing_of_salvation_on_party;
                creators["paladin aura"] = &AiObjectContextInternal::paladin_aura;
                creators["devotion aura"] = &AiObjectContextInternal::devotion_aura;
                creators["concentration aura"] = &AiObjectContextInternal::concentration_aura;
                creators["crusader aura"] = &AiObjectContextInternal::crusader_aura;
                creators["divine storm"] = &AiObjectContextInternal::divine_storm;
                creators["redemption"] = &AiObjectContextInternal::redemption;
                creators["crusader strike"] = &AiObjectContextInternal::crusader_strike;
                creators["retribution aura"] = &AiObjectContextInternal::retribution_aura;
                creators["shadow resistance aura"] = &AiObjectContextInternal::shadow_resistance_aura;
                creators["fire resistance aura"] = &AiObjectContextInternal::fire_resistance_aura;
                creators["frost resistance aura"] = &AiObjectContextInternal::frost_resistance_aura;
                creators["sanctity aura"] = &AiObjectContextInternal::sanctity_aura;
                creators["seal of light"] = &AiObjectContextInternal::seal_of_light;
                creators["holy wrath"] = &AiObjectContextInternal::holy_wrath;
                creators["consecration"] = &AiObjectContextInternal::consecration;
                creators["cleanse disease"] = &AiObjectContextInternal::cleanse_disease;
                creators["cleanse poison"] = &AiObjectContextInternal::cleanse_poison;
                creators["cleanse magic"] = &AiObjectContextInternal::cleanse_magic;
                creators["purify disease"] = &AiObjectContextInternal::purify_disease;
                creators["purify poison"] = &AiObjectContextInternal::purify_poison;
                creators["cleanse poison on party"] = &AiObjectContextInternal::cleanse_poison_on_party;
                creators["cleanse disease on party"] = &AiObjectContextInternal::cleanse_disease_on_party;
                creators["cleanse magic on party"] = &AiObjectContextInternal::cleanse_magic_on_party;
                creators["purify poison on party"] = &AiObjectContextInternal::purify_poison_on_party;
                creators["purify disease on party"] = &AiObjectContextInternal::purify_disease_on_party;
                creators["seal of wisdom"] = &AiObjectContextInternal::seal_of_wisdom;
                creators["seal of justice"] = &AiObjectContextInternal::seal_of_justice;
                creators["seal of righteousness"] = &AiObjectContextInternal::seal_of_righteousness;
                creators["flash of light"] = &AiObjectContextInternal::flash_of_light;
                creators["hand of reckoning"] = &AiObjectContextInternal::hand_of_reckoning;
                creators["avenger's shield"] = &AiObjectContextInternal::avengers_shield;
                creators["exorcism"] = &AiObjectContextInternal::exorcism;
                creators["judgement"] = &AiObjectContextInternal::judgement;
                creators["judgement of light"] = &AiObjectContextInternal::judgement_of_light;
                creators["judgement of wisdom"] = &AiObjectContextInternal::judgement_of_wisdom;
                creators["divine shield"] = &AiObjectContextInternal::divine_shield;
                creators["divine protection"] = &AiObjectContextInternal::divine_protection;
                creators["divine protection on party"] =&AiObjectContextInternal::divine_protection_on_party;
                creators["hammer of justice"] = &AiObjectContextInternal::hammer_of_justice;
                creators["flash of light on party"] = &AiObjectContextInternal::flash_of_light_on_party;
                creators["holy light"] = &AiObjectContextInternal::holy_light;
                creators["holy light on party"] = &AiObjectContextInternal::holy_light_on_party;
                creators["lay on hands"] = &AiObjectContextInternal::lay_on_hands;
                creators["lay on hands on party"] = &AiObjectContextInternal::lay_on_hands_on_party;
                creators["judgement of justice"] = &AiObjectContextInternal::judgement_of_justice;
                creators["hammer of wrath"] = &AiObjectContextInternal::hammer_of_wrath;
                creators["holy shield"] = &AiObjectContextInternal::holy_shield;
                creators["hammer of the righteous"] = &AiObjectContextInternal::hammer_of_the_righteous;
                creators["righteous fury"] = &AiObjectContextInternal::righteous_fury;
                creators["hammer of justice on enemy healer"] = &AiObjectContextInternal::hammer_of_justice_on_enemy_healer;
                creators["hammer of justice on snare target"] = &AiObjectContextInternal::hammer_of_justice_on_snare_target;
                creators["divine favor"] = &AiObjectContextInternal::divine_favor;
                creators["turn undead"] = &AiObjectContextInternal::turn_undead;
                creators["blessing of protection on party"] = &AiObjectContextInternal::blessing_of_protection_on_party;
                creators["blessing of freedom on party"] = &AiObjectContextInternal::blessing_of_freedom_on_party;
                creators["righteous defense"] = &AiObjectContextInternal::righteous_defense;
                creators["repentance"] = &AiObjectContextInternal::repentance;
                creators["repentance on snare target"] = &AiObjectContextInternal::repentance_on_snare_target;
                creators["repentance on enemy healer"] = &AiObjectContextInternal::repentance_on_enemy_healer;
                creators["holy shock"] = &AiObjectContextInternal::holy_shock;
                creators["holy shock on party"] = &AiObjectContextInternal::holy_shock_on_party;
                creators["blessing of freedom"] = &AiObjectContextInternal::blessing_of_freedom;
                creators["avenging wrath"] = &AiObjectContextInternal::avenging_wrath;
                creators["divine illumination"] = &AiObjectContextInternal::divine_illumination;
                creators["hand of sacrifice"] = &AiObjectContextInternal::hand_of_sacrifice;
                creators["blessing of sacrifice"] = &AiObjectContextInternal::blessing_of_sacrifice;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
            }

        private:
            static Action* divine_illumination(PlayerbotAI* ai) { return new CastDivineIlluminationAction(ai); }
            static Action* avenging_wrath(PlayerbotAI* ai) { return new CastAvengingWrathAction(ai); }
            static Action* blessing_of_freedom(PlayerbotAI* ai) { return new CastBlessingOfFreedomAction(ai); }
            static Action* blessing_of_protection_on_party(PlayerbotAI* ai) { return new CastBlessingOfProtectionOnPartyAction(ai); }
            static Action* blessing_of_freedom_on_party(PlayerbotAI* ai) { return new CastBlessingOfFreedomOnPartyAction(ai); }
            static Action* turn_undead(PlayerbotAI* ai) { return new CastTurnUndeadAction(ai); }
            static Action* divine_favor(PlayerbotAI* ai) { return new CastDivineFavorAction(ai); }
            static Action* righteous_fury(PlayerbotAI* ai) { return new CastRighteousFuryAction(ai); }
            static Action* seal_of_command(PlayerbotAI* ai) { return new CastSealOfCommandAction(ai); }
            static Action* seal_of_vengeance(PlayerbotAI* ai) { return new CastSealOfVengeanceAction(ai); }
            static Action* pve_blessing(PlayerbotAI* ai) { return new CastPveBlessingAction(ai); }
            static Action* pve_greater_blessing(PlayerbotAI* ai) { return new CastPveGreaterBlessingAction(ai); }
            static Action* pvp_blessing(PlayerbotAI* ai) { return new CastPvpBlessingAction(ai); }
            static Action* pvp_greater_blessing(PlayerbotAI* ai) { return new CastPvpGreaterBlessingAction(ai); }
            static Action* raid_blessing(PlayerbotAI* ai) { return new CastRaidBlessingAction(ai); }
            static Action* raid_greater_blessing(PlayerbotAI* ai) { return new CastRaidGreaterBlessingAction(ai); }
            static Action* pve_blessing_on_party(PlayerbotAI* ai) { return new CastPveBlessingOnPartyAction(ai); }
            static Action* pve_greater_blessing_on_party(PlayerbotAI* ai) { return new CastPveGreaterBlessingOnPartyAction(ai); }
            static Action* pvp_blessing_on_party(PlayerbotAI* ai) { return new CastPvpBlessingOnPartyAction(ai); }
            static Action* pvp_greater_blessing_on_party(PlayerbotAI* ai) { return new CastPvpGreaterBlessingOnPartyAction(ai); }
            static Action* raid_blessing_on_party(PlayerbotAI* ai) { return new CastRaidBlessingOnPartyAction(ai); }
            static Action* raid_greater_blessing_on_party(PlayerbotAI* ai) { return new CastRaidGreaterBlessingOnPartyAction(ai); }
            static Action* blessing_of_might(PlayerbotAI* ai) { return new CastBlessingOfMightAction(ai); }
            static Action* greater_blessing_of_might(PlayerbotAI* ai) { return new CastGreaterBlessingOfMightAction(ai); }
            static Action* blessing_of_might_on_party(PlayerbotAI* ai) { return new CastBlessingOfMightOnPartyAction(ai); }
            static Action* greater_blessing_of_might_on_party(PlayerbotAI* ai) { return new CastGreaterBlessingOfMightOnPartyAction(ai); }
            static Action* blessing_of_wisdom(PlayerbotAI* ai) { return new CastBlessingOfWisdomAction(ai); }
            static Action* greater_blessing_of_wisdom(PlayerbotAI* ai) { return new CastGreaterBlessingOfWisdomAction(ai); }
            static Action* blessing_of_wisdom_on_party(PlayerbotAI* ai) { return new CastBlessingOfWisdomOnPartyAction(ai); }
            static Action* greater_blessing_of_wisdom_on_party(PlayerbotAI* ai) { return new CastGreaterBlessingOfWisdomOnPartyAction(ai); }
            static Action* blessing_of_kings(PlayerbotAI* ai) { return new CastBlessingOfKingsAction(ai); }
            static Action* greater_blessing_of_kings(PlayerbotAI* ai) { return new CastGreaterBlessingOfKingsAction(ai); }
            static Action* blessing_of_kings_on_party(PlayerbotAI* ai) { return new CastBlessingOfKingsOnPartyAction(ai); }
            static Action* greater_blessing_of_kings_on_party(PlayerbotAI* ai) { return new CastGreaterBlessingOfKingsOnPartyAction(ai); }
            static Action* blessing_of_sanctuary(PlayerbotAI* ai) { return new CastBlessingOfSanctuaryAction(ai); }
            static Action* greater_blessing_of_sanctuary(PlayerbotAI* ai) { return new CastGreaterBlessingOfSanctuaryAction(ai); }
            static Action* blessing_of_sanctuary_on_party(PlayerbotAI* ai) { return new CastBlessingOfSanctuaryOnPartyAction(ai); }
            static Action* greater_blessing_of_sanctuary_on_party(PlayerbotAI* ai) { return new CastGreaterBlessingOfSanctuaryOnPartyAction(ai); }
            static Action* blessing_of_light(PlayerbotAI* ai) { return new CastBlessingOfLightAction(ai); }
            static Action* greater_blessing_of_light(PlayerbotAI* ai) { return new CastGreaterBlessingOfLightAction(ai); }
            static Action* blessing_of_light_on_party(PlayerbotAI* ai) { return new CastBlessingOfLightOnPartyAction(ai); }
            static Action* greater_blessing_of_light_on_party(PlayerbotAI* ai) { return new CastGreaterBlessingOfLightOnPartyAction(ai); }
            static Action* blessing_of_salvation(PlayerbotAI* ai) { return new CastBlessingOfSalvationAction(ai); }
            static Action* greater_blessing_of_salvation(PlayerbotAI* ai) { return new CastGreaterBlessingOfSalvationAction(ai); }
            static Action* blessing_of_salvation_on_party(PlayerbotAI* ai) { return new CastBlessingOfSalvationOnPartyAction(ai); }
            static Action* greater_blessing_of_salvation_on_party(PlayerbotAI* ai) { return new CastGreaterBlessingOfSalvationOnPartyAction(ai); }
            static Action* divine_storm(PlayerbotAI* ai) { return new CastDivineStormAction(ai); }
            static Action* redemption(PlayerbotAI* ai) { return new CastRedemptionAction(ai); }
            static Action* crusader_strike(PlayerbotAI* ai) { return new CastCrusaderStrikeAction(ai); }
            static Action* seal_of_light(PlayerbotAI* ai) { return new CastSealOfLightAction(ai); }
            static Action* paladin_aura(PlayerbotAI* ai) { return new CastPaladinAuraAction(ai); }
            static Action* devotion_aura(PlayerbotAI* ai) { return new CastDevotionAuraAction(ai); }
            static Action* concentration_aura(PlayerbotAI* ai) { return new CastConcentrationAuraAction(ai); }
            static Action* crusader_aura(PlayerbotAI* ai) { return new CastCrusaderAuraAction(ai); }
            static Action* retribution_aura(PlayerbotAI* ai) { return new CastRetributionAuraAction(ai); }
            static Action* shadow_resistance_aura(PlayerbotAI* ai) { return new CastShadowResistanceAuraAction(ai); }
            static Action* fire_resistance_aura(PlayerbotAI* ai) { return new CastFireResistanceAuraAction(ai); }
            static Action* frost_resistance_aura(PlayerbotAI* ai) { return new CastFrostResistanceAuraAction(ai); }
            static Action* sanctity_aura(PlayerbotAI* ai) { return new CastSanctityAuraAction(ai); }
            static Action* holy_wrath(PlayerbotAI* ai) { return new CastHolyWrathAction(ai); }
            static Action* consecration(PlayerbotAI* ai) { return new CastConsecrationAction(ai); }
            static Action* cleanse_poison(PlayerbotAI* ai) { return new CastCleansePoisonAction(ai); }
            static Action* cleanse_disease(PlayerbotAI* ai) { return new CastCleanseDiseaseAction(ai); }
            static Action* cleanse_magic(PlayerbotAI* ai) { return new CastCleanseMagicAction(ai); }
            static Action* purify_poison(PlayerbotAI* ai) { return new CastPurifyPoisonAction(ai); }
            static Action* purify_disease(PlayerbotAI* ai) { return new CastPurifyDiseaseAction(ai); }
            static Action* cleanse_poison_on_party(PlayerbotAI* ai) { return new CastCleansePoisonOnPartyAction(ai); }
            static Action* cleanse_disease_on_party(PlayerbotAI* ai) { return new CastCleanseDiseaseOnPartyAction(ai); }
            static Action* cleanse_magic_on_party(PlayerbotAI* ai) { return new CastCleanseMagicOnPartyAction(ai); }
            static Action* purify_poison_on_party(PlayerbotAI* ai) { return new CastPurifyPoisonOnPartyAction(ai); }
            static Action* purify_disease_on_party(PlayerbotAI* ai) { return new CastPurifyDiseaseOnPartyAction(ai); }
            static Action* seal_of_wisdom(PlayerbotAI* ai) { return new CastSealOfWisdomAction(ai); }
            static Action* seal_of_justice(PlayerbotAI* ai) { return new CastSealOfJusticeAction(ai); }
            static Action* seal_of_righteousness(PlayerbotAI* ai) { return new CastSealOfRighteousnessAction(ai); }
            static Action* flash_of_light(PlayerbotAI* ai) { return new CastFlashOfLightAction(ai); }
            static Action* hand_of_reckoning(PlayerbotAI* ai) { return new CastHandOfReckoningAction(ai); }
            static Action* avengers_shield(PlayerbotAI* ai) { return new CastAvengersShieldAction(ai); }
            static Action* exorcism(PlayerbotAI* ai) { return new CastExorcismAction(ai); }
            static Action* judgement(PlayerbotAI* ai) { return new CastJudgementAction(ai); }
            static Action* judgement_of_light(PlayerbotAI* ai) { return new CastJudgementOfLightAction(ai); }
            static Action* judgement_of_wisdom(PlayerbotAI* ai) { return new CastJudgementOfWisdomAction(ai); }
            static Action* divine_shield(PlayerbotAI* ai) { return new CastDivineShieldAction(ai); }
            static Action* divine_protection(PlayerbotAI* ai) { return new CastDivineProtectionAction(ai); }
            static Action* divine_protection_on_party(PlayerbotAI* ai) { return new CastDivineProtectionOnPartyAction(ai); }
            static Action* hammer_of_justice(PlayerbotAI* ai) { return new CastHammerOfJusticeAction(ai); }
            static Action* flash_of_light_on_party(PlayerbotAI* ai) { return new CastFlashOfLightOnPartyAction(ai); }
            static Action* holy_light(PlayerbotAI* ai) { return new CastHolyLightAction(ai); }
            static Action* holy_light_on_party(PlayerbotAI* ai) { return new CastHolyLightOnPartyAction(ai); }
            static Action* lay_on_hands(PlayerbotAI* ai) { return new CastLayOnHandsAction(ai); }
            static Action* lay_on_hands_on_party(PlayerbotAI* ai) { return new CastLayOnHandsOnPartyAction(ai); }
            static Action* judgement_of_justice(PlayerbotAI* ai) { return new CastJudgementOfJusticeAction(ai); }
            static Action* hammer_of_wrath(PlayerbotAI* ai) { return new CastHammerOfWrathAction(ai); }
            static Action* holy_shield(PlayerbotAI* ai) { return new CastHolyShieldAction(ai); }
            static Action* hammer_of_the_righteous(PlayerbotAI* ai) { return new CastHammerOfTheRighteousAction(ai); }
            static Action* hammer_of_justice_on_enemy_healer(PlayerbotAI* ai) { return new CastHammerOfJusticeOnEnemyHealerAction(ai); }
            static Action* hammer_of_justice_on_snare_target(PlayerbotAI* ai) { return new CastHammerOfJusticeSnareAction(ai); }
            static Action* righteous_defense(PlayerbotAI* ai) { return new CastRighteousDefenseAction(ai); }
            static Action* repentance(PlayerbotAI* ai) { return new CastRepentanceAction(ai); }
            static Action* repentance_on_snare_target(PlayerbotAI* ai) { return new CastRepentanceSnareAction(ai); }
            static Action* repentance_on_enemy_healer(PlayerbotAI* ai) { return new CastRepentanceOnHealerAction(ai); }
            static Action* holy_shock(PlayerbotAI* ai) { return new CastHolyShockAction(ai); }
            static Action* holy_shock_on_party(PlayerbotAI* ai) { return new CastHolyShockOnPartyAction(ai); }
            static Action* hand_of_sacrifice(PlayerbotAI* ai) { return new CastHandOfSacrificeAction(ai); }
            static Action* blessing_of_sacrifice(PlayerbotAI* ai) { return new CastBlessingOfSacrificeAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdatePaladinPveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdatePaladinPvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdatePaladinRaidStrategiesAction(ai); }
        };
    };
};

PaladinAiObjectContext::PaladinAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::paladin::StrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::OffhealSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::CcSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::AuraManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::AuraSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BlessingManualStrategyFactoryInternal());
    strategyContexts.Add(new ai::paladin::BlessingSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::paladin::AiObjectContextInternal());
    triggerContexts.Add(new ai::paladin::TriggerFactoryInternal());
}
