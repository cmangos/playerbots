#pragma once

#include "ListQuestsActions.h"
#include "StatsAction.h"
#include "LeaveGroupAction.h"
#include "TellReputationAction.h"
#include "LogLevelAction.h"
#include "TellLosAction.h"
#include "DropQuestAction.h"
#include "QueryQuestAction.h"
#include "QueryItemUsageAction.h"
#include "LootStrategyAction.h"
#include "AddLootAction.h"
#include "LootRollAction.h"
#include "ReleaseSpiritAction.h"
#include "TeleportAction.h"
#include "TaxiAction.h"
#include "RepairAllAction.h"
#include "UseItemAction.h"
#include "TellItemCountAction.h"
#include "RewardAction.h"
#include "BuyAction.h"
#include "SellAction.h"
#include "UnequipAction.h"
#include "EquipAction.h"
#include "KeepItemAction.h"
#include "TradeAction.h"
#include "ChangeTalentsAction.h"
#include "ListSpellsAction.h"
#include "ChangeStrategyAction.h"
#include "TrainerAction.h"
#include "ChangeChatAction.h"
#include "SetHomeAction.h"
#include "ResetAiAction.h"
#include "DestroyItemAction.h"
#include "BuffAction.h"
#include "AttackAction.h"
#include "HelpAction.h"
#include "GuildBankAction.h"
#include "ChatShortcutActions.h"
#include "GossipHelloAction.h"
#include "CastCustomSpellAction.h"
#include "InviteToGroupAction.h"
#include "TellCastFailedAction.h"
#include "RtiAction.h"
#include "ReviveFromCorpseAction.h"
#include "BankAction.h"
#include "PositionAction.h"
#include "TellTargetAction.h"
#include "UseMeetingStoneAction.h"
#include "WhoAction.h"
#include "SaveManaAction.h"
#include "playerbot/strategy/values/Formations.h"
#include "playerbot/strategy/values/Stances.h"
#include "CustomStrategyEditAction.h"
#include "AhAction.h"
#include "DebugAction.h"
#include "GoAction.h"
#include "MailAction.h"
#include "SendMailAction.h"
#include "ShareQuestAction.h"
#include "SkipSpellsListAction.h"
#include "CustomStrategyEditAction.h"
#include "FlagAction.h"
#include "HireAction.h"
#include "RangeAction.h"
#include "SetCraftAction.h"
#include "WtsAction.h"
#include "PassLeadershipToMasterAction.h"
#include "CheatAction.h"
#include "GuildManagementActions.h"
#include "RtscAction.h"
#include "BattleGroundJoinAction.h"
#include "MoveStyleAction.h"
#include "ValueActions.h"
#include "QuestRewardActions.h"
#include "ChooseTravelTargetAction.h"
#include "SkillAction.h"
#include "FactionAction.h"
#include "SetValueAction.h"

