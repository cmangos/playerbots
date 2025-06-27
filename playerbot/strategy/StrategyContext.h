#pragma once

#include "CustomStrategy.h"
#include "generic/NonCombatStrategy.h"
#include "generic/RacialsStrategy.h"
#include "generic/ChatCommandHandlerStrategy.h"
#include "generic/WorldPacketHandlerStrategy.h"
#include "generic/DeadStrategy.h"
#include "generic/QuestStrategies.h"
#include "generic/LootNonCombatStrategy.h"
#include "generic/DuelStrategy.h"
#include "generic/KiteStrategy.h"
#include "generic/FleeStrategy.h"
#include "generic/FollowMasterStrategy.h"
#include "generic/RunawayStrategy.h"
#include "generic/StayStrategy.h"
#include "generic/UseFoodStrategy.h"
#include "generic/ConserveManaStrategy.h"
#include "generic/EmoteStrategy.h"
#include "generic/TankAssistStrategy.h"
#include "generic/DpsAssistStrategy.h"
#include "generic/PassiveStrategy.h"
#include "generic/GrindingStrategy.h"
#include "generic/UsePotionsStrategy.h"
#include "generic/GuardStrategy.h"
#include "generic/CastTimeStrategy.h"
#include "generic/ThreatStrategy.h"
#include "generic/TellTargetStrategy.h"
#include "generic/AttackEnemyPlayersStrategy.h"
#include "generic/MarkRtiStrategy.h"
#include "generic/MeleeCombatStrategy.h"
#include "generic/PullStrategy.h"
#include "generic/RangedCombatStrategy.h"
#include "generic/ReturnStrategy.h"
#include "generic/RpgStrategy.h"
#include "generic/TravelStrategy.h"
#include "generic/RTSCStrategy.h"
#include "generic/DebugStrategy.h"
#include "generic/BattlegroundStrategy.h"
#include "generic/LfgStrategy.h"
#include "generic/MaintenanceStrategy.h"
#include "generic/GroupStrategy.h"
#include "generic/GuildStrategy.h"
#include "generic/FocusTargetStrategy.h"
#include "generic/AvoidMobsStrategy.h"
#include "generic/CircleFormationStrategy.h"

#include "generic/DungeonStrategy.h"
#include "generic/OnyxiasLairDungeonStrategies.h"
#include "generic/MoltenCoreDungeonStrategies.h"
#include "generic/KarazhanDungeonStrategies.h"
#include "generic/NaxxramasDungeonStrategies.h"

