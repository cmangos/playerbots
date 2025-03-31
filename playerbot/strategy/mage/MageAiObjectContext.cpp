
#include "playerbot/playerbot.h"
#include "playerbot/strategy/Strategy.h"
#include "MageActions.h"
#include "MageAiObjectContext.h"
#include "FrostMageStrategy.h"
#include "ArcaneMageStrategy.h"
#include "FireMageStrategy.h"
#include "playerbot/strategy/generic/PullStrategy.h"
#include "MageTriggers.h"
#include "playerbot/strategy/NamedObjectContext.h"

namespace ai
{
    namespace mage
    {
        class StrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            StrategyFactoryInternal()
            {
                creators["aoe"] = &mage::StrategyFactoryInternal::aoe;
                creators["cure"] = &mage::StrategyFactoryInternal::cure;
                creators["buff"] = &mage::StrategyFactoryInternal::buff;
                creators["boost"] = &mage::StrategyFactoryInternal::boost;
                creators["pull"] = &mage::StrategyFactoryInternal::pull;
                creators["cc"] = &mage::StrategyFactoryInternal::cc;
            }

        private:
            static Strategy* aoe(PlayerbotAI* ai) { return new AoePlaceholderStrategy(ai); }
            static Strategy* cure(PlayerbotAI* ai) { return new CurePlaceholderStrategy(ai); }
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
                creators["aoe frost pve"] = &mage::AoeSituationStrategyFactoryInternal::aoe_frost_pve;
                creators["aoe frost pvp"] = &mage::AoeSituationStrategyFactoryInternal::aoe_frost_pvp;
                creators["aoe frost raid"] = &mage::AoeSituationStrategyFactoryInternal::aoe_frost_raid;
                creators["aoe fire pve"] = &mage::AoeSituationStrategyFactoryInternal::aoe_fire_pve;
                creators["aoe fire pvp"] = &mage::AoeSituationStrategyFactoryInternal::aoe_fire_pvp;
                creators["aoe fire raid"] = &mage::AoeSituationStrategyFactoryInternal::aoe_fire_raid;
                creators["aoe arcane pve"] = &mage::AoeSituationStrategyFactoryInternal::aoe_arcane_pve;
                creators["aoe arcane pvp"] = &mage::AoeSituationStrategyFactoryInternal::aoe_arcane_pvp;
                creators["aoe arcane raid"] = &mage::AoeSituationStrategyFactoryInternal::aoe_arcane_raid;
            }

