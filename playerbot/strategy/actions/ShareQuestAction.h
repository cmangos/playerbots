#pragma once
#include "GenericActions.h"

namespace ai
{
    class ShareQuestAction : public ChatCommandAction
    {
    public:
        ShareQuestAction(PlayerbotAI* ai, std::string name = "share quest") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
    };

    class AutoShareQuestAction : public ShareQuestAction
    {
    public:
        AutoShareQuestAction(PlayerbotAI* ai) : ShareQuestAction(ai, "auto share quest") {}
        virtual bool Execute(Event& event) override;

        virtual bool isUseful() override { return bot->GetGroup() && !ai->HasActivePlayerMaster(); }
    };
}
