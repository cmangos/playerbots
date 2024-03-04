#pragma once
#include "../Value.h"
#include "TargetValue.h"
#include "playerbot/LootObjectStack.h"

namespace ai
{
    class HaveAnyTotemValue : public BoolCalculatedValue, public Qualified
	{
	public:
        HaveAnyTotemValue(PlayerbotAI* ai, string name = "have any totem") : BoolCalculatedValue(ai, name), Qualified() {}

        bool Calculate()
        {
            list<ObjectGuid> units = *context->GetValue<list<ObjectGuid> >("nearest npcs");
            for (list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                Creature* creature = dynamic_cast<Creature*>(unit);
                if (creature && creature->IsTotem()) 
                {
                    return true;
                }
            }

            return false;
        }
    };
}
