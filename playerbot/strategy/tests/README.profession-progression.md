# Offline profession regressions

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
protection, continuation ownership, maintenance dispatch eligibility, practical
cash-vendor policy and 1,050,000 combinations of batch/reagent/target/stock
quantities. Its fairness cases cover repeated high-score batches, opportunity
expiry, pending ownership, disappeared recipes, changing readiness, clock rollback
and arbitrary runtime IDs/candidate order. Assertions also
compile as part of the normal PlayerBots source build without a standalone main.

The component test extracts actual source bodies for focus resolution, spell
readiness and tool usefulness, plus the core focus and category predicates. It
compiles these with small inventory/world/context doubles. The current patch
adds actual vendor-stock and indexed destination lookup bodies for 30 cases,
including duplicate offers aligned with BuyItem's first matching stock slot.
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
vendor fallback. They do not execute C++ or simulate the scheduler.

The previous audit reported passing tests/production-unit compilation for an
earlier version of the patch. Those results do not validate the current fairness
and vendor changes. This local-only continuation passed 13 source checks and
Python syntax checks; no C++ compiler/core checkout is available here. The current
C++ policy/component tests and full core build await owner validation. No CMake,
remote build, deployment or restart was performed during this continuation.

The patch addresses dispatch/pending lifecycle, real focus resolution, honest
batch quantities, learned tool requirements, bounded ready-skill fairness and
practical vendor fallback. It does not implement grey intermediate production,
autonomous milling/prospecting or dedicated tool acquisition. Generic stale travel
expiry/reset usefulness and the reset-command bad_alloc were not patched.
