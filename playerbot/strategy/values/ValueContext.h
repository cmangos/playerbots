#pragma once

#include "ActiveSpellValue.h"
#include "NearestGameObjects.h"
#include "LogLevelValue.h"
#include "NearestNpcsValue.h"
#include "PossibleTargetsValue.h"
#include "NearestAdsValue.h"
#include "NearestCorpsesValue.h"
#include "PartyMemberWithoutAuraValue.h"
#include "PartyMemberToHeal.h"
#include "PartyMemberToResurrect.h"
#include "CurrentTargetValue.h"
#include "SelfTargetValue.h"
#include "MasterTargetValue.h"
#include "LineTargetValue.h"
#include "TankTargetValue.h"
#include "DpsTargetValue.h"
#include "CcTargetValue.h"
#include "CurrentCcTargetValue.h"
#include "PetTargetValue.h"
#include "GrindTargetValue.h"
#include "RtiTargetValue.h"
#include "PartyMemberToDispel.h"
#include "StatsValues.h"
#include "AttackerCountValues.h"
#include "PossibleAttackTargetsValue.h"
#include "AvailableLootValue.h"
#include "AlwaysLootListValue.h"
#include "LootStrategyValue.h"
#include "HasAvailableLootValue.h"
#include "LastMovementValue.h"
#include "DistanceValue.h"
#include "IsMovingValue.h"
#include "IsBehindValue.h"
#include "IsFacingValue.h"
#include "ItemCountValue.h"
#include "SpellIdValue.h"
#include "ItemForSpellValue.h"
#include "SpellCastUsefulValue.h"
#include "LastSpellCastValue.h"
#include "ChatValue.h"
#include "HasTotemValue.h"
#include "HaveAnyTotemValue.h"
#include "LeastHpTargetValue.h"
#include "AoeHealValues.h"
#include "AoeValues.h"
#include "RtiValue.h"
#include "PositionValue.h"
#include "ThreatValues.h"
#include "DuelTargetValue.h"
#include "InvalidTargetValue.h"
#include "EnemyPlayerValue.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "CollisionValue.h"
#include "CraftValues.h"
#include "LastSpellCastTimeValue.h"
#include "CombatStartTimeValue.h"
#include "ManaSaveLevelValue.h"
#include "LfgValues.h"
#include "PvpValues.h"
#include "EnemyHealerTargetValue.h"
#include "Formations.h"
#include "ItemUsageValue.h"
#include "LastSaidValue.h"
#include "NearestFriendlyPlayersValue.h"
#include "NearestNonBotPlayersValue.h"
#include "NewPlayerNearbyValue.h"
#include "OutfitListValue.h"
#include "PartyMemberWithoutItemValue.h"
#include "PossibleRpgTargetsValue.h"
#include "RandomBotUpdateValue.h"
#include "RangeValues.h"
#include "SkipSpellsListValue.h"
#include "SnareTargetValue.h"
#include "Stances.h"
#include "QuestValues.h"
#include "BudgetValues.h"
#include "MaintenanceValues.h"
#include "GroupValues.h"
#include "GuildValues.h"
#include "TradeValues.h"
#include "RpgValues.h"
#include "RTSCValues.h"
#include "VendorValues.h"
#include "TrainerValues.h"
#include "AttackersValue.h"
#include "WaitForAttackTimeValue.h"
#include "LastPotionUsedTimeValue.h"
#include "OperatorValues.h"
#include "EntryValues.h"
#include "GuidPositionValues.h"
#include "EngineValues.h"
#include "FreeMoveValues.h"
#include "HazardsValue.h"
#include "FocusTargetValue.h"
#include "TalentSpecValue.h"
#include "MountValues.h"
#include "DeadValues.h"
#include "../druid/DruidValues.h"

namespace ai
{
    class ValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        ValueContext()
        {
            creators["active spell"] = &ValueContext::active_spell;
            creators["craft"] = &ValueContext::craft;
            creators["collision"] = &ValueContext::collision;
            creators["skip spells list"] = &ValueContext::skip_spells_list_value;
            creators["nearest game objects"] = &ValueContext::nearest_game_objects;
            creators["nearest game objects no los"] = &ValueContext::nearest_game_objects_no_los;
            creators["nearest dynamic objects"] = &ValueContext::nearest_dynamic_objects;
            creators["nearest dynamic objects no los"] = &ValueContext::nearest_dynamic_objects_no_los;
            creators["closest game objects"] = &ValueContext::closest_game_objects;
            creators["nearest npcs"] = &ValueContext::nearest_npcs;
            creators["nearest npcs no los"] = &ValueContext::nearest_npcs_no_los;
            creators["nearest vehicles"] = &ValueContext::nearest_vehicles;
            creators["nearest friendly players"] = &ValueContext::nearest_friendly_players;
            creators["closest friendly players"] = &ValueContext::closest_friendly_players;
            creators["possible targets"] = &ValueContext::possible_targets;
            creators["possible targets no los"] = &ValueContext::possible_targets_no_los;
            creators["possible adds"] = &ValueContext::possible_adds;
            creators["all targets"] = &ValueContext::all_targets;
            creators["possible rpg targets"] = &ValueContext::possible_rpg_targets;
            creators["nearest adds"] = &ValueContext::nearest_adds;
            creators["nearest corpses"] = &ValueContext::nearest_corpses;
            creators["nearest stealthed units"] = &ValueContext::nearest_stealthed_units;
            creators["nearest stealthed unit"] = &ValueContext::nearest_stealthed_unit;
            creators["log level"] = &ValueContext::log_level;
            creators["friendly unit without aura"] = &ValueContext::friendly_unit_without_aura;
            creators["party member without aura"] = &ValueContext::party_member_without_aura;
            creators["party tank without aura"] = &ValueContext::party_tank_without_aura;
            creators["party member without my aura"] = &ValueContext::party_member_without_my_aura;
            creators["attacker without aura"] = &ValueContext::attacker_without_aura;
            creators["party member to heal"] = &ValueContext::party_member_to_heal;
            creators["party member to resurrect"] = &ValueContext::party_member_to_resurrect;
            creators["current target"] = &ValueContext::current_target;
            creators["self target"] = &ValueContext::self_target;
            creators["master target"] = &ValueContext::master;
            creators["line target"] = &ValueContext::line_target;
            creators["tank target"] = &ValueContext::tank_target;
            creators["dps target"] = &ValueContext::dps_target;
            creators["dps aoe target"] = &ValueContext::dps_aoe_target;
            creators["least hp target"] = &ValueContext::least_hp_target;
            creators["enemy player target"] = &ValueContext::enemy_player_target;
            creators["enemy player targets"] = &ValueContext::enemy_player_targets;
            creators["has enemy player targets"] = &ValueContext::has_enemy_player_targets;
            creators["cc target"] = &ValueContext::cc_target;
            creators["current cc target"] = &ValueContext::current_cc_target;
            creators["pet target"] = &ValueContext::pet_target;
            creators["old target"] = &ValueContext::old_target;
            creators["grind target"] = &ValueContext::grind_target;
            creators["rti target"] = &ValueContext::rti_target;
            creators["rti cc target"] = &ValueContext::rti_cc_target;
            creators["duel target"] = &ValueContext::duel_target;
            creators["party member to dispel"] = &ValueContext::party_member_to_dispel;
            creators["party member to protect"] = &ValueContext::party_member_to_protect;
            creators["party member to remove roots"] = &ValueContext::party_member_to_remove_roots;
            creators["health"] = &ValueContext::health;
            creators["rage"] = &ValueContext::rage;
            creators["energy"] = &ValueContext::energy;
            creators["mana"] = &ValueContext::mana;
            creators["combo"] = &ValueContext::combo;
            creators["dead"] = &ValueContext::dead;
            creators["pet dead"] = &ValueContext::pet_dead;
            creators["pet happy"] = &ValueContext::pet_happy;
            creators["has mana"] = &ValueContext::has_mana;
            creators["attackers count"] = &ValueContext::attackers_count;
            creators["possible attack targets count"] = &ValueContext::possible_attack_targets_count;
            creators["my attacker count"] = &ValueContext::my_attacker_count;
            creators["has aggro"] = &ValueContext::has_aggro;
            creators["has attackers"] = &ValueContext::has_attackers;
            creators["has possible attack targets"] = &ValueContext::has_possible_attack_targets;
            creators["mounted"] = &ValueContext::mounted;

