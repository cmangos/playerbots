#pragma once
#include "../Value.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

namespace ai
{
    class NearestUnitsValue : public ObjectGuidListCalculatedValue
	{
	public:
        NearestUnitsValue(PlayerbotAI* ai, string name = "nearest units", float range = sPlayerbotAIConfig.sightDistance, bool ignoreLos = false) :
            ObjectGuidListCalculatedValue(ai, name, 2), range(range), ignoreLos(ignoreLos) {}

	public:
        virtual list<ObjectGuid> Calculate()
        {
            list<Unit*> targets;
            FindUnits(targets);

            list<ObjectGuid> results;
            for(list<Unit *>::iterator i = targets.begin(); i!= targets.end(); ++i)
            {
                Unit* unit = *i;
                if(ai->IsSafe(unit))
                {
                    if ((ignoreLos || sServerFacade.IsWithinLOSInMap(bot, unit)) && AcceptUnit(unit))
                        results.push_back(unit->GetObjectGuid());
                }
            }
            return results;
        }

    protected:
        virtual void FindUnits(list<Unit*> &targets) = 0;
        virtual bool AcceptUnit(Unit* unit) = 0;

    protected:
        float range;
        bool ignoreLos;
	};

    class NearestStealthedUnitsValue : public NearestUnitsValue
    {
    public:
        NearestStealthedUnitsValue(PlayerbotAI* ai, float range = 30.0f) :
            NearestUnitsValue(ai, "nearest stealthed units", range) {}

    protected:
        void FindUnits(list<Unit*>& targets)
        {
            MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(bot, range);
            MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
            Cell::VisitAllObjects(bot, searcher, range);
        }
        bool AcceptUnit(Unit* unit)
        {
            if (!unit || !unit->IsAlive() || !sServerFacade.IsHostileTo(unit, bot))
                return false;

            return unit->HasAuraType(SPELL_AURA_MOD_STEALTH) || unit->HasAuraType(SPELL_AURA_MOD_INVISIBILITY);

            /*uint32 dispelMask = 0;
            dispelMask |= GetDispellMask(DispelType(DISPEL_STEALTH));
            dispelMask |= GetDispellMask(DispelType(DISPEL_INVISIBILITY));

            return unit->HasMechanicMaskOrDispelMaskAura(dispelMask, 0, bot);*/
        }
    };

    class NearestStealthedSingleUnitValue : public UnitCalculatedValue
    {
    public:
        NearestStealthedSingleUnitValue(PlayerbotAI* ai) :
            UnitCalculatedValue(ai, "nearest stealthed unit") {}

        virtual Unit* Calculate()
        {
            list<ObjectGuid> targets = AI_VALUE(list<ObjectGuid>, "nearest stealthed units");
            if (targets.empty())
                return nullptr;

            vector<Unit*> units;
            for (list<ObjectGuid>::iterator i = targets.begin(); i != targets.end(); ++i)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                units.push_back(unit);
            }

            if (units.empty())
                return nullptr;

            return units[urand(0, units.size() - 1)];
        }
    };
}
