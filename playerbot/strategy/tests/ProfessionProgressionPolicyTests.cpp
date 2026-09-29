#include "playerbot/playerbot.h"
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"

using ai::profession::AssignmentPair;

namespace
{
    constexpr bool Same(AssignmentPair value, uint16_t first, uint16_t second)
    {
        return value.first == first && value.second == second;
    }

    static_assert(Same(ai::profession::ReconcileAssignments(164, 165, 164, 165), 164, 165),
        "two persisted professions must remain authoritative");
    static_assert(Same(ai::profession::ReconcileAssignments(171, 182, 164, 165), 164, 165),
        "two stale metadata values must be replaced by persisted professions");
    static_assert(Same(ai::profession::ReconcileAssignments(164, 165, 164, 0), 164, 165),
        "a valid stored companion may restore a lost skill-1 assignment");
    static_assert(Same(ai::profession::ReconcileAssignments(171, 182, 164, 0), 164, 0),
        "unrelated stale metadata must not resurrect a profession");
    static_assert(Same(ai::profession::ReconcileAssignments(164, 164, 0, 0), 164, 0),
        "the same profession must never be assigned twice");

    static_assert(ai::profession::CanaryBucket(12345) == ai::profession::CanaryBucket(12345),
        "canary selection must be deterministic");
    static_assert(!ai::profession::IsInCanary(12345, 0), "a zero-percent canary must be empty");
    static_assert(ai::profession::IsInCanary(12345, 100), "a full canary must include every bot");

    static_assert(ai::profession::IsReasonableAuctionStack(4, 0, 4, 20, 20),
        "an exact stack must be accepted");
    static_assert(ai::profession::IsReasonableAuctionStack(20, 0, 4, 20, 20),
        "one bounded reserve stack may be accepted");
    static_assert(!ai::profession::IsReasonableAuctionStack(200, 0, 4, 20, 200),
        "an excessive reserve stack must be rejected");
    static_assert(!ai::profession::IsReasonableAuctionStack(20, 16, 4, 20, 20),
        "a reserve stack must not exceed the total bounded reserve");

    static_assert(ai::profession::IsCooldownReady(100, 0, 30), "no previous craft is ready");
    static_assert(!ai::profession::IsCooldownReady(100, 80, 30), "craft cooldown must pace batches");
    static_assert(ai::profession::IsCooldownReady(110, 80, 30), "elapsed craft cooldown must reopen");

    static_assert(ai::profession::ShouldTravelToAuctionHouse(true, true, true, true, true, true, true, false),
        "a valid affordable profession AH need should drive travel");
    static_assert(!ai::profession::ShouldTravelToAuctionHouse(true, true, true, true, true, false, true, false),
        "AH cooldown must suppress travel");
    static_assert(!ai::profession::ShouldTravelToAuctionHouse(true, true, true, true, true, true, true, true),
        "a real-player master must suppress autonomous travel");
    static_assert(ai::profession::ShouldTravelForSources(true, true, true, true, true, false),
        "a valid vendor or gathering source should drive travel");
    static_assert(!ai::profession::ShouldTravelForSources(true, true, false, true, true, false),
        "vendor travel requires a real vendor source");
    static_assert(!ai::profession::ShouldTravelForSources(true, true, true, true, true, true),
        "gather travel excludes a real-player-master bot");
    static_assert(ai::profession::ShouldTravelToSpellFocus(true, true, 3, false, true, false),
        "a ready recipe with materials should travel to its real spell focus");
    static_assert(!ai::profession::ShouldTravelToSpellFocus(true, true, 3, true, true, false),
        "a focus trip must wait until recipe materials are present");

#ifdef MANGOSBOT_TWO
    static_assert(SKILL_INSCRIPTION > 0, "WotLK builds must expose Inscription");
#endif
}
