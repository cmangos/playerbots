#pragma once
#include "playerbot/ServerFacade.h"

namespace ai
{
    //                    spell                  , trainers
    typedef unordered_map<TrainerSpell const*, vector<CreatureInfo const*>>  spellTrainerMap;
    //              spellsTypes, spell
    typedef unordered_map<uint32, spellTrainerMap>          trainableSpellList;
    //              trainerType, spellsTypes
    typedef unordered_map<uint8, trainableSpellList>         trainableSpellMap;

    class TrainableSpellMapValue : public SingleCalculatedValue<trainableSpellMap*>
    {
    public:
        TrainableSpellMapValue(PlayerbotAI* ai) : SingleCalculatedValue<trainableSpellMap*>(ai, "trainable spell map") {}

        virtual trainableSpellMap* Calculate() override;
        virtual ~TrainableSpellMapValue() { delete value; }
    };

    class TrainableClassSpells : public CalculatedValue<vector< TrainerSpell const*>>
    {
    public:
        TrainableClassSpells(PlayerbotAI* ai) : CalculatedValue<vector<TrainerSpell const*>>(ai, "trainable class spells") {}

        virtual vector<TrainerSpell const*> Calculate();

        virtual string Format() override;
    };

    class TrainCostValue : public Uint32CalculatedValue
    {
    public:
        TrainCostValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "train cost", 60) {}

        virtual uint32 Calculate();
    };
}

