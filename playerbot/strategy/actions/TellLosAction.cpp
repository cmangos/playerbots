
#include "playerbot/playerbot.h"
#include "TellLosAction.h"

using namespace ai;

bool TellLosAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    string param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits(requester, "--- Targets ---", *context->GetValue<list<ObjectGuid> >("possible targets"));
        ListUnits(requester, "--- Targets (All) ---", *context->GetValue<list<ObjectGuid> >("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits(requester, "--- NPCs ---", *context->GetValue<list<ObjectGuid> >("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits(requester, "--- Corpses ---", *context->GetValue<list<ObjectGuid> >("nearest corpses"));
    }

    if (param.empty() || param == "gos" || param == "game objects")
    {
        ListGameObjects(requester, "--- Game objects ---", *context->GetValue<list<ObjectGuid> >("nearest game objects"));
    }

    if (param.empty() || param == "players")
    {
        ListUnits(requester, "--- Friendly players ---", *context->GetValue<list<ObjectGuid> >("nearest friendly players"));
    }

    return true;
}

void TellLosAction::ListUnits(Player* requester, string title, list<ObjectGuid> units)
{
    ai->TellPlayer(requester, title);

    for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit)
            ai->TellPlayer(requester, unit->GetName());
    }

}
void TellLosAction::ListGameObjects(Player* requester, string title, list<ObjectGuid> gos)
{
    ai->TellPlayer(requester, title);

    for (list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
    {
        GameObject* go = ai->GetGameObject(*i);
        if (go)
            ai->TellPlayer(requester, chat->formatGameobject(go));
    }
}
