#pragma once
#include "WorldPosition.h"

//This is a location container for quicker lookup of a collection of related points.
//Locations are added and sorted into map, grid, cell containers and the bounding box of each container are calculated.
//When determining the (distance to) the closest point or what points fall in a distance range aproximations are used based on the bounding boxes.
//This is to replace the old system where 1000's of points were stored in a vector and such determinations required a loop and distance calculation over each point.

namespace ai
{
    class WorldPointSquare
    {
    public:
        virtual bool IsIn(const WorldPosition& point) const {
            return point.coord_x >= min.coord_x && point.coord_x <= max.coord_x && point.coord_y >= min.coord_y && point.coord_y <= max.coord_y;
        }

        virtual float sqInDistance(const WorldPosition& point) const = 0;

        virtual float sqOutDistance(const WorldPosition& point) const
        {
            float sqX = std::min(std::max(min.getX(), point.getX()), max.getX());
            float sqY = std::min(std::max(min.getY(), point.getY()), max.getY());

            return point.sqDistance2d(WorldPosition(0, sqX, sqY));
        }

        virtual float sqMaxOutDistance(const WorldPosition& point) const
        {
            float farX = (point.getX() < (min.getX() + max.getX()) / 2) ? max.getX() : min.getX();
            float farY = (point.getY() < (min.getY() + max.getY()) / 2) ? max.getY() : min.getY();

            return point.sqDistance(WorldPosition(0, farX, farY));
        }

        virtual float sqDistance(const WorldPosition& point) const
        {
            if (IsIn(point))
                return sqInDistance(point);
            else
                return sqOutDistance(point);   
        }   

        virtual uint32 GetCurrentPartition(const WorldPosition& point, const std::vector<uint32>& distancePartitions) const
        {
            float minDist = sqOutDistance(point);
            float maxDist = sqMaxOutDistance(point);

            uint32 LastPartition = 0;
            for (auto& partition : distancePartitions)
            {
                uint32 sqPartition = partition * partition;
                if (minDist < sqPartition && maxDist < sqPartition && minDist >= LastPartition && maxDist >= LastPartition)
                {
                    return sqPartition;
                    break;
                }

                LastPartition = sqPartition;
            }

            return 0;
        }

        virtual void AddPoint(WorldPosition* point) 
        {
            if (min)
            {
                min.setX(std::min(min.getX(), point->getX()));
                min.setY(std::min(min.getY(), point->getY()));
                min.setZ(std::min(min.getZ(), point->getZ()));
                max.setX(std::max(max.getX(), point->getX()));
                max.setY(std::max(max.getY(), point->getY()));
                max.setZ(std::max(max.getZ(), point->getZ()));
            }
            else
            {
                min = *point;
                max = *point;
            }
        }

        virtual uint32 GetSize() const = 0;
    private:
        WorldPosition min, max;
    };

    class WorldPointContainer : public WorldPointSquare
    {
    public:
        virtual float sqInDistance(const WorldPosition& point) const
        {
            float minDistSq = FLT_MAX;
            for (auto& p : points)
            {
                float distSq = p->sqDistance(point);
                if (distSq < minDistSq)
                    minDistSq = distSq;
            }

            return minDistSq;
        }

        virtual float sqDistance(const WorldPosition& point) const
        {
            if (IsIn(point))
                return sqInDistance(point);
            else
                return sqOutDistance(point);
        }

        virtual std::pair<uint32, std::vector<WorldPosition*>> GetClosestPartition(const WorldPosition& point, const std::vector<uint32>& distancePartitions) const
        {
            std::pair<uint32, std::vector<WorldPosition*>> closestPartition = { 0,{} };

            if (!IsIn(point))
            {
                float minDist = sqOutDistance(point);
                float maxDist = sqMaxOutDistance(point);

                uint32 currentPartition = GetCurrentPartition(point, distancePartitions);

                if (currentPartition) //All points fall within this partition get all.
                {
                    closestPartition = { currentPartition, {} };

                    for (auto& p : points)
                        closestPartition.second.push_back(p);

                    return closestPartition;
                }
            }

            for (auto& p : points) //Check all points.
            {
                float distance = p->sqDistance2d(point);

                uint32 currentPartition = 0;
                for (auto& partition : distancePartitions)
                {
                    uint32 sqPartition = partition * partition;
                    if (partition == distancePartitions.back() || distance < sqPartition)
                    {
                        currentPartition = sqPartition;
                        break;
                    }
                }

                if (closestPartition.first && currentPartition >= closestPartition.first)
                    continue;

                if (currentPartition < closestPartition.first || !closestPartition.first)
                {
                    closestPartition.first = currentPartition;
                    closestPartition.second.clear();
                }

                if (currentPartition == closestPartition.first)
                    closestPartition.second.push_back(p);
            }

            return closestPartition;
        }

