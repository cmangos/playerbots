#pragma once
#include "MovementActions.h"
#include "playerbot/AiFactory.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/ItemVisitors.h"
#include "playerbot/RandomPlayerbotMgr.h"
#include "BattleGround/BattleGround.h"
#include "BattleGround/BattleGroundMgr.h"
#include "BattleGround/BattleGroundWS.h"
#include "ChooseTargetActions.h"
#include "CheckMountStateAction.h"
#include "G3D/Vector3.h"
#include "Entities/GameObject.h"

using namespace ai;


class BGJoinAction : public Action
{
public:
    BGJoinAction(PlayerbotAI* ai, std::string name = "bg join") : Action(ai, name) {}
    virtual bool Execute(Event& event);
    virtual bool isUseful();
    virtual bool canJoinBg(Player* player, BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId);
    virtual bool shouldJoinBg(BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId);
#ifndef MANGOSBOT_ZERO
    virtual bool gatherArenaTeam(ArenaType type);
#endif
protected:
    bool JoinQueue(uint32 type);
    std::vector<uint32> bgList;
    std::vector<uint32> ratedList;
};

class FreeBGJoinAction : public BGJoinAction
{
public:
    FreeBGJoinAction(PlayerbotAI* ai, std::string name = "free bg join") : BGJoinAction(ai, name) {}
    virtual bool shouldJoinBg(BattleGroundQueueTypeId queueTypeId, BattleGroundBracketId bracketId);
};

class BGLeaveAction : public Action
{
public:
    BGLeaveAction(PlayerbotAI* ai, std::string name = "bg leave") : Action(ai) {}
    virtual bool Execute(Event& event);
};

class BGStatusAction : public Action
{
public:
    BGStatusAction(PlayerbotAI* ai) : Action(ai, "bg status") {}
    virtual bool Execute(Event& event);
    virtual bool isUseful();
};

class BGStatusCheckAction : public Action
{
public:
    BGStatusCheckAction(PlayerbotAI* ai, std::string name = "bg status check") : Action(ai, name) {}
    virtual bool Execute(Event& event);
    virtual bool isUseful();
};