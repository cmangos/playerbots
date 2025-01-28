
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
        {"",TravelDestinationPurpose::Vendor, 6.95f},                                         //See PassTrough 90%
        {"",TravelDestinationPurpose::AH, 6.94f},                                                           // 90%
        {"",TravelDestinationPurpose::Repair, 6.93f},                                                       // 90%
        {"val::and::{not::travel target active,should get money,can get mail}", TravelDestinationPurpose::Mail, 6.79f}, //100%
        {"val::and::{not::travel target active,should get money}", TravelDestinationPurpose::Grind, 6.77f},             // 90%
        {"",TravelDestinationPurpose::Mail, 6.6f},                                                          // 70%
        {"",TravelDestinationPurpose::GatherMining, 6.5f},                                                  // 90%/40% in group
        {"",TravelDestinationPurpose::GatherSkinning, 6.5f},                                                // 90%/40% in group
        {"",TravelDestinationPurpose::GatherHerbalism, 6.5f},                                               // 90%/40% in group
        //{"",TravelDestinationPurpose::GatherFishing, 6.5f},                                               // 90%/40% in group
        {"",TravelDestinationPurpose::Boss, 6.4f},                                                          // 50%
        {"",TravelDestinationPurpose::Explore, 6.29f},                                                      // 10%
        {"",TravelDestinationPurpose::GenericRpg, 6.28f},                                                   // 50%
        {"",TravelDestinationPurpose::Grind, 6.27f},                                                        // 100%
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
        {"no travel target","choose group travel target", 6.99f},                         //See PassTrough 100%
        {"val::should travel named::city","choose async named travel target::city", 6.85f},              // 10%
        {"val::and::{has strategy::rpg quest,not::travel target active,has focus travel target}","choose async quest travel target", 6.84f}, //100%
        //{"val::should travel named::pvp","choose async named travel target::pvp", 6.83f},              // 25%
        {"val::and::{has strategy::rpg quest,not::travel target active,should get money}", "choose async quest travel target", 6.78f},       // 90%
        {"no travel target","refresh travel target", 6.7f},                                              // 90%
        {"val::and::{has strategy::rpg quest,not::travel target active}", "choose async quest travel target", 6.3f}                                   // 95%
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