        virtual WorldPosition* GetClosestPoint(const WorldPosition& point) const
        {
            float minDist = FLT_MAX;
            WorldPosition* minPoint = nullptr;
            for (auto& p : points) //Check all points.
            {
                float distance = p->sqDistance2d(point);

                if (distance >= minDist)
                    continue;

                minPoint = p;   
                minDist = distance;
            }

            return minPoint;
        }

        virtual WorldPosition* GetNextPoint(const WorldPosition& point) const
        {
            WorldPosition* nextPoint = nullptr;

            for (int i = 0; i < 10; i++)
            {
                nextPoint = points[urand(0, points.size() - 1)];

                if (*nextPoint != point && points.size())
                    return nextPoint;
            }

            return nullptr;
        }

        static uint32 GetGridId(const WorldPosition* point)
        {
            uint32 gridX = point->getGridPair().x_coord;
            uint32 gridY = point->getGridPair().y_coord;
            return gridX + MAX_NUMBER_OF_GRIDS * gridY;
        }

        static uint32 GetCellId(const WorldPosition* point)
        {
            uint32 gridX = point->getGridPair().x_coord;
            uint32 gridY = point->getGridPair().y_coord;
            uint32 cellX = point->getCellPair().x_coord;
            uint32 cellY = point->getCellPair().y_coord;
            cellX -= MAX_NUMBER_OF_CELLS * gridX;
            cellY -= MAX_NUMBER_OF_CELLS * gridY;
            return cellX + MAX_NUMBER_OF_CELLS * cellY;
        }

        virtual void AddPoint(WorldPosition* point) {
            points.push_back(point);
           
            WorldPointSquare::AddPoint(point);
        }

        const std::vector<WorldPosition*>& GetPoints() const { return points; }

        void printWKT(std::ostringstream& out, const uint32 dim = 0, const bool loop = false) const;

        virtual uint32 GetSize() const override { return points.size(); }
    private:
        std::vector<WorldPosition*> points;
    };

    template <class T>
    class WorldPointSquareContainer : public WorldPointSquare {
    public:
        virtual float sqInDistance(const WorldPosition& point) const
        {
            float minDistSq = FLT_MAX;
            for (auto& [id, sq] : subSquares)
            {
                float distSq = sq.sqDistance(point);
                if (distSq < minDistSq)
                    minDistSq = distSq;
            }

            return minDistSq;
        }

        virtual std::pair<uint32, std::vector<WorldPosition*>> GetClosestPartition(const WorldPosition& point, const std::vector<uint32>& distancePartitions) const
        {
            std::pair<uint32, std::vector<WorldPosition*>> closestPartition = { 0,{} };

            if (!IsIn(point))
            {
                float minDist = sqOutDistance(point);
                float maxDist = sqMaxOutDistance(point);

                uint32 currentPartition = GetCurrentPartition(point, distancePartitions);

                if (currentPartition) //All squares fall within this partition 
                {
                    for (auto& [id, sq] : subSquares)
                    {
                        closestPartition.first = currentPartition;
                        for(auto& p : sq.GetPoints())
                            closestPartition.second.push_back(p);
                    }

                    return closestPartition;
                }
            }

            for (auto& [id, sq] : subSquares) //Check all squares
            {
                std::pair<uint32,std::vector<WorldPosition*>> subPartitions = sq.GetClosestPartition(point, distancePartitions);
               
                if (closestPartition.first && subPartitions.first >= closestPartition.first)
                    continue;

                if (subPartitions.first < closestPartition.first || closestPartition.first == 0)
                {
                    closestPartition.first = subPartitions.first;
                    closestPartition.second.clear();
                }

                if (subPartitions.first == closestPartition.first)
                    closestPartition.second.insert(closestPartition.second.end(), subPartitions.second.begin(), subPartitions.second.end());
            }

            return closestPartition;
        }

        virtual const T* GetClosestSquare(const WorldPosition& point) const
        {
            float minDist = FLT_MAX;
            const T* minSq = nullptr;

            for (const auto& [id, sq] : subSquares)
            {
                float distance = sq.sqDistance(point);
                if (distance >= minDist)
                    continue;

                minSq = &sq;
                minDist = distance;
            }
            return minSq;
        }

