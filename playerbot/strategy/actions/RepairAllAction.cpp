
#include "playerbot/playerbot.h"
#include "RepairAllAction.h"

#include "playerbot/ServerFacade.h"

using namespace ai;

bool RepairAllAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, "nearest npcs");
    for (list<ObjectGuid>::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Creature *unit = bot->GetNPCIfCanInteractWith(*i, UNIT_NPC_FLAG_REPAIR);
        if (!unit)
            continue;

#ifdef MANGOS
        if(bot->hasUnitState(UNIT_STAT_DIED))
#endif
#ifdef CMANGOS
        if (bot->hasUnitState(UNIT_STAT_FEIGN_DEATH))
#endif
            bot->RemoveSpellsCausingAura(SPELL_AURA_FEIGN_DEATH);

        sServerFacade.SetFacingTo(bot, unit);
        float discountMod = bot->GetReputationPriceDiscount(unit);

        float durability = AI_VALUE(uint8, "durability");

        uint32 botMoney = bot->GetMoney();
        if (ai->HasCheat(BotCheatMask::gold))
        {
            bot->SetMoney(10000000);
        }

        //Repair weapons first.
        uint32 totalCost = bot->DurabilityRepair(EQUIPMENT_SLOT_MAINHAND, true, discountMod
#ifndef MANGOSBOT_ZERO
            , false
#endif
        );

        totalCost += bot->DurabilityRepair(EQUIPMENT_SLOT_RANGED, true, discountMod
#ifndef MANGOSBOT_ZERO
            , false
#endif
        );

        totalCost += bot->DurabilityRepair(EQUIPMENT_SLOT_OFFHAND, true, discountMod
#ifndef MANGOSBOT_ZERO
            , false
#endif
        );

        totalCost += bot->DurabilityRepairAll(true, discountMod
#ifndef MANGOSBOT_ZERO
            , false
#endif
        );

        if (ai->HasCheat(BotCheatMask::gold))
        {
            bot->SetMoney(botMoney);
        }

        //Totalcost is bugged in core. For now we use this work-around.
        totalCost = botMoney - bot->GetMoney();

        if (totalCost > 0)
        {
            ostringstream out;
            out << "Repair: " << chat->formatMoney(totalCost) << " (" << unit->GetName() << ")";
            ai->TellPlayerNoFacing(requester, out.str(),PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            if (sPlayerbotAIConfig.globalSoundEffects)
                bot->PlayDistanceSound(1116);

            sPlayerbotAIConfig.logEvent(ai, "RepairAllAction", to_string(durability), to_string(totalCost));

            ai->DoSpecificAction("equip upgrades", event, true);
        }

        context->GetValue<uint32>("death count")->Set(0);

        return durability < 100 && AI_VALUE(uint8, "durability") > durability;
    }

    ai->TellPlayerNoFacing(requester, "Cannot find any npc to repair at");
    return false;
}
