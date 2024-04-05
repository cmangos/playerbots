#pragma once

#include "playerbot/strategy/Action.h"
#include "UseItemAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

namespace ai
{
    class DrinkAction : public UseItemAction
    {
    public:
        DrinkAction(PlayerbotAI* ai) : UseItemAction(ai, "drink", true) {}

        virtual bool Execute(Event& event) override
        {
            if (sServerFacade.IsInCombat(bot))
                return false;

            if (!bot->HasMana())
                return false;

            if (ai->HasCheat(BotCheatMask::item))
            {
                if (bot->IsNonMeleeSpellCasted(true))
                    return false;

                bot->clearUnitState(UNIT_STAT_CHASE);
                bot->clearUnitState(UNIT_STAT_FOLLOW);

                if (ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                {
                    ai->StopMoving();
                }

                if (sServerFacade.isMoving(bot))
                {
                    ai->StopMoving();
                    SetDuration(sPlayerbotAIConfig.globalCoolDown);
                    return false;
                }

                bot->addUnitState(UNIT_STAND_STATE_SIT);
                ai->InterruptSpell();

                const float mpMissingPct = 100.0f - bot->GetPowerPercent();
                const float multiplier = bot->InBattleGround() ? 20000.0f : 27000.0f;
                const float drinkDuration = multiplier * (mpMissingPct / 100.0f);

                const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(24355);
                if (!pSpellInfo)
                    return false;

                ai->Unmount();

                ai->CastSpell(24355, bot);
                SetDuration(drinkDuration);
                bot->RemoveSpellCooldown(*pSpellInfo);

                // Eat and drink at the same time
                if (AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.lowHealth)
                {
                    const SpellEntry* pSpellInfo2 = sServerFacade.LookupSpellInfo(24005);
                    if (pSpellInfo2)
                    {
                        ai->CastSpell(24005, bot);
                        bot->RemoveSpellCooldown(*pSpellInfo2);
                    }
                }

                return true;
            }

            return UseItemAction::Execute(event);
        }

        virtual bool isUseful()
        {
            return UseItemAction::isUseful() && bot->HasMana() && (AI_VALUE2(uint8, "mana", "self target") < 85);
        }

        virtual bool isPossible()
        {
            return !sServerFacade.IsInCombat(bot) && UseItemAction::isPossible();
        }
    };

    class EatAction : public UseItemAction
    {
    public:
        EatAction(PlayerbotAI* ai) : UseItemAction(ai, "food", true) {}

        virtual bool Execute(Event& event) override
        {
            if (sServerFacade.IsInCombat(bot))
                return false;

            if (ai->HasCheat(BotCheatMask::item))
            {
                if (bot->IsNonMeleeSpellCasted(true))
                    return false;

                bot->clearUnitState(UNIT_STAT_CHASE);
                bot->clearUnitState(UNIT_STAT_FOLLOW);

                if (ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                {
                    ai->StopMoving();
                }

                if (sServerFacade.isMoving(bot))
                {
                    ai->StopMoving();
                    SetDuration(sPlayerbotAIConfig.globalCoolDown);
                    return false;
                }

                bot->addUnitState(UNIT_STAND_STATE_SIT);
                ai->InterruptSpell();

                const float hpMissingPct = 100.0f - bot->GetPowerPercent();
                const float multiplier = bot->InBattleGround() ? 20000.0f : 27000.0f;
                const float eatDuration = multiplier * (hpMissingPct / 100.0f);

                const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(24005);
                if (!pSpellInfo)
                    return false;

                ai->Unmount();

                ai->CastSpell(24005, bot);
                SetDuration(eatDuration);
                bot->RemoveSpellCooldown(*pSpellInfo);

                // Eat and drink at the same time
                if (bot->HasMana() && (AI_VALUE2(uint8, "mana", "self target") < 85))
                {
                    const SpellEntry* pSpellInfo2 = sServerFacade.LookupSpellInfo(24355);
                    if (pSpellInfo2)
                    {
                        ai->CastSpell(24355, bot);
                        bot->RemoveSpellCooldown(*pSpellInfo2);
                    }
                }

                return true;
            }

            return UseItemAction::Execute(event);
        }

        virtual bool isUseful()
        {
            return UseItemAction::isUseful() && (AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.lowHealth);
        }

        virtual bool isPossible()
        {
            return !sServerFacade.IsInCombat(bot) && UseItemAction::isPossible();
        }
    };
}
