#include "TravelProbe.h"
#ifdef GenerateBotTests

#include "playerbot/playerbot.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/strategy/values/TravelValues.h"
#include "playerbot/strategy/actions/ChooseTravelTargetAction.h"
#include "Config/Config.h"

#include <algorithm>
#include <atomic>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace ai;

static bool IsNullDestination(TravelDestination* destination)
{
    return destination && typeid(*destination) == typeid(NullTravelDestination);
}

static std::atomic<bool> TravelProbeTraceEnabled{ false };

static std::string TravelProbePurposeName(const std::string& purpose)
{
    if (Qualified::isValidNumberString(purpose) && TravelDestinationPurposeName.find(TravelDestinationPurpose(stoi(purpose))) != TravelDestinationPurposeName.end())
        return TravelDestinationPurposeName.at(TravelDestinationPurpose(stoi(purpose)));

    return "unknown";
}

static std::string TravelProbeBucket(float distance)
{
    uint32 prev = 0;
    for (auto& partition : travelPartitions)
    {
        if (distance <= (float)partition)
            return std::to_string(prev) + "-" + std::to_string(partition);

        prev = partition;
    }

    return "> " + std::to_string(prev);
}

static std::string TravelProbeSanitize(std::string text)
{
    std::replace(text.begin(), text.end(), ',', ';');
    return text;
}

void TravelProbe::ReportFlip(Player* bot, TravelTarget* newTarget, TravelTarget* oldTarget)
{
    if (!TravelProbeTraceEnabled)
        return;

    PlayerbotAI* ai = bot->GetPlayerbotAI();
    if (!ai || !sPlayerbotAIConfig.hasLog("travel_flip.csv"))
        return;

    TravelDestination* destination = newTarget->GetDestination();
    TravelDestination* oldDestination = oldTarget ? oldTarget->GetDestination() : nullptr;

    bool newIsNull = IsNullDestination(destination);
    bool oldIsNull = IsNullDestination(oldDestination);

    if ((!destination || newIsNull) && (!oldDestination || oldIsNull))
        return;

    AiObjectContext* context = ai->GetAiObjectContext();
    std::string purposeName = TravelProbePurposeName(context->GetValue<std::string>("manual string", "future travel purpose")->Get());

    std::string reason;
    if (newTarget->IsForced())
        reason = "forced";
    else if (!oldDestination || oldIsNull)
        reason = (!destination || newIsNull) ? "init" : "from-nowhere";
    else if (!destination || newIsNull)
        reason = "to-nowhere";
    else if (oldDestination == destination)
        reason = "continue";
    else
        reason = "change";

    float distance = 0.0f;
    if (destination && !newIsNull && newTarget->GetPosition())
        distance = newTarget->Distance(bot);

    std::ostringstream out;
    out << sPlayerbotAIConfig.GetTimestampStr() << ",";
    out << bot->GetName() << ",";
    out << bot->GetLevel() << ",";
    out << purposeName << ",";
    out << (newTarget->IsForced() ? "1" : "0") << ",";
    out << (oldDestination && !oldIsNull ? TravelProbeSanitize(oldDestination->GetTitle()) : "none") << ",";
    out << (destination && !newIsNull ? TravelProbeSanitize(destination->GetTitle()) : "none") << ",";
    out << std::fixed << std::setprecision(0) << distance << ",";
    out << (distance > 0.0f ? TravelProbeBucket(distance) : "none") << ",";
    out << reason;

    sPlayerbotAIConfig.log("travel_flip.csv", out.str().c_str());
}

static std::vector<uint32> TravelProbeParseLevels(const std::string& spec)
{
    std::vector<uint32> levels;
    std::vector<std::string> parts = Qualified::getMultiQualifiers(spec, ",");
    for (auto& part : parts)
    {
        if (!Qualified::isValidNumberString(part))
            continue;

        uint32 level = (uint32)stoul(part);
        if (level > 0 && level <= 255)
            levels.push_back(level);
    }

    return levels;
}

