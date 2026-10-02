# Profession progression: local source review

## Current continuation status — 2026-10-01 15:41 CEST

Local branch and fetched GitHub feature both point to ff775621; existing local
changes were preserved (0 ahead / 0 behind before the new uncommitted edits).
The published 258c1f7e/ff775621 patch was subsequently built successfully,
848/848, on the isolated two-job server build. Build/installed/running SHA256
was verified as 215f5320c72ebcd8cf2f0c8d702f77c3bfc27e8e0aa431e36fa5df57ce6d77f0.
That result does not validate the new local tool-acquisition/retry-lease edits.
Read the dated continuation sections below for current changes/results;
earlier Git/test/scope snapshots are historical.

## Owner-authorized server build continuation

On 2026-10-01 the owner subsequently authorized pushing the patch, a two-job
server build and restarting WoW services. Production profession changes were
published as `258c1f7e802a56e61bc5fae58e675ac304fc67e3`. Master and the closed
upstream PR were not modified. The current C++ policy/fairness executable passed
all cases, including 1,050,000 batch combinations; all 30 component cases and
13 source checks passed on the server.

The first full build compiled the changed production files and profession policy
test sources, then failed in the unchanged baseline `TestContext.cpp:42-44`.
`Reset()` closes before its delivery cleanup, leaving two statements and another
closing brace at file scope. The same malformed source is present in `c0100429`,
so this is a pre-existing test-framework build blocker, not a profession failure.
Both services were restored automatically and the installed binary remained
unchanged. The build peaked at 81C; temperature warnings did not stop it.

Smallest proposed unblock, kept in a separate commit: move that delivery cleanup
inside Reset by removing the premature brace, and use the existing
groupDeliveryMutex that already protects RecordDeliveredGroupMember and
GetDeliveredGroupMembers. No profession/engine/travel behavior changes. This
executes only when the test context resets, adding no normal-bot planning cost.
The cached full build will verify the actual translation unit before deployment.

The next full two-job build linked successfully, but initial startup aborted in
PlayerbotAIConfig::Initialize. The deployment coordinator restored the previous
binary and both services. Offline core analysis identifies Config::Reload under
that initializer; source inspection proved a build setup error: game/CMakeLists.txt
exports the canonical `src/modules/PlayerBots` include path before the overridden
module path. Core code therefore used upstream `1bafc213` headers while the linked
library used the audited feature. Compiled layout probes measured
sizeof(PlayerbotAIConfig)=377088 versus377136 bytes. This violates the C++ class
layout contract and is distinct from the earlier full-AI-reset bad_alloc.

Smallest operational correction: preserve the older module directory inside the
isolated audit area and make its canonical header path resolve to the audited
module. Recompile core translation units/PCH against the same headers, retaining
the already correctly compiled module objects. No live checkout or CMaNGOS
source behavior is changed. A FetchContent source override alone is insufficient
on this core; every canonical module include must resolve to the same source.

Scope: `C:/Users/Gebruiker/Documents/GitHub/playerbots`, feature branch at
`c0100429a9b80b3b571d663399ac8a278d91cfbf`, with the existing uncommitted audit
patch preserved. No SSH, live access, CMake/full build, commit or push is part of
this continuation. Earlier population observations motivate this review; this
document's new conclusions are derived from the current source.

## Existing fix hardening — evidence and proposed changes

`ChatCommandTrigger` does not implement `IsExternalEvent`, so Engine's packet
event latch does not preserve chat casts. ReactionEngine pops a reaction before
checking usefulness; rejecting a moving nc craft can leave the manual pending
spell without Execute cleanup. The existing lease/movement correction is
therefore still appropriate; no Engine rewrite is needed.

The current shared action checks cached craft readiness, and the optional RPG
preference can insert the selected spell without rechecking that it is still
learned. Maintenance must recheck live learned state, reagents, tools, focus and
nonzero cast quantity before assigning pending ownership. Normal core spell
validation remains final. Use the existing readiness/action path, not a second
spell implementation. The planner's partial-batch pass currently counts each
reagent twice; store counts locally per calculation to avoid additional bag
walks. These are bounded changes to the existing fixes.

## Arbitration — demonstrated root cause and smallest proposal

`ProfessionCraftingPlanValue::Calculate` considers all known primary/secondary
crafts and retains only the largest score. The ready tier is one billion; then
absolute DBC yellow threshold and missing quantities decide. There is no memory
of a competing skill's wait. The same ready, higher-scoring skill can therefore
win on every cache refresh even when another skill has a valid ready recipe.
Clearing pending cannot correct this selection invariant.

Keep every score coefficient unchanged. Add a small per-bot fairness value,
keyed only by runtime skill IDs, tracking continuously ready candidates and the
current recipe's bounded ownership opportunity. Normal scoring decides until
a ready skill has waited five minutes. The oldest overdue skill then receives
an opportunity; existing scores break equal-age ties and choose useful recipes.
Hold that selected recipe through a batch or a five-minute opportunity, so a
refresh cannot switch immediately back to the former winner. Release ownership
after the last accepted cast; a failed/stuck opportunity expires without needing
an AI reset. Pending continuations must not be reassigned. Remove waiting state
for skills with no ready candidate. Clock rollback must not strand ownership.

Reuse CalculatedValue planning, ManualSetValue runtime state, the existing
CastCustomSpellAction acceptance/continuation hook and CraftRandomItemAction.
Do not change profession order, names, configured recipe scores or action
priorities. Fairness grants an opportunity, not a fake skill gain, guaranteed
material supply or automatic trainer eligibility.

For about1,500 bots, add one small map of currently ready skills and an ownership
record per bot. Work occurs only on cached plan calculation/accepted craft, not
each AI tick. The existing cached known-spell scan remains; no world/item/vendor
scan enters arbitration. The time bound applies while candidates stay ready and
AI planning runs; an active real cast/pending batch retains ownership until it
terminates or the existing recovery lease permits replanning.

## Intermediate production — investigation before implementation

`CraftSpellsValue` enumerates learned CREATE_ITEM spells. Smelting, cloth bolts,
engineering components and inks already fit this enumeration and use the shared
normal craft action. They can be ordinary skill-up candidates. Optional
RpgCraftAction also delegates to that action; enabling its strategy does not
build a dependency graph. ShouldCraftSpellValue can allow a grey recipe for a
useful output, but ProfessionCraftingPlan filters it out unless it gives a
skill-up. CraftData tracks only direct required/obtained items.

Milling/prospecting are item-target processing effects, rather than direct
CREATE_ITEM recipes with a standard reagent array. CastCustomSpellAction and
PlayerbotAI already accept item targets; ItemForSpellValue searches inventory
for spell-compatible items. LootValues knows the separate milling/prospecting
loot stores. None of these establishes an autonomous demand-to-processing path:
the profession planner neither enumerates those effects nor requests their
input/output chain. They must not be appended blindly to the direct recipe list.

No recursive production implementation is proposed here. Ready initial recipes
do not require it to resolve the demonstrated arbitration failure. Later grey
producers and multi-step material chains need a separately tested bounded demand
policy; existing actions should execute each normal spell. Required inputs,
known producer, tools, focus, processing eligibility and loot completion remain
real game requirements.

## Direct acquisition — demonstrated generic inconsistency

GetMaterialSources treats any indexed vendor as a usable source and suppresses
AH fallback. VendorMap includes unlimited extended-cost stock; meanwhile
VendorHasUsefulItem explicitly skips extended-cost items. TravelMgr's normal
partitions also reject locations inappropriate for the bot, but material source
classification does not apply that check. An index hit can therefore select a
currency-only, hostile or unusable-location vendor without a viable purchase
journey, while preventing the existing AH fallback.

Smallest proposed correction: filter only profession source classification
through actual vendor stock metadata and existing hostility/location/destination
rules. Do not alter the shared vendor index's behavior for other users. Reuse
BuyAction, normal budget checks, TravelMgr location validity and AH actions.
Expose a read-only indexed destination lookup by purpose and entry so this check
does not scan every vendor destination for each bot/material. Evaluate selected
missing materials only through the existing cached source value; no arbitrary
mob farming, SQL scan or currency purchase system is added.

