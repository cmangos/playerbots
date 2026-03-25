#pragma once
#include "GenericActions.h"

namespace ai
{
    class PassLeadershipToMasterAction : public ChatCommandAction
    {
    public:
        PassLeadershipToMasterAction(PlayerbotAI* ai, std::string name = "leader", std::string message = "Passing leader to you!") : ChatCommandAction(ai, name), message(message) {}

        virtual Player* PassLeaderTo(Event& event) { return GetMaster(); };

        virtual bool Execute(Event& event) override
        {
            Player* passLeaderTo = PassLeaderTo(event);
            if (passLeaderTo && passLeaderTo != bot && bot->GetGroup() && bot->GetGroup()->IsMember(passLeaderTo->GetObjectGuid()))
            {
                WorldPacket p(SMSG_GROUP_SET_LEADER, 8);
                p << passLeaderTo->GetObjectGuid();
                bot->GetSession()->HandleGroupSetLeaderOpcode(p);
                
                if (!message.empty())
                    ai->TellPlayerNoFacing(passLeaderTo, message);

                if (sRandomPlayerbotMgr.IsRandomBot(bot))
                {
                    ai->ResetStrategies();
                    ai->Reset();
                }
                
                return true;
            }

            return false;
        }

        virtual bool isUseful() override
        {
            return ai->IsAlt() && bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid());
        }

        virtual bool isUsefulWhenStunned() override { return true; }

    protected:
        std::string message;
    };

    class GiveLeaderAction : public PassLeadershipToMasterAction 
    {
    public:
        GiveLeaderAction(PlayerbotAI* ai, std::string message = "Lead the way!") : PassLeadershipToMasterAction(ai, "give leader", message) {}

        virtual Player* PassLeaderTo(Event& event) { return event.getOwner(); };

        virtual bool isUseful() override
        {
            return bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid());
        }        
    };    
}