            creators["can loot"] = &ValueContext::can_loot;
            creators["loot target"] = &ValueContext::loot_target;
            creators["available loot"] = &ValueContext::available_loot;
            creators["has available loot"] = &ValueContext::has_available_loot;
            creators["stack space for item"] = &ValueContext::stack_space_for_item;
            creators["should loot object"] = &ValueContext::should_loot_object;
            creators["always loot list"] = &ValueContext::always_loot_list;
            creators["skip loot list"] = &ValueContext::skip_loot_list;
            creators["skip go loot list"] = &ValueContext::skip_go_loot_list;
            creators["loot strategy"] = &ValueContext::loot_strategy;
            creators["active rolls"] = &ValueContext::active_rolls;
            creators["last movement"] = &ValueContext::last_movement;
            creators["stay time"] = &ValueContext::stay_time;
            creators["last taxi"] = &ValueContext::last_movement;
            creators["last area trigger"] = &ValueContext::last_movement;
            creators["distance"] = &ValueContext::distance;
            creators["moving"] = &ValueContext::moving;
            creators["swimming"] = &ValueContext::swimming;
            creators["behind"] = &ValueContext::behind;
            creators["facing"] = &ValueContext::facing;

            creators["item count"] = &ValueContext::item_count;
            creators["inventory items"] = &ValueContext::inventory_item;
            creators["inventory item ids"] = &ValueContext::inventory_item_ids;
            creators["trinkets on use"] = &ValueContext::trinkets_on_use;

            creators["spell id"] = &ValueContext::spell_id;
            creators["vehicle spell id"] = &ValueContext::vehicle_spell_id;
            creators["item for spell"] = &ValueContext::item_for_spell;
            creators["spell cast useful"] = &ValueContext::spell_cast_useful;
            creators["last spell cast"] = &ValueContext::last_spell_cast;
            creators["last spell cast time"] = &ValueContext::last_spell_cast_time;
            creators["last potion used time"] = &ValueContext::last_potion_used_time;
            creators["chat"] = &ValueContext::chat;
            creators["has totem"] = &ValueContext::has_totem;
            creators["have any totem"] = &ValueContext::have_any_totem;

            creators["aoe heal"] = &ValueContext::aoe_heal;

            creators["rti cc"] = &ValueContext::rti_cc;
            creators["rti"] = &ValueContext::rti;
            creators["position"] = &ValueContext::position;
            creators["pos"] = &ValueContext::pos;
            creators["current position"] = &ValueContext::current_position;
            creators["master position"] = &ValueContext::master_position;
            creators["custom position"] = &ValueContext::custom_position;
            creators["my threat"] = &ValueContext::my_threat;
            creators["tank threat"] = &ValueContext::tank_threat;
            creators["threat"] = &ValueContext::threat;

            creators["incoming damage"] = &ValueContext::incoming_damage;
            creators["balance"] = &ValueContext::balance;
            creators["possible attack targets"] = &ValueContext::possible_attack_targets;
            creators["attackers"] = &ValueContext::attackers;
            creators["attackers targeting me"] = &ValueContext::attackers_targeting_me;
            creators["closest attacker targeting me"] = &ValueContext::closest_attacker_targeting_me;
            creators["add hazard"] = &ValueContext::add_hazard;
            creators["stored hazards"] = &ValueContext::stored_hazards;
            creators["hazards"] = &ValueContext::hazards;
            creators["invalid target"] = &ValueContext::invalid_target;
            creators["mana save level"] = &ValueContext::mana_save_level;
            creators["combat"] = &ValueContext::combat;
            creators["lfg proposal"] = &ValueContext::lfg_proposal;
            creators["bag space"] = &ValueContext::bag_space;
            creators["durability"] = &ValueContext::durability;
            creators["max repair cost"] = &ValueContext::max_repair_cost;
            creators["repair cost"] = &ValueContext::repair_cost;
            creators["train cost"] = &ValueContext::train_cost;
            creators["enemy healer target"] = &ValueContext::enemy_healer_target;
            creators["snare target"] = &ValueContext::snare_target;
            creators["formation"] = &ValueContext::formation;
            creators["formation position"] = &ValueContext::formation_position;
            creators["stance"] = &ValueContext::stance;
            creators["item usage"] = &ValueContext::item_usage;
            creators["force item usage"] = &ValueContext::force_item_usage;
            creators["speed"] = &ValueContext::speed;
            creators["last said"] = &ValueContext::last_said;
            creators["last emote"] = &ValueContext::last_emote;