Final hardening evidence: `BuyAction.cpp:322` BuyItem returns after its first
matching stock slot, even if that purchase fails. Source filtering must use that
first matching offer in each list; accepting a later cash offer behind an
unaffordable currency offer would repeat the same false availability. Align the
stock helper with this existing behavior, rather than changing generic buying.
BuyAction already has currency affordability checks when it runs; the demonstrated
mismatch is with VendorHasUsefulItem/travel eligibility, not a total absence of
currency support. The filter conservatively retains the normal cash-source path;
dedicated currency-offer routing remains outside this patch.

Blacksmithing bars are direct reagents; a mining source for ore is not a source
for bars. A bot with a known smelt spell may plan it while eligible, but missing
bars cannot be declared present. Vendor/AH/ordinary loot must really supply the
selected direct reagent if no producer policy exists. Empty AH supply or budget
limits are legitimate blockers, not permission to create materials.

## Inscription distinctions

- Recipe selection: subject to the same generic fairness rule as every skill.
- Pigment to ink: learned CREATE_ITEM spells already have a normal action path;
  missing pigment and grey prerequisite selection remain separate issues.
- Milling: core item-target casting exists, but autonomous demand and processing
  output/input selection are absent; no new processing subsystem is authorized
  merely by this finding.
- Ink sourcing: the generic vendor-index/AH inconsistency above is actionable.
- Parchment: ordinary direct vendor reagent under existing buying/budgets.
- Tools: the earlier patch recognized and retained tools without an acquisition
  demand. The dated one-copy continuation below now adds that demand through the
  existing vendor/AH routes; actual supply and affordability remain necessary.

## Separate investigations preserved

Generic stale TravelTarget expiry/manual reset usefulness and the reset-command
bad_alloc remain outside this patch. No live reset or speculative Engine/travel
lifecycle rewrite is performed. Rank training remains the existing normal
trainer path; no demonstrated capped-bot failure authorizes a trainer rewrite.

## Implemented changes and exact evidence

2026-10-01 Europe/Amsterdam, target: local Windows checkout only. The proposals
above were recorded before implementation; no new production subsystem was added.

| Source | Change and demonstrated root cause |
| --- | --- |
| `playerbot/strategy/actions/CastCustomSpellAction.cpp:12` | Pending autonomous nc requests can reach Execute's existing StopMoving behavior. A popped chat reaction rejected by usefulness formerly stranded pending state. |
| `playerbot/strategy/actions/CastCustomSpellAction.cpp:174` | Facing retries retain nc routing, actual GO and batch count; accepted continuations retain/refresh pending ownership. Releasing after each accepted cast formerly allowed competing batches. |
| `playerbot/strategy/actions/CastCustomSpellAction.cpp:279` | Failures/last accepted cast release pending; final acceptance finishes the fairness opportunity. Acceptance is not proof of completed effects or skill gain. |
| `playerbot/strategy/actions/CastCustomSpellAction.cpp:670` | Maintenance dispatch only tries its selected known plan, rather than falling through to randomized craft spells. Fresh learned/reagent/tool checks precede queueing; optional RPG/manual selection retains its normal fallback. |
| `playerbot/strategy/actions/CastCustomSpellAction.h` | Moves nc usefulness to the implementation for the generic pending/movement policy. |
| `playerbot/strategy/values/CraftValues.cpp:56` | Cache learned craft tools from actual Totem/TotemCategory requirements. |
| `playerbot/strategy/values/CraftValues.cpp:151` | Share direct tool/category readiness, retaining core category compatibility. Reagents alone were insufficient for a valid cast. |
| `playerbot/strategy/values/CraftValues.cpp:298` | Filter selected direct-reagent vendor sources through actual cash stock and existing hostility/destination/location rules; an unusable index hit formerly suppressed AH fallback. |
| `playerbot/strategy/values/CraftValues.cpp:433` | Recover orphan pending state after a conservative lease, protecting active real casts. Reset readiness rather than deleting a value that may currently be calculating. |
| `playerbot/strategy/values/CraftValues.cpp:525` | Core GameObjectFocusCheck validates range/map/spawn/type/ID. Fall back to the cached nearby GO list when the selected destination spawn is distant; retain WORK and CraftingFocus gates. |
| `playerbot/strategy/values/CraftValues.cpp:558` | Calculate whole-cast material demand and use available partial batches. The old demand clipped units below actual batch consumption and could require five casts before performing one. Inventory is counted once per reagent slot per calculation. |
| `playerbot/strategy/values/CraftValues.cpp:646` | Preserve all score coefficients, then apply per-runtime-skill fairness. Absolute DBC threshold scores previously allowed an indefinitely repeated winner. |
| `playerbot/strategy/values/CraftValues.cpp:662` | Maintenance readiness rechecks live inventory, learned state, tools, actual casting and focus. |
| `playerbot/strategy/values/CraftValues.h` | Registers the shared helpers, cached tool metadata and separate manual fairness state. Plan invalidation cannot erase competing skills' waiting times. |
| `playerbot/strategy/values/ProfessionCraftingFairness.h` | New small policy ledger: ready waits, bounded exact-recipe ownership, pending protection, expiry, clock rollback and readiness pruning. |
| `playerbot/strategy/values/ProfessionProgressionPolicy.h` | Pure pending/dispatch/batch/cash-vendor predicates with regression coverage. |
| `playerbot/strategy/values/ItemUsageValue.cpp:939` | Recognized learned tools use existing item usage, vendor buying and inventory retention. |
| `playerbot/strategy/values/ValueContext.h` | Register fairness/tool values through the existing context. |
| `playerbot/TravelMgr.h:465` | Read-only indexed entry lookup for source filtering. No TravelTarget lifecycle or movement changes. |
| `playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp` | Standalone C++ policy/batch/fairness regression executable and compile-time assertions. |
| `playerbot/strategy/tests/ProfessionProgressionComponentTests.py` | Extract actual production resolver/readiness/tool/vendor/index bodies and core predicates; 30 component cases with small test doubles. Temporary files stay inside the checkout and are cleaned automatically. |
| `playerbot/strategy/tests/ProfessionProgressionSourceTests.py` | Thirteen executable source wiring checks, including score preservation and absence of added sample/cheat exceptions. |
| `playerbot/strategy/tests/README.profession-progression.md` | Updated test instructions and explicit distinction between earlier validation and this patch. |
| `profession-audit/REPORT.md` | Proposals, source evidence, limits and manual validation instructions. |

## Profession comparison and remaining blockers

This continuation has no access to live runtime/DBC/DB data. The population
values below are owner-provided earlier observations, not new measurements.
Source-supported generic paths explain vulnerabilities; they do not prove that
every sampled bot had the same immediate blocker or that the patch has already
produced a natural skill gain.

| Profession | Earlier observed maximum | Source finding / correction | Remaining evidence or functionality needed |
| --- | --- | --- | --- |
| Alchemy | 25/75 | Ordinary learned CREATE_ITEM recipes use the shared craft path; pending/batch/fairness corrections apply. | Confirm sustainable real reagent supply and post-patch natural gains/cooldown/replan. |
| Enchanting | 12/75 | Existing enchanting and disenchanting actions are separate from CREATE_ITEM planning. CREATE_ITEM enchanting recipes can also be candidates. | Determine which actual casts produced those gains and whether dust/essence supply was sustainable. Population gains alone do not identify the planner path. |
| Leatherworking | 17/75 | Ordinary CREATE_ITEM path and existing gathering/vendor sources remain. | Actual known recipe/inventory/source trace; progression already shows that a global missing cast route cannot explain every profession. |
| Cooking | 55/75 | Focus WORK bridge from baseline retained; distant/invalid focus resolution hardened. Ready Cooking cannot indefinitely beat another continuously ready skill through score alone. | Trace real focus object/arrival/cast for any progressing Cook; population values alone do not prove focus travel worked. |
| Blacksmithing | 1/75 | Tools/focus/partial batches and practical direct-source fallback corrected generically. Ore is not substituted for required bars. | Real bars, known/eligible smelting or AH/loot supply; grey prerequisite demand remains absent. No source-only claim that all shortages were acquisition bugs. |
| Engineering | 2/75 | Ready competing skills gain bounded opportunities; tools, selected dispatch and focus requirements enforced. | Components/metals must really exist or be made by eligible known CREATE_ITEM recipes. Grey/nested producer demand remains absent. |
| Inscription | 1/75 | Same ready-skill fairness; parchment uses normal cash vendor path; currency-only vendor hits no longer suppress AH fallback. | Autonomous milling demand absent; pigment supply, eligible ink recipes, tools, valid vendor offers and budgets remain distinct checks. |
| Jewelcrafting | 1/75 | Same ready-skill fairness and tool/batch handling; ordinary learned CREATE_ITEM recipes retain their normal path. | Autonomous prospecting demand absent; actual ore/gems/stone/known producers and trainer state need runtime verification. |
| Tailoring | 1/75 | Ready bolt/item recipes cannot be permanently displaced by another ready skill; whole-batch material math corrected. | Grey bolts and missing cloth still require real production/acquisition; ordinary eligible bolt spells already have a shared action. |