        private:
            static Strategy* aoe_frost_pve(PlayerbotAI* ai) { return new FrostMageAoePveStrategy(ai); }
            static Strategy* aoe_frost_pvp(PlayerbotAI* ai) { return new FrostMageAoePvpStrategy(ai); }
            static Strategy* aoe_frost_raid(PlayerbotAI* ai) { return new FrostMageAoeRaidStrategy(ai); }
            static Strategy* aoe_fire_pve(PlayerbotAI* ai) { return new FireMageAoePveStrategy(ai); }
            static Strategy* aoe_fire_pvp(PlayerbotAI* ai) { return new FireMageAoePvpStrategy(ai); }
            static Strategy* aoe_fire_raid(PlayerbotAI* ai) { return new FireMageAoeRaidStrategy(ai); }
            static Strategy* aoe_arcane_pve(PlayerbotAI* ai) { return new ArcaneMageAoePveStrategy(ai); }
            static Strategy* aoe_arcane_pvp(PlayerbotAI* ai) { return new ArcaneMageAoePvpStrategy(ai); }
            static Strategy* aoe_arcane_raid(PlayerbotAI* ai) { return new ArcaneMageAoeRaidStrategy(ai); }
        };

        class CureSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CureSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cure frost pve"] = &mage::CureSituationStrategyFactoryInternal::cure_frost_pve;
                creators["cure frost pvp"] = &mage::CureSituationStrategyFactoryInternal::cure_frost_pvp;
                creators["cure frost raid"] = &mage::CureSituationStrategyFactoryInternal::cure_frost_raid;
                creators["cure fire pve"] = &mage::CureSituationStrategyFactoryInternal::cure_fire_pve;
                creators["cure fire pvp"] = &mage::CureSituationStrategyFactoryInternal::cure_fire_pvp;
                creators["cure fire raid"] = &mage::CureSituationStrategyFactoryInternal::cure_fire_raid;
                creators["cure arcane pve"] = &mage::CureSituationStrategyFactoryInternal::cure_arcane_pve;
                creators["cure arcane pvp"] = &mage::CureSituationStrategyFactoryInternal::cure_arcane_pvp;
                creators["cure arcane raid"] = &mage::CureSituationStrategyFactoryInternal::cure_arcane_raid;
            }

        private:
            static Strategy* cure_frost_pve(PlayerbotAI* ai) { return new FrostMageCurePveStrategy(ai); }
            static Strategy* cure_frost_pvp(PlayerbotAI* ai) { return new FrostMageCurePvpStrategy(ai); }
            static Strategy* cure_frost_raid(PlayerbotAI* ai) { return new FrostMageCureRaidStrategy(ai); }
            static Strategy* cure_fire_pve(PlayerbotAI* ai) { return new FireMageCurePveStrategy(ai); }
            static Strategy* cure_fire_pvp(PlayerbotAI* ai) { return new FireMageCurePvpStrategy(ai); }
            static Strategy* cure_fire_raid(PlayerbotAI* ai) { return new FireMageCureRaidStrategy(ai); }
            static Strategy* cure_arcane_pve(PlayerbotAI* ai) { return new ArcaneMageCurePveStrategy(ai); }
            static Strategy* cure_arcane_pvp(PlayerbotAI* ai) { return new ArcaneMageCurePvpStrategy(ai); }
            static Strategy* cure_arcane_raid(PlayerbotAI* ai) { return new ArcaneMageCureRaidStrategy(ai); }
        };

        class BuffSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BuffSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["buff frost pve"] = &mage::BuffSituationStrategyFactoryInternal::buff_frost_pve;
                creators["buff frost pvp"] = &mage::BuffSituationStrategyFactoryInternal::buff_frost_pvp;
                creators["buff frost raid"] = &mage::BuffSituationStrategyFactoryInternal::buff_frost_raid;
                creators["buff fire pve"] = &mage::BuffSituationStrategyFactoryInternal::buff_fire_pve;
                creators["buff fire pvp"] = &mage::BuffSituationStrategyFactoryInternal::buff_fire_pvp;
                creators["buff fire raid"] = &mage::BuffSituationStrategyFactoryInternal::buff_fire_raid;
                creators["buff arcane pve"] = &mage::BuffSituationStrategyFactoryInternal::buff_arcane_pve;
                creators["buff arcane pvp"] = &mage::BuffSituationStrategyFactoryInternal::buff_arcane_pvp;
                creators["buff arcane raid"] = &mage::BuffSituationStrategyFactoryInternal::buff_arcane_raid;
            }

        private:
            static Strategy* buff_frost_pve(PlayerbotAI* ai) { return new FrostMageBuffPveStrategy(ai); }
            static Strategy* buff_frost_pvp(PlayerbotAI* ai) { return new FrostMageBuffPvpStrategy(ai); }
            static Strategy* buff_frost_raid(PlayerbotAI* ai) { return new FrostMageBuffRaidStrategy(ai); }
            static Strategy* buff_fire_pve(PlayerbotAI* ai) { return new FireMageBuffPveStrategy(ai); }
            static Strategy* buff_fire_pvp(PlayerbotAI* ai) { return new FireMageBuffPvpStrategy(ai); }
            static Strategy* buff_fire_raid(PlayerbotAI* ai) { return new FireMageBuffRaidStrategy(ai); }
            static Strategy* buff_arcane_pve(PlayerbotAI* ai) { return new ArcaneMageBuffPveStrategy(ai); }
            static Strategy* buff_arcane_pvp(PlayerbotAI* ai) { return new ArcaneMageBuffPvpStrategy(ai); }
            static Strategy* buff_arcane_raid(PlayerbotAI* ai) { return new ArcaneMageBuffRaidStrategy(ai); }
        };

        class BoostSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            BoostSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["boost frost pve"] = &mage::BoostSituationStrategyFactoryInternal::boost_frost_pve;
                creators["boost frost pvp"] = &mage::BoostSituationStrategyFactoryInternal::boost_frost_pvp;
                creators["boost frost raid"] = &mage::BoostSituationStrategyFactoryInternal::boost_frost_raid;
                creators["boost fire pve"] = &mage::BoostSituationStrategyFactoryInternal::boost_fire_pve;
                creators["boost fire pvp"] = &mage::BoostSituationStrategyFactoryInternal::boost_fire_pvp;
                creators["boost fire raid"] = &mage::BoostSituationStrategyFactoryInternal::boost_fire_raid;
                creators["boost arcane pve"] = &mage::BoostSituationStrategyFactoryInternal::boost_arcane_pve;
                creators["boost arcane pvp"] = &mage::BoostSituationStrategyFactoryInternal::boost_arcane_pvp;
                creators["boost arcane raid"] = &mage::BoostSituationStrategyFactoryInternal::boost_arcane_raid;
            }

        private:
            static Strategy* boost_frost_pve(PlayerbotAI* ai) { return new FrostMageBoostPveStrategy(ai); }
            static Strategy* boost_frost_pvp(PlayerbotAI* ai) { return new FrostMageBoostPvpStrategy(ai); }
            static Strategy* boost_frost_raid(PlayerbotAI* ai) { return new FrostMageBoostRaidStrategy(ai); }
            static Strategy* boost_fire_pve(PlayerbotAI* ai) { return new FireMageBoostPveStrategy(ai); }
            static Strategy* boost_fire_pvp(PlayerbotAI* ai) { return new FireMageBoostPvpStrategy(ai); }
            static Strategy* boost_fire_raid(PlayerbotAI* ai) { return new FireMageBoostRaidStrategy(ai); }
            static Strategy* boost_arcane_pve(PlayerbotAI* ai) { return new ArcaneMageBoostPveStrategy(ai); }
            static Strategy* boost_arcane_pvp(PlayerbotAI* ai) { return new ArcaneMageBoostPvpStrategy(ai); }
            static Strategy* boost_arcane_raid(PlayerbotAI* ai) { return new ArcaneMageBoostRaidStrategy(ai); }
        };

        class CcSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            CcSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["cc fire pve"] = &mage::CcSituationStrategyFactoryInternal::cc_fire_pve;
                creators["cc fire pvp"] = &mage::CcSituationStrategyFactoryInternal::cc_fire_pvp;
                creators["cc fire raid"] = &mage::CcSituationStrategyFactoryInternal::cc_fire_raid;
                creators["cc arcane pve"] = &mage::CcSituationStrategyFactoryInternal::cc_arcane_pve;
                creators["cc arcane pvp"] = &mage::CcSituationStrategyFactoryInternal::cc_arcane_pvp;
                creators["cc arcane raid"] = &mage::CcSituationStrategyFactoryInternal::cc_arcane_raid;
                creators["cc frost pve"] = &mage::CcSituationStrategyFactoryInternal::cc_frost_pve;
                creators["cc frost pvp"] = &mage::CcSituationStrategyFactoryInternal::cc_frost_pvp;
                creators["cc frost raid"] = &mage::CcSituationStrategyFactoryInternal::cc_frost_raid;
            }

        private:
            static Strategy* cc_fire_pve(PlayerbotAI* ai) { return new FireMageCcPveStrategy(ai); }
            static Strategy* cc_fire_pvp(PlayerbotAI* ai) { return new FireMageCcPvpStrategy(ai); }
            static Strategy* cc_fire_raid(PlayerbotAI* ai) { return new FireMageCcRaidStrategy(ai); }
            static Strategy* cc_arcane_pve(PlayerbotAI* ai) { return new ArcaneMageCcPveStrategy(ai); }
            static Strategy* cc_arcane_pvp(PlayerbotAI* ai) { return new ArcaneMageCcPvpStrategy(ai); }
            static Strategy* cc_arcane_raid(PlayerbotAI* ai) { return new ArcaneMageCcRaidStrategy(ai); }
            static Strategy* cc_frost_pve(PlayerbotAI* ai) { return new FrostMageCcPveStrategy(ai); }
            static Strategy* cc_frost_pvp(PlayerbotAI* ai) { return new FrostMageCcPvpStrategy(ai); }
            static Strategy* cc_frost_raid(PlayerbotAI* ai) { return new FrostMageCcRaidStrategy(ai); }
        };

        class ClassStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["fire"] = &mage::ClassStrategyFactoryInternal::fire;
                creators["arcane"] = &mage::ClassStrategyFactoryInternal::arcane;
                creators["frost"] = &mage::ClassStrategyFactoryInternal::frost;
            }

        private:
            static Strategy* fire(PlayerbotAI* ai) { return new FireMagePlaceholderStrategy(ai); }
            static Strategy* arcane(PlayerbotAI* ai) { return new ArcaneMagePlaceholderStrategy(ai); }
            static Strategy* frost(PlayerbotAI* ai) { return new FrostMagePlaceholderStrategy(ai); }
        };

        class ClassSituationStrategyFactoryInternal : public NamedObjectContext<Strategy>
        {
        public:
            ClassSituationStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
            {
                creators["frost pvp"] = &mage::ClassSituationStrategyFactoryInternal::frost_pvp;
                creators["frost pve"] = &mage::ClassSituationStrategyFactoryInternal::frost_pve;
                creators["frost raid"] = &mage::ClassSituationStrategyFactoryInternal::frost_raid;
                creators["fire pvp"] = &mage::ClassSituationStrategyFactoryInternal::fire_pvp;
                creators["fire pve"] = &mage::ClassSituationStrategyFactoryInternal::fire_pve;
                creators["fire raid"] = &mage::ClassSituationStrategyFactoryInternal::fire_raid;
                creators["arcane pvp"] = &mage::ClassSituationStrategyFactoryInternal::arcane_pvp;
                creators["arcane pve"] = &mage::ClassSituationStrategyFactoryInternal::arcane_pve;
                creators["arcane raid"] = &mage::ClassSituationStrategyFactoryInternal::arcane_raid;
            }

        private:
            static Strategy* frost_pvp(PlayerbotAI* ai) { return new FrostMagePvpStrategy(ai); }
            static Strategy* frost_pve(PlayerbotAI* ai) { return new FrostMagePveStrategy(ai); }
            static Strategy* frost_raid(PlayerbotAI* ai) { return new FrostMageRaidStrategy(ai); }
            static Strategy* fire_pvp(PlayerbotAI* ai) { return new FireMagePvpStrategy(ai); }
            static Strategy* fire_pve(PlayerbotAI* ai) { return new FireMagePveStrategy(ai); }
            static Strategy* fire_raid(PlayerbotAI* ai) { return new FireMageRaidStrategy(ai); }
            static Strategy* arcane_pvp(PlayerbotAI* ai) { return new ArcaneMagePvpStrategy(ai); }
            static Strategy* arcane_pve(PlayerbotAI* ai) { return new ArcaneMagePveStrategy(ai); }
            static Strategy* arcane_raid(PlayerbotAI* ai) { return new ArcaneMageRaidStrategy(ai); }
        };
    };
};

