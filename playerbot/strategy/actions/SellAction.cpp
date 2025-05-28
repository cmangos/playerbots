
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

    std::string text = event.getParam();

    if (text == "*" || text.empty())
        text = "gray";

    std::list<Item*> items = ai->InventoryParseItems(text, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);

    if (event.getSource() == "rpg action")
    {
        items.sort([](Item* i, Item* j) {return i->GetProto()->SellPrice * i->GetCount() < j->GetProto()->SellPrice * j->GetCount(); }); //Sell cheapest items first.
    }

    for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        Sell(requester, *i);

        if (event.getSource() == "rpg action" && std::distance(i, items.begin()) > std::max(5, int(items.size() / 5))) //Sell 20% or 5 items at once.
            break;
    }

    return true;
}

void SellAction::Sell(Player* requester, FindItemVisitor* visitor)
{
    ai->InventoryIterateItems(visitor, IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    std::list<Item*> items = visitor->GetResult();
    for (std::list<Item*>::iterator i = items.begin(); i != items.end(); ++i)
    {
        Sell(requester, *i);
    }
}

void SellAction::Sell(Player* requester, Item* item)
{
    std::ostringstream out;
    std::list<ObjectGuid> vendors = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest npcs")->Get();

    for (std::list<ObjectGuid>::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature *pCreature = bot->GetNPCIfCanInteractWith(vendorguid,UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;     

        if (!item->GetProto()->SellPrice)
        {
            if(ai->HasActivePlayerMaster())
                out << "Unable to sell " << chat->formatItem(item);

            continue;
        }

        ObjectGuid itemguid = item->GetObjectGuid();
        uint32 count = item->GetCount();

        uint32 botMoney = bot->GetMoney();

        sPlayerbotAIConfig.logEvent(ai, "SellAction", item->GetProto()->Name1, std::to_string(item->GetProto()->ItemId));

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