No gathering skill values are used as proof of the new crafting dispatch.
No arbitrary drop farming, free materials, skill mutation, cheat flag, global
`rpg craft` activation or recipe-ID exception is introduced.

## Intermediate support classification

| Generic chain | Existing execution mechanism | Classification / remaining gap |
| --- | --- | --- |
| Known smelt, bolt, component or ink CREATE_ITEM spell | CraftSpellsValue -> CraftRandomItemAction -> castnc -> normal core cast | Already reachable when an eligible skill-up candidate with actual inputs/tools/focus. Gathering/training/acquisition may still block it. |
| Grey known CREATE_ITEM prerequisite | Shared action can execute it; ShouldCraftSpellValue can recognize useful output | Autonomous dependency demand is absent; the progression planner filters non-skill-up recipes. Need a separately scoped bounded producer-demand policy if this must be automated. |
| Milling/prospecting | CastCustomSpellAction/PlayerbotAI item-target casts, ItemForSpellValue compatibility, LootValues milling/prospect loot stores, StoreLootAction | Core execution/loot mechanisms exist, but autonomous demand, input eligibility/quantity and output completion are not connected. Adding them to direct reagent recipes would be incorrect. |
| Unknown producer recipe | Existing trainer system | Cannot craft until legitimately learned; no free learning added. |
| Missing direct reagent without a known producer | Existing gathering/vendor/AH/ordinary loot | Can succeed only when real exact-item supply, destination and budgets permit. Empty supply is not fake availability. |
| Multiple grey/processing dependencies | Existing actions can execute individual steps | No dependency graph exists in CraftData or RpgCraftAction. A larger planner may eventually be justified, but is avoided in this patch. |

Evidence: `CraftValues.cpp:15,167,248,567`, `RpgSubActions.h:331`,
`ItemForSpellValue.cpp:16,103`, `LootValues.cpp:111`, `LootAction.cpp:254`,
`PlayerbotAI.cpp:4257,4745`. No recursive production was implemented. It was not
needed to correct ready-skill arbitration; unresolved chains remain documented.

## Trainer/rank path

`TrainerValues.cpp:111,169` uses actual GetTrainerSpellState/known skills to
discover eligible spells and available trainers. Normal travel selects trainers;
`RpgStrategy.cpp:162` schedules `rpg train` under ordinary RPG, and
`RpgSubActions.h:226` delegates to TrainerAction. This is separate from optional
`rpg craft`. `TrainerAction.cpp:81,195,244` checks trainer offers/eligibility and
learns for free bots/authorized learning, while Learn at line9 charges the
existing spell budget unless the pre-existing free/cheat configuration permits
otherwise. This patch enables neither free mode nor cheats.

ProfessionProgression does not independently learn ranks or guarantee training
at caps. Lack of a skill-up candidate cannot itself create a trainer journey;
the existing general trainer/RPG need path must still run. Cached learned-spell
and tool values refresh normally; no new persistent learned-recipe store exists.
Without current runtime eligibility, budgets and travel evidence, no trainer
rewrite or guarantee of 75->150->...->450 is justified.

## Separate generic travel/reset issues

`TravelAction.cpp:20` calls CheckStatus. `TravelMgr.cpp:977` expires timed-out
targets; `TravelMgr.cpp:1044` IsActive reads status, and
`TravelValues.cpp:513` only returns IsActive. That bool is cached for five
seconds (`TravelValues.h:153`), but recalculating it does not perform expiration.
Thus this is not merely a permanently cached bool: status can stay active if
actions stop calling CheckStatus. `ChooseTravelTargetAction.cpp:88` rejects an
active target; `ResetTargetAction::isUseful` at line696 inherits that rejection.
An active stale target can prevent the recommended manual reset action too.

These are shared targets, so quest/vendor/AH/trainer/RPG/focus travel can all
be affected. A separate minimal correction should make expiry evaluation
independent of movement scheduling and give explicit reset its own usefulness
rule. Calling CheckStatus from value calculation needs care: its cleanup can
invalidate other values and touch strategies. No such lifecycle change was
bundled here. The new indexed lookup is read-only and does not solve this bug.

`PlayerbotAI.cpp:1447` Reset(true) clears travel/future state and at line1534
initializes all engines. That establishes possible allocation paths, not the
bad_alloc site. No core dump/journal was inspected in this local-only continuation,
and no reset was reproduced. No causal connection to professions is established;
the reset crash remains a separate unresolved issue.

## Performance and limits at roughly 1,500 bots

- Fairness runs inside the cached plan calculation, not every update. Persistent
  memory is O(ready runtime skills) per bot; temporary candidate/plan vectors are
  O(eligible recipes plus their direct reagent maps) during calculation.
- Known craft enumeration and DBC ability lookup remain existing cached work.
  Readiness now adds actual tool checks to candidate calculations; inventory
  counts are reused between partial-batch calculation and missing-demand scoring.
- Tool metadata has a ten-second cache. Material sources retain a fifteen-second
  cache. Accepted casts can invalidate plans/sources, as before; fairness state
  survives those invalidations. Source caches can lag a selection change until
  refresh, so runtime travel validation is still necessary.
- Vendor filtering evaluates indexed vendors for selected missing items, not a
  new creature/item/world DB scan. Each destination lookup is indexed; stock and
  spawn checks are proportional to those entries. Common reagents can have many
  vendors, so this extra cost still needs profiling with population load. No
  numerical throughput or memory improvement is claimed without measurement.
- No new AH scan, arbitrary mob scan, processing graph or per-tick trainer scan
  is introduced. Existing budget/cooldown/search limits stay authoritative.

Fairness uses a five-minute opportunity, not a fixed profession rotation. A
candidate is ready for aging when its actual planned batch has reagents and
required tools. Focus travel can consume its opportunity; actual casting and
pending continuations cannot be interrupted by fairness. The selected exact
recipe is held until its opportunity completes, expires or becomes invalid.
Only continuously ready competitors age; missing-material plans do not receive
fabricated readiness. The bound includes normal plan-check/scheduling delays
and queued/active batch completion, and is an opportunity rather than a promise
of skill gain. Neither server/bot identity nor the ordering of profession names
enters selection. Runtime-ID/candidate-order regression cases cover this.

## Validation record

Local continuation results:

- `python -B playerbot/strategy/tests/ProfessionProgressionSourceTests.py`:
  PASS, 13 tests. These verify source integration contracts; they do not execute
  compiled production code.
- Python `compile()` syntax checks: PASS, both Python test scripts.
- `git diff --check`: PASS, exit0; Git reports only existing autocrlf conversion
  notices, not whitespace errors.
- Current C++ regression executable: NOT RUN. No `c++`/`clang++`/`g++`/`cl`/`zig`
  compiler is available on PATH. Prepared coverage includes compile-time policy
  assertions, 1,050,000 batch combinations, repeated winner fairness, invalid
  owners/readiness, pending batches and varied arbitrary IDs/order.
- Current component executable: NOT RUN. Also needs a compatible CMaNGOS core
  checkout for real focus/category predicate extraction. Prepared coverage is
  30 production-body cases; scheduler behavior remains outside its scope.
