#include "MemoryMonitor.h"
#include <iostream>     // std::cout, std::ios
#include <sstream>
#include <time.h>
#include "Util/Timer.h"
#include "Log/Log.h"
#define BOOST_STACKTRACE_LINK
#include <boost/stacktrace.hpp>

#if PLATFORM == PLATFORM_WINDOWS
#include "psapi.h"
#include "windows.h"
#include <dbghelp.h>
#endif
#include "Config/Config.h"
#include <cstdio>
#include <cstring>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>

#include "RandomPlayerbotMgr.h"

void MemoryMonitor::Add(std::string objectType, uint64_t object, int level, std::string stack)
{
    if(stack.empty())
        objectnumbers[std::this_thread::get_id()][objectType]++; 

    if (level && (int)object % level == 0)
    {
        std::ostringstream out;
        if (stack.empty())
            out << boost::stacktrace::stacktrace();
        else
            out << stack;
        adds[std::this_thread::get_id()][objectType][object] = make_pair(out.str(), time(0));
    }
}

void MemoryMonitor::Rem(std::string objectType, uint64_t object, int level)
{
    objectnumbers[std::this_thread::get_id()][objectType]--; 
    
    if (level && (int)object % level == 0)
    {
        if (adds[std::this_thread::get_id()][objectType].find(object) != adds[std::this_thread::get_id()][objectType].end())
            adds[std::this_thread::get_id()][objectType].erase(object);
        else
            rems[std::this_thread::get_id()][objectType][object] = std::make_pair("rem", time(0));
    }
}

void MemoryMonitor::Count(ObjectCounterType type, int delta)
{
    if (type >= 0 && type < OBJECT_COUNTER_COUNT)
        objectCounters[type].fetch_add(delta, std::memory_order_relaxed);
}

long long MemoryMonitor::GetCount(ObjectCounterType type) const
{
    if (type >= 0 && type < OBJECT_COUNTER_COUNT)
        return objectCounters[type].load(std::memory_order_relaxed);

    return 0;
}

uint64 MemoryMonitor::GetPrivateUsage() const
{
#if PLATFORM == PLATFORM_WINDOWS
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    return pmc.PrivateUsage;
#else
    return 0;
#endif
}

void MemoryMonitor::SampleDiagnostics()
{
    std::string logsDir = sConfig.GetStringDefault("LogsDir");

    sHeapWatch.SampleIfNeeded(logsDir + "/" + "heap.csv",
        sConfig.GetIntDefault("Playerbots.HeapWatchIntervalSec", 60),
        sConfig.GetBoolDefault("Playerbots.HeapWatchFullWalk", false));

    if (sConfig.GetBoolDefault("Playerbots.AllocWatch", false))
        sAllocWatch.StartIfNeeded(logsDir + "/" + "alloc_sites.csv",
            sConfig.GetIntDefault("Playerbots.AllocWatchIntervalSec", 300),
            sConfig.GetIntDefault("Playerbots.AllocWatchSampleRate", 256));
}

void MemoryMonitor::Print()
{
    Browse();

    std::map<std::string, int> nums;

    for (auto& t : objectnumbers) // Aggregate object counts over all threads
        for (auto& object : t.second)
            nums[object.first]+=object.second;

    objectnumbersHist.push_back(nums);

    if (objectnumbersHist.size() > 10)
        objectnumbersHist.pop_front();

    #if PLATFORM == PLATFORM_WINDOWS
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    sLog.outString("MEMORY: %.2fgb", pmc.PrivateUsage/1000000000.0f);
    #endif

    for (auto& num : nums)
    {
        if(num.second > 1000000)
            sLog.outString("%s : %.2fm (%.2fk)", num.first.c_str(), num.second / 1000000.0f, (objectnumbersHist.back()[num.first] - objectnumbersHist.front()[num.first]) / 1000.0f);            
        else if(num.second > 1000)
            sLog.outString("%s : %.2fk (%.2fk)", num.first.c_str(), num.second / 1000.0f, (objectnumbersHist.back()[num.first] - objectnumbersHist.front()[num.first]) / 1000.0f);
        else
            sLog.outString("%s : %d (%d)", num.first.c_str(), num.second, (objectnumbersHist.back()[num.first] - objectnumbersHist.front()[num.first]));
    }
}

