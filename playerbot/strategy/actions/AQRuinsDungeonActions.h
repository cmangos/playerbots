#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"

namespace ai
{
    class AQRuinsEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        AQRuinsEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable aq ruins strategy", "+aq ruins") {}
    };

    class AQRuinsDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        AQRuinsDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable aq ruins strategy", "-aq ruins") {}
    };

    class KurinnaxxEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
       KurinnaxxEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable kurinnaxx fight strategy", "+kurinnaxx") {}
    };

    class KurinnaxxDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
       KurinnaxxDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable kurinnaxx fight strategy", "-kurinnaxx") {}
    };

    class OssirianEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        OssirianEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable ossirian fight strategy", "+ossirian") {}
    };

    class OssirianDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
       OssirianDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable ossirian fight strategy", "-ossirian") {}
    };
}