- Existing ProfessionProgressionPolicyTests.cpp: unchanged, compiled with the
  normal PlayerBots test sources during the owner's full core build. It was not
  recompiled here. Previous audit results do not validate the latest patch.
- CMake/full mangosd build: deliberately NOT RUN. No SSH, live access, service
  operations, DB writes, commit or push occurred in this continuation.

Genericity check: added production code has no sampled names, numeric recipe
exceptions, account/GUID/realm selection or server address assumptions. Direct
item IDs, categories, focus IDs, skill IDs and spell IDs come from runtime/core
metadata. Existing baseline special CREATE_ITEM_2 handling and canary selection
are unchanged; this audit did not add individual identity logic.

## Owner manual validation commands

PlayerBots is a module, not a standalone CMaNGOS core. Use a compatible WotLK
core checkout and an isolated build directory. This checkout must be the module
source actually selected by that core; do not unknowingly build an upstream
download or the older live checkout. Preserve your normal dependency/platform
configuration options. For a core using FetchContent's `playerbots` declaration,
the explicit source override is:

```powershell
$PlayerBots = 'C:\Users\Gebruiker\Documents\GitHub\playerbots'
$Core = Read-Host 'Absolute path to your compatible CMaNGOS WotLK core checkout'
$Build = Join-Path $Core 'build-profession-audit'
$Module = Join-Path $Core 'src/modules/PlayerBots'
if (Test-Path $Module) { throw 'Use a fresh isolated core checkout; do not overwrite an existing module' }
git clone --no-hardlinks "$PlayerBots" "$Module"
git -C $PlayerBots diff --check
python -B "$PlayerBots\playerbot\strategy\tests\ProfessionProgressionSourceTests.py"
cmake -S "$Core" -B "$Build" -DBUILD_PLAYERBOTS=ON "-DFETCHCONTENT_SOURCE_DIR_PLAYERBOTS=$Module" -DFETCHCONTENT_UPDATES_DISCONNECTED=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$Build" --target mangosd --parallel 2
```

If your core integrates `src/modules/PlayerBots` directly instead, point that
module at a separate copy of these reviewed sources using your usual workflow;
FetchContent overrides do not replace a direct add_subdirectory. Verify configure
output and, where the generator provides it, compile_commands.json identifies
these edited sources. Do not use an unrelated configured build directory. On
the Linux build machine use your established thermal/offline build safeguards.
These are owner instructions, not agent-executed commands; no install/restart
commands are included.

The local clone command copies committed changes only. Include any reviewed
uncommitted changes before building if running these instructions on a dirty
checkout. The current CMaNGOS FetchContent integration calls add_subdirectory
without an explicit binary directory, so the override must be nested under the
core source tree. An external override produces a configure error; this is a
core CMake constraint, not a profession compile failure.
This core also hard-codes the canonical module header path. Use that exact
directory in the isolated core, so core and library compile against identical
class definitions; never leave a different upstream checkout on that include path.

On a C++17 compiler-equipped build machine, run the lightweight executables too:

```sh
PLAYERBOTS=/absolute/path/to/the/reviewed/playerbots
CORE=/absolute/path/to/compatible/mangos-wotlk
BUILD=/absolute/path/to/isolated/build-profession-audit
c++ -std=c++17 -DPROFESSION_POLICY_TEST_MAIN -I "$PLAYERBOTS" \
  "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp" \
  -o "$BUILD/profession-policy-tests"
"$BUILD/profession-policy-tests"
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionComponentTests.py" --core-source "$CORE"
```

## Subsequent live validation plan (owner-controlled, no deployment here)

Select any normally operating bot with at least two learned, material/tool-ready
skill-up recipes in different skills. Record private `profession` diagnostics,
selected spell, inventory, pending/casting/cooldown and travel state at normal
plan intervals. Use those actual recipe IDs as evidence only. Initially the
higher score should win; a continuously ready competitor should receive a
bounded opportunity after five minutes plus plan/batch scheduling delays.
Replanning must not flip the owned recipe every update. Observe both skills
receiving normal casts over multiple opportunities and real item/skill changes.

Repeat with arbitrary Engineering/Tailoring/Jewelcrafting plus Cooking bots;
also use Alchemy/Leatherworking/Cooking controls. For a focus recipe, require
normal travel, WORK at a real matching nearby focus, correct GO dispatch and
accepted core cast. Wrong/distant/despawned/other-map focuses must not pass. For
partial stock, expected batch consumption must equal the actual whole-cast count.
For missing tools, expect honest rejection and existing vendor behavior when
already visiting a suitable vendor, not a synthesized tool.

For direct shortages record exact-item gather/vendor/AH availability and budget.
Currency-only or rejected-location vendor offers must leave AH fallback eligible;
ordinary cash vendors must remain usable. Record empty AH supply as a real blocker.
For Inscription separately trace herb/pigment/ink/parchment/tool state: expect no
new automatic milling demand from this patch. For capped/grey producer recipes,
expect the documented planner limitation rather than a fabricated production chain.

Check final accepted cast clears pending, real active casting prevents a competing
batch, cooldown gates maintenance and next plan refresh preserves other skills'
waits. Observe orphan recovery only if it naturally occurs; do not reset bots,
give items, teleport, mutate skills or use cheats to manufacture a successful test.
Keep stale travel and reset-crash observations separate from profession results.

## Earlier pre-publication Git snapshot (superseded)

Branch: `feature/playerbot-profession-economy`.
HEAD: `c0100429a9b80b3b571d663399ac8a278d91cfbf`.
Nothing staged, committed or pushed. Tracked diff:
`8 files changed, 337 insertions(+), 40 deletions(-)`.
Six new untracked files are additional to that tracked stat (14 files in total).

Exact `git status --porcelain=v1 --untracked-files=all`:

```text
 M playerbot/TravelMgr.h
 M playerbot/strategy/actions/CastCustomSpellAction.cpp
 M playerbot/strategy/actions/CastCustomSpellAction.h
 M playerbot/strategy/values/CraftValues.cpp
 M playerbot/strategy/values/CraftValues.h
 M playerbot/strategy/values/ItemUsageValue.cpp
 M playerbot/strategy/values/ProfessionProgressionPolicy.h
 M playerbot/strategy/values/ValueContext.h
?? playerbot/strategy/tests/ProfessionProgressionComponentTests.py
?? playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp
?? playerbot/strategy/tests/ProfessionProgressionSourceTests.py
?? playerbot/strategy/tests/README.profession-progression.md
?? playerbot/strategy/values/ProfessionCraftingFairness.h
?? profession-audit/REPORT.md
```

## 2026-10-01: one-copy tool acquisition continuation (before source edits)

Fork feature HEAD and origin/feature/playerbot-profession-economy both resolve to
ff7756216c2e332d0eea806305a19afa7e1c8eca after fetch (0 ahead / 0 behind).
The existing two uncommitted test/documentation files are preserved.
The successful two-job build and current live observation are recorded in
C:/Users/Gebruiker/codex-homelab/profession-audit/POSTBUILD-VALIDATION.md.

Demonstrated generic cause: CraftToolRequirementsValue reads actual learned
recipe Totem/TotemCategory requirements, HasRequiredTools rejects missing tools,
and ItemUsage recognizes them when already at a vendor. However GetMaterialSources,
the bounded BuyAction profession pass and AhAction profession pass use only
selected direct reagents. A missing tool cannot request an acquisition trip.
The generic ItemUsage stack rule also asks for one full stack and treats every
compatible tool as needed even when another owned tool satisfies the category.

Smallest proposed correction: add a shared item-category index (one metadata
scan for the realm lifetime), plus a cached per-bot selection of missing learned
recipe tools. Exact Totem items require one copy; TotemCategory accepts the core
compatibility predicate, including upgraded/superset tools. Choose a practical
cash-vendor alternative when available, otherwise a bounded compatible-item AH
fallback. Reuse the existing material sources, profession vendor/AH requests,
BuyAction and AhAction. Recheck live inventory/category satisfaction before buying;
request quantity is one, independent of craft batch or MaterialTarget. ItemUsage
keeps owned usable tools and stops requesting interchangeable duplicates.
A feasible tool vendor request takes priority over a new crafting-focus trip,
using existing travel requests; no target reset or travel lifecycle rewrite.
Tool demands come from all currently known craft spells, so a losing profession
can acquire its prerequisite without waiting to win the recipe score first.

