#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"
#include "playerbot/strategy/values/ProfessionCraftingFairness.h"

namespace
{
    using namespace ai::profession;

    static_assert(!IsPendingCraftExpired(200, 150, 120, false),
        "a recently queued recipe must still exclude a competing dispatch");
    static_assert(IsPendingCraftExpired(270, 150, 120, false),
        "a consumed or discarded event must not lock crafting forever");
    static_assert(!IsPendingCraftExpired(1000, 150, 120, true),
        "an actual long cast must never expire just because its lease is old");
    static_assert(IsPendingCraftExpired(200, 0, 120, false),
        "a legacy orphan without a timestamp must recover");
    static_assert(!IsPendingCraftExpired(200, 0, 120, true),
        "legacy recovery must still preserve a genuine active cast");
    static_assert(IsPendingCraftExpired(100, 150, 120, false),
        "clock rollback must not make pending state permanent");
    static_assert(IsNcCraftUseful(true, true),
        "a pending autonomous request must reach Execute's StopMoving path");
    static_assert(!IsNcCraftUseful(true, false),
        "ordinary nc commands must retain their movement restriction");
    static_assert(IsNcCraftUseful(false, false),
        "stationary ordinary nc commands must remain eligible");
    static_assert(KeepsPendingCraft(true, 5),
        "an accepted batch must retain ownership of its continuation");
    static_assert(!KeepsPendingCraft(true, 1),
        "the last accepted cast must release the pending request");
    static_assert(!KeepsPendingCraft(false, 5),
        "a rejected cast must release the pending request even in a batch");
    static_assert(CanDispatchProfessionPlan(true, true, false, false),
        "a known ready plan may use the shared maintenance action");
    static_assert(!CanDispatchProfessionPlan(false, true, false, false),
        "a stale plan must never insert an unknown recipe");
    static_assert(!CanDispatchProfessionPlan(true, false, false, false),
        "cooldown must not dispatch a fallback recipe");
    static_assert(!CanDispatchProfessionPlan(true, true, true, false),
        "pending ownership must exclude another batch");
    static_assert(!CanDispatchProfessionPlan(true, true, false, true),
        "the last accepted cast must finish before another dispatch");

    // Quantities from live WotLK data are examples, not spell-ID branches.
    static_assert(AvailableCraftBatch(BoundCraftBatch(5, 1, 20), 4) == 4,
        "four available units must allow four normal one-unit crafts");
    static_assert(AvailableCraftBatch(BoundCraftBatch(5, 8, 20), 20 / 8) == 2,
        "a target of twenty must cover two whole eight-unit casts, not five");
    static_assert(BoundCraftBatch(5, 8, 20) * 8 == 16,
        "requirements must equal the actual whole batch consumption");
    static_assert(BoundCraftBatch(5, 25, 20) == 1,
        "one cast's requirement must never be truncated below the spell cost");
    static_assert(AvailableCraftBatch(BoundCraftBatch(5, 2, 20), 0) == 5,
        "a missing-material plan must retain bounded acquisition demand");
    static_assert(AvailableCraftBatch(5, 2) == 2,
        "the least available reagent must limit a multi-reagent batch");
    static_assert(BoundCraftBatch(5, 0, 20) == 5,
        "an absent reagent must not divide by zero or reduce the batch");
    static_assert(IsCashVendorStock(true, false, 0), "ordinary unlimited vendor stock is usable");
    static_assert(!IsCashVendorStock(true, false, 42), "currency-only stock must not suppress AH fallback");
    static_assert(!IsCashVendorStock(true, true, 0), "the existing index excludes limited stock");
    static_assert(!IsCashVendorStock(false, false, 0), "unrelated stock is not a reagent source");
}

#ifdef PROFESSION_POLICY_TEST_MAIN
#include <cassert>
#include <iostream>