            creators["aoe count"] = &ValueContext::aoe_count;
            creators["aoe position"] = &ValueContext::aoe_position;
            creators["outfit list"] = &ValueContext::outfit_list_value;

            creators["random bot update"] = &ValueContext::random_bot_update_value;
            creators["nearest non bot players"] = &ValueContext::nearest_non_bot_players;
            creators["new player nearby"] = &ValueContext::new_player_nearby;
            creators["already seen players"] = &ValueContext::already_seen_players;
            creators["rpg target"] = &ValueContext::rpg_target;
            creators["ignore rpg target"] = &ValueContext::ignore_rpg_target;
            creators["next rpg action"] = &ValueContext::next_rpg_action;
            creators["travel target"] = &ValueContext::travel_target;
            creators["talk target"] = &ValueContext::talk_target;
            creators["attack target"] = &ValueContext::attack_target;
            creators["pull target"] = &ValueContext::pull_target;
            creators["follow target"] = &ValueContext::follow_target;
            creators["manual follow target"] = &ValueContext::manual_follow_target;
            creators["group"] = &ValueContext::group;
            creators["range"] = &ValueContext::range;
            creators["inside target"] = &ValueContext::inside_target;
            creators["party member without item"] = &ValueContext::party_member_without_item;
            creators["party member without food"] = &ValueContext::party_member_without_food;
            creators["party member without water"] = &ValueContext::party_member_without_water;
            creators["death count"] = &ValueContext::death_count;

            creators["bg type"] = &ValueContext::bg_type;
            creators["rpg bg type"] = &ValueContext::rpg_bg_type; 
            creators["arena type"] = &ValueContext::arena_type;
            creators["bg role"] = &ValueContext::bg_role;
            creators["bg master"] = &ValueContext::bg_master;
            creators["enemy flag carrier"] = &ValueContext::enemy_fc;
            creators["team flag carrier"] = &ValueContext::team_fc;

            creators["home bind"] = &ValueContext::home_bind;
            creators["last long move"] = &ValueContext::last_long_move;
            creators["graveyard"] = &ValueContext::graveyard;
            creators["best graveyard"] = &ValueContext::best_graveyard;
            creators["should spirit healer"] = &ValueContext::should_spirit_healer;

            creators["bot roles"] = &ValueContext::bot_roles;
            creators["talent spec"] = &ValueContext::talent_spec;
            
            creators["free quest log slots"] = &ValueContext::free_quest_log_slots;
            creators["dialog status"] = &ValueContext::dialog_status;
            creators["dialog status quest"] = &ValueContext::dialog_status_quest;
            creators["can accept quest npc"] = &ValueContext::can_accept_quest_npc;
            creators["can accept quest low level npc"] = &ValueContext::can_accept_quest_low_level_npc;
            creators["can turn in quest npc"] = &ValueContext::can_turn_in_quest_npc;
            creators["can repeat quest npc"] = &ValueContext::can_repeat_quest_npc;
            creators["need quest reward"] = &ValueContext::need_quest_reward;
            creators["need quest objective"] = &ValueContext::need_quest_objective;
            creators["can use item on"] = &ValueContext::can_use_item_on;
            creators["quest reward"] = &ValueContext::quest_reward;
            
            creators["money needed for"] = &ValueContext::money_needed_for;
            creators["total money needed for"] = &ValueContext::total_money_needed_for;
            creators["free money for"] = &ValueContext::free_money_for;
            creators["should get money"] = &ValueContext::should_get_money;

            creators["free move center"] = &ValueContext::free_move_center;
            creators["free move range"] = &ValueContext::free_move_range;
            creators["can free move to"] = &ValueContext::can_free_move_to;
            creators["can free attack"] = &ValueContext::can_free_attack;
            creators["can free target"] = &ValueContext::can_free_target;
            
            creators["can move around"] = &ValueContext::can_move_around;
            creators["should home bind"] = &ValueContext::should_home_bind;
            creators["should repair"] = &ValueContext::should_repair;
            creators["can repair"] = &ValueContext::can_repair;
            creators["should sell"] = &ValueContext::should_sell;
            creators["can sell"] = &ValueContext::can_sell;
            creators["can buy"] = &ValueContext::can_buy;
            creators["can ah sell"] = &ValueContext::can_ah_sell;
            creators["can ah buy"] = &ValueContext::can_ah_buy;
            creators["can get mail"] = &ValueContext::can_get_mail;
            creators["can fight equal"] = &ValueContext::can_fight_equal;
            creators["can fight elite"] = &ValueContext::can_fight_elite;
            creators["can fight boss"] = &ValueContext::can_fight_boss;

            creators["vendor has useful item"] = &ValueContext::vendor_has_useful_item;
            creators["craft spells"] = &ValueContext::craft_spells;
            creators["has reagents for"] = &ValueContext::has_reagents_for;
            creators["can craft spell"] = &ValueContext::can_craft_spell;
            creators["should craft spell"] = &ValueContext::should_craft_spell;

            creators["group members"] = &ValueContext::group_members;
            creators["following party"] = &ValueContext::following_party;
            creators["near leader"] = &ValueContext::near_leader;
            creators["and"] = &ValueContext::and_value;
            creators["not"] = &ValueContext::not_value;
            creators["group count"] = &ValueContext::group_count;
            creators["group and"] = &ValueContext::group_and;
            creators["group or"] = &ValueContext::group_or;
            creators["group ready"] = &ValueContext::group_ready;

            creators["petition signs"] = &ValueContext::petition_signs;

            creators["experience"] = &ValueContext::experience;
            creators["honor"] = &ValueContext::honor;

            creators["entry loot usage"] = &ValueContext::entry_loot_usage;
            creators["has upgrade"] = &ValueContext::has_upgrade;
            creators["items useful to give"] = &ValueContext::items_useful_to_give;

