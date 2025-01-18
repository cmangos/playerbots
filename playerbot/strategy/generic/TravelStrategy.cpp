
#include "playerbot/playerbot.h"
#include "TravelStrategy.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

NextAction** TravelStrategy::GetDefaultNonCombatActions()
{
    return NextAction::array(0, new NextAction("travel", 1.0f), NULL);
}

void TravelStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    //The way these actions work:
    //The first one to trigger will attempt to get destinations async and block all lower actions from executing.
    //If a destination is found, it is set as new destination and none of the actions will trigger.
    //If no destinations are found the next action in line will start getting destinations.
    //Once "choose travel target" triggers all destination fetches are reset so we can try again from the top next time.

    //Todo group target

    //All the standard choose travel target actions using either a specific val trigger or a generic purpose trigger.
    const std::vector<std::tuple<std::string, TravelDestinationPurpose, float>> PurposeActions =
    {
        {"",TravelDestinationPurpose::Vendor, 6.95f},
        {"",TravelDestinationPurpose::AH, 6.94f},
        {"",TravelDestinationPurpose::Repair, 6.93f},
        {"val:and{no travel target,should get money,can get mail}", TravelDestinationPurpose::Mail, 6.79f},
        {"val:and{no travel target,should get money}", TravelDestinationPurpose::Grind, 6.77f},
        {"",TravelDestinationPurpose::Mail, 6.6f},
        {"",TravelDestinationPurpose::GatherMining, 6.5f},
        {"",TravelDestinationPurpose::GatherSkinning, 6.5f},
        {"",TravelDestinationPurpose::GatherHerbalism, 6.5f},
        //{"",TravelDestinationPurpose::GatherFishing, 6.5f},
        {"",TravelDestinationPurpose::Boss, 6.4f},
        {"",TravelDestinationPurpose::Explore, 6.29f},
        {"",TravelDestinationPurpose::GenericRpg, 6.28f},
        {"",TravelDestinationPurpose::Grind, 6.27f},
    };

    for (auto& [condition, purpose, relevance] : PurposeActions)
    {
        std::string trigger = condition.empty() ? "val::need travel purpose::" + std::to_string((uint32)purpose) : condition;
        triggers.push_back(new TriggerNode(
            trigger,
            NextAction::array(0, new NextAction("choose async travel target::" + std::to_string((uint32)purpose), relevance), NULL)));
    }

    //Specific named exceptions
    const std::vector<std::tuple<std::string, std::string, float>> StringActions =
    {
        {"no travel target","choose group travel target", 6.99f},
        {"val::should travel named::city","choose async named travel target::city", 6.85f},
        {"val:and{no travel target,has focus travel target}","choose async quest travel target", 6.84f},
        //{"val::should travel named::pvp","choose async named travel target::pvp", 6.83f},
        {"val:and{no travel target,should get money}", "choose async quest travel target", 6.78f},
        {"no travel target","refresh travel target", 6.7f},
        {"no travel target", "choose async quest travel target", 6.3f}
    };

    for (auto& [trigger, action, relevance] : StringActions)
    {
        triggers.push_back(new TriggerNode(
            trigger,
            NextAction::array(0, new NextAction(action, relevance), NULL)));
    }

    //Todo current target
    
    triggers.push_back(new TriggerNode(
        "no travel target",
        NextAction::array(0, new NextAction("choose travel target", 6.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "has nearby quest taker",
        NextAction::array(0, new NextAction("choose travel target", 6.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "far from travel target",
        NextAction::array(0, new NextAction("check mount state", 1), new NextAction("move to travel target", 1), NULL)));
}
