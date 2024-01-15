#include "../../botpch.h"
#include "../playerbot.h"
#include "Value.h"
#include "../PerformanceMonitor.h"
#include "ChatHelper.h"

using namespace ai;

std::string ObjectGuidCalculatedValue::Format()
{
    GuidPosition guid = GuidPosition(this->Calculate(), bot->GetMapId());
    return guid ? chat->formatGuidPosition(guid) : "<none>";
}

std::string ObjectGuidListCalculatedValue::Format()
{
    ostringstream out; out << "{";
    list<ObjectGuid> guids = this->Calculate();
    for (list<ObjectGuid>::iterator i = guids.begin(); i != guids.end(); ++i)
    {
        GuidPosition guid = GuidPosition(*i, bot->GetMapId());
        out << chat->formatGuidPosition(guid) << ",";
    }
    out << "}";
    return out.str();
}

std::string GuidPositionCalculatedValue::Format()
{
    ostringstream out;
    GuidPosition guidP = this->Calculate();
    return chat->formatGuidPosition(guidP);
}

std::string GuidPositionListCalculatedValue::Format()
{
    ostringstream out; out << "{";
    list<GuidPosition> guids = this->Calculate();
    for (list<GuidPosition>::iterator i = guids.begin(); i != guids.end(); ++i)
    {
        GuidPosition guidP = *i;
        out << chat->formatGuidPosition(guidP) << ",";
    }
    out << "}";
    return out.str();
}

std::string GuidPositionManualSetValue::Format()
{
    return chat->formatGuidPosition(value);
}
