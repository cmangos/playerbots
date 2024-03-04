
#include "playerbot/playerbot.h"
#include "SkipSpellsListAction.h"
#include "playerbot/strategy/values/SkipSpellsListValue.h"
#include "LootAction.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool SkipSpellsListAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    string cmd = event.getParam();
    set<uint32>& skipSpells = AI_VALUE(set<uint32>&, "skip spells list");

    if (cmd == "reset")
    {
        skipSpells.clear();
        ai->TellPlayer(requester, "The ignored spell list has been cleared");
        return true;
    }
    else if (cmd.empty() || cmd == "?")
    {   
        if (skipSpells.empty())
        {
            ai->TellPlayer(requester, "Ignored spell list is empty");
        }
        else
        {
            bool first = true;
            ostringstream out;
            out << "Ignored spell list: ";
            for (set<uint32>::iterator i = skipSpells.begin(); i != skipSpells.end(); i++)
            {
                const SpellEntry* spellEntry = sServerFacade.LookupSpellInfo(*i);
                if (!spellEntry)
                {
                    continue;
                }

                if (first) first = false; else out << ", ";
                out << chat->formatSpell(spellEntry);
            }

            ai->TellPlayer(requester, out);
        }

        return true;
    }
    else
    {
        std::vector<string> spells = ParseSpells(cmd);
        if (!spells.empty())
        {
            for (string& spell : spells)
            {
                const bool remove = spell.substr(0, 1) == "-";
                if (remove)
                {
                    // Remove the -
                    spell = spell.substr(1);
                }

                uint32 spellId = chat->parseSpell(spell);
                if (!spellId)
                {
                    spellId = AI_VALUE2(uint32, "spell id", spell);
                }

                if (!spellId)
                {
                    ai->TellError(requester, "Unknown spell " + spell);
                    continue;
                }

                const SpellEntry* spellEntry = sServerFacade.LookupSpellInfo(spellId);
                if (!spellEntry)
                {
                    ai->TellError(requester, "Unknown spell " + spell);
                    continue;
                }

                if (remove)
                {
                    set<uint32>::iterator j = skipSpells.find(spellId);
                    if (j != skipSpells.end())
                    {
                        skipSpells.erase(j);
                        ostringstream out;
                        out << chat->formatSpell(spellEntry) << " removed from ignored spells";
                        ai->TellPlayer(requester, out);
                    }
                }
                else
                {
                    set<uint32>::iterator j = skipSpells.find(spellId);
                    if (j == skipSpells.end())
                    {
                        skipSpells.insert(spellId);
                        ostringstream out;
                        out << chat->formatSpell(spellEntry) << " added to ignored spells";
                        ai->TellPlayer(requester, out);
                    }
                }
            }

            return true;
        }
        else
        {
            ai->TellPlayer(requester, "Please specify one or more spells to ignore");
        }
    }

    return false;
}

std::vector<string> SkipSpellsListAction::ParseSpells(const string& text)
{
    std::vector<std::string> spells;

    size_t pos = 0;
    while (pos != std::string::npos) 
    {
        size_t nextPos = text.find(',', pos);
        std::string token = text.substr(pos, nextPos - pos);
        spells.push_back(token);

        if (nextPos != std::string::npos) 
        {
            pos = nextPos + 1;
        }
        else 
        {
            break;
        }
    }

    return spells;
}
