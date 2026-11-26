#pragma once
#include "playerbot/strategy/Trigger.h"
#include "Entities/Pet.h"

#include <string>

namespace ai
{
    // Fires while the bot's live pet is not the one "pet autocast" last set up: a fresh tame or a
    // resummoned demon (new guid), or a learned spell ("initialize pet", a level up), which starts with
    // its default autocast (new spell count). A higher rank needs no re-run: Pet::addSpell gives it the
    // replaced rank's autocast state and unlearns the old rank, so the count stays. It keeps firing until the action has run, so an action
    // that loses its turn or expires in the queue is queued again. The record is cleared while the bot
    // has no live pet, which lets a recalled hunter pet with the same guid fire again.
    class NewPetTrigger : public Trigger
    {
    public:
        NewPetTrigger(PlayerbotAI* ai) : Trigger(ai, "new pet", 2) {}

        // What "pet autocast" records once it has set up a pet: the pet guid and its spell count.
        static std::string PetKey(const Pet* pet)
        {
            uint32 spellCount = 0;
            for (const auto& itr : pet->m_spells)
            {
                if (itr.second.state != PETSPELL_REMOVED)
                    ++spellCount;
            }

            return std::to_string(pet->GetObjectGuid().GetCounter()) + ":" + std::to_string(spellCount);
        }

        virtual bool IsActive() override
        {
            const Pet* pet = bot->GetPet();
            if (!pet || !pet->IsAlive())
            {
                RESET_AI_VALUE2(std::string, "manual string", "pet autocast applied");
                return false;
            }

            return PetKey(pet) != AI_VALUE2(std::string, "manual string", "pet autocast applied");
        }
    };
}
