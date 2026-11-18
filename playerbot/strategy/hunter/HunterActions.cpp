
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotFactory.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/strategy/actions/GenericActions.h"
#include "HunterActions.h"

using namespace ai;

bool CastSerpentStingAction::isUseful()
{
    return CastRangedDebuffSpellAction::isUseful() && AI_VALUE2(uint8, "health", GetTargetName()) > 50 && (!(AI_VALUE2(uint8, "mana", GetTargetName()) >= 10) || (ai->HasStrategy("sting serpent", BotState::BOT_STATE_COMBAT)));
}

bool CastViperStingAction::isUseful()
{
    return CastRangedDebuffSpellAction::isUseful() && AI_VALUE2(uint8, "mana", GetTargetName()) >= 10;
}

bool FeedPetAction::Execute(Event& event)
{
    Pet* pet = bot->GetPet();
    if (pet && pet->getPetType() == HUNTER_PET && pet->GetHappinessState() != HAPPY)
        pet->SetPower(POWER_HAPPINESS, HAPPINESS_LEVEL_SIZE * 2);

    return true;
}

bool CastAutoShotAction::isUseful()
{
    if (ai->IsInVehicle() && !ai->IsInVehicle(false, false, true))
        return false;

    if (!ai->HasStrategy("ranged", BotState::BOT_STATE_COMBAT) || ai->IsJumping() || bot->IsFalling())
        return false;

    Unit const* target = GetTarget();
    return target && (!IsAutoShooting(target) || bot->IsMoving() || !bot->IsStopped());
}

bool CastAutoShotAction::IsAutoShooting(Unit const* target) const
{
    Spell const* spell = bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL);
    return target && spell && spell->m_spellInfo->Id == GetSpellID() &&
        spell->getState() != SPELL_STATE_FINISHED && spell->m_targets.getUnitTargetGuid() == target->GetObjectGuid();
}

bool HunterEquipAmmoAction::HasAmmoEquipped(Player* bot, Item const* weapon)
{
    const uint32 ammoId = bot->GetUInt32Value(PLAYER_AMMO_ID);
    ItemPrototype const* ammo = sObjectMgr.GetItemPrototype(ammoId);
    return ammo && FindAmmoVisitor(bot, weapon->GetProto()->SubClass).Accept(ammo) && bot->HasItemCount(ammoId, 1);
}

bool HunterEquipAmmoAction::Execute(Event& event)
{
    Item const* ranged = bot->GetWeaponForAttack(RANGED_ATTACK, true, false);
    if (!ranged)
        return false;

    const auto ammoItems = ai->InventoryParseItems("ammo", IterateItemsMask::ITERATE_ITEMS_IN_BAGS);
    ItemPrototype const* bestAmmo = nullptr;
    for (Item const* item : ammoItems)
    {
        ItemPrototype const* ammo = item->GetProto();
        if (ammo->InventoryType == INVTYPE_AMMO && (!bestAmmo || ammo->ItemLevel > bestAmmo->ItemLevel))
            bestAmmo = ammo;
    }

    if (bestAmmo)
    {
        if (bestAmmo->ItemId == bot->GetUInt32Value(PLAYER_AMMO_ID))
            return false;

        bot->SetAmmo(bestAmmo->ItemId);
        return bot->GetUInt32Value(PLAYER_AMMO_ID) == bestAmmo->ItemId;
    }

    if (!ai->HasCheat(BotCheatMask::item))
        return false;

    PlayerbotFactory(bot, bot->GetLevel(), 0).InitAmmo();
    return HasAmmoEquipped(bot, ranged);
}

bool CastAutoShotAction::Execute(Event& event)
{
    if (bot->IsMoving() || !bot->IsStopped())
    {
        ai->StopMoving();
    }

    if (IsAutoShooting(GetTarget()))
        return !bot->IsMoving() && bot->IsStopped();

    return CastSpellAction::Execute(event);
}

bool CastSteadyShotAction::Execute(Event& event)
{
    if (CastSpellAction::Execute(event))
    {
        const Item* equippedWeapon = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (equippedWeapon)
        {
            SetDuration(GetDuration() + sPlayerbotAIConfig.globalCoolDown);
        }

        return true;
    }

    return false;
}
