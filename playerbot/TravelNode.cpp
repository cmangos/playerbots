#include "TravelNode.h"
#include "TravelMgr.h"

#include <iomanip>
#include <regex>

#include "ObjectMgr.h"
#include "PlayerbotAI.h"
#include "MoveMapSharedDefines.h"
#include "MotionGenerators/PathFinder.h"
#include "Entities/Transports.h"
#include "strategy/values/BudgetValues.h"
#include "ServerFacade.h"

using namespace ai;
using namespace MaNGOS;

//TravelNodePath(float distance = 0.1f, float extraCost = 0, TravelNodePathType pathType = TravelNodePathType::walk, uint64 pathObject = 0, bool calculated = false, vector<uint8> maxLevelCreature = { 0,0,0 }, float swimDistance = 0)
string TravelNodePath::print()
{
    ostringstream out;
    out << std::fixed << std::setprecision(1);
    out << distance << "f,";
    out << extraCost << "f,";
    out << to_string(uint8(pathType)) << ",";
    out << pathObject << ",";
    out << (calculated ? "true" : "false") << ",";
    out << to_string(maxLevelCreature[0]) << "," << to_string(maxLevelCreature[1]) << "," << to_string(maxLevelCreature[2]) << ",";
    out << swimDistance << "f";

    return out.str().c_str();
}

//Gets the extra information needed to properly calculate the cost.
void TravelNodePath::calculateCost(bool distanceOnly)
{
    std::unordered_map<FactionTemplateEntry const*, bool> aReact, hReact;

    bool aFriend, hFriend;

    if (calculated)
        return;

    try
    {

        distance = 0.1f;
        maxLevelCreature = { 0,0,0 };
        swimDistance = 0;

        WorldPosition lastPoint = WorldPosition();
        for (auto& point : path)
        {
            if (!distanceOnly)
                for (auto& creaturePair : point.getCreaturesNear(50)) //Agro radius + 5
                {
                    CreatureData const cData = creaturePair->second;
                    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

                    if (cInfo)
                    {
                        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);

                        if (aReact.find(factionEntry) == aReact.end())
                            aReact.insert(make_pair(factionEntry, PlayerbotAI::friendToAlliance(factionEntry)));
                        aFriend = aReact.find(factionEntry)->second;

                        if (hReact.find(factionEntry) == hReact.end())
                            hReact.insert(make_pair(factionEntry, PlayerbotAI::friendToHorde(factionEntry)));
                        hFriend = hReact.find(factionEntry)->second;

                        if (maxLevelCreature[0] < cInfo->MaxLevel && !aFriend && !hFriend)
                            maxLevelCreature[0] = cInfo->MaxLevel;
                        if (maxLevelCreature[1] < cInfo->MaxLevel && aFriend && !hFriend)
                            maxLevelCreature[1] = cInfo->MaxLevel;
                        if (maxLevelCreature[2] < cInfo->MaxLevel && !aFriend && hFriend)
                            maxLevelCreature[2] = cInfo->MaxLevel;
                    }
                }

            if (lastPoint && point.getMapId() == lastPoint.getMapId())
            {
                if (!distanceOnly && (point.isInWater() || lastPoint.isInWater()))
                    swimDistance += point.distance(lastPoint);

                distance += point.distance(lastPoint);
            }

            lastPoint = point;
        }

        if (!distanceOnly)
            calculated = true;
    }
    catch (...)
    {
    }
}

//The cost to travel this path. 
float TravelNodePath::getCost(Unit* unit, uint32 cGold)
{
    float modifier = 1.0f; //Global modifier
    float timeCost = 0.1f;
    float runDistance = distance - swimDistance;
    float speed = 8.0f; //default run speed
    float swimSpeed = 4.0f; //default swim speed.

    Player* bot = dynamic_cast<Player*>(unit);
    if (bot)
    {
        //Check if we can use this area trigger.
        if (getPathType() == TravelNodePathType::areaTrigger && pathObject)
        {
            uint32 triggerId = getPathObject();
            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(pathObject);
            AreaTrigger const* at = sObjectMgr.GetAreaTrigger(pathObject);
            if (atEntry && at && atEntry->mapid == bot->GetMapId())
            {
                Map* map = WorldPosition(atEntry->mapid, atEntry->box_x, atEntry->box_y, atEntry->box_z).getMap();
                if (map)
                    if (at && at->conditionId && !sObjectMgr.IsConditionSatisfied(at->conditionId, bot, map, nullptr, CONDITION_FROM_AREATRIGGER_TELEPORT))
                        return -1;
            }
        }

        if (getPathType() == TravelNodePathType::staticPortal && pathObject)
        {
            uint32 goEntry = getPathObject();

            auto data = sGOStorage.LookupEntry<GameObjectInfo>(goEntry);

            if (!data)
                return -1;

            FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(data->faction);

            if(factionEntry)
                if (PlayerbotAI::GetFactionReaction(factionEntry, bot->GetFactionTemplateEntry()) < REP_NEUTRAL)
                    return -1;
        }

        if (getPathType() == TravelNodePathType::flightPath && pathObject)
        {
            if (!bot->IsAlive())
                return -1;

            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

            if (!taxiPath)
                return -1;

            if (!bot->isTaxiCheater() && taxiPath->price > cGold)
                return -1;

            if (!bot->isTaxiCheater() && !bot->m_taxi.IsTaximaskNodeKnown(taxiPath->to))
                return -1;

            TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
            TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);

            if (!startTaxiNode || !endTaxiNode || !startTaxiNode->MountCreatureID[bot->GetTeam() == ALLIANCE ? 1 : 0] || !endTaxiNode->MountCreatureID[bot->GetTeam() == ALLIANCE ? 1 : 0])
                return -1;
        }

        speed = bot->GetSpeed(MOVE_RUN);
        swimSpeed = bot->GetSpeed(MOVE_SWIM);

        if (bot->HasSpell(1066))
            swimSpeed *= 1.5;

        uint32 level = bot->GetLevel();
        bool isAlliance = PlayerbotAI::friendToAlliance(bot->GetFactionTemplateEntry());

        int factionAnnoyance = 0;

        if (maxLevelCreature.size() > 0)
        {
            int mobAnnoyance = (maxLevelCreature[0] - level) - 10; //Mobs 10 levels below do not bother us.

            if (isAlliance)
                factionAnnoyance = (maxLevelCreature[2] - level) - 10;              //Opposite faction below 10 do not bother us.
            else if (!isAlliance)
                factionAnnoyance = (maxLevelCreature[1] - level) - 10;

            if (mobAnnoyance > 0)
                modifier += 0.1 * mobAnnoyance;     //For each level the whole path takes 10% longer.
            if (factionAnnoyance > 0)
                modifier += 0.3 * factionAnnoyance; //For each level the whole path takes 30% longer.
        }
    }
    else if (getPathType() == TravelNodePathType::flightPath)
        return -1;


    if (getPathType() != TravelNodePathType::walk)
        timeCost = extraCost * modifier;
    else
        timeCost = (runDistance / speed + swimDistance / swimSpeed) * modifier;

    return timeCost;
}

uint32 TravelNodePath::getPrice()
{
    if (getPathType() != TravelNodePathType::flightPath)
        return 0;

    if (!pathObject)
        return 0;

    TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

    return taxiPath->price;
}

//Creates or appends the path from one node to another. Returns if the path.
TravelNodePath* TravelNode::buildPath(TravelNode* endNode, Unit* bot, bool postProcess)
{
    if (getMapId() != endNode->getMapId())
        return nullptr;

    TravelNodePath* returnNodePath;

    if (!hasPathTo(endNode))                                //Create path if it doesn't exists
        returnNodePath = setPathTo(endNode, TravelNodePath(), false);
    else
        returnNodePath = getPathTo(endNode);                //Get the exsisting path.

    if (returnNodePath->getComplete())                      //Path is already complete. Return it.
        return returnNodePath;

    vector<WorldPosition> path = returnNodePath->getPath();

    if (path.empty())
        path = { *getPosition() };                      //Start the path from the current Node.

    WorldPosition* endPos = endNode->getPosition();     //Build the path to the end Node.

    path = endPos->getPathFromPath(path, bot);          //Pathfind from the existing path to the end Node.

    bool canPath = endPos->isPathTo(path);              //Check if we reached our destination.

    if (!canPath && endNode->hasLinkTo(this)) //Unable to find a path? See if the reverse is possible.
    {
        TravelNodePath backNodePath = *endNode->getPathTo(this);

        if (backNodePath.getPathType() == TravelNodePathType::walk)
        {
            vector<WorldPosition> bPath = backNodePath.getPath();

            if (!backNodePath.getComplete()) //Build it if it's not already complete.
            {
                if (bPath.empty())
                    bPath = { *endNode->getPosition() };            //Start the path from the end Node.

                WorldPosition* thisPos = getPosition();             //Build the path to this Node.

                bPath = thisPos->getPathFromPath(bPath, bot);         //Pathfind from the existing path to the this Node.

                canPath = thisPos->isPathTo(bPath);              //Check if we reached our destination.
            }
            else
                canPath = true;

            if (canPath)
            {
                std::reverse(bPath.begin(), bPath.end());
                path = bPath;
            }
        }
    }    

    //Transports are (probably?) not solid at this moment. We need to walk over them so we need extra code for this.
    //Some portals are 'too' solid so we can't properly walk in them. Again we need to bypass this.
    if (!isTransport() && !isPortal() && (endNode->isPortal() || endNode->isTransport()))
    {
        if (endNode->isTransport() && path.back().isInWater()) //Do not swim to boats.
            canPath = false;
        else if (!canPath && endPos->isPathTo(path, 20.0f)) //Cheat a little for transports and portals.
        {
            path.push_back(*endPos);
            canPath = true;

            if (!endNode->hasPathTo(this) || !endNode->getPathTo(this)->getComplete())
            {
                vector<WorldPosition> reversePath = path;
                reverse(reversePath.begin(), reversePath.end());

                TravelNodePath* backNodePath = endNode->setPathTo(this, TravelNodePath(), false);

                backNodePath->setComplete(canPath);

                endNode->setLinkTo(this, true);

                backNodePath->setPath(reversePath);

                backNodePath->calculateCost(!postProcess);
            }
        }
    }

    if (isTransport() && path.size() > 1)
    {
        WorldPosition secondPos = *std::next(path.begin()); //This is to prevent bots from jumping in the water from a transport. Need to remove this when transports are properly handled.
        if (secondPos.getMap() && secondPos.getTerrain() && secondPos.isInWater())
            canPath = false;
    }

    returnNodePath->setComplete(canPath);

    if (canPath && !hasLinkTo(endNode))
        setLinkTo(endNode, true);

    returnNodePath->setPath(path);

    if (!returnNodePath->getCalculated())
    {
        returnNodePath->calculateCost(!postProcess);
    }

    if (canPath && endNode->hasPathTo(this) && !endNode->hasLinkTo(this))
    {
        TravelNodePath* backNodePath = endNode->getPathTo(this);

        vector<WorldPosition> reversePath = path;
        reverse(reversePath.begin(), reversePath.end());
        backNodePath->setPath(reversePath);
        endNode->setLinkTo(this, true);

        if (!backNodePath->getCalculated())
        {
            backNodePath->calculateCost(!postProcess);
        }
    }

    return returnNodePath;
}


