#pragma once
#include "../Value.h"
#include "../../TravelMgr.h"

namespace ai
{
    class FindTargetStrategy
    {
    public:
        FindTargetStrategy(PlayerbotAI* ai)
        {
            result = NULL;
            this->ai = ai;
        }

    public:
        Unit* GetResult() { return result; }

    public:
        virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) = 0;
        void GetPlayerCount(Unit* creature, int* tankCount, int* dpsCount);

    protected:
        Unit* result;
        PlayerbotAI* ai;

    protected:
        map<Unit*, int> tankCountCache;
        map<Unit*, int> dpsCountCache;
    };

    class FindNonCcTargetStrategy : public FindTargetStrategy
    {
    public:
        FindNonCcTargetStrategy(PlayerbotAI* ai) : FindTargetStrategy(ai) {}

    protected:
        virtual bool IsCcTarget(Unit* attacker);

    };

    class TargetValue : public UnitCalculatedValue
	{
	public:
        TargetValue(PlayerbotAI* ai, string name = "target", int checkInterval = 1) : UnitCalculatedValue(ai, name, checkInterval) {}
#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "target"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "target"; }
        virtual string GetHelpDescription() { return "This value contains a [h:object|unit] which is the best target based on the current target strategy."; }
        virtual vector<string> GetUsedValues() { return { "possible attack targets" }; }
#endif 
    protected:
        Unit* FindTarget(FindTargetStrategy* strategy);
    };

    class RpgTargetValue : public ManualSetValue<GuidPosition>
    {
    public:
        RpgTargetValue(PlayerbotAI* ai, string name = "rpg target") : ManualSetValue<GuidPosition>(ai, GuidPosition(), name) {}
#ifdef GenerateBotHelp
        virtual string GetHelpName() { return "rpg target"; } //Must equal iternal name
        virtual string GetHelpTypeName() { return "rpg"; }
        virtual string GetHelpDescription() { return "This value contains the [h:object|objectGuid] of a [h:object|unit] or [h:object|gameObject] to move to and rpg with.\nThis value is manually set."; }
        virtual vector<string> GetUsedValues() { return {}; }
#endif 

        virtual string Format()
        {
            return chat->formatGuidPosition(value);
        }
    };

    class TravelTargetValue : public ManualSetValue<TravelTarget*>
    {
    public:
        TravelTargetValue(PlayerbotAI* ai, string name = "travel target") : ManualSetValue<TravelTarget*>(ai, new TravelTarget(ai), name) {}
        virtual ~TravelTargetValue() { delete value; }
    };	

    class LastLongMoveValue : public CalculatedValue<WorldPosition>
    {
    public:
        LastLongMoveValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "last long move", 30) {}

        WorldPosition Calculate();
    };

    class HomeBindValue : public CalculatedValue<WorldPosition>
    {
    public:
        HomeBindValue(PlayerbotAI* ai) : CalculatedValue<WorldPosition>(ai, "home bind", 30) {}

        WorldPosition Calculate();
    };

    class IgnoreRpgTargetValue : public ManualSetValue<set<ObjectGuid>& >
    {
    public:
        IgnoreRpgTargetValue(PlayerbotAI* ai) : ManualSetValue<set<ObjectGuid>& >(ai, data, "ignore rpg targets") {}

    private:
        set<ObjectGuid> data;
    };

    class TalkTargetValue : public ManualSetValue<ObjectGuid>
    {
    public:
        TalkTargetValue(PlayerbotAI* ai, string name = "talk target") : ManualSetValue<ObjectGuid>(ai, ObjectGuid(), name) {}
    };

    class AttackTargetValue : public ManualSetValue<ObjectGuid>
    {
    public:
        AttackTargetValue(PlayerbotAI* ai, string name = "attack target") : ManualSetValue<ObjectGuid>(ai, ObjectGuid(), name) {}
    };

    class PullTargetValue : public UnitManualSetValue
    {
    public:
        PullTargetValue(PlayerbotAI* ai, string name = "pull target") : UnitManualSetValue(ai, nullptr, name) {}
        void Set(Unit* unit) override;
        Unit* Get() override;

    private:
        ObjectGuid guid;
    };

    class FollowTargetValue : public UnitCalculatedValue
    {
    public:
        FollowTargetValue(PlayerbotAI* ai, string name = "follow target") : UnitCalculatedValue(ai, name) {}
        Unit* Calculate() override;
    };

    class ManualFollowTargetValue : public GuidPositionManualSetValue
    {
    public:
        ManualFollowTargetValue(PlayerbotAI* ai, string name = "manual follow target") : GuidPositionManualSetValue(ai, GuidPosition(), name) {}
    };

    class ClosestAttackerTargetingMeTargetValue : public UnitCalculatedValue
    {
    public:
        ClosestAttackerTargetingMeTargetValue(PlayerbotAI* ai, string name = "closest attacker targeting me") : UnitCalculatedValue(ai, name) {}
        Unit* Calculate() override;
    };
}