namespace ai
{
    class ChatActionContext : public NamedObjectContext<Action>
    {
    public:
        ChatActionContext()
        {
            creators["range"] = &ChatActionContext::range;
            creators["stats"] = &ChatActionContext::stats;
            creators["quests"] = &ChatActionContext::quests;
            creators["quest reward"] = &ChatActionContext::quest_reward;
            creators["leave"] = &ChatActionContext::leave;
            creators["reputation"] = &ChatActionContext::reputation;
            creators["log"] = &ChatActionContext::log;
            creators["los"] = &ChatActionContext::los;
            creators["drop"] = &ChatActionContext::drop;
            creators["clean quest log"] = &ChatActionContext::clean_quest_log;
            creators["share"] = &ChatActionContext::share;
            creators["query quest"] = &ChatActionContext::query_quest;
            creators["query item usage"] = &ChatActionContext::query_item_usage;
            creators["ll"] = &ChatActionContext::ll;
            creators["ss"] = &ChatActionContext::ss;
            creators["loot"] = &ChatActionContext::add_all_loot;
            creators["add all loot"] = &ChatActionContext::add_all_loot;
            creators["roll"] = &ChatActionContext::roll;
            creators["release"] = &ChatActionContext::release;
            creators["repop"] = &ChatActionContext::repop;
            creators["teleport"] = &ChatActionContext::teleport;
            creators["taxi"] = &ChatActionContext::taxi;
            creators["repair"] = &ChatActionContext::repair;
            creators["use"] = &ChatActionContext::use;
            creators["item count"] = &ChatActionContext::item_count;
            creators["equip"] = &ChatActionContext::equip;
            creators["equip upgrades"] = &ChatActionContext::equip_upgrades;
            creators["unequip"] = &ChatActionContext::unequip;
            creators["keep"] = &ChatActionContext::keep;
            creators["sell"] = &ChatActionContext::sell;
            creators["buy"] = &ChatActionContext::buy;
            creators["buy back"] = &ChatActionContext::buy_back;
            creators["reward"] = &ChatActionContext::reward;
            creators["trade"] = &ChatActionContext::trade;
            creators["talents"] = &ChatActionContext::talents;
            creators["spells"] = &ChatActionContext::spells;
            creators["co"] = &ChatActionContext::co;
            creators["nc"] = &ChatActionContext::nc;
            creators["de"] = &ChatActionContext::dead;
            creators["react"] = &ChatActionContext::react;
            creators["all"] = &ChatActionContext::all;
            creators["trainer"] = &ChatActionContext::trainer;
            creators["attack my target"] = &ChatActionContext::attack_my_target;
            creators["attack rti target"] = &ChatActionContext::attack_rti_target;
            creators["chat"] = &ChatActionContext::chat;
            creators["home"] = &ChatActionContext::home;
            creators["destroy"] = &ChatActionContext::destroy;
            creators["load ai"] = [](PlayerbotAI* ai) { return new LoadAiAction(ai); };
            creators["list ai"] = [](PlayerbotAI* ai) { return new ListAiAction(ai); };
            creators["save ai"] = [](PlayerbotAI* ai) { return new SaveAiAction(ai); };
            creators["reset ai"] = &ChatActionContext::reset_ai;
            creators["reset strats"] = &ChatActionContext::reset_strats;
            creators["reset values"] = &ChatActionContext::reset_values;
            creators["reset ai soft"] = &ChatActionContext::reset_ai_soft;
            creators["buff"] = &ChatActionContext::buff;
            creators["help"] = &ChatActionContext::help;
            creators["gb"] = &ChatActionContext::gb;
            creators["bank"] = &ChatActionContext::bank;
            creators["follow chat shortcut"] = &ChatActionContext::follow_chat_shortcut;
            creators["stay chat shortcut"] = &ChatActionContext::stay_chat_shortcut;
            creators["guard chat shortcut"] = &ChatActionContext::guard_chat_shortcut;
            creators["free chat shortcut"] = &ChatActionContext::free_chat_shortcut;
            creators["flee chat shortcut"] = &ChatActionContext::flee_chat_shortcut;
            creators["runaway chat shortcut"] = &ChatActionContext::runaway_chat_shortcut;
            creators["grind chat shortcut"] = &ChatActionContext::grind_chat_shortcut;
            creators["tank attack chat shortcut"] = &ChatActionContext::tank_attack_chat_shortcut;
            creators["gossip hello"] = &ChatActionContext::gossip_hello;
            creators["cast"] = &ChatActionContext::cast_custom_spell;
            creators["cast custom nc spell"] = &ChatActionContext::cast_custom_nc_spell;
            creators["invite"] = &ChatActionContext::invite;
            creators["join"] = &ChatActionContext::join;
            creators["lfg"] = &ChatActionContext::lfg;
            creators["spell"] = &ChatActionContext::spell;
            creators["rti"] = &ChatActionContext::rti;
            creators["spirit healer"] = &ChatActionContext::spirit_healer;
            creators["position"] = &ChatActionContext::position;
            creators["tell target"] = &ChatActionContext::tell_target;
            creators["summon"] = &ChatActionContext::summon;
            creators["who"] = &ChatActionContext::who;
            creators["save mana"] = &ChatActionContext::save_mana;
            creators["max dps chat shortcut"] = &ChatActionContext::max_dps_chat_shortcut;
            creators["tell possible attack targets"] = &ChatActionContext::tell_possible_attack_targets;
            creators["tell attackers"] = &ChatActionContext::tell_attackers;
            creators["formation"] = &ChatActionContext::formation;
            creators["stance"] = &ChatActionContext::stance;
            creators["sendmail"] = &ChatActionContext::sendmail;
            creators["mail"] = &ChatActionContext::mail;
            creators["go"] = &ChatActionContext::go;
            creators["debug"] = &ChatActionContext::debug;
            creators["cdebug"] = &ChatActionContext::debug;
            creators["cs"] = &ChatActionContext::cs;
            creators["wts"] = &ChatActionContext::wts;
            creators["hire"] = &ChatActionContext::hire;
            creators["craft"] = &ChatActionContext::craft;
            creators["flag"] = &ChatActionContext::flag;
            creators["give leader"] = &ChatActionContext::give_leader;
            creators["cheat"] = &ChatActionContext::cheat;
            creators["rtsc"] = &ChatActionContext::rtsc;
            creators["ah"] = &ChatActionContext::ah;
            creators["ah bid"] = &ChatActionContext::ah_bid;
            creators["wait for attack time"] = &ChatActionContext::wait_for_attack_time;
            creators["self resurrect"] = &ChatActionContext::self_resurrect;
            creators["pet"] = &ChatActionContext::pet;

            creators["focus heal targets"] = &ChatActionContext::focus_heal_targets;
            creators["follow target"] = &ChatActionContext::follow_target;
            creators["boost targets"] = &ChatActionContext::boost_targets;
            creators["revive targets"] = &ChatActionContext::revive_targets;
            creators["buff targets"] = &ChatActionContext::buff_targets;

            creators["guild invite"] = &ChatActionContext::guild_invite;
            creators["guild join"] = &ChatActionContext::guild_join;
            creators["guild promote"] = &ChatActionContext::guild_promote;
            creators["guild demote"] = &ChatActionContext::guild_demote;
            creators["guild remove"] = &ChatActionContext::guild_remove;
            creators["guild leave"] = &ChatActionContext::guild_leave;
            creators["guild leader"] = &ChatActionContext::guild_leader;

            creators["bg free"] = &ChatActionContext::bg_free;
            creators["move style"] = &ChatActionContext::move_style;

            creators["jump"] = &ChatActionContext::jump;
            creators["doquest"] = [](PlayerbotAI* ai) { return new FocusTravelTargetAction(ai); };
            creators["skill"] = [](PlayerbotAI* ai) { return new SkillAction(ai); };
            creators["faction"] = [](PlayerbotAI* ai) { return new FactionAction(ai); };
            creators["set value"] = [](PlayerbotAI* ai) { return new SetValueAction(ai); };
        }

