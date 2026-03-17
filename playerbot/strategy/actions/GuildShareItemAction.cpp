#include "playerbot/playerbot.h"
#include "GuildShareItemAction.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool GuildShareItemAction::isUseful()
{
    if (!bot->GetGuildId())
        return false;

    if (bot->IsInCombat())
        return false;

    return AI_VALUE(GuildShareTarget, "guild share target").IsValid();
}

bool GuildShareItemAction::Execute(Event& event)
{
    GuildShareTarget shareTarget = AI_VALUE(GuildShareTarget, "guild share target");
    if (!shareTarget.IsValid())
        return false;

    Player* receiver = shareTarget.receiver;
    uint32 itemId = shareTarget.itemId;

    PlayerbotAI* receiverAi = receiver->GetPlayerbotAI();
    if (!receiverAi)
        return false;

    // Find the item in our bags
    std::vector<Item*> items = ai->GetInventoryItems();
    for (Item* item : items)
    {
        if (item->GetProto()->ItemId != itemId)
            continue;

        // Check if receiver can store it
        ItemPosCountVec dest;
        InventoryResult msg = receiver->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
        if (msg != EQUIP_ERR_OK)
        {
            sLog.outDetail("Bot #%d <%s> cannot give %s to %s - bags full",
                bot->GetGUIDLow(), bot->GetName(), item->GetProto()->Name1, receiver->GetName());
            return false;
        }

        // Move the item
        bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
        item->SetOwnerGuid(receiver->GetObjectGuid());
        receiver->MoveItemToInventory(dest, item, true);

        std::ostringstream out;
        out << "Sharing " << chat->formatItem(item, item->GetCount()) << " with guild member " << receiver->GetName();
        ai->TellPlayerNoFacing(GetMaster(), out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        std::ostringstream receiverOut;
        receiverOut << "Got " << chat->formatItem(item, item->GetCount()) << " from guild member " << bot->GetName();
        receiverAi->TellPlayerNoFacing(receiverAi->GetMaster(), receiverOut.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

        return true;
    }

    return false;
}