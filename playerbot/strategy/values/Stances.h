#pragma once
#include "../Value.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "Formations.h"

namespace ai
{
    class Stance : public Formation
    {
    public:
        Stance(PlayerbotAI* ai, string name) : Formation (ai, name) {}
        virtual ~Stance() {}
    protected:
        virtual Unit* GetTarget();
        virtual WorldLocation GetLocationInternal() = 0;
        virtual WorldLocation GetNearLocation(float angle, float distance);

    public:
        virtual WorldLocation GetLocation();
        virtual string GetTargetName() { return "current target"; }
        virtual float GetMaxDistance() { return sPlayerbotAIConfig.contactDistance; }
    };

    class MoveStance : public Stance
    {
    public:
        MoveStance(PlayerbotAI* ai, string name) : Stance (ai, name) {}

    protected:
        virtual WorldLocation GetLocationInternal();
        virtual float GetAngle() = 0;

    public:
    };


    class StanceValue : public ManualSetValue<Stance*>
	{
	public:
        StanceValue(PlayerbotAI* ai);
        ~StanceValue() { if (value) { delete value; value = NULL; } }
        virtual void Reset();
        virtual string Save();
        virtual bool Load(string value);
    };

    class SetStanceAction : public ChatCommandAction
    {
    public:
        SetStanceAction(PlayerbotAI* ai) : ChatCommandAction(ai, "set Stance") {}
        virtual bool Execute(Event& event) override;
    };
};

