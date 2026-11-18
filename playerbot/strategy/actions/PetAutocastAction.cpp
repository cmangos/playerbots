#include "playerbot/playerbot.h"
#include "PetAutocastAction.h"
#include "playerbot/strategy/triggers/NewPetTrigger.h"

#include "playerbot/PlayerbotAIConfig.h"

#include <string>
#include <unordered_set>
#include <vector>

using namespace ai;

namespace
{
    // Matched by name, so every rank in every expansion is covered: Cower, Prowl, Spell Lock, Devour Magic.
    const std::unordered_set<std::string> disabledAutocastSpells =
    {
        "Cower", "Prowl", "Spell Lock", "Devour Magic"
    };

    bool IsDisabledAutocastSpell(uint32 spellId)
    {
        const SpellEntry* spell = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
        return spell && spell->SpellName[0] && disabledAutocastSpells.count(spell->SpellName[0]);
    }
}

bool PetAutocastAction::isUseful()
{
    const Pet* pet = bot->GetPet();
    return pet && pet->IsAlive();
}

bool PetAutocastAction::Execute(Event& event)
{
    Pet* pet = bot->GetPet();
    if (!pet)
        return false;

    // ToggleAutocast changes m_autospells, not m_spells, but copy the ids to keep the loop independent of it.
    std::vector<uint32> spellIds;
    for (const auto& itr : pet->m_spells)
    {
        if (itr.second.state == PETSPELL_REMOVED || IsPassiveSpell(itr.first) || !IsAutocastable(itr.first))
            continue;

        spellIds.push_back(itr.first);
    }

    for (const uint32 spellId : spellIds)
        pet->ToggleAutocast(spellId, !IsDisabledAutocastSpell(spellId));

    // Stops "new pet" until the pet changes again.
    SET_AI_VALUE2(std::string, "manual string", "pet autocast applied", NewPetTrigger::PetKey(pet));
    return !spellIds.empty();
}