void MemoryMonitor::LogCount(std::string filename)
{
    std::map<std::string, int> nums;

    for (auto& t : objectnumbers) // Aggregate object counts over all threads
        for (auto& object : t.second)
            nums[object.first] += object.second;

    objectnumbersHist.push_back(nums);

    if (objectnumbersHist.size() > 10)
        objectnumbersHist.pop_front();

    std::ostringstream out;

    if (!headers)
    {
        out << "time,onlinebots";
#if PLATFORM == PLATFORM_WINDOWS
        out << ",totalMemory";
#endif
        out << ",counts\n";
    }

    std::ostringstream counts;
    for (auto& num : nums)
        counts << ";" << num.first << "=" << num.second;

    static char const* const counterNames[OBJECT_COUNTER_COUNT] =
    {
        "Creature_generic",
        "Creature_pet",
        "Creature_totem",
        "Creature_temp_summon",
        "GameObject",
        "GridMap"
    };

    for (int i = 0; i < OBJECT_COUNTER_COUNT; ++i)
        counts << ";" << counterNames[i] << "=" << objectCounters[i].load(std::memory_order_relaxed);

    out << std::to_string(static_cast<uint32>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch() - GetApplicationStartTime().time_since_epoch()).count()));

    out << "," << std::to_string(sRandomPlayerbotMgr.GetPlayerbotsAmount());

#if PLATFORM == PLATFORM_WINDOWS
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
    out << "," << std::to_string(pmc.PrivateUsage);
#endif

    out << ",\"" << counts.str() << "\"\n";

    FILE* file = fopen(filename.c_str(), headers ? "a" : "w");
    if (!file)
        return;

    fprintf(file, "%s", out.str().c_str());
    fflush(file);
    fclose(file);
    fflush(stdout);
    headers = true;
}

void MemoryMonitor::DumpStacks(std::string filename)
{
    FILE* file = fopen(filename.c_str(), "w");
    if (!file)
        return;

    BrowseTo(file);
    fclose(file);
}

void MemoryMonitor::Browse()
{
    BrowseTo(nullptr);
}

void MemoryMonitor::BrowseTo(FILE* file)
{
    std::unordered_map<std::string, std::unordered_map<std::string, double>> here;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<time_t>>> hereTime;
    std::unordered_map<std::string, std::vector<double>> notHere;

    for (auto& thre : adds) //loop over all threads
        for(auto typ : thre.second) //loop over all types
        {
            for (auto& a : typ.second) //loop over all additions
            {
                bool stillHere = true;
                for (auto& rem : rems) //loop over all threads of removed items.
                {
                    for (auto& r : rem.second[typ.first]) //loop over all items of the same type.
                        if (r.first == a.first) 
                        {
                            stillHere = false;
                            break;
                        }

                    if (!stillHere)
                        break;
                }

                if (stillHere) //Item was not removed somewhere, report it.
                {
                    here[typ.first][a.second.first]++;
                    hereTime[typ.first][a.second.first].push_back(a.second.second);
                }
                else //Item was removed. Clean it up.
                    notHere[typ.first].push_back(a.first);
            }           
        }

    //Clean up removed items.
    for (auto& thre : adds) 
    {
        for (auto& typ : thre.second)
        {
            for (auto& nh : notHere[typ.first])
            {
                adds[thre.first][typ.first].erase(nh);
                rems[thre.first][typ.first].erase(nh);
            }
        }
    }

    std::ostringstream browseOut;

    for (auto& t : here)
    {
        browseOut << "[" << t.first << "] = " << t.second.size() << "\n";

        std::vector<std::pair<std::string, int>> top(10);
        std::partial_sort_copy(here[t.first].begin(),
            here[t.first].end(),
            top.begin(),
            top.end(),
            [](std::pair<const std::string, int> const& l,
                std::pair<const std::string, int> const& r)
            {
                return l.second > r.second;
            });

        for (auto& p : top)
        {
            if (hereTime[t.first][p.first].empty())
                continue;

            uint32 f, l, a = 0;

            f = time(0) - hereTime[t.first][p.first].front();
            l = time(0) - hereTime[t.first][p.first].back();

            for (auto p : hereTime[t.first][p.first])
                a += (time(0) - p);

            if (hereTime[t.first][p.first].size())
                a /= hereTime[t.first][p.first].size();

            browseOut << "<" << p.second << "> \nfrom: " << f << "\navg: " << a << "\nto: " << l << "\n" << p.first << "\n";
        }
    }

    if (file)
        fprintf(file, "%s", browseOut.str().c_str());
    else
        sLog.outError("%s", browseOut.str().c_str());
}