            creators["see spell location"] = &ValueContext::see_spell_location;
            creators["RTSC selected"] = &ValueContext::RTSC_selected;
            creators["RTSC next spell action"] = &ValueContext::RTSC_next_spell_action;
            creators["RTSC saved location"] = &ValueContext::RTSC_saved_location;

            creators["trainable class spells"] = &ValueContext::trainable_class_spells;
            creators["mount list"] = &ValueContext::mount_list;
            creators["current mount speed"] = &ValueContext::current_mount_speed;

            creators["has area debuff"] = &ValueContext::has_area_debuff;
            creators["combat start time"] = &ValueContext::combat_start_time;
            creators["wait for attack time"] = &ValueContext::wait_for_attack_time;
            creators["focus heal target"] = &ValueContext::focus_heal_target;

            creators["mc runes"] = &ValueContext::mc_runes;
            creators["gos"] = &ValueContext::gos;
            creators["entry filter"] = &ValueContext::entry_filter;
            creators["range filter"] = &ValueContext::range_filter;
            creators["go usable filter"] = &ValueContext::go_usable_filter;
            creators["go trapped filter"] = &ValueContext::go_trapped_filter;
            creators["gos in sight"] = &ValueContext::gos_in_sight;
            creators["gos close"] = &ValueContext::gos_close;
            creators["has object"] = &ValueContext::has_object;

            creators["action possible"] = &ValueContext::action_possible;
            creators["action useful"] = &ValueContext::action_useful;
            creators["trigger active"] = &ValueContext::trigger_active;

            creators["party tank without lifebloom"] = &ValueContext::party_tank_without_lifebloom;
            creators["move style"] = &ValueContext::move_style;
        }

    private:
        static UntypedValue* team_fc(PlayerbotAI* ai) { return new FlagCarrierValue(ai, true, true); }
        static UntypedValue* enemy_fc(PlayerbotAI* ai) { return new FlagCarrierValue(ai, false, true); }
        static UntypedValue* bg_master(PlayerbotAI* ai) { return new BgMasterValue(ai); }
        static UntypedValue* bg_role(PlayerbotAI* ai) { return new BgRoleValue(ai); }
        static UntypedValue* arena_type(PlayerbotAI* ai) { return new ArenaTypeValue(ai); }
        static UntypedValue* bg_type(PlayerbotAI* ai) { return new BgTypeValue(ai); }
        static UntypedValue* rpg_bg_type(PlayerbotAI* ai) { return new RpgBgTypeValue(ai); }
        static UntypedValue* party_member_without_water(PlayerbotAI* ai) { return new PartyMemberWithoutWaterValue(ai); }
        static UntypedValue* party_member_without_food(PlayerbotAI* ai) { return new PartyMemberWithoutFoodValue(ai); }
        static UntypedValue* party_member_without_item(PlayerbotAI* ai) { return new PartyMemberWithoutItemValue(ai); }
        static UntypedValue* inside_target(PlayerbotAI* ai) { return new InsideTargetValue(ai); }
        static UntypedValue* range(PlayerbotAI* ai) { return new RangeValue(ai); }
        static UntypedValue* active_spell(PlayerbotAI* ai) { return new ActiveSpellValue(ai); }
        static UntypedValue* group(PlayerbotAI* ai) { return new IsInGroupValue(ai); }
        static UntypedValue* craft(PlayerbotAI* ai) { return new CraftValue(ai); }
        static UntypedValue* collision(PlayerbotAI* ai) { return new CollisionValue(ai); }
        static UntypedValue* already_seen_players(PlayerbotAI* ai) { return new AlreadySeenPlayersValue(ai); }
        static UntypedValue* new_player_nearby(PlayerbotAI* ai) { return new NewPlayerNearbyValue(ai); }
        static UntypedValue* item_usage(PlayerbotAI* ai) { return new ItemUsageValue(ai); }
        static UntypedValue* force_item_usage(PlayerbotAI* ai) { return new ForceItemUsageValue(ai); }
        static UntypedValue* formation(PlayerbotAI* ai) { return new FormationValue(ai); }
        static UntypedValue* formation_position(PlayerbotAI* ai) { return new FormationPositionValue(ai); }
        static UntypedValue* stance(PlayerbotAI* ai) { return new StanceValue(ai); }
        static UntypedValue* mana_save_level(PlayerbotAI* ai) { return new ManaSaveLevelValue(ai); }
        static UntypedValue* invalid_target(PlayerbotAI* ai) { return new InvalidTargetValue(ai); }
        static UntypedValue* balance(PlayerbotAI* ai) { return new BalancePercentValue(ai); }
        static UntypedValue* incoming_damage(PlayerbotAI* ai) { return new IncomingDamageValue(ai); }
        static UntypedValue* possible_attack_targets(PlayerbotAI* ai) { return new PossibleAttackTargetsValue(ai); }
        static UntypedValue* attackers(PlayerbotAI* ai) { return new AttackersValue(ai); }
        static UntypedValue* attackers_targeting_me(PlayerbotAI* ai) { return new AttackersTargetingMeValue(ai); }
        static UntypedValue* closest_attacker_targeting_me(PlayerbotAI* ai) { return new ClosestAttackerTargetingMeTargetValue(ai); }
        static UntypedValue* add_hazard(PlayerbotAI* ai) { return new AddHazardValue(ai); }
        static UntypedValue* stored_hazards(PlayerbotAI* ai) { return new StoredHazardsValue(ai); }
        static UntypedValue* hazards(PlayerbotAI* ai) { return new HazardsValue(ai); }

        static UntypedValue* position(PlayerbotAI* ai) { return new PositionValue(ai); }
        static UntypedValue* pos(PlayerbotAI* ai) { return new SinglePositionValue(ai); }
        static UntypedValue* current_position(PlayerbotAI* ai) { return new CurrentPositionValue(ai); }
        static UntypedValue* master_position(PlayerbotAI* ai) { return new MasterPositionValue(ai); }
        static UntypedValue* custom_position(PlayerbotAI* ai) { return new CustomPositionValue(ai); }
        static UntypedValue* rti(PlayerbotAI* ai) { return new RtiValue(ai); }
        static UntypedValue* rti_cc(PlayerbotAI* ai) { return new RtiCcValue(ai); }