//Generic routine to remove references to nodes. 
void TravelNode::removeLinkTo(TravelNode* node, bool removePaths) {

    if (node) //Unlink this specific node
    {
        if (removePaths)
            paths.erase(node);

        links.erase(node);
        routes.erase(node);
    }
    else { //Remove all references to this node.        
        for (auto& node : sTravelNodeMap.getNodes())
        {
            if (node->hasPathTo(this))
                node->removeLinkTo(this, removePaths);
        }
        links.clear();
        paths.clear();
        routes.clear();
    }
}

vector<TravelNode*> TravelNode::getNodeMap(bool importantOnly, vector<TravelNode*> ignoreNodes, bool mapOnly)
{
    vector<TravelNode*> openList;
    vector<TravelNode*> closeList;

    openList.push_back(this);

    uint32 i = 0;

    while (i < openList.size())
    {
        TravelNode* currentNode = openList[i];

        i++;

        if (!importantOnly || currentNode->isImportant())
            closeList.push_back(currentNode);

        for (auto& nextPath : *currentNode->getLinks())
        {
            TravelNode* nextNode = nextPath.first;

            if (mapOnly && nextNode->getMapId() != getMapId())
                continue;

            if (std::find(openList.begin(), openList.end(), nextNode) != openList.end())
                continue;

            if (!ignoreNodes.empty() && std::find(ignoreNodes.begin(), ignoreNodes.end(), nextNode) != ignoreNodes.end())
                continue;

            openList.push_back(nextNode);
        }
    }

    return closeList;
}

bool TravelNode::isUselessLink(TravelNode* farNode)
{
    if (getPathTo(farNode)->getPathType() != TravelNodePathType::walk)
        return false;

    float farLength;
    TravelNodePath* farPath = nullptr;
    if (hasLinkTo(farNode))
    {
        farPath = getPathTo(farNode);
        farLength = farPath->getDistance();
    }
    else
        farLength = getDistance(farNode);

    for (auto& link : *getLinks())
    {
        TravelNode* nearNode = link.first;
        WorldPosition nearPos = *nearNode->getPosition();
        float nearLength = link.second->getDistance();

        if (farNode == nearNode)
            continue;

        if (farNode->hasLinkTo(this) && !nearNode->hasLinkTo(this))
            continue;

        if (nearNode->hasLinkTo(farNode))
        {
            //Is it quicker to go past second node to reach first node instead of going directly?
            if (nearLength + nearNode->linkDistanceTo(farNode) < farLength * 1.1)
                return true;

            //Does the path come across the nearby node.
            if (farPath)
                if (nearPos.closestSq(farPath->getPath()).distance(nearPos) < INTERACTION_DISTANCE)
                    return true;
        }
        else
        {
            if (!nearNode->hasRouteTo(farNode, true))
                continue;

            TravelNodeRoute route = sTravelNodeMap.getRoute(nearNode, farNode, nullptr);

            if (route.isEmpty())
                continue;

            if (route.hasNode(this))
                continue;

            //Is it quicker to go past second (and multiple) nodes to reach the first node instead of going directly?
            if (nearLength + route.getTotalDistance() < farLength * 1.1)
                return true;
        }
    }

    return false;
}

void TravelNode::cropUselessLink(TravelNode* farNode)
{
    if (isUselessLink(farNode))
        removeLinkTo(farNode);
}

bool TravelNode::cropUselessLinks()
{
    bool hasRemoved = false;

    for (auto& firstLink : *getPaths())
    {
        TravelNode* farNode = firstLink.first;
        if (this->hasLinkTo(farNode) && this->isUselessLink(farNode))
        {
            this->removeLinkTo(farNode);
            hasRemoved = true;

            if (sPlayerbotAIConfig.hasLog("crop.csv"))
            {
                ostringstream out;
                out << getName() << ",";
                out << farNode->getName() << ",";
                WorldPosition().printWKT({ *getPosition(),*farNode->getPosition() },out,1);
                out << std::fixed;

                sPlayerbotAIConfig.log("crop.csv", out.str().c_str());
            }
        }
        if (farNode->hasLinkTo(this) && farNode->isUselessLink(this))
        {
            farNode->removeLinkTo(this);
            hasRemoved = true;

            if (sPlayerbotAIConfig.hasLog("crop.csv"))
            {
                ostringstream out;
                out << getName() << ",";
                out << farNode->getName() << ",";
                WorldPosition().printWKT({ *getPosition(),*farNode->getPosition() }, out,1);
                out << std::fixed;

                sPlayerbotAIConfig.log("crop.csv", out.str().c_str());
            }
        }
    }

    return hasRemoved;

    /*

    //vector<pair<TravelNode*, TravelNode*>> toRemove;
    for (auto& firstLink : getLinks())
    {


        TravelNode* firstNode = firstLink.first;
        float firstLength = firstLink.second.getDistance();
        for (auto& secondLink : getLinks())
        {
            TravelNode* secondNode = secondLink.first;
            float secondLength = secondLink.second.getDistance();

            if (firstNode == secondNode)
                continue;

            if (std::find(toRemove.begin(), toRemove.end(), [firstNode, secondNode](pair<TravelNode*, TravelNode*> pair) {return pair.first == firstNode || pair.first == secondNode;}) != toRemove.end())
                continue;

            if (firstNode->hasLinkTo(secondNode))
            {
                //Is it quicker to go past first node to reach second node instead of going directly?
                if (firstLength + firstNode->linkLengthTo(secondNode) < secondLength * 1.1)
                {
                    if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                        continue;

                    toRemove.push_back(make_pair(this, secondNode));
                }
            }
            else
            {
                TravelNodeRoute route = sTravelNodeMap.getRoute(firstNode, secondNode, false);

                if (route.isEmpty())
                    continue;

                if (route.hasNode(this))
                    continue;

                //Is it quicker to go past first (and multiple) nodes to reach the second node instead of going directly?
                if (firstLength + route.getLength() < secondLength * 1.1)
                {
                    if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                        continue;

                    toRemove.push_back(make_pair(this, secondNode));
                }
            }
        }

        //Reverse cleanup. This is needed when we add a node in an existing map.
        if (firstNode->hasLinkTo(this))
        {
            firstLength = firstNode->getPathTo(this)->getDistance();

            for (auto& secondLink : firstNode->getLinks())
            {
                TravelNode* secondNode = secondLink.first;
                float secondLength = secondLink.second.getDistance();

                if (this == secondNode)
                    continue;

                if (std::find(toRemove.begin(), toRemove.end(), [firstNode, secondNode](pair<TravelNode*, TravelNode*> pair) {return pair.first == firstNode || pair.first == secondNode; }) != toRemove.end())
                    continue;

                if (firstNode->hasLinkTo(secondNode))
                {
                    //Is it quicker to go past first node to reach second node instead of going directly?
                    if (firstLength + firstNode->linkLengthTo(secondNode) < secondLength * 1.1)
                    {
                        if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                            continue;

                        toRemove.push_back(make_pair(this, secondNode));
                    }
                }
                else
                {
                    TravelNodeRoute route = sTravelNodeMap.getRoute(firstNode, secondNode, false);

                    if (route.isEmpty())
                        continue;

                    if (route.hasNode(this))
                        continue;

                    //Is it quicker to go past first (and multiple) nodes to reach the second node instead of going directly?
                    if (firstLength + route.getLength() < secondLength * 1.1)
                    {
                        if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                            continue;

                        toRemove.push_back(make_pair(this, secondNode));
                    }
                }
            }
        }

    }

    for (auto& nodePair : toRemove)
        nodePair.first->unlinkNode(nodePair.second, false);
        */
}

bool TravelNode::isEqual(TravelNode* compareNode)
{
    if (!hasLinkTo(compareNode))
        return false;

    if (!compareNode->hasLinkTo(this))
        return false;

    for (auto& node : sTravelNodeMap.getNodes())
    {
        if (node == this || node == compareNode)
            continue;

        if (node->hasLinkTo(this) != node->hasLinkTo(compareNode))
            return false;

        if (hasLinkTo(node) != compareNode->hasLinkTo(node))
            return false;
    }

    return true;
}

void TravelNode::print(bool printFailed)
{
    WorldPosition* startPosition = getPosition();

    uint32 mapSize = getNodeMap(true).size();

    ostringstream out;
    string name = getName();
    name.erase(remove(name.begin(), name.end(), '\"'), name.end());
    out << name.c_str() << ",";
    out << std::fixed << std::setprecision(2);
    point.printWKT(out);
    out << getZ() << ",";
    out << getO() << ",";
    out << (isImportant() ? 1 : 0) << ",";
    out << mapSize;

    sPlayerbotAIConfig.log("travelNodes.csv", out.str().c_str());

    vector<WorldPosition> ppath;

    for (auto& endNode : sTravelNodeMap.getNodes())
    {
        if (endNode == this)
            continue;

        if (!hasPathTo(endNode))
            continue;

        TravelNodePath* path = getPathTo(endNode);

        if (!hasLinkTo(endNode) && urand(0, 20) && !printFailed)
            continue;

        ppath = path->getPath();

        if (ppath.size() < 2 && hasLinkTo(endNode))
        {
            ppath.push_back(point);
            ppath.push_back(*endNode->getPosition());
        }

        if (ppath.size() > 1)
        {
            ostringstream out;

            uint32 pathType = 1;
            if (!hasLinkTo(endNode))
                pathType = 0;
            else if (path->getPathType() == TravelNodePathType::transport)
                pathType = 2;
            else if (path->getPathType() == TravelNodePathType::areaTrigger && getMapId() == endNode->getMapId())
                pathType = 3;
            else if (path->getPathType() == TravelNodePathType::areaTrigger)
                pathType = 4;
            else if (path->getPathType() == TravelNodePathType::flightPath)
                pathType = 5;
            else if (!path->getComplete())
                pathType = 6;
            else if (path->getPathType() == TravelNodePathType::staticPortal)
                pathType = 7;

            out << pathType << ",";
            out << std::fixed << std::setprecision(2);
            point.printWKT(ppath, out, 1);
            out << path->getPathObject() << ",";
            out << path->getDistance() << ",";
            out << path->getCost() << ",";
            out << (path->getComplete() ? 0 : 1) << ",";
            out << to_string(path->getMaxLevelCreature()[0])<< ",";
            out << to_string(path->getMaxLevelCreature()[1]) << ",";
            out << to_string(path->getMaxLevelCreature()[2]);

            sPlayerbotAIConfig.log("travelPaths.csv", out.str().c_str());
        }
    }
}

