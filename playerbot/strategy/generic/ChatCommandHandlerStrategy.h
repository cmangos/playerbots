#pragma once
#include "PassTroughStrategy.h"

namespace ai
{
    class ChatCommandHandlerStrategy : public PassTroughStrategy
    {
    public:
        ChatCommandHandlerStrategy(PlayerbotAI* ai);
        std::string getName() override { return "chat"; }
        static constexpr const char* ProfessionTrigger = "profession";
        static constexpr const char* ProfessionAction = "profession";
        static constexpr const char* CastNcTrigger = "castnc";
        static constexpr const char* CastNcAction = "cast custom nc spell";


#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "chat"; } //Must equal internal name
        virtual std::string GetHelpDescription() 
        {
            return "This strategy will make bots respond to various chat commands.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { }; }
#endif

    private:
        void InitReactionTriggers(std::list<TriggerNode*> &triggers) override;
    };
}