#include <cstdio>
#include <ctime>
#include <chrono>
#include <thread>

#if PLATFORM == PLATFORM_WINDOWS

#include "windows.h"
#include "psapi.h"

// Define heap-walk plumbing locally so we don't depend on SDK header variants.
#ifndef PROCESS_HEAP_ENTRY_BUSY
#define PROCESS_HEAP_ENTRY_BUSY 0x0004
#endif
#ifndef PROCESS_HEAP_ENTRY_MOVEABLE
#define PROCESS_HEAP_ENTRY_MOVEABLE 0x0010
#endif
#ifndef PROCESS_HEAP_ENTRY_REGION
#define PROCESS_HEAP_ENTRY_REGION 0x0002
#endif

typedef BOOL (NTAPI *HeapEnumRoutine_t)(PVOID HeapHandle, PVOID UserParameter);
typedef ULONG (WINAPI *EnumProcessHeapsFn_t)(HeapEnumRoutine_t, PVOID);

struct HeapSum
{
    uint64_t committed = 0; // bytes committed in region heaps
    uint64_t busy = 0;      // bytes in live allocations
    uint64_t freeBytes = 0; // bytes in free blocks
    uint64_t busyCount = 0;
    uint64_t freeCount = 0;
    uint32_t heaps = 0;
    // live-block size buckets: <=32,<=64,<=128,<=256,<=512,<=1024,<=4096,>4096
    uint64_t busyCntB[8] = {};
    uint64_t busyBytesB[8] = {};
    uint64_t freeCntB[8] = {};
};

static int SizeBucket(uint32_t size)
{
    if (size <= 32) return 0;
    if (size <= 64) return 1;
    if (size <= 128) return 2;
    if (size <= 256) return 3;
    if (size <= 512) return 4;
    if (size <= 1024) return 5;
    if (size <= 4096) return 6;
    return 7;
}

static BOOL NTAPI HeapSumCallback(PVOID heap, PVOID user)
{
    HeapSum* sum = (HeapSum*)user;
    PROCESS_HEAP_ENTRY entry;
    ZeroMemory(&entry, sizeof(entry));

    if (!HeapLock(heap))
        return TRUE; // skip heap on lock failure rather than abort the whole walk

    entry.lpData = nullptr;
    while (HeapWalk(heap, &entry))
    {
        if (entry.wFlags & PROCESS_HEAP_ENTRY_BUSY)
        {
            int b = SizeBucket(entry.cbData);
            ++sum->busyCount;
            sum->busy += entry.cbData;
            ++sum->busyCntB[b];
            sum->busyBytesB[b] += entry.cbData;
        }
        else if (entry.wFlags & PROCESS_HEAP_ENTRY_REGION)
        {
            sum->committed += entry.Region.dwCommittedSize;
        }
        else // free block (no busy/region/moveable flags)
        {
            int b = SizeBucket(entry.cbData);
            ++sum->freeCount;
            sum->freeBytes += entry.cbData;
            ++sum->freeCntB[b];
        }
    }

    HeapUnlock(heap);
    ++sum->heaps;
    return TRUE;
}

void HeapWatch::SampleIfNeeded(std::string const& filename, uint32 intervalSec, bool fullWalk)
{
    m_fullWalk = fullWalk;

    bool expected = false;
    if (!m_running.compare_exchange_strong(expected, true))
        return; // thread already running

    std::thread t([this, filename, intervalSec]()
    {
        bool headers = false;
        while (true)
        {
            Sample(filename, headers, m_fullWalk);
            headers = true;
            std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
        }
    });
    t.detach();
}

