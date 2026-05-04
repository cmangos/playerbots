#pragma once
#include "playerbot/PlayerbotAI.h"
#include "TestContext.h"
#include <ctime>

namespace ai
{
    class BossFocusManager
    {
    public:
        BossFocusManager(Player* bot, PlayerbotAI* ai, TestContext& ctx)
            : bot(bot), ai(ai), ctx(ctx), lastFocusRespawnTime(0), lastObserveLogTime(0) {}

        void Update();
        void Reset() { lastFocusRespawnTime = 0; lastObserveLogTime = 0; }

    private:
        Creature* FindFocusMob();
        void RespawnFocusMob();
        void EngageFocusMob(Creature* focusMob);
        void LogObserveStatus();

        Player* bot;
        PlayerbotAI* ai;
        TestContext& ctx;
        uint32 lastFocusRespawnTime;
        uint32 lastObserveLogTime;
    };
}
