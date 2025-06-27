
#include "playerbot/playerbot.h"
#include "CircleFormationStrategy.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/AiObjectContext.h"
#include "playerbot/strategy/values/PositionValue.h"

using namespace ai;

void CircleFormationStrategy::InitCombatTriggers(std::list<TriggerNode*>& triggers)
{
  
    
    Player* bot = ai->GetBot();
    //PlayerbotAI* ai = bot->GetPlayerbotAI();

    std::string role = GetBotRole(bot); 
    float radius = GetRadiusForRole(role);

    float angleDegrees = 0; 
    Group* group = bot->GetGroup();
    int totalBotsInGroup = 1; 
    int botIndexInGroup = 0;    

    if (group)
    {
        botIndexInGroup = GetBotGroupIndex(bot, group, totalBotsInGroup);
    }
    
    if (totalBotsInGroup > 0 && botIndexInGroup != -1)
    {
        float angleIncrement = (totalBotsInGroup > 0) ? (360.0f / totalBotsInGroup) : 0.0f;
        angleDegrees = botIndexInGroup * angleIncrement;

        // Optional: Offset angles based on current target's orientation
        Unit* currentTarget = ai->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();  // Target bot will form around
        if (currentTarget) {
            float targetOrientationDegrees = currentTarget->GetOrientation() * (180.0f / M_PI_F);
            angleDegrees = fmod(targetOrientationDegrees + angleDegrees, 360.0f);
            if (angleDegrees < 0) angleDegrees += 360.0f; // Ensure positive angle
        }

    }
    else 
    {
        angleDegrees = (bot->GetObjectGuid().GetCounter() % 24) * 15.0f;
        // ai->TellPlayerNoFacing(GetMaster(), "Warning: Bot " + bot->GetName() + " cound not determine group index, using default angle " + std::to_string(angleDegrees));
    }

    std::ostringstream qualifierStream;
    qualifierStream.precision(1); // For one decimal place for float
    qualifierStream << std::fixed << radius << "," << angleDegrees;
    std::string actionQualifier = qualifierStream.str();
    
    // For debugging the generated qualifier:
    // if (GetMaster()) ai->TellPlayerNoFacing(GetMaster(), "CircleFormationStrategy: Bot " + bot->GetName() + " (Role: " + role + ") using qualifier: \"" + actionQualifier + "\" for move to circle formation");


    triggers.push_back(new TriggerNode(
        "combat start", 
        NextAction::array(0, new NextAction("move to circle formation::" + actionQualifier, ACTION_EMERGENCY+10), NULL))); 
    
        triggers.push_back(new TriggerNode(
        "often", 
        NextAction::array(0, 
                            new NextAction("say::often circle formation", ACTION_EMERGENCY), 
                            new NextAction("move to circle formation::" + actionQualifier, ACTION_EMERGENCY+10), // Ensure this action has high priority
                            NULL))); 
}


void CircleFormationStrategy::InitNonCombatTriggers(std::list<TriggerNode*>& triggers)
{
    /*
    triggers.push_back(new TriggerNode(
        "combat start",
        NextAction::array(0, new NextAction("move to circle formation::30,10", 100.0f), new NextAction("say::combat start", 99.0f), NULL)));


    triggers.push_back(new TriggerNode(
        "very often",
        NextAction::array(0,new NextAction("move to circle formation::30,10", 100.0f),  NULL)));
        */
 
}

std::string CircleFormationStrategy::GetBotRole(Player* pBot)
{
    PlayerbotAI* botAi = pBot->GetPlayerbotAI();
    if (botAi->HasStrategy("ranged", BotState::BOT_STATE_COMBAT))
    {
        return "ranged";
    }
    else
    {
        return "melee";
    }
  
}

float CircleFormationStrategy::GetRadiusForRole(const std::string& roleName)
{
    if (roleName == "melee")
        return 2;
    else // ranged (and default)
        return 30;
}

int CircleFormationStrategy::GetBotGroupIndex(Player* pBot, Group* group, int& outTotalBotsInFormation)
{
    outTotalBotsInFormation = 0;
    if (!pBot || !group) return -1;

    std::vector<ObjectGuid> groupBotGuids;
    for (GroupReference* ref = group->GetFirstMember(); ref != nullptr; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (member && !member->IsPlayer() && member->IsAlive() && member->GetMapId() == pBot->GetMapId())
        {
            groupBotGuids.push_back(member->GetObjectGuid());
            // We count all playerbots in the group for the total slots,
            // assuming they will all attempt some form of positioning.
            // If only bots with THIS strategy should be counted, the check needs to be more specific.
        }
    }
    
    outTotalBotsInFormation = groupBotGuids.size(); // Total is simply the number of valid playerbots found
    if (outTotalBotsInFormation == 0) return -1;

    std::sort(groupBotGuids.begin(), groupBotGuids.end());

    for (int i = 0; i < groupBotGuids.size(); ++i)
    {
        if (groupBotGuids[i] == pBot->GetObjectGuid())
        {
            return i; 
        }
    }
    return -1; 
}