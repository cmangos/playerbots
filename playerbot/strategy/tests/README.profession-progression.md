# Profession progression regression tests

The lightweight suites require Python 3 and a compatible C++17 compiler. Some
fixtures additionally extract predicates from a compatible CMaNGOS WotLK core.
They use temporary files and world doubles, without a running realm or database.

## Run the tests

Set PLAYERBOTS to this module checkout, CORE to a compatible WotLK core, and
CORE_REFERENCE to an unpatched compatible reference for remote bridge testing.

```sh
PLAYERBOTS=/path/to/playerbots
CORE=/path/to/compatible/mangos-wotlk
CORE_REFERENCE=/path/to/unpatched/compatible/mangos-wotlk

git -C "$PLAYERBOTS" diff --check
c++ -std=c++17 -DPROFESSION_POLICY_TEST_MAIN -I "$PLAYERBOTS" \
  "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp" \
  -o /tmp/profession-policy-tests
/tmp/profession-policy-tests

python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionSourceTests.py"
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProductionTests.py" --compiler c++
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionComponentTests.py" \
  --compiler c++ --core-source "$CORE"
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionReadinessTests.py" --compiler c++
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/TravelTargetLifecycleTests.py" --compiler c++
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/OutgoingChatRegressionTests.py" --compiler c++
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/RemoteServicesTests.py" \
  --compiler c++ --core-source "$CORE_REFERENCE"
```

The remote suite applies the companion core patch to temporary copies only.
Standalone ProfessionProgressionPolicyTests.cpp needs core headers; use the
standalone regression executable above for independent policy testing.
Scripts accept --compiler for a compatible platform-specific compiler wrapper.

## Coverage and recorded results

| Suite | Coverage | Recorded result |
| --- | --- | --- |
| Policy/regression | Assignment, pending leases, cooldown/clock rollback, whole-cast quantities, one-copy tools, practical vendor policy and runtime-skill fairness. | PASS, including 1,050,000 quantity combinations. |
| Production | Actual planner/processing/config bodies, grey producers, partial quantities, alternate inputs, cycles/depth, retained root state, real input stacks and loot cleanup. | 2,087 cases PASS. |
| Component | Actual focus/tool/vendor/readiness bodies, core category/focus predicates, duplicate vendor slots, upgraded tools, changed inventory and bounded retries. | 54 cases PASS. |
| Complete readiness | Actual CanCraftProfessionValue::Calculate with the processing helper available only through its real class boundary; material/tool/focus/cast/participation/space/input gates. | 17 cases PASS. |
| Source | Shared command/action wiring, selected-plan dispatch, live checks, preserved score, separate fairness state, bounded tool acquisition and config integration. | 23 checks PASS. |
| Travel | Actual expiry/activity/reset predicates, timed/forced/unlimited phases, clock wrap and cleanup. | 59 cases PASS. |
| Outgoing chat | Actual post-parse guard; empty messages stop before recording/reply, nonempty messages proceed. | 6 cases PASS. |
| Remote services | Service/player/thread isolation, nesting/exception restoration, eligibility/busy guards, existing handlers, disabled/unpatched fallback and empty-attempt throttles. | 84 bridge + 7 fallback checks PASS. |

## Negative baseline checks

Where the historical commits are available, these optional comparisons establish
that the fixtures reject the earlier defects:

```sh
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionReadinessTests.py" \
  --compiler c++ --baseline-ref d6482097
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionComponentTests.py" \
  --compiler c++ --core-source "$CORE" --baseline-ref c0100429
```

The readiness script reports success when the old unqualified helper fails to
compile. The older component baseline is expected to fail the focus regression.
The optional remote --baseline-ref 4541a89e comparison checks unchanged transaction
and profession-policy bodies; omit it when testing later intentional changes.

## Full build and gameplay limits

Profession revision be2fa8a4 completed a compatible WotLK mangosd build and startup.
The subsequent upstream Netherspite merge was not part of that full build.
See [the technical review](../../../profession-audit/REPORT.md) and
[build guide](../../../docs/wiki/Build-and-Tests.md) for evidence and instructions.

Fixtures complement full core compilation; extracted bodies do not reproduce the
complete scheduler, database, live economy or every trainer/rank transition.
A queued cast is not completed output. Milling/Prospecting tests never credit
random loot merely because a cast was accepted. Sustained natural progression
across every profession and live remote transactions require observation.