void HeapWatch::Sample(std::string const& filename, bool headers, bool fullWalk)
{
    EnumProcessHeapsFn_t enumHeaps = (EnumProcessHeapsFn_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "RtlEnumProcessHeaps");

    HeapSum sum;
    auto walkStart = std::chrono::steady_clock::now();
    if (fullWalk && enumHeaps)
        enumHeaps((HeapEnumRoutine_t)&HeapSumCallback, &sum);
    m_walkMs = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - walkStart).count();

    // PrivateUsage covers committed memory outside heap regions too.
    PROCESS_MEMORY_COUNTERS_EX pmc;
    memset(&pmc, 0, sizeof(pmc));
    GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));

    // Region census: aggregate private committed memory by size class and
    // remember the largest regions (VirtualAlloc'd data: mmap/vmap caches,
    // thread arenas, DB buffers, etc.) - this catches growth the heap walk
    // cannot attribute.
    uint64_t privateTotal = 0, imageTotal = 0, mappedTotal = 0;
    uint32_t privateRegions = 0, mappedRegions = 0;
    std::vector<std::pair<uint64_t, uint64_t>> top; // (size, baseAddr) largest private regions

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    uintptr_t addr = (uintptr_t)si.lpMinimumApplicationAddress;
    uintptr_t maxAddr = (uintptr_t)si.lpMaximumApplicationAddress;
    MEMORY_BASIC_INFORMATION mbi;
    while (addr < maxAddr)
    {
        if (!VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)))
            break;
        if (mbi.State == MEM_COMMIT)
        {
            uint64_t size = (uint64_t)mbi.RegionSize;
            if (mbi.Type == MEM_PRIVATE)
            {
                privateTotal += size;
                ++privateRegions;
                top.emplace_back(size, (uint64_t)mbi.BaseAddress);
                if (top.size() > 64)
                {
                    std::partial_sort(top.begin(), top.begin() + 20, top.end(),
                        [](std::pair<uint64_t, uint64_t> const& l, std::pair<uint64_t, uint64_t> const& r)
                        { return l.first > r.first; });
                    top.resize(20);
                }
            }
            else if (mbi.Type == MEM_MAPPED)
            {
                mappedTotal += size;
                ++mappedRegions;
            }
            else
                imageTotal += size;
        }
        addr += (uintptr_t)mbi.RegionSize;
    }

    FILE* file = fopen(filename.c_str(), headers ? "a" : "w");
    if (!file)
        return;

    if (!headers)
        fprintf(file, "time,heaps,committed,busy,busyCount,free,freeCount,privateUsage,busyCnt32,busyCnt64,busyCnt128,busyCnt256,busyCnt512,busyCnt1k,busyCnt4k,busyCntBig,busyBy32,busyBy64,busyBy128,busyBy256,busyBy512,busyBy1k,busyBy4k,busyByBig,freeCnt32,freeCnt64,freeCnt128,freeCnt256,freeCnt512,freeCnt1k,privBytes,privRegions,mappedBytes,mappedRegions,imageBytes,mode,walkMs\n");

    time_t now = time(nullptr);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(file, "%s,%u,%llu,%llu,%llu,%llu,%llu,%zu",
        buf, sum.heaps, (unsigned long long)sum.committed, (unsigned long long)sum.busy,
        (unsigned long long)sum.busyCount, (unsigned long long)sum.freeBytes,
        (unsigned long long)sum.freeCount, (size_t)pmc.PrivateUsage);
    for (int i = 0; i < 8; ++i)
        fprintf(file, ",%llu", (unsigned long long)sum.busyCntB[i]);
    for (int i = 0; i < 8; ++i)
        fprintf(file, ",%llu", (unsigned long long)sum.busyBytesB[i]);
    for (int i = 0; i < 6; ++i)
        fprintf(file, ",%llu", (unsigned long long)sum.freeCntB[i]);
    fprintf(file, ",%llu,%u,%llu,%u,%llu",
        (unsigned long long)privateTotal, privateRegions, (unsigned long long)mappedTotal, mappedRegions, (unsigned long long)imageTotal);
    fprintf(file, ",%u,%llu", fullWalk ? 1 : 2, (unsigned long long)m_walkMs);
    fprintf(file, "\n");
    fclose(file);
}

#else // non-Windows: stub

