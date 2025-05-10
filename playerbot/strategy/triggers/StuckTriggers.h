#pragma once
#include "playerbot/strategy/Trigger.h"
#include <MotionGenerators/MoveMap.h>

namespace ai
{
    class MoveStuckTrigger : public Trigger
    {
    public:
        MoveStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "move stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->HasActivePlayerMaster())
                return false;


            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            LogCalculatedValue<WorldPosition>* posVal = dynamic_cast<LogCalculatedValue<WorldPosition>*>(context->GetUntypedValue("current position"));

            if (!ai->AllowActivity(ALL_ACTIVITY))
            {
                posVal->Reset();
                return false;
            }

            WorldPosition botPos(bot);

            if (posVal->LastChangeDelay() > 5 * MINUTE)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in the same position for %d seconds", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

                return true;
            }

            bool longLog = false;

            for (auto tPos : posVal->ValueLog())
            {
                uint32 timePassed = time(0) - tPos.second;

                if (timePassed > 10 * MINUTE)
                {
                    if (botPos.fDist(tPos.first) > 50.0f)
                        return false;

                    longLog = true;
                }
            }

            if (longLog)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in the same position for 10mins", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

            }

            return longLog;
        }
    };

    class MoveLongStuckTrigger : public Trigger
    {
    public:
        MoveLongStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "move long stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            LogCalculatedValue<WorldPosition>* posVal = dynamic_cast<LogCalculatedValue<WorldPosition>*>(context->GetUntypedValue("current position"));
            MemoryCalculatedValue<uint32>* expVal = dynamic_cast<MemoryCalculatedValue<uint32>*>(context->GetUntypedValue("experience"));

            if (!ai->AllowActivity(ALL_ACTIVITY))
            {
                posVal->Reset();
                expVal->Reset();
                return false;
            }

            WorldPosition botPos(bot);

            Cell const& cell = bot->GetCurrentCell();

            GridPair grid = botPos.getGridPair();

            if (grid.x_coord < 0 || grid.x_coord >= MAX_NUMBER_OF_GRIDS)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in grid %d,%d on map %d", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), grid.x_coord, grid.y_coord, botPos.getMapId());

                return true;
            }

            if (grid.y_coord < 0 || grid.y_coord >= MAX_NUMBER_OF_GRIDS)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in grid %d,%d on map %d", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), grid.x_coord, grid.y_coord, botPos.getMapId());

                return true;
            }

#ifdef MANGOSBOT_TWO
            if (cell.GridX() > 0 && cell.GridY() > 0 && !MMAP::MMapFactory::createOrGetMMapManager()->IsMMapTileLoaded(botPos.getMapId(), 0, cell.GridX(), cell.GridY()) && !MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), botPos.getMapId(), 0, cell.GridX(), cell.GridY(), 0))
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in unloaded grid %d,%d on map %d", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), grid.x_coord, grid.y_coord, botPos.getMapId());

                return true;
            }
#else
            if (cell.GridX() > 0 && cell.GridY() > 0 && !MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(botPos.getMapId(), cell.GridX(), cell.GridY()) 
                && !MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), botPos.getMapId(), cell.GridX(), cell.GridY()))
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in unloaded grid %d,%d on map %d", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), grid.x_coord, grid.y_coord, botPos.getMapId());

                return true;
            }
#endif

            if (posVal->LastChangeDelay() > 10 * MINUTE)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in the same position for %d seconds", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

                return true;
            }

            if (expVal->LastChangeDelay() < 15 * MINUTE)
                return false;

            bool longLog = false;

            for (auto tPos : posVal->ValueLog())
            {
                uint32 timePassed = time(0) - tPos.second;

                if (timePassed > 15 * MINUTE)
                {
                    if (botPos.fDist(tPos.first) > 50.0f)
                        return false;

                    longLog = true;
                }
            }

            if (longLog)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in the same position for 15mins", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());
            }

            return longLog;
        }
    };

    class CombatStuckTrigger : public Trigger
    {
    public:
        CombatStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "combat stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->GetState() != BotState::BOT_STATE_COMBAT)
                return false;

            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            if (!ai->AllowActivity(ALL_ACTIVITY))
                return false;

            WorldPosition botPos(bot);

            MemoryCalculatedValue<bool>* combatVal = dynamic_cast<MemoryCalculatedValue<bool>*>(context->GetUntypedValue("combat::self target"));

            if (combatVal->LastChangeDelay() > 5 * MINUTE)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in combat for %d seconds", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

                return true;
            }

            if (bot->duel && bot->duel->startTime - time(0) > 15 * MINUTE)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in combat for %d seconds", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

                return true;
            }

            return false;
        }
    };

    class CombatLongStuckTrigger : public Trigger
    {
    public:
        CombatLongStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "combat long stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->GetState() != BotState::BOT_STATE_COMBAT)
                return false;

            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            if (!ai->AllowActivity(ALL_ACTIVITY))
                return false;

            WorldPosition botPos(bot);

            if (MEM_AI_VALUE(bool,"combat::self target")->LastChangeDelay() > 15 * MINUTE)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in combat for %d seconds", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

                return true;
            }

            if (bot->duel && bot->duel->startTime - time(0) > 15 * MINUTE)
            {
                //sLog.outBasic("Bot #%d %s:%d <%s> was in combat for %d seconds", bot->GetGUIDLow(), bot->GetTeam() == ALLIANCE ? "A" : "H", bot->getLevel(), bot->GetName(), posVal->LastChangeDelay());

                return true;
            }

            return false;
        }
    };

    class LeaderIsAfkTrigger : public Trigger
    {
    public:
        LeaderIsAfkTrigger(PlayerbotAI* ai) : Trigger(ai, "leader is afk", 10) {}

        virtual bool IsActive()
        {
            if (ai->HasRealPlayerMaster())
                return false;

            if (Group* group = bot->GetGroup())
            {
                Player* leader = sObjectMgr.GetPlayer(group->GetLeaderGuid(), true);
                if (!leader)
                    return false;

                return leader->isAFK();
            }

            return false;
        }
    };
}
