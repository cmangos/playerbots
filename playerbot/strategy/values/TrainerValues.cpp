
#include "playerbot/playerbot.h"
#include "TrainerValues.h"
#include "SharedValueContext.h"
#include "playerbot/PlayerbotHelpMgr.h"

using namespace ai;


trainableSpellMap* TrainableSpellMapValue::Calculate()
{
    trainableSpellMap* spellMap = new trainableSpellMap;

    //           template, trainer
    std::unordered_map <uint32, std::vector<CreatureInfo const*>> trainerTemplateIds;

    //Select all trainer lists and their trainers.
    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!co)
            continue;

        if (!co->TrainerType && !co->TrainerClass)
            continue;

        if(co->TrainerTemplateId)
            trainerTemplateIds[co->TrainerTemplateId].push_back(co);      
        else
            trainerTemplateIds[id].push_back(co);
    }

    for (auto& templateId : trainerTemplateIds)
    {
        TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerTemplateSpells(templateId.first);
        if (!trainer_spells)
            trainer_spells = sObjectMgr.GetNpcTrainerSpells(templateId.first);

        if (!trainer_spells)
            continue;

        uint8 trainerType = templateId.second.front()->TrainerType;

        uint32 spellType;
        if (trainerType == TRAINER_TYPE_CLASS || trainerType == TRAINER_TYPE_PETS)
            spellType = templateId.second.front()->TrainerClass;
        else if (trainerType == TRAINER_TYPE_MOUNTS)
            spellType = templateId.second.front()->TrainerRace;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            if (trainerType == TRAINER_TYPE_TRADESKILLS)
            {
                if (tSpell->reqSkill)
                    spellType = tSpell->reqSkill;
                else
                {
                    // exist, already checked at loading
                    SpellEntry const* spell = sSpellTemplate.LookupEntry<SpellEntry>(tSpell->learnedSpell);

                    spellType = spell->EffectMiscValue[1];
                }
            }

            for (auto& trainer : templateId.second)
                (*spellMap)[templateId.second.front()->TrainerType][spellType][tSpell].push_back(trainer);
        }
    }

    return spellMap;
}

std::vector<TrainerSpell const*> TrainableClassSpells::Calculate()
{
    std::vector<TrainerSpell const*> trainableSpells;
    std::unordered_map<uint32, bool> hasSpell;

    trainableSpellMap* spellMap = GAI_VALUE(trainableSpellMap*, "trainable spell map");

    for (auto& spells : (*spellMap)[TRAINER_TYPE_CLASS][bot->getClass()])
    {
        TrainerSpell const* tSpell = spells.first;

        if (!tSpell)
            continue;

        if (hasSpell.find(tSpell->spell) != hasSpell.end())
            continue;

        hasSpell[tSpell->spell] = true;

        uint32 reqLevel = 0;

        reqLevel = tSpell->isProvidedReqLevel ? tSpell->reqLevel : std::max(reqLevel, tSpell->reqLevel);
        TrainerSpellState state = bot->GetTrainerSpellState(tSpell, reqLevel);
        if (state != TRAINER_SPELL_GREEN)
            continue;

        trainableSpells.push_back(tSpell);
    }

    return trainableSpells;
}

std::string TrainableClassSpells::Format()
{
    std::vector<std::string> vec;  
    for (auto t : value) {
        SpellEntry const* spell = sServerFacade.LookupSpellInfo(t->spell);
        if (!spell)
            continue;
        vec.push_back(chat->formatSpell(spell));
    } 
    
    return sPlayerbotHelpMgr.makeList(vec, "[<part>]");
}

uint32 TrainCostValue::Calculate()
{
    uint32 TotalCost = 0;

    for (auto& spells : AI_VALUE(std::vector<TrainerSpell const*>, "trainable class spells"))
        TotalCost += spells->spellCost;

    /*
    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!co)
            continue;

        if (co->TrainerType != TRAINER_TYPE_CLASS && co->TrainerType != TRAINER_TYPE_TRADESKILLS)
            continue;

        if (co->TrainerType == TRAINER_TYPE_CLASS && co->TrainerClass != bot->getClass())
            continue;

        uint32 trainerId = co->TrainerTemplateId;
        if (!trainerId)
            trainerId = co->Entry;

        TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerTemplateSpells(trainerId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr.GetNpcTrainerSpells(trainerId);

        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            uint32 reqLevel = 0;

            reqLevel = tSpell->isProvidedReqLevel ? tSpell->reqLevel : std::max(reqLevel, tSpell->reqLevel);
            TrainerSpellState state = bot->GetTrainerSpellState(tSpell, reqLevel);
            if (state != TRAINER_SPELL_GREEN)
                continue;

            if (co->TrainerType == TRAINER_TYPE_TRADESKILLS)
                continue;

            if (spells.find(tSpell->spell) != spells.end())
                continue;

            TotalCost += tSpell->spellCost;
            spells.insert(tSpell->spell);
        }
    }
    */
    return TotalCost;
}
