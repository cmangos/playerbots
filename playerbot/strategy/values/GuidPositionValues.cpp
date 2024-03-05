#include "GuidPositionValues.h"
#include "playerbot/TravelMgr.h"
#include "NearestGameObjects.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

std::list<GuidPosition> GameObjectsValue::Calculate()
{
    std::list<GameObject*> targets;

    AnyGameObjectInObjectRangeCheck u_check(bot, sPlayerbotAIConfig.reactDistance);
    GameObjectListSearcher<AnyGameObjectInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects((const WorldObject*)bot, searcher, sPlayerbotAIConfig.reactDistance);

    std::list<GuidPosition> result;
    for (auto& target : targets)
        result.push_back(target);

    return result;
}

std::list<GuidPosition> EntryFilterValue::Calculate()
{
    std::vector<std::string> pair = getMultiQualifiers(getQualifier(), ",");

    std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, pair[0]);
    std::vector<std::string> entryList = getMultiQualifiers(AI_VALUE(std::string ,pair[1]), ",");

    std::list<GuidPosition> result;
    
    for (auto guid : guidList)
    {
        for (auto entry : entryList)
            if (guid.GetEntry() == stoi(entry))
                result.push_back(guid);
    }

    return result;
}

std::list<GuidPosition> RangeFilterValue::Calculate()
{
    std::vector<std::string> pair = getMultiQualifiers(getQualifier(), ",");

    std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, pair[0]);
    float range = stof(pair[1]);

    std::list<GuidPosition> result;

    for (auto guid : guidList)
    {
        if(guid.sqDistance(bot) <= range*range)
            result.push_back(guid);
    }

    return result;
}

std::list<GuidPosition> GoUsableFilterValue::Calculate()
{
    std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, getQualifier());

    std::list<GuidPosition> result;

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

std::list<GuidPosition> GoTrappedFilterValue::Calculate()
{
    std::list<GuidPosition> guidList = AI_VALUE(std::list<GuidPosition>, getQualifier());

    std::list<GuidPosition> result;

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





