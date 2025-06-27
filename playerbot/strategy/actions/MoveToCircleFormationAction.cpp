#include "playerbot/playerbot.h"
#include "MoveToTravelTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/LootObjectStack.h"
#include "MotionGenerators/PathFinder.h"
#include "playerbot/TravelMgr.h"
#include "MoveToCircleFormationAction.h"

const float FORMATION_POSITION_THRESHOLD = 2.0f;

bool MoveToCircleFormationAction::ParseQualifierParameters(float& outRadius, float& outAngleDegrees, Player* requester_for_messaging)
{
    outRadius = 0.0f; // Initialize output parameters
    outAngleDegrees = 0.0f;

    std::string qualifierStr = getQualifier();
    std::vector<std::string> params = getMultiQualifiers(qualifierStr, ",");

    if (params.size() < 2)
    {
        if (requester_for_messaging) ai->TellPlayerNoFacing(requester_for_messaging, "MoveToCircleFormationAction: Qualifier expects <radius>,<angle_degrees>. Example: '10,90'");
        return false;
    }
    if (params.size() > 2) // Strict check for exactly two parameters
    {
        if (requester_for_messaging) ai->TellPlayerNoFacing(requester_for_messaging, "MoveToCircleFormationAction: Qualifier has too many parts (" + std::to_string(params.size()) + "). Expects <radius>,<angle_degrees>.");
        return false;
    }

    try
    {
        size_t sz_radius = 0;
        outRadius = std::stof(params[0], &sz_radius);
        if (sz_radius != params[0].length()) {
            throw std::invalid_argument("Invalid radius characters in qualifier");
        }

        size_t sz_angle = 0;
        outAngleDegrees = std::stof(params[1], &sz_angle);
        if (sz_angle != params[1].length()) {
            throw std::invalid_argument("Invalid angle characters in qualifier");
        }
    }
    catch (const std::invalid_argument& ia)
    {
        if (requester_for_messaging) {
            std::string errorMsg = "MoveToCircleFormationAction: Invalid radius/angle format in qualifier: ";
            errorMsg += ia.what(); errorMsg += ". Radius part='"; errorMsg += params[0];
            errorMsg += "', Angle part='"; errorMsg += params[1]; errorMsg += "'";
            ai->TellPlayerNoFacing(requester_for_messaging, errorMsg);
        }
        return false;
    }


    if (outRadius <= 0) {
        if (requester_for_messaging) ai->TellPlayerNoFacing(requester_for_messaging, "MoveToCircleFormationAction: Radius from qualifier must be positive.");
        return false;
    }

    return true; // Success
}

WorldPosition MoveToCircleFormationAction::CalculateFormationSpot(float radius, float angleDegrees, Unit* circleCenterTarget)
{
    float angleRadians = angleDegrees * (M_PI_F / 180.0f);
    WorldPosition centerPos(circleCenterTarget);

    float targetX = centerPos.getX() + radius * cos(angleRadians);
    float targetY = centerPos.getY() + radius * sin(angleRadians);

    WorldPosition calculatedPos(centerPos.getMapId(), targetX, targetY, centerPos.getZ());
    calculatedPos.setZ(calculatedPos.getHeight()); // Get ground Z

    if (!calculatedPos.isValid() || calculatedPos.getZ() == INVALID_HEIGHT)
    {
        calculatedPos.setZ(centerPos.getZ()); // Fallback
    }

    float botOrientation = atan2(centerPos.getY() - calculatedPos.getY(), centerPos.getX() - calculatedPos.getX());
    calculatedPos.setO(botOrientation);

    return calculatedPos; // Return the fully calculated position
}


bool MoveToCircleFormationAction::Execute(Event& event)
{ 
    //std::string paramsStr = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    //std::string qualifierStr = getQualifier();
    //std::vector<std::string> params = getMultiQualifiers(qualifierStr, ",");

    float radius, angleDegrees;

    if (!ParseQualifierParameters(radius, angleDegrees, requester))
    {
        return false; // Parsing failed, error message already sent by helper
    }
    Unit* circleCenterTarget = AI_VALUE(Unit*, "current target"); // Get the target

    WorldPosition targetSpot = CalculateFormationSpot(radius,angleDegrees, circleCenterTarget);

    if (requester) {
        std::ostringstream outMsg;
        outMsg << "Moving to radius " << radius << ", angle " << angleDegrees << " deg around " << ChatHelper::formatWorldobject(circleCenterTarget) << ". Target pos: "
               << targetSpot.getX() << "," << targetSpot.getY() << "," << targetSpot.getZ();
        ai->TellPlayerNoFacing(requester, outMsg.str());
    }

    bool moved = MoveTo(targetSpot.getMapId(),
                        targetSpot.getX(),
                        targetSpot.getY(),
                        targetSpot.getZ(),
                        false, 
                        true,  
                        false, 
                        false); 

    if (moved) {
        SetDuration(10000); 
    }

    return moved;
}

bool MoveToCircleFormationAction::isUseful()
{
    if (!MovementAction::isUseful()) // Base class checks (like "stay" strategy)
        return false;

    float radius, angleDegrees;

    // Suppress messages from ParseQualifierParameters during isUseful check
    Player* requester_for_parse_check = nullptr; 
    if (!ParseQualifierParameters(radius, angleDegrees, requester_for_parse_check))
    {
        return false; // Invalid parameters, not useful
    }

    WorldPosition targetSpot = CalculateFormationSpot(radius,angleDegrees, AI_VALUE(Unit*, "current target"));

    if (!targetSpot.isValid())
    {
        return false; // Cannot calculate a valid spot, so not useful to try moving
    }

    Player* bot = ai->GetBot();
    // Check if bot is already close to the target spot
    float distanceToTargetSpot = bot->GetDistance(targetSpot.getX(), targetSpot.getY(), targetSpot.getZ());
    return distanceToTargetSpot > FORMATION_POSITION_THRESHOLD;
}

bool MoveToCircleFormationAction::isPossible()
{
    if (!MovementAction::isPossible()) // Base class checks (canMove, not stunned etc.)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !sServerFacade.IsAlive(target)) return false; // No valid center target

    return true;
}