Affected files: CraftValues.h/.cpp, SharedValueContext.h, ValueContext.h,
ItemUsageValue.cpp, BuyAction.cpp, AhAction.cpp, ProfessionStatusAction.cpp and
existing source/component regression suites. Keep recipe scoring unchanged.

Performance: one shared item metadata scan; subsequent bot work considers only
cached learned requirements and the small matching-tool set, plus existing
indexed vendor destinations. Cache tool choice for30 seconds; live ownership
rechecks avoid stale duplicate purchases. No per-tick item/DBC/world scan,
recursive production planner, free recipe/item, bot/profession/recipe exception,
or additional AH scan policy is introduced.

Limits: buying an inking set does not produce pigments or ink. Milling/prospecting
still lack demand-driven processing; upgraded crafted tools without vendor/AH
supply can still require production dependencies. Existing RPG crafting can
craft useful known tools, including grey recipes, but this proposal does not
silently add recursive arbitrary production. Jewelcrafting's observed ready
ring recipes need no tool/focus, so its pending-request blocker is separate.
The live continuously renewed bandage request is not claimed fixed by this work.

Coverage: missing exact tools, one-copy demand, owned and upgraded compatible
tools, category alternatives, newly learned advanced requirements, no-vendor
fallback, no tool/reagent demand collision, unchanged score and shared-action
wiring. Source checks run locally; lightweight C++ component execution requires
a compiler, and is not a substitute for the full owner CMaNGOS build.

## 2026-10-01: renewed pending lease — evidence before editing

Live observations show the same pending recipe repeatedly receiving a newer
queued-at marker without a newer accepted-craft marker. Current source confirms
QueuePendingCraft always overwrites the timestamp. CastCustomSpellAction calls
it from both the facing retry (no accepted cast) and an accepted batch
continuation. Thus retries can perpetually renew the recovery deadline. Pending
ownership in CraftingFairness::Select then defers other ready skills regardless
of the five-minute opportunity. The exact live renewal branch was not logged;
this proves an unbounded source path, not that every observed stall uses it.

Smallest fix: QueuePendingCraft preserves an existing same-spell lease on a
retry. A new request starts a lease; an accepted continuation explicitly renews
it. Existing actual-casting protection, normal spell checks, nc routing, cleanup
and shared action remain authoritative. No bot/recipe/profession exception,
Engine rewrite or travel reset. Cost is one manual-value comparison per queue
operation; no recipe/item/world scan or added per-bot cache. Regression cases:
same request retry does not renew, accepted continuation does, new spell/new
request starts ownership; production QueuePendingCraft exercised directly.
This removes one mechanism that can block ready Jewelcrafting and other skills;
it does not prove live completion or solve unrelated casting/travel failures.

### Tool quantity at the Auction House: source evidence before editing

GetMissingTools requests one copy, but AhBidAction passes the configured
MaterialTarget to IsReasonableAuctionStack. That permits a whole reserve stack
even when a nonconsumable tool needs only one. Reuse the existing auction capacity
policy with reserve target one for tool-only supplies; preserve MaterialTarget
for actual recipe reagents, including an item used both as a tool and reagent.
Apply the same rule at candidate filtering and before purchase. No auction scan,
new purchasing action or additional inventory lookup is needed. Regression
assertions cover one-copy tools, unchanged reagent reserves and dual-use items.

## Current implemented fixes and validation

- CraftValues.h/.cpp: cached learned craft/enchant tool requirements, core
  compatible ownership checks, shared item-category index, cached alternative
  selection, one-copy missing tools merged with direct reagents, and preservation
  of the same-spell retry lease. The accepted continuation explicitly renews it.
- SharedValueContext.h / ValueContext.h: register the shared metadata index and
  per-bot 30-second purchase selection through existing value infrastructure.
- ItemUsageValue.cpp: retain owned tools, use one-copy readiness rather than a
  reagent stack, and stop requesting an alternative after category satisfaction.
- BuyAction.cpp / AhAction.cpp: consume the combined missing supplies through
  existing bounded normal purchases. Tool-only auctions have a reserve of one;
  consumable and dual-use reagent reserves retain their configured behavior.
- CastCustomSpellAction.cpp / ProfessionProgressionPolicy.h: accepted casts
  renew continuation ownership; facing/requeued requests preserve lease age.
  Core checks, real spell execution and actual-casting protection remain intact.
- ProfessionStatusAction.cpp: privately show missing tools and combined source
  diagnostics. A tool prerequisite can be diagnosed separately from pigments/ink.
- The three regression suites, test README and this report document and cover
  the changes. The pre-existing uncommitted source-test baseline hardening and
  report updates were preserved.

Validation of current sources:

```text
Python source checks: PASS, 18 tests
C++ policy/regression: PASS, including 1,050,000 batch cases
Production-body components: PASS, 54 cases; core focus/category predicates
git diff --check: PASS
Current tool/retry patch full mangosd build: NOT RUN
Current tool/retry patch live validation: NOT RUN
```

Lightweight C++ execution only used copied test inputs in
/srv/cmangos/audit/profession-tool-tests-mi4mjjyz and read the compatible isolated
core predicates. It did not change the original deployment checkout, binary,
services, configuration, database or running bots. Source changes remain local,
uncommitted and unpushed. Component doubles do not establish scheduler behavior,
translation-unit integration or natural skill gain.

### Working versus still blocked

The earlier installed patch produced directly observed Engineering skill gain
and persisted gains in Blacksmithing and Tailoring; their original skill-1
population result is therefore historical, not the current universal state.
The later read-only snapshot still showed all sampled Inscription and
Jewelcrafting records at1. Those professions need independent validation:

| Path | Current source result | Remaining evidence/blocker |
| --- | --- | --- |
| Any learned tool/category | One-copy legitimate vendor/AH demand, including advanced craft/enchant requirements, independent of the shared winning recipe | Gold, practical destinations, stock, selected AH alternative and real arrival are still required. Crafted upgrades can lack supply. |
| Jewelcrafting ready direct crafts | Generic nonprogressing retry lease now has a bounded recovery deadline | Sampled ring recipes needed no tool/focus. The exact live renewal branch was not logged; real cast/skill gain remains unproven after this edit. |
| Inscription tools/parchment | Tools use the new common supply path; parchment retains normal reagent buying | An inking set does not supply pigment or ink. Milling demand and grey ink-production dependency are still absent. |
| Blacksmithing/Engineering/Tailoring direct inputs | Existing eligible learned smelt/bolt/component recipes and direct gathering/vendor/AH sources remain authoritative | Ore is not a bar, cloth is not a bolt. Grey/nested prerequisite demand and unavailable AH supply remain real blockers. |
| Other skills competing with Cooking | Existing runtime-skill aging/ownership fairness is unchanged; retry loops can no longer extend one lease indefinitely | Fairness grants a bounded opportunity, not a guaranteed cast. Active casts, travel, missing inputs and core rejection can still prevent progress. |

No recursive production was added. Existing smelt/bolt/component/ink CREATE_ITEM
execution is reusable; demand for grey prerequisites is not modeled. Milling and
prospecting have core item-processing/loot mechanisms but need a separately
designed demand/eligibility/completion bridge. This remains outside these fixes.
Trainer/rank paths, stale travel/reset-target lifecycle, and the original reset
bad_alloc remain as separately documented findings, without speculative changes.

### Fairness and performance

Recipe scores are unchanged. The existing per-runtime-skill ready aging and
five-minute bounded opportunity remain intact; accepted batch continuation owns
its pending request. A same-spell retry now preserves its deadline so it cannot
indefinitely defeat recovery. No fixed profession rotation or identity exception.

