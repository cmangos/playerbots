#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{
    class AQRuinsEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
       AQRuinsEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter aq ruins", "aq ruins", 509) {}
    };

    class AQRuinsLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
       AQRuinsLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave aq ruins", "aq ruins", 509) {}
    };

    class KurinnaxxStartFightTrigger : public StartBossFightTrigger
    {
    public:
       KurinnaxxStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start kurinnaxx fight", "kurinnaxx", 15348) {}
    };

    class KurinnaxxEndFightTrigger : public EndBossFightTrigger
    {
    public:
       KurinnaxxEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end kurinnaxx fight", "kurinnaxx", 15348) {}
    };

    class OssirianStartFightTrigger : public StartBossFightTrigger
    {
    public:
       OssirianStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start ossirian fight", "ossirian", 15339) {}
    };

    class OssirianEndFightTrigger : public EndBossFightTrigger
    {
    public:
       OssirianEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end ossirian fight", "ossirian", 15339) {}
    };
}