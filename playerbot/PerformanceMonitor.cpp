#include "playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "PerformanceMonitor.h"

#include "Database/DatabaseEnv.h"
#include "PlayerbotAI.h"

PerformanceMonitor::PerformanceMonitor() 
{

}

PerformanceMonitor::~PerformanceMonitor()
{
}

std::unique_ptr<PerformanceMonitorOperation> PerformanceMonitor::start(PerformanceMetric metric, std::string name, PerformanceStack* stack, uint32 mapId, uint32 instanceId)
{
    if (!sPlayerbotAIConfig.perfMonEnabled)
    {
        return {};
    }

    auto md = mapsData.find(mapId);

    if (md == mapsData.end())
        return nullptr;

    auto id = md->second.find(instanceId);

    if (id == md->second.end())
        return nullptr;

    std::vector<std::string> localStack;

    // Build the key vector efficiently
    if (stack)
    {
        stack->push_back(name);
        localStack = *stack;
    }
    else
    {
        localStack = {name};
    }

    auto& pd = id->second[metric][localStack];

    return std::make_unique<PerformanceMonitorOperation>(pd, name, stack);
}

std::unique_ptr<PerformanceMonitorOperation> PerformanceMonitor::start(PerformanceMetric metric, std::string name, PlayerbotAI * ai)
{
    if (!sPlayerbotAIConfig.perfMonEnabled) return NULL;

    if (ai->GetAiObjectContext())
        return start(metric, name, &ai->GetAiObjectContext()->performanceStack, ai->GetBot()->GetMapId(), ai->GetBot()->GetInstanceId());
    else
        return start(metric, name);
}

std::string StackString(const std::vector<std::string>& stack, bool fullStack = false)
{
    if (stack.empty())
        return "";

    if (stack.size() == 1)
        return stack[0];

    // Pre-calculate total size to avoid reallocations
    size_t totalSize = stack.back().size() + 3; // last element + " []"
    for (size_t i = 0; i < stack.size() - 1; ++i)
        totalSize += stack[i].size() + 1; // +1 for '|' separator

    std::string result;
    result.reserve(totalSize);

    // Add the last element (most recent)
    result = stack.back();
    result += " [";

    // Add remaining elements in reverse order (oldest first)
    for (auto it = stack.rbegin() + 1; it != stack.rend(); ++it)
    {
        result += *it;

        if (!fullStack)
            break;

        if (std::next(it) != stack.rend())
            result += '|';
    }

    result += ']';
    return result;
}

