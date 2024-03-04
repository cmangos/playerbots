#pragma once

#include "playerbot/playerbot.h"
#include "../Value.h"


namespace ai
{
    class GameObjectsValue : public GuidPositionListCalculatedValue
    {
    public:
        GameObjectsValue(PlayerbotAI* ai, string name = "gos") : GuidPositionListCalculatedValue(ai, name, 1) {}

        virtual list<GuidPosition> Calculate();
    };


    class EntryFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        EntryFilterValue(PlayerbotAI* ai, string name = "entry filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "entry filter"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "entry"; }
        virtual string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids of specific entries.";
        }
        virtual vector<string> GetUsedValues() { return { }; }
#endif 
    };

    class RangeFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        RangeFilterValue(PlayerbotAI* ai, string name = "range filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "range filter"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "entry"; }
        virtual string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids within a specific range.";
        }
        virtual vector<string> GetUsedValues() { return { }; }
#endif 
    };

    class GoUsableFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        GoUsableFilterValue(PlayerbotAI* ai, string name = "go usable filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "go usable filter"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "entry"; }
        virtual string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids within a specific range.";
        }
        virtual vector<string> GetUsedValues() { return { }; }
#endif 
    };

    class GoTrappedFilterValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        GoTrappedFilterValue(PlayerbotAI* ai, string name = "go trapped filter") : GuidPositionListCalculatedValue(ai, name, 1), Qualified() {}

        virtual list<GuidPosition> Calculate();

#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "go trapped filter"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "entry"; }
        virtual string GetHelpDescription()
        {
            return "This value will returns only the ObjectGuids that are not trapped.";
        }
        virtual vector<string> GetUsedValues() { return { }; }
#endif 
    };


    class GosInSightValue : public GuidPositionListCalculatedValue
	{
	public:
        GosInSightValue(PlayerbotAI* ai, string name = "gos in sight") : GuidPositionListCalculatedValue(ai, name, 3) {}

        virtual list<GuidPosition> Calculate() { return AI_VALUE2(list<GuidPosition>, "range filter", "gos," + to_string(sPlayerbotAIConfig.sightDistance)); }
    };

    class GoSCloseValue : public GuidPositionListCalculatedValue, public Qualified
    {
    public:
        GoSCloseValue(PlayerbotAI* ai, string name = "gos close") : GuidPositionListCalculatedValue(ai, name, 3), Qualified() {}

        virtual list<GuidPosition> Calculate() { return AI_VALUE2(list<GuidPosition>, "range filter", "gos," + to_string(INTERACTION_DISTANCE)); }
    };



    class HasObjectValue : public BoolCalculatedValue, public Qualified
    {
    public:
        HasObjectValue(PlayerbotAI* ai, string name = "has object") : BoolCalculatedValue(ai, name, 3), Qualified() {}

        virtual bool Calculate() { return !AI_VALUE(list<GuidPosition>, getQualifier()).empty(); }
    };
}
