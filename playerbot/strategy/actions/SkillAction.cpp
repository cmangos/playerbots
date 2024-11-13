
#include "playerbot/playerbot.h"
#include "SkillAction.h"
#include "Tools/Language.h"

using namespace ai;

bool SkillAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string cmd = event.getParam();

    bool unlearn = (cmd.find("unlearn ") == 0);

    std::string skillName = cmd;

    if (unlearn)
        skillName = skillName.substr(8);

    std::wstring wnamepart;

    if (!Utf8toWStr(skillName, wnamepart))
        return false;

    std::set<uint32> skillIds = ChatHelper::ExtractAllSkillIds(skillName);

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    bool skillFound = false;
    std::map<std::string, std::string> args;

    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); ++id)
    {
        if (!bot->HasSkill(id))
            continue;

        SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(id);
        if (skillInfo)
        {
            int loc = requester->GetSession()->GetSessionDbcLocale();

            std::string name = skillInfo->name[loc];

            if (!skillName.empty() && skillIds.empty())
            {
                if (name.empty())
                    continue;

                if (!Utf8FitTo(name, wnamepart))
                {
                    loc = 0;
                    for (; loc < MAX_LOCALE; ++loc)
                    {
                        if (loc == requester->GetSession()->GetSessionDbcLocale())
                            continue;

                        name = skillInfo->name[loc];
                        if (name.empty())
                            continue;

                        if (Utf8FitTo(name, wnamepart))
                            break;
                    }
                }
            }

            if (skillName.empty() || skillIds.find(id) != skillIds.end() || (loc < MAX_LOCALE && skillIds.empty()))
            {
                if (unlearn)
                {
                    args["%skillname"] = name;

                    if (!bot->GetSkillInfo(uint16(id), ([](SkillRaceClassInfoEntry const& entry) { return (entry.flags & SKILL_FLAG_CAN_UNLEARN); })))
                    {
                        ai->TellPlayerNoFacing(requester, BOT_TEXT2("Unable to unlearn %skillname", args));
                        return false;
                    }

                    bot->SetSkillStep(uint16(id), 0);

                    ai->TellPlayerNoFacing(requester, BOT_TEXT2("Unlearned %skillname", args));

                    return true;
                }

                char valStr[50] = "";
                char const* knownStr = "";
                knownStr = requester->GetSession()->GetMangosString(LANG_KNOWN);
                uint32 curValue = bot->GetSkillValuePure(id);
                uint32 maxValue = bot->GetSkillMaxPure(id);
                uint32 permValue = bot->GetSkillBonusPermanent(id);
                uint32 tempValue = bot->GetSkillBonusTemporary(id);

                char const* valFormat = requester->GetSession()->GetMangosString(LANG_SKILL_VALUES);
                snprintf(valStr, 50, valFormat, curValue, maxValue, permValue, tempValue);
                std::ostringstream out;
                out << "|cffffffff|Hskill:";
                out << id;
                out << "|h[";
                out << name;
                out << "]|h|r (";
                
                out << curValue << "/" << maxValue;

                if (permValue)
                    out << " +perm " << permValue;

                if (tempValue)
                    out << " +temp " << permValue;

                out << ")";

                ai->TellPlayerNoFacing(requester, out);

                skillFound = true;
            }
        }
    }

    if (!skillFound)
    {
        if (skillName.empty())
        {
            ai->TellPlayerNoFacing(requester, BOT_TEXT2("No skills found.", args));
        }
        else
        {
            args["%skillname"] = skillName;
            ai->TellPlayerNoFacing(requester, BOT_TEXT2("Skill %skillname not found.", args));
        }
        return false;
    }

    return true;
}