//Attempts to move ahead of the path.
bool TravelPath::makeShortCut(WorldPosition startPos, float maxDist, Unit* bot)
{
    if (getPath().empty())
        return false;
    float maxDistSq = maxDist * maxDist;
    float minDist = -1;
    float totalDist = fullPath.begin()->point.sqDistance(startPos);
    vector<PathNodePoint> newPath;
    WorldPosition firstNode;

    for (auto& p : fullPath) //cycle over the full path
    {
        //if (p.point.getMapId() != startPos.getMapId())
        //    continue;

        if (p.point.getMapId() == startPos.getMapId())
        {
            float curDist = p.point.sqDistance(startPos);

            if (&p != &fullPath.front())
                totalDist += p.point.sqDistance(std::prev(&p)->point);

            if (curDist < sPlayerbotAIConfig.tooCloseDistance * sPlayerbotAIConfig.tooCloseDistance) //We are on the path. This is a good starting point
            {
                minDist = curDist;
                totalDist = curDist;
                newPath.clear();
            }

            if (p.type != PathNodeType::NODE_PREPATH) //Only look at the part after the first node and in the same map.
            {
                if (!firstNode)
                    firstNode = p.point;

                if (minDist == -1 || curDist < minDist || (curDist < maxDistSq && curDist < totalDist / 2)) //Start building from the last closest point or a point that is close but far on the path.
                {
                    minDist = curDist;
                    totalDist = curDist;
                    newPath.clear();
                }
            }
        }

        newPath.push_back(p);
    }

    if (newPath.empty() || minDist > maxDistSq || newPath.front().point.getMapId() != startPos.getMapId())
    {
        clear();
        return false;
    }

    WorldPosition beginPos = newPath.begin()->point;

    //The old path seems to be the best.
    if (beginPos.distance(firstNode) < sPlayerbotAIConfig.tooCloseDistance)
        return false;

    //We are (nearly) on the new path. Just follow the rest.
    if (beginPos.distance(startPos) < sPlayerbotAIConfig.tooCloseDistance)
    {
        fullPath = newPath;
        return true;
    }

    vector<WorldPosition> toPath = startPos.getPathTo(beginPos, bot);

    //We can not reach the new begin position. Follow the complete path.
    if (!beginPos.isPathTo(toPath))
        return false;

    //Move to the new path and continue.
    fullPath.clear();
    addPath(toPath);
    addPath(newPath);

    return true;
}

bool TravelPath::shouldMoveToNextPoint(WorldPosition startPos, vector<PathNodePoint>::iterator beg, vector<PathNodePoint>::iterator ed, vector<PathNodePoint>::iterator p, float& moveDist, float maxDist)
{
    if (p == ed) //We are the end. Stop now.
        return false;

    auto nextP = std::next(p);

    // Fix assertion fail due to nextP to be invalidate
    if (nextP == ed) //We are the end. Stop now.
        return false;

    //We are moving to a area trigger node and want to move to the next teleport node.
    if (p->type == PathNodeType::NODE_AREA_TRIGGER && nextP->type == PathNodeType::NODE_AREA_TRIGGER && p->entry == nextP->entry)
    {
        return false; //Move to teleport and activate area trigger.
    }

    //We are moving to a area trigger node and want to move to the next teleport node.
    if (p->type == PathNodeType::NODE_STATIC_PORTAL && nextP->type == PathNodeType::NODE_STATIC_PORTAL && p->entry == nextP->entry)
    {
        return false; //Move to teleport and activate area trigger.
    }

    //We are using a hearthstone.
    if (p->type == PathNodeType::NODE_TELEPORT && nextP->type == PathNodeType::NODE_TELEPORT && p->entry == nextP->entry)
    {
        return false; //Use the teleport
    }

    //We are almost at a transport node. Move to the node before this.   
    if (nextP->type == PathNodeType::NODE_TRANSPORT && nextP->entry)
    {
        return false;
    }
    
    //We are moving to a transport node.
    if (p->type == PathNodeType::NODE_TRANSPORT && p->entry)
    {
        if (nextP->type != PathNodeType::NODE_TRANSPORT && p != beg && std::prev(p)->type != PathNodeType::NODE_TRANSPORT) //We are not using the transport. Skip it.
            return true;

        return false; //Teleport to exit of transport.
    }

    //We are moving to a flightpath and want to fly.
    if (p->type == PathNodeType::NODE_FLIGHTPATH && nextP->type == PathNodeType::NODE_FLIGHTPATH)
    {
        return false;
    }

    float nextMove = p->point.distance(nextP->point);

    if (p->point.getMapId() != startPos.getMapId() || ((moveDist + nextMove > maxDist || startPos.distance(nextP->point) > maxDist) && moveDist > 0))
    {
        return false;
    }

    moveDist += nextMove;

    return true;
}

//Next position to move to
WorldPosition TravelPath::getNextPoint(WorldPosition startPos, float maxDist, TravelNodePathType& pathType, uint32& entry, bool onTransport, WorldPosition& telePosition)
{
    if (getPath().empty())
        return WorldPosition();

    auto beg = fullPath.begin();
    auto ed = fullPath.end();

    float minDist = 0.0f;
    auto startP = beg;

    //Get the closest point on the path to start from.
    for (auto p = startP; p != ed; p++)
    {
        if (p->point.getMapId() != startPos.getMapId())
            continue;

        float curDist = p->point.distance(startPos);

        if (curDist <= minDist || p == beg)
        {
            minDist = curDist;
            startP = p;
        }
    }

    float moveDist = startP->point.distance(startPos);

    //Move as far as we are allowed
    for (auto p = startP; p != ed; p++)
    {
        if (shouldMoveToNextPoint(startPos, beg, ed, p, moveDist, maxDist))
            continue;

        startP = p;

        break;
    }

    auto prevP = startP, nextP = startP;
    if (startP != beg)
        prevP = std::prev(prevP);
    if (std::next(nextP) != ed)
        nextP = std::next(nextP);

    //We are moving towards a teleport. Move to portal an activate area trigger
    if (startP->type == PathNodeType::NODE_AREA_TRIGGER)
    {
        pathType = TravelNodePathType::areaTrigger;
        entry = startP->entry;
        return startP->point;
    }

    //We are moving towards a static portal. Move to portal and use it.
    if (startP->type == PathNodeType::NODE_STATIC_PORTAL && startPos.distance(startP->point) < INTERACTION_DISTANCE)
    {
        pathType = TravelNodePathType::staticPortal;
        entry = startP->entry;
        return startP->point;
    }

    //We are using a hearthstone
    if (nextP->type == PathNodeType::NODE_TELEPORT)
    {
        pathType = TravelNodePathType::teleportSpell;
        entry = startP->entry;
        return startP->point;
    }

    //We are moving towards a flight path. Move to flight master and activate flight path.
    if (startP->type == PathNodeType::NODE_FLIGHTPATH && startPos.distance(startP->point) < INTERACTION_DISTANCE)
    {
        pathType = TravelNodePathType::flightPath;
        entry = startP->entry;
        return startP->point;
    }

    if (nextP->type == PathNodeType::NODE_TRANSPORT)
    {
        pathType = TravelNodePathType::areaTrigger;
        entry = 0;

        for (auto p = startP + 1; p != ed; p++) //Move along the transport path to the end of the boat ride. 
        {
            if (p->type != PathNodeType::NODE_TRANSPORT)
            {
                return p->point;              //We want to move here.
            }
        }
    }

    if (startP->type == PathNodeType::NODE_TRANSPORT)
    {
        pathType = TravelNodePathType::areaTrigger;
        entry = 0;

        for (auto p = startP + 1; p != ed; p++) //Move along the transport path to the end of the boat ride. 
        {
            if (p->type != PathNodeType::NODE_TRANSPORT)
            {
                return p->point;              //We want to move here.
            }
        }
    }

    //We are on or very near a transport.
    if (startP->type == PathNodeType::NODE_TRANSPORT)
    {
        
        
        if (onTransport) //We are moving with the transport
        {
            pathType = TravelNodePathType::transport;
            entry = startP->entry;
        }
        else//We are probably in the water. Just teleport now.
        {
            pathType = TravelNodePathType::areaTrigger;
            entry = 0;
        }
        
        for (auto p = startP + 1; p != ed; p++) //Move along the transport path to the end of the boat ride. 
        {
            if (p->type != PathNodeType::NODE_TRANSPORT)
            {
                telePosition = prevP->point;  //Boat needs to be here
                return p->point;              //We want to move here.
            }
            prevP = p;
        }
    }

    //We are about the enter a transport.
    if (nextP->type == PathNodeType::NODE_TRANSPORT)
    {
        pathType = TravelNodePathType::transport;
        entry = nextP->entry;
        telePosition = nextP->point; //Boat needs to be here.
        return startP->point;        //We want to stand somewhere here.
    } 

    //We have to move far for next point. Try to make a cropped path.
    if (moveDist < sPlayerbotAIConfig.targetPosRecalcDistance && std::next(startP) != ed)
    {
        //vector<WorldPosition> path = startPos.getPathTo(std::next(startP)->point, nullptr);
        //startP->point = startPos.lastInRange(path, -1, maxDist);
        return WorldPosition();
    }

    return startP->point;
}

ostringstream TravelPath::print()
{
    ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr();
    out << "+00," << "1,";
    out << std::fixed;

    WorldPosition().printWKT(getPointPath(), out, 1);

    return out;
}

float TravelNodeRoute::getTotalDistance()
{
    float totalLength = 0;
    for (uint32 i = 0; i < nodes.size() - 2; i++)
    {
        totalLength += nodes[i]->linkDistanceTo(nodes[i + 1]);
    }

    return totalLength;
}

TravelPath TravelNodeRoute::buildPath(vector<WorldPosition> pathToStart, vector<WorldPosition> pathToEnd, Unit* bot)
{
    TravelPath travelPath;

    Unit* botForPath = bot;

    if (!pathToStart.empty()) //From start position to start of path.
        travelPath.addPath(pathToStart, PathNodeType::NODE_PREPATH);

    TravelNode* prevNode = nullptr;
    for (auto& node : nodes)
    {
        if (prevNode)
        {
            TravelNodePath* nodePath = nullptr;
            if (prevNode->hasPathTo(node))  //Get the path to the next node if it exists.
                nodePath = prevNode->getPathTo(node);

            if (!nodePath || !nodePath->getComplete()) //Build the path to the next node if it doesn't exist.
            {
                if (!prevNode->isTransport())
                    nodePath = prevNode->buildPath(node, botForPath);
                else //For transports we have no proper path since the node is in air/water. Instead we build a reverse path and follow that.
                {
                    node->buildPath(prevNode, botForPath); //Reverse build to get proper path.
                    nodePath = prevNode->getPathTo(node);
                }
            }

            TravelNodePath returnNodePath;

            if (!nodePath || !nodePath->getComplete()) //It looks like we can't properly path to our node. Make a temporary reverse path and see if that works instead.
            {
                returnNodePath = *node->buildPath(prevNode, botForPath); //Build reverse path and save it to a temporary variable.
                vector<WorldPosition> path = returnNodePath.getPath();
                reverse(path.begin(), path.end()); //Reverse the path 
                returnNodePath.setPath(path);
                nodePath = &returnNodePath;
            }

            if (!nodePath || !nodePath->getComplete()) //If we can not build a path just try to move to the node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_NODE);
            }
            else if (nodePath->getPathType() == TravelNodePathType::areaTrigger) //Teleport to next node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_AREA_TRIGGER, nodePath->getPathObject()); //Entry point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_AREA_TRIGGER, nodePath->getPathObject());     //Exit point
            }
            else if (nodePath->getPathType() == TravelNodePathType::staticPortal) //Teleport to next node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_STATIC_PORTAL, nodePath->getPathObject()); //Entry point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_STATIC_PORTAL, nodePath->getPathObject());     //Exit point
            }
            else if (nodePath->getPathType() == TravelNodePathType::transport) //Move onto transport
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_TRANSPORT, nodePath->getPathObject()); //Departure point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_TRANSPORT, nodePath->getPathObject());     //Arrival point        
            }
            else if (nodePath->getPathType() == TravelNodePathType::flightPath) //Use the flightpath
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_FLIGHTPATH, nodePath->getPathObject()); //Departure point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_FLIGHTPATH, nodePath->getPathObject());     //Arrival point        
            }
            else if (nodePath->getPathType() == TravelNodePathType::teleportSpell)
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_TELEPORT, nodePath->getPathObject());
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_TELEPORT, nodePath->getPathObject());
            }
            else
            {
                vector<WorldPosition> path = nodePath->getPath();

                if (path.size() > 1 && node != nodes.back()) //Remove the last point since that will also be the start of the next path.
                    path.pop_back();

                if (path.size() > 1 && prevNode->isPortal() && nodePath->getPathType() != TravelNodePathType::areaTrigger) //Do not move to the area trigger if we don't plan to take the portal.
                    path.erase(path.begin());

                if (path.size() > 1 && prevNode->isTransport() && nodePath->getPathType() != TravelNodePathType::transport) //Do not move to the transport if we aren't going to take it.
                    path.erase(path.begin());

                travelPath.addPath(path, PathNodeType::NODE_PATH);
            }
        }
        prevNode = node;
    }

    if (!pathToEnd.empty())
        travelPath.addPath(pathToEnd, PathNodeType::NODE_PATH);

    return travelPath;
}

