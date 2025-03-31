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
            creators["racials"] = &StrategyContext::racials;
            creators["loot"] = &StrategyContext::loot;
            creators["gather"] = &StrategyContext::gather;
            creators["roll"] = &StrategyContext::roll;
            creators["delayed roll"] = &StrategyContext::delayed_roll;
            creators["emote"] = &StrategyContext::emote;
            creators["passive"] = &StrategyContext::passive;
            creators["conserve mana"] = &StrategyContext::conserve_mana;
            creators["food"] = &StrategyContext::food;
            creators["chat"] = &StrategyContext::chat;
            creators["default"] = &StrategyContext::world_packet;
            creators["ready check"] = &StrategyContext::ready_check;
            creators["dead"] = &StrategyContext::dead;
            creators["flee"] = &StrategyContext::flee;
            creators["avoid mobs"] = &StrategyContext::avoid_mobs;
            creators["duel"] = &StrategyContext::duel;
            creators["start duel"] = &StrategyContext::start_duel;
            creators["kite"] = &StrategyContext::kite;
            creators["potions"] = &StrategyContext::potions;
            creators["cast time"] = &StrategyContext::cast_time;
            creators["threat"] = &StrategyContext::threat;
            creators["tell target"] = &StrategyContext::tell_target;
            creators["pvp"] = &StrategyContext::pvp;
            creators["return"] = &StrategyContext::_return;
            creators["lfg"] = &StrategyContext::lfg;
            creators["custom"] = &StrategyContext::custom;
            creators["reveal"] = &StrategyContext::reveal;
            creators["collision"] = &StrategyContext::collision;
            creators["rpg"] = &StrategyContext::rpg;
            creators["rpg quest"] = &StrategyContext::rpg_quest;
            creators["rpg vendor"] = &StrategyContext::rpg_vendor;
            creators["rpg explore"] = &StrategyContext::rpg_explore;
            creators["rpg maintenance"] = &StrategyContext::rpg_maintenance;
            creators["rpg guild"] = &StrategyContext::rpg_guild;
            creators["rpg bg"] = &StrategyContext::rpg_bg;
            creators["rpg player"] = &StrategyContext::rpg_player;
            creators["rpg craft"] = &StrategyContext::rpg_craft;
            creators["rpg jump"] = &StrategyContext::rpg_jump;
            creators["follow jump"] = &StrategyContext::follow_jump;
            creators["chase jump"] = &StrategyContext::chase_jump;
			creators["travel"] = &StrategyContext::travel;
            creators["travel once"] = &StrategyContext::travel_once;
            creators["explore"] = &StrategyContext::explore;
            creators["map"] = &StrategyContext::map;
            creators["map full"] = &StrategyContext::map_full;
            creators["sit"] = &StrategyContext::sit;
            creators["mark rti"] = &StrategyContext::mark_rti;
            creators["ads"] = &StrategyContext::possible_ads;
            creators["close"] = &StrategyContext::close;
            creators["ranged"] = &StrategyContext::ranged;
            creators["behind"] = &StrategyContext::behind;
            creators["bg"] = &StrategyContext::bg;
            creators["battleground"] = &StrategyContext::battleground;
            creators["warsong"] = &StrategyContext::warsong;
            creators["alterac"] = &StrategyContext::alterac;
            creators["arathi"] = &StrategyContext::arathi;
            creators["eye"] = &StrategyContext::eye;
            creators["isle"] = &StrategyContext::isle;
            creators["arena"] = &StrategyContext::arena;
            creators["mount"] = &StrategyContext::mount;
            creators["attack tagged"] = &StrategyContext::attack_tagged;
            creators["debug"] = &StrategyContext::debug;
            creators["debug action"] = &StrategyContext::debug_action;
            creators["debug move"] = &StrategyContext::debug_move;
            creators["debug rpg"] = &StrategyContext::debug_rpg;
            creators["debug spell"] = &StrategyContext::debug_spell;
            creators["debug travel"] = &StrategyContext::debug_travel;
            creators["debug threat"] = &StrategyContext::debug_threat;
            creators["debug mount"] = &StrategyContext::debug_mount;
            creators["debug grind"] = &StrategyContext::debug_grind;
            creators["debug loot"] = &StrategyContext::debug_loot;
            creators["debug log"] = &StrategyContext::debug_log;
            creators["debug llm"] = [](PlayerbotAI* ai) { return new DebugLLMStrategy(ai); };
            creators["debug logname"] = &StrategyContext::debug_logname;
            creators["rtsc"] = &StrategyContext::rtsc;
            creators["rtsc jump"] = &StrategyContext::rtsc_jump;
            creators["maintenance"] = &StrategyContext::maintenance;
            creators["group"] = &StrategyContext::group;
            creators["guild"] = &StrategyContext::guild;
            creators["grind"] = &StrategyContext::grind;
            creators["avoid aoe"] = &StrategyContext::avoid_aoe;
            creators["wait for attack"] = &StrategyContext::wait_for_attack;
            creators["pull back"] = &StrategyContext::pull_back;
            creators["focus heal targets"] = &StrategyContext::focus_heal_targets;
            creators["focus rti targets"] = [](PlayerbotAI* ai) { return new FocusRtiTargetsStrategy(ai); };
            creators["heal interrupt"] = &StrategyContext::heal_interrupt;
            creators["preheal"] = &StrategyContext::preheal;
            creators["wbuff"] = &StrategyContext::world_buff;
            creators["silent"] = &StrategyContext::silent;
            creators["nowar"] = &StrategyContext::nowar;
            creators["glyph"] = [](PlayerbotAI* ai) { return new GlyphStrategy(ai); };
            creators["ai chat"] = [](PlayerbotAI* ai) { return new AIChatStrategy(ai); };

            // Dungeon Strategies
            creators["dungeon"] = &StrategyContext::dungeon;
            creators["onyxia's lair"] = &StrategyContext::onyxias_lair;
            creators["molten core"] = &StrategyContext::molten_core;
            creators["karazhan"] = &StrategyContext::karazhan;
            creators["naxxramas"] = &StrategyContext::naxxramas;

            // Dungeon Boss Strategies
            creators["onyxia"] = &StrategyContext::onyxia;
            creators["magmadar"] = &StrategyContext::magmadar;
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
            creators["follow"] = &MovementStrategyContext::follow_master;
            creators["stay"] = &MovementStrategyContext::stay;
            creators["runaway"] = &MovementStrategyContext::runaway;
            creators["flee from adds"] = &MovementStrategyContext::flee_from_adds;
            creators["guard"] = &MovementStrategyContext::guard;
            creators["free"] = &MovementStrategyContext::free;
        }

    private:
        static Strategy* guard(PlayerbotAI* ai) { return new GuardStrategy(ai); }
        static Strategy* follow_master(PlayerbotAI* ai) { return new FollowMasterStrategy(ai); }
        static Strategy* stay(PlayerbotAI* ai) { return new StayStrategy(ai); }
        static Strategy* runaway(PlayerbotAI* ai) { return new RunawayStrategy(ai); }
        static Strategy* flee_from_adds(PlayerbotAI* ai) { return new FleeFromAddsStrategy(ai); }
        static Strategy* free(PlayerbotAI* ai) { return new FreeStrategy(ai); }
    };

    class AssistStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        AssistStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["dps assist"] = &AssistStrategyContext::dps_assist;
            creators["dps aoe"] = &AssistStrategyContext::dps_aoe;
            creators["tank assist"] = &AssistStrategyContext::tank_assist;
        }

    private:
        static Strategy* dps_assist(PlayerbotAI* ai) { return new DpsAssistStrategy(ai); }
        static Strategy* dps_aoe(PlayerbotAI* ai) { return new DpsAoeStrategy(ai); }
        static Strategy* tank_assist(PlayerbotAI* ai) { return new TankAssistStrategy(ai); }
    };

    class QuestStrategyContext : public NamedObjectContext<Strategy>
    {
    public:
        QuestStrategyContext() : NamedObjectContext<Strategy>(false, true)
        {
            creators["quest"] = &QuestStrategyContext::quest;
            creators["accept all quests"] = &QuestStrategyContext::accept_all_quests;
        }

    private:
        static Strategy* quest(PlayerbotAI* ai) { return new DefaultQuestStrategy(ai); }
        static Strategy* accept_all_quests(PlayerbotAI* ai) { return new AcceptAllQuestsStrategy(ai); }
    };
};
