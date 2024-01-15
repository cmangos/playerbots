#pragma once
#include "Action.h"
#include "Event.h"
#include "../PlayerbotAIAware.h"
#include "../PerformanceMonitor.h"
#include "ObjectMgr.h"
#include "AiObject.h"
#include "GuidPosition.h"

namespace ai
{
    class UntypedValue : public AiNamedObject
    {
    public:
        UntypedValue(PlayerbotAI* ai, string name) : AiNamedObject(ai, name) {}
        virtual void Update() {}  //Nonfunctional see AiObjectContext::Update() to enable.
        virtual void Reset() {}
        virtual string Format() { return "?"; }
        virtual string Save() { return "?"; }
        virtual bool Load(string value) { return false; }
        virtual bool Expired() { return false; }
        virtual bool Expired(uint32 interval) { return false; }
        virtual bool Protected() { return false; }

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "dummy"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return ""; }
        virtual string GetHelpDescription() { return "This is a value."; }
        virtual vector<string> GetUsedValues() { return {}; }
#endif 
    };

    template<class T>
    class Value
    {
    public:
        virtual T Get() = 0;
        virtual T LazyGet() = 0;
        virtual void Reset() {}
        virtual void Set(T value) = 0;
        operator T() { return Get(); }
    };

    template<class T>
    class CalculatedValue : public UntypedValue, public Value<T>
	{
	public:
        CalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) : UntypedValue(ai, name),
            checkInterval(checkInterval)
        {
            lastCheckTime = 0;
        }
        virtual ~CalculatedValue() {}

	public:
        virtual T Get()
        {
            time_t now = time(0);
            if (!lastCheckTime || (checkInterval < 2 && (now - lastCheckTime > 0.1)) || now - lastCheckTime >= checkInterval / 2)
            {
                lastCheckTime = now;

                PerformanceMonitorOperation *pmo = sPerformanceMonitor.start(PERF_MON_VALUE, AiNamedObject::getName(), this->ai);
                value = Calculate();
                if (pmo) pmo->finish();
            }
            return value;
        }
        virtual T LazyGet()
        {
            if (!lastCheckTime)
                return Get();
            return value;
        }
        virtual void Set(T value) { this->value = value; }
        virtual void Update() { }
        virtual void Reset() { lastCheckTime = 0; }
        virtual bool Expired() { return Expired(checkInterval / 2); }
        virtual bool Expired(uint32 interval) { return time(0) - lastCheckTime >= interval; }
    protected:
        virtual T Calculate() = 0;

    protected:
		int checkInterval;
		time_t lastCheckTime;
        T value;
	};

    template <class T> class SingleCalculatedValue : public CalculatedValue<T>
    {
    public:
        SingleCalculatedValue(PlayerbotAI* ai, string name = "value") : CalculatedValue<T>(ai, name) { this->Reset(); }

        virtual T Get()
        {
            time_t now = time(0);
            if (!this->lastCheckTime)
            {
                this->lastCheckTime = now;

                PerformanceMonitorOperation* pmo = sPerformanceMonitor.start(PERF_MON_VALUE, AiNamedObject::getName(), this->ai);
                this->value = this->Calculate();
                if (pmo) pmo->finish();
            }
            return this->value;
        }
    };
    
    template<class T> class MemoryCalculatedValue : public CalculatedValue<T>
    {
    public:
        MemoryCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) : CalculatedValue<T>(ai, name,checkInterval) { lastChangeTime = 0; }
        virtual bool EqualToLast(T value) = 0;
        virtual bool CanCheckChange() { return !lastChangeTime || (time(0) - lastChangeTime > minChangeInterval && !EqualToLast(this->value)); }
        virtual bool UpdateChange() { if (!CanCheckChange()) return false; lastChangeTime = time(0); lastValue = this->value; return true; }

        virtual void Set(T value) { CalculatedValue<T>::Set(value); UpdateChange(); }
        virtual T Get() { this->value = CalculatedValue<T>::Get(); UpdateChange(); return this->value;}

        time_t LastChangeOn() {Get(); return lastChangeTime;}
        uint32 LastChangeDelay() { return time(0) - LastChangeOn(); }
        T GetLastValue() { return lastValue; }
        time_t GetLastTime() { return lastChangeTime; }

        virtual T GetDelta() { T lVal = lastValue; time_t lTime = lastChangeTime; if (lastChangeTime == time(0)) return Get() - Get(); return (Get() - lVal) / float(time(0) - lTime); }

        virtual void Reset() { CalculatedValue<T>::Reset(); lastChangeTime = time(0); }
        virtual bool Protected() override { return true; }
    protected:
        T lastValue;
        uint32 minChangeInterval = 0; //Change will not be checked untill this interval has passed.
        time_t lastChangeTime;
    };

    template<class T> class LogCalculatedValue : public MemoryCalculatedValue<T>
    {
    public:
        LogCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) : MemoryCalculatedValue<T>(ai, name, checkInterval) {};
        virtual bool UpdateChange() { if (MemoryCalculatedValue<T>::UpdateChange()) return false; valueLog.push_back(make_pair(this->value, time(0))); if (valueLog.size() > logLength) valueLog.pop_front(); return true; }

        virtual T Get() { return MemoryCalculatedValue<T>::Get(); }

        list<pair<T, time_t>> ValueLog() { return valueLog; }

        pair<T, time_t> GetLogOn(time_t t) { auto log = std::find_if(valueLog.rbegin(), valueLog.rend(), [t](std::pair<T, time_t> p) {return p.second < t; }); if (log == valueLog.rend()) return valueLog.front(); return *log; }
        T GetValueOn(time_t t) { return GetLogOn(t)->first; }
        T GetTimeOn(time_t t) { return GetTimeOn(t)->second; }

        virtual T GetDelta(uint32 window) { pair<T, time_t> log = GetLogOn(time(0) - window); if (log.second == time(0)) return Get() - Get(); return (Get() - log.first) / float(time(0) - log.second);}

        virtual void Reset() { MemoryCalculatedValue<T>::Reset(); valueLog.clear(); }
    protected:
        list<pair<T, time_t>> valueLog;
        uint8 logLength = 10; //Maxium number of values recorded.
    };    

    class Uint8CalculatedValue : public CalculatedValue<uint8>
    {
    public:
        Uint8CalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<uint8>(ai, name, checkInterval) {}

        virtual string Format()
        {
            ostringstream out; out << (int)this->Calculate();
            return out.str();
        }
    };

    class Uint32CalculatedValue : public CalculatedValue<uint32>
    {
    public:
        Uint32CalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<uint32>(ai, name, checkInterval) {}

        virtual string Format()
        {
            ostringstream out; out << (int)this->Calculate();
            return out.str();
        }
    };

    class FloatCalculatedValue : public CalculatedValue<float>
    {
    public:
        FloatCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<float>(ai, name, checkInterval) {}

        virtual string Format()
        {
            ostringstream out; out << this->Calculate();
            return out.str();
        }
    };

    class BoolCalculatedValue : public CalculatedValue<bool>
    {
    public:
        BoolCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<bool>(ai, name, checkInterval) {}

        virtual string Format()
        {
            return this->Calculate() ? "true" : "false";
        }
    };

    class StringCalculatedValue : public CalculatedValue<string>
    {
    public:
        StringCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<string>(ai, name, checkInterval) {}

        virtual string Format()
        {
            return this->Calculate();
        }
    };

    class UnitCalculatedValue : public CalculatedValue<Unit*>
    {
    public:
        UnitCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<Unit*>(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval / 2; }

        virtual string Format()
        {
            Unit* unit = this->Calculate();
            return unit ? unit->GetName() : "<none>";
        }
    };
    
    class CDPairCalculatedValue : public CalculatedValue<CreatureDataPair const*>
    {
    public:
        CDPairCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<CreatureDataPair const*>(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval / 2; }

        virtual string Format()
        {
            CreatureDataPair const* creatureDataPair = this->Calculate();
            CreatureInfo const* bmTemplate = ObjectMgr::GetCreatureTemplate(creatureDataPair->second.id);
            return creatureDataPair ? bmTemplate->Name : "<none>";
        }
    };

    class CDPairListCalculatedValue : public CalculatedValue<list<CreatureDataPair const*>>
    {
    public:
        CDPairListCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<list<CreatureDataPair const*>>(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval / 2; }

        virtual string Format()
        {
            ostringstream out; out << "{";
            list<CreatureDataPair const*> cdPairs = this->Calculate();
            for (list<CreatureDataPair const*>::iterator i = cdPairs.begin(); i != cdPairs.end(); ++i)
            {
                CreatureDataPair const* cdPair = *i;
                out << cdPair->first << ",";
            }
            out << "}";
            return out.str();
        }
    };

    class ObjectGuidCalculatedValue : public CalculatedValue<ObjectGuid>
    {
    public:
        ObjectGuidCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<ObjectGuid>(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval / 2; }

        virtual string Format();
    };

    class ObjectGuidListCalculatedValue : public CalculatedValue<list<ObjectGuid> >
    {
    public:
        ObjectGuidListCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<list<ObjectGuid> >(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval/2; }

        virtual string Format();
    };

    class GuidPositionCalculatedValue : public CalculatedValue<GuidPosition>
    {
    public:
        GuidPositionCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<GuidPosition>(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval / 2; }

        virtual string Format();
    };

    class GuidPositionListCalculatedValue : public CalculatedValue<list<GuidPosition> >
    {
    public:
        GuidPositionListCalculatedValue(PlayerbotAI* ai, string name = "value", int checkInterval = 1) :
            CalculatedValue<list<GuidPosition> >(ai, name, checkInterval) { this->lastCheckTime = time(0) - checkInterval / 2; }

        virtual string Format();
    };

    template<class T>
    class ManualSetValue : public UntypedValue, public Value<T>
    {
    public:
        ManualSetValue(PlayerbotAI* ai, T defaultValue, string name = "value") :
            UntypedValue(ai, name), value(defaultValue), defaultValue(defaultValue) {}
        virtual ~ManualSetValue() {}

    public:
        virtual T Get() { return value; }
        virtual T LazyGet() { return value; }
        virtual void Set(T value) { this->value = value; }
        virtual void Update() { }
        virtual void Reset() { value = defaultValue; }

    protected:
        T value;
        T defaultValue;
    };

    class UnitManualSetValue : public ManualSetValue<Unit*>
    {
    public:
        UnitManualSetValue(PlayerbotAI* ai, Unit* defaultValue, string name = "value") :
            ManualSetValue<Unit*>(ai, defaultValue, name) {}

        virtual string Format()
        {
            Unit* unit = Get();
            return unit ? unit->GetName() : "<none>";
        }
    };

    class GuidPositionManualSetValue : public ManualSetValue<GuidPosition>
    {
    public:
        GuidPositionManualSetValue(PlayerbotAI* ai, GuidPosition defaultValue, string name = "value") :
            ManualSetValue<GuidPosition>(ai, defaultValue, name) {}

        virtual string Format() override;
    };
}
