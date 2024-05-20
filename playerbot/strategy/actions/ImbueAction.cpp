#include "playerbot/playerbot.h"
#include "ImbueAction.h"
#include "playerbot/PlayerbotAI.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool ImbueWithStoneAction::Execute(Event& event)
{
    Player* requester = event.getOwner();
    if (bot->IsInCombat())
        return false;

    // remove stealth
    if (bot->HasAura(SPELL_AURA_MOD_STEALTH))
        bot->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

    // hp check
    if (bot->getStandState() != UNIT_STAND_STATE_STAND)
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    // Search and apply stone to weapons
    // Mainhand
    Item* mainWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (mainWeapon && mainWeapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        Item* stone = ai->FindStoneFor(mainWeapon);
        if (stone)
        {
            return UseItem(requester, stone->GetEntry(), mainWeapon);
        }
    }

    // Offhand
    Item* secondaryWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (secondaryWeapon && secondaryWeapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        Item* stone = ai->FindStoneFor(secondaryWeapon);
        if (stone)
        {
            return UseItem(requester, stone->GetEntry(), secondaryWeapon);
        }
    }

    return false;
}

bool ImbueWithStoneAction::isUseful()
{
    // Search and apply stone to weapons
    // Mainhand
    Item* mainWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (mainWeapon && mainWeapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        if (ai->FindStoneFor(mainWeapon))
        {
            return true;
        }
    }

    // Offhand
    Item* secondaryWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (secondaryWeapon && secondaryWeapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        if (ai->FindStoneFor(secondaryWeapon))
        {
            return true;
        }
    }

    return false;
}

bool ImbueWithOilAction::Execute(Event& event)
{
    Player* requester = event.getOwner();
    if (bot->IsInCombat())
        return false;

    // remove stealth
    if (bot->HasAura(SPELL_AURA_MOD_STEALTH))
        bot->RemoveSpellsCausingAura(SPELL_AURA_MOD_STEALTH);

    // hp check
    if (bot->getStandState() != UNIT_STAND_STATE_STAND)
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    // Search and apply oil to weapons
    Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (weapon && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        Item* oil = ai->FindOilFor(weapon);
        if (oil)
        {
            return UseItem(requester, oil->GetEntry(), weapon);
        }
    }

    return false;
}

bool ImbueWithOilAction::isUseful()
{
    Item* weapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (weapon && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        if (ai->FindOilFor(weapon))
        {
            return true;
        }
    }

    return false;
}

bool TryEmergencyAction::Execute(Event& event)
{
    // Do not use consumable if bot can heal self
    if (ai->IsHeal(bot) && ai->GetManaPercent() > 20)
        return false;

    // If bot does not have aggro: use bandage instead of potion/stone/crystal
    if (!(AI_VALUE(uint8, "my attacker count") >= 1) && !bot->HasAura(11196)) // Recently bandaged
    {
        Item* bandage = ai->FindBandage();
        if (bandage)
        {
            ai->ImbueItem(bandage, bot);
            SetDuration(sPlayerbotAIConfig.globalCoolDown);
            return true;
        }
    }

    // Else loop over the list of health consumable to pick one
    Item* healthItem = nullptr;
    for (uint8 i = 0; i < countof(uPriorizedHealingItemIds); ++i)
    {
        healthItem = ai->FindConsumable(uPriorizedHealingItemIds[i]);
        if (healthItem)
        {
            ai->ImbueItem(healthItem);
            return true;
        }
    }

    return false;
}
