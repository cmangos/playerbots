#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class WarsongStrategy : public Strategy
    {
    public:
        WarsongStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        int GetType() override { return STRATEGY_TYPE_GENERIC; }
        std::string getName() override { return "warsong"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "warsong"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy controls the behavior during a warsong gluch battleground like capturing/retaking flags and picking up buffs.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "battleground", "bg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArathiStrategy : public Strategy
    {
    public:
        ArathiStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        int GetType() override { return STRATEGY_TYPE_GENERIC; }
        std::string getName() override { return "arathi"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "arathi"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy controls the behavior during an arathi basin battleground.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "battleground","bg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class EyeStrategy : public Strategy
    {
    public:
        EyeStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        int GetType() override { return STRATEGY_TYPE_GENERIC; }
        std::string getName() { return "eye"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "eye"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy controls the behavior during an eye of the storm basin battleground.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "battleground","bg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class IsleStrategy : public Strategy
    {
    public:
        IsleStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        int GetType() override { return STRATEGY_TYPE_GENERIC; }
        std::string getName() override { return "isle"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "isle"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy controls the behavior during an isle of conquest battleground.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "battleground","bg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };

    class ArenaStrategy : public Strategy
    {
    public:
        ArenaStrategy(PlayerbotAI* ai) : Strategy(ai) {};
        int GetType() override { return STRATEGY_TYPE_GENERIC; }
        std::string getName() override { return "arena"; }
#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "arena"; } //Must equal iternal name
        virtual std::string GetHelpDescription() {
            return "This strategy controls the behavior arena fight.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "battleground","bg" }; }
#endif
    private:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitCombatTriggers(std::list<TriggerNode*>& triggers) override;
    };
}