namespace ai
{
    namespace mage
    {
        using namespace ai;

        class TriggerFactoryInternal : public NamedObjectContext<Trigger>
        {
        public:
            TriggerFactoryInternal()
            {
                creators["fireball"] = &TriggerFactoryInternal::fireball;
                creators["pyroblast"] = &TriggerFactoryInternal::pyroblast;
                creators["combustion"] = &TriggerFactoryInternal::combustion;
                creators["icy veins"] = &TriggerFactoryInternal::icy_veins;
                creators["arcane intellect"] = &TriggerFactoryInternal::arcane_intellect;
                creators["arcane intellect on party"] = &TriggerFactoryInternal::arcane_intellect_on_party;
                creators["arcane brilliance on party"] = &TriggerFactoryInternal::arcane_brilliance_on_party;
                creators["any mage armor"] = &TriggerFactoryInternal::any_mage_armor;
                creators["mage armor"] = &TriggerFactoryInternal::mage_armor;
                creators["ice armor"] = &TriggerFactoryInternal::ice_armor;
                creators["molten armor"] = &TriggerFactoryInternal::molten_armor;
                creators["remove curse"] = &TriggerFactoryInternal::remove_curse;
                creators["remove curse on party"] = &TriggerFactoryInternal::remove_curse_on_party;
                creators["counterspell"] = &TriggerFactoryInternal::counterspell;
                creators["polymorph"] = &TriggerFactoryInternal::polymorph;
                creators["spellsteal"] = &TriggerFactoryInternal::spellsteal;
                creators["missile barrage"] = &TriggerFactoryInternal::missile_barrage;
                creators["arcane blast"] = &TriggerFactoryInternal::arcane_blast;
                creators["counterspell on enemy healer"] = &TriggerFactoryInternal::counterspell_enemy_healer;
                creators["arcane power"] = &TriggerFactoryInternal::arcane_power;
                creators["presence of mind"] = &TriggerFactoryInternal::presence_of_mind;
                creators["presence of mind aura"] = &TriggerFactoryInternal::presence_of_mind_aura;
                creators["fire ward"] = &TriggerFactoryInternal::fire_ward;
                creators["frost ward"] = &TriggerFactoryInternal::frost_ward;
                creators["blink"] = &TriggerFactoryInternal::blink;
                creators["mana shield"] = &TriggerFactoryInternal::mana_shield;
                creators["summon water frost"] = &TriggerFactoryInternal::summon_water_frost;
                creators["ice lance"] = &TriggerFactoryInternal::ice_lance;
                creators["fire spells locked"] = &TriggerFactoryInternal::fire_spells_locked;
                creators["cold snap"] = &TriggerFactoryInternal::cold_snap;
                creators["ice barrier"] = &TriggerFactoryInternal::ice_barrier;
                creators["hot streak"] = &TriggerFactoryInternal::hot_streak;
                creators["free fireball"] = &TriggerFactoryInternal::free_fireball;
                creators["finger of frost"] = &TriggerFactoryInternal::finger_of_frost;
                creators["mirror image"] = &TriggerFactoryInternal::mirror_image;                
                creators["no improved scorch"] = &TriggerFactoryInternal::no_improved_scorch;
                creators["no fire vulnerability"] = &TriggerFactoryInternal::no_fire_vulnerability;
                creators["living bomb"] = &TriggerFactoryInternal::living_bomb;
                creators["no food"] = &TriggerFactoryInternal::no_food;
                creators["no drink"] = &TriggerFactoryInternal::no_drink;
                creators["no mana gem"] = &TriggerFactoryInternal::no_mana_gem;
            }