void HeapWatch::SampleIfNeeded(std::string const&, uint32, bool) {}
void HeapWatch::Sample(std::string const&, bool, bool) {}

#endif

#include "Log/Log.h"

#if PLATFORM == PLATFORM_WINDOWS

#include "windows.h"
#include "psapi.h"
#include <dbghelp.h>

#include <cstdio>
#include <cstring>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

typedef PVOID (NTAPI *RtlAllocateHeap_t)(PVOID, ULONG, SIZE_T);
typedef BOOLEAN (NTAPI *RtlFreeHeap_t)(PVOID, ULONG, PVOID);
typedef PVOID (NTAPI *RtlReAllocateHeap_t)(PVOID, ULONG, PVOID, SIZE_T);

typedef BOOL (WINAPI *SymInitialize_t)(HANDLE, PCSTR, BOOL);
typedef BOOL (WINAPI *SymFromAddr_t)(HANDLE, DWORD64, PDWORD64, PSYMBOL_INFO);
typedef DWORD (WINAPI *SymSetOptions_t)(DWORD);

static RtlAllocateHeap_t g_realAlloc = nullptr;
static RtlFreeHeap_t g_realFree = nullptr;
static RtlReAllocateHeap_t g_realReAlloc = nullptr;

static const uint32_t SITE_TABLE_SIZE = 1 << 14;
static const uint32_t SITE_MASK = SITE_TABLE_SIZE - 1;
static const uint32_t BLOCK_TABLE_SIZE = 1 << 20;
static const uint32_t BLOCK_MASK = BLOCK_TABLE_SIZE - 1;
static const uint32_t MAX_FRAMES = 10;

struct AllocSite
{
    uint64_t hash;
    void* frames[MAX_FRAMES];
    uint16_t frameCount;
    int64_t live;
    int64_t lastLive;
    uint64_t allocs;
};

struct AllocBlock
{
    PVOID addr;
    uint32_t site;
    uint32_t size;
};

static AllocSite* g_sites = nullptr;
static AllocBlock* g_blocks = nullptr;

struct SpinLock
{
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    void lock() { while (flag.test_and_set(std::memory_order_acquire)) {} }
    void unlock() { flag.clear(std::memory_order_release); }
};

static SpinLock g_siteLock;
static SpinLock g_blockLock;

static std::atomic<bool> g_installed(false);
static std::atomic<uint32_t> g_sampleRate(256);
static std::atomic<uint32_t> g_counter(0);
static std::atomic<uint64_t> g_totalAllocs(0);
static std::atomic<uint64_t> g_totalFrees(0);
static std::atomic<uint64_t> g_sampled(0);
static std::atomic<uint64_t> g_dropped(0);
static std::atomic<uint64_t> g_liveBlocks(0);
static std::atomic<int64_t> g_liveBytes(0);

static thread_local int g_depth = 0;
static thread_local bool g_ignore = false;

static inline uint64_t HashFrames(void* const* frames, size_t count)
{
    uint64_t h = 1469598103934665603ull;

    for (size_t i = 0; i < count; ++i)
    {
        uint64_t value = (uint64_t)frames[i];

        for (int b = 0; b < 8; ++b)
        {
            h ^= (value >> (b * 8)) & 0xFF;
            h *= 1099511628211ull;
        }
    }

    return h;
}

static uint32_t FindOrAddSite(void* const* frames, size_t count, uint64_t hash)
{
    uint32_t index = (uint32_t)(hash % SITE_TABLE_SIZE);

    g_siteLock.lock();

    for (uint32_t probe = 0; probe < SITE_TABLE_SIZE; ++probe)
    {
        AllocSite& site = g_sites[index];

        if (site.frameCount == 0)
        {
            site.hash = hash;

            for (size_t i = 0; i < count && i < MAX_FRAMES; ++i)
                site.frames[i] = frames[i];

            site.frameCount = (uint16_t)count;
            g_siteLock.unlock();
            return index;
        }

        if (site.hash == hash)
        {
            g_siteLock.unlock();
            return index;
        }

        index = (index + 1) % SITE_TABLE_SIZE;
    }

    g_siteLock.unlock();
    return UINT32_MAX;
}

