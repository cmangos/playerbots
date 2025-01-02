
#include "playerbot/playerbot.h"
#include "SetValueAction.h"

using namespace ai;

bool SetValueAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    std::string param = event.getParam();
    std::string value;

    if (param.find("group size ") == 0)
    {
        value = param.substr(std::string("group size ").size());

        if (!Qualified::isValidNumberString(value))
        {
#ifdef MANGOSBOT_ZERO
            ai->TellPlayer(requester, "Incorrect value " + value + " allowed values: -1,0,1,2,3,4,5,20,40", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
#else
            ai->TellPlayer(requester, "Incorrect value " + value + " allowed values: -1,0,1,2,3,4,5,10,25", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
#endif
            return false;
        }

        int8 type = stoi(value);

        switch (type)
        {
        case -1:
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
#ifndef MANGOSBOT_ZERO
        case 10:
        case 25:
#else
        case 20:
        case 40:
#endif
            SET_AI_VALUE2(int32, "manual saved int", "grouper override", uint32(type));

            if (type == -1)
            {
                ai->TellPlayer(requester, "Prefered group size reset to random:", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
                value = std::to_string((uint8)ai->GetGrouperType());
            }

            if (type == 0)
            ai->TellPlayer(requester, "Prefered group size set to solo.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            else if (type == 1)
                ai->TellPlayer(requester, "Prefered group size set to allow others to invite.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            else
                ai->TellPlayer(requester, "Prefered group size set to invite up to " + value, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);    

            return true;
            break;
        default:
#ifdef MANGOSBOT_ZERO
            ai->TellPlayer(requester, "Incorrect value " + value + " allowed values: -1,0,1,2,3,4,5,20,40", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
#else
            ai->TellPlayer(requester, "Incorrect value " + value + " allowed values: -1,0,1,2,3,4,5,10,25", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
#endif
        }
        return false;
    }
    else if (param.find("guild size ") == 0)
    {
        value = param.substr(std::string("guild size ").size());

        if (!Qualified::isValidNumberString(value))
        {
            ai->TellPlayer(requester, "Incorrect value " + value + " needs to be a number.", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            return false;
        }

        std::vector<int32> ranges = { 0,30,50,70,120,250 };

        uint8 type = stoi(value);

        if (type < -1)
            type = -1;
        if (type > 250)
            type = 250;

        for (auto& r : ranges)
        {
            if (type > r)
                continue;

            type = r;
            break;
        }
        SET_AI_VALUE2(int32, "manual saved int", "guilder override", uint32(type));

        if (type == -1)
        {
            ai->TellPlayer(requester, "Prefered guild size reset to random:", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            value = std::to_string((uint8)ai->GetGuilderType());
        }

        ai->TellPlayer(requester, "Prefered guild size set to " + std::to_string(type), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
        return true;
    }

    ai->TellPlayer(requester, "Correct usage: set value <value name> <value>", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    ai->TellPlayer(requester, "Values supported: group size, guild size", PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    return true;
}