ostringstream TravelNodeRoute::print()
{
    ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr();
    out << "+00" << ",0," << "\"LINESTRING(";

    for (auto& node : nodes)
    {
        out << std::fixed << node->getPosition()->getDisplayX() << " " << node->getPosition()->getDisplayY() << ",";
    }

    out << ")\"";

    return out;
}

TravelNodeMap::TravelNodeMap(TravelNodeMap* baseMap)
{
    TravelNode* newNode;

    baseMap->m_nMapMtx.lock_shared();

    for (auto& node : baseMap->getNodes())
    {
        newNode = new TravelNode(node);

        m_nodes.push_back(newNode);
    }

    for (auto& node : baseMap->getNodes())
    {
        newNode = getNode(node);

        for (auto& path : *node->getPaths())
        {
            TravelNode* endNode = getNode(path.first);

            newNode->setPathTo(endNode, path.second);
        }
    }

    baseMap->m_nMapMtx.unlock_shared();
}

TravelNode* TravelNodeMap::addNode(WorldPosition pos, string preferedName, bool isImportant, bool checkDuplicate, bool transport, uint32 transportId)
{
    TravelNode* newNode;

    if (checkDuplicate)
    {
        newNode = getNode(pos, nullptr, 5.0f);
        if (newNode)
            return newNode;
    }

    string finalName = preferedName;

    if (!isImportant)
    {
        std::regex last_num("[[:digit:]]+$");
        finalName = std::regex_replace(finalName, last_num, "");
        uint32 nameCount = 1;

        for (auto& node : getNodes())
        {
            if (node->getName().find(preferedName + to_string(nameCount)) != std::string::npos)
                nameCount++;
        }

        if(nameCount)
            finalName += to_string(nameCount);
    }

    newNode = new TravelNode(pos, finalName, isImportant);

    m_nodes.push_back(newNode);

    return newNode;
}

void TravelNodeMap::removeNode(TravelNode* node)
{
    node->removeLinkTo(NULL, true);

    for (auto& tnode : m_nodes)
    {
        if (tnode == node)
        {
            delete tnode;
            tnode = nullptr;
        }
    }

    m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), nullptr), m_nodes.end());
}

void TravelNodeMap::fullLinkNode(TravelNode* startNode, Unit* bot)
{
    WorldPosition* startPosition = startNode->getPosition();
    vector<TravelNode*> linkNodes = getNodes(*startPosition);

    for (auto& endNode : linkNodes)
    {
        if (endNode == startNode)
            continue;

        if (startNode->hasLinkTo(endNode))
            continue;

        startNode->buildPath(endNode, bot);
        endNode->buildPath(startNode, bot);
    }

    startNode->setLinked(true);
}

vector<TravelNode*> TravelNodeMap::getNodes(WorldPosition pos, float range)
{
    vector<TravelNode*> retVec;
    for (auto& node : m_nodes)
    {
        if (node->getMapId() == pos.getMapId())
            if (range == -1 || node->getDistance(pos) <= range)
                retVec.push_back(node);
    }

    std::sort(retVec.begin(), retVec.end(), [pos](TravelNode* i, TravelNode* j) { return i->getPosition()->distance(pos) < j->getPosition()->distance(pos); });
    return retVec;
}


TravelNode* TravelNodeMap::getNode(WorldPosition pos, vector<WorldPosition>& ppath, Unit* bot, float range)
{
    float x = pos.getX();
    float y = pos.getY();
    float z = pos.getZ();

    if (bot && !bot->IsInWorld())
        return NULL;

    uint32 c = 0;

    vector<TravelNode*> nodes = sTravelNodeMap.getNodes(pos, range);
    for (auto& node : nodes)
    {
        if (!bot || pos.canPathTo(*node->getPosition(), bot))
            return node;

        c++;

        if (c > 5) //Max 5 attempts
            break;
    }

    return NULL;
}

TravelNodeRoute TravelNodeMap::getRoute(TravelNode* start, TravelNode* goal, Unit* unit)
{
    float unitSpeed = unit ? unit->GetSpeed(MOVE_RUN) : 7.0f;

    if (start == goal)
        return TravelNodeRoute();

    //Basic A* algoritm
    std::unordered_map<TravelNode*, TravelNodeStub> m_stubs;

    TravelNodeStub* startStub = &m_stubs.insert(make_pair(start, TravelNodeStub(start))).first->second;

    TravelNodeStub* currentNode, * childNode;

    float f, g, h;

    std::vector<TravelNodeStub*> open, closed;

    std::vector<TravelNode*> portNodes;

    Player* bot = dynamic_cast<Player*>(unit);
    if (bot)
    {
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (ai)
        {
            AiObjectContext* context = ai->GetAiObjectContext();

            if (ai->HasCheat(BotCheatMask::gold))
                startStub->currentGold = 10000000;
            else {
                AiObjectContext* context = ai->GetAiObjectContext();
                startStub->currentGold = AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::travel);
            }

            if (AI_VALUE2(bool, "action useful", "hearthstone") && bot->IsAlive())
            {
                TravelNode* homeNode = sTravelNodeMap.getNode(AI_VALUE(WorldPosition, "home bind"), nullptr, 10.0f);
                if (homeNode)
                {
                    PortalNode* portNode = new PortalNode(start);
                    portNode->SetPortal(start, homeNode, 8690);

                    childNode = &m_stubs.insert(make_pair(portNode, TravelNodeStub(portNode))).first->second;

                    childNode->m_g = std::max((uint32)2, (10 - AI_VALUE(uint32, "death count")) * MINUTE); //If we can walk there in 10 minutes, walk instead.
                    childNode->m_h = childNode->dataNode->fDist(goal) / unitSpeed;
                    childNode->m_f = childNode->m_g + childNode->m_h;

                    open.push_back(childNode);
                    std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
                    childNode->open = true;
                    portNodes.push_back(portNode);
                }
            }
        }
        else
            startStub->currentGold = bot->GetMoney();       

        vector<uint32> teleSpells = { 3561,3562,3563,3565,3566,3567,18960 };

        for (auto spellId : teleSpells)
        {
            if (!bot->IsAlive())
                continue;

            if (!bot->HasSpell(spellId))
                continue;

            if (!sServerFacade.IsSpellReady(bot, spellId))
                continue;

            if (!sSpellMgr.GetSpellTargetPosition(spellId))
                continue;

            WorldPosition telePos(sSpellMgr.GetSpellTargetPosition(spellId));

            TravelNode* homeNode = sTravelNodeMap.getNode(telePos, nullptr, 10.0f);

            if (!homeNode)
                continue;

            PortalNode* portNode = new PortalNode(start);
            portNode->SetPortal(start, homeNode, spellId);

            childNode = &m_stubs.insert(make_pair(portNode, TravelNodeStub(portNode))).first->second;

            childNode->m_g = MINUTE; //If we can walk there in a minute. Walk instead.
            childNode->m_h = childNode->dataNode->fDist(goal) / unitSpeed;
            childNode->m_f = childNode->m_g + childNode->m_h;

            open.push_back(childNode);
            std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
            childNode->open = true;
            portNodes.push_back(portNode);
        }
    }

    if (open.size() == 0 && !start->hasRouteTo(goal))
    {
        for (auto node : portNodes) delete node;
        return TravelNodeRoute();
    }

    std::make_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });

    open.push_back(startStub);
    std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
    startStub->open = true;

    while (!open.empty())
    {
        std::sort(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });

        currentNode = open.front(); // pop n node from open for which f is minimal

        std::pop_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
        open.pop_back();
        currentNode->open = false;

        currentNode->close = true;
        closed.push_back(currentNode);

        if (currentNode->dataNode == goal)
        {
            TravelNodeStub* parent = currentNode->parent;

            vector<TravelNode*> path;

            path.push_back(currentNode->dataNode);

            while (parent != nullptr)
            {
                path.push_back(parent->dataNode);
                parent = parent->parent;
            }

            reverse(path.begin(), path.end());

            return TravelNodeRoute(path, portNodes);
        }

        for (const auto& link : *currentNode->dataNode->getLinks())// for each successor n' of n
        {
            TravelNode* linkNode = link.first;

            float linkCost = link.second->getCost(unit, currentNode->currentGold);

            if (linkCost <= 0)
                continue;

            childNode = &m_stubs.insert(make_pair(linkNode, TravelNodeStub(linkNode))).first->second;

            g = currentNode->m_g + linkCost; // stance from start + distance between the two nodes
            if ((childNode->open || childNode->close) && childNode->m_g <= g) // n' is already in opend or closed with a lower cost g(n')
                continue; // consider next successor

            h = childNode->dataNode->fDist(goal) / unitSpeed;
            f = g + h; // compute f(n')
            childNode->m_f = f;
            childNode->m_g = g;
            childNode->m_h = h;
            childNode->parent = currentNode;

            if (bot && !bot->isTaxiCheater())
                childNode->currentGold = currentNode->currentGold - link.second->getPrice();

            if (childNode->close)
                childNode->close = false;
            if (!childNode->open)
            {
                open.push_back(childNode);
                std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
                childNode->open = true;
            }
        }
    }

    for (auto node : portNodes) delete node;

    return TravelNodeRoute();
}

