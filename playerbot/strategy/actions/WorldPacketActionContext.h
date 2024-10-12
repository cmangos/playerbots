#pragma once

#include "AcceptInvitationAction.h"
#include "PassLeadershipToMasterAction.h"
#include "TellMasterAction.h"
#include "TalkToQuestGiverAction.h"
#include "AcceptQuestAction.h"
#include "LootRollAction.h"
#include "ReviveFromCorpseAction.h"
#include "AcceptResurrectAction.h"
#include "UseMeetingStoneAction.h"
#include "AreaTriggerAction.h"
#include "CheckMountStateAction.h"
#include "RememberTaxiAction.h"
#include "TradeStatusAction.h"
#include "InventoryChangeFailureAction.h"
#include "LootAction.h"
#include "QuestAction.h"
#include "LeaveGroupAction.h"
#include "TellCastFailedAction.h"
#include "AcceptDuelAction.h"
#include "ReadyCheckAction.h"
#include "LfgActions.h"
#include "SecurityCheckAction.h"
#include "GuildAcceptAction.h"
#include "AcceptBattleGroundInvitationAction.h"
#include "PetitionSignAction.h"
#include "BattleGroundJoinAction.h"
#include "SeeSpellAction.h"
#include "ArenaTeamActions.h"

namespace ai
{
    class WorldPacketActionContext : public NamedObjectContext<Action>
    {
    public:
        WorldPacketActionContext()
        {   
            creators["lfg join"] = &WorldPacketActionContext::lfg_join;
            creators["lfg accept"] = &WorldPacketActionContext::lfg_accept;
            creators["lfg role check"] = &WorldPacketActionContext::lfg_role_check;
            creators["lfg leave"] = &WorldPacketActionContext::lfg_leave;
            creators["lfg teleport"] = &WorldPacketActionContext::lfg_teleport;
            creators["bg status check"] = &WorldPacketActionContext::bg_status_check;
            creators["bg status"] = &WorldPacketActionContext::bg_status;
            creators["bg join"] = &WorldPacketActionContext::bg_join;
            creators["bg leave"] = &WorldPacketActionContext::bg_leave;
            creators["arena tactics"] = &WorldPacketActionContext::arena_tactics;
            creators["accept invitation"] = &WorldPacketActionContext::accept_invitation;
            creators["give leader in dungeon"] = &WorldPacketActionContext::give_leader_in_dungeon;
            creators["leader"] = &WorldPacketActionContext::pass_leadership_to_master;
            creators["tell not enough money"] = &WorldPacketActionContext::tell_not_enough_money;
            creators["tell not enough reputation"] = &WorldPacketActionContext::tell_not_enough_reputation;
            creators["tell cannot equip"] = &WorldPacketActionContext::tell_cannot_equip;
            creators["talk to quest giver"] = &WorldPacketActionContext::turn_in_quest;
            creators["accept quest"] = &WorldPacketActionContext::accept_quest;
            creators["confirm quest"] = &WorldPacketActionContext::confirm_quest;
            creators["quest details"] = &WorldPacketActionContext::quest_details;
            creators["accept all quests"] = &WorldPacketActionContext::accept_all_quests;
            creators["accept quest share"] = &WorldPacketActionContext::accept_quest_share;
            creators["loot start roll"] = &WorldPacketActionContext::loot_start_roll;
            creators["loot roll"] = &WorldPacketActionContext::loot_roll;
            creators["revive from corpse"] = &WorldPacketActionContext::revive_from_corpse;
            creators["find corpse"] = &WorldPacketActionContext::find_corpse;
            creators["auto release"] = &WorldPacketActionContext::auto_release;
            creators["accept resurrect"] = &WorldPacketActionContext::accept_resurrect;
            creators["use meeting stone"] = &WorldPacketActionContext::use_meeting_stone;
            creators["accept summon"] = &WorldPacketActionContext::accept_summon;
            creators["area trigger"] = &WorldPacketActionContext::area_trigger;
            creators["reach area trigger"] = &WorldPacketActionContext::reach_area_trigger;
            creators["check mount state"] = &WorldPacketActionContext::check_mount_state;
            creators["remember taxi"] = &WorldPacketActionContext::remember_taxi;
            creators["accept trade"] = &WorldPacketActionContext::accept_trade;
            creators["store loot"] = &WorldPacketActionContext::store_loot;
            creators["quest update add kill"] = &WorldPacketActionContext::quest_update_add_kill;
            creators["quest update add item"] = &WorldPacketActionContext::quest_update_add_item;
            creators["quest update failed"] = &WorldPacketActionContext::quest_update_failed;
            creators["quest update failed timer"] = &WorldPacketActionContext::quest_update_failed_timer;
            creators["quest update complete"] = &WorldPacketActionContext::quest_update_complete;
            creators["party command"] = &WorldPacketActionContext::party_command;
            creators["tell cast failed"] = &WorldPacketActionContext::tell_cast_failed;
            creators["accept duel"] = &WorldPacketActionContext::accept_duel;
            creators["ready check"] = &WorldPacketActionContext::ready_check;
            creators["ready check finished"] = &WorldPacketActionContext::ready_check_finished;
            creators["uninvite"] = &WorldPacketActionContext::uninvite;
            creators["security check"] = &WorldPacketActionContext::security_check;
            creators["guild accept"] = &WorldPacketActionContext::guild_accept;
            creators["inventory change failure"] = &WorldPacketActionContext::inventory_change_failure;
            creators["petition sign"] = &WorldPacketActionContext::petition_sign;
            creators["see spell"] = &WorldPacketActionContext::see_spell;
            creators["arena team accept"] = &WorldPacketActionContext::arena_team_accept;
        }

