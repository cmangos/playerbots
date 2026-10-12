#ifndef _MemoryMonitor_H
#define _MemoryMonitor_H
#define MEMORY_MONITOR

#include "Entities/Creature.h"
#include <string>
#include <unordered_map>
#include <map>
#include <thread>
#include <list>
#include <atomic>
#include <cstdint>

enum ObjectCounterType
{
    OBJECT_COUNTER_CREATURE_GENERIC = 0,
    OBJECT_COUNTER_CREATURE_PET,
    OBJECT_COUNTER_CREATURE_TOTEM,
    OBJECT_COUNTER_CREATURE_TEMP_SUMMON,
    OBJECT_COUNTER_GAMEOBJECT,
    OBJECT_COUNTER_GRIDMAP,
    OBJECT_COUNTER_COUNT
};

inline ObjectCounterType CreatureCounterType(CreatureSubtype subtype)
{
    switch (subtype)
    {
        case CREATURE_SUBTYPE_PET: return OBJECT_COUNTER_CREATURE_PET;
        case CREATURE_SUBTYPE_TOTEM: return OBJECT_COUNTER_CREATURE_TOTEM;
        case CREATURE_SUBTYPE_TEMPORARY_SUMMON: return OBJECT_COUNTER_CREATURE_TEMP_SUMMON;
        default: return OBJECT_COUNTER_CREATURE_GENERIC;
    }
}

class MemoryMonitor
{
    public:
        MemoryMonitor() {};
        virtual ~MemoryMonitor() {};
        static MemoryMonitor& instance()
        {
            static MemoryMonitor instance;
            return instance;
        }

	public:        
        //Objecttype is the name of the class/struct.
        //Object is a pointer to the object or unique identifier.
        //level = 0 only count total number of objects, otherwise log the creation stack of any (object%level) = 0 object.
        void Add(std::string objectType, uint64_t object, int level = 0, std::string stack = ""); //Method to call when object gets created.
        void Rem(std::string objectType, uint64_t object, int level = 0); //Method to call when object gets destroyed.
        void Count(ObjectCounterType type, int delta = 1);
        long long GetCount(ObjectCounterType type) const;
        uint64 GetPrivateUsage() const;
        void Print(); //Print number of objects that still exist and their creation call-stack.
        void Browse();
        void DumpStacks(std::string filename);
        void LogCount(std::string filename);
        void SampleDiagnostics();
	private:
        void BrowseTo(FILE* file);
        std::unordered_map<std::thread::id, std::map<std::string, int>> objectnumbers;
        std::list<std::map<std::string, int >> objectnumbersHist;
        std::unordered_map < std::thread::id, std::unordered_map<std::string, std::unordered_map<uint64_t,std::pair<std::string,time_t>>>> adds, rems;
        std::atomic<long long> objectCounters[OBJECT_COUNTER_COUNT]{};
        bool headers = false;
};


#define sMemoryMonitor MemoryMonitor::instance()

class HeapWatch
{
    public:
        HeapWatch() : m_running(false), m_fullWalk(false), m_walkMs(0) {};
        static HeapWatch& instance()
        {
            static HeapWatch instance;
            return instance;
        }
        void SampleIfNeeded(std::string const& filename, uint32 intervalSec, bool fullWalk);
    private:
        void Sample(std::string const& filename, bool headers, bool fullWalk);
        std::atomic<bool> m_running;
        bool m_fullWalk;
        uint64_t m_walkMs;
};

#define sHeapWatch HeapWatch::instance()


class AllocWatch
{
    public:
        AllocWatch() : m_running(false) {};
        static AllocWatch& instance()
        {
            static AllocWatch instance;
            return instance;
        }
        void StartIfNeeded(std::string const& filename, uint32 intervalSec, uint32 sampleRate);
    private:
        std::atomic<bool> m_running;
};

#define sAllocWatch AllocWatch::instance()

#endif