static void TrackBlock(PVOID addr, size_t size, void* const* frames, size_t count)
{
    if (!addr)
        return;

    uint64_t hash = HashFrames(frames, count);
    uint32_t siteIndex = FindOrAddSite(frames, count, hash);

    if (siteIndex == UINT32_MAX)
        return;

    uint32_t index = (uint32_t)(((uint64_t)addr >> 4) % BLOCK_TABLE_SIZE);

    g_blockLock.lock();

    for (uint32_t probe = 0; probe < BLOCK_TABLE_SIZE; ++probe)
    {
        AllocBlock& block = g_blocks[index];

        if (!block.addr)
        {
            block.addr = addr;
            block.site = siteIndex;
            block.size = (uint32_t)size;
            g_blockLock.unlock();

            g_sites[siteIndex].live += (int64_t)size;
            g_sites[siteIndex].allocs++;
            g_liveBlocks++;
            g_liveBytes += (int64_t)size;
            return;
        }

        if (block.addr == addr)
        {
            g_blockLock.unlock();
            return;
        }

        index = (index + 1) % BLOCK_TABLE_SIZE;
    }

    g_blockLock.unlock();
    g_dropped++;
}

static void UntrackBlock(PVOID addr)
{
    if (!addr)
        return;

    uint32_t index = (uint32_t)(((uint64_t)addr >> 4) % BLOCK_TABLE_SIZE);

    g_blockLock.lock();

    for (uint32_t probe = 0; probe < BLOCK_TABLE_SIZE; ++probe)
    {
        AllocBlock& block = g_blocks[index];

        if (!block.addr)
            break;

        if (block.addr == addr)
        {
            uint32_t siteIndex = block.site;
            uint32_t size = block.size;
            block.addr = nullptr;
            g_blockLock.unlock();

            g_sites[siteIndex].live -= (int64_t)size;
            g_liveBlocks--;
            g_liveBytes -= (int64_t)size;
            return;
        }

        index = (index + 1) % BLOCK_TABLE_SIZE;
    }

    g_blockLock.unlock();
}

static inline bool ShouldSample(size_t size)
{
    if (size >= 4096)
        return true;

    uint32_t rate = g_sampleRate.load(std::memory_order_relaxed);

    if (!rate)
        return true;

    return (g_counter.fetch_add(1, std::memory_order_relaxed) % rate) == 0;
}

static void SampleStack(PVOID addr, size_t size)
{
    void* frames[MAX_FRAMES + 2];
    USHORT captured = CaptureStackBackTrace(2, MAX_FRAMES, frames, nullptr);

    if (!captured)
        return;

    g_sampled++;
    TrackBlock(addr, size, frames, captured);
}

static PVOID NTAPI HookAlloc(PVOID heap, ULONG flags, SIZE_T size)
{
    if (g_ignore)
        return g_realAlloc(heap, flags, size);

    PVOID result = g_realAlloc(heap, flags, size);
    g_totalAllocs++;

    if (result && g_depth == 0 && ShouldSample(size))
    {
        g_depth = 1;
        SampleStack(result, size);
        g_depth = 0;
    }

    return result;
}

static BOOLEAN NTAPI HookFree(PVOID heap, ULONG flags, PVOID addr)
{
    if (g_ignore)
        return g_realFree(heap, flags, addr);

    g_totalFrees++;

    if (addr && g_depth == 0)
    {
        g_depth = 1;
        UntrackBlock(addr);
        g_depth = 0;
    }

    return g_realFree(heap, flags, addr);
}

static PVOID NTAPI HookReAlloc(PVOID heap, ULONG flags, PVOID addr, SIZE_T size)
{
    if (g_ignore)
        return g_realReAlloc(heap, flags, addr, size);

    PVOID result = g_realReAlloc(heap, flags, addr, size);
    g_totalAllocs++;

    if (g_depth == 0)
    {
        g_depth = 1;

        if (addr)
            UntrackBlock(addr);

        if (result && ShouldSample(size))
            SampleStack(result, size);

        g_depth = 0;
    }

    return result;
}

