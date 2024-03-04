#pragma once

#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/Action.h"

namespace ai
{
    class AcceptResurrectAction : public Action {
    public:
        AcceptResurrectAction(PlayerbotAI* ai) : Action(ai, "accept resurrect") {}

        virtual bool Execute(Event& event)
        {
            if (!sServerFacade.IsAlive(bot))
            {
                WorldPacket p(event.getPacket());
                p.rpos(0);
                ObjectGuid guid;
                p >> guid;

                WorldPacket packet(CMSG_RESURRECT_RESPONSE, 8+1);
                packet << guid;
                packet << uint8(1);                                       // accept
                bot->GetSession()->HandleResurrectResponseOpcode(packet); // queue the packet to get around race condition
                return true;
            }

            return false;
        }
    };

}
