
#include "playerbot/playerbot.h"
#include "SayAction.h"
#include "playerbot/PlayerbotTextMgr.h"
#include "Chat/ChannelMgr.h"
#include "playerbot/ServerFacade.h"
#include <regex>

using namespace ai;

std::unordered_set<std::string> noReplyMsgs = {
  "join", "leave", "follow", "attack", "pull", "flee", "reset", "reset ai",
  "all ?", "talents", "talents list", "talents auto", "talk", "stay", "stats",
  "who", "items", "leave", "join", "repair", "summon", "nc ?", "co ?", "de ?",
  "dead ?", "follow", "los", "guard", "do accept invitation", "stats", "react ?",
  "reset strats", "home",
};
std::unordered_set<std::string> noReplyMsgParts = { "+", "-","@" , "follow target", "focus heal", "boost target", "buff target", "revive target", "cast ", "accept [", "e [", "destroy [", "go zone" };

std::unordered_set<std::string> noReplyMsgStarts = { "e ", "accept ", "cast ", "destroy " };

SayAction::SayAction(PlayerbotAI* ai) : Action(ai, "say"), Qualified()
{
}

bool SayAction::Execute(Event& event)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target) target = AI_VALUE(Unit*, "current target");

    // set replace std::strings
    if (target) placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->getRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (pItem)
        {
            switch (pItem->GetProto()->SubClass)
            {
            case ITEM_SUBCLASS_WEAPON_GUN:
                placeholders["<ammo>"] = "bullets";
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                placeholders["<ammo>"] = "arrows";
                break;
            }
        }
    }

    if (bot->IsInWorld())
    {
        if (AreaTableEntry const* area = GetAreaEntryByAreaID(sServerFacade.GetAreaId(bot)))
            placeholders["<subzone>"] = area->area_name[0];
    }

    // set delay before next say
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    uint32 nextTime = time(0) + urand(1, 30);
    ai->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            PlayerbotAI* memberAi = member->GetPlayerbotAI();
            if (memberAi) members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (std::vector<Player*>::iterator i = members.begin(); i != members.end(); ++i)
        {
            PlayerbotAI* memberAi = (*i)->GetPlayerbotAI();
            if (memberAi)
                memberAi->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!sPlayerbotTextMgr.GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    else
        bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    return true;
}


bool SayAction::isUseful()
{
    if (!ai->AllowActivity())
        return false;

    if (ai->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT))
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(0) - lastSaid) > 30;
}

void ChatReplyAction::ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name)
{
    // if we're just commanding bots around, don't respond...
    // first one is for exact word matches
    if (noReplyMsgs.find(msg) != noReplyMsgs.end()) {
        //ostringstream out;
        //out << "DEBUG ChatReplyDo decided to ignore exact blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);
        return;
    }

    // second one is for partial matches like + or - where we change strats
    if (std::any_of(noReplyMsgParts.begin(), noReplyMsgParts.end(), [&msg](const std::string& part) { return msg.find(part) != std::string::npos; })) {
        //ostringstream out;
        //out << "DEBUG ChatReplyDo decided to ignore partial blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);

        return;
    }

    if (std::any_of(noReplyMsgStarts.begin(), noReplyMsgStarts.end(), [&msg](const std::string& start) {
        return msg.find(start) == 0;  // Check if the start matches the beginning of msg
        })) {
        //ostringstream out;
        //out << "DEBUG ChatReplyDo decided to ignore start blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);
        return;
    }

    std::string responseMessage = CreateReplyMessage(bot, msg, guid1, name);

    // send responds
    if (type == CHAT_MSG_CHANNEL)
    {
        if (chanName == "World")
        {
            bot->GetPlayerbotAI()->SayToWorld(responseMessage);
        }
    }
    else
    {
        switch (type)
        {
            case CHAT_MSG_WHISPER:
            {
                bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                break;
            }
            case CHAT_MSG_SAY:
            {
                bot->GetPlayerbotAI()->Say(responseMessage);
                break;
            }
            case CHAT_MSG_YELL:
            {
                bot->GetPlayerbotAI()->Yell(responseMessage);
                break;
            }
            case CHAT_MSG_GUILD:
            {
                bot->GetPlayerbotAI()->SayToGuild(responseMessage);
                break;
            }
            case CHAT_MSG_PARTY:
            case CHAT_MSG_RAID:
            case CHAT_MSG_EMOTE:
            case CHAT_MSG_TEXT_EMOTE:
            {
                //TODO unhandled
                break;
            }
            default:
                break;
        }
    }
    bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
}

