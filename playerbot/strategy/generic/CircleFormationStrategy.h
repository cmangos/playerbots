#pragma once
#include "playerbot/strategy/Strategy.h"


namespace ai
{
    class CircleFormationStrategy : public Strategy
    {
    public:
        CircleFormationStrategy(PlayerbotAI* ai): Strategy(ai) {}
        std::string getName() override {  return "circle formation"; };

        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;



    private:

        std::string GetBotRole(Player* pBot); // To determine melee/ranged
        float GetRadiusForRole(const std::string& roleName);
        // Helper to get a consistent index for the bot within its group
        int GetBotGroupIndex(Player* pBot, Group* group, int& outTotalBotsInFormation);
    };
    
}