static void PatchModuleImports(HMODULE module)
{
    uint8_t* base = (uint8_t*)module;
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;

    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return;

    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

    if (nt->Signature != IMAGE_NT_SIGNATURE)
        return;

    IMAGE_DATA_DIRECTORY directory = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];

    if (!directory.VirtualAddress || !directory.Size)
        return;

    IMAGE_IMPORT_DESCRIPTOR* descriptor = (IMAGE_IMPORT_DESCRIPTOR*)(base + directory.VirtualAddress);

    for (; descriptor->Name; ++descriptor)
    {
        if (!descriptor->OriginalFirstThunk || !descriptor->FirstThunk)
            continue;

        IMAGE_THUNK_DATA* original = (IMAGE_THUNK_DATA*)(base + descriptor->OriginalFirstThunk);
        IMAGE_THUNK_DATA* iat = (IMAGE_THUNK_DATA*)(base + descriptor->FirstThunk);

        for (; original->u1.AddressOfData; ++original, ++iat)
        {
            if (original->u1.Ordinal & IMAGE_ORDINAL_FLAG)
                continue;

            IMAGE_IMPORT_BY_NAME* name = (IMAGE_IMPORT_BY_NAME*)(base + original->u1.AddressOfData);
            const char* function = (const char*)name->Name;
            void* replacement = nullptr;

            if (!strcmp(function, "RtlAllocateHeap") || !strcmp(function, "HeapAlloc"))
                replacement = (void*)&HookAlloc;
            else if (!strcmp(function, "RtlFreeHeap") || !strcmp(function, "HeapFree"))
                replacement = (void*)&HookFree;
            else if (!strcmp(function, "RtlReAllocateHeap") || !strcmp(function, "HeapReAlloc"))
                replacement = (void*)&HookReAlloc;

            if (!replacement || iat->u1.Function == (ULONG_PTR)replacement)
                continue;

            DWORD oldProtect;

            if (VirtualProtect(&iat->u1.Function, sizeof(void*), PAGE_READWRITE, &oldProtect))
            {
                iat->u1.Function = (ULONG_PTR)replacement;
                VirtualProtect(&iat->u1.Function, sizeof(void*), oldProtect, &oldProtect);
            }
        }
    }
}

static void Install()
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");

    if (!ntdll)
        return;

    g_realAlloc = (RtlAllocateHeap_t)GetProcAddress(ntdll, "RtlAllocateHeap");
    g_realFree = (RtlFreeHeap_t)GetProcAddress(ntdll, "RtlFreeHeap");
    g_realReAlloc = (RtlReAllocateHeap_t)GetProcAddress(ntdll, "RtlReAllocateHeap");

    if (!g_realAlloc || !g_realFree || !g_realReAlloc)
        return;

    HANDLE heap = GetProcessHeap();
    g_sites = (AllocSite*)HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(AllocSite) * SITE_TABLE_SIZE);
    g_blocks = (AllocBlock*)HeapAlloc(heap, HEAP_ZERO_MEMORY, sizeof(AllocBlock) * BLOCK_TABLE_SIZE);

    if (!g_sites || !g_blocks)
        return;

    HMODULE modules[1024];
    DWORD needed = 0;

    if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed))
        for (DWORD i = 0; i < needed / sizeof(HMODULE); ++i)
            PatchModuleImports(modules[i]);

    sLog.outString("AllocWatch: heap imports patched in %u modules (sample rate 1/%u, >=4KB always)",
        (uint32)(needed / sizeof(HMODULE)), g_sampleRate.load());
}

struct Symbolizer
{
    HMODULE dll;
    SymInitialize_t initialize;
    SymFromAddr_t fromAddr;
    SymSetOptions_t setOptions;
    bool ready;
    std::vector<char> buffer;

    Symbolizer() : dll(nullptr), initialize(nullptr), fromAddr(nullptr), setOptions(nullptr), ready(false)
    {
        dll = LoadLibraryA("dbghelp.dll");

        if (!dll)
            return;

        initialize = (SymInitialize_t)GetProcAddress(dll, "SymInitialize");
        fromAddr = (SymFromAddr_t)GetProcAddress(dll, "SymFromAddr");
        setOptions = (SymSetOptions_t)GetProcAddress(dll, "SymSetOptions");

        if (!initialize || !fromAddr)
            return;

        setOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
        ready = initialize(GetCurrentProcess(), nullptr, TRUE) != FALSE;
    }