std::string ChatReplyAction::CreateReplyMessage(Player* bot, std::string incomingMessage, uint32 guid1, std::string name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand

    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(incomingMessage);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (incomingMessage.find("?") != std::string::npos)
        is_quest = 1;
    if (word[0].find("what") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("who") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "when")
        is_quest = 4;
    else if (word[0] == "where")
        is_quest = 5;
    else if (word[0] == "why")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
        // blame gm with chat tag
        if (Player* plr = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid1)))
        {
            if (plr->isGMChat())
            {
                replyType = REPLY_ADMIN_ABUSE;
                found = true;
                break;
            }
        }

        if (word[i] == "hi" || word[i] == "hey" || word[i] == "hello" || word[i] == "wazzup")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "am" || word[i] == "are" || word[i] == "is")
            {
                verb_pos = i;
                verb_type = 2; // present
                if (verb_pos == 0)
                    is_quest = 1;
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3; // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1; // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (incomingMessage.find(bot->GetName()) == std::string::npos)
                {
                    continue; // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "sorry %s, ill shut up now";
                    if (rnd == 1)
                        msg = "ok ok %s";
                    if (rnd == 2)
                        msg = "fine, i wont talk to you anymore %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
        case 2:
        {
            uint32 rnd = urand(0, 3);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "i dont know what";
                break;
            case 1:
                msg = "i dont know %s";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afraid that was before i was around or paying attention";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 4);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "nobody";
                break;
            case 1:
                msg = "we all do";
                break;
            case 2:
                msg = "perhaps its you, %s";
                break;
            case 3:
                msg = "dunno %s";
                break;
            case 4:
                msg = "is it me?";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 4:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "soon perhaps %s";
                break;
            case 1:
                msg = "probably later";
                break;
            case 2:
                msg = "never";
                break;
            case 3:
                msg = "what do i look like, a psychic?";
                break;
            case 4:
                msg = "a few minutes, maybe an hour ... years?";
                break;
            case 5:
                msg = "when? good question %s";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 5:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "really want me to answer that?";
                break;
            case 1:
                msg = "on the map?";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afk?";
                break;
            case 4:
                msg = "none of your buisiness where";
                break;
            case 5:
                msg = "yeah, where?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 6:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "dunno %s";
                break;
            case 1:
                msg = "why? just because %s";
                break;
            case 2:
                msg = "why is the sky blue?";
                break;
            case 3:
                msg = "dont ask me %s, im just a bot";
                break;
            case 4:
                msg = "your asking the wrong person";
                break;
            case 5:
                msg = "who knows?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        default:
        {
            switch (verb_type)
            {
            case 1:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                    break;
                case 1:
                    msg = "ya %s but thats in the past";
                    break;
                case 2:
                    msg = "nah, but " + word[verb_pos + 1] + " will " + word[verb_pos + 3] + " again though %s";
                    break;
                case 3:
                    msg = "afraid that was before i was around or paying attention";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 1:
                    msg = "ya %s thats true";
                    break;
                case 2:
                    msg = "maybe " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 3:
                    msg = "dunno %s";
                    break;
                case 4:
                    msg = "i dont think so %s";
                    break;
                case 5:
                    msg = "yes";
                    break;
                case 6:
                    msg = "no";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 8);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "dunno %s";
                    break;
                case 1:
                    msg = "beats me %s";
                    break;
                case 2:
                    msg = "how should i know %s";
                    break;
                case 3:
                    msg = "dont ask me %s, im just a bot";
                    break;
                case 4:
                    msg = "your asking the wrong person";
                    break;
                case 5:
                    msg = "what do i look like, a psychic?";
                    break;
                case 6:
                    msg = "sure %s";
                    break;
                case 7:
                    msg = "i dont think so %s";
                    break;
                case 8:
                    msg = "maybe";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            }
        }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
        case 1:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "yeah %s, the key word being " + word[verb_pos] + " " + word[verb_pos + 1];
                break;
            case 1:
                msg = "ya %s but thats in the past";
                break;
            case 2:
                msg = word[verb_pos - 1] + " will " + word[verb_pos + 1] + " again though %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 2:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "%s, what do you mean " + word[verb_pos + 1] + "?";
                break;
            case 1:
                msg = "%s, what is a " + word[verb_pos + 1] + "?";
                break;
            case 2:
                msg = "yeah i know " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " is a " + word[verb_pos + 1];
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 1);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "are you sure thats going to happen %s?";
                break;
            case 1:
                msg = "%s, what will happen %s?";
                break;
            case 2:
                msg = "are you saying " + word[verb_pos - 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        }
    }

    if (!found)
    {
        // Name Responds
        if (incomingMessage.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // load text if needed
    if (respondsText.empty())
    {
        respondsText = BOT_TEXT2(replyType, name);
    }

    if (respondsText.size() > 255)
    {
        respondsText.resize(255);
    }

    return respondsText;

}

bool ChatReplyAction::isUseful()
{
    return !ai->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT);
}
