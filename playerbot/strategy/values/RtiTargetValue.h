#pragma once
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/Value.h"
#include "Groups/Group.h"
#include "TargetValue.h"

namespace ai
{
    class RtiTargetValue : public TargetValue
    {
    public:
        RtiTargetValue(PlayerbotAI* ai, std::string type = "rti", std::string name = "rti target") : type(type), TargetValue(ai,name)
        {}

    public:
        static int GetRtiIndex(std::string rti)
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

        static bool IsTarget(Player* player, Unit* target)
        {
            if (!player || !target || !player->GetPlayerbotAI())
                return false;

            Group* group = player->GetGroup();
            if (!group)
                return false;

            ObjectGuid const guid = target->GetObjectGuid();
            for (uint8 icon = 0; icon < TARGET_ICON_COUNT; ++icon)
            {
                if (group->GetTargetIcon(icon) == guid)
                    return PAI_VALUE(ObjectGuid, "rti target") == guid;
            }

            return false;
        }

        ObjectGuid Calculate() override
        {
            Group *group = bot->GetGroup();
            if(!group)
                return ObjectGuid();

            std::string rti = AI_VALUE(std::string, type);
            int index = GetRtiIndex(rti);

            if (index == -1)
                return ObjectGuid();

            ObjectGuid guid = group->GetTargetIcon(index);
            if (!guid)
                return ObjectGuid();

            std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("possible targets no los")->Get();
            if (std::find(attackers.begin(), attackers.end(), guid) == attackers.end()) return ObjectGuid();

            Unit* unit = ai->GetUnit(ObjectGuid(guid));
            if (!unit || sServerFacade.UnitIsDead(unit) ||
                !bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance, false))
                return ObjectGuid();

            return unit ? unit->GetObjectGuid() : ObjectGuid();
        }

    private:
    std::string type;
    };

    class RtiCcTargetValue : public RtiTargetValue
    {
    public:
        RtiCcTargetValue(PlayerbotAI* ai, std::string name = "rti cc target") : RtiTargetValue(ai, "rti cc", name) {}
    };
}
