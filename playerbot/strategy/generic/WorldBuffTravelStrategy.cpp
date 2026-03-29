#include "playerbot/playerbot.h"
#include "WorldBuffTravelStrategy.h"
#include "playerbot/TravelMgr.h"

using namespace ai;

float WorldBuffTravelMultiplier::GetValue(Action* action)
{
    if (!action)
        return 1.0f;

    std::string name = action->getName();

    if (name == "choose travel target" || name == "request travel target" ||
        name == "request named travel target" || name == "request quest travel target" ||
        name == "choose group travel target" || name == "refresh travel target" ||
        name == "reset travel target" ||
        name == "rpg" || name == "choose rpg target" ||
        name == "grind" || name == "attack anything")
    {
        return 0.0f;
    }

    return 1.0f;
}

void WorldBuffTravelStrategy::InitNonCombatMultipliers(std::list<Multiplier*>& multipliers)
{
    multipliers.push_back(new WorldBuffTravelMultiplier(ai));
}

void WorldBuffTravelStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "world buff travel zone reached",
        NextAction::array(0, new NextAction("world buff travel apply", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel need move",
        NextAction::array(0, new NextAction("world buff travel set target", 10.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel dm buffed",
        NextAction::array(0, new NextAction("world buff travel dm buffed", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel dm exited",
        NextAction::array(0, new NextAction("world buff travel dm exited", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel dm portal cast",
        NextAction::array(0, new NextAction("world buff travel dm cast portal", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel dm portal use",
        NextAction::array(0, new NextAction("world buff travel dm take portal", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "val::travel target traveling",
        NextAction::array(0, new NextAction("check mount state", 1), new NextAction("move to travel target", 3), NULL)));

    triggers.push_back(new TriggerNode(
        "often",
        NextAction::array(0, new NextAction("travel", 3), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel portal step",
        NextAction::array(0, new NextAction("world buff travel cast portal", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel use portal",
        NextAction::array(0, new NextAction("world buff travel take portal", 15.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "world buff travel done",
        NextAction::array(0, new NextAction("world buff travel finish", 20.0f), NULL)));
}

void WorldBuffTravelStrategy::OnStrategyAdded(BotState state)
{
    WorldBuffTravelStep startStep = GetFirstNeededStep(ai->GetBot());

    ai->GetAiObjectContext()->GetValue<uint8>("world buff travel step")->Set(
        static_cast<uint8>(startStep));
}

void WorldBuffTravelStrategy::OnStrategyRemoved(BotState state)
{
    ai->GetAiObjectContext()->GetValue<uint8>("world buff travel step")->Set(
        static_cast<uint8>(WorldBuffTravelStep::STEP_DONE));
}