namespace
{
    void TestFairness()
    {
        // Arbitrary runtime IDs, independent of actual professions/recipes.
        std::vector<CraftCandidate> candidates = {
            {1001, 900001, 1000055000, true},
            {1002, 900002, 1000025000, true},
            {1003, 900003, 1000020000, true},
            {1002, 900004, 1000015000, true}
        };
        CraftingFairness state;
        assert(state.Select(candidates, 100, false) == 0);
        // Hold the opportunity through ordinary cache recalculations.
        assert(state.Select(candidates, 399, false) == 0);
        assert(state.readySince.size() == 3); // Per skill, not per recipe.
        assert(state.Select(candidates, 400, false) == 1);
        assert(state.Select(candidates, 401, false) == 1);
        state.FinishOpportunity(405);
        assert(state.Select(candidates, 406, false) == 2);
        state.FinishOpportunity(410);
        assert(state.Select(candidates, 411, false) == 0); // Scores still matter.

        // A pending batch retains ownership after consuming its materials and
        // beyond the fairness deadline. Its final accepted cast releases it.
        candidates[0].ready = false;
        assert(state.Select(candidates, 1000, true) == 0);
        assert(state.ownerSkill == 1001);
        state.FinishOpportunity(1001);
        assert(state.Select(candidates, 1002, false) == 1);

        // A capped/unlearned recipe can disappear during the pending batch.
        // Do not transfer its ledger ownership to a different skill mid-cast.
        CraftingFairness disappearing;
        disappearing.Select(candidates, 100, false);
        candidates.erase(candidates.begin());
        disappearing.Select(candidates, 101, true);
        assert(disappearing.ownerSkill == 0); // Initial best had no tools/materials.
        candidates = {{2001, 800001, 100, true}, {2002, 800002, 90, true}};
        disappearing.Select(candidates, 200, false);
        candidates.erase(candidates.begin());
        assert(disappearing.Select(candidates, 900, true) == 0);
        assert(disappearing.ownerSkill == 2001);

        // An unready skill neither ages nor forces an acquisition rotation.
        CraftingFairness unready;
        candidates = {{3001, 700001, 100, true}, {3002, 700002, 90, false}};
        unready.Select(candidates, 100, false);
        assert(unready.Select(candidates, 1000, false) == 0);
        assert(!unready.readySince.count(3002));
        candidates[1].ready = true;
        unready.Select(candidates, 1001, false);
        assert(unready.readySince.at(3002) == 1001);
        candidates[1].ready = false;
        unready.Select(candidates, 1002, false);
        assert(!unready.readySince.count(3002));

        // Clock rollback resets waits/ownership instead of unsigned-age wrap.
        CraftingFairness rollback;
        candidates = {{4001, 600001, 100, true}, {4002, 600002, 90, true}};
        rollback.Select(candidates, 1000, false);
        assert(rollback.Select(candidates, 100, false) == 0);
        assert(rollback.readySince.at(4002) == 100);
        assert(rollback.Select(candidates, 400, false) == 1);
        assert(rollback.Select({}, 401, false) == CraftingFairness::NoCandidate);
        assert(rollback.readySince.empty() && !rollback.ownerSkill);

        // Repeated successful high-score batches cannot monopolize selection.
        // Vary runtime IDs and candidate order without changing the policy.
        for (uint32_t offset = 0; offset < 100; ++offset)
        {
            CraftingFairness recurring;
            std::vector<CraftCandidate> ready = {
                {5001 + offset * 10, 500001 + offset * 10, 1000055000, true},
                {5002 + offset * 10, 500002 + offset * 10, 1000025000, true},
                {5003 + offset * 10, 500003 + offset * 10, 1000020000, true}
            };
            if (offset % 2)
                std::reverse(ready.begin(), ready.end());
            std::map<uint32_t, uint32_t> lastServed;
            for (uint32_t now = 100; now <= 1300; now += 5)
            {
                size_t selected = recurring.Select(ready, now, false);
                uint32_t skill = ready[selected].skillId;
                lastServed[skill] = now;
                recurring.FinishOpportunity(now + 1);
                if (now >= 415)
                    for (const CraftCandidate& candidate : ready)
                    {
                        assert(lastServed.count(candidate.skillId));
                        assert(now - lastServed.at(candidate.skillId) <= 315);
                    }
            }
        }
    }
}

int main()
{
    TestFairness();
    // A moving request is consumed, never executed, and becomes orphaned.
    uint32_t queuedAt = 100;
    assert(!IsPendingCraftExpired(219, queuedAt, 120, false));
    assert(IsPendingCraftExpired(220, queuedAt, 120, false));
    // Recovery queues a new request; the existing spell executes normally.
    queuedAt = 220;
    assert(IsNcCraftUseful(true, true));
    assert(!IsPendingCraftExpired(500, queuedAt, 120, true));
    // A continuation refreshes ownership rather than allowing a second batch.
    queuedAt = 500;
    assert(!IsPendingCraftExpired(501, queuedAt, 120, false));
    // Exhaustively vary quantities independently of names, IDs and professions.
    for (uint32_t batch = 1; batch <= 20; ++batch)
        for (uint32_t perCast = 1; perCast <= 50; ++perCast)
            for (uint32_t target = 1; target <= 50; ++target)
                for (uint32_t available = 0; available <= 20; ++available)
                {
                    uint32_t bounded = BoundCraftBatch(batch, perCast, target);
                    uint32_t count = AvailableCraftBatch(bounded, available);
                    assert(count >= 1 && count <= batch);
                    assert(count * perCast >= perCast);
                    assert(count * perCast <= target || count == 1);
                    assert(!available || count <= available);
                }
    std::cout << "PASS: pending lifecycle, movement, vendor eligibility, fairness, and 1,050,000 batch cases\n";
}
#endif