    std::string Resolve(void* address)
    {
        if (!ready || !address)
            return "?";

        buffer.assign(sizeof(SYMBOL_INFO) + 512, 0);
        SYMBOL_INFO* symbol = (SYMBOL_INFO*)buffer.data();
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = 500;

        DWORD64 displacement = 0;
        char text[640];

        if (fromAddr(GetCurrentProcess(), (DWORD64)address, &displacement, symbol))
            sprintf_s(text, "%s+0x%llx", symbol->Name, (unsigned long long)displacement);
        else
            sprintf_s(text, "%p", address);

        return std::string(text);
    }
};

static void Dump(std::string const& filename, bool headers, Symbolizer& symbolizer)
{
    struct Entry
    {
        uint32_t index;
        int64_t live;
        int64_t delta;
        uint64_t allocs;
    };

    std::vector<Entry> entries;
    entries.reserve(SITE_TABLE_SIZE);

    g_siteLock.lock();

    for (uint32_t i = 0; i < SITE_TABLE_SIZE; ++i)
    {
        AllocSite& site = g_sites[i];

        if (!site.frameCount)
            continue;

        Entry entry;
        entry.index = i;
        entry.live = site.live;
        entry.delta = site.live - site.lastLive;
        entry.allocs = site.allocs;
        site.lastLive = site.live;
        entries.push_back(entry);
    }

    g_siteLock.unlock();

    std::sort(entries.begin(), entries.end(), [](Entry const& left, Entry const& right)
    {
        if (left.live != right.live)
            return left.live > right.live;

        return left.delta > right.delta;
    });

    FILE* file = fopen(filename.c_str(), headers ? "a" : "w");

    if (!file)
        return;

    if (!headers)
        fprintf(file, "timestamp,rank,liveMB,deltaMB,sampledAllocs,symbols\n");

    time_t now = time(nullptr);
    char stamp[32];
    std::strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    uint32_t shown = 0;

    for (Entry const& entry : entries)
    {
        if (shown >= 40)
            break;

        AllocSite& site = g_sites[entry.index];
        std::string symbols;

        for (uint16_t f = 0; f < site.frameCount; ++f)
        {
            if (f)
                symbols += " | ";

            symbols += symbolizer.Resolve(site.frames[f]);
        }

        fprintf(file, "%s,%u,%.2f,%.2f,%llu,\"%s\"\n", stamp, shown + 1,
            entry.live / 1048576.0, entry.delta / 1048576.0,
            (unsigned long long)entry.allocs, symbols.c_str());
        ++shown;
    }

    fprintf(file, "%s,summary,%.2f,%.2f,%llu,\"tracked=%llu dropped=%llu allocs=%llu frees=%llu rate=1/%u\"\n",
        stamp, g_liveBytes.load() / 1048576.0, 0.0, (unsigned long long)g_sampled.load(),
        (unsigned long long)g_liveBlocks.load(), (unsigned long long)g_dropped.load(),
        (unsigned long long)g_totalAllocs.load(), (unsigned long long)g_totalFrees.load(),
        g_sampleRate.load());

    fclose(file);

    sLog.outString("AllocWatch: tracked %.1f MB in %llu live blocks, sampled %llu, dropped %llu (top sites in csv)",
        g_liveBytes.load() / 1048576.0, (unsigned long long)g_liveBlocks.load(),
        (unsigned long long)g_sampled.load(), (unsigned long long)g_dropped.load());
}

void AllocWatch::StartIfNeeded(std::string const& filename, uint32 intervalSec, uint32 sampleRate)
{
    bool expected = false;

    if (!m_running.compare_exchange_strong(expected, true))
        return;

    g_sampleRate = sampleRate ? sampleRate : 1;
    Install();

    std::thread t([this, filename, intervalSec]()
    {
        g_ignore = true;
        Symbolizer symbolizer;
        bool headers = false;

        while (true)
        {
            Dump(filename, headers, symbolizer);
            headers = true;
            std::this_thread::sleep_for(std::chrono::seconds(intervalSec));
        }
    });
    t.detach();
}

#else

void AllocWatch::StartIfNeeded(std::string const&, uint32, uint32) {}

#endif
