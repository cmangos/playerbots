
#include "playerbot/playerbot.h"
#include "AddLootAction.h"

#include "playerbot/LootObjectStack.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

using namespace ai;

bool AddLootAction::Execute(Event& event)
{
    ObjectGuid guid = event.getObject();
    if (!guid)
        return false;

    return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
}

bool AddAllLootAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();
    bool added = false;

    std::list<ObjectGuid> gos;

    if (param.empty())
    {
       gos = context->GetValue<std::list<ObjectGuid> >("nearest game objects no los")->Get();
    }
    else
    {
       gos = chat->queryGameobjects(requester, context, ai, param);
    }
     
    for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); i++)
        added |= AddLoot(requester, *i);

    std::list<ObjectGuid> corpses = context->GetValue<std::list<ObjectGuid> >("nearest corpses")->Get();
    for (std::list<ObjectGuid>::iterator i = corpses.begin(); i != corpses.end(); i++)
        added |= AddLoot(requester, *i);

    return added;
}

bool AddLootAction::isUseful()
{
    return true;
}

bool AddAllLootAction::isUseful()
{
    return true;
}

bool AddAllLootAction::AddLoot(Player* requester, ObjectGuid guid)
{
    LootObject loot(bot, guid);

    WorldObject* wo = loot.GetWorldObject(bot);
    if (loot.IsEmpty() || !wo)
        return false;

    if (abs(wo->GetPositionZ() - bot->GetPositionZ()) > INTERACTION_DISTANCE)
        return false;

    GameObject* obj = requester->GetMap()->GetGameObject(guid);

    if (obj && obj->GetGoType() == GAMEOBJECT_TYPE_TRAP)
    {
       return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
    }

    if (ai->HasRealPlayerMaster())
    {
       bool inDungeon = false;
       if (requester->IsInWorld() &&
          requester->GetMap()->IsDungeon() &&
          bot->GetMapId() == requester->GetMapId())
          inDungeon = true;

       if (inDungeon && sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(requester, wo), sPlayerbotAIConfig.lootDistance))
          return false;

       if (Group* group = bot->GetGroup())
       {
          if (group->GetLootMethod() == LootMethod::MASTER_LOOT && group->GetMasterLooterGuid() && group->GetMasterLooterGuid() != bot->GetObjectGuid())
             return false;
       }
    }

    return AI_VALUE(LootObjectStack*, "available loot")->Add(guid);
}

bool AddGatheringLootAction::AddLoot(Player* requester, ObjectGuid guid)
{
    LootObject loot(bot, guid);

    WorldObject *wo = loot.GetWorldObject(bot);
    if (loot.IsEmpty() || !wo)
        return false;

    if (!sServerFacade.IsWithinLOSInMap(bot, wo))
        return false;

    if (loot.skillId == SKILL_NONE)
        return false;

    if (!loot.IsLootPossible(bot))
        return false;

    if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(bot, wo), INTERACTION_DISTANCE))
    {
        std::list<Unit*> targets;
        MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, sPlayerbotAIConfig.lootDistance);
        MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
        Cell::VisitAllObjects(wo, searcher, sPlayerbotAIConfig.spellDistance * 1.5);
        if (!targets.empty())
        {
            std::ostringstream out;
            out << "Kill that " << targets.front()->GetName() << " so I can loot freely";
            ai->TellError(requester, out.str());
            return false;
        }
    }

    return AddAllLootAction::AddLoot(requester, guid);
}
