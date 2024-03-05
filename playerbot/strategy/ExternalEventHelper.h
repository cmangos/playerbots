#pragma once
#include "Trigger.h"
#include "triggers/WorldPacketTrigger.h"
#include "playerbot/ChatHelper.h"

namespace ai
{
    class ExternalEventHelper 
    {
    public:
        ExternalEventHelper(AiObjectContext* aiObjectContext) : aiObjectContext(aiObjectContext) {}

        bool ParseChatCommand(string command, Player* owner = NULL)
        {
            string linkCommand = ChatHelper::parseValue("command", command);
            bool forceCommand = false;

            if (!linkCommand.empty())
            {
                command = linkCommand;
                forceCommand = true;
            }

            if (HandleCommand(command, "", owner, forceCommand))
                return true;

            size_t i = string::npos;
            while (true)
            {
                size_t found = command.rfind(" ", i);
                if (found == string::npos || !found)
                    break;

                string name = command.substr(0, found);
                string param = command.substr(found + 1);

                i = found - 1;

                if (HandleCommand(name, param, owner, forceCommand))
                    return true;
            }

            if (!ChatHelper::parseable(command))
                return false;

            if (command.find("Hvalue:help") != string::npos || command.find("[h:") != string::npos)
            {
                HandleCommand("help", command, owner);
                return true;
            }

            HandleCommand("c", command, owner);
            HandleCommand("t", command, owner);
            return true;
        }

        bool HandlePacket(map<uint16, string> &handlers, const WorldPacket &packet, Player* owner = NULL)
        {
            uint16 opcode = packet.GetOpcode();
            string name = handlers[opcode];
            if (name.empty())
                return true;

            Trigger* trigger = aiObjectContext->GetTrigger(name);
            if (!trigger)
                return true;

            Event event = trigger->Check();            
            if (!event.getSource().empty())
                return false;
                        
            WorldPacket p(packet);
            trigger->ExternalEvent(p, owner);

            return true;
        }

        bool HandleCommand(string name, string param, Player* owner = NULL, bool forceCommand = false)
        {
            Trigger* trigger = aiObjectContext->GetTrigger(name);
            if (!trigger)
                return false;

            if(!forceCommand)
                trigger->ExternalEvent(param, owner);
            else
                trigger->ExternalEventForce(param, owner);
            return true;
        }

    private:
        AiObjectContext* aiObjectContext;
    };
}
