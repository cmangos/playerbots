#pragma once
#include "playerbot/strategy/Multiplier.h"
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    // Below the mana threshold a healer drops heals that restore less than the target is missing or that spend mana poorly.
    class HealerAutoSaveManaMultiplier : public Multiplier
    {
    public:
        HealerAutoSaveManaMultiplier(PlayerbotAI* ai);

    public:
        virtual float GetValue(Action* action) override;

    private:
        uint32 manaThreshold;
    };

    class SaveManaStrategy : public Strategy
    {
    public:
        SaveManaStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        std::string getName() override { return "save mana"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "save mana"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy makes healers skip heals that waste mana when their mana is low.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { }; }
#endif
    private:
        void InitCombatMultipliers(std::list<Multiplier*> &multipliers) override;
    };
}
