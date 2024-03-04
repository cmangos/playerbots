#pragma once
#include "MovementActions.h"
#include "playerbot/strategy/values/HazardsValue.h"

namespace ai
{
    class MoveAwayFromHazard : public MovementAction
    {
    public:
        MoveAwayFromHazard(PlayerbotAI* ai, string name = "move away from hazard") : MovementAction(ai, name) {}
        bool Execute(Event& event) override;
        bool isPossible() override;

    private:
        bool IsHazardNearby(const WorldPosition& point, const list<HazardPosition>& hazards) const;
    };

    class MoveAwayFromCreature : public MovementAction
    {
    public:
        MoveAwayFromCreature(PlayerbotAI* ai, string name, uint32 creatureID, float range) : MovementAction(ai, name), creatureID(creatureID), range(range) {}
        bool Execute(Event& event) override;
        bool isPossible() override;

    private:
        bool IsValidPoint(const WorldPosition& point, const list<Creature*>& creatures, const list<HazardPosition>& hazards);
        bool HasCreaturesNearby(const WorldPosition& point, const list<Creature*>& creatures) const;
        bool IsHazardNearby(const WorldPosition& point, const list<HazardPosition>& hazards) const;

    private:
        uint32 creatureID;
        float range;
    };
}