void PerformanceMonitor::PrintStats(bool perTick, bool fullStack, bool showMap)
{
    if (mapsData.empty())
        return;

    uint32 total = 0;

    performanceMetricMap data;

    for (auto& [mapId, mapData] : mapsData)
    {
        for (auto& [instanceId, instanceData] : mapData)
        {
            for (auto& [metric, namedData] : instanceData)
            {
                for (auto& [stack, performanceData] : namedData)
                {
                    std::vector<std::string> newStack = stack;
                    
                    if (showMap)
                    {
                        if (metric != PERF_MON_TOTAL || (newStack[0].find("PlayerbotAIBase::FullTick") == std::string::npos && (newStack[0].find("PlayerbotAI::UpdateAI") == std::string::npos || newStack[0].find(" I") != newStack[0].size() - 2)))
                            newStack[0] = newStack[0] + " " + std::to_string(mapId) + (instanceId ? " (" + std::to_string(instanceId) + ")" : "");
                    }                    

                    PerformanceData& pd = data[metric][newStack];

                    if (performanceData.totalTime > 0)
                    {
                        if (!pd.minTime || pd.minTime > performanceData.minTime)
                            pd.minTime = performanceData.minTime;
                        if (!pd.maxTime || pd.maxTime < performanceData.minTime)
                            pd.maxTime = performanceData.minTime;
                        pd.totalTime += performanceData.totalTime;
                    }
                    pd.count += performanceData.count;
                }
            }
        }
    }

    if (data.empty())
        return;

    uint32 totalCount = 0;

    sLog.outString(" ");
    sLog.outString(" ");

    if (!perTick)
    {
        for (auto& [name, performanceData] : data[PERF_MON_TOTAL])
        {
            if (name[0].find("PlayerbotAI::UpdateAI ") == 0)
            {
                totalCount += performanceData.count;
                total += performanceData.totalTime;
            }
        }

        sLog.outString("--------------------------------------[TOTAL BOT]------------------------------------------------------");
    }
    else
    {
        totalCount = data[PERF_MON_TOTAL][{"PlayerbotAIBase::FullTick"}].count;
        total = data[PERF_MON_TOTAL][{"PlayerbotAIBase::FullTick"}].totalTime;

        sLog.outString("---------------------------------------[PER TICK]------------------------------------------------------");
    }

    for (auto& [metric, namedData] : data)
    {
        std::string key;
        switch (metric)
        {
            case PERF_MON_TRIGGER: key = "T"; break;
            case PERF_MON_VALUE: key = "V"; break;
            case PERF_MON_ACTION: key = "A"; break;
            case PERF_MON_RNDBOT: key = "RndBot"; break;
            case PERF_MON_TOTAL: key = "Total"; break;
            default: key = "?";
        }

        std::list<std::vector<std::string>> stacks;

        for (auto& [stack, performanceData] : namedData)
        {
            if (!perTick && metric == PERF_MON_TOTAL && stack[0].find("PlayerbotAI") == std::string::npos)
                continue;
            stacks.push_back(stack);
        }

        auto& nameD = namedData;

        stacks.sort([&](std::vector<std::string> i, std::vector<std::string> j) { return nameD.at(i).totalTime < nameD.at(j).totalTime; });

        float tPerc = 0, tCount = 0;
        uint32 tMin = 99999, tMax = 0, tTime = 0;

        sLog.outString("percentage   time    |   min  ..    max (     avg  of     count ) - type : name                        ");

        for (auto& stack : stacks)
        {
            PerformanceData& pd = namedData[stack];
            float perc = (float)pd.totalTime / (float)total * 100.0f;
            float secs = (float)pd.totalTime / (perTick ? totalCount : 1000.0f);
            float avg = (float)pd.totalTime / (float)pd.count;
            float amount = (float)pd.count / (perTick ? (float)totalCount : 1);

            std::string disName = StackString(stack);
            if (!fullStack && disName.find("|") != std::string::npos)
                disName = disName.substr(0, disName.find("|")) + disName.substr(disName.find("]"));

            if (perc > 0.1)
            {
                if (perTick)
                    sLog.outString("%7.3f%% %9ums | %6u .. %6u (%9.2f of %10.2f) - %s    : %s", perc, (uint32)secs, pd.minTime, pd.maxTime, avg, amount, key.c_str(), disName.c_str());
                else
                    sLog.outString("%7.3f%% %10.3fs | %6u .. %6u (%9.4f of %10u) - %s    : %s", perc, secs, pd.minTime, pd.maxTime, avg, (uint32)amount, key.c_str(), disName.c_str());

            }

            bool countTot = false;

            if (metric == PERF_MON_VALUE)
            {
                if (disName.find("<") == std::string::npos)
                    countTot = count(disName.begin(), disName.end(), '|') < 1;
                else
                    countTot = disName.find("|") == std::string::npos;
            }
            else
                countTot = disName.find("|") == std::string::npos;

            if (countTot)
            {
                tPerc += perc;
                tMin = pd.minTime < tMin ? pd.minTime : tMin;
                tMax = pd.maxTime > tMax ? pd.maxTime : tMax;
                tCount += amount;
                tTime += pd.totalTime;
            }
        }

        float secs = tTime / (perTick ? totalCount : 1000.0f);
        float avg = tTime / tCount;

        if (metric != PERF_MON_TOTAL)
        {
            if (perTick)
                sLog.outString("%7.3f%% %9ums | %6u .. %6u (%9.4f of %10.2f) - %s    : %s", tPerc, (uint32)secs, tMin, tMax, avg, tCount, key.c_str(), "TOTAL");
            else
                sLog.outString("%7.3f%% %10.3fs | %6u .. %6u (%9.2f of %10u) - %s    : %s", tPerc, secs, tMin, tMax, avg, (uint32)tCount, key.c_str(), "TOTAL");
        }
        sLog.outString(" ");
    }

    uint32 maxMapTime = 0;

    for (auto& [stack, performanceData] : data[PERF_MON_TOTAL])
        if (stack[0].find("PlayerbotAI::UpdateAI ") == 0 && stack[0].find(" I") == stack[0].size() - 2)
            maxMapTime = std::max(performanceData.totalTime, maxMapTime);

    if (total)
    {
        float avgDiff = data[PERF_MON_TOTAL][{"PlayerbotAIBase::FullTick"}].totalTime / data[PERF_MON_TOTAL][{"PlayerbotAIBase::FullTick"}].count;
        float aiPerc = (maxMapTime * 100.0f) / (float)(total);

        sLog.outString("Estimated avg diff: %3.2f with ai load at least: %5.2f%%", avgDiff, aiPerc);

        sLog.outString(" ");
    }
}

