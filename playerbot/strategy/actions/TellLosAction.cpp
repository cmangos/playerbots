
#include "playerbot/playerbot.h"
#include "TellLosAction.h"

using namespace ai;

bool TellLosAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits(requester, "--- Targets ---", *context->GetValue<std::list<ObjectGuid> >("possible targets"));
        ListUnits(requester, "--- Targets (All) ---", *context->GetValue<std::list<ObjectGuid> >("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits(requester, "--- NPCs ---", *context->GetValue<std::list<ObjectGuid> >("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits(requester, "--- Corpses ---", *context->GetValue<std::list<ObjectGuid> >("nearest corpses"));
    }

    if (param.empty() || chat->starts_with(param, "gos") || chat->starts_with(param, "game objects"))
    {
        auto p = chat->local_split(param, '|');
        ListGameObjects(requester, "--- Game objects ---", chat->queryGameobjects(requester, context, ai, param), p);
    }

    if (param.empty() || param == "players")
    {
        ListUnits(requester, "--- Friendly players ---", *context->GetValue<std::list<ObjectGuid> >("nearest friendly players"));
    }

    return true;
}

void TellLosAction::ListUnits(Player* requester, std::string title, std::list<ObjectGuid> units)
{
    ai->TellPlayer(requester, title);

    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit)
            ai->TellPlayer(requester, unit->GetName());
    }

}
void TellLosAction::ListGameObjects(Player* requester, std::string title, const std::list<ObjectGuid>& gos, const std::vector<std::string_view>& filters)
{
    ai->TellPlayer(requester, title);

    for (std::list<ObjectGuid>::const_iterator i = gos.begin(); i != gos.end(); i++)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (go)
        {
            std::ostringstream ss;
            ss << chat->formatGameobject(go);

            if (std::find_if(filters.begin(), filters.end(), [](const std::string_view& el) { return el == "info:range";}) != filters.end())
               ss << " " << go->GetPosition().GetDistance2d(ai->GetBot()->GetPosition()) << "m";

            ai->TellPlayer(requester, ss.str());
        }
    }
}