TravelNodeRoute TravelNodeMap::getRoute(WorldPosition startPos, WorldPosition endPos, vector<WorldPosition>& startPath, Unit* unit)
{
    if (m_nodes.empty())
        return TravelNodeRoute();

    vector<WorldPosition> newStartPath;
    vector<TravelNode*> startNodes = m_nodes, endNodes = m_nodes;
    //Partial sort to get the closest 5 nodes at the begin of the array.        
    std::partial_sort(startNodes.begin(), startNodes.begin() + 5, startNodes.end(), [startPos](TravelNode* i, TravelNode* j) {return i->fDist(startPos) < j->fDist(startPos); });
    std::partial_sort(endNodes.begin(), endNodes.begin() + 5, endNodes.end(), [endPos](TravelNode* i, TravelNode* j) {return i->fDist(endPos) < j->fDist(endPos); });

    //Cycle over the combinations of these 5 nodes.
    uint32 startI = 0, endI = 0;
    while (startI < 5 && endI < 5)
    {
        TravelNode* startNode = startNodes[startI];
        TravelNode* endNode = endNodes[endI];

        WorldPosition startNodePosition = *startNode->getPosition();
        WorldPosition endNodePosition = *endNode->getPosition();

        float maxStartDistance = startNode->isTransport() ? 20.0f : sPlayerbotAIConfig.targetPosRecalcDistance;

        TravelNodeRoute route = getRoute(startNode, endNode, unit);

        if (!route.isEmpty())
        {
            //Check if the bot can actually walk to this start position.
            newStartPath = startPath;
            if (startNodePosition.cropPathTo(newStartPath, maxStartDistance) || startNode->getPosition()->isPathTo(newStartPath = startPos.getPathTo(startNodePosition, unit), maxStartDistance))
            {
                startPath = newStartPath;
                return route;
            }
            startI++;
        }

        //Prefer a differnt end-node. 
        endI++;

        //Cycle to a different start-node if needed.
        if (endI > startI + 1)
        {
            startI++;
            endI = 0;
        }
    }

    Player* bot = dynamic_cast<Player*>(unit);
    if (bot)
    {
        Player* player = bot;
        if (PAI_VALUE2(bool, "action useful", "hearthstone"))
        {
            startPath.clear();
            TravelNode* botNode = new TravelNode(startPos, "Bot Pos", false);
            botNode->setPoint(startPos);

            endI = 0;
            while (endI < 5)
            {
                TravelNode* endNode = endNodes[endI];
                TravelNodeRoute route = getRoute(botNode, endNode, bot);
                route.addTempNodes({ botNode });

                if (!route.isEmpty())
                    return route;
                endI++;
            }
        }
    }

    return TravelNodeRoute();
}

TravelPath TravelNodeMap::getFullPath(WorldPosition startPos, WorldPosition endPos, Unit* unit)
{
    TravelPath movePath;
    vector<WorldPosition> beginPath, endPath;

    beginPath = endPos.getPathFromPath({ startPos }, unit, 40);

    if (endPos.isPathTo(beginPath,sPlayerbotAIConfig.spellDistance)) //If we can get within spell distance a longer route won't help.
        return TravelPath(beginPath);

    //[[Node pathfinding system]]
                //We try to find nodes near the bot and near the end position that have a route between them.
                //Then bot has to move towards/along the route.
    sTravelNodeMap.m_nMapMtx.lock_shared();

    //Find the route of nodes starting at a node closest to the start position and ending at a node closest to the endposition.
    //Also returns longPath: The path from the start position to the first node in the route.
    TravelNodeRoute route = sTravelNodeMap.getRoute(startPos, endPos, beginPath, unit);

    if (route.isEmpty())
    {
        route.cleanTempNodes();
        return movePath;
    }

    if (sPlayerbotAIConfig.hasLog("bot_pathfinding.csv"))
    {
        Player* bot = dynamic_cast<Player*>(unit);
        if (bot)
        {
            PlayerbotAI* ai = bot->GetPlayerbotAI();
            if (ai)
            {
                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
                    sPlayerbotAIConfig.log("bot_pathfinding.csv", route.print().str().c_str());
                }
            }
        }
    }

    //endPath = route.getNodes().back()->getPosition()->getPathTo(endPos, bot);
    endPath = { *route.getNodes().back()->getPosition(),endPos };
    movePath = route.buildPath(beginPath, endPath);

    route.cleanTempNodes();

    if (sPlayerbotAIConfig.hasLog("bot_pathfinding.csv"))
    {
        Player* bot = dynamic_cast<Player*>(unit);
        if (bot)
        {
            PlayerbotAI* ai = bot->GetPlayerbotAI();
            if (ai)
            {
                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
                    sPlayerbotAIConfig.log("bot_pathfinding.csv", movePath.print().str().c_str());
                }
            }
        }
    }

    sTravelNodeMap.m_nMapMtx.unlock_shared();

    return movePath;
}

bool TravelNodeMap::cropUselessNode(TravelNode* startNode)
{
    if (!startNode->isLinked() || startNode->isImportant())
        return false;

    vector<TravelNode*> ignore = { startNode };

    for (auto& node : getNodes(*startNode->getPosition(), 5000))
    {
        if (startNode == node)
            continue;

        if (node->getNodeMap(true).size() > node->getNodeMap(true, ignore).size())
            return false;
    }

    removeNode(startNode);

    return true;
}

TravelNode* TravelNodeMap::addZoneLinkNode(TravelNode* startNode)
{
    for (auto& path : *startNode->getPaths())
    {


        TravelNode* endNode = path.first;

        string zoneName = startNode->getPosition()->getAreaName(true, true);
        for (auto& pos : path.second.getPath())
        {
            string newZoneName = pos.getAreaName(true, true);
            if (zoneName != newZoneName)
            {
                if (!getNode(pos, NULL, 100.0f))
                {
                    string nodeName = zoneName + " to " + newZoneName;
                    return sTravelNodeMap.addNode(pos, nodeName, false, true);
                }
                zoneName = newZoneName;
            }

        }
    }

    return NULL;
}

TravelNode* TravelNodeMap::addRandomExtNode(TravelNode* startNode)
{
    std::unordered_map<TravelNode*, TravelNodePath> paths = *startNode->getPaths();

    if (paths.empty())
        return NULL;

    for (uint32 i = 0; i < 20; i++)
    {
        auto random_it = std::next(std::begin(paths), urand(0, paths.size() - 1));

        TravelNode* endNode = random_it->first;
        vector<WorldPosition> path = random_it->second.getPath();

        if (path.empty())
            continue;

        //Prefer to skip complete links
        if (endNode->hasLinkTo(startNode) && startNode->hasLinkTo(endNode) && !urand(0, 20))
            continue;

        //Prefer to skip no links
        if (!startNode->hasLinkTo(endNode) && !urand(0, 20))
            continue;

        WorldPosition point = path[urand(0, path.size() - 1)];

        if (!getNode(point, NULL, 100.0f))
            return sTravelNodeMap.addNode(point, startNode->getName(), false, true);
    }

    return NULL;
}

void TravelNodeMap::manageNodes(Unit* bot, bool mapFull)
{
    bool rePrint = false;

    if (!bot->IsInWorld())
        return;

    if (m_nMapMtx.try_lock())
    {

        TravelNode* startNode;
        TravelNode* newNode;

        for (auto startNode : m_nodes)
        {
            cropUselessNode(startNode);
        }

        //Pick random Node
        for (uint32 i = 0; i < (mapFull ? (uint32)20 : (uint32)1); i++)
        {
            vector<TravelNode*> rnodes = getNodes(WorldPosition(bot));

            if (!rnodes.empty())
            {
                uint32 j = urand(0, rnodes.size() - 1);

                startNode = rnodes[j];
                newNode = NULL;

                bool nodeDone = false;

                if (!nodeDone)
                    nodeDone = cropUselessNode(startNode);

                if (!nodeDone && !urand(0, 20))
                    newNode = addZoneLinkNode(startNode);

                if (!nodeDone && !newNode && !urand(0, 20))
                    newNode = addRandomExtNode(startNode);

                rePrint = nodeDone || rePrint || newNode;
            }

        }

        if (rePrint && (mapFull || !urand(0, 20)))
            printMap();

        m_nMapMtx.unlock();
    }

    sTravelNodeMap.m_nMapMtx.lock_shared();

    if (!rePrint && mapFull)
        printMap();

    m_nMapMtx.unlock_shared();
}


void TravelNodeMap::generateNpcNodes()
{
    unordered_map<uint32, GuidPosition> bossMap;

    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        GuidPosition guidP(creaturePair);
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(guidP.GetEntry());

        if (!cInfo)
            continue;

        uint32 flagMask = UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_FLIGHTMASTER | UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE;

        if (cInfo->NpcFlags & flagMask)
        {
            string nodeName = guidP.getAreaName(false);

            if (cInfo->NpcFlags & UNIT_NPC_FLAG_INNKEEPER)
                nodeName += " innkeeper";
            else if (cInfo->NpcFlags & UNIT_NPC_FLAG_FLIGHTMASTER)
                nodeName += " flightMaster";
            else if (cInfo->NpcFlags & UNIT_NPC_FLAG_SPIRITHEALER)
                nodeName += " spirithealer";
            else if (cInfo->NpcFlags & UNIT_NPC_FLAG_SPIRITGUIDE)
                nodeName += " spiritguide";

            TravelNode* node = sTravelNodeMap.addNode(guidP, nodeName, true, true);
        }
        else if (cInfo->Rank == 3)
        {
            string nodeName = cInfo->Name;

            sTravelNodeMap.addNode(guidP, nodeName, true, true);
        }
        else if (cInfo->Rank == 1 && !guidP.isOverworld())
        {
            if (bossMap.find(cInfo->Entry) == bossMap.end())
                bossMap[cInfo->Entry] = guidP;
            else if (bossMap[cInfo->Entry])
                bossMap[cInfo->Entry] = GuidPosition();
        }
    }

    for (auto boss : bossMap)
    {
        GuidPosition guidP = boss.second;

        if (!guidP)
            continue;

        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(guidP.GetEntry());

        if (!cInfo)
            continue;

        string nodeName = cInfo->Name;

        sTravelNodeMap.addNode(guidP, nodeName, true, true);
    }
}

void TravelNodeMap::generateStartNodes()
{
    map<uint8, string> startNames;
    startNames[RACE_HUMAN] = "Human";
    startNames[RACE_ORC] = "Orc and Troll";
    startNames[RACE_DWARF] = "Dwarf and Gnome";
    startNames[RACE_NIGHTELF] = "Night Elf";
    startNames[RACE_UNDEAD] = "Undead";
    startNames[RACE_TAUREN] = "Tauren";
    startNames[RACE_GNOME] = "Dwarf and Gnome";
    startNames[RACE_TROLL] = "Orc and Troll";
    startNames[RACE_GOBLIN] = "Goblin";

    for (uint32 i = 0; i < MAX_RACES; i++)
    {
        for (uint32 j = 0; j < MAX_CLASSES; j++)
        {
            PlayerInfo const* info = sObjectMgr.GetPlayerInfo(i, j);

            if (!info)
                continue;

             WorldPosition pos(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

            string nodeName = startNames[i] + " start";

            sTravelNodeMap.addNode(pos, nodeName, true, true);

            break;
        }
    }
}

void TravelNodeMap::generateAreaTriggerNodes()
{
    //Entrance nodes

    for (uint32 i = 0; i < sAreaTriggerStore.GetNumRows(); i++)
    {
        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(i);
        if (!atEntry)
            continue;

        AreaTrigger const* at = sObjectMgr.GetAreaTrigger(i);
        if (!at)
            continue;

        WorldPosition inPos = WorldPosition(atEntry->mapid, atEntry->x, atEntry->y, atEntry->z - 4.0f, 0);

        WorldPosition outPos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);

        string nodeName;

        if (!outPos.isOverworld())
            nodeName = outPos.getAreaName(false) + " entrance";
        else if (!inPos.isOverworld())
            nodeName = inPos.getAreaName(false) + " exit";
        else
            nodeName = inPos.getAreaName(false) + " portal";

        sTravelNodeMap.addNode(inPos, nodeName, true, true);
    }

    //Exit nodes

    for (uint32 i = 0; i < sAreaTriggerStore.GetNumRows(); i++)
    {
        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(i);
        if (!atEntry)
            continue;

        AreaTrigger const* at = sObjectMgr.GetAreaTrigger(i);
        if (!at)
            continue;

        WorldPosition inPos = WorldPosition(atEntry->mapid, atEntry->x, atEntry->y, atEntry->z - 4.0f, 0);

        WorldPosition outPos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);

        string nodeName;

        if (!outPos.isOverworld())
            nodeName = outPos.getAreaName(false) + " entrance";
        else if (!inPos.isOverworld())
            nodeName = inPos.getAreaName(false) + " exit";
        else
            nodeName = inPos.getAreaName(false) + " portal";

        TravelNode* entryNode = sTravelNodeMap.getNode(outPos, NULL, 20.0f); //Entry side, portal exit.

        TravelNode* outNode = sTravelNodeMap.addNode(outPos, nodeName, true, true); //Exit size, portal exit.

        TravelNode* inNode = sTravelNodeMap.getNode(inPos, NULL, 5.0f); //Entry side, portal center.

        //Portal link from area trigger to area trigger destination.
        if (outNode && inNode)
        {
            TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::areaTrigger, i, true);
            travelPath.setPath({ *inNode->getPosition(), *outNode->getPosition() });
            inNode->setPathTo(outNode, travelPath);
        }
    }
}