        static UntypedValue* aoe_heal(PlayerbotAI* ai) { return new AoeHealValue(ai); }

        static UntypedValue* chat(PlayerbotAI* ai) { return new ChatValue(ai); }
        static UntypedValue* last_spell_cast(PlayerbotAI* ai) { return new LastSpellCastValue(ai); }
        static UntypedValue* last_spell_cast_time(PlayerbotAI* ai) { return new LastSpellCastTimeValue(ai); }
        static UntypedValue* last_potion_used_time(PlayerbotAI* ai) { return new LastPotionUsedTimeValue(ai); }
        static UntypedValue* spell_cast_useful(PlayerbotAI* ai) { return new SpellCastUsefulValue(ai); }
        static UntypedValue* item_for_spell(PlayerbotAI* ai) { return new ItemForSpellValue(ai); }
        static UntypedValue* spell_id(PlayerbotAI* ai) { return new SpellIdValue(ai); }
        static UntypedValue* vehicle_spell_id(PlayerbotAI* ai) { return new VehicleSpellIdValue(ai); }
        static UntypedValue* inventory_item(PlayerbotAI* ai) { return new InventoryItemValue(ai); }
        static UntypedValue* inventory_item_ids(PlayerbotAI* ai) { return new InventoryItemIdValue(ai); }
        static UntypedValue* trinkets_on_use(PlayerbotAI* ai) { return new EquipedUsableTrinketValue(ai); }
        static UntypedValue* item_count(PlayerbotAI* ai) { return new ItemCountValue(ai); }
        static UntypedValue* behind(PlayerbotAI* ai) { return new IsBehindValue(ai); }
        static UntypedValue* facing(PlayerbotAI* ai) { return new IsFacingValue(ai); }
        static UntypedValue* moving(PlayerbotAI* ai) { return new IsMovingValue(ai); }
        static UntypedValue* swimming(PlayerbotAI* ai) { return new IsSwimmingValue(ai); }
        static UntypedValue* distance(PlayerbotAI* ai) { return new DistanceValue(ai); }
        static UntypedValue* last_movement(PlayerbotAI* ai) { return new LastMovementValue(ai); }
        static UntypedValue* stay_time(PlayerbotAI* ai) { return new StayTimeValue(ai); }

        static UntypedValue* can_loot(PlayerbotAI* ai) { return new CanLootValue(ai); }
        static UntypedValue* available_loot(PlayerbotAI* ai) { return new AvailableLootValue(ai); }
        static UntypedValue* loot_target(PlayerbotAI* ai) { return new LootTargetValue(ai); }
        static UntypedValue* has_available_loot(PlayerbotAI* ai) { return new HasAvailableLootValue(ai); }
        static UntypedValue* stack_space_for_item(PlayerbotAI* ai) { return new StackSpaceForItem(ai); }
        static UntypedValue* should_loot_object(PlayerbotAI* ai) { return new ShouldLootObject(ai); }
        static UntypedValue* always_loot_list(PlayerbotAI* ai) { return new AlwaysLootListValue(ai, "always loot list"); }
        static UntypedValue* skip_loot_list(PlayerbotAI* ai) { return new AlwaysLootListValue(ai, "skip loot list"); }
        static UntypedValue* skip_go_loot_list(PlayerbotAI* ai) { return new AlwaysLootListValue(ai, "skip go loot list"); }
        static UntypedValue* loot_strategy(PlayerbotAI* ai) { return new LootStrategyValue(ai); }
        static UntypedValue* active_rolls(PlayerbotAI* ai) { return new ActiveRolls(ai); }
        

