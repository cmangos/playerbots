#pragma once
#include "playerbot/strategy/Trigger.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/RtiTargetValue.h"

namespace ai
{
    class NoRtiTrigger : public Trigger
    {
    public:
        NoRtiTrigger(PlayerbotAI* ai) : Trigger(ai, "no rti target") {}

        virtual bool IsActive() override
		{
            if (AI_VALUE(Unit*, "rti target"))
            {
                return false;
            }
            else
            {
                // Check for the default rti if the bot is setup to ignore rti targets
                std::string rti = AI_VALUE(std::string, "rti");
                if (rti == "none")
                {
                    Group* group = bot->GetGroup();
                    if (group)
                    {
                        const ObjectGuid guid = group->GetTargetIcon(RtiTargetValue::GetRtiIndex("skull"));
                        if (!guid.IsEmpty())
                        {
                            Unit* unit = ai->GetUnit(ObjectGuid(guid));
                            if (unit && !sServerFacade.UnitIsDead(unit) && bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance, false))
                            {
                                return false;
                            }
                        }
                    }
                }
            }

            return true;
        }
    };

    class NoRtiCCTrigger : public Trigger
    {
    public:
        NoRtiCCTrigger(PlayerbotAI* ai) : Trigger(ai, "no rti cc target") {}

        virtual bool IsActive() override
		{
            if (AI_VALUE(Unit*, "rti cc target"))
            {
                return false;
            }
            else
            {
                // Check for the default rti cc if the bot is setup to ignore rti cc targets
                std::string rti = AI_VALUE(std::string, "rti cc");
                int index = RtiTargetValue::GetRtiIndex(rti);
                if (index == -1)
                {
                    // If we have cc enabled, but no mark set, then we're open to ccing stuff at random
                    if (ai->HasStrategy("cc", BotState::BOT_STATE_COMBAT))
                        return true;
                }

                return false;
            }
        }
    };
}
