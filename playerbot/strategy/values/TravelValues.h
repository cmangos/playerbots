#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    typedef std::set<uint32> focusQuestTravelList;

    class FocusTravelTargetValue : public ManualSetValue<focusQuestTravelList>
    {
    public:
        FocusTravelTargetValue(PlayerbotAI* ai, focusQuestTravelList defaultValue = {}, std::string name = "forced travel target") : ManualSetValue<focusQuestTravelList>(ai, defaultValue, name) {};
    };

    //Travel destinations

    using DestinationEntry = int32;
    using EntryGuidps = std::unordered_map<DestinationEntry, std::vector<GuidPosition>>;

    //All creature and gameobject entries and reason a bot might want to travel to them.
    class EntryGuidpsValue : public SingleCalculatedValue<EntryGuidps>
    {
    public:
        EntryGuidpsValue(PlayerbotAI* ai, std::string name = "entry guidps") : SingleCalculatedValue(ai, name) {};

        virtual EntryGuidps Calculate() override;
    };

    using DestinationPurose = uint32;

    //Reasons a bot might want to travel to a creature or object
    enum class TravelDestinationPurposeFlag : uint32
    {
        NONE = 0,
        QUEST = 1 << 0,  //Starts, ends or is objective of some quest.
        RPG = 1 << 1,    //Has a usefull npc flag or gameobject type.
        GRIND = 1 << 2,  //Drops gold. 
        GATHER = 1 << 3, //Can be mined/herbed/skinned ect.       
        BOSS = 1 << 4    //Is a boss, worldboss or unique (silver) elite.
    };

    using EntryTravelPurposeMap = std::unordered_map<DestinationEntry, DestinationPurose>;

    //All creature and gameobject entries and reason a bot might want to travel to them.
    class EntryTravelPurposeMapValue : public SingleCalculatedValue<EntryTravelPurposeMap>
    {
    public:
        EntryTravelPurposeMapValue(PlayerbotAI* ai, std::string name = "entry travel purpose") : SingleCalculatedValue(ai, name) {};

        virtual EntryTravelPurposeMap Calculate() override;

        static uint32 SkillIdToGatherEntry(int32 entry);
    };

    //Travel conditions

    class QuestStageActiveValue : public BoolCalculatedValue, Qualified
    {
    public:
        QuestStageActiveValue(PlayerbotAI* ai, std::string name = "quest stage active", int checkInterval = 1) : BoolCalculatedValue(ai, name, checkInterval) {};

        virtual bool Calculate();
    };
}