New costs: one realm-shared item-category metadata scan, a small per-bot set of
chosen tools cached for30 seconds, and live checks of cached learned requirements
and ownership. Vendor checks use existing item/destination indexes; they can
visit matching vendor points during a selection refresh, not all world spawns
per AI tick. Requirement enumeration uses existing cached craft/enchant spells.
AH scanning retains its existing cooldown, budget and purchase limits. There is
no measured 1,500-bot benchmark for this new patch; cache interval and indexed
work bound frequency but do not prove a latency target.

### Genericity and remaining risks

Added production branches derive exact tools/categories/compatible alternatives
from spell/item metadata and inventory. No sampled names, GUID/account/realm
checks, recipe exceptions, cheats or fabricated availability were added.
The baseline canary hash and legacy special spell handling are unchanged.
Tools in the bank do not satisfy carried-tool requirements. Known advanced
requirements refresh through normal value caches, rather than free training.

The tool-choice AH fallback currently chooses one compatible metadata item,
without searching all interchangeable alternatives for current auction supply.
Normal purchases require money and actual stock; a positive tradeskill budget
is not proof that every requested tool is affordable. Tool trips can compete
with focus travel, and a stale active travel target can still suppress either.
Pending lease expiry releases the marker; it does not cancel every already
queued generic chat command. End-to-end continuation ordering and acquisition
remain live-validation requirements. No guarantee of arbitrary advanced chains.

### Specific next validation

After a reviewed isolated full build, use existing read-only `profession`,
`spells`, item-count and travel/debug diagnostics on any ordinary random bots:

1. Missing tool: expect a private tool x1 diagnostic and vendor/AH source request.
   Observe normal payment, one tool in inventory, live satisfaction and no
   duplicate request. Repeat with an already-owned compatible upgraded tool;
   expect no missing-tool demand. Use any known advanced recipe as another case.
2. Jewelcrafting control: choose a bot with a known, reagent-complete direct
   recipe. A nonprogressing retry must preserve queued-at; accepted continuation
   may advance it. If no cast is active, expect expired pending to clear after
   max(120, expireActionTime/1000+60) seconds. Then observe bounded competition,
   actual cast, item/skill change, cleanup and replan. Do not reset to create it.
3. Inscription: record tool, parchment, pigment and ink separately. If tool
   acquisition succeeds but pigment/ink remains absent, report the production
   limitation honestly; do not infer that buying a tool fixed milling.
4. Blacksmithing/Engineering/Tailoring: repeat normal direct-craft checks with
   real input inventory and eligible known producers. Record grey/nested missing
   chains separately. Keep expired travel observations separate from cast results.

### Manual full-build validation

Stage these reviewed uncommitted sources in an isolated compatible core first.
Git clone/fetch alone will omit the current working-tree edits. The canonical
src/modules/PlayerBots header path and FetchContent source must resolve to the
same staged module, as the previous header-layout failure proved. With that
prerequisite satisfied, the existing isolated build paths are:

```sh
CORE=/srv/cmangos/audit/profession-20261001/source
PLAYERBOTS="$CORE/src/modules/PlayerBots"
BUILD=/srv/cmangos/audit/profession-20261001/build
git -C "$PLAYERBOTS" diff --check
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/ProfessionProgressionSourceTests.py"
cmake -S "$CORE" -B "$BUILD" -DBUILD_PLAYERBOTS=ON \
  -DFETCHCONTENT_SOURCE_DIR_PLAYERBOTS="$PLAYERBOTS" \
  -DFETCHCONTENT_UPDATES_DISCONNECTED=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD" --target mangosd --parallel 2
```

Preserve the verified cache/dependency options and normal offline/thermal build
procedure. These commands validate sources; they contain no install or restart.

### Current Git snapshot

Branch: feature/playerbot-profession-economy.
HEAD: ff7756216c2e332d0eea806305a19afa7e1c8eca.
Fetched origin feature: same commit, 0 ahead / 0 behind. All15 modified files
are unstaged; there are no untracked files, commits or pushes from this continuation.
The first source-test/report modifications predated this continuation.

```text
 M playerbot/strategy/actions/AhAction.cpp
 M playerbot/strategy/actions/BuyAction.cpp
 M playerbot/strategy/actions/CastCustomSpellAction.cpp
 M playerbot/strategy/actions/ProfessionStatusAction.cpp
 M playerbot/strategy/tests/ProfessionProgressionComponentTests.py
 M playerbot/strategy/tests/ProfessionProgressionRegressionTests.cpp
 M playerbot/strategy/tests/ProfessionProgressionSourceTests.py
 M playerbot/strategy/tests/README.profession-progression.md
 M playerbot/strategy/values/CraftValues.cpp
 M playerbot/strategy/values/CraftValues.h
 M playerbot/strategy/values/ItemUsageValue.cpp
 M playerbot/strategy/values/ProfessionProgressionPolicy.h
 M playerbot/strategy/values/SharedValueContext.h
 M playerbot/strategy/values/ValueContext.h
 M profession-audit/REPORT.md
```

## Owner-confirmed full build result — 2026-10-01 17:05 +02:00

The owner supplied the final background job output: the incremental CMake build
completed37/37 with Linking CXX executable src/mangosd/mangosd and job exit-code0.
The log includes CraftValues.cpp, ItemUsageValue.cpp and both existing
ProfessionProgressionPolicyTests.cpp and ProfessionProgressionRegressionTests.cpp
translation units, followed by the PlayerBots library and mangosd link.
This supersedes the earlier NOT RUN full-build status for the tool/retry patch.
37 is the incremental dependency count, not the previous clean-build848 count.
No new remote inspection was performed to confirm this owner-provided output.
The job was build-only: the newly compiled binary has not been installed by it,
and these new fixes are not claimed active or validated on live bots.

## Owner-authorized installation — 2026-10-01 17:10 +02:00

Read-only inspection proved the successful new build (SHA256
6c3ffb4c878a5080caba1a0563519e695d7b6a159a267175a552c08178c2d5e9)
was not installed; the installed and running binary still had the earlier hash.
At the owner's explicit request it was installed using the updater lock, normal
graceful service stops, a previous-binary backup, atomic replacement and rollback
if startup verification failed. Installation completed with exit0 at17:09:31 CEST.
Build, installed and running-process hashes now match. Both world/realm services
are active/running and TCP8085/3724 listen. The original live source checkout and
PlayerBots configuration are unchanged; no commit, push or DB mutation occurred.
Backup and deployment evidence:
/srv/cmangos/audit/profession-tool-install-20261001-170801/backup/mangosd,
operation.log, verified.sha256 and exit-code in that directory.
This supersedes the preceding not-installed status. Actual autonomous tool
acquisition, cast completion and natural skill gain still require live observation.

## Live observation and next source-proven corrections — 2026-10-01

An explicitly READ ONLY population/diagnostic window17:20:55 to17:28:48 captured
new persisted gains in5 Blacksmiths,4 Engineers,1 Alchemist,16 Leatherworkers,
2 Enchanters and82 Cooks. No sampled Inscription gain;15/15 remain1. Jewelcrafting
is now2 on one of four bots but did not gain further within this window. These
counts span an unexpected crash and delayed saves; they do not prove a clean
eight-minute run or attribute every gain to this patch. Full private evidence
is in the owner's homelab profession-audit/LIVE-TOOLS-20261001.md.

The one-copy tool diagnostic/source bridge is live. Sampled scribes request an
inking set x1; cash vendors exist. Its DB base cost is750 copper, while two sampled
bots have only148/155. Another has1447 but has an expired quest travel target.
No purchase was observed. Pigment/ink is absent, ink vendor stock is currency-only
and the checked AH supply is empty. Buying a tool alone cannot close that chain.

### Generic travel expiry — demonstrated cause and proposal before editing

Multiple live TRAVEL/WORK/COOLDOWN targets remain marked active hundreds of seconds
past deadline. TravelTargetActiveValue calls IsActive, which only checks the
status enum. The normal expiry branch lives inside CheckStatus and therefore
depends on movement/travel execution. TravelActionMultiplier then suppresses
new requests, including legitimate tool acquisition. This is generic across
quest/vendor/AH/trainer/focus purposes, not a profession-specific rule.

