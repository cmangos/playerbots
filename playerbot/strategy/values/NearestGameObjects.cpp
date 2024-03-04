
#include "playerbot/playerbot.h"
#include "NearestGameObjects.h"

#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

bool AnyGameObjectInObjectRangeCheck::operator()(GameObject* u)
{
    if (u && i_obj->IsWithinDistInMap(u, i_range) && sServerFacade.isSpawned(u) && u->GetGOInfo())
        return true;

    return false;
}

bool GameObjectsInObjectRangeCheck::operator()(GameObject* u)
{
    if (u && i_obj->IsWithinDistInMap(u, i_range) && sServerFacade.isSpawned(u) && u->GetGOInfo() && u->GetEntry() == i_gameObjectID)
        return true;

    return false;
}

class AnyDynamicObjectInObjectRangeCheck
{
public:
    AnyDynamicObjectInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
    WorldObject const& GetFocusObject() const { return *i_obj; }
    bool operator()(DynamicObject* u)
    {
        if (u && i_obj->IsWithinDistInMap(u, i_range))
            return true;

        return false;
    }

private:
    WorldObject const* i_obj;
    float i_range;
};

list<ObjectGuid> NearestGameObjects::Calculate()
{
    list<GameObject*> targets;

    if (!qualifier.empty())
    {
        uint32 gameObjectID = stoi(qualifier);
        GameObjectsInObjectRangeCheck u_check(bot, range, gameObjectID);
        GameObjectListSearcher<GameObjectsInObjectRangeCheck> searcher(targets, u_check);
        Cell::VisitAllObjects((const WorldObject*)bot, searcher, range);
    }
    else
    {
        AnyGameObjectInObjectRangeCheck u_check(bot, range);
        GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(targets, u_check);
        Cell::VisitAllObjects((const WorldObject*)bot, searcher, range);
    }

    list<ObjectGuid> result;
    for(list<GameObject*>::iterator tIter = targets.begin(); tIter != targets.end(); ++tIter)
    {
		GameObject* go = *tIter;
        if(ignoreLos || sServerFacade.IsWithinLOSInMap(bot, go))
			result.push_back(go->GetObjectGuid());
    }

    return result;
}

list<ObjectGuid> NearestDynamicObjects::Calculate()
{
    list<DynamicObject*> targets;

    // Remove this when updating wotlk core
#ifndef MANGOSBOT_TWO
    AnyDynamicObjectInObjectRangeCheck u_check(bot, range);
    MaNGOS::DynamicObjectListSearcher<AnyDynamicObjectInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects((const WorldObject*)bot, searcher, range);
#endif

    list<ObjectGuid> result;
    for (list<DynamicObject*>::iterator tIter = targets.begin(); tIter != targets.end(); ++tIter)
    {
        DynamicObject* go = *tIter;
        if (ignoreLos || sServerFacade.IsWithinLOSInMap(bot, go))
            result.push_back(go->GetObjectGuid());
    }

    return result;
}
