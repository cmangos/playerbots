# Profession progression: local source review

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
- Tools: existing generic learned-tool readiness/retention is retained. A missing
  tool alone still has no dedicated acquisition/travel demand; do not claim that
  recognizing a tool manufactures or guarantees it.

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
git -C $PlayerBots diff --check
python -B "$PlayerBots\playerbot\strategy\tests\ProfessionProgressionSourceTests.py"
cmake -S "$Core" -B "$Build" -DBUILD_PLAYERBOTS=ON "-DFETCHCONTENT_SOURCE_DIR_PLAYERBOTS=$PlayerBots" -DFETCHCONTENT_UPDATES_DISCONNECTED=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$Build" --target mangosd --parallel 5
```

If your core integrates `src/modules/PlayerBots` directly instead, point that
module at a separate copy of these reviewed sources using your usual workflow;
FetchContent overrides do not replace a direct add_subdirectory. Verify configure
output and, where the generator provides it, compile_commands.json identifies
these edited sources. Do not use an unrelated configured build directory. On
the Linux build machine use your established thermal/offline build safeguards.
These are owner instructions, not agent-executed commands; no install/restart
commands are included.

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

## Final Git snapshot

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