    private:
        static Action* petition_sign(PlayerbotAI* ai) { return new PetitionSignAction(ai); }
        static Action* lfg_teleport(PlayerbotAI* ai) { return new LfgTeleportAction(ai); }
        static Action* lfg_leave(PlayerbotAI* ai) { return new LfgLeaveAction(ai); }
        static Action* lfg_accept(PlayerbotAI* ai) { return new LfgAcceptAction(ai); }
        static Action* lfg_role_check(PlayerbotAI* ai) { return new LfgRoleCheckAction(ai); }
        static Action* lfg_join(PlayerbotAI* ai) { return new LfgJoinAction(ai); }
        static Action* bg_leave(PlayerbotAI* ai) { return new BGLeaveAction(ai); }
        static Action* bg_join(PlayerbotAI* ai) { return new BGJoinAction(ai); }
        static Action* bg_status_check(PlayerbotAI* ai) { return new BGStatusCheckAction(ai); }
        static Action* bg_status(PlayerbotAI* ai) { return new BGStatusAction(ai); }
        static Action* arena_tactics(PlayerbotAI* ai) { return new ArenaTactics(ai); }
        static Action* inventory_change_failure(PlayerbotAI* ai) { return new InventoryChangeFailureAction(ai); }
        static Action* guild_accept(PlayerbotAI* ai) { return new GuildAcceptAction(ai); }
        static Action* security_check(PlayerbotAI* ai) { return new SecurityCheckAction(ai); }
        static Action* uninvite(PlayerbotAI* ai) { return new UninviteAction(ai); }
        static Action* ready_check_finished(PlayerbotAI* ai) { return new FinishReadyCheckAction(ai); }
        static Action* ready_check(PlayerbotAI* ai) { return new ReadyCheckAction(ai); }
        static Action* accept_duel(PlayerbotAI* ai) { return new AcceptDuelAction(ai); }
        static Action* tell_cast_failed(PlayerbotAI* ai) { return new TellCastFailedAction(ai); }
        static Action* party_command(PlayerbotAI* ai) { return new PartyCommandAction(ai); }
        static Action* quest_update_add_kill(PlayerbotAI* ai) { return new QuestUpdateAddKillAction(ai); }
        static Action* quest_update_add_item(PlayerbotAI* ai) { return new QuestUpdateAddItemAction(ai); }
        static Action* quest_update_failed(PlayerbotAI* ai) { return new QuestUpdateFailedAction(ai); }
        static Action* quest_update_failed_timer(PlayerbotAI* ai) { return new QuestUpdateFailedTimerAction(ai); }
        static Action* quest_update_complete(PlayerbotAI* ai) { return new QuestUpdateCompleteAction(ai); }
        static Action* store_loot(PlayerbotAI* ai) { return new StoreLootAction(ai); }
        static Action* accept_trade(PlayerbotAI* ai) { return new TradeStatusAction(ai); }
        static Action* remember_taxi(PlayerbotAI* ai) { return new RememberTaxiAction(ai); }
        static Action* check_mount_state(PlayerbotAI* ai) { return new CheckMountStateAction(ai); }
        static Action* area_trigger(PlayerbotAI* ai) { return new AreaTriggerAction(ai); }
        static Action* reach_area_trigger(PlayerbotAI* ai) { return new ReachAreaTriggerAction(ai); }
        static Action* use_meeting_stone(PlayerbotAI* ai) { return new UseMeetingStoneAction(ai); }
        static Action* accept_summon(PlayerbotAI* ai) { return new AcceptSummonAction(ai); }
        static Action* accept_resurrect(PlayerbotAI* ai) { return new AcceptResurrectAction(ai); }
        static Action* revive_from_corpse(PlayerbotAI* ai) { return new ReviveFromCorpseAction(ai); }
        static Action* find_corpse(PlayerbotAI* ai) { return new FindCorpseAction(ai); }
        static Action* auto_release(PlayerbotAI* ai) { return new AutoReleaseSpiritAction(ai); }
        static Action* accept_invitation(PlayerbotAI* ai) { return new AcceptInvitationAction(ai); }
        static Action* pass_leadership_to_master(PlayerbotAI* ai) { return new PassLeadershipToMasterAction(ai); }
        static Action* give_leader_in_dungeon(PlayerbotAI* ai) { return new GiveLeaderAction(ai, "I don't know this dungeon, lead the way!"); }
        static Action* tell_not_enough_money(PlayerbotAI* ai) { return new TellMasterAction(ai, "Not enough money"); }
        static Action* tell_not_enough_reputation(PlayerbotAI* ai) { return new TellMasterAction(ai, "Not enough reputation"); }
        static Action* tell_cannot_equip(PlayerbotAI* ai) { return new InventoryChangeFailureAction(ai); }
        static Action* turn_in_quest(PlayerbotAI* ai) { return new TalkToQuestGiverAction(ai); }
        static Action* accept_quest(PlayerbotAI* ai) { return new AcceptQuestAction(ai); }
        static Action* confirm_quest(PlayerbotAI* ai) { return new ConfirmQuestAction(ai); }
        static Action* quest_details(PlayerbotAI* ai) { return new QuestDetailsAction(ai); }
        static Action* accept_all_quests(PlayerbotAI* ai) { return new AcceptAllQuestsAction(ai); }
        static Action* accept_quest_share(PlayerbotAI* ai) { return new AcceptQuestShareAction(ai); }
        static Action* loot_start_roll(PlayerbotAI* ai) { return (QueryItemUsageAction*)new LootStartRollAction(ai); }
        static Action* loot_roll(PlayerbotAI* ai) { return (QueryItemUsageAction*)new LootRollAction(ai); }
        static Action* see_spell(PlayerbotAI* ai) { return new SeeSpellAction(ai); }
        static Action* arena_team_accept(PlayerbotAI* ai) { return new ArenaTeamAcceptAction(ai); }
    };
};