void PerformanceMonitor::Reset()
{
    for (auto& [mapId, mapData] : mapsData)
    {
        for (auto& [instanceId, instanceData] : mapData)
        {
            for (auto& [metric, namedData] : instanceData)
            {
                for (auto& [name, performanceData] : namedData)
                {
                    performanceData.minTime = performanceData.maxTime = performanceData.totalTime = performanceData.count = 0;
                }
            }
        }
    }
}

void PerformanceMonitor::Init(uint32 mapId, uint32 instanceId)
{    
    if (sPlayerbotAIConfig.perfMonEnabled)
    {
        mapsData[mapId][instanceId];
    }
}                       

PerformanceMonitorOperation::PerformanceMonitorOperation(PerformanceData& data, std::string name, PerformanceStack* stack) : data(data), name(name), stack(stack)
{
    started = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
}

PerformanceMonitorOperation::~PerformanceMonitorOperation()
{
   finish();
}

void PerformanceMonitorOperation::finish()
{
    if (!sPlayerbotAIConfig.perfMonEnabled)
        return;

    std::chrono::milliseconds finished = (std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now())).time_since_epoch();
    uint32 elapsed = (finished - started).count();

   // std::lock_guard<std::mutex> guard(data.lock);
    if (elapsed > 0)
    {
        if (!data.minTime || data.minTime > elapsed)
            data.minTime = elapsed;
        if (!data.maxTime || data.maxTime < elapsed)
            data.maxTime = elapsed;
        data.totalTime += elapsed;
    }
    data.count++;

    if (stack)
        stack->erase(std::remove(stack->begin(), stack->end(), name), stack->end());
}

bool ChatHandler::HandlePerfMonCommand(char* args)
{
    if (!strcmp(args, "reset"))
    {
        sPerformanceMonitor.Reset();
        sLog.outString("Performance monitor reset");
        return true;
    }

    if (!strcmp(args, "toggle"))
    {
        sPlayerbotAIConfig.perfMonEnabled = !sPlayerbotAIConfig.perfMonEnabled;
        if (sPlayerbotAIConfig.perfMonEnabled)
            sLog.outString("Performance monitor enabled");
        else
            sLog.outString("Performance monitor disabled");

        return true;
    }   

    std::string arguments = args;

    bool tick = false, stack = false, map = false;

    if (arguments.find("tick") != std::string::npos) 
    {
        tick = true;
    }

    if (arguments.find("stack") != std::string::npos)
    {
        stack = true;
    }

    if (arguments.find("map") != std::string::npos)
    {
        map = true;
    }

    sPerformanceMonitor.PrintStats(tick, stack, map);
    return true;
}