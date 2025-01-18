#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    //Definition
    using DestinationEntry = int32;
    using EntryGuidps = std::unordered_map<DestinationEntry, std::vector<AsyncGuidPosition>>;

    //All creature and gameobject entries and reason a bot might want to travel to them.
    class EntryGuidpsValue : public SingleCalculatedValue<EntryGuidps>
    {
    public:
        EntryGuidpsValue(PlayerbotAI* ai, std::string name = "entry guidps") : SingleCalculatedValue(ai, name) {};

        virtual EntryGuidps Calculate() override;
    };

    using DestinationPurose = uint32;

    using EntryTravelPurposeMap = std::unordered_map<DestinationEntry, DestinationPurose>;

    //All creature and gameobject entries and reason a bot might want to travel to them.
    class EntryTravelPurposeMapValue : public SingleCalculatedValue<EntryTravelPurposeMap>
    {
    public:
        EntryTravelPurposeMapValue(PlayerbotAI* ai, std::string name = "entry travel purpose") : SingleCalculatedValue(ai, name) {};

        virtual EntryTravelPurposeMap Calculate() override;

        static uint32 SkillIdToGatherEntry(int32 entry);
    };

    enum class TravelDestinationPurpose : uint32
    {
        None = 0,
        QuestGiver = 1 << 0,
        QuestObjective1 = 1 << 1,
        QuestObjective2 = 1 << 2,
        QuestObjective3 = 1 << 3,
        QuestObjective4 = 1 << 4,
        QuestTaker = 1 << 5,
        GenericRpg = 1 << 6,
        Trainer = 1 << 7,
        Repair = 1 << 8,
        Vendor = 1 << 9,
        AH = 1 << 10,
        Mail = 1 << 11,
        Grind = 1 << 12,
        Boss = 1 << 13,
        GatherSkinning = 1 << 14,
        GatherMining = 1 << 15,
        GatherHerbalism = 1 << 16,
        GatherFishing = 1 << 17,
        Explore = 1 << 18,
        MaxFlag = 1 << 19
    };

    class TravelDestination;

    using DestinationList = std::list<TravelDestination*>;

    //TypedDestinationMap[Purpose][QuestId/Entry]={TravelDestination}
    using EntryDestinationMap = std::unordered_map<int32, DestinationList>;
    using PurposeDestinationMap = std::unordered_map<TravelDestinationPurpose, EntryDestinationMap>;

    //Usage

    //TravelPoint[point, destination, distance]
    using TravelPoint = std::tuple<TravelDestination*, WorldPosition*, float>;
    using TravelPointList = std::list<TravelPoint>;
    using PartitionedTravelList = std::unordered_map<uint32, TravelPointList>;

    typedef std::set<uint32> focusQuestTravelList;

    class FocusTravelTargetValue : public ManualSetValue<focusQuestTravelList>
    {
    public:
        FocusTravelTargetValue(PlayerbotAI* ai, focusQuestTravelList defaultValue = {}, std::string name = "forced travel target") : ManualSetValue<focusQuestTravelList>(ai, defaultValue, name) {};
    };

    class HasFocusTravelTargetValue : public BoolCalculatedValue
    {
    public:
        HasFocusTravelTargetValue(PlayerbotAI* ai, std::string name = "has focus travel target", int checkInterval = 10) : BoolCalculatedValue(ai, name, checkInterval) {}

        virtual bool Calculate() override { return !AI_VALUE(focusQuestTravelList, "forced travel target").empty(); };
    };

    class TravelDestinationsValue : public ManualSetValue<PartitionedTravelList>, public Qualified
    {
    public:
        TravelDestinationsValue(PlayerbotAI* ai, std::string name = "travel destinations") : ManualSetValue<PartitionedTravelList>(ai, {}, name), Qualified() {}
    };

    class NeedTravelPurposeValue : public BoolCalculatedValue, public Qualified
    {
    public:
        NeedTravelPurposeValue(PlayerbotAI* ai, std::string name = "need travel purpose", int checkInterval = 10) : BoolCalculatedValue(ai, name, checkInterval), Qualified() {};

        virtual bool Calculate() override;
    };

    class ShouldTravelNamedValue : public BoolCalculatedValue, public Qualified
    {
    public:
        ShouldTravelNamedValue(PlayerbotAI* ai, std::string name = "should travel named", int checkInterval = 10) : BoolCalculatedValue(ai, name, checkInterval), Qualified() {};

        virtual bool Calculate() override;
    };

    class InOverworldValue : public BoolCalculatedValue
    {
    public:
        InOverworldValue(PlayerbotAI* ai, std::string name = "in overworld", int checkInterval = 10) : BoolCalculatedValue(ai, name, checkInterval) {};

        virtual bool Calculate() override {return WorldPosition(bot).isOverworld();}
    };

    //Travel conditions

    class QuestStageActiveValue : public BoolCalculatedValue, public Qualified
    {
    public:
        QuestStageActiveValue(PlayerbotAI* ai, std::string name = "quest stage active", int checkInterval = 1) : BoolCalculatedValue(ai, name, checkInterval), Qualified() {};

        virtual bool Calculate();
    };
}