    private:
        static Action* range(PlayerbotAI* ai) { return new RangeAction(ai); }
        static Action* flag(PlayerbotAI* ai) { return new FlagAction(ai); }
        static Action* craft(PlayerbotAI* ai) { return new SetCraftAction(ai); }
        static Action* hire(PlayerbotAI* ai) { return new HireAction(ai); }
        static Action* wts(PlayerbotAI* ai) { return new WtsAction(ai); }
        static Action* cs(PlayerbotAI* ai) { return new CustomStrategyEditAction(ai); }
        static Action* debug(PlayerbotAI* ai) { return new DebugAction(ai); }
        static Action* mail(PlayerbotAI* ai) { return new MailAction(ai); }
        static Action* go(PlayerbotAI* ai) { return new GoAction(ai); }
        static Action* sendmail(PlayerbotAI* ai) { return new SendMailAction(ai); }
        static Action* formation(PlayerbotAI* ai) { return new SetFormationAction(ai); }
        static Action* stance(PlayerbotAI* ai) { return new SetStanceAction(ai); }
        static Action* tell_possible_attack_targets(PlayerbotAI* ai) { return new TellPossibleAttackTargetsAction(ai); }
        static Action* tell_attackers(PlayerbotAI* ai) { return new TellAttackersAction(ai); }
        static Action* max_dps_chat_shortcut(PlayerbotAI* ai) { return new MaxDpsChatShortcutAction(ai); }
        static Action* save_mana(PlayerbotAI* ai) { return new SaveManaAction(ai); }
        static Action* who(PlayerbotAI* ai) { return new WhoAction(ai); }
        static Action* summon(PlayerbotAI* ai) { return new SummonAction(ai); }
        static Action* tell_target(PlayerbotAI* ai) { return new TellTargetAction(ai); }
        static Action* position(PlayerbotAI* ai) { return new PositionAction(ai); }
        static Action* spirit_healer(PlayerbotAI* ai) { return new SpiritHealerAction(ai); }
        static Action* rti(PlayerbotAI* ai) { return new RtiAction(ai); }
        static Action* invite(PlayerbotAI* ai) { return new InviteToGroupAction(ai); }
        static Action* join(PlayerbotAI* ai) { return new JoinGroupAction(ai); }
        static Action* lfg (PlayerbotAI* ai) { return new LfgAction(ai); }
        static Action* spell(PlayerbotAI* ai) { return new TellSpellAction(ai); }
        static Action* cast_custom_spell(PlayerbotAI* ai) { return new CastCustomSpellAction(ai); }
        static Action* cast_custom_nc_spell(PlayerbotAI* ai) { return new CastCustomNcSpellAction(ai); }        
        static Action* tank_attack_chat_shortcut(PlayerbotAI* ai) { return new TankAttackChatShortcutAction(ai); }
        static Action* grind_chat_shortcut(PlayerbotAI* ai) { return new GrindChatShortcutAction(ai); }
        static Action* flee_chat_shortcut(PlayerbotAI* ai) { return new FleeChatShortcutAction(ai); }
        static Action* runaway_chat_shortcut(PlayerbotAI* ai) { return new GoawayChatShortcutAction(ai); }
        static Action* stay_chat_shortcut(PlayerbotAI* ai) { return new StayChatShortcutAction(ai); }
        static Action* follow_chat_shortcut(PlayerbotAI* ai) { return new FollowChatShortcutAction(ai); }
        static Action* free_chat_shortcut(PlayerbotAI* ai) { return new FreeChatShortcutAction(ai); }
        static Action* guard_chat_shortcut(PlayerbotAI* ai) { return new GuardChatShortcutAction(ai); }
        static Action* gb(PlayerbotAI* ai) { return new GuildBankAction(ai); }
        static Action* bank(PlayerbotAI* ai) { return new BankAction(ai); }
        static Action* help(PlayerbotAI* ai) { return new HelpAction(ai); }
        static Action* buff(PlayerbotAI* ai) { return new BuffAction(ai); }
        static Action* destroy(PlayerbotAI* ai) { return new DestroyItemAction(ai); }
        static Action* home(PlayerbotAI* ai) { return new SetHomeAction(ai); }
        static Action* chat(PlayerbotAI* ai) { return new ChangeChatAction(ai); }
        static Action* attack_my_target(PlayerbotAI* ai) { return new AttackMyTargetAction(ai); }
        static Action* attack_rti_target(PlayerbotAI* ai) { return new AttackRTITargetAction(ai); }
        static Action* trainer(PlayerbotAI* ai) { return new TrainerAction(ai); }
        static Action* co(PlayerbotAI* ai) { return new ChangeCombatStrategyAction(ai); }
        static Action* nc(PlayerbotAI* ai) { return new ChangeNonCombatStrategyAction(ai); }
        static Action* dead(PlayerbotAI* ai) { return new ChangeDeadStrategyAction(ai); }
        static Action* react(PlayerbotAI* ai) { return new ChangeReactionStrategyAction(ai); }
        static Action* all(PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai); }
        static Action* spells(PlayerbotAI* ai) { return new ListSpellsAction(ai); }
        static Action* talents(PlayerbotAI* ai) { return new ChangeTalentsAction(ai); }

