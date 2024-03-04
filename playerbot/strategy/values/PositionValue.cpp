
#include "playerbot/playerbot.h"
#include "PositionValue.h"

using namespace ai;

PositionValue::PositionValue(PlayerbotAI* ai, string name)
    : ManualSetValue<ai::PositionMap&>(ai, positions, name)
{
}

string PositionValue::Save()
{
    ostringstream out;
    bool first = true;
    for (ai::PositionMap::iterator i = value.begin(); i != value.end(); ++i)
    {
        string name = i->first;
        ai::PositionEntry pos = i->second;
        if (pos.isSet())
        {
            if (!first) out << "^";
            else first = false;

            out << name << "=" << pos.x << "," << pos.y << "," << pos.z << "," << pos.mapId;
        }
    }
    return out.str();
}

bool PositionValue::Load(string text)
{
    value.clear();

    vector<string> ss = split(text, '^');
    for (vector<string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        vector<string> s1 = split(*i, '=');
        if (s1.size() != 2) continue;
        string name = s1[0];

        vector<string> s2 = split(s1[1], ',');
        if (s2.size() != 4) continue;
        double x = atof(s2[0].c_str());
        double y = atof(s2[1].c_str());
        double z = atof(s2[2].c_str());
        uint32 mapId = atoi(s2[3].c_str());

        value[name].Set(x, y, z, mapId);
    }
    return true;
}

PositionEntry SinglePositionValue::Calculate()
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    return posMap[getQualifier()];
}
void SinglePositionValue::Set(PositionEntry value)
{
    PositionMap& posMap = AI_VALUE(PositionMap&,"position");
    PositionEntry pos = posMap[getQualifier()];
    pos = value;
    posMap[getQualifier()] = pos;
}
void SinglePositionValue::Reset() 
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionEntry pos = posMap[getQualifier()];
    pos.Reset();
    posMap[getQualifier()] = pos;
}

