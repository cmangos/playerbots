#pragma once

#ifdef GenerateBotTests

#include <string>
#include <list>

class Player;

namespace ai
{
    class TravelTarget;
}

class TravelProbe
{
public:
    static void ReportFlip(Player* bot, ai::TravelTarget* newTarget, ai::TravelTarget* oldTarget);
    static std::list<std::string> HandleTravelSample(Player* bot, const std::string& param);
    static std::list<std::string> HandleTravelTrace(const std::string& param);
};

#endif