        static Action* equip(PlayerbotAI* ai) { return new EquipAction(ai); }
        static Action* equip_upgrades(PlayerbotAI* ai) { return new EquipUpgradesAction(ai); }
        static Action* unequip(PlayerbotAI* ai) { return new UnequipAction(ai); }
        static Action* keep(PlayerbotAI* ai) { return new KeepItemAction(ai); }
        static Action* sell(PlayerbotAI* ai) { return new SellAction(ai); }
        static Action* buy(PlayerbotAI* ai) { return new BuyAction(ai); }
        static Action* buy_back(PlayerbotAI* ai) { return new BuyBackAction(ai); }
        static Action* reward(PlayerbotAI* ai) { return new RewardAction(ai); }
        static Action* trade(PlayerbotAI* ai) { return new TradeAction(ai); }

        static Action* item_count(PlayerbotAI* ai) { return new TellItemCountAction(ai); }
        static Action* use(PlayerbotAI* ai) { return new UseAction(ai); }
        static Action* repair(PlayerbotAI* ai) { return new RepairAllAction(ai); }
        static Action* taxi(PlayerbotAI* ai) { return new TaxiAction(ai); }
        static Action* teleport(PlayerbotAI* ai) { return new TeleportAction(ai); }
        static Action* release(PlayerbotAI* ai) { return new ReleaseSpiritAction(ai); }
        static Action* repop(PlayerbotAI* ai) { return new RepopAction(ai); }
        static Action* query_item_usage(PlayerbotAI* ai) { return new QueryItemUsageAction(ai); }
        static Action* query_quest(PlayerbotAI* ai) { return new QueryQuestAction(ai); }
        static Action* drop(PlayerbotAI* ai) { return new DropQuestAction(ai); }
        static Action* clean_quest_log(PlayerbotAI* ai) { return new CleanQuestLogAction(ai); }
        static Action* share(PlayerbotAI* ai) { return new ShareQuestAction(ai); }
        static Action* stats(PlayerbotAI* ai) { return new StatsAction(ai); }
        static Action* quests(PlayerbotAI* ai) { return new ListQuestsAction(ai); }
        static Action* quest_reward(PlayerbotAI* ai) { return new QuestRewardAction(ai); }
        static Action* leave(PlayerbotAI* ai) { return new LeaveGroupAction(ai); }
        static Action* reputation(PlayerbotAI* ai) { return new TellReputationAction(ai); }
        static Action* log(PlayerbotAI* ai) { return new LogLevelAction(ai); }
        static Action* los(PlayerbotAI* ai) { return new TellLosAction(ai); }
        static Action* ll(PlayerbotAI* ai) { return new LootStrategyAction(ai); }
        static Action* ss(PlayerbotAI* ai) { return new SkipSpellsListAction(ai); }
        static Action* add_all_loot(PlayerbotAI* ai) { return new AddAllLootAction(ai); }
        static Action* roll(PlayerbotAI* ai) { return new RollAction(ai); }
        static Action* reset_ai(PlayerbotAI* ai) { return new ResetAiAction(ai, true); }
        static Action* reset_strats(PlayerbotAI* ai) { return new ResetStratsAction(ai); }
        static Action* reset_values(PlayerbotAI* ai) { return new ResetValuesAction(ai); }
        static Action* reset_ai_soft(PlayerbotAI* ai) { return new ResetAiAction(ai, false); }
        static Action* gossip_hello(PlayerbotAI* ai) { return new GossipHelloAction(ai); }
        static Action* give_leader(PlayerbotAI* ai) { return new GiveLeaderAction(ai); }
        static Action* cheat(PlayerbotAI* ai) { return new CheatAction(ai); }
        static Action* rtsc(PlayerbotAI* ai) { return new RTSCAction(ai); }
        static Action* ah(PlayerbotAI* ai) { return new AhAction(ai); }
        static Action* ah_bid(PlayerbotAI* ai) { return new AhBidAction(ai); }
        static Action* wait_for_attack_time(PlayerbotAI* ai) { return new SetWaitForAttackTimeAction(ai); }
        static Action* self_resurrect(PlayerbotAI* ai) { return new SelfResurrectAction(ai); }
        static Action* pet(PlayerbotAI* ai) { return new SetPetAction(ai); }

