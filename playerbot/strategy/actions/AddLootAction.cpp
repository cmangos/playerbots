
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
    bool added = false;

    std::list<ObjectGuid> gos = context->GetValue<std::list<ObjectGuid> >("nearest game objects no los")->Get();
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

    if (!loot.IsLootPossible(bot))
        return false;

    float lootDistanceToUse = sPlayerbotAIConfig.lootDistance;

    Group* group = bot->GetGroup();

    bool isInGroup = group ? true : false;
    bool isGroupLeader = isInGroup ? group->GetLeaderGuid() == bot->GetObjectGuid() : false;
    bool isInDungeon = bot->GetMap()->IsDungeon();

    if (isInGroup)
    {
        //if is not master looter (and loot is set to MASTER_LOOT)
        //NOTE: They are !unable to loot quests items! too if so
        if (isInDungeon
            && group->GetLootMethod() == LootMethod::MASTER_LOOT
            && group->GetMasterLooterGuid()
            && group->GetMasterLooterGuid() != bot->GetObjectGuid())
            return false;

        if (isGroupLeader)
        {
            lootDistanceToUse = sPlayerbotAIConfig.lootDistance;
        }
        else
        {
            if (ai->HasRealPlayerMaster())
            {
                lootDistanceToUse = sPlayerbotAIConfig.groupMemberLootDistanceWithRealMaster;
            }
            else
            {
                lootDistanceToUse = sPlayerbotAIConfig.groupMemberLootDistance;
            }
        }
    }
    else
    {
        lootDistanceToUse = sPlayerbotAIConfig.lootDistance;
    }

    if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(requester, wo), lootDistanceToUse))
    {
        return false;
    }

    //check hostile units after distance checks, to avoid unnecessary calculations

    if (isInGroup && !isGroupLeader)
    {
        float MOB_AGGRO_DISTANCE = 30.0f;
        std::list<Unit*> hostiles = ai->GetAllHostileNonPlayerUnitsAroundWO(wo, MOB_AGGRO_DISTANCE);

        if (hostiles.size() > 0)
        {
            std::ostringstream out;
            out << hostiles.front()->GetName() << " is blocking " << wo->GetName() << ", need to kill it or I will not loot";
            ai->TellError(requester, out.str());
            return false;
        }
    }

    uint8 freeBagSpace = AI_VALUE(uint8, "bag space");

    if (freeBagSpace < 1 && !ai->CanLootSomethingFromWO(wo))
    {
        if (ai->HasQuestItemsInWOLootList(wo))
        {
            if (freeBagSpace < 1)
            {
                ai->DestroyAllGrayItemsInBags(requester);
                //recount freeBagSpace
                freeBagSpace = AI_VALUE(uint8, "bag space");
            }

            if (freeBagSpace < 1)
            {
                ai->TellPlayer(requester, "Can not loot quest item, my bags are full", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                return false;
            }

        }

        if (freeBagSpace < 1)
        {
            ai->TellError(requester, "There is some loot but I do not have free bag space, so not looting");
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

    float gatheringDistanceToUse = sPlayerbotAIConfig.gatheringDistance;

    Group* group = bot->GetGroup();

    bool isInGroup = group ? true : false;
    bool isGroupLeader = isInGroup ? group->GetLeaderGuid() == bot->GetObjectGuid() : false;
    bool isInDungeon = bot->GetMap()->IsDungeon();

    if (isInGroup && !isGroupLeader)
    {
        if (ai->HasRealPlayerMaster())
        {
            gatheringDistanceToUse = sPlayerbotAIConfig.groupMemberGatheringDistanceWithRealMaster;
        }
        else
        {
            gatheringDistanceToUse = sPlayerbotAIConfig.groupMemberGatheringDistance;
        }
    }
    else if (isInGroup && isGroupLeader)
    {
        gatheringDistanceToUse = sPlayerbotAIConfig.gatheringDistance;
    }
    else
    {
        gatheringDistanceToUse = sPlayerbotAIConfig.gatheringDistance;
    }

    if (sServerFacade.IsDistanceGreaterThan(sServerFacade.GetDistance2d(requester, wo), gatheringDistanceToUse))
    {
        return false;
    }

    //check hostile units after distance checks, to avoid unnecessary calculations

    float MOB_AGGRO_DISTANCE = 30.0f;
    std::list<Unit*> hostiles = ai->GetAllHostileNonPlayerUnitsAroundWO(wo, MOB_AGGRO_DISTANCE);
    std::list<Unit*> strongHostiles;
    for (auto hostile : hostiles)
    {
        if (!(bot->GetLevel() > hostile->GetLevel() + 7))
        {
            strongHostiles.push_back(hostile);
        }
    }

    if (isInGroup && !isGroupLeader)
    {
        if (hostiles.size() > 0)
        {
            std::ostringstream out;
            out << hostiles.front()->GetName() << " is blocking " << wo->GetName() << ", need to kill it or I will not gather";
            ai->TellError(requester, out.str());
            return false;
        }
    }
    else
    {
        if (strongHostiles.size() > 1)
        {
            std::ostringstream out;
            out << strongHostiles.front()->GetName() << " is blocking " << wo->GetName() << ", need to kill it or I will not gather";
            ai->TellError(requester, out.str());
            return false;
        }
    }

    uint8 freeBagSpace = AI_VALUE(uint8, "bag space");

    if (freeBagSpace < 1 && !ai->CanLootSomethingFromWO(wo))
    {
        if (ai->HasQuestItemsInWOLootList(wo))
        {
            if (freeBagSpace < 1)
            {
                ai->DestroyAllGrayItemsInBags(requester);
                //recount freeBagSpace
                freeBagSpace = AI_VALUE(uint8, "bag space");
            }

            if (freeBagSpace < 1)
            {
                ai->TellPlayer(requester, "Can not loot quest item, my bags are full", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                return false;
            }

        }

        if (freeBagSpace < 1)
        {
            ai->TellError(requester, "There is some loot but I do not have free bag space, so not looting");
            return false;
        }
    }

    return AddAllLootAction::AddLoot(requester, guid);
}
