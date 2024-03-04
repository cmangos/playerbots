
#include "playerbot/playerbot.h"
#include "SellAction.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/strategy/values/ItemUsageValue.h"

using namespace ai;

class SellItemsVisitor : public IterateItemsVisitor
{
public:
    SellItemsVisitor(SellAction* action) : IterateItemsVisitor()
    {
        this->action = action;
    }

    virtual bool Visit(Item* item)
    {
        action->Sell(nullptr, item);
        return true;
    }

private:
    SellAction* action;
};

bool SellAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    string text = event.getParam();

    if (text == "*" || text.empty())
        text = "gray";

    list<Item*> items = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    for (list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        Sell(requester, *i);
    }

    return true;
}

void SellAction::Sell(Player* requester, FindItemVisitor* visitor)
{
    ai->InventoryIterateItems(visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    list<Item*> items = visitor->GetResult();
    for (list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        Sell(requester, *i);
    }
}

void SellAction::Sell(Player* requester, Item* item)
{
    ostringstream out;
    list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<list<ObjectGuid> >("nearest npcs")->Get();

    for (list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature *pCreature = bot->GetNPCIfCanInteractWith(vendorguid,UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;     

        ObjectGuid itemguid = item->GetObjectGuid();
        uint32 count = item->GetCount();

        uint32 botMoney = bot->GetMoney();

        sPlayerbotAIConfig.logEvent(ai, "SellAction", item->GetProto()->Name1, to_string(item->GetProto()->ItemId));

        WorldPacket p;
        p << vendorguid << itemguid << count;
        bot->GetSession()->HandleSellItemOpcode(p);

        if (ai->HasCheat(BotCheatMask::gold))
        {
            bot->SetMoney(botMoney);
        }

        out << "Selling " << chat->formatItem(item);
        if (sPlayerbotAIConfig.globalSoundEffects)
            bot->PlayDistanceSound(120);

        ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        break;
    }
}