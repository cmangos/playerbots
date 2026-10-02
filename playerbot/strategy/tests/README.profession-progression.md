# Offline profession regressions

The 2026-10-02 follow-up below supersedes older staging/build notes for the
current production-demand and configuration patch.

These tests require no realm, database, service, items or skill modification.
All implementation decisions use learned spells, inventory, tool/category and
focus data, and runtime action state. Live bot names and recipe IDs are evidence
labels, not dispatch exceptions.

With `PLAYERBOTS` pointing to this checkout and `CORE` to compatible CMaNGOS:

```sh
c++ -std=c++17 -DPROFESSION_POLICY_TEST_MAIN -I "$PLAYERBOTS" \
  "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp" \
  -o /tmp/profession-policy-tests
/tmp/profession-policy-tests
python3 "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionComponentTests.py" \
  --core-source "$CORE"
```

The policy executable covers pending leases, clock rollback, active-cast
protection, continuation ownership, nonrenewing same-spell retries, one-copy
tool auction reserves, maintenance dispatch eligibility, practical
cash-vendor policy and 1,050,000 combinations of batch/reagent/target/stock
quantities. Its fairness cases cover repeated high-score batches, opportunity
expiry, pending ownership, disappeared recipes, changing readiness, clock rollback
and arbitrary runtime IDs/candidate order. Assertions also
compile as part of the normal PlayerBots source build without a standalone main.

## 2026-10-02 production-demand follow-up

```sh
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProductionTests.py" \
  --compiler c++
```

This runs 2,087 C++ cases with world doubles, including the actual full planner,
processing input/output metadata index, owned input target/request lifecycle,
post-cast cleanup block and configuration assignments. It checks bounded learned
prerequisite expansion, grey producers, partial quantities, alternate processing
inputs, unknown recipes, cycles/depth limits, root fairness ownership, real-stack
requirements, matching loot completion and canonical/legacy setting precedence.
No test credits a processing output just because a cast was accepted.

The separate source suite now has 23 wiring checks. Existing policy, 54 component,
59 travel and 6 outgoing-chat cases remain relevant. A portable checksum-verified
Zig compiler under the local repository's `.git/profession-test-tools` can run the
Windows fixtures without installing a compiler globally. It is not a repository
dependency; the scripts accept an ordinary compatible C++17 compiler.

The current patch has passed these lightweight tests. It has **not** undergone a
full CMaNGOS build or live skill-progression validation. Earlier successful full
build/deployment records below refer to earlier revisions. The bounded bridge
supports at most two prerequisite steps, not arbitrary-depth production; missing
raw materials/tools/money/recipes and unsupported processing loot sources remain
real blockers. See [configuration options](../../../docs/PROFESSION_PROGRESSION.md)
and [the audit report](../../../profession-audit/REPORT.md).

The component test extracts actual source bodies for focus resolution, spell
readiness and tool usefulness, plus the core focus and category predicates. It
compiles these with small inventory/world/context doubles. The current patch
adds actual vendor-stock, indexed destination lookup, learned craft/enchant
requirements, tool selection/supplies and pending queue bodies for 54 cases.
These include duplicate offers aligned with BuyItem's first matching stock slot,
compatible upgraded tools, newly learned requirements, one-copy demands,
live ownership after a cached selection and 100 retries preserving lease age.
It does not simulate the AI scheduler or establish live skill gain.

For a negative baseline comparison:

```sh
python3 "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionComponentTests.py" \
  --core-source "$CORE" --baseline-ref c0100429
```

This is expected to fail on a distant selected focus with a valid nearby focus.

Source wiring checks require only Python and Git:

```sh
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionSourceTests.py"
git -C "$PLAYERBOTS" diff --check
```

These check integration boundaries, including unchanged score coefficients,
live-readiness checks before queueing, single-plan maintenance dispatch,
continuations, retained focus/tool checks, independent fairness state and
vendor fallback, common tool acquisition and one-copy auction filtering. They do
not execute C++ or simulate the scheduler.

On 2026-10-01 the current local patch passed 18 Python source checks, the C++17
policy/regression executable (including 1,050,000 batch cases) and all 54 component
cases. Windows has no C++ compiler on PATH; lightweight C++ execution used an
isolated temporary directory on the existing Linux compiler host and read-only
compatible core predicates. This was not a CMaNGOS build. The new tool/retry patch
has not been fully compiled or deployed; the earlier successful 848/848 core build
predates these edits. Full core compilation and live validation remain required.

The patch addresses dispatch/pending lifecycle, real focus resolution, honest
batch quantities, learned tool requirements, bounded ready-skill fairness and
practical vendor fallback and one-copy tool demands through existing vendor/AH
actions. It does not implement grey intermediate production or autonomous
milling/prospecting. Tool acquisition still requires affordable legitimate supply;
the AH fallback currently selects one metadata-compatible alternative, and cannot
guarantee that alternative is listed. Generic stale travel
expiry/reset usefulness and the reset-command bad_alloc were not patched.

The tool/retry patch subsequently compiled in the isolated 37/37 full mangosd
build and was installed at the owner's explicit request. Live observation found
additional generic travel expiry and empty-chat defects. The separate fixtures
execute the production deadline/status/reset predicates (59 cases) and the
post-parse outgoing-chat guard (6 cases):

```sh
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/TravelTargetLifecycleTests.py"
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/OutgoingChatRegressionTests.py"
```

They require a C++17 compiler but no running realm. Expiry applies on activity
reads without a movement action; forced/unlimited targets, inactive phases,
clock wrap, cleanup once and existing reset safety gates are covered. Empty chat
returns before recording or reply handling; nonempty payloads still pass.
Earlier NOT BUILT and NOT PATCHED statements above describe the earlier snapshot,
not the final lifecycle fixes.
