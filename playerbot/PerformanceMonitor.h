#ifndef _PerformanceMonitor_H
#define _PerformanceMonitor_H
#define PMO_MEMTEST

#include "Common.h"
#include "PlayerbotAIBase.h"

#include <mutex>
#include <chrono>
#include <ctime>

typedef std::vector<std::string> PerformanceStack;

struct PerformanceData
{
    uint32 minTime, maxTime, totalTime, count;
    std::mutex lock;
};

enum PerformanceMetric
{
    PERF_MON_VALUE,
    PERF_MON_TRIGGER,
    PERF_MON_ACTION,
    PERF_MON_RNDBOT,
    PERF_MON_TOTAL
};

class PerformanceMonitorOperation
{
public:
    PerformanceMonitorOperation(PerformanceData& data, std::string name, PerformanceStack* stack);
    ~PerformanceMonitorOperation();

private:
    void finish();

    PerformanceData& data;
    std::string name;
    PerformanceStack* stack;
#ifdef CMANGOS
    std::chrono::milliseconds started;
#endif
};

struct VectorStringHash
{
    std::size_t operator()(const std::vector<std::string>& v) const
    {
        std::size_t seed = 0;
        for (const auto& str : v)
        {
            // Combine with XOR and bit shifting (fast)
            seed ^= std::hash<std::string> {}(str) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};

using performanceMap = std::unordered_map<std::vector<std::string>, PerformanceData, VectorStringHash>;
using performanceMetricMap = std::map<PerformanceMetric, performanceMap>;
using performanceInstanceMap = std::map<uint32, performanceMetricMap>;
using performanceMapMap = std::map<uint32, performanceInstanceMap>;

class PerformanceMonitor
{
    public:
        PerformanceMonitor();
        virtual ~PerformanceMonitor();
        static PerformanceMonitor& instance()
        {
            static PerformanceMonitor instance;
            return instance;
        }

    public:
        std::unique_ptr<PerformanceMonitorOperation> start(PerformanceMetric metric, std::string name, PerformanceStack* stack = nullptr, uint32 mapId = 0, uint32 instanceId = 0);
        std::unique_ptr<PerformanceMonitorOperation> start(PerformanceMetric metric, std::string name, PlayerbotAI* ai);
        void PrintStats(bool perTick = false,  bool fullStack = false, bool showMap = false);
        void Reset();
        void Init(uint32 mapId, uint32 instanceId);
    private:
        performanceMetricMap data;
        performanceMapMap mapsData;
        //std::mutex lock;
};


#define sPerformanceMonitor PerformanceMonitor::instance()

#endif

