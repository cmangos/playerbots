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
    uint32 shareAmount = shareTarget.amount; // 0 = give all (existing behavior)

    PlayerbotAI* receiverAi = receiver->GetPlayerbotAI();
    if (!receiverAi)
        return false;

    std::vector<Item*> items = ai->GetInventoryItems();
    for (Item* item : items)
    {
        if (item->GetProto()->ItemId != itemId)
            continue;

        uint32 stackCount = item->GetCount();
        uint32 giveCount = (shareAmount > 0 && shareAmount < stackCount) ? shareAmount : stackCount;

        if (giveCount == stackCount)
        {
            ItemPosCountVec dest;
            InventoryResult msg = receiver->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
            if (msg != EQUIP_ERR_OK)
            {
                sLog.outDetail("Bot #%d <%s> cannot give %s to %s - bags full",
                    bot->GetGUIDLow(), bot->GetName(), item->GetProto()->Name1, receiver->GetName());
                return false;
            }

            bot->MoveItemFromInventory(item->GetBagSlot(), item->GetSlot(), true);
            item->SetOwnerGuid(receiver->GetObjectGuid());
            receiver->MoveItemToInventory(dest, item, true);

            std::ostringstream receiverOut;
            receiverOut << "Got " << chat->formatItem(item, giveCount) << " from guild member " << bot->GetName();
            receiverAi->TellPlayerNoFacing(receiverAi->GetMaster(), receiverOut.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }
        else
        {
            ItemPosCountVec dest;
            InventoryResult msg = receiver->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, giveCount);
            if (msg != EQUIP_ERR_OK)
            {
                sLog.outDetail("Bot #%d <%s> cannot give %s to %s - bags full",
                    bot->GetGUIDLow(), bot->GetName(), item->GetProto()->Name1, receiver->GetName());
                return false;
            }

            item->SetCount(stackCount - giveCount);
            item->SetState(ITEM_CHANGED, bot);
            bot->SaveInventoryAndGoldToDB();

            Item* newItem = Item::CreateItem(itemId, giveCount, receiver);
            if (!newItem)
                return false;

            receiver->StoreItem(dest, newItem, true);

            std::ostringstream receiverOut;
            receiverOut << "Got " << chat->formatItem(newItem, giveCount) << " from guild member " << bot->GetName();
            receiverAi->TellPlayerNoFacing(receiverAi->GetMaster(), receiverOut.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        }

        return true;
    }

    return false;
}