        private:
            static Trigger* ice_lance(PlayerbotAI* ai) { return new IceLanceTrigger(ai); }
            static Trigger* summon_water_frost(PlayerbotAI* ai) { return new WaterElementalBoostTrigger(ai); }
            static Trigger* mana_shield(PlayerbotAI* ai) { return new ManaShieldTrigger(ai); }
            static Trigger* blink(PlayerbotAI* ai) { return new BlinkTrigger(ai); }
            static Trigger* frost_ward(PlayerbotAI* ai) { return new FrostWardTrigger(ai); }
            static Trigger* fire_ward(PlayerbotAI* ai) { return new FireWardTrigger(ai); }
            static Trigger* presence_of_mind(PlayerbotAI* ai) { return new PresenceOfMindTrigger(ai); }
            static Trigger* presence_of_mind_aura(PlayerbotAI* ai) { return new PresenceOfMindAuraTrigger(ai); }
            static Trigger* arcane_power(PlayerbotAI* ai) { return new ArcanePowerTrigger(ai); }
            static Trigger* fireball(PlayerbotAI* ai) { return new FireballTrigger(ai); }
            static Trigger* pyroblast(PlayerbotAI* ai) { return new PyroblastTrigger(ai); }
            static Trigger* combustion(PlayerbotAI* ai) { return new CombustionTrigger(ai); }
            static Trigger* icy_veins(PlayerbotAI* ai) { return new IcyVeinsTrigger(ai); }
            static Trigger* arcane_intellect(PlayerbotAI* ai) { return new ArcaneIntellectTrigger(ai); }
            static Trigger* arcane_intellect_on_party(PlayerbotAI* ai) { return new ArcaneIntellectOnPartyTrigger(ai); }
            static Trigger* arcane_brilliance_on_party(PlayerbotAI* ai) { return new ArcaneBrillianceOnPartyTrigger(ai); }
            static Trigger* any_mage_armor(PlayerbotAI* ai) { return new AnyMageArmorTrigger(ai); }
            static Trigger* mage_armor(PlayerbotAI* ai) { return new MageArmorTrigger(ai); }
            static Trigger* ice_armor(PlayerbotAI* ai) { return new IceArmorTrigger(ai); }
            static Trigger* molten_armor(PlayerbotAI* ai) { return new MoltenArmorTrigger(ai); }
            static Trigger* remove_curse(PlayerbotAI* ai) { return new RemoveCurseTrigger(ai); }
            static Trigger* remove_curse_on_party(PlayerbotAI* ai) { return new PartyMemberRemoveCurseTrigger(ai); }
            static Trigger* counterspell(PlayerbotAI* ai) { return new CounterspellInterruptSpellTrigger(ai); }
            static Trigger* polymorph(PlayerbotAI* ai) { return new PolymorphTrigger(ai); }
            static Trigger* spellsteal(PlayerbotAI* ai) { return new SpellstealTrigger(ai); }
            static Trigger* missile_barrage(PlayerbotAI* ai) { return new MissileBarrageTrigger(ai); }
            static Trigger* arcane_blast(PlayerbotAI* ai) { return new ArcaneBlastTrigger(ai); }
            static Trigger* counterspell_enemy_healer(PlayerbotAI* ai) { return new CounterspellEnemyHealerTrigger(ai); }
            static Trigger* fire_spells_locked(PlayerbotAI* ai) { return new FireSpellsLocked(ai); }
            static Trigger* cold_snap(PlayerbotAI* ai) { return new ColdSnapTrigger(ai); }
            static Trigger* ice_barrier(PlayerbotAI* ai) { return new IceBarrierTrigger(ai); }
            static Trigger* hot_streak(PlayerbotAI* ai) { return new HotStreakTrigger(ai); }
            static Trigger* free_fireball(PlayerbotAI* ai) { return new FireballOrFrostfireBoltFreeTrigger(ai); }
            static Trigger* finger_of_frost(PlayerbotAI* ai) { return new FingersOfFrostTrigger(ai); }
            static Trigger* mirror_image(PlayerbotAI* ai) { return new MirrorImageTrigger(ai); }          
            static Trigger* no_improved_scorch(PlayerbotAI* ai) { return new NoImprovedScorchDebuffTrigger(ai); }
            static Trigger* no_fire_vulnerability(PlayerbotAI* ai) { return new NoFireVulnerabilityTrigger(ai); }
            static Trigger* living_bomb(PlayerbotAI* ai) { return new LivingBombTrigger(ai); }
            static Trigger* no_food(PlayerbotAI* ai) { return new NoFoodTrigger(ai); }
            static Trigger* no_drink(PlayerbotAI* ai) { return new NoDrinkTrigger(ai); }
            static Trigger* no_mana_gem(PlayerbotAI* ai) { return new NoManaGemTrigger(ai); }
        };