static std::vector<TravelDestinationPurpose> TravelProbeParsePurposes(const std::string& spec)
{
    std::vector<TravelDestinationPurpose> purposes;

    std::string lowered = spec;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

    if (lowered == "all")
    {
        for (auto& [purpose, name] : TravelDestinationPurposeName)
        {
            if (purpose != TravelDestinationPurpose::None && purpose != TravelDestinationPurpose::MaxFlag)
                purposes.push_back(purpose);
        }

        return purposes;
    }

    std::vector<std::string> parts = Qualified::getMultiQualifiers(spec, ",");
    for (auto& part : parts)
    {
        std::string loweredPart = part;
        std::transform(loweredPart.begin(), loweredPart.end(), loweredPart.begin(), ::tolower);

        bool found = false;
        for (auto& [purpose, name] : TravelDestinationPurposeName)
        {
            std::string loweredName = name;
            std::transform(loweredName.begin(), loweredName.end(), loweredName.begin(), ::tolower);
            if (loweredPart == loweredName)
            {
                purposes.push_back(purpose);
                found = true;
                break;
            }
        }

        if (!found && Qualified::isValidNumberString(part) && part.size() <= 9)
        {
            int value = stoi(part);
            if (TravelDestinationPurposeName.find(TravelDestinationPurpose(value)) != TravelDestinationPurposeName.end())
                purposes.push_back(TravelDestinationPurpose(value));
        }
    }

    return purposes;
}

std::list<std::string> TravelProbe::HandleTravelSample(Player* bot, const std::string& param)
{
    std::list<std::string> messages;

    if (!bot || !bot->GetPlayerbotAI())
    {
        messages.push_back("usage: travelsample <bot> <levels> <purposes> [runs]");
        return messages;
    }

    PlayerbotAI* ai = bot->GetPlayerbotAI();

    std::vector<std::string> parts = Qualified::getMultiQualifiers(param, " ");
    if (parts.size() < 2)
    {
        messages.push_back("usage: travelsample <bot> <levels> <purposes> [runs]");
        return messages;
    }

    std::vector<uint32> levels = TravelProbeParseLevels(parts[0]);
    std::vector<TravelDestinationPurpose> purposes = TravelProbeParsePurposes(parts[1]);

    uint32 runs = 50;
    if (parts.size() > 2 && Qualified::isValidNumberString(parts[2]))
        runs = (uint32)stoul(parts[2]);

    if (levels.empty() || purposes.empty() || levels.size() * purposes.size() > 200)
    {
        messages.push_back("usage: travelsample <bot> <levels> <purposes> [runs] (levels: 6,20,40 purposes: Grind,GenericRpg,QuestGiver or all)");
        return messages;
    }

    uint32 combos = (uint32)(levels.size() * purposes.size());
    if (runs * combos > 2000)
    {
        runs = 2000 / combos;
        messages.push_back("travelsample: runs capped at " + std::to_string(runs) + " (level x purpose x runs total)");
    }

    WorldPosition center(bot);
    PlayerTravelInfo baseInfo(bot);

    std::ostringstream header;
    header << "travelsample: bot " << bot->GetName() << " (real level " << baseInfo.GetLevel() << ") center " << center.getAreaName() << " runs " << runs;
    messages.push_back(header.str());

    for (auto level : levels)
    {
        PlayerTravelInfo info = baseInfo;
        info.SetProbeLevel(level);

        for (auto purpose : purposes)
        {
            float maxDistance = 10000.0f;
            if (purpose == TravelDestinationPurpose::QuestGiver || purpose == TravelDestinationPurpose::QuestTaker)
                maxDistance = 1000.0f + (float)(level * level) * 75.0f;

            uint32 noPick = 0;
            std::map<std::string, uint32> bucketCounts;
            std::vector<float> distances;
            std::map<std::string, uint32> destCounts;

            for (uint32 run = 0; run < runs; ++run)
            {
                PartitionedTravelList list = sTravelMgr.GetPartitions(center, travelPartitions, info, (uint32)purpose, {}, true, maxDistance);
                TravelPoint picked = ChooseTravelTargetAction::PickBestTarget(bot, nullptr, ai, false, info, list);

                if (!std::get<0>(picked))
                {
                    ++noPick;
                    continue;
                }

                float distance = std::get<2>(picked);
                distances.push_back(distance);
                ++bucketCounts[TravelProbeBucket(distance)];
                ++destCounts[TravelProbeSanitize(std::get<0>(picked)->GetTitle())];
            }

            std::ostringstream combo;
            combo << "level " << level << " purpose " << TravelDestinationPurposeName.at(purpose)
                  << " (maxRange " << (uint32)maxDistance << "y) picks " << distances.size() << "/" << runs << " nopick " << noPick;
            messages.push_back(combo.str());

            if (distances.empty())
                continue;

            std::ostringstream buckets;
            buckets << "  buckets:";
            for (auto& [bucket, count] : bucketCounts)
                buckets << " " << bucket << "y:" << count;
            messages.push_back(buckets.str());

            std::vector<float> sorted = distances;
            std::sort(sorted.begin(), sorted.end());
            std::ostringstream stats;
            stats << "  distance yd: min=" << (uint32)sorted.front()
                  << " med=" << (uint32)sorted[sorted.size() / 2]
                  << " max=" << (uint32)sorted.back();
            messages.push_back(stats.str());

            std::vector<std::pair<std::string, uint32>> top(destCounts.begin(), destCounts.end());
            std::sort(top.begin(), top.end(), [](auto& a, auto& b) { return a.second > b.second; });
            std::ostringstream tops;
            tops << "  top:";
            uint32 shown = 0;
            for (auto& [title, count] : top)
            {
                if (shown >= 5)
                    break;
                tops << " [" << (100 * count / distances.size()) << "%]" << title;
                ++shown;
            }
            messages.push_back(tops.str());

            uint32 beyond = 0;
            for (auto distance : distances)
                if (distance > 1000.0f)
                    ++beyond;

            float pct = 100.0f * (float)beyond / (float)distances.size();
            std::ostringstream flag;
            flag << "  P(>1000y)=" << std::fixed << std::setprecision(1) << pct << "%" << (pct > 5.0f ? " FLAGGED" : "");
            messages.push_back(flag.str());
        }
    }

    return messages;
}