Smallest correction: make IsActive expire an elapsed, timed, nonforced target
using the same SetStatus/failed-destination invalidation already used by
CheckStatus; reuse that path rather than adding a tick or destination scan.
Preserve forced destinations, zero/unlimited timers and already inactive states.
Keep CheckStatus's group/destination/arrival logic where it is. Correct reset
action eligibility separately if source proves active-state rejection prevents
its manual purpose; do not run a live reset. Tests must execute the production
methods with clock/status/context doubles, covering all phases, forced targets,
no timer, cache read expiry, cleanup once and reset safety conditions.
Cost: constant-time status/deadline checks on existing reads, no world scan or
new per-bot cache. Stage this generic travel change in a separate commit.

### Empty-chat abort — demonstrated cause and proposal before editing

The new core records SIGABRT in HandleBotOutgoingPacket's cold path, called by
WorldSession::SendPacket. The matching installed binary's unique abort branch
is preceded by fprintf arguments naming `!message.empty()` and
HandleBotOutgoingPacket; source has exactly that assertion before QueueChatResponse.
Thus empty parsed chat can terminate the whole server. This is not the earlier
reset bad_alloc, and the source of the empty text is not proven.

Smallest correction: ignore an empty parsed chat payload before message recording,
reply selection and response queueing. Normal nonempty packet handling remains.
Replace the fatal assumption with input validation; no catch-all exception mask,
profession/name/packet-ID exception or free craft. Regression must exercise
the same production guard and prove an empty message cannot reach the response
path while nonempty text still can. No extra scan/cache; one string emptiness
check per relevant packet. Keep this fix in a separate chat-handling commit.

### Priority finding

Craft dispatch1.2 is above quest accept1.08/turn-in1.09 but below attack anything5.
Profession travel requests6.965-6.99 rank above ordinary quest travel6.3; active
target and normal multipliers still gate them. Recipe scores do not arbitrate
combat/quest actions. The short interrupted observation does not justify changing
these base priorities; fix stale target lifecycle and known packet failure first.

### Implemented lifecycle corrections and reset scope

Expiry now runs once from TravelTarget::IsActive, retaining forced/unlimited and
inactive states. CheckStatus reuses that path through its existing opening call.
ResetTargetAction itself is deliberately unchanged: TravelStrategy automatically
schedules it at a nearby quest taker as well as when inactive. Removing its
active-target gate globally would repeatedly discard healthy targets. Expired
targets now become inactive and can pass the existing reset gate naturally.
The diagnostic reset recommendation is still unsuitable for a healthy active
target; no full AI reset or live reset was used.

Empty parsed chat is ignored before recording, response and LLM paths; the fatal
assertion is removed. Separate production-body C++ fixtures cover these two
generic fixes. No profession priorities, recipe identifiers or sampled identities
were introduced.

## 2026-10-02 — local configuration and production-demand follow-up

Scope: local checkout only; no live inspection, deployment, commit or full build.
The previously recorded Inscription inventory is historical evidence, not a
claim about current online bots. Participation remains 10% by default.

### Demonstrated root causes and proposed bounded correction

`ProfessionCraftingPlanValue::Calculate` considers only skill-up item creation
spells. It neither follows a missing reagent to another learned creation spell
nor represents item-targeted Milling/Prospecting. Thus a scroll's missing ink
does not request learned ink creation; missing pigment does not request Milling.
Grey ink/bolt/bar recipes are also excluded even when needed by a useful recipe.
`ItemForSpellValue` chooses arbitrary fitting inventory items without checking
processing stack size, flags or skill rank. Reusing that selection blindly would
target the wrong herbs/ores. Processing produces real loot, not EffectItemType.

Smallest proposed bridge: expand demand by at most two prerequisite steps, using
only known ordinary creation recipes and known processing effects. Infer possible
processing inputs from the existing loot-template access and item flags. Reuse
normal `craft random item`, `castnc`, core spell checks/casts and `store loot`.
No arbitrary-depth production planner, guaranteed-output assumption, recipe ID
exception, injected item or profession skill mutation. Conditional/reference-only
loot sources are conservatively unsupported by the initial metadata index.

Preserve root recipe scoring and existing bounded fairness, but give a useful
recipe a materially ready prerequisite when available. Keep the root identity
through prerequisite changes. Bound processing to one real cast and keep its
owned input/request through the actual loot response/release before replanning.
Reserve demanded outputs so normal item usage does not sell or reject them.
Absent herbs, money, tools, learned recipes or reachable sources remain real
blockers; the bridge does not promise to solve those world/economy conditions.

Performance: build a realm-shared processing input/output index once, alongside
existing shared item/loot indices. Expand only the bot's cached known recipes at
the existing plan interval, with a strict depth bound; cache inventory counts
within that calculation. No full item/spawn/DBC scan per bot or AI update.

Configuration: publish flat `AiPlayerbot.Profession...` options matching nearby
PlayerBots settings, replace user-facing rollout terminology with participation,
and keep the percentage default at 10. Read old dotted settings as fallback so
existing configured budgets/intervals/participation survive migration; the new
setting takes precedence. The old percentage spelling remains only as a legacy
configuration input. Test defaults, overrides, bounds, production chains, cycles,
processing ownership/cleanup and the existing policy/focus/tool regressions.

### Implemented behavior and evidence

The bounded bridge is implemented locally. `CraftValues.cpp` indexes only known
ordinary producers, including grey recipes, and known WotLK processing effects.
`ProfessionProduction.h::NextProductionStep` follows at most two prerequisite
edges, detects repeated spell identities and requests one processing cast at a
time. Ordinary prerequisite batches obey the existing batch/material bounds and
use partial available inputs. A processing output is a possible loot outcome;
selection and accepted casts never credit that output to inventory.

The normal shared craft action dispatches the selected execution spell. Its
ordinary skill-up filter is waived only for a selected demanded prerequisite;
known-spell, reagent, tool and exact focus checks remain in place. Processing
targets are actual owned bag stacks with matching item flags, skill rank,
metadata-derived quantity and no temporary loot; trade targets are rejected.
The core's normal CanCastSpell/CastSpell checks still decide actual success.

A manual `profession craft request` stores the goal/step snapshot and selected
input GUID. This GUID identifies the runtime inventory object, never a special
bot identity. Pending processing stays owned until `StoreLootAction` handles the
matching real Milling/Prospecting loot and releases it normally. Failure or the
existing bounded pending timeout resets the request and plan/source caches.
Unrelated loot and unaccepted requests cannot complete the step. Planned output
ingredients stay visible to existing item usage/retention during loot handling.

The previous historical WotLK evidence shows learned Ivory Ink needs Alabaster
Pigment, while the learned scrolls need Ivory Ink and parchment. The old source
had no connecting production path. The compiled planner fixture now proves the
same *generic metadata shape*: useful final recipe -> grey learned producer ->
known Milling, then actual observed pigment -> partial ink batch -> final recipe.
It also proves unknown recipes are not invented and missing herbs remain a real
acquisition request. This is source/component validation, not live skill evidence.

### Cross-profession scope and remaining blockers

| Path | Local change | Remaining world/data requirement |
| --- | --- | --- |
| Inscription | Milling/pigment/learned ink can satisfy useful recipe demand | Compatible herb stack, learned Milling/ink, tools, parchment, cash and normal loot response |
| Jewelcrafting | Known Prospecting can supply a demanded gem; normal owned-item cast/loot | Prospectable ore, sufficient JC skill, known spell and a real probabilistic gem result |
| Tailoring | Learned bolts remain eligible as demanded prerequisites after becoming grey | Cloth, learned bolt recipe, thread and tools where metadata requires them |
| Engineering | Learned components can be demanded prerequisites | Raw bars/materials, tools/focus; chains deeper than two edges remain unsupported |
| Blacksmithing | A known bar-producing recipe can supply missing bars | Learned smelting/mining conditions, ore and real forge; no ore/bar substitution or fake availability |
| Alchemy / Leatherworking / Enchanting / Cooking | Existing direct path retained; same score formula and fairness | Existing acquisition, focus/tool/trainer and natural core skill-gain rules still apply |

