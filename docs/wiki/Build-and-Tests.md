# Build and tests

[Wiki home](Home.md) | [Regression reference](../../playerbot/strategy/tests/README.profession-progression.md)

## Build model

PlayerBots compiles into the CMaNGOS server build. This feature is not a runtime
loadable binary addon or a client UI addon. The optional remote-service code is
currently integrated into PlayerBots, with a separate companion **core patch**.
Extraction into an independent extension was discussed but not implemented.

Documentation and passing fixtures do not establish that a currently running
realm uses this source. Check actual core/module revisions, CMake source paths,
built binary and installed binary independently.

## Isolated manual build

Use an isolated compatible core/source/build tree. Replace these placeholders
with your actual staging paths:

```sh
CORE=/path/to/isolated/mangos-wotlk
PLAYERBOTS=/path/to/playerbots
BUILD=/path/to/isolated/build
```

For remote services only, first review and apply the bridge to the isolated core:

```sh
git -C "$CORE" apply --check "$PLAYERBOTS/patches/cmangos-wotlk-remote-services.patch"
git -C "$CORE" apply "$PLAYERBOTS/patches/cmangos-wotlk-remote-services.patch"
```

Skip that application if remote services are not wanted, or if the compatible
changes are already present. Do not apply the patch repeatedly or set its marker
manually. Its reference core is WotLK `2fc8161e`; check compatibility with your core.

Configure/build using the core's existing required options as well as these:

```sh
cmake -S "$CORE" -B "$BUILD" -DBUILD_PLAYERBOTS=ON \
  -DFETCHCONTENT_SOURCE_DIR_PLAYERBOTS="$PLAYERBOTS" \
  -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
cmake --build "$BUILD" --target mangosd --parallel 2
```

The source override is for a compatible core using that FetchContent dependency
name. Inspect your core's CMake integration and configure output to confirm it
actually selects this checkout; a successful build of another module checkout
does not validate this one. Preserve your database/client-data/build options.

These commands do not install, deploy or restart services. Run full compilation
under your server's normal build/load/temperature policy. A build failure must be
resolved against the actual compatible core, not concealed with test-only stubs.

## Lightweight checks on Linux

With a C++17 compiler and Python 3 available:

```sh
git -C "$PLAYERBOTS" diff --check

c++ -std=c++17 -DPROFESSION_POLICY_TEST_MAIN -I "$PLAYERBOTS" \
  "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp" \
  -o /tmp/profession-policy-tests
/tmp/profession-policy-tests

python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionSourceTests.py"
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProductionTests.py" --compiler c++
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionComponentTests.py" \
  --compiler c++ --core-source "$CORE"
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/TravelTargetLifecycleTests.py" --compiler c++
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/OutgoingChatRegressionTests.py" --compiler c++
```

For remote services, `CORE_REFERENCE` must be an **unpatched** compatible core
reference because this test applies the bridge to temporary copies:

```sh
CORE_REFERENCE=/path/to/unpatched/compatible/mangos-wotlk
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/RemoteServicesTests.py" \
  --compiler c++ --core-source "$CORE_REFERENCE"
```

The optional `--baseline-ref 4541a89e` remote audit compares transaction/planner
bodies against that historical revision. Omit it for routine future testing when
intentional changes have moved beyond that baseline.

## Lightweight checks on Windows

From PowerShell in the local PlayerBots repository:

```powershell
git diff --check
python -B playerbot/strategy/tests/ProfessionProgressionSourceTests.py

$compilerPath = 'C:\path\to\compatible-cxx-wrapper.cmd'
$coreReference = 'C:\path\to\unpatched\mangos-wotlk'
python -B playerbot/strategy/tests/ProfessionProductionTests.py --compiler $compilerPath
python -B playerbot/strategy/tests/ProfessionProgressionComponentTests.py `
  --compiler $compilerPath --core-source $coreReference
python -B playerbot/strategy/tests/TravelTargetLifecycleTests.py --compiler $compilerPath
python -B playerbot/strategy/tests/OutgoingChatRegressionTests.py --compiler $compilerPath
python -B playerbot/strategy/tests/RemoteServicesTests.py `
  --compiler $compilerPath --core-source $coreReference
```

The C++ harnesses accept a compiler/wrapper compatible with their GCC/Clang-style
arguments. A locally cached Zig wrapper was used for this audit; it is not shipped
as a repository dependency. Python-only source checks do not need a compiler.
Full CMaNGOS configuration on Windows follows your existing core/toolchain setup;
the Linux build example is not a PowerShell command block.

## What the tests prove

| Suite | Scope | Recorded result at the reviewed baseline |
| --- | --- | --- |
| Policy/regression executable | Pending ownership, cooldown/rollback, quantity bounds, generic fairness | PASS, including 1,050,000 quantity combinations |
| Production tests | Actual planner/adapter bodies with doubles, bounded producers, processing ownership/loot and config precedence | 2,087 cases PASS |
| Component tests | Actual focus/tool/vendor/readiness bodies and compatible core predicates with doubles | 54 cases PASS |
| Source tests | Integration routes, metadata-derived requirements, score preservation and no sampled exceptions | 23 checks PASS |
| Travel lifecycle | Actual activity/expiry/reset predicates with lifecycle doubles | 59 cases PASS |
| Outgoing chat | Actual post-parse empty-message guard | 6 cases PASS |
| Remote services | Scoped authorization, patched access predicates, dispatcher and existing action adapters | 84 bridge + 7 fallback checks PASS |

Some recorded suites ran before the upstream merge; 91 remote and 23 source
checks were rerun immediately before `13039f29`, and travel/chat checks passed on
the prepared upstream merge. The table describes evidence, not tests secretly
performed by reading this page.

World doubles do not simulate the complete scheduler, full linkage, live world,
AH economy or every trainer/rank transition. A standalone compilation of
`ProfessionProgressionPolicyTests.cpp` also requires core headers; use the documented
standalone regression executable rather than assuming every test translation unit
builds independently.

## Final validation sequence

1. Pass diff checks and relevant lightweight tests.
2. Confirm actual core/module compatibility and selected CMake source.
3. Complete the full `mangosd` target and inspect errors/warnings.
4. Separately review deployment/configuration through the normal operating process.
5. Observe real progression and remote transactions using [Usage](Usage.md).

Compilation proves compatibility at build time. Only observed game state proves
natural crafting, loot, economy and rank behavior on the running realm.
