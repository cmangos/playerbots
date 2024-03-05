#pragma once
#include "playerbot/ServerFacade.h"

namespace ai
{
    //                    spell                  , trainers
    typedef std::unordered_map<TrainerSpell const*, std::vector<CreatureInfo const*>>  spellTrainerMap;
    //              spellsTypes, spell
    typedef std::unordered_map<uint32, spellTrainerMap>          trainableSpellList;
    //              trainerType, spellsTypes
    typedef std::unordered_map<uint8, trainableSpellList>         trainableSpellMap;

    class TrainableSpellMapValue : public SingleCalculatedValue<trainableSpellMap*>
    {
    public:
        TrainableSpellMapValue(PlayerbotAI* ai) : SingleCalculatedValue<trainableSpellMap*>(ai, "trainable spell map") {}

        virtual trainableSpellMap* Calculate() override;
        virtual ~TrainableSpellMapValue() { delete value; }
    };

    class TrainableClassSpells : public CalculatedValue<std::vector< TrainerSpell const*>>
    {
    public:
        TrainableClassSpells(PlayerbotAI* ai) : CalculatedValue<std::vector<TrainerSpell const*>>(ai, "trainable class spells") {}

        virtual std::vector<TrainerSpell const*> Calculate();

        virtual std::string Format() override;
    };

    class TrainCostValue : public Uint32CalculatedValue
    {
    public:
        TrainCostValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "train cost", 60) {}

        virtual uint32 Calculate();
    };
}