void TravelNodeMap::generatePortalNodes()
{
    //Static portals.
    for (auto goData : WorldPosition().getGameObjectsNear(0, 0))
    {
        GuidPosition go(goData);

        auto data = sGOStorage.LookupEntry<GameObjectInfo>(go.GetEntry());

        if (!data)
            continue;

        if (data->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(data->spellcaster.spellId);

        if(pSpellInfo->EffectTriggerSpell[0])
            pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        SpellTargetPosition const* pos = sSpellMgr.GetSpellTargetPosition(pSpellInfo->Id);

        if (!pos)
            continue;

        WorldPosition inPos(go);
        WorldPosition outPos(pos);

        TravelNode* inNode = sTravelNodeMap.addNode(inPos, data->name, true, true);
        TravelNode* outNode = sTravelNodeMap.addNode(outPos, data->name, true, true);

        TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::staticPortal, go.GetEntry(), true);
        travelPath.setPath({ *inNode->getPosition(), *outNode->getPosition() });
        inNode->setPathTo(outNode, travelPath); 
    }

    //Portal spell destinations.
    for (uint32 i = 0; i < GetSpellStore()->GetMaxEntry(); ++i)
    {
        const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(i);

        if (!pSpellInfo)
            continue;

        if (pSpellInfo->EffectTriggerSpell[0])
            pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        SpellTargetPosition const* pos = sSpellMgr.GetSpellTargetPosition(pSpellInfo->Id);

        if (!pos)
            continue;

        WorldPosition outPos(pos);

        TravelNode* destNode = sTravelNodeMap.addNode(outPos, pSpellInfo->SpellName[0], true, true);
    }
}

void TravelNodeMap::generateTransportNodes()
{
    for (uint32 entry = 1; entry <= sGOStorage.GetMaxEntry(); ++entry)
    {
        auto data = sGOStorage.LookupEntry<GameObjectInfo>(entry);
        if (data && (data->type == GAMEOBJECT_TYPE_TRANSPORT || data->type == GAMEOBJECT_TYPE_MO_TRANSPORT))
        {
            TransportAnimation const* animation = sTransportMgr.GetTransportAnimInfo(entry);

            uint32 pathId = data->moTransport.taxiPathId;
            float moveSpeed = data->moTransport.moveSpeed;
            if (pathId >= sTaxiPathNodesByPath.size())
                continue;

            TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

            vector<WorldPosition> ppath;
            TravelNode* prevNode = nullptr;

            //Elevators/Trams
            if (path.empty())
            {
                if (animation)
                {
                    TransportPathContainer aPath = animation->Path;
                    float timeStart;

                    for (auto& transport : WorldPosition().getGameObjectsNear(0,entry))
                    {
                        GuidPosition guidP(transport);
                        prevNode = nullptr;
                        WorldPosition basePos(guidP);
                        WorldPosition lPos = WorldPosition();

                        for (auto& p : aPath)
                        {
#ifndef MANGOSBOT_TWO
                            float dx = cos(basePos.getO()) * p.second->X - sin(basePos.getO()) * p.second->Y;
                            float dy = sin(basePos.getO()) * p.second->X + cos(basePos.getO()) * p.second->Y;
#else
                            float dx = -1 * p.second->X;
                            float dy = -1 * p.second->Y;
#endif

                            WorldPosition pos = WorldPosition(basePos.getMapId(), basePos.getX() + dx, basePos.getY() + dy, basePos.getZ() + p.second->Z, basePos.getO());

                            if (prevNode)
                            {
                                ppath.push_back(pos);
                            }

                            if (pos.distance(lPos) == 0)
                            {
                                TravelNode* node = sTravelNodeMap.addNode(pos, data->name, true, true, true, entry);

                                if (!prevNode)
                                {
                                    ppath.push_back(pos);
                                    timeStart = p.second->TimeSeg;
                                }
                                else
                                {
                                    float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;

                                    TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::transport, entry, true);
                                    node->setPathTo(prevNode, travelPath);
                                    ppath.clear();
                                    ppath.push_back(pos);
                                    timeStart = p.second->TimeSeg;
                                }

                                prevNode = node;
                            }

                            lPos = pos;
                        }

                        if (prevNode)
                        {
                            for (auto& p : aPath)
                            {
#ifndef MANGOSBOT_TWO
                                float dx = cos(basePos.getO()) * p.second->X - sin(basePos.getO()) * p.second->Y;
                                float dy = sin(basePos.getO()) * p.second->X + cos(basePos.getO()) * p.second->Y;
#else
                                float dx = -1 * p.second->X;
                                float dy = -1 * p.second->Y;
#endif
                                WorldPosition pos = WorldPosition(basePos.getMapId(), basePos.getX() + dx, basePos.getY() + dy, basePos.getZ() + p.second->Z, basePos.getO());

                                ppath.push_back(pos);

                                if (pos.distance(lPos) == 0)
                                {
                                    TravelNode* node = sTravelNodeMap.addNode(pos, data->name, true, true, true, entry);
                                    if (node != prevNode) {
                                        if (p.second->TimeSeg < timeStart)
                                            timeStart = 0;

                                        float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;

                                        TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::transport, entry, true);
                                        travelPath.setPath(ppath);
                                        node->setPathTo(prevNode, travelPath);
                                        ppath.clear();
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }
                                }

                                lPos = pos;
                            }
                        }

                        ppath.clear();
                    }
                }
            }
            else //Boats/Zepelins
            {
                //Loop over the path and connect stop locations.
                for (auto& p : path)
                {
                    WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                    //if (data->displayId == 3015) 
                    //    pos.setZ(pos.getZ() + 6.0f);
                    //else if(data->displayId == 3031)
                   //     pos.setZ(pos.getZ() - 17.0f);

                    if (prevNode)
                    {
                        ppath.push_back(pos);
                    }

                    if (p->delay > 0)
                    {
                        TravelNode* node = sTravelNodeMap.addNode(pos, data->name, true, true, true, entry);
                        
                        pos.loadMapAndVMap(0);
                        WorldPosition exitPos = pos;

                        if (data->displayId == 3015) 
                            exitPos.setZ(exitPos.getZ() + 6.0f);
                        else if(data->displayId == 3031)
                            exitPos.setZ(exitPos.getZ() - 17.0f);

                        if (exitPos.ClosestCorrectPoint(20.0f, 10.0f))
                        {
                            TravelNode* exitNode = sTravelNodeMap.addNode(exitPos, data->name + string(" dock"), true, true);

                            TravelNodePath travelPath(exitPos.distance(pos), 0.0f, (uint8)TravelNodePathType::walk,0, true);
                            travelPath.setPath({ exitPos, pos });
                            exitNode->setPathTo(node,travelPath,true);
                            travelPath.setPath({ pos, exitPos });
                            node->setPathTo(exitNode, travelPath, true);
                        }

                        if (!prevNode)
                        {
                            ppath.push_back(pos);
                        }
                        else
                        {
                            TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, entry, true);
                            travelPath.setPathAndCost(ppath, moveSpeed);
                            node->setPathTo(prevNode, travelPath);
                            ppath.clear();
                            ppath.push_back(pos);
                        }

                        prevNode = node;
                    }
                }

                if (prevNode)
                {
                    //Continue from start until first stop and connect to end.
                    for (auto& p : path)
                    {
                        WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                        //if (data->displayId == 3015)
                        //    pos.setZ(pos.getZ() + 6.0f);
                        //else if (data->displayId == 3031)
                        //    pos.setZ(pos.getZ() - 17.0f);

                        ppath.push_back(pos);

                        if (p->delay > 0)
                        {
                            TravelNode* node = sTravelNodeMap.getNode(pos, NULL, 5.0f);

                            if (node != prevNode) {
                                TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, entry, true);
                                travelPath.setPathAndCost(ppath, moveSpeed);

                                node->setPathTo(prevNode, travelPath);
                            }
                        }
                    }
                }
                ppath.clear();
            }
        }
    }
}

void TravelNodeMap::generateZoneMeanNodes()
{
    //Zone means   
    for (auto& loc : sTravelMgr.getExploreLocs())
    {
        vector<WorldPosition*> points;

        for (auto p : loc.second->getPoints(true))
            if (!p->isUnderWater())
                points.push_back(p);

        if (points.empty())
            points = loc.second->getPoints(true);

        WorldPosition  pos = WorldPosition(points, WP_MEAN_CENTROID);

        TravelNode* node = sTravelNodeMap.addNode(pos, pos.getAreaName(), true, true, false);
    }
}

void TravelNodeMap::generateNodes()
{
    sLog.outString("-Generating Start nodes");
    generateStartNodes();
    sLog.outString("-Generating npc nodes");
    generateNpcNodes();
    sLog.outString("-Generating area trigger nodes");
    generateAreaTriggerNodes();
    sLog.outString("-Generating transport nodes");
    generateTransportNodes();
    sLog.outString("-Generating zone mean nodes");
    generateZoneMeanNodes();
    sLog.outString("-Generating static portal nodes");
    generatePortalNodes();
}

void TravelNodeMap::generateWalkPathMap(uint32 mapId)
{
    for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
    {
        if (startNode->isLinked())
            continue;

        for (auto& endNode : sTravelNodeMap.getNodes(*startNode->getPosition(), 2000.0f))
        {
            if (startNode == endNode)
                continue;

            if (startNode->hasCompletePathTo(endNode))
                continue;

            if (startNode->getMapId() != endNode->getMapId())
                continue;

            startNode->buildPath(endNode, nullptr, false);
        }

        startNode->setLinked(true);
    }
}

