#include "WorldSquare.h"
#include "TravelMgr.h"


using namespace ai;
void WorldPointContainer::printWKT(std::ostringstream& out, const uint32 dim, const bool loop) const
{
    switch (dim) {
    case 0:
        if (points.size() == 1)
            out << "\"POINT(";
        else
            out << "\"MULTIPOINT(";
        break;
    case 1:
        out << "\"LINESTRING(";
        break;
    case 2:
        out << "\"POLYGON((";
    }

    for (auto& p : points)
        out << p->getDisplayX() << " " << p->getDisplayY() << (!loop && &p == &points.back() ? "" : ",");

    if (loop)
        out << points.front()->getDisplayX() << " " << points.front()->getDisplayY();

    out << (dim == 2 ? "))\"," : ")\",");
}

float MapWpSquare::sqOutDistance(const WorldPosition& point) const
{
    if (point.mapid != mapId)
    {
        float minSqDist = FLT_MAX;
        //distance = point -> portal (on map of square)
        for (auto& [portal, distance] : sTravelMgr.sqMapTransDistances(point, mapId))
        {
            float sqDist = distance + sqDistance(portal); //Add sqDistance = portal -> square.
            if (sqDist < minSqDist)
                minSqDist = sqDist;
        }

        return minSqDist;
    }

    return WorldPointSquareContainer::sqOutDistance(point);
}

uint32 WorldWpSquare::ClosestMapId(const WorldPosition& point) const
{
    uint32 closetMapId = 999;
    float minsqDistance = FLT_MAX;
    for (auto& mapId : GetSubSquareIds())
    {
        float sqDistance = GetSubSquare(mapId).sqDistance(point);

        if (sqDistance < minsqDistance)
        {
            closetMapId = mapId;
            minsqDistance = sqDistance;
        }
    }

    return closetMapId;
}