        class AiObjectContextInternal : public NamedObjectContext<Action>
        {
        public:
            AiObjectContextInternal()
            {
                creators["arcane power"] = &AiObjectContextInternal::arcane_power;
                creators["presence of mind"] = &AiObjectContextInternal::presence_of_mind;
                creators["frostbolt"] = &AiObjectContextInternal::frostbolt;
                creators["blizzard"] = &AiObjectContextInternal::blizzard;
                creators["frost nova"] = &AiObjectContextInternal::frost_nova;
                creators["arcane intellect"] = &AiObjectContextInternal::arcane_intellect;
                creators["arcane intellect on party"] = &AiObjectContextInternal::arcane_intellect_on_party;
                creators["arcane brilliance on party"] = &AiObjectContextInternal::arcane_brilliance_on_party;
                creators["conjure water"] = &AiObjectContextInternal::conjure_water;
                creators["conjure food"] = &AiObjectContextInternal::conjure_food;
                creators["conjure mana gem"] = &AiObjectContextInternal::conjure_mana_gem;
                creators["mana gem"] = &AiObjectContextInternal::mana_gem;
                creators["molten armor"] = &AiObjectContextInternal::molten_armor;
                creators["mage armor"] = &AiObjectContextInternal::mage_armor;
                creators["ice armor"] = &AiObjectContextInternal::ice_armor;
                creators["frost armor"] = &AiObjectContextInternal::frost_armor;
                creators["fireball"] = &AiObjectContextInternal::fireball;
                creators["pyroblast"] = &AiObjectContextInternal::pyroblast;
                creators["flamestrike"] = &AiObjectContextInternal::flamestrike;
                creators["fire blast"] = &AiObjectContextInternal::fire_blast;
                creators["scorch"] = &AiObjectContextInternal::scorch;
                creators["counterspell"] = &AiObjectContextInternal::counterspell;
                creators["remove curse"] = &AiObjectContextInternal::remove_curse;
                creators["remove curse on party"] = &AiObjectContextInternal::remove_curse_on_party;
                creators["remove lesser curse"] = &AiObjectContextInternal::remove_lesser_curse;
                creators["remove lesser curse on party"] = &AiObjectContextInternal::remove_lesser_curse_on_party;
                creators["icy veins"] = &AiObjectContextInternal::icy_veins;
                creators["combustion"] = &AiObjectContextInternal::combustion;
                creators["ice block"] = &AiObjectContextInternal::ice_block;
                creators["polymorph"] = &AiObjectContextInternal::polymorph;
                creators["spellsteal"] = &AiObjectContextInternal::spellsteal;
                creators["invisibility"] = &AiObjectContextInternal::invisibility;
                creators["lesser invisibility"] = &AiObjectContextInternal::lesser_invisibility;
                creators["evocation"] = &AiObjectContextInternal::evocation;
                creators["arcane blast"] = &AiObjectContextInternal::arcane_blast;
                creators["arcane barrage"] = &AiObjectContextInternal::arcane_barrage;
                creators["arcane missiles"] = &AiObjectContextInternal::arcane_missiles;
                creators["counterspell on enemy healer"] = &AiObjectContextInternal::counterspell_on_enemy_healer;
                creators["fire ward"] = &AiObjectContextInternal::fire_ward;
                creators["frost ward"] = &AiObjectContextInternal::frost_ward;
                creators["blink"] = &AiObjectContextInternal::blink;
                creators["ice barrier"] = &AiObjectContextInternal::ice_barrier;
                creators["mana shield"] = &AiObjectContextInternal::mana_shield;
                creators["arcane explosion"] = &AiObjectContextInternal::arcane_explosion;
                creators["cone of cold"] = &AiObjectContextInternal::cone_of_cold;
                creators["summon water frost"] = &AiObjectContextInternal::summon_water_frost;
                creators["ice lance"] = &AiObjectContextInternal::ice_lance;
                creators["cold snap"] = &AiObjectContextInternal::cold_snap;
                creators["dragon's breath"] = &AiObjectContextInternal::dragons_breath;
                creators["blast wave"] = &AiObjectContextInternal::blast_wave;
                creators["living bomb"] = &AiObjectContextInternal::living_bomb;
                creators["frostfire bolt"] = &AiObjectContextInternal::frostfire_bolt;
                creators["deep freeze"] = &AiObjectContextInternal::deep_freeze;
                creators["mirror image"] = &AiObjectContextInternal::mirror_image;
                creators["update pve strats"] = &AiObjectContextInternal::update_pve_strats;
                creators["update pvp strats"] = &AiObjectContextInternal::update_pvp_strats;
                creators["update raid strats"] = &AiObjectContextInternal::update_raid_strats;
            }

