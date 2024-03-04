#pragma once
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "../Value.h"
#include "Groups/Group.h"
#include "TargetValue.h"

namespace ai
{
    class RtiTargetValue : public TargetValue
    {
    public:
        RtiTargetValue(PlayerbotAI* ai, string type = "rti", string name = "rti target") : type(type), TargetValue(ai,name)
        {}

    public:
        static int GetRtiIndex(string rti)
        {
            int index = -1;
            if(rti == "star") index = 0;
            else if(rti == "circle") index = 1;
            else if(rti == "diamond") index = 2;
            else if(rti == "triangle") index = 3;
            else if(rti == "moon") index = 4;
            else if(rti == "square") index = 5;
            else if(rti == "cross") index = 6;
            else if(rti == "skull") index = 7;
            return index;
        }

        Unit *Calculate()
        {
            Group *group = bot->GetGroup();
            if(!group)
                return NULL;

            string rti = AI_VALUE(string, type);
            int index = GetRtiIndex(rti);

            if (index == -1)
                return NULL;

            ObjectGuid guid = group->GetTargetIcon(index);
            if (!guid)
                return NULL;

            list<ObjectGuid> attackers = context->GetValue<list<ObjectGuid>>("possible targets")->Get();
            if (find(attackers.begin(), attackers.end(), guid) == attackers.end()) return NULL;

            Unit* unit = ai->GetUnit(ObjectGuid(guid));
            if (!unit || sServerFacade.UnitIsDead(unit) ||
                !bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance, false))
                return NULL;

            return unit;
        }

    private:
        string type;
    };

    class RtiCcTargetValue : public RtiTargetValue
    {
    public:
        RtiCcTargetValue(PlayerbotAI* ai, string name = "rti cc target") : RtiTargetValue(ai, "rti cc", name) {}
    };
}
