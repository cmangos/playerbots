#pragma once

#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/Value.h"

namespace ai
{
    class RemoteServicesAction : public Action
    {
    public:
        RemoteServicesAction(PlayerbotAI* ai) : Action(ai, "remote services") {}
        bool Execute(Event& event) override;
        bool isUseful() override { return IsReady(ai); }
        static bool IsReady(PlayerbotAI* ai);
    };

    class RemoteServicesReadyValue : public BoolCalculatedValue
    {
    public:
        RemoteServicesReadyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "remote services ready", 2) {}
        bool Calculate() override { return RemoteServicesAction::IsReady(ai); }
    };
}
