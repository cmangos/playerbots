#include "GuidPositionValues.h"
#include "playerbot/TravelMgr.h"
#include "NearestGameObjects.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

list<GuidPosition> GameObjectsValue::Calculate()
{
    list<GameObject*> targets;

    AnyGameObjectInObjectRangeCheck u_check(bot, sPlayerbotAIConfig.reactDistance);
    GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects((const WorldObject*)bot, searcher, sPlayerbotAIConfig.reactDistance);

    list<GuidPosition> result;
    for (auto& target : targets)
        result.push_back(target);

    return result;
}

list<GuidPosition> EntryFilterValue::Calculate()
{
    vector<string> pair = getMultiQualifiers(getQualifier(), ",");

    list<GuidPosition> guidList = AI_VALUE(list<GuidPosition>, pair[0]);
    vector<string> entryList = getMultiQualifiers(AI_VALUE(string ,pair[1]), ",");

    list<GuidPosition> result;
    
    for (auto guid : guidList)
    {
        for (auto entry : entryList)
            if (guid.GetEntry() == stoi(entry))
                result.push_back(guid);
    }

    return result;
}

list<GuidPosition> RangeFilterValue::Calculate()
{
    vector<string> pair = getMultiQualifiers(getQualifier(), ",");

    list<GuidPosition> guidList = AI_VALUE(list<GuidPosition>, pair[0]);
    float range = stof(pair[1]);

    list<GuidPosition> result;

    for (auto guid : guidList)
    {
        if(guid.sqDistance(bot) <= range*range)
            result.push_back(guid);
    }

    return result;
}

list<GuidPosition> GoUsableFilterValue::Calculate()
{
    list<GuidPosition> guidList = AI_VALUE(list<GuidPosition>, getQualifier());

    list<GuidPosition> result;

    for (auto guid : guidList)
    {
        if (guid.IsGameObject())
        {
            GameObject* go = guid.GetGameObject();
            if(go && !go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE))
                result.push_back(guid);
        }
    }

    return result;
}

list<GuidPosition> GoTrappedFilterValue::Calculate()
{
    list<GuidPosition> guidList = AI_VALUE(list<GuidPosition>, getQualifier());

    list<GuidPosition> result;

    for (auto guid : guidList)
    {
        if (guid.IsGameObject())
        {
            if (!guid.GetGameObjectInfo()->GetLinkedGameObjectEntry())
                result.push_back(guid);
            else
            {
                GameObject* go = guid.GetGameObject();
                if (go && !go->GetLinkedTrap())
                    result.push_back(guid);
            }
        }
    }

    return result;
}