void TravelNodeMap::generateWalkPaths()
{
    //Pathfinder
    vector<WorldPosition> ppath;

    map<uint32, bool> nodeMaps;

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        nodeMaps[startNode->getMapId()] = true;
    }

    vector<std::future<void>> calculations;

    BarGoLink bar(nodeMaps.size());
    for (auto& map : nodeMaps)
    {
        uint32 mapId = map.first;
        calculations.push_back(std::async([this,mapId] { generateWalkPathMap(mapId); }));
        bar.step();
    }

    BarGoLink bar2(calculations.size());
    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
        bar2.step();
    }

    sLog.outString(">> Generated paths for " SIZEFMTD " nodes.", sTravelNodeMap.getNodes().size());
}

void TravelNodeMap::generateHelperNodes(uint32 mapId)
{
    vector<TravelNode*> startNodes = getNodes(WorldPosition(mapId, 1, 1));

    vector<std::pair<WorldPosition, string>> places_to_reach;

    //Find all places we might want to reach.
    for (auto& node : startNodes)
    {
        places_to_reach.push_back(make_pair(GuidPosition(0, *node->getPosition()), node->getName()));
    }

    for (auto& obj : WorldPosition().getCreaturesNear(WorldPosition(mapId, 1, 1)))
    {
        places_to_reach.push_back(make_pair(obj, sObjectMgr.GetCreatureTemplate(obj->second.id)->Name));
    }

    for (auto& obj : WorldPosition().getGameObjectsNear(WorldPosition(mapId, 1, 1)))
    {
        places_to_reach.push_back(make_pair(obj, sObjectMgr.GetGameObjectInfo(obj->second.id)->name));
    }

    if (places_to_reach.empty() || startNodes.empty())
        return;

    m_nMapMtx.lock();
    BarGoLink bar(places_to_reach.size());
    m_nMapMtx.unlock();

    for (auto& pos : places_to_reach)
    {
        startNodes = getNodes(WorldPosition(mapId, 1, 1));

        //Find closest 5 nodes.
        std::partial_sort(startNodes.begin(), startNodes.begin() + std::min(int(startNodes.size()), 5), startNodes.end(), [pos](TravelNode* i, TravelNode* j) {return i->fDist(pos.first) < j->fDist(pos.first); });

        bool found = false;

        for (uint8 i = 0; i < std::min(int(startNodes.size()), 5); i++)
        {
            TravelNode* node = startNodes[i];
            if (node->getPosition()->canPathTo(pos.first, nullptr)) //
                continue;

            for (auto& path : *node->getPaths())
            {
                for (auto& ppoint : path.second.getPath())
                {
                    if (!ppoint.canPathTo(pos.first, nullptr))
                        continue;

                    string name = node->getName() + " to " + pos.second;
                    sTravelNodeMap.addNode(ppoint, name, false, true);
                    found = true;

                    break;
                }

                if (found)
                    break;
            }

            if (found)
            {
                sTravelNodeMap.generateWalkPathMap(mapId);
                break;
            }
        }

        if (!found) {
            string name = pos.second;
            sTravelNodeMap.addNode(pos.first, name, false, true);
        }

        m_nMapMtx.lock();
        bar.step();
        printf("\r%d", mapId);
        fflush(stdout);
        m_nMapMtx.unlock();
    }

    for (auto& node : startNodes)
    {
        node->setLinked(false);
    }

    sTravelNodeMap.generateWalkPathMap(mapId);

    m_nMapMtx.lock();
    bar.SetOutputState(false);
    m_nMapMtx.unlock();
}

void TravelNodeMap::generateHelperNodes()
{
    //Pathfinder
    vector<WorldPosition> ppath;

    map<uint32, bool> nodeMaps;

    uint32 old = sTravelNodeMap.getNodes().size();

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        nodeMaps[startNode->getMapId()] = true;
    }

    vector<std::future<void>> calculations;

    BarGoLink bar(nodeMaps.size());
    for (auto& map : nodeMaps)
    {
        uint32 mapId = map.first;
        calculations.push_back(std::async([this, mapId] { generateHelperNodes(mapId); }));
        bar.step();
    }

    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
    }

    sLog.outString(">> Generated " SIZEFMTD " helpdernodes.", sTravelNodeMap.getNodes().size()-old);
}

void TravelNodeMap::generateTaxiPaths()
{
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);

        if (!taxiPath)
            continue;

        TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);

        if (!startTaxiNode)
            continue;

        TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);

        if (!endTaxiNode)
            continue;

        TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[taxiPath->ID];

        if (nodes.empty())
            continue;

        WorldPosition startPos(startTaxiNode->map_id, startTaxiNode->x, startTaxiNode->y, startTaxiNode->z);
        WorldPosition endPos(endTaxiNode->map_id, endTaxiNode->x, endTaxiNode->y, endTaxiNode->z);

        TravelNode* startNode = sTravelNodeMap.getNode(startPos, nullptr, 15.0f);
        TravelNode* endNode = sTravelNodeMap.getNode(endPos, nullptr, 15.0f);

        if (!startNode || !endNode)
            continue;

        vector<WorldPosition> ppath;

        for (auto& n : nodes)
            ppath.push_back(WorldPosition(n->mapid, n->x, n->y, n->z, 0.0));

        float totalTime = startPos.getPathLength(ppath) / (450 * 8.0f);

        TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::flightPath, i, true);
        travelPath.setPath(ppath);

        startNode->setPathTo(endNode, travelPath);
    }
}

void TravelNodeMap::removeLowNodes()
{
    vector<TravelNode*> goodNodes;
    vector<TravelNode*> remNodes;
    for (auto& node : sTravelNodeMap.getNodes())
    {
        if (!node->getPosition()->isOverworld())
            continue;

        if (std::find(goodNodes.begin(), goodNodes.end(), node) != goodNodes.end())
            continue;

        if (std::find(remNodes.begin(), remNodes.end(), node) != remNodes.end())
            continue;

        vector<TravelNode*> nodes = node->getNodeMap(true);

        if (nodes.size() < 5)
            remNodes.insert(remNodes.end(), nodes.begin(), nodes.end());
        else
            goodNodes.insert(goodNodes.end(), nodes.begin(), nodes.end());
    }

    for (auto& node : remNodes)
        sTravelNodeMap.removeNode(node);
}

void TravelNodeMap::removeUselessPathMap(uint32 mapId)
{
    //Clean up node links
    for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
    {
        for (auto& path : *startNode->getPaths())
            if (path.second.getComplete() && startNode->hasLinkTo(path.first))
                MANGOS_ASSERT(true);
    }
    uint32 it = 0, rem = 0;
    while (true)
    {
        uint32 rem = 0;
        //Clean up node links
        for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
        {
            if (startNode->cropUselessLinks())
                rem++;
        }

        for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
        {
            startNode->clearRoutes();
        }

        if (!rem)
            break;

        hasToSave = true;

        it++;

        sLog.outDetail("MapId %d Iteration %d, removed %d", mapId, it, rem);
    }
}

void TravelNodeMap::removeUselessPaths()
{
    //Pathfinder
    vector<WorldPosition> ppath;

    map<uint32, bool> nodeMaps;

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        nodeMaps[startNode->getMapId()] = true;
    }

    vector<std::future<void>> calculations;

    BarGoLink bar(nodeMaps.size());
    for (auto& map : nodeMaps)
    {
        uint32 mapId = map.first;
        calculations.push_back(std::async([this, mapId] { removeUselessPathMap(mapId); }));
        bar.step();
    }

    BarGoLink bar2(calculations.size());
    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
        bar2.step();
    }
}

void TravelNodeMap::calculatePathCosts()
{   
    BarGoLink bar(sTravelNodeMap.getNodes().size());

    vector<std::future<void>> calculations;

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        bar.step();

        for (auto& path : *startNode->getLinks())
        {
            TravelNodePath* nodePath = path.second;

            if (path.second->getPathType() != TravelNodePathType::walk)
                continue;

            if (nodePath->getCalculated())
                continue;

            calculations.push_back(std::async([nodePath] {nodePath->calculateCost(); }));
        }
    }

    BarGoLink bar2(calculations.size());
    for (uint32 i = 0; i < calculations.size(); i++)
    {
        bar2.step();
        calculations[i].wait();
    }

    sLog.outString(">> Calculated cost for " SIZEFMTD " paths.", calculations.size());

    if (calculations.size()) //Repeat until we have all paths calculated.
        calculatePathCosts();
}

void TravelNodeMap::generatePaths(bool helpers)
{
    sLog.outString("-Calculating walkable paths");
    generateWalkPaths();

    if (helpers)
    {
        sLog.outString("-Generating helper nodes");
        generateHelperNodes();
    }

    sLog.outString("-Removing useless nodes");
    removeLowNodes();
    sLog.outString("-Removing useless paths");
    removeUselessPaths();

    printMap();

    sLog.outString("-Calculating path costs");
    calculatePathCosts();
    sLog.outString("-Generating taxi paths");
    generateTaxiPaths();
}

void TravelNodeMap::generateAll()
{
    if (hasToFullGen)
        generateNodes();

    sLog.outString("-Calculating mapoffset");
    calcMapOffset();

    sLog.outString("-Generating maptransfers");
    sTravelMgr.loadMapTransfers();

    if (hasToGen || hasToFullGen)
    {
        generatePaths();
        hasToGen = false;
        hasToFullGen = false;
        hasToSave = true;
    }

    sLog.outString("-Calculating coverage"); //This prevents crashes when bots from multiple maps try to calculate this on the fly.
    for (auto& node : getNodes())
        node->hasRouteTo(node);
}

void TravelNodeMap::printMap()
{
    if (!sPlayerbotAIConfig.hasLog("travelNodes.csv") && !sPlayerbotAIConfig.hasLog("travelPaths.csv"))
        return;

    printf("\r [Qgis] \r\x3D");
    fflush(stdout);

    sPlayerbotAIConfig.openLog("travelNodes.csv", "w");
    sPlayerbotAIConfig.openLog("travelPaths.csv", "w");

    vector<TravelNode*> anodes = getNodes();

    uint32 nr = 0;

    for (auto& node : anodes)
    {
        node->print(false);
    }
}

