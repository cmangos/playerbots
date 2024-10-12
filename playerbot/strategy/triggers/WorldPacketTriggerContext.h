#pragma once

#include "WorldPacketTrigger.h"
#include "WithinAreaTrigger.h"

namespace ai
{
    class WorldPacketTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        WorldPacketTriggerContext()
        {
            creators["gossip hello"] = &WorldPacketTriggerContext::gossip_hello;
            creators["group invite"] = &WorldPacketTriggerContext::group_invite;
            creators["group set leader"] = &WorldPacketTriggerContext::group_set_leader;
            creators["not enough money"] = &WorldPacketTriggerContext::no_money;
            creators["not enough reputation"] = &WorldPacketTriggerContext::no_reputation;
            creators["cannot equip"] = &WorldPacketTriggerContext::cannot_equip;
            creators["use game object"] = &WorldPacketTriggerContext::use_game_object;
            creators["complete quest"] = &WorldPacketTriggerContext::complete_quest;
            creators["accept quest"] = &WorldPacketTriggerContext::accept_quest;
            creators["confirm quest"] = &WorldPacketTriggerContext::confirm_quest;
            creators["quest details"] = &WorldPacketTriggerContext::quest_details;
            creators["quest share"] = &WorldPacketTriggerContext::quest_share;
            creators["loot start roll"] = &WorldPacketTriggerContext::loot_start_roll;
            creators["loot roll"] = &WorldPacketTriggerContext::loot_roll;
            creators["resurrect request"] = &WorldPacketTriggerContext::resurrect_request;
            creators["area trigger"] = &WorldPacketTriggerContext::area_trigger;
            creators["within area trigger"] = &WorldPacketTriggerContext::within_area_trigger;
            creators["check mount state"] = &WorldPacketTriggerContext::check_mount_state;
            creators["activate taxi"] = &WorldPacketTriggerContext::taxi;
            creators["trade status"] = &WorldPacketTriggerContext::trade_status;
            creators["loot response"] = &WorldPacketTriggerContext::loot_response;
            creators["quest update add kill"] = &WorldPacketTriggerContext::quest_update_add_kill;
            creators["quest update add item"] = &WorldPacketTriggerContext::quest_update_add_item;
            creators["quest update failed"] = &WorldPacketTriggerContext::quest_update_failed;
            creators["quest update failed timer"] = &WorldPacketTriggerContext::quest_update_failed_timer;
            creators["quest update complete"] = &WorldPacketTriggerContext::quest_update_complete;
            creators["item push result"] = &WorldPacketTriggerContext::item_push_result;
            creators["party command"] = &WorldPacketTriggerContext::party_command;
            creators["taxi done"] = &WorldPacketTriggerContext::taxi_done;
            creators["cast failed"] = &WorldPacketTriggerContext::cast_failed;
            creators["duel requested"] = &WorldPacketTriggerContext::duel_requested;
            creators["ready check"] = &WorldPacketTriggerContext::ready_check;
            creators["ready check finished"] = &WorldPacketTriggerContext::ready_check_finished;
            creators["uninvite"] = &WorldPacketTriggerContext::uninvite;
            creators["uninvite guid"] = &WorldPacketTriggerContext::uninvite_guid;
            creators["lfg join"] = &WorldPacketTriggerContext::lfg_update;
            creators["lfg proposal"] = &WorldPacketTriggerContext::lfg_proposal;
            creators["lfg role check"] = &WorldPacketTriggerContext::lfg_role_check;
            creators["lfg leave"] = &WorldPacketTriggerContext::lfg_leave;
            creators["guild accept"] = &WorldPacketTriggerContext::guild_accept;
            creators["petition offer"] = &WorldPacketTriggerContext::petition_offer;
            creators["lfg teleport"] = &WorldPacketTriggerContext::lfg_teleport;
            creators["inventory change failure"] = &WorldPacketTriggerContext::inventory_change_failure;
            creators["bg status"] = &WorldPacketTriggerContext::bg_status;
            creators["levelup"] = &WorldPacketTriggerContext::levelup;
            creators["xpgain"] = &WorldPacketTriggerContext::xpgain;
            creators["honorgain"] = &WorldPacketTriggerContext::honorgain;
            creators["see spell"] = &WorldPacketTriggerContext::seespell;
            creators["release spirit"] = &WorldPacketTriggerContext::release_spirit;
            creators["revive from corpse"] = &WorldPacketTriggerContext::revive_from_corpse;
            creators["receive emote"] = &WorldPacketTriggerContext::receive_emote;
            creators["receive text emote"] = &WorldPacketTriggerContext::receive_text_emote;
            creators["arena team invite"] = &WorldPacketTriggerContext::arena_team_invite;
            creators["summon request"] = &WorldPacketTriggerContext::summon_request;
        }

