
#include "playerbot/playerbot.h"
#include "BuyFailedAction.h"


using namespace ai;

std::map<BuyResult, std::string> BuyFailedAction::messages;

bool BuyFailedAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    if (messages.empty())
    {
        messages[BuyResult::CANT_FIND_ITEM] = "Can't find the item.";
        messages[BuyResult::ITEM_ALREADY_SOLD] = "The item is already sold.";
        messages[BuyResult::NOT_ENOUGHT_MONEY] = "I don't have enough money.";
        messages[BuyResult::SELLER_DONT_LIKE_YOU] = "The seller doesn't like me.";
        messages[BuyResult::DISTANCE_TOO_FAR] = "I'm too far away from the seller.";
        messages[BuyResult::ITEM_SOLD_OUT] = "The item is already sold out.";
        messages[BuyResult::CANT_CARRY_MORE] = "I can't carry any more of this item.";
        messages[BuyResult::RANK_REQUIRE] = "I don't have the rank required for this item.";
        messages[BuyResult::REPUTATION_REQUIRE] = "I don't have enough reputation for this item.";
        messages[BuyResult::SILENT_ERROR] = "";
    }

    WorldPacket p(event.getPacket()); // 8+4+4+1
    ObjectGuid vendorguid;
    p >> vendorguid;
    uint32 vendorSlot;
    p >> vendorSlot;
    uint8 err;
    p >> err; // error msg
    p.resize(13);

    std::string msg = messages[(BuyResult)err];
    if (!msg.empty())
    {
        ai->TellError(requester, msg);
        return true;
    }

    return false;
}
