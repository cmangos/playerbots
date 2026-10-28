#ifndef _PLAYERBOT_UNSTUCKACTION_H
#define _PLAYERBOT_UNSTUCKACTION_H

#include "GenericActions.h"

class PlayerbotAI;

class UnstuckAction : public Action
{
public:
    UnstuckAction(PlayerbotAI* ai) : Action(ai, "unstuck") {}
    virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() { return "unstuck"; }
    virtual std::string GetHelpDescription()
    {
        return "This action attempts to unstuck the bot by choosing an appropriate method based on the event source.\n"
               "Methods include resetting position, using hearthstone, or repopping at the spirit healer.";
    }
    virtual std::vector<std::string> GetUsedActions() { return {"reset", "hearthstone", "repop"}; }
    virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif
};

class DropStandoffAction : public Action
{
public:
    DropStandoffAction(PlayerbotAI* ai) : Action(ai, "drop standoff") {}
    virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() { return "drop standoff"; }
    virtual std::string GetHelpDescription()
    {
        return "Drops a frozen combat stand-off: nobody exchanged damage while the bot stood still, so combat is stopped\n"
               "and targets cleared, letting the state machine re-decide (fresh engagement or non-combat agenda).";
    }
    virtual std::vector<std::string> GetUsedActions() { return {}; }
    virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif
};

class ReleaseStuckMovementAction : public Action
{
public:
    ReleaseStuckMovementAction(PlayerbotAI* ai) : Action(ai, "release stuck movement") {}
    virtual bool Execute(Event& event) override;

#ifdef GenerateBotHelp
    virtual std::string GetHelpName() { return "release stuck movement"; }
    virtual std::string GetHelpDescription()
    {
        return "Drops the current target when a movement action has been looping without the bot actually moving,\n"
               "losing health or changing target - the strategy machine then re-decides.";
    }
    virtual std::vector<std::string> GetUsedActions() { return {}; }
    virtual std::vector<std::string> GetUsedValues() { return { "movement loop age" }; }
#endif
};

#endif
