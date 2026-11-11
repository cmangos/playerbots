
#include "playerbot/playerbot.h"
#include "RtiAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/RtiTargetValue.h"
#include "playerbot/strategy/values/RtiValue.h"

using namespace ai;

bool RtiAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    std::string type = "rti";
    if (text.find("cc ") == 0)
    {
        type = "rti cc";
        text = text.substr(3);
    }
    if (text.empty() || text == "?" || text == "cc ?")
    {
        std::ostringstream outRti; outRti << "rti" << ": ";
        AppendRti(outRti, "rti");
        ai->TellPlayer(requester, outRti);

        std::ostringstream outRtiCc; outRtiCc << "rti cc" << ": ";
        AppendRti(outRtiCc, "rti cc");
        ai->TellPlayer(requester, outRtiCc);
        return true;
    }

    context->GetValue<std::string>(type)->Set(text);
    std::ostringstream out; out << type << " set to: ";
    AppendRti(out, type);
    ai->TellPlayer(requester, out);
    return true;
}

void RtiAction::AppendRti(std::ostringstream & out, std::string type)
{
    out << AI_VALUE(std::string, type);

    std::ostringstream n; n << type << " target";
    Unit* target = ai->GetUnit(AI_VALUE(ObjectGuid, n.str()));
    if (target)
        out << " (" << target->GetName() << ")";
}

bool CombatOrderTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    const size_t first = text.find_first_not_of(' ');
    text = first == std::string::npos ? "" : text.substr(first);
    const size_t last = text.find_last_not_of(' ');
    text = last == std::string::npos ? "" : text.substr(0, last + 1);

    std::string current = AI_VALUE(std::string, "combat order target");

    if (text.empty() || text == "?")
    {
        ai->TellPlayer(requester, current.empty() ? "No named combat order." : "Combat order: " + current);
        return true;
    }

    std::string lower = text;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    if (lower == "clear" || lower == "none" || lower == "reset")
    {
        SET_AI_VALUE(std::string, "combat order target", "");
        if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
            sPlayerbotDbStore.Save(ai);
        ai->TellPlayer(requester, "Named combat order cleared.");
        return true;
    }

    const size_t space = text.find(' ');
    const std::string mode = space == std::string::npos ? lower : lower.substr(0, space);
    const std::string name = space == std::string::npos ? "" : text.substr(space + 1);
    if (mode != "assist" && mode != "protect")
        return false;

    if (name.empty())
    {
        ai->TellPlayer(requester, "Usage: order assist <name>, order protect <name>, order clear");
        return false;
    }

    Player* member = CombatOrderTargetValue::FindGroupMember(bot, name);
    if (!member)
    {
        ai->TellPlayer(requester, name + " is not in my group.");
        return false;
    }

    if (member == bot)
    {
        ai->TellPlayer(requester, "I cannot " + mode + " myself.");
        return false;
    }

    SET_AI_VALUE(std::string, "combat order target", mode + ":" + member->GetName());

    if (!ai->HasStrategy("tank assist", BotState::BOT_STATE_COMBAT) && !ai->HasStrategy("dps assist", BotState::BOT_STATE_COMBAT) &&
        (mode == "assist" || !ai->ContainsStrategy(STRATEGY_TYPE_HEAL)))
        ai->ChangeStrategy("+dps assist", BotState::BOT_STATE_COMBAT);

    if (!sPlayerbotAIConfig.bExplicitDbStoreSave)
        sPlayerbotDbStore.Save(ai);

    ai->TellPlayer(requester, std::string(mode == "assist" ? "I will assist " : "I will protect ") + member->GetName() + ".");
    return true;
}

bool MarkRtiAction::Execute(Event& event)
{
    Group *group = bot->GetGroup();
    if (!group) return false;

    if (bot->InBattleGround())
        return false;

    Unit* target = NULL;
    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit)
            continue;

        // do not mark players
        if (unit->IsPlayer())
            continue;

        bool marked = false;
        for (int i = 0; i < 8; i++)
        {
            ObjectGuid guid = group->GetTargetIcon(i);
            if (guid == unit->GetObjectGuid())
            {
                marked = true;
                break;
            }
        }

        if (marked) continue;

        if (!target || (int)target->GetHealth() > (int)unit->GetHealth()) target = unit;
    }

    if (!target) return false;

    std::string rti = AI_VALUE(std::string, "rti");

    // Add the default rti if the bot is setup to ignore rti targets
    if (rti == "none")
    {
        rti = "skull";
    }

    int index = RtiTargetValue::GetRtiIndex(rti);
#ifndef MANGOSBOT_TWO
    group->SetTargetIcon(index, target->GetObjectGuid());
#else
    group->SetTargetIcon(index, bot->GetObjectGuid(), target->GetObjectGuid());
#endif
    return true;
}

