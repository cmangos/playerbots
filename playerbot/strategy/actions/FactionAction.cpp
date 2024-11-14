
#include "playerbot/playerbot.h"
#include "FactionAction.h"
#include "Tools/Language.h"

using namespace ai;

bool FactionAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string cmd = event.getParam();

    bool setwar = (cmd.find("+atwar") == 0);
    bool removeWar = (cmd.find("-atwar") == 0);

    std::string factionName = cmd;

    if (setwar || removeWar)
        factionName = factionName.substr(7);

    std::wstring wnamepart;

    if (!Utf8toWStr(factionName, wnamepart))
        return false;

    std::set<uint32> factionIds = ChatHelper::ExtractAllFactionIds(factionName);

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    bool factionFound = false;
    std::map<std::string, std::string> args;

    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); ++id)
    {
#ifndef MANGOSBOT_ONE
        const FactionEntry* factionEntry = sFactionStore.LookupEntry(id);
#else
        const FactionEntry* factionEntry = sFactionStore.LookupEntry<FactionEntry>(id);
#endif

        if (!factionEntry)
            continue;

        FactionState const* repState = bot->GetReputationMgr().GetState(factionEntry);

        if (!repState || repState->Flags & FACTION_FLAG_VISIBLE)
            continue;

        if (!bot->HasSkill(id))
            continue;

        int loc = requester->GetSession()->GetSessionDbcLocale();

        if (!factionName.empty() && factionIds.empty())
        {
            std::string name = factionEntry->name[loc];

            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (loc == requester->GetSession()->GetSessionDbcLocale())
                        continue;

                    name = factionEntry->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }
        }

        if (factionName.empty() || factionIds.find(id) != factionIds.end() || (loc < MAX_LOCALE && factionIds.empty()))
        {
            ReputationMgr& mgr = bot->GetReputationMgr();

            if (setwar || removeWar)
            {
                args["%factionname"] = ChatHelper::formatSkill(id);

                if (removeWar && mgr.IsAtWar(id) && bot->GetReputationMgr().GetRank(factionEntry) == REP_HATED)
                {
                    ai->TellPlayerNoFacing(requester, BOT_TEXT2("Unable uncheck at war for %factionname because they hate me too much.", args));
                    return false;
                }

                if (!factionName.empty())
                {
                    if (repState->Flags & FACTION_FLAG_INVISIBLE_FORCED)
                    {
                        ai->TellPlayerNoFacing(requester, BOT_TEXT2("Unable change at war for %factionname because the warstate is forced.", args));
                        return false;
                    }

                    if (removeWar && !mgr.IsAtWar(id))
                    {
                        ai->TellPlayerNoFacing(requester, BOT_TEXT2("I already have at war unchecked for %factionname.", args));
                        return false;
                    }

                    if (setwar && mgr.IsAtWar(id))
                    {
                        ai->TellPlayerNoFacing(requester, BOT_TEXT2("I already have at war checked for %factionname.", args));
                        return false;
                    }
                }
                
                bot->GetReputationMgr().SetAtWar(id, setwar);

                if(setwar)
                    ai->TellPlayerNoFacing(requester, BOT_TEXT2("Checked at war for %factionname", args));
                else
                    ai->TellPlayerNoFacing(requester, BOT_TEXT2("Unchecked at war for %factionname", args));

                return true;
            }

            ai->TellPlayerNoFacing(requester, ChatHelper::formatFaction(id, bot));

            factionFound = true;
        }
    }

    if (!factionFound)
    {
        if (factionName.empty())
        {
            ai->TellPlayerNoFacing(requester, BOT_TEXT2("I do not have any visible factions.", args));
        }
        else
        {
            args["%factionname"] = factionName;
            ai->TellPlayerNoFacing(requester, BOT_TEXT2("I do not know %factionname.", args));
        }
        return false;
    }

    return true;
}