void TravelNodeMap::printNodeStore()
{
    string nodeStore = "TravelNodeStore.h";

    if (!sPlayerbotAIConfig.hasLog(nodeStore))
        return;

    printf("\r [Map] \r\x3D");
    fflush(stdout);

    sPlayerbotAIConfig.openLog(nodeStore, "w");

    std::unordered_map<TravelNode*, uint32> saveNodes;

    vector<TravelNode*> anodes = getNodes();

    sPlayerbotAIConfig.log(nodeStore, "#pragma once");
    sPlayerbotAIConfig.log(nodeStore, "#include \"TravelMgr.h\"");
    sPlayerbotAIConfig.log(nodeStore, "namespace ai");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "    class TravelNodeStore");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "    public:");
    sPlayerbotAIConfig.log(nodeStore, "    static void loadNodes()");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "        TravelNode** nodes = new TravelNode*[%d];", anodes.size());

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        ostringstream out;

        string name = node->getName();
        name.erase(remove(name.begin(), name.end(), '\"'), name.end());

        //        struct addNode {uint32 node; WorldPosition point; string name; bool isPortal; bool isTransport; uint32 transportId; };
        out << std::fixed << std::setprecision(2) << "        addNodes.push_back(addNode{" << i << ",";
        out << "WorldPosition(" << node->getMapId() << ", " << node->getX() << "f, " << node->getY() << "f, " << node->getZ() << "f, " << node->getO() << "f),";
        out << "\"" << name << "\"";
        if (node->isTransport())
            out << "," << (node->isTransport() ? "true" : "false") << "," << node->getTransportId();
        out << "});";

        /*
                out << std::fixed << std::setprecision(2) << "        nodes[" << i << "] = sTravelNodeMap.addNode(&WorldPosition(" << node->getMapId() << "," << node->getX() << "f," << node->getY() << "f," << node->getZ() << "f,"<< node->getO() <<"f), \""
                    << name << "\", " << (node->isImportant() ? "true" : "false") << ", true";
                if (node->isTransport())
                    out << "," << (node->isTransport() ? "true" : "false") << "," << node->getTransportId();

                out << ");";
                */
        sPlayerbotAIConfig.log(nodeStore, out.str().c_str());

        saveNodes.insert(make_pair(node, i));
    }

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        for (auto& Link : *node->getLinks())
        {
            ostringstream out;

            //        struct linkNode { uint32 node1; uint32 node2; float distance; float extraCost; bool isPortal; bool isTransport; uint32 maxLevelMob; uint32 maxLevelAlliance; uint32 maxLevelHorde; float swimDistance; };

            out << std::fixed << std::setprecision(2) << "        linkNodes3.push_back(linkNode3{" << i << "," << saveNodes.find(Link.first)->second << ",";
            out << Link.second->print() << "});";

            //out << std::fixed << std::setprecision(1) << "        nodes[" << i << "]->setPathTo(nodes[" << saveNodes.find(Link.first)->second << "],TravelNodePath(";
            //out << Link.second->print() << "), true);";
            sPlayerbotAIConfig.log(nodeStore, out.str().c_str());
        }
    }

    sPlayerbotAIConfig.log(nodeStore, "	}");
    sPlayerbotAIConfig.log(nodeStore, "};");
    sPlayerbotAIConfig.log(nodeStore, "}");

    printf("\r [Done] \r\x3D");
    fflush(stdout);
}

void TravelNodeMap::saveNodeStore(bool force)
{
    if (!hasToSave && !force)
        return;

    hasToSave = false;

    WorldDatabase.PExecute("DELETE FROM ai_playerbot_travelnode");
    WorldDatabase.PExecute("DELETE FROM ai_playerbot_travelnode_link");
    WorldDatabase.PExecute("DELETE FROM ai_playerbot_travelnode_path");

    std::unordered_map<TravelNode*, uint32> saveNodes;
    vector<TravelNode*> anodes = sTravelNodeMap.getNodes();

    BarGoLink bar(anodes.size());
    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        string name = node->getName();
        name.erase(remove(name.begin(), name.end(), '\''), name.end());

        WorldDatabase.PExecute("INSERT INTO `ai_playerbot_travelnode` (`id`, `name`, `map_id`, `x`, `y`, `z`, `linked`) VALUES ('%lu', '%s', '%d', '%f', '%f', '%f', '%d%')"
            , i, name.c_str(), node->getMapId(), node->getX(), node->getY(), node->getZ(), (node->isLinked() ? 1 : 0));

        saveNodes.insert(make_pair(node, i));

        bar.step();
    }

    sLog.outString(">> Saved " SIZEFMTD " travelNodes.", anodes.size());

    {
        uint32 paths = 0, points = 0;
        BarGoLink bar(anodes.size());
        for (uint32 i = 0; i < anodes.size(); i++)
        {
            TravelNode* node = anodes[i];

            for (auto& link : *node->getLinks())
            {
                TravelNodePath* path = link.second;

                WorldDatabase.PExecute("INSERT INTO `ai_playerbot_travelnode_link` (`node_id`, `to_node_id`,`type`,`object`,`distance`,`swim_distance`, `extra_cost`,`calculated`, `max_creature_0`,`max_creature_1`,`max_creature_2`) VALUES ('%lu','%lu', '%d', '%lu', '%f', '%f', '%f', '%d', '%d', '%d', '%d')"
                    , i
                    , saveNodes.find(link.first)->second
                    , uint8(path->getPathType())
                    , path->getPathObject()
                    , path->getDistance()
                    , path->getSwimDistance()
                    , path->getExtraCost()
                    , (path->getCalculated() ? 1 : 0)
                    , path->getMaxLevelCreature()[0]
                    , path->getMaxLevelCreature()[1]
                    , path->getMaxLevelCreature()[2]);

                paths++;

                vector<WorldPosition> ppath = path->getPath();

                for (uint32 j = 0; j < ppath.size(); j++)
                {
                    WorldPosition point = ppath[j];
                    WorldDatabase.PExecute("INSERT INTO `ai_playerbot_travelnode_path` (`node_id`, `to_node_id`, `nr`, `map_id`, `x`, `y`, `z`) VALUES ('%lu', '%lu', '%d','%d', '%f', '%f', '%f')"
                        , i
                        , saveNodes.find(link.first)->second
                        , j
                        , point.getMapId()
                        , point.getX()
                        , point.getY()
                        , point.getZ());

                    points++;
                }
            }

            bar.step();
        }

        sLog.outString(">> Saved %d travelNode Paths, %d points.", paths,points);
    }
}

void TravelNodeMap::loadNodeStore()
{    
    string query = "SELECT id, name, map_id, x, y, z, linked FROM ai_playerbot_travelnode";

    std::unordered_map<uint32, TravelNode*> saveNodes;   

    {
        auto result = WorldDatabase.PQuery(query.c_str());

        if (result)
        {
            BarGoLink bar(result->GetRowCount());
            do
            {
                Field* fields = result->Fetch();
                bar.step();

                TravelNode* node = addNode(WorldPosition(fields[2].GetUInt32(), fields[3].GetFloat(), fields[4].GetFloat(), fields[5].GetFloat()), fields[1].GetCppString(), true);

                if (fields[6].GetBool())
                    node->setLinked(true);
                else
                    hasToGen = true;

                saveNodes.insert(make_pair(fields[0].GetUInt32(), node));

            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " travelNodes.", saveNodes.size());
        }
        else
        {
            hasToFullGen = true;
            sLog.outString();
            sLog.outErrorDb(">> Error loading travelNodes.");
            return;
        }
    }

    {
        //                     0        1          2    3      4         5              6          7          8               9             10 
        string query = "SELECT node_id, to_node_id,type,object,distance,swim_distance, extra_cost,calculated, max_creature_0,max_creature_1,max_creature_2 FROM ai_playerbot_travelnode_link";

        auto result = WorldDatabase.PQuery(query.c_str());

        if (result)
        {
            BarGoLink bar(result->GetRowCount());
            do
            {
                Field* fields = result->Fetch();
                bar.step();

                TravelNode* startNode = saveNodes.find(fields[0].GetUInt32())->second;
                TravelNode* endNode = saveNodes.find(fields[1].GetUInt32())->second;

                if (!startNode || !endNode)
                    continue;

                startNode->setPathTo(endNode, TravelNodePath(fields[4].GetFloat(), fields[6].GetFloat(), fields[2].GetUInt8(), fields[3].GetUInt64(), fields[7].GetBool(), { fields[8].GetUInt8(),fields[9].GetUInt8(),fields[10].GetUInt8() }, fields[5].GetFloat()), true);

                if (!fields[7].GetBool())
                    hasToGen = true;

            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " travelNode paths.", result->GetRowCount());
        }
        else
        {
            sLog.outString();
            sLog.outErrorDb(">> Error loading travelNode links.");
        }
    }

    {
        //                     0        1           2   3      4   5  6
        string query = "SELECT node_id, to_node_id, nr, map_id, x, y, z FROM ai_playerbot_travelnode_path order by node_id, to_node_id, nr";

        auto result = WorldDatabase.PQuery(query.c_str());

        if (result)
        {
            BarGoLink bar(result->GetRowCount());
            do
            {
                Field* fields = result->Fetch();
                bar.step();

                TravelNode* startNode = saveNodes.find(fields[0].GetUInt32())->second;
                TravelNode* endNode = saveNodes.find(fields[1].GetUInt32())->second;

                if (!startNode || !endNode || !startNode->hasPathTo(endNode))
                    continue;

                TravelNodePath* path = startNode->getPathTo(endNode);

                vector<WorldPosition> ppath = path->getPath();
                ppath.push_back(WorldPosition(fields[3].GetUInt32(), fields[4].GetFloat(), fields[5].GetFloat(), fields[6].GetFloat()));

                path->setPath(ppath);

                if (path->getCalculated())
                    path->setComplete(true);

            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " travelNode paths points.", result->GetRowCount());
        }
        else
        {
            sLog.outString();
            sLog.outErrorDb(">> Error loading travelNode paths.");
        }
    }
}

void TravelNodeMap::calcMapOffset()
{
    mapOffsets.push_back(make_pair(0, WorldPosition(0, 0, 0, 0, 0)));
    mapOffsets.push_back(make_pair(1, WorldPosition(1, -3680.0, 13670.0, 0, 0)));
    mapOffsets.push_back(make_pair(530, WorldPosition(530, 15000.0, -20000.0, 0, 0)));
    mapOffsets.push_back(make_pair(571, WorldPosition(571, 10000.0, 5000.0, 0, 0)));

    vector<uint32> mapIds;

    for (auto& node : m_nodes)
    {
        if (!node->getPosition()->isOverworld())
            if (std::find(mapIds.begin(), mapIds.end(), node->getMapId()) == mapIds.end())
                mapIds.push_back(node->getMapId());
    }

    std::sort(mapIds.begin(), mapIds.end());

    vector<WorldPosition> min, max;

    for (auto& mapId : mapIds)
    {
        bool doPush = true;
        for (auto& node : m_nodes)
        {
            if (node->getMapId() != mapId)
                continue;

            if (doPush)
            {
                min.push_back(*node->getPosition());
                max.push_back(*node->getPosition());
                doPush = false;
            }
            else
            {
                min.back().setX(std::min(min.back().getX(), node->getX()));
                min.back().setY(std::min(min.back().getY(), node->getY()));
                max.back().setX(std::max(max.back().getX(), node->getX()));
                max.back().setY(std::max(max.back().getY(), node->getY()));
            }
        }
    }

    WorldPosition curPos = WorldPosition(0, -13000, -13000, 0, 0);
    WorldPosition endPos = WorldPosition(0, 3000, -13000, 0, 0);

    uint32 i = 0;
    float maxY = 0;
    //+X -> -Y
    for (auto& mapId : mapIds)
    {
        mapOffsets.push_back(make_pair(mapId, WorldPosition(mapId, curPos.getX() - min[i].getX(), curPos.getY() - max[i].getY(), 0, 0)));

        maxY = std::max(maxY, (max[i].getY() - min[i].getY() + 500));
        curPos.setX(curPos.getX() + (max[i].getX() - min[i].getX() + 500));

        if (curPos.getX() > endPos.getX())
        {
            curPos.setY(curPos.getY() - maxY);
            curPos.setX(-13000);
        }
        i++;
    }
}

WorldPosition TravelNodeMap::getMapOffset(uint32 mapId)
{
    for (auto& offset : mapOffsets)
    {
        if (offset.first == mapId)
            return offset.second;
    }

    return WorldPosition(mapId, 0, 0, 0, 0);
}