This does not provide arbitrary mob-drop farming, learn recipes for free, fund
vendors/AH, bypass rank training or guarantee rare processing outputs. Loot
sources whose only relevant output is conditional/reference-based are not added
to the conservative shared index; direct acquisition/AH remains available.
Processing stack size uses spell base metadata, with the core checking final
cast requirements; custom spell modifiers still require integration validation.

### Fairness and performance

The existing five-minute bounded ready-skill opportunity remains unchanged.
Candidates retain their useful root spell/skill identity while the execution
step changes. Accepting a prerequisite does not finish the root opportunity;
finishing the final batch does. Pending requests prevent competing continuations.
Recipe value scoring is preserved; material readiness now reflects the next real
production step. No profession-name rotation or sampled recipe exception exists.

Fairness ages ready candidates. A profession with no viable ready input can
still lose to a ready profession: this change does not assert that scarce-material
acquisition is fair or sufficient. Expanding arbitration to blocked acquisition
would be a separate policy change requiring evidence, not an implicit part of
the Inscription fix.

For 1,500 bots, the only full item/processing-loot scan is realm-shared and runs
once. Per-plan work uses cached ordinary recipes plus one known-spell enumeration
for processing, an output-key lookup restricted to demanded known reagents, and
memoized inventory counts. Depth is at most two prerequisite edges. The existing
CalculatedValue interval of 60 uses its inherited half-interval semantics (~30s)
and accepted-cast invalidation; no new per-tick world scan or timer is introduced.
There is no production load benchmark in this local-only task. Processing target
validation scans bag inventory, and full population profiling remains advisable.

### Changed files

- `PlayerbotAIConfig.cpp/.h`, `aiplayerbot.conf.dist.in`: canonical flat options,
  legacy fallback, bounded 10% default and renamed percentage member.
- `ProfessionProgressionPolicy.h`, `ProfessionProgressionPolicyTests.cpp`:
  participation terminology, unchanged stable selection policy.
- `CraftValues.cpp/.h`, new `ProfessionProduction.h`: bounded production demand,
  shared processing metadata, live target validation and owned request lifecycle.
- `ValueContext.h`, `SharedValueContext.h`: register request and shared index.
- `CastCustomSpellAction.cpp`: selected prerequisite dispatch, owned processing
  item, one cast/loot lifecycle and root opportunity preservation.
- `LootAction.cpp`: complete only processing requests after normal loot release.
- `ItemUsageValue.cpp`: retain demanded intermediate and goal ingredients.
- `ProfessionStatusAction.cpp`: participation, prerequisite goal and input details.
- `ProfessionProgressionSourceTests.py`, new `ProfessionProductionTests.py`:
  integration-boundary and compiled production-body regressions.
- `profession-audit/REPORT.md`: evidence, proposal, implementation and limits.

### Validation on the Windows workstation

- `git diff --check`: PASS.
- Standalone policy regression: PASS, including 1,050,000 quantity combinations
  and existing fairness/pending policy cases.
- Static policy assertions: PASS using actual policy code and extracted route
  constants, with core-dependent includes replaced by a local declaration shim.
- Existing component fixture: PASS, 54 focus/tool/vendor/index cases, with the
  upstream core focus/category predicate bodies.
- New production fixture: PASS, 2,087 cases. Includes the actual whole planner,
  processing index/target/request bodies, post-cast cleanup block and all actual
  configuration assignments. Doubles supply world state, not cast skill gains.
- Source wiring: PASS, 23 cases, including normal cast/loot integration,
  participation options and absence of new sampled identity/recipe exceptions.
- Existing separate travel lifecycle / outgoing chat fixtures: PASS, 59 / 6.

Portable official Zig 0.13.0 is stored only under `.git/profession-test-tools`,
with its archive SHA256 verified against the official download index. No global
compiler installation was made. Reference core headers under `.git` are upstream
WotLK sources, not proof of binary compatibility with the deployment's exact core.
Primary references checked: [CMaNGOS item/temporary-loot API](https://github.com/cmangos/mangos-wotlk/blob/master/src/game/Entities/Item.h),
[CMaNGOS loot metadata](https://github.com/cmangos/mangos-wotlk/blob/master/src/game/Loot/LootMgr.h)
and the [official portable compiler manifest](https://ziglang.org/download/index.json).
No CMake, full mangosd build, SSH, deployment, database mutation, restart, commit
or push was performed. Existing bad_alloc/travel findings are not expanded here.

Local rerun (portable compiler already present, PowerShell):

```powershell
Set-Location C:\Users\Gebruiker\Documents\GitHub\playerbots
$Compiler = (Resolve-Path .git/profession-test-tools/cxx.cmd).Path
git diff --check
python -B playerbot/strategy/tests/ProfessionProgressionSourceTests.py
python -B playerbot/strategy/tests/ProfessionProductionTests.py --compiler $Compiler
python -B playerbot/strategy/tests/ProfessionProgressionComponentTests.py --compiler $Compiler --core-source .git/profession-core-reference
```

### Manual full build and later live validation

First stage these exact local sources into the existing isolated module yourself;
the current server/GitHub do not contain this uncommitted patch. Verify the module
include alias and FetchContent path resolve to the same staged source. These are
build-only commands for that isolated layout, preserving its existing cache and
normal offline/thermal procedure; no install/restart command is included:

```sh
CORE=/srv/cmangos/audit/profession-20261001/source
MODULE="$CORE/src/modules/PlayerBotsDeploy"
BUILD=/srv/cmangos/audit/profession-20261001/build
git -C "$MODULE" diff --check
readlink -f "$CORE/src/modules/PlayerBots"
readlink -f "$MODULE"
cmake -S "$CORE" -B "$BUILD" -DBUILD_PLAYERBOTS=ON \
  -DFETCHCONTENT_SOURCE_DIR_PLAYERBOTS="$MODULE" \
  -DFETCHCONTENT_UPDATES_DISCONNECTED=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$BUILD" --target mangosd --parallel 2
```

After separately authorized deployment, retain 10% participation and pick any
normal free scribe reported active by `rndbot do <bot> profession`. Record known
recipes, tool/skill, inventory, selected goal/step and acquisition/travel state.
With a legitimate compatible herb stack, expect one Milling cast, real herb
consumption and actual pigment loot; only that inventory observation should lead
to learned ink creation, then a normal useful final cast and natural skill gain.
When pigment is already present, expect Milling to be skipped. Repeat after ink
becomes grey but a useful final recipe remains. Observe another ready profession
across five-minute opportunities. Test a JC bot's compatible ore/gem path, and
grey learned bolts/bars/components with actual inputs. No giving items, reset,
teleport, skill manipulation or recipe-ID exception is part of validation.

The implementation snapshot above retains branch/HEAD `feature/playerbot-profession-economy` /
`bce49806a05edb6dfc0c7b37d89a9dd44907f841`. All edits are local and unstaged:
15 modified tracked files listed above, plus the two new source/test files.
Live 10% configuration and installed binary were untouched in this task.

### Configuration guide follow-up

Added [the user-facing configuration guide](../docs/PROFESSION_PROGRESSION.md)
and linked it from README and the distributed configuration. It explains all 12
options/defaults, loader bounds, stable 10% participation, inherited half-interval
cache behavior, cast/item quantity distinctions, tools, real vendor/AH budgets,
buying-disable options, legacy precedence and current production/fairness limits.
Comments were clarified without changing runtime behavior. Documentation was
checked against all 12 current option names/defaults and the actual loader/buying
paths; whitespace/link checks and `git diff --check` passed. No build/deployment,
commit, push or live configuration change was involved.

### Owner-authorized GitHub publication

The owner subsequently authorized committing and pushing this reviewed local
implementation and documentation to the fork's existing
`feature/playerbot-profession-economy` branch. A fresh fetch confirmed the remote
and local starting revision both remained `bce49806` (0 ahead / 0 behind).
The publication includes the configuration guide, source/regression changes,
updated regression instructions and this audit record. Publication does not
constitute a full build or live validation, and does not change fork master,
the closed upstream PR, live sources/configuration, binaries or services.
The resulting commit and verified remote parity are recorded in the operator's
operational memory after pushing.