        static UntypedValue* attackers_count(PlayerbotAI* ai) { return new AttackersCountValue(ai); }
        static UntypedValue* possible_attack_targets_count(PlayerbotAI* ai) { return new PossibleAttackTargetsCountValue(ai); }
        static UntypedValue* my_attacker_count(PlayerbotAI* ai) { return new MyAttackerCountValue(ai); }
        static UntypedValue* has_aggro(PlayerbotAI* ai) { return new HasAggroValue(ai); }
        static UntypedValue* has_attackers(PlayerbotAI* ai) { return new HasAttackersValue(ai); }
        static UntypedValue* has_possible_attack_targets(PlayerbotAI* ai) { return new HasPossibleAttackTargetsValue(ai); }
        static UntypedValue* mounted(PlayerbotAI* ai) { return new IsMountedValue(ai); }
        static UntypedValue* health(PlayerbotAI* ai) { return new HealthValue(ai); }
        static UntypedValue* rage(PlayerbotAI* ai) { return new RageValue(ai); }
        static UntypedValue* energy(PlayerbotAI* ai) { return new EnergyValue(ai); }
        static UntypedValue* mana(PlayerbotAI* ai) { return new ManaValue(ai); }
        static UntypedValue* combo(PlayerbotAI* ai) { return new ComboPointsValue(ai); }
        static UntypedValue* dead(PlayerbotAI* ai) { return new IsDeadValue(ai); }
        static UntypedValue* pet_happy(PlayerbotAI* ai) { return new PetIsHappyValue(ai); }
        static UntypedValue* pet_dead(PlayerbotAI* ai) { return new PetIsDeadValue(ai); }
        static UntypedValue* has_mana(PlayerbotAI* ai) { return new HasManaValue(ai); }
        static UntypedValue* nearest_game_objects(PlayerbotAI* ai) { return new NearestGameObjects(ai); }
        static UntypedValue* nearest_game_objects_no_los(PlayerbotAI* ai) { return new NearestGameObjects(ai, sPlayerbotAIConfig.sightDistance, true); }
        static UntypedValue* nearest_dynamic_objects(PlayerbotAI* ai) { return new NearestDynamicObjects(ai); }
        static UntypedValue* nearest_dynamic_objects_no_los(PlayerbotAI* ai) { return new NearestDynamicObjects(ai, sPlayerbotAIConfig.sightDistance, true); }
        static UntypedValue* closest_game_objects(PlayerbotAI* ai) { return new NearestGameObjects(ai, INTERACTION_DISTANCE); }
        static UntypedValue* log_level(PlayerbotAI* ai) { return new LogLevelValue(ai); }
        static UntypedValue* nearest_npcs(PlayerbotAI* ai) { return new NearestNpcsValue(ai); }
        static UntypedValue* nearest_npcs_no_los(PlayerbotAI* ai) { return new NearestNpcsValue(ai, sPlayerbotAIConfig.sightDistance, true); }
        static UntypedValue* nearest_vehicles(PlayerbotAI* ai) { return new NearestVehiclesValue(ai); }
        static UntypedValue* nearest_friendly_players(PlayerbotAI* ai) { return new NearestFriendlyPlayersValue(ai); }
        static UntypedValue* closest_friendly_players(PlayerbotAI* ai) { return new NearestFriendlyPlayersValue(ai, INTERACTION_DISTANCE); }
        static UntypedValue* nearest_corpses(PlayerbotAI* ai) { return new NearestCorpsesValue(ai); }
        static UntypedValue* nearest_stealthed_units(PlayerbotAI* ai) { return new NearestStealthedUnitsValue(ai); }
        static UntypedValue* nearest_stealthed_unit(PlayerbotAI* ai) { return new NearestStealthedSingleUnitValue(ai); }
        static UntypedValue* possible_rpg_targets(PlayerbotAI* ai) { return new PossibleRpgTargetsValue(ai); }
        static UntypedValue* possible_targets(PlayerbotAI* ai) { return new PossibleTargetsValue(ai); }
        static UntypedValue* possible_targets_no_los(PlayerbotAI* ai) { return new PossibleTargetsValue(ai, "possible targets", sPlayerbotAIConfig.sightDistance, true); }
        static UntypedValue* possible_adds(PlayerbotAI* ai) { return new PossibleAddsValue(ai); }
        static UntypedValue* all_targets(PlayerbotAI* ai) { return new AllTargetsValue(ai); }
        static UntypedValue* nearest_adds(PlayerbotAI* ai) { return new NearestAddsValue(ai); }
        static UntypedValue* friendly_unit_without_aura(PlayerbotAI* ai) { return new FriendlyUnitWithoutAuraValue(ai); }
        static UntypedValue* party_member_without_aura(PlayerbotAI* ai) { return new PartyMemberWithoutAuraValue(ai); }
        static UntypedValue* party_member_without_my_aura(PlayerbotAI* ai) { return new PartyMemberWithoutMyAuraValue(ai); }
        static UntypedValue* party_tank_without_aura(PlayerbotAI* ai) { return new PartyTankWithoutAuraValue(ai); }
        static UntypedValue* attacker_without_aura(PlayerbotAI* ai) { return new AttackerWithoutAuraTargetValue(ai); }
        static UntypedValue* party_member_to_heal(PlayerbotAI* ai) { return new PartyMemberToHeal(ai); }
        static UntypedValue* party_member_to_resurrect(PlayerbotAI* ai) { return new PartyMemberToResurrect(ai); }
        static UntypedValue* party_member_to_dispel(PlayerbotAI* ai) { return new PartyMemberToDispel(ai); }
        static UntypedValue* party_member_to_protect(PlayerbotAI* ai) { return new PartyMemberToProtect(ai); }
        static UntypedValue* party_member_to_remove_roots(PlayerbotAI* ai) { return new PartyMemberToRemoveRoots(ai); }
        static UntypedValue* current_target(PlayerbotAI* ai) { return new CurrentTargetValue(ai); }
        static UntypedValue* old_target(PlayerbotAI* ai) { return new CurrentTargetValue(ai); }
        static UntypedValue* self_target(PlayerbotAI* ai) { return new SelfTargetValue(ai); }
        static UntypedValue* master(PlayerbotAI* ai) { return new MasterTargetValue(ai); }
        static UntypedValue* line_target(PlayerbotAI* ai) { return new LineTargetValue(ai); }
        static UntypedValue* tank_target(PlayerbotAI* ai) { return new TankTargetValue(ai); }
        static UntypedValue* dps_target(PlayerbotAI* ai) { return new DpsTargetValue(ai); }
        static UntypedValue* dps_aoe_target(PlayerbotAI* ai) { return new DpsAoeTargetValue(ai); }
        static UntypedValue* least_hp_target(PlayerbotAI* ai) { return new LeastHpTargetValue(ai); }
        static UntypedValue* enemy_player_target(PlayerbotAI* ai) { return new EnemyPlayerValue(ai); }
        static UntypedValue* enemy_player_targets(PlayerbotAI* ai) { return new EnemyPlayersValue(ai); }
        static UntypedValue* has_enemy_player_targets(PlayerbotAI* ai) { return new HasEnemyPlayersValue(ai); }
        static UntypedValue* cc_target(PlayerbotAI* ai) { return new CcTargetValue(ai); }
        static UntypedValue* current_cc_target(PlayerbotAI* ai) { return new CurrentCcTargetValue(ai); }
        static UntypedValue* pet_target(PlayerbotAI* ai) { return new PetTargetValue(ai); }
        static UntypedValue* grind_target(PlayerbotAI* ai) { return new GrindTargetValue(ai); }
        static UntypedValue* rti_target(PlayerbotAI* ai) { return new RtiTargetValue(ai); }
        static UntypedValue* rti_cc_target(PlayerbotAI* ai) { return new RtiCcTargetValue(ai); }
        static UntypedValue* duel_target(PlayerbotAI* ai) { return new DuelTargetValue(ai); }
        static UntypedValue* has_totem(PlayerbotAI* ai) { return new HasTotemValue(ai); }
        static UntypedValue* have_any_totem(PlayerbotAI* ai) { return new HaveAnyTotemValue(ai); }
        static UntypedValue* my_threat(PlayerbotAI* ai) { return new MyThreatValue(ai); }
        static UntypedValue* tank_threat(PlayerbotAI* ai) { return new TankThreatValue(ai); }
        static UntypedValue* threat(PlayerbotAI* ai) { return new ThreatValue(ai); }
        static UntypedValue* combat(PlayerbotAI* ai) { return new IsInCombatValue(ai); }
        static UntypedValue* lfg_proposal(PlayerbotAI* ai) { return new LfgProposalValue(ai); }
        static UntypedValue* bag_space(PlayerbotAI* ai) { return new BagSpaceValue(ai); }
        static UntypedValue* durability(PlayerbotAI* ai) { return new DurabilityValue(ai); }
        static UntypedValue* max_repair_cost(PlayerbotAI* ai) { return new MaxGearRepairCostValue(ai); }
        static UntypedValue* repair_cost(PlayerbotAI* ai) { return new RepairCostValue(ai); }
        static UntypedValue* train_cost(PlayerbotAI* ai) { return new TrainCostValue(ai); }
        static UntypedValue* enemy_healer_target(PlayerbotAI* ai) { return new EnemyHealerTargetValue(ai); }
        static UntypedValue* snare_target(PlayerbotAI* ai) { return new SnareTargetValue(ai); }
        static UntypedValue* speed(PlayerbotAI* ai) { return new SpeedValue(ai); }
        static UntypedValue* last_said(PlayerbotAI* ai) { return new LastSaidValue(ai); }
        static UntypedValue* last_emote(PlayerbotAI* ai) { return new LastEmoteValue(ai); }
        static UntypedValue* aoe_count(PlayerbotAI* ai) { return new AoeCountValue(ai); }
        static UntypedValue* aoe_position(PlayerbotAI* ai) { return new AoePositionValue(ai); }
        static UntypedValue* outfit_list_value(PlayerbotAI* ai) { return new OutfitListValue(ai); }
        static UntypedValue* random_bot_update_value(PlayerbotAI* ai) { return new RandomBotUpdateValue(ai); }
        static UntypedValue* nearest_non_bot_players(PlayerbotAI* ai) { return new NearestNonBotPlayersValue(ai); }
        static UntypedValue* skip_spells_list_value(PlayerbotAI* ai) { return new SkipSpellsListValue(ai); }
        static UntypedValue* rpg_target(PlayerbotAI* ai) { return new RpgTargetValue(ai); }
        static UntypedValue* ignore_rpg_target(PlayerbotAI* ai) { return new IgnoreRpgTargetValue(ai); }
        static UntypedValue* next_rpg_action(PlayerbotAI* ai) { return new NextRpgActionValue(ai); }
        static UntypedValue* travel_target(PlayerbotAI* ai) { return new TravelTargetValue(ai); }
        static UntypedValue* talk_target(PlayerbotAI* ai) { return new TalkTargetValue(ai); }
        static UntypedValue* attack_target(PlayerbotAI* ai) { return new AttackTargetValue(ai); }
        static UntypedValue* pull_target(PlayerbotAI* ai) { return new PullTargetValue(ai); }
        static UntypedValue* follow_target(PlayerbotAI* ai) { return new FollowTargetValue(ai); }
        static UntypedValue* manual_follow_target(PlayerbotAI* ai) { return new ManualFollowTargetValue(ai); }
        static UntypedValue* death_count(PlayerbotAI* ai) { return new DeathCountValue(ai); }