    private:
        static Trigger* bg_status(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "bg status"); }
        static Trigger* inventory_change_failure(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "inventory change failure"); }
        static Trigger* petition_offer(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "petition offer"); }
        static Trigger* guild_accept(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "guild accept"); }
        static Trigger* lfg_teleport(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg teleport"); }
        static Trigger* lfg_leave(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg leave"); }
        static Trigger* lfg_proposal(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg proposal"); }
        static Trigger* lfg_role_check(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg role check"); }
        static Trigger* lfg_update(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "lfg join"); }
        static Trigger* uninvite(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "uninvite"); }
        static Trigger* uninvite_guid(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "uninvite guid"); }
        static Trigger* ready_check_finished(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "ready check finished"); }
        static Trigger* ready_check(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "ready check"); }
        static Trigger* duel_requested(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "duel requested"); }
        static Trigger* cast_failed(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "cast failed"); }
        static Trigger* taxi_done(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "taxi done"); }
        static Trigger* party_command(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "party command"); }
        static Trigger* item_push_result(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "item push result"); }
        static Trigger* quest_update_add_kill(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add kill"); }
        static Trigger* quest_update_add_item(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add item"); }
        static Trigger* quest_update_failed(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed"); }
        static Trigger* quest_update_failed_timer(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed timer"); }
        static Trigger* quest_update_complete(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update complete"); }
        static Trigger* loot_response(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "loot response"); }
        static Trigger* trade_status(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "trade status"); }
        static Trigger* cannot_equip(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "cannot equip"); }
        static Trigger* check_mount_state(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "check mount state"); }
        static Trigger* area_trigger(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "area trigger"); }
        static Trigger* within_area_trigger(PlayerbotAI* ai) { return new WithinAreaTrigger(ai); }
        static Trigger* resurrect_request(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "resurrect request"); }
        static Trigger* gossip_hello(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "gossip hello"); }
        static Trigger* group_invite(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "group invite"); }
        static Trigger* group_set_leader(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "group set leader"); }
        static Trigger* no_money(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "not enough money"); }
        static Trigger* no_reputation(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "not enough reputation"); }
        static Trigger* use_game_object(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "use game object"); }
        static Trigger* complete_quest(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "complete quest"); }
        static Trigger* accept_quest(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "accept quest"); }
        static Trigger* confirm_quest(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "confirm quest"); }
        static Trigger* quest_details(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest details"); }
        static Trigger* quest_share(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest share"); }
        static Trigger* loot_start_roll(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "loot start roll"); }
        static Trigger* loot_roll(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "loot roll"); }
        static Trigger* taxi(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "activate taxi"); }
        static Trigger* levelup(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "levelup"); }
        static Trigger* xpgain(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "xpgain"); }
        static Trigger* honorgain(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "honorgain"); }
        static Trigger* seespell(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "see spell"); }
        static Trigger* release_spirit(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "release spirit"); }
        static Trigger* revive_from_corpse(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "revive from corpse"); }
        static Trigger* receive_emote(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "receive emote"); }
        static Trigger* receive_text_emote(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "receive text emote"); }
        static Trigger* arena_team_invite(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "arena team invite"); }
        static Trigger* summon_request(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "summon request"); }
    };
};
