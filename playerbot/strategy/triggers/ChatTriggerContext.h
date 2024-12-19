#pragma once

#include "ChatCommandTrigger.h"

namespace ai
{
    class ChatTriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        ChatTriggerContext()
        {
            creators["quests"] = &ChatTriggerContext::quests;
            creators["quest reward"] = &ChatTriggerContext::quest_reward;
            creators["stats"] = &ChatTriggerContext::stats;
            creators["leave"] = &ChatTriggerContext::leave;
            creators["rep"] = &ChatTriggerContext::reputation;
            creators["reputation"] = &ChatTriggerContext::reputation;
            creators["log"] = &ChatTriggerContext::log;
            creators["los"] = &ChatTriggerContext::los;
            creators["drop"] = &ChatTriggerContext::drop;
            creators["roll"] = &ChatTriggerContext::roll;
            creators["share"] = &ChatTriggerContext::share;
            creators["q"] = &ChatTriggerContext::q;
            creators["ll"] = &ChatTriggerContext::ll;
            creators["ss"] = &ChatTriggerContext::ss;
            creators["loot"] = &ChatTriggerContext::loot_all;
            creators["add all loot"] = &ChatTriggerContext::loot_all;
            creators["release"] = &ChatTriggerContext::release;
            creators["teleport"] = &ChatTriggerContext::teleport;
            creators["taxi"] = &ChatTriggerContext::taxi;
            creators["repair"] = &ChatTriggerContext::repair;
            creators["u"] = &ChatTriggerContext::use;
            creators["use"] = &ChatTriggerContext::use;
            creators["c"] = &ChatTriggerContext::item_count;
            creators["items"] = &ChatTriggerContext::item_count;
            creators["inventory"] = &ChatTriggerContext::item_count;
            creators["inv"] = &ChatTriggerContext::item_count;
            creators["e"] = &ChatTriggerContext::equip;
            creators["equip"] = &ChatTriggerContext::equip;
            creators["keep"] = &ChatTriggerContext::keep;
            creators["ue"] = &ChatTriggerContext::uneqip;
            creators["s"] = &ChatTriggerContext::sell;
            creators["b"] = &ChatTriggerContext::buy;
            creators["bb"] = &ChatTriggerContext::buy_back;
            creators["r"] = &ChatTriggerContext::reward;
            creators["t"] = &ChatTriggerContext::trade;
            creators["nt"] = &ChatTriggerContext::nontrade;
            creators["talents"] = &ChatTriggerContext::talents;
            creators["spells"] = &ChatTriggerContext::spells;
            creators["co"] = &ChatTriggerContext::co;
            creators["nc"] = &ChatTriggerContext::nc;
            creators["de"] = &ChatTriggerContext::dead;
            creators["react"] = &ChatTriggerContext::react;
            creators["all"] = &ChatTriggerContext::all;
            creators["trainer"] = &ChatTriggerContext::trainer;
            creators["attack"] = &ChatTriggerContext::attack;
            creators["attack rti"] = &ChatTriggerContext::attack_rti;
            creators["pull"] = &ChatTriggerContext::pull;
            creators["pull rti"] = &ChatTriggerContext::pull_rti;
            creators["chat"] = &ChatTriggerContext::chat;
            creators["accept"] = &ChatTriggerContext::accept;
            creators["home"] = &ChatTriggerContext::home;
            creators["load ai"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "load ai"); };
            creators["list ai"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "list ai"); };
            creators["save ai"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "save ai"); };
            creators["reset ai"] = &ChatTriggerContext::reset_ai;
            creators["reset strats"] = &ChatTriggerContext::reset_strats;
            creators["reset values"] = &ChatTriggerContext::reset_values;
            creators["destroy"] = &ChatTriggerContext::destroy;
            creators["emote"] = &ChatTriggerContext::emote;
            creators["buff"] = &ChatTriggerContext::buff;
            creators["help"] = &ChatTriggerContext::help;
            creators["gb"] = &ChatTriggerContext::gb;
            creators["gbank"] = &ChatTriggerContext::gb;
            creators["bank"] = &ChatTriggerContext::bank;
            creators["follow"] = &ChatTriggerContext::follow;
            creators["stay"] = &ChatTriggerContext::stay;
            creators["guard"] = &ChatTriggerContext::guard;
            creators["free"] = &ChatTriggerContext::free;
            creators["wait for attack time"] = &ChatTriggerContext::wait_for_attack_time;
            creators["pet"] = &ChatTriggerContext::pet;
            creators["focus heal"] = &ChatTriggerContext::focus_heal_target;
            creators["follow target"] = &ChatTriggerContext::follow_target;
            creators["boost target"] = &ChatTriggerContext::boost_target;
            creators["buff target"] = &ChatTriggerContext::buff_target;
            creators["revive target"] = &ChatTriggerContext::revive_target;
            creators["self res"] = &ChatTriggerContext::self_resurrect;
            creators["flee"] = &ChatTriggerContext::flee;
            creators["grind"] = &ChatTriggerContext::grind;
            creators["tank attack"] = &ChatTriggerContext::tank_attack;
            creators["talk"] = &ChatTriggerContext::talk;
            creators["cast"] = &ChatTriggerContext::cast;
            creators["castnc"] = &ChatTriggerContext::castnc;
            creators["invite"] = &ChatTriggerContext::invite;
            creators["join"] = &ChatTriggerContext::join;
            creators["lfg"] = &ChatTriggerContext::lfg;
            creators["spell"] = &ChatTriggerContext::spell;
            creators["rti"] = &ChatTriggerContext::rti;
            creators["revive"] = &ChatTriggerContext::revive;
            creators["runaway"] = &ChatTriggerContext::runaway;
            creators["warning"] = &ChatTriggerContext::warning;
            creators["position"] = &ChatTriggerContext::position;
            creators["summon"] = &ChatTriggerContext::summon;
            creators["who"] = &ChatTriggerContext::who;
            creators["save mana"] = &ChatTriggerContext::save_mana;
            creators["max dps"] = &ChatTriggerContext::max_dps;
            creators["possible attack targets"] = &ChatTriggerContext::possible_attack_targets;
            creators["attackers"] = &ChatTriggerContext::attackers;
            creators["formation"] = &ChatTriggerContext::formation;
            creators["stance"] = &ChatTriggerContext::stance;
            creators["sendmail"] = &ChatTriggerContext::sendmail;
            creators["mail"] = &ChatTriggerContext::mail;
            creators["outfit"] = &ChatTriggerContext::outfit;
            creators["go"] = &ChatTriggerContext::go;
            creators["ready"] = &ChatTriggerContext::ready_check;
            creators["debug"] = &ChatTriggerContext::debug;
            creators["cdebug"] = &ChatTriggerContext::cdebug;
            creators["cs"] = &ChatTriggerContext::cs;
            creators["wts"] = &ChatTriggerContext::wts;
            creators["hire"] = &ChatTriggerContext::hire;
            creators["craft"] = &ChatTriggerContext::craft;
            creators["flag"] = &ChatTriggerContext::craft;
            creators["range"] = &ChatTriggerContext::range;
            creators["ra"] = &ChatTriggerContext::ra;
            creators["give leader"] = &ChatTriggerContext::give_leader;
            creators["cheat"] = &ChatTriggerContext::cheat;
            creators["rtsc"] = &ChatTriggerContext::rtsc;
            creators["ah"] = &ChatTriggerContext::ah;
            creators["ah bid"] = &ChatTriggerContext::ah_bid;
            creators["guild invite"] = &ChatTriggerContext::guild_invite;
            creators["guild join"] = &ChatTriggerContext::guild_join;
            creators["guild promote"] = &ChatTriggerContext::guild_promote;
            creators["guild demote"] = &ChatTriggerContext::guild_demote;
            creators["guild remove"] = &ChatTriggerContext::guild_remove;
            creators["guild leave"] = &ChatTriggerContext::guild_leave;
            creators["guild leader"] = &ChatTriggerContext::guild_leader;
            creators["bg free"] = &ChatTriggerContext::bg_free;
            creators["move style"] = &ChatTriggerContext::move_style;
            creators["jump"] = &ChatTriggerContext::jump;
            creators["doquest"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "doquest");  };
            creators["skill"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "skill");  };
            creators["faction"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "faction");  };
            creators["set value"] = [](PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "set value");  };
        }

    private:
        static Trigger* ra(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ra"); }
        static Trigger* range(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "range"); }
        static Trigger* flag(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "flag"); }
        static Trigger* craft(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "craft"); }
        static Trigger* hire(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "hire"); }
        static Trigger* wts(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "wts"); }
        static Trigger* cs(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "cs"); }
        static Trigger* debug(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "debug"); }
        static Trigger* cdebug(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "cdebug"); }
        static Trigger* go(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "go"); }
        static Trigger* outfit(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "outfit"); }
        static Trigger* mail(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "mail"); }
        static Trigger* sendmail(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "sendmail"); }
        static Trigger* formation(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "formation"); }
        static Trigger* stance(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "stance"); }
        static Trigger* possible_attack_targets(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "possible attack targets"); }
        static Trigger* attackers(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "attackers"); }
        static Trigger* max_dps(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "max dps"); }
        static Trigger* save_mana(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "save mana"); }
        static Trigger* who(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "who"); }
        static Trigger* summon(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "summon"); }
        static Trigger* position(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "position"); }
        static Trigger* runaway(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "runaway"); }
        static Trigger* warning(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "warning"); }
        static Trigger* revive(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "revive"); }
        static Trigger* rti(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "rti"); }
        static Trigger* invite(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "invite"); }
        static Trigger* join(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "join"); }
        static Trigger* lfg(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "lfg"); }
        static Trigger* cast(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "cast"); }
        static Trigger* castnc(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "castnc"); }
        static Trigger* talk(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "talk"); }
        static Trigger* flee(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "flee"); }
        static Trigger* grind(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "grind"); }
        static Trigger* tank_attack(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "tank attack"); }
        static Trigger* stay(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "stay"); }
        static Trigger* guard(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guard"); }
        static Trigger* free(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "free"); }
        static Trigger* wait_for_attack_time(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "wait for attack time"); }
        static Trigger* pet(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "pet"); }
        static Trigger* focus_heal_target(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "focus heal"); }
        static Trigger* follow_target(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "follow target"); }
        static Trigger* boost_target(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "boost target"); }
        static Trigger* buff_target(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "buff target"); }
        static Trigger* revive_target(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "revive target"); }
        static Trigger* self_resurrect(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "self res"); }
        static Trigger* follow(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "follow"); }
        static Trigger* gb(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "gb"); }
        static Trigger* bank(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "bank"); }
        static Trigger* help(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "help"); }
        static Trigger* buff(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "buff"); }
        static Trigger* emote(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "emote"); }
        static Trigger* destroy(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "destroy"); }
        static Trigger* home(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "home"); }
        static Trigger* accept(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "accept"); }
        static Trigger* chat(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "chat"); }
        static Trigger* attack(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "attack"); }
        static Trigger* attack_rti(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "attack rti"); }
        static Trigger* pull(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "pull"); }
        static Trigger* pull_rti(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "pull rti"); }
        static Trigger* trainer(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "trainer"); }
        static Trigger* co(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "co"); }
        static Trigger* nc(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "nc"); }
        static Trigger* dead(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "de"); }
        static Trigger* react(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "react"); }
        static Trigger* all(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "all"); }
        static Trigger* spells(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "spells"); }
        static Trigger* talents(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "talents"); }
        static Trigger* equip(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "e"); }
        static Trigger* keep(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "keep"); }
        static Trigger* uneqip(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ue"); }
        static Trigger* sell(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "s"); }
        static Trigger* buy(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "b"); }
        static Trigger* buy_back(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "bb"); }
        static Trigger* reward(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "r"); }
        static Trigger* trade(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "t"); }
        static Trigger* nontrade(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "nt"); }

        static Trigger* item_count(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "c"); }
        static Trigger* use(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "use"); }
        static Trigger* repair(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "repair"); }
        static Trigger* taxi(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "taxi"); }
        static Trigger* teleport(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "teleport"); }
        static Trigger* q(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "q"); }
        static Trigger* ll(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ll"); }
        static Trigger* ss(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ss"); }
        static Trigger* drop(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "drop"); }
        static Trigger* roll(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "roll"); }
        static Trigger* share(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "share"); }
        static Trigger* quests(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "quests"); }
        static Trigger* quest_reward(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "quest reward"); }
        static Trigger* stats(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "stats"); }
        static Trigger* leave(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "leave"); }
        static Trigger* reputation(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "reputation"); }
        static Trigger* log(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "log"); }
        static Trigger* los(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "los"); }
        static Trigger* loot_all(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "add all loot"); }
        static Trigger* release(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "release"); }
        static Trigger* reset_ai(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "reset ai"); }
        static Trigger* reset_strats(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "reset strats"); }
        static Trigger* reset_values(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "reset values"); }
        static Trigger* spell(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "spell"); }
        static Trigger* ready_check(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ready check"); }
        static Trigger* give_leader(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "give leader"); }
        static Trigger* cheat(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "cheat"); }
        static Trigger* rtsc(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "rtsc"); }
        static Trigger* guild_invite(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild invite"); }
        static Trigger* guild_join(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild join"); }
        static Trigger* ah(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ah"); }
        static Trigger* ah_bid(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "ah bid"); }
        static Trigger* guild_promote(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild promote"); }
        static Trigger* guild_demote(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild demote"); }
        static Trigger* guild_remove(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild remove"); }
        static Trigger* guild_leave(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild leave"); }
        static Trigger* guild_leader(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "guild leader"); }
        static Trigger* bg_free(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "bg free"); }
        static Trigger* move_style(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "move style"); }
        static Trigger* jump(PlayerbotAI* ai) { return new ChatCommandTrigger(ai, "jump"); }
    };
};