        static UntypedValue* last_long_move(PlayerbotAI* ai) { return new LastLongMoveValue(ai); }
        static UntypedValue* home_bind(PlayerbotAI* ai) { return new HomeBindValue(ai); }
        static UntypedValue* graveyard(PlayerbotAI* ai) { return new GraveyardValue(ai); }
        static UntypedValue* best_graveyard(PlayerbotAI* ai) { return new BestGraveyardValue(ai); }
        static UntypedValue* should_spirit_healer(PlayerbotAI* ai) { return new ShouldSpiritHealerValue(ai); }

        static UntypedValue* bot_roles(PlayerbotAI* ai) { return new BotRolesValue(ai); }
        static UntypedValue* talent_spec(PlayerbotAI* ai) { return new TalentSpecValue(ai); }

        static UntypedValue* free_quest_log_slots(PlayerbotAI* ai) { return new FreeQuestLogSlotValue(ai); }
        static UntypedValue* dialog_status(PlayerbotAI* ai) { return new DialogStatusValue(ai); }
        static UntypedValue* dialog_status_quest(PlayerbotAI* ai) { return new DialogStatusQuestValue(ai); }
        static UntypedValue* can_accept_quest_npc(PlayerbotAI* ai) { return new CanAcceptQuestValue(ai); }
        static UntypedValue* can_accept_quest_low_level_npc(PlayerbotAI* ai) { return new CanAcceptQuestLowLevelValue(ai); }
        static UntypedValue* can_turn_in_quest_npc(PlayerbotAI* ai) { return new CanTurnInQuestValue(ai); }
        static UntypedValue* can_repeat_quest_npc(PlayerbotAI* ai) { return new CanRepeatQuestValue(ai); }
        static UntypedValue* need_quest_reward(PlayerbotAI* ai) { return new NeedQuestRewardValue(ai); }
        static UntypedValue* need_quest_objective(PlayerbotAI* ai) { return new NeedQuestObjectiveValue(ai); }
        static UntypedValue* can_use_item_on(PlayerbotAI* ai) { return new CanUseItemOn(ai); }
        static UntypedValue* quest_reward(PlayerbotAI* ai) { return new QuestRewardValue(ai); }

        static UntypedValue* money_needed_for(PlayerbotAI* ai) { return new MoneyNeededForValue(ai); }
        static UntypedValue* total_money_needed_for(PlayerbotAI* ai) { return new TotalMoneyNeededForValue(ai); }
        static UntypedValue* free_money_for(PlayerbotAI* ai) { return new FreeMoneyForValue(ai); }
        static UntypedValue* should_get_money(PlayerbotAI* ai) { return new ShouldGetMoneyValue(ai); }

        static UntypedValue* free_move_center(PlayerbotAI* ai) { return new FreeMoveCenterValue(ai); }
        static UntypedValue* free_move_range(PlayerbotAI* ai) { return new FreeMoveRangeValue(ai); }
        static UntypedValue* can_free_move_to(PlayerbotAI* ai) { return new CanFreeMoveToValue(ai); }
        static UntypedValue* can_free_attack(PlayerbotAI* ai) { return new CanFreeAttackValue(ai); }
        static UntypedValue* can_free_target(PlayerbotAI* ai) { return new CanFreeTargetValue(ai); }