namespace ai
{
    class StrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        StrategyContext()
        {
            creators["racials"] = [](PlayerbotAI* ai) { return new RacialsStrategy(ai); };
            creators["loot"] = [](PlayerbotAI* ai) { return new LootNonCombatStrategy(ai); };
            creators["gather"] = [](PlayerbotAI* ai) { return new GatherStrategy(ai); };
            creators["roll"] = [](PlayerbotAI* ai) { return new RollStrategy(ai); };
            creators["delayed roll"] = [](PlayerbotAI* ai) { return new DelayedRollStrategy(ai); };
            creators["emote"] = [](PlayerbotAI* ai) { return new EmoteStrategy(ai); };
            creators["passive"] = [](PlayerbotAI* ai) { return new PassiveStrategy(ai); };
            creators["conserve mana"] = [](PlayerbotAI* ai) { return new ConserveManaStrategy(ai); };
            creators["food"] = [](PlayerbotAI* ai) { return new UseFoodStrategy(ai); };
            creators["chat"] = [](PlayerbotAI* ai) { return new ChatCommandHandlerStrategy(ai); };
            creators["default"] = [](PlayerbotAI* ai) { return new WorldPacketHandlerStrategy(ai); };
            creators["ready check"] = [](PlayerbotAI* ai) { return new ReadyCheckStrategy(ai); };
            creators["dead"] = [](PlayerbotAI* ai) { return new DeadStrategy(ai); };
            creators["flee"] = [](PlayerbotAI* ai) { return new FleeStrategy(ai); };
            creators["avoid mobs"] = [](PlayerbotAI* ai) { return new AvoidMobsStrategy(ai); };
            creators["duel"] = [](PlayerbotAI* ai) { return new DuelStrategy(ai); };
            creators["start duel"] = [](PlayerbotAI* ai) { return new StartDuelStrategy(ai); };
            creators["kite"] = [](PlayerbotAI* ai) { return new KiteStrategy(ai); };
            creators["potions"] = [](PlayerbotAI* ai) { return new UsePotionsStrategy(ai); };
            creators["cast time"] = [](PlayerbotAI* ai) { return new CastTimeStrategy(ai); };
            creators["threat"] = [](PlayerbotAI* ai) { return new ThreatStrategy(ai); };
            creators["tell target"] = [](PlayerbotAI* ai) { return new TellTargetStrategy(ai); };
            creators["pvp"] = [](PlayerbotAI* ai) { return new AttackEnemyPlayersStrategy(ai); };
            creators["return"] = [](PlayerbotAI* ai) { return new ReturnStrategy(ai); };
            creators["lfg"] = [](PlayerbotAI* ai) { return new LfgStrategy(ai); };
            creators["custom"] = [](PlayerbotAI* ai) { return new CustomStrategy(ai); };
            creators["reveal"] = [](PlayerbotAI* ai) { return new RevealStrategy(ai); };
            creators["collision"] = [](PlayerbotAI* ai) { return new CollisionStrategy(ai); };
            creators["rpg"] = [](PlayerbotAI* ai) { return new RpgStrategy(ai); };
            creators["rpg quest"] = [](PlayerbotAI* ai) { return new RpgQuestStrategy(ai); };
            creators["rpg vendor"] = [](PlayerbotAI* ai) { return new RpgVendorStrategy(ai); };
            creators["rpg explore"] = [](PlayerbotAI* ai) { return new RpgExploreStrategy(ai); };
            creators["rpg maintenance"] = [](PlayerbotAI* ai) { return new RpgMaintenanceStrategy(ai); };
            creators["rpg guild"] = [](PlayerbotAI* ai) { return new RpgGuildStrategy(ai); };
            creators["rpg bg"] = [](PlayerbotAI* ai) { return new RpgBgStrategy(ai); };
            creators["rpg player"] = [](PlayerbotAI* ai) { return new RpgPlayerStrategy(ai); };
            creators["rpg craft"] = [](PlayerbotAI* ai) { return new RpgCraftStrategy(ai); };
            creators["rpg jump"] = [](PlayerbotAI* ai) { return new RpgJumpStrategy(ai); };
            creators["follow jump"] = [](PlayerbotAI* ai) { return new FollowJumpStrategy(ai); };
            creators["chase jump"] = [](PlayerbotAI* ai) { return new ChaseJumpStrategy(ai); };
			creators["travel"] = [](PlayerbotAI* ai) { return new TravelStrategy(ai); };
            creators["travel once"] = [](PlayerbotAI* ai) { return new TravelOnceStrategy(ai); };
            creators["explore"] = [](PlayerbotAI* ai) { return new ExploreStrategy(ai); };
            creators["map"] = [](PlayerbotAI* ai) { return new MapStrategy(ai); };
            creators["map full"] = [](PlayerbotAI* ai) { return new MapFullStrategy(ai); };
            creators["sit"] = [](PlayerbotAI* ai) { return new SitStrategy(ai); };
            creators["mark rti"] = [](PlayerbotAI* ai) { return new MarkRtiStrategy(ai); };
            creators["ads"] = [](PlayerbotAI* ai) { return new PossibleAdsStrategy(ai); };
            creators["close"] = [](PlayerbotAI* ai) { return new MeleeCombatStrategy(ai); };
            creators["ranged"] = [](PlayerbotAI* ai) { return new RangedCombatStrategy(ai); };
            creators["behind"] = [](PlayerbotAI* ai) { return new SetBehindCombatStrategy(ai); };
            creators["bg"] = [](PlayerbotAI* ai) { return new BGStrategy(ai); };
            creators["battleground"] = [](PlayerbotAI* ai) { return new BattlegroundStrategy(ai); };
            creators["warsong"] = [](PlayerbotAI* ai) { return new WarsongStrategy(ai); };
            creators["alterac"] = [](PlayerbotAI* ai) { return new AlteracStrategy(ai); };
            creators["arathi"] = [](PlayerbotAI* ai) { return new ArathiStrategy(ai); };
            creators["eye"] = [](PlayerbotAI* ai) { return new EyeStrategy(ai); };
            creators["isle"] = [](PlayerbotAI* ai) { return new IsleStrategy(ai); };
            creators["arena"] = [](PlayerbotAI* ai) { return new ArenaStrategy(ai); };
            creators["mount"] = [](PlayerbotAI* ai) { return new MountStrategy(ai); };
            creators["attack tagged"] = [](PlayerbotAI* ai) { return new AttackTaggedStrategy(ai); };
            creators["debug"] = [](PlayerbotAI* ai) { return new DebugStrategy(ai); };
            creators["debug action"] = [](PlayerbotAI* ai) { return new DebugActionStrategy(ai); };
            creators["debug move"] = [](PlayerbotAI* ai) { return new DebugMoveStrategy(ai); };
            creators["debug rpg"] = [](PlayerbotAI* ai) { return new DebugRpgStrategy(ai); };
            creators["debug spell"] = [](PlayerbotAI* ai) { return new DebugSpellStrategy(ai); };
            creators["debug travel"] = [](PlayerbotAI* ai) { return new DebugTravelStrategy(ai); };
            creators["debug threat"] = [](PlayerbotAI* ai) { return new DebugThreatStrategy(ai); };
            creators["debug mount"] = [](PlayerbotAI* ai) { return new DebugMountStrategy(ai); };
            creators["debug grind"] = [](PlayerbotAI* ai) { return new DebugGrindStrategy(ai); };
            creators["debug loot"] = [](PlayerbotAI* ai) { return new DebugLootStrategy(ai); };
            creators["debug log"] = [](PlayerbotAI* ai) { return new DebugLogStrategy(ai); };
            creators["debug llm"] = [](PlayerbotAI* ai) { return new DebugLLMStrategy(ai); };
            creators["debug stuck"] = [](PlayerbotAI* ai) { return new DebugStuckStrategy(ai); };
            creators["debug logname"] = [](PlayerbotAI* ai) { return new DebugLogNameStrategy(ai); };
            creators["rtsc"] = [](PlayerbotAI* ai) { return new RTSCStrategy(ai); };
            creators["rtsc jump"] = [](PlayerbotAI* ai) { return new RTSCSJumptrategy(ai); };
            creators["maintenance"] = [](PlayerbotAI* ai) { return new MaintenanceStrategy(ai); };
            creators["group"] = [](PlayerbotAI* ai) { return new GroupStrategy(ai); };
            creators["guild"] = [](PlayerbotAI* ai) { return new GuildStrategy(ai); };
            creators["grind"] = [](PlayerbotAI* ai) { return new GrindingStrategy(ai); };
            creators["avoid aoe"] = [](PlayerbotAI* ai) { return new AvoidAoeStrategy(ai); };
            creators["wait for attack"] = [](PlayerbotAI* ai) { return new WaitForAttackStrategy(ai); };
            creators["pull back"] = [](PlayerbotAI* ai) { return new PullBackStrategy(ai); };
            creators["focus heal targets"] = [](PlayerbotAI* ai) { return new FocusHealTargetsStrategy(ai); };
            creators["focus rti targets"] = [](PlayerbotAI* ai) { return new FocusRtiTargetsStrategy(ai); };
            creators["heal interrupt"] = [](PlayerbotAI* ai) { return new HealInterruptStrategy(ai); };
            creators["preheal"] = [](PlayerbotAI* ai) { return new PreHealStrategy(ai); };
            creators["wbuff"] = [](PlayerbotAI* ai) { return new WorldBuffStrategy(ai); };
            creators["silent"] = [](PlayerbotAI* ai) { return new SilentStrategy(ai); };
            creators["nowar"] = [](PlayerbotAI* ai) { return new NoWarStrategy(ai); };
            creators["glyph"] = [](PlayerbotAI* ai) { return new GlyphStrategy(ai); };
            creators["ai chat"] = [](PlayerbotAI* ai) { return new AIChatStrategy(ai); };
            creators["circle formation"] = [](PlayerbotAI* ai) { return new CircleFormationStrategy(ai); };

