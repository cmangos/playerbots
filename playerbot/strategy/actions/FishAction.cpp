
#include "playerbot/playerbot.h"
#include "FishAction.h"
#include "playerbot/TravelMgr.h"
#include "TellLosAction.h"

using namespace ai;

bool MoveToFishAction::isUseful()
{
    if (qualifier == "travel")
    {
        TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

        if (target->GetStatus() != TravelStatus::TRAVEL_STATUS_WORK)
            return false;

        if (target->GetDestination()->GetPurpose() != TravelDestinationPurpose::GatherFishing)
            return false;
    }

    return true;
}

bool MoveToFishAction::Execute(Event& event)
{    
    WorldPosition fishSpot;
    
    if (qualifier == "travel")
    {
        fishSpot = *AI_VALUE(TravelTarget*, "travel target")->GetPosition();
        SET_AI_VALUE2(WorldPosition, "custom position", "fish spot", fishSpot);
    }
    else
    {
        fishSpot = AI_VALUE2(WorldPosition, "custom position", "fish spot");

        if (!fishSpot)
        {
            fishSpot = *sTravelMgr.GetFishSpot(bot);
            SET_AI_VALUE2(WorldPosition, "custom position", "fish spot", fishSpot);
        }
    }
   
    if (fishSpot.distance(bot) < 1.0f)
        return false;

    return MoveTo(fishSpot);
}

bool FishAction::isUseful()
{
    if (qualifier == "travel")
    {
        TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

        if (target->GetStatus() != TravelStatus::TRAVEL_STATUS_WORK)
            return false;

        if (target->GetDestination()->GetPurpose() != TravelDestinationPurpose::GatherFishing)
            return false;
    }

    WorldPosition fishSpot = AI_VALUE2(WorldPosition, "custom position", "fish spot");

    if (!fishSpot)
        return false;

    if (!AI_VALUE(bool, "can fish"))
        return false;

    if (fishSpot.distance(bot) > 1.0f)
        return false;

    return true;
}

bool FishAction::Execute(Event& event)
{
    if (bot->IsMoving())
    {
        ai->StopMoving();
        SetDuration(100);
        return true;
    }

    //WorldPosition* fishSpot = travelTarget->GetPosition();
    WorldPosition* fishSpot = sTravelMgr.GetFishSpot(bot);

    if (abs(fishSpot->getO() - bot->GetOrientation()) > 0.5)
    {
        bot->SetFacingTo(fishSpot->getO());
        SetDuration(100);
        return true;
    }

    ai->StopMoving();

    std::list<Item*> poles = AI_VALUE2(std::list<Item*>, "inventory items", "fishing pole");

    Item* pole = poles.front();
    uint8 bagIndex = pole->GetBagSlot();
    uint8 slot = pole->GetSlot();

    WorldPacket packet(CMSG_AUTOEQUIP_ITEM, 2);
    packet << bagIndex << slot;
    bot->GetSession()->HandleAutoEquipItemOpcode(packet);

    Event fishCastEvent = Event("fish", "7731 " + chat->formatWorldobject(bot));
    bool didCast = CastCustomSpellAction::Execute(fishCastEvent);

    SetDuration(sPlayerbotAIConfig.globalCoolDown);

    return didCast;
}

bool UseFishingBobberAction::Execute(Event& event)
{
    std::list<GameObject*> objects = TellLosAction::GoGuidListToObjList(ai, AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los"));

    for (auto& obj : objects)
    {
        if (obj->GetEntry() != 35591)
            continue;

        if (obj->GetOwnerGuid() != bot->GetObjectGuid())
            continue;

        if (obj->GetLootState() != GO_READY)
        {
            time_t bobberActiveTime = obj->GetRespawnTime() - FISHING_BOBBER_READY_TIME;
            if (bobberActiveTime > time(0))
                SetDuration((bobberActiveTime - time(0)) * IN_MILLISECONDS + 500);
            else
                SetDuration(1000);
            return true;
        }

        std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
        *packet << obj->GetObjectGuid();
        bot->GetSession()->QueuePacket(std::move(packet));

        std::ostringstream out; out << "Opening " << chat->formatGameobject(obj);
        ai->TellPlayerNoFacing(ai->GetMaster(), out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        SetDuration(3000);

        if (!urand(0,10))
        {
            RESET_AI_VALUE2(WorldPosition, "custom position", "fish spot");
        }

        return true;
    }

    return false;
}