        static UntypedValue* can_move_around(PlayerbotAI* ai) { return new CanMoveAroundValue(ai); }
        static UntypedValue* should_home_bind(PlayerbotAI* ai) { return new ShouldHomeBindValue(ai); }
        static UntypedValue* should_repair(PlayerbotAI* ai) { return new ShouldRepairValue(ai); }
        static UntypedValue* can_repair(PlayerbotAI* ai) { return new CanRepairValue(ai); }
        static UntypedValue* should_sell(PlayerbotAI* ai) { return new ShouldSellValue(ai); }
        static UntypedValue* can_sell(PlayerbotAI* ai) { return new CanSellValue(ai); }
        static UntypedValue* can_buy(PlayerbotAI* ai) { return new CanBuyValue(ai); }
        static UntypedValue* can_ah_sell(PlayerbotAI* ai) { return new CanAHSellValue(ai); }
        static UntypedValue* can_ah_buy(PlayerbotAI* ai) { return new CanAHBuyValue(ai); }
        static UntypedValue* can_get_mail(PlayerbotAI* ai) { return new CanGetMailValue(ai); }
        static UntypedValue* can_fight_equal(PlayerbotAI* ai) { return new CanFightEqualValue(ai); }
        static UntypedValue* can_fight_elite(PlayerbotAI* ai) { return new CanFightEliteValue(ai); }
        static UntypedValue* can_fight_boss(PlayerbotAI* ai) { return new CanFightBossValue(ai); }

        static UntypedValue* vendor_has_useful_item(PlayerbotAI* ai) { return new VendorHasUsefulItemValue(ai); }
        static UntypedValue* craft_spells(PlayerbotAI* ai) { return new CraftSpellsValue(ai); }
        static UntypedValue* has_reagents_for(PlayerbotAI* ai) { return new HasReagentsForValue(ai); }
        static UntypedValue* can_craft_spell(PlayerbotAI* ai) { return new CanCraftSpellValue(ai); }
        static UntypedValue* should_craft_spell(PlayerbotAI* ai) { return new ShouldCraftSpellValue(ai); }

        static UntypedValue* group_members(PlayerbotAI* ai) { return new GroupMembersValue(ai); }
        static UntypedValue* following_party(PlayerbotAI* ai) { return new IsFollowingPartyValue(ai); }
        static UntypedValue* near_leader(PlayerbotAI* ai) { return new IsNearLeaderValue(ai); }
        static UntypedValue* and_value(PlayerbotAI* ai) { return new BoolAndValue(ai); }
        static UntypedValue* not_value(PlayerbotAI* ai) { return new NotValue(ai); }
        static UntypedValue* group_count(PlayerbotAI* ai) { return new GroupBoolCountValue(ai); }
        static UntypedValue* group_and(PlayerbotAI* ai) { return new GroupBoolANDValue(ai); }
        static UntypedValue* group_or(PlayerbotAI* ai) { return new GroupBoolORValue(ai); }
        static UntypedValue* group_ready(PlayerbotAI* ai) { return new GroupReadyValue(ai); }

        static UntypedValue* petition_signs(PlayerbotAI* ai) { return new PetitionSignsValue(ai); }

        static UntypedValue* experience(PlayerbotAI* ai) { return new ExperienceValue(ai); }
        static UntypedValue* honor(PlayerbotAI* ai) { return new HonorValue(ai); }

        static UntypedValue* entry_loot_usage(PlayerbotAI* ai) { return new EntryLootUsageValue(ai); }
        static UntypedValue* has_upgrade(PlayerbotAI* ai) { return new HasUpgradeValue(ai); }
        static UntypedValue* items_useful_to_give(PlayerbotAI* ai) { return new ItemsUsefulToGiveValue(ai); }

        static UntypedValue* see_spell_location(PlayerbotAI* ai) { return new SeeSpellLocationValue(ai); }
        static UntypedValue* RTSC_selected(PlayerbotAI* ai) { return new RTSCSelectedValue(ai); }
        static UntypedValue* RTSC_next_spell_action(PlayerbotAI* ai) { return new RTSCNextSpellActionValue(ai); }
        static UntypedValue* RTSC_saved_location(PlayerbotAI* ai) { return new RTSCSavedLocationValue(ai); }

        static UntypedValue* trainable_class_spells(PlayerbotAI* ai) { return new TrainableClassSpells(ai); }
        static UntypedValue* mount_list(PlayerbotAI* ai) { return new MountListValue(ai); }
        static UntypedValue* current_mount_speed(PlayerbotAI* ai) { return new CurrentMountSpeedValue(ai); }

        static UntypedValue* has_area_debuff(PlayerbotAI* ai) { return new HasAreaDebuffValue(ai); }
        static UntypedValue* combat_start_time(PlayerbotAI* ai) { return new CombatStartTimeValue(ai); }
        static UntypedValue* wait_for_attack_time(PlayerbotAI* ai) { return new WaitForAttackTimeValue(ai); }
        static UntypedValue* focus_heal_target(PlayerbotAI* ai) { return new FocusHealTargetValue(ai); }

        static UntypedValue* mc_runes(PlayerbotAI* ai) { return new MCRunesValue(ai); }
        static UntypedValue* gos(PlayerbotAI* ai) { return new GameObjectsValue(ai); }
        static UntypedValue* entry_filter(PlayerbotAI* ai) { return new EntryFilterValue(ai); }
        static UntypedValue* range_filter(PlayerbotAI* ai) { return new RangeFilterValue(ai); }
        static UntypedValue* go_usable_filter(PlayerbotAI* ai) { return new GoUsableFilterValue(ai); }
        static UntypedValue* go_trapped_filter(PlayerbotAI* ai) { return new GoTrappedFilterValue(ai); }
        static UntypedValue* gos_in_sight(PlayerbotAI* ai) { return new GosInSightValue(ai); }
        static UntypedValue* gos_close(PlayerbotAI* ai) { return new GoSCloseValue(ai); }
        static UntypedValue* has_object(PlayerbotAI* ai) { return new HasObjectValue(ai); }

        static UntypedValue* action_possible(PlayerbotAI* ai) { return new ActionPossibleValue(ai); }
        static UntypedValue* action_useful(PlayerbotAI* ai) { return new ActionUsefulValue(ai); }
        static UntypedValue* trigger_active(PlayerbotAI* ai) { return new TriggerActiveValue(ai); }

        static UntypedValue* party_tank_without_lifebloom(PlayerbotAI* ai) { return new PartyTankWithoutLifebloomValue(ai); }
        static UntypedValue* move_style(PlayerbotAI* ai) { return new MoveStyleValue(ai); }
    };
};