            // Dungeon Strategies
            creators["dungeon"] = [](PlayerbotAI* ai) { return new DungeonStrategy(ai); };
            creators["onyxia's lair"] = [](PlayerbotAI* ai) { return new OnyxiasLairDungeonStrategy(ai); };
            creators["molten core"] = [](PlayerbotAI* ai) { return new MoltenCoreDungeonStrategy(ai); };
            creators["karazhan"] = [](PlayerbotAI* ai) { return new KarazhanDungeonStrategy(ai); };
            creators["naxxramas"] = [](PlayerbotAI* ai) { return new NaxxramasDungeonStrategy(ai); };

            // Dungeon Boss Strategies
            creators["onyxia"] = [](PlayerbotAI* ai) { return new OnyxiaFightStrategy(ai); };
            creators["magmadar"] = [](PlayerbotAI* ai) { return new MagmadarFightStrategy(ai); };
            creators["netherspite"] = [](PlayerbotAI* ai) { return new NetherspiteFightStrategy(ai); };
            creators["prince malchezaar"] = [](PlayerbotAI* ai) { return new PrinceMalchezaarFightStrategy(ai); };
            creators["four horseman"] = [](PlayerbotAI* ai) { return new FourHorsemanFightStrategy(ai); };
        }
            creators["onyxia"] = &StrategyContext::onyxia;
            creators["lucifron"] = [](PlayerbotAI* ai){ return new LucifronFightStrategy(ai); };
            creators["magmadar"] = [](PlayerbotAI* ai){ return new MagmadarFightStrategy(ai); };
            creators["gehennas"] = [](PlayerbotAI* ai){ return new GehennasFightStrategy(ai); };
            creators["garr"] = [](PlayerbotAI* ai){ return new GarrFightStrategy(ai); };
            creators["baron geddon"] = [](PlayerbotAI* ai){ return new BaronGeddonFightStrategy(ai); };
            creators["shazzrah"] = [](PlayerbotAI* ai){ return new ShazzrahFightStrategy(ai); };
            creators["sulfuron harbinger"] = [](PlayerbotAI* ai){ return new SulfuronHarbingerFightStrategy(ai); };
            creators["golemagg"] = [](PlayerbotAI* ai){ return new GolemaggFightStrategy(ai); };
            creators["ragnaros"] = [](PlayerbotAI* ai){ return new RagnarosFightStrategy(ai); };