        private:
            static Action* ice_lance(PlayerbotAI* ai) { return new CastIceLanceAction(ai); }
            static Action* summon_water_frost(PlayerbotAI* ai) { return new CastSummonWaterElementalAction(ai); }
            static Action* cone_of_cold(PlayerbotAI* ai) { return new CastConeOfColdAction(ai); }
            static Action* arcane_explosion(PlayerbotAI* ai) { return new CastArcaneExplosionAction(ai); }
            static Action* mana_shield(PlayerbotAI* ai) { return new CastManaShieldAction(ai); }
            static Action* ice_barrier(PlayerbotAI* ai) { return new CastIceBarrierAction(ai); }
            static Action* blink(PlayerbotAI* ai) { return new CastBlinkAction(ai); }
            static Action* frost_ward(PlayerbotAI* ai) { return new CastFrostWardAction(ai); }
            static Action* fire_ward(PlayerbotAI* ai) { return new CastFireWardAction(ai); }
            static Action* presence_of_mind(PlayerbotAI* ai) { return new CastPresenceOfMindAction(ai); }
            static Action* arcane_power(PlayerbotAI* ai) { return new CastArcanePowerAction(ai); }
            static Action* arcane_missiles(PlayerbotAI* ai) { return new CastArcaneMissilesAction(ai); }
            static Action* arcane_barrage(PlayerbotAI* ai) { return new CastArcaneBarrageAction(ai); }
            static Action* arcane_blast(PlayerbotAI* ai) { return new CastArcaneBlastAction(ai); }
            static Action* frostbolt(PlayerbotAI* ai) { return new CastFrostboltAction(ai); }
            static Action* blizzard(PlayerbotAI* ai) { return new CastBlizzardAction(ai); }
            static Action* frost_nova(PlayerbotAI* ai) { return new CastFrostNovaAction(ai); }
            static Action* arcane_intellect(PlayerbotAI* ai) { return new CastArcaneIntellectAction(ai); }
            static Action* arcane_intellect_on_party(PlayerbotAI* ai) { return new CastArcaneIntellectOnPartyAction(ai); }
            static Action* arcane_brilliance_on_party(PlayerbotAI* ai) { return new CastArcaneBrillianceOnPartyAction(ai); }
            static Action* conjure_water(PlayerbotAI* ai) { return new CastConjureWaterAction(ai); }
            static Action* conjure_food(PlayerbotAI* ai) { return new CastConjureFoodAction(ai); }
            static Action* conjure_mana_gem(PlayerbotAI* ai) { return new CastConjureManaGemAction(ai); }
            static Action* mana_gem(PlayerbotAI* ai) { return new UseManaGemAction(ai); }
            static Action* molten_armor(PlayerbotAI* ai) { return new CastMoltenArmorAction(ai); }
            static Action* mage_armor(PlayerbotAI* ai) { return new CastMageArmorAction(ai); }
            static Action* ice_armor(PlayerbotAI* ai) { return new CastIceArmorAction(ai); }
            static Action* frost_armor(PlayerbotAI* ai) { return new CastFrostArmorAction(ai); }
            static Action* fireball(PlayerbotAI* ai) { return new CastFireballAction(ai); }
            static Action* pyroblast(PlayerbotAI* ai) { return new CastPyroblastAction(ai); }
            static Action* flamestrike(PlayerbotAI* ai) { return new CastFlamestrikeAction(ai); }
            static Action* fire_blast(PlayerbotAI* ai) { return new CastFireBlastAction(ai); }
            static Action* scorch(PlayerbotAI* ai) { return new CastScorchAction(ai); }
            static Action* counterspell(PlayerbotAI* ai) { return new CastCounterspellAction(ai); }
            static Action* remove_curse(PlayerbotAI* ai) { return new CastRemoveCurseAction(ai); }
            static Action* remove_curse_on_party(PlayerbotAI* ai) { return new CastRemoveCurseOnPartyAction(ai); }
            static Action* remove_lesser_curse(PlayerbotAI* ai) { return new CastRemoveLesserCurseAction(ai); }
            static Action* remove_lesser_curse_on_party(PlayerbotAI* ai) { return new CastRemoveLesserCurseOnPartyAction(ai); }
            static Action* icy_veins(PlayerbotAI* ai) { return new CastIcyVeinsAction(ai); }
            static Action* combustion(PlayerbotAI* ai) { return new CastCombustionAction(ai); }
            static Action* ice_block(PlayerbotAI* ai) { return new CastIceBlockAction(ai); }
            static Action* polymorph(PlayerbotAI* ai) { return new CastPolymorphAction(ai); }
            static Action* spellsteal(PlayerbotAI* ai) { return new CastSpellstealAction(ai); }
            static Action* invisibility(PlayerbotAI* ai) { return new CastInvisibilityAction(ai); }
            static Action* lesser_invisibility(PlayerbotAI* ai) { return new CastLesserInvisibilityAction(ai); }
            static Action* evocation(PlayerbotAI* ai) { return new CastEvocationAction(ai); }
            static Action* counterspell_on_enemy_healer(PlayerbotAI* ai) { return new CastCounterspellOnEnemyHealerAction(ai); }
            static Action* cold_snap(PlayerbotAI* ai) { return new CastColdSnapAction(ai); }
            static Action* dragons_breath(PlayerbotAI* ai) { return new CastDragonsBreathAction(ai); }
            static Action* blast_wave(PlayerbotAI* ai) { return new CastBlastWaveAction(ai); }
            static Action* living_bomb(PlayerbotAI* ai) { return new CastLivingBombAction(ai); }
            static Action* frostfire_bolt(PlayerbotAI* ai) { return new CastFrostfireBoltAction(ai); }
            static Action* deep_freeze(PlayerbotAI* ai) { return new DeepFreezeAction(ai); }
            static Action* mirror_image(PlayerbotAI* ai) { return new MirrorImageAction(ai); }
            static Action* update_pve_strats(PlayerbotAI* ai) { return new UpdateMagePveStrategiesAction(ai); }
            static Action* update_pvp_strats(PlayerbotAI* ai) { return new UpdateMagePvpStrategiesAction(ai); }
            static Action* update_raid_strats(PlayerbotAI* ai) { return new UpdateMageRaidStrategiesAction(ai); }
        };
    };
};

MageAiObjectContext::MageAiObjectContext(PlayerbotAI* ai) : AiObjectContext(ai)
{
    strategyContexts.Add(new ai::mage::StrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::AoeSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::CureSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::ClassStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::ClassSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::BuffSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::BoostSituationStrategyFactoryInternal());
    strategyContexts.Add(new ai::mage::CcSituationStrategyFactoryInternal());
    actionContexts.Add(new ai::mage::AiObjectContextInternal());
    triggerContexts.Add(new ai::mage::TriggerFactoryInternal());
}
