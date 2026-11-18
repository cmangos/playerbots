
#include "playerbot/playerbot.h"
#include "SaveManaStrategy.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/actions/GenericSpellActions.h"

using namespace ai;

HealerAutoSaveManaMultiplier::HealerAutoSaveManaMultiplier(PlayerbotAI* ai) : Multiplier(ai, "healer save mana")
{
    manaThreshold = sPlayerbotAIConfig.saveManaThreshold;
}

// Ported from AzerothCore mod-playerbots d750b4b3, src/Ai/Base/Strategy/ConserveManaStrategy.cpp (HealerAutoSaveManaMultiplier).
float HealerAutoSaveManaMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    uint8 mana = AI_VALUE2(uint8, "mana", "self target");
    if (mana > manaThreshold)
        return 1.0f;

    CastHealingSpellAction* healingAction = dynamic_cast<CastHealingSpellAction*>(action);
    if (!healingAction)
    {
        // Not in mod-playerbots: the mana a skipped heal saves must not go into damage, so mana-costing damage spells stop too.
        // The wand costs no mana and keeps going; interrupts such as Earth Shock stay.
        CastSpellAction* spellAction = dynamic_cast<CastSpellAction*>(action);
        const SpellEntry* spell = spellAction ? sServerFacade.LookupSpellInfo(spellAction->GetSpellID()) : nullptr;
        if (spell && spell->powerType == POWER_MANA && (spell->manaCost || spell->ManaCostPercentage) &&
            !IsSpellHaveEffect(spell, SPELL_EFFECT_INTERRUPT_CAST) &&
            (IsSpellHaveEffect(spell, SPELL_EFFECT_SCHOOL_DAMAGE) || IsSpellHaveAura(spell, SPELL_AURA_PERIODIC_DAMAGE)))
            return 0.0f;

        return 1.0f;
    }

    Unit* target = action->GetTarget();
    if (!target)
        return 1.0f;

    Player* targetPlayer = target->IsPlayer() ? (Player*)target : nullptr;
    bool isTank = targetPlayer ? PlayerbotAI::IsTank(targetPlayer) : false;
    uint8 health = (uint8)target->GetHealthPercent();
    HealingManaEfficiency manaEfficiency = healingAction->manaEfficiency;
    float estAmount = healingAction->estAmount;
    float lossAmount = 100 - health;

    if (isTank)
    {
        estAmount /= 1.5f;  // tanks have more health
        if (health >= sPlayerbotAIConfig.mediumHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::MEDIUM))
            return 0.0f;
        if (health >= sPlayerbotAIConfig.lowHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::LOW))
            return 0.0f;
    }
    else
    {
        if (health >= sPlayerbotAIConfig.mediumHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::MEDIUM))
            return 0.0f;
        // Not in mod-playerbots, which skips here at any health: a non-tank at critical health gets the heal.
        if (health >= sPlayerbotAIConfig.criticalHealth &&
            (lossAmount < estAmount || manaEfficiency <= HealingManaEfficiency::LOW))
            return 0.0f;
    }

    return 1.0f;
}

void SaveManaStrategy::InitCombatMultipliers(std::list<Multiplier*> &multipliers)
{
    multipliers.push_back(new HealerAutoSaveManaMultiplier(ai));
}
