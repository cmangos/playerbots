#pragma once

#include "playerbot/playerbot.h"
#include "playerbot/Talentspec.h"
#include "GenericActions.h"

namespace ai
{
    class ChangeTalentsAction : public ChatCommandAction
    {
    public:
        ChangeTalentsAction(PlayerbotAI* ai, std::string name = "talents") : ChatCommandAction(ai, name) {}

        virtual bool isUsefulWhenStunned() override { return true; }

    public:
        virtual bool Execute(Event& event) override;
        static bool AutoSelectTalents(Player* bot, std::ostringstream* out, BotRoles role = BotRoles::BOT_ROLE_NONE);
    private:
        static std::vector<TalentPath*> getPremadePaths(uint8 cls, std::string findName, BotRoles role = BotRoles::BOT_ROLE_NONE);
        static std::vector<TalentPath*> getPremadePaths(Player* bot, TalentSpec* oldSpec);
        static TalentPath* getPremadePath(uint8 cls, int id);
        static void listPremadePaths(uint8 cls, std::vector<TalentPath*> paths, std::ostringstream* out);
        static TalentPath* PickPremadePath(std::vector<TalentPath*> paths, bool useProbability);
        static TalentSpec* GetBestPremadeSpec(Player* bot, int spec);
    };

    class AutoSetTalentsAction : public ChangeTalentsAction 
    {
    public:
        AutoSetTalentsAction(PlayerbotAI* ai) : ChangeTalentsAction(ai, "auto talents") {}
        virtual bool Execute(Event& event) override;
        virtual bool isUsefulWhenStunned() override { return true; }
    };
}