            creators["netherspite"] = &StrategyContext::netherspite;
            creators["prince malchezaar"] = &StrategyContext::prince_malchezaar;
            creators["four horseman"] = &StrategyContext::fourhorseman;
        }

    private:
        static Strategy* mount(PlayerbotAI* ai) { return new MountStrategy(ai); }
        static Strategy* arena(PlayerbotAI* ai) { return new ArenaStrategy(ai); }
        static Strategy* bg(PlayerbotAI* ai) { return new BGStrategy(ai); }
        static Strategy* battleground(PlayerbotAI* ai) { return new BattlegroundStrategy(ai); }
        static Strategy* warsong(PlayerbotAI* ai) { return new WarsongStrategy(ai); }
        static Strategy* alterac(PlayerbotAI* ai) { return new AlteracStrategy(ai); }
        static Strategy* arathi(PlayerbotAI* ai) { return new ArathiStrategy(ai); }
        static Strategy* eye(PlayerbotAI* ai) { return new EyeStrategy(ai); }
        static Strategy* isle(PlayerbotAI* ai) { return new IsleStrategy(ai); }
        static Strategy* behind(PlayerbotAI* ai) { return new SetBehindCombatStrategy(ai); }
        static Strategy* ranged(PlayerbotAI* ai) { return new RangedCombatStrategy(ai); }
        static Strategy* close(PlayerbotAI* ai) { return new MeleeCombatStrategy(ai); }
        static Strategy* mark_rti(PlayerbotAI* ai) { return new MarkRtiStrategy(ai); }
        static Strategy* tell_target(PlayerbotAI* ai) { return new TellTargetStrategy(ai); }
        static Strategy* threat(PlayerbotAI* ai) { return new ThreatStrategy(ai); }
        static Strategy* cast_time(PlayerbotAI* ai) { return new CastTimeStrategy(ai); }
        static Strategy* potions(PlayerbotAI* ai) { return new UsePotionsStrategy(ai); }
        static Strategy* kite(PlayerbotAI* ai) { return new KiteStrategy(ai); }
        static Strategy* duel(PlayerbotAI* ai) { return new DuelStrategy(ai); }
        static Strategy* start_duel(PlayerbotAI* ai) { return new StartDuelStrategy(ai); }
        static Strategy* flee(PlayerbotAI* ai) { return new FleeStrategy(ai); }
        static Strategy* avoid_mobs(PlayerbotAI* ai) { return new AvoidMobsStrategy(ai); }
        static Strategy* dead(PlayerbotAI* ai) { return new DeadStrategy(ai); }
        static Strategy* racials(PlayerbotAI* ai) { return new RacialsStrategy(ai); }
        static Strategy* loot(PlayerbotAI* ai) { return new LootNonCombatStrategy(ai); }
        static Strategy* gather(PlayerbotAI* ai) { return new GatherStrategy(ai); }
        static Strategy* roll(PlayerbotAI* ai) { return new RollStrategy(ai); }
        static Strategy* delayed_roll(PlayerbotAI* ai) { return new DelayedRollStrategy(ai); }
        static Strategy* emote(PlayerbotAI* ai) { return new EmoteStrategy(ai); }
        static Strategy* passive(PlayerbotAI* ai) { return new PassiveStrategy(ai); }
        static Strategy* conserve_mana(PlayerbotAI* ai) { return new ConserveManaStrategy(ai); }
        static Strategy* food(PlayerbotAI* ai) { return new UseFoodStrategy(ai); }
        static Strategy* chat(PlayerbotAI* ai) { return new ChatCommandHandlerStrategy(ai); }
        static Strategy* world_packet(PlayerbotAI* ai) { return new WorldPacketHandlerStrategy(ai); }
        static Strategy* ready_check(PlayerbotAI* ai) { return new ReadyCheckStrategy(ai); }
        static Strategy* pvp(PlayerbotAI* ai) { return new AttackEnemyPlayersStrategy(ai); }
        static Strategy* _return(PlayerbotAI* ai) { return new ReturnStrategy(ai); }
        static Strategy* lfg(PlayerbotAI* ai) { return new LfgStrategy(ai); }
        static Strategy* custom(PlayerbotAI* ai) { return new CustomStrategy(ai); }
        static Strategy* reveal(PlayerbotAI* ai) { return new RevealStrategy(ai); }
        static Strategy* collision(PlayerbotAI* ai) { return new CollisionStrategy(ai); }
        static Strategy* rpg(PlayerbotAI* ai) { return new RpgStrategy(ai); }
        static Strategy* rpg_quest(PlayerbotAI* ai) { return new RpgQuestStrategy(ai); }
        static Strategy* rpg_vendor(PlayerbotAI* ai) { return new RpgVendorStrategy(ai); }
        static Strategy* rpg_explore(PlayerbotAI* ai) { return new RpgExploreStrategy(ai); }
        static Strategy* rpg_maintenance(PlayerbotAI* ai) { return new RpgMaintenanceStrategy(ai); }
        static Strategy* rpg_guild(PlayerbotAI* ai) { return new RpgGuildStrategy(ai); }
        static Strategy* rpg_bg(PlayerbotAI* ai) { return new RpgBgStrategy(ai); }
        static Strategy* rpg_player(PlayerbotAI* ai) { return new RpgPlayerStrategy(ai); }
        static Strategy* rpg_craft(PlayerbotAI* ai) { return new RpgCraftStrategy(ai); }
        static Strategy* rpg_jump(PlayerbotAI* ai) { return new RpgJumpStrategy(ai); }
        static Strategy* follow_jump(PlayerbotAI* ai) { return new FollowJumpStrategy(ai); }
        static Strategy* chase_jump(PlayerbotAI* ai) { return new ChaseJumpStrategy(ai); }
		static Strategy* travel(PlayerbotAI* ai) { return new TravelStrategy(ai); }
        static Strategy* travel_once(PlayerbotAI* ai) { return new TravelOnceStrategy(ai); }
        static Strategy* explore(PlayerbotAI* ai) { return new ExploreStrategy(ai); }
        static Strategy* map(PlayerbotAI* ai) { return new MapStrategy(ai); }
        static Strategy* map_full(PlayerbotAI* ai) { return new MapFullStrategy(ai); }
        static Strategy* sit(PlayerbotAI* ai) { return new SitStrategy(ai); }
        static Strategy* possible_ads(PlayerbotAI* ai) { return new PossibleAdsStrategy(ai); }
        static Strategy* attack_tagged(PlayerbotAI* ai) { return new AttackTaggedStrategy(ai); }
        static Strategy* debug(PlayerbotAI* ai) { return new DebugStrategy(ai); }
        static Strategy* debug_action(PlayerbotAI* ai) { return new DebugActionStrategy(ai); }
        static Strategy* debug_move(PlayerbotAI* ai) { return new DebugMoveStrategy(ai); }
        static Strategy* debug_rpg(PlayerbotAI* ai) { return new DebugRpgStrategy(ai); }
        static Strategy* debug_spell(PlayerbotAI* ai) { return new DebugSpellStrategy(ai); }
        static Strategy* debug_travel(PlayerbotAI* ai) { return new DebugTravelStrategy(ai); }
        static Strategy* debug_threat(PlayerbotAI* ai) { return new DebugThreatStrategy(ai); }
        static Strategy* debug_mount(PlayerbotAI* ai) { return new DebugMountStrategy(ai); }
        static Strategy* debug_grind(PlayerbotAI* ai) { return new DebugGrindStrategy(ai); }
        static Strategy* debug_loot(PlayerbotAI* ai) { return new DebugLootStrategy(ai); }
        static Strategy* debug_log(PlayerbotAI* ai) { return new DebugLogStrategy(ai); }
        static Strategy* debug_logname(PlayerbotAI* ai) { return new DebugLogNameStrategy(ai); }
        static Strategy* rtsc(PlayerbotAI* ai) { return new RTSCStrategy(ai); }
        static Strategy* rtsc_jump(PlayerbotAI* ai) { return new RTSCSJumptrategy(ai); }
        static Strategy* maintenance(PlayerbotAI* ai) { return new MaintenanceStrategy(ai); }
        static Strategy* group(PlayerbotAI* ai) { return new GroupStrategy(ai); }
        static Strategy* guild (PlayerbotAI* ai) { return new GuildStrategy(ai); }
        static Strategy* grind(PlayerbotAI* ai) { return new GrindingStrategy(ai); }
        static Strategy* avoid_aoe(PlayerbotAI* ai) { return new AvoidAoeStrategy(ai); }
        static Strategy* wait_for_attack(PlayerbotAI* ai) { return new WaitForAttackStrategy(ai); }
        static Strategy* pull_back(PlayerbotAI* ai) { return new PullBackStrategy(ai); }
        static Strategy* focus_heal_targets(PlayerbotAI* ai) { return new FocusHealTargetsStrategy(ai); }
        static Strategy* heal_interrupt(PlayerbotAI* ai) { return new HealInterruptStrategy(ai); }
        static Strategy* preheal(PlayerbotAI* ai) { return new PreHealStrategy(ai); }
        static Strategy* world_buff(PlayerbotAI* ai) { return new WorldBuffStrategy(ai); }
        static Strategy* silent(PlayerbotAI* ai) { return new SilentStrategy(ai); }
        static Strategy* nowar(PlayerbotAI* ai) { return new NoWarStrategy(ai); }

        // Dungeon Strategies
        static Strategy* dungeon(PlayerbotAI* ai) { return new DungeonStrategy(ai); }
        static Strategy* onyxias_lair(PlayerbotAI* ai) { return new OnyxiasLairDungeonStrategy(ai); }
        static Strategy* molten_core(PlayerbotAI* ai) { return new MoltenCoreDungeonStrategy(ai); }
        static Strategy* karazhan(PlayerbotAI* ai) { return new KarazhanDungeonStrategy(ai); }
        static Strategy* naxxramas(PlayerbotAI* ai) { return new NaxxramasDungeonStrategy(ai); }

        // Dungeon Boss Strategy
        static Strategy* onyxia(PlayerbotAI* ai) { return new OnyxiaFightStrategy(ai); }
        static Strategy* magmadar(PlayerbotAI* ai) { return new MagmadarFightStrategy(ai); }
        static Strategy* netherspite(PlayerbotAI* ai) { return new NetherspiteFightStrategy(ai); }
        static Strategy* prince_malchezaar(PlayerbotAI* ai) { return new PrinceMalchezaarFightStrategy(ai); }
        static Strategy* fourhorseman(PlayerbotAI* ai) { return new FourHorsemanFightStrategy(ai); }
    };

    class MovementStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        MovementStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["follow"] = [](PlayerbotAI* ai) { return new FollowMasterStrategy(ai); };
            creators["stay"] = [](PlayerbotAI* ai) { return new StayStrategy(ai); };
            creators["runaway"] = [](PlayerbotAI* ai) { return new RunawayStrategy(ai); };
            creators["flee from adds"] = [](PlayerbotAI* ai) { return new FleeFromAddsStrategy(ai); };
            creators["guard"] = [](PlayerbotAI* ai) { return new GuardStrategy(ai); };
            creators["free"] = [](PlayerbotAI* ai) { return new FreeStrategy(ai); };
        }
    };

    class AssistStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        AssistStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["dps assist"] = [](PlayerbotAI* ai) { return new DpsAssistStrategy(ai); };
            creators["dps aoe"] = [](PlayerbotAI* ai) { return new DpsAoeStrategy(ai); };
            creators["tank assist"] = [](PlayerbotAI* ai) { return new TankAssistStrategy(ai); };
        }
    };

    class QuestStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        QuestStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["quest"] = [](PlayerbotAI* ai) { return new DefaultQuestStrategy(ai); };
            creators["accept all quests"] = [](PlayerbotAI* ai) { return new AcceptAllQuestsStrategy(ai); };
        }
    };

    class FishStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        FishStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["fish"] = [](PlayerbotAI* ai) { return new FishStrategy(ai); };
            creators["tfish"] = [](PlayerbotAI* ai) { return new TFishStrategy(ai); };
        }   
    };
};