        static Action* focus_heal_targets(PlayerbotAI* ai) { return new SetFocusHealTargetsAction(ai); }
        static Action* follow_target(PlayerbotAI* ai) { return new SetFollowTargetAction(ai); }
        static Action* boost_targets(PlayerbotAI* ai) { return new SetBoostTargetsAction(ai); }
        static Action* revive_targets(PlayerbotAI* ai) { return new SetReviveTargetsAction(ai); }
        static Action* buff_targets(PlayerbotAI* ai) { return new SetBuffTargetsAction(ai); }

        static Action* guild_invite(PlayerbotAI* ai) { return new GuildInviteAction(ai); }
        static Action* guild_join(PlayerbotAI* ai) { return new GuildJoinAction(ai); }
        static Action* guild_promote(PlayerbotAI* ai) { return new GuildPromoteAction(ai); }
        static Action* guild_demote(PlayerbotAI* ai) { return new GuildDemoteAction(ai); }
        static Action* guild_remove(PlayerbotAI* ai) { return new GuildRemoveAction(ai); }
        static Action* guild_leave(PlayerbotAI* ai) { return new GuildLeaveAction(ai); }
        static Action* guild_leader(PlayerbotAI* ai) { return new GuildLeaderAction(ai); }
        static Action* bg_free(PlayerbotAI* ai) { return new BGLeaveAction(ai); }
        static Action* move_style(PlayerbotAI* ai) { return new MoveStyleAction(ai); }
        static Action* jump(PlayerbotAI* ai) { return new JumpAction(ai); }
    };
};