        virtual WorldPosition* GetClosestPoint(const WorldPosition& point) const
        {
            return GetClosestSquare(point)->GetClosestPoint(point);
        }

        virtual WorldPosition* GetNextPoint(const WorldPosition& point) const
        {
            if (subSquares.empty())
                return nullptr;
            
            const T* nextSq = urand(0, 1)
                ? GetClosestSquare(point)
                : &std::next(subSquares.begin(), urand(0, subSquares.size() - 1))->second;

            if (!nextSq)
                return nullptr;

            return nextSq->GetNextPoint(point);
        }

        virtual void AddPoint(WorldPosition* point) override {
            subSquares[GetSubSquareId(point)].AddPoint(point);
            WorldPointSquare::AddPoint(point);
        }

        const std::vector<WorldPosition*> GetPoints() const { std::vector<WorldPosition*> points; for (auto& [id, sq] : subSquares) points.insert(points.end(), sq.GetPoints().begin(), sq.GetPoints().end()); return points; }

        void printWKT(std::ostringstream& out, const uint32 dim = 0, const bool loop = false) const {for (auto& [id, sq] : subSquares) sq.printWKT(out, dim, loop);}

        virtual uint32 GetSize() const override { return subSquares.begin()->second.GetSize(); }
    protected:
        virtual uint32 GetSubSquareId(const WorldPosition* point) const = 0;

        bool HasSubSquare(uint32 id) const { return subSquares.find(id) != subSquares.end(); }
        std::vector<uint32> GetSubSquareIds() const { std::vector<uint32> ids; for (auto& [id, sq] : subSquares) ids.push_back(id); return ids; }
        const T& GetSubSquare(uint32 id) const { return subSquares.at(id); }
    private:
        std::unordered_map<uint32, T> subSquares;
    };

    class CellWpSquare: public WorldPointContainer
    {
    };

    class GridWpSquare : public WorldPointSquareContainer<CellWpSquare> 
    {
    public:
        virtual void AddPoint(WorldPosition* point) override {            
            WorldPointSquareContainer::AddPoint(point);
        }  
    protected:
        static uint32 GetCellId(const WorldPosition* point)
        {
            uint32 gridX = point->getGridPair().x_coord;
            uint32 gridY = point->getGridPair().y_coord;
            uint32 cellX = point->getCellPair().x_coord;
            uint32 cellY = point->getCellPair().y_coord;
            cellX -= MAX_NUMBER_OF_CELLS * gridX;
            cellY -= MAX_NUMBER_OF_CELLS * gridY;
            return cellX + MAX_NUMBER_OF_CELLS * cellY;
        }

        virtual uint32 GetSubSquareId(const WorldPosition* point) const override { return GetCellId(point); };
    };

    class MapWpSquare : public WorldPointSquareContainer<GridWpSquare>
    {
    public:      
        virtual bool IsIn(const WorldPosition& point) const override {
            if (point.getMapId() != mapId) return false; return WorldPointSquareContainer
                ::IsIn(point);
        }

        virtual float sqOutDistance(const WorldPosition& point) const override;

        virtual void AddPoint(WorldPosition* point) override {
            WorldPointSquareContainer::AddPoint(point);
            mapId = point->getMapId();
        }
    private:
        static uint32 GetGridId(const WorldPosition* point)
        {
            uint32 gridX = point->getGridPair().x_coord;
            uint32 gridY = point->getGridPair().y_coord;
            return gridX + MAX_NUMBER_OF_GRIDS * gridY;
        }

        virtual uint32 GetSubSquareId(const WorldPosition* point) const override { return GetGridId(point); };
        uint32 mapId;
    };

    class WorldWpSquare : public WorldPointSquareContainer<MapWpSquare>
    {
    public:
        virtual void AddPoint(WorldPosition* point) {
            WorldPointSquareContainer::AddPoint(point);
        }      
    protected:
        virtual bool IsIn(const WorldPosition& point) const override { return true; }
        virtual bool OnMap(const WorldPosition& point) const { return HasSubSquare(point.getMapId()); }
        virtual uint32 ClosestMapId(const WorldPosition& point) const;
        const MapEntry* ClosetMapEntry(const WorldPosition& point) const { return sMapStore.LookupEntry(ClosestMapId(point));}
        virtual bool IsOverWorld(const WorldPosition& point) const { const MapEntry* entry = ClosetMapEntry(point); return entry ? entry->IsContinent() : false; }
        virtual bool IsUnique() const { return GetSize() == 1; }
    private:
        virtual uint32 GetSubSquareId(const WorldPosition* point) const override { return point->mapid; };
    };
}
