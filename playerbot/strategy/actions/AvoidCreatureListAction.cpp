
#include "playerbot/playerbot.h"
#include "AvoidCreatureListAction.h"
#include "playerbot/strategy/values/AvoidCreatureListValue.h"
#include "LootAction.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool AvoidCreatureListAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string cmd = event.getParam();
    std::set<uint32>&avoidCreatures = AI_VALUE(std::set<uint32>&, "avoid creature list");

    if (cmd == "reset")
    {
        avoidCreatures.clear();
        ai->TellPlayer(requester, "The avoid creature list has been cleared");
        return true;
    }
    else if (cmd.empty() || cmd == "?")
    {   
        if (avoidCreatures.empty())
        {
            ai->TellPlayer(requester, "Avoid creature list is empty");
        }
        else
        {
            bool first = true;
            std::ostringstream out;
            out << "Avoid creature list: ";
            for (std::set<uint32>::iterator i = avoidCreatures.begin(); i != avoidCreatures.end(); i++)
            {
                const CreatureInfo* creatureEntry = sServerFacade.LookupCreatureInfo(*i);
                if (!creatureEntry)
                {
                    continue;
                }

                if (first) first = false; else out << ", ";
                out << *i;
            }

            ai->TellPlayer(requester, out);
        }

        return true;
    }
    else
    {
        std::vector<std::string> creatures = ParseCreatures(cmd);
        if (!creatures.empty())
        {
            for (std::string& creature : creatures)
            {
                const bool remove = creature.substr(0, 1) == "-";
                if (remove)
                {
                    // Remove the -
                    creature = creature.substr(1);
                }
                PlayerbotChatHandler handler(bot);
                uint32 creatureId = handler.extractCreatureId(creature);
                if (!creatureId)
                {
                    creatureId = AI_VALUE2(uint32, "creature id", creature);
                }

                if (!creatureId)
                {
                    ai->TellError(requester, "Unknown creature " + creature);
                    continue;
                }

                const CreatureInfo* creatureEntry = sServerFacade.LookupCreatureInfo(creatureId);
                if (!creatureEntry)
                {
                    ai->TellError(requester, "Unknown creature " + creature);
                    continue;
                }

                if (remove)
                {
                    std::set<uint32>::iterator j = avoidCreatures.find(creatureId);
                    if (j != avoidCreatures.end())
                    {
                        avoidCreatures.erase(j);
                        std::ostringstream out;
                        out << creatureId << " removed from avoid creature list";
                        ai->TellPlayer(requester, out);
                    }
                }
                else
                {
                    std::set<uint32>::iterator j = avoidCreatures.find(creatureId);
                    if (j == avoidCreatures.end())
                    {
                        avoidCreatures.insert(creatureId);
                        std::ostringstream out;
                        out << creatureId << " added to avoid creature list";
                        ai->TellPlayer(requester, out);
                    }
                }
            }

            return true;
        }
        else
        {
            ai->TellPlayer(requester, "Please specify one or more creatures to avoid");
        }
    }

    return false;
}

std::vector<std::string> AvoidCreatureListAction::ParseCreatures(const std::string& text)
{
    std::vector<std::string> creatures;

    size_t pos = 0;
    while (pos != std::string::npos) 
    {
        size_t nextPos = text.find(',', pos);
        std::string token = text.substr(pos, nextPos - pos);
        creatures.push_back(token);

        if (nextPos != std::string::npos) 
        {
            pos = nextPos + 1;
        }
        else 
        {
            break;
        }
    }

    return creatures;
}