std::list<std::string> TravelProbe::HandleTravelTrace(const std::string& param)
{
    std::list<std::string> messages;

    std::string lowered = param;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

    if (lowered == "on")
    {
        TravelProbeTraceEnabled = true;
        messages.push_back(sPlayerbotAIConfig.hasLog("travel_flip.csv")
            ? "traveltrace: ON (logs/travel_flip.csv)"
            : "traveltrace: ON but travel_flip.csv is missing from AiPlayerbot.AllowedLogFiles, no rows will be written");
    }
    else if (lowered == "off")
    {
        TravelProbeTraceEnabled = false;
        messages.push_back("traveltrace: OFF");
    }
    else if (lowered == "summary")
    {
        std::string logsDir = sConfig.GetStringDefault("LogsDir");
        if (!logsDir.empty() && logsDir.back() != '/' && logsDir.back() != '\\')
            logsDir.append("/");

        std::ifstream in(logsDir + "travel_flip.csv");
        if (!in)
        {
            messages.push_back("traveltrace: no logs/travel_flip.csv found");
            return messages;
        }

        std::map<uint32, std::pair<uint32, uint32>> perLevel;
        std::map<std::string, uint32> reasons;
        uint32 total = 0;

        std::string line;
        while (std::getline(in, line))
        {
            std::vector<std::string> fields;
            std::string field;
            std::istringstream row(line);
            while (std::getline(row, field, ','))
                fields.push_back(field);

            if (fields.size() < 10)
                continue;

            ++total;

            uint32 level = Qualified::isValidNumberString(fields[2]) ? (uint32)stoul(fields[2]) : 0;
            std::string reason = fields[9];
            bool flip = reason == "change" || reason == "forced" || reason == "from-nowhere" || reason == "to-nowhere";

            auto& entry = perLevel[level];
            ++entry.first;
            if (flip)
                ++entry.second;

            ++reasons[reason];
        }

        std::ostringstream head;
        head << "traveltrace summary: " << total << " rows";
        messages.push_back(head.str());

        for (auto& [level, entry] : perLevel)
        {
            std::ostringstream out;
            out << "  level " << level << ": " << entry.second << "/" << entry.first
                << " flips (" << std::fixed << std::setprecision(1) << (entry.first ? 100.0f * (float)entry.second / (float)entry.first : 0.0f) << "%)";
            messages.push_back(out.str());
        }

        std::vector<std::pair<std::string, uint32>> top(reasons.begin(), reasons.end());
        std::sort(top.begin(), top.end(), [](auto& a, auto& b) { return a.second > b.second; });
        std::ostringstream tops;
        tops << "  reasons:";
        for (auto& [reason, count] : top)
            tops << " " << reason << ":" << count;
        messages.push_back(tops.str());
    }
    else
    {
        messages.push_back("usage: traveltrace on|off|summary");
    }

    return messages;
}

#endif
