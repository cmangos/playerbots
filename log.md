# Profession economy implementation log

Last updated: 2026-09-30 (Europe/Amsterdam)

## Repository checkpoint

- Local branch: `feature/playerbot-profession-economy`
- Base commit: `99e6f15eb154bec2c8602e8b425fef87e67de501`
- Current source tip preceding the final GitHub handoff update:
  `537e4a97` (`playerbots: revalidate cached profession material needs`)
- Official upstream: `upstream` -> `https://github.com/cmangos/playerbots.git`
- Writable fork: `origin` -> `https://github.com/goakiller900/playerbots.git`
- Remote branch: `origin/feature/playerbot-profession-economy`
- Pull request: <https://github.com/goakiller900/playerbots/pull/1>
- The pull request targets the writable fork's `master` branch and is intentionally
  left unmerged.

Do not add or clean the untracked `.core-reference/` and `.validation-tools/`
directories. They predate this feature work. `.core-reference/` contains unrelated
local server changes, and `.validation-tools/` contains local compilers, headers,
and generated validation object files.

## Completed and committed

Commit `b2a02625` (`playerbots: preserve and recognize professions correctly`):

- Stops the random-bot refresh path from deleting assigned profession skills that
  are still at skill 1.
- Reconciles persisted profession assignments with skills already stored on the
  character.
- Writes repaired assignment metadata back even when both old IDs were nonzero,
  so skill-1 cleanup uses the corrected professions.
- Preserves earned profession skill and rank instead of synthesizing skill from
  character level.
- Recognizes and assigns Inscription on WotLK builds.

Commit `3105524e` (`playerbots: add autonomous profession progression`):

- Cached profession planner that selects known recipes capable of natural skill-ups.
- Bounded crafting batches and per-reagent material targets.
- Maintenance integration that crafts a planned recipe when materials are present.
- Reagent retention through item-usage classification, including Inscription
  reagents.
- Configuration and documented defaults in Classic, TBC, and WotLK templates.

Commit `d37e0cfa` (`playerbots: add profession material acquisition`):

- Vendor purchases for only the current plan's missing reagents, with live
  inventory checks, quantity limits, and the existing tradeskill budget.
- World gathering continues through the established item-usage, loot, gathering,
  and RPG travel systems.

Commit `1430cec5` (`playerbots: integrate profession Auction House economy`):

- Auction House purchases for missing reagents with per-bot cooldowns, price
  validation, gold budgets, purchase limits, and no partial-bid speculation.
- Empty-house checks are also throttled.
- Posting limits apply only to rollout bots, so disabling the feature preserves
  previous posting behavior.

The implementation deliberately reuses the existing trainer progression and
crafted-item equip/use/keep/sell behavior rather than duplicating those systems.

Commit `ebe036e0` (`playerbots: add profession progression diagnostics`):

- `profession` chat diagnostic showing rollout state, persisted skills, selected
  recipe, and missing materials.
- This implementation log.

The final correctness change recorded with this version of the log recalculates
material deficits from live inventory while retaining the cached recipe choice.
This prevents vendor purchases made inside the plan-cache interval from causing
unnecessary AH purchases, and keeps crafting readiness and diagnostics current.

Commit `537e4a97` (`playerbots: revalidate cached profession material needs`)
contains that live-deficit correction.

## Configuration defaults

- Enabled: `1`
- CanaryPercent: `10`
- PlanCheckInterval: `60` seconds
- CraftBatchSize: `5`
- MaterialTarget: `20`
- VendorPurchaseLimit: `10`
- AHSearchCooldown: `600` seconds
- AHPurchaseLimit: `2`
- AHBudgetPercent: `10`
- AHMaxPriceMultiplier: `1.25`
- AuctionPostLimit: `3`

## Validation completed

- `git diff --check` passes (only Git line-ending notices were emitted).
- All directly changed WotLK translation units compile independently with Zig C++
  0.13 against the local CMaNGOS WotLK headers.
- Representative consumers of the modified headers also compile, including
  `RpgSubActions.cpp` and `AiObjectContext.cpp`; this verifies the RPG and chat
  action registrations.
- A Classic/TBC compile probe is not authoritative in this workspace because only
  WotLK core headers are available. It stops on pre-existing cross-core API
  differences in `PlayerbotFactory.cpp`, not on the new profession code.
- The fork's three Actions workflows are marked active, but the feature-branch
  push did not enqueue a run and GitHub reports zero checks for the source tip.

The compile objects are under `.validation-tools/` and must not be committed.

## Remaining work

1. Review pull request #1; do not merge it automatically.
2. Before merge/deployment, run the repository's full Classic/TBC/WotLK CI matrix
   in an environment where the matching core checkouts are available.
3. For deployment, rebuild and restart the worldserver, copy the desired
   `AiPlayerbot.ProfessionProgression.*` settings, and retain the default 10%
   canary until live diagnostics are satisfactory.

## Recovery commands

```powershell
git switch feature/playerbot-profession-economy
git status --short --branch
git log --oneline --decorate -8
git diff --check
git diff 99e6f15eb154bec2c8602e8b425fef87e67de501
```

Before committing, ensure `.core-reference/` and `.validation-tools/` remain
untracked and unstaged.

## Correction pass checkpoint (2026-09-29)

Work is continuing on `feature/playerbot-profession-economy` from published head
`65ac8666d69083f33f6cb65c4e7a2189a0fde5ec`. The initial audit found
`upstream/master` and `origin/master` at the original base
`99e6f15eb154bec2c8602e8b425fef87e67de501`; no merge, rebase, or pull was done.

The review findings were confirmed: missing profession materials did not activate
vendor/AH travel, spell-focus recipes were excluded from autonomous crafting, AH
stacks larger than the exact deficit were rejected, assignment reconciliation had
a stale-metadata edge case, and `CraftBatchSize` was not a durable pacing limit.

Uncommitted correction work currently includes:

- a small testable policy layer for metadata reconciliation, deterministic canary
  selection, cooldown checks, and bounded AH overbuying;
- authoritative reconciliation of persisted real professions;
- a 30-second autonomous profession craft cooldown configuration;
- cached material acquisition classification for mining, herbalism, skinning,
  fishing, vendor reagents, and AH fallback;
- profession-specific travel activation values plus a real spell-focus travel
  purpose;
- compile-time policy tests, including WotLK Inscription guards and AH/cooldown
  cases.

Still to complete after this checkpoint: wire the new named travel requests into
TravelMgr/TravelStrategy, record successful craft cooldowns, finish AH reasonable
overbuy safeguards, scope crafted-output posting limits, compile/link the complete
WotLK worldserver, commit/push follow-up commits, and update PR #1. Full autonomous
Enchanting and targeted arbitrary cloth/drop farming are being evaluated as
explicit limitations rather than implemented unsafely.

The untracked `.core-reference/` and `.validation-tools/` directories remain
preserved and must not be added to Git. Nothing has been deployed or restarted.

Checkpoint validation: `git diff --check` passes apart from Git's existing
line-ending notices. WotLK compile probes succeeded for `PlayerbotAIConfig.cpp`,
`PlayerbotFactory.cpp`, `CraftValues.cpp`, `LootValues.cpp`, `TravelValues.cpp`,
and `ProfessionProgressionPolicyTests.cpp`. This is not the required full
worldserver build/link, so the branch remains **not ready for a live canary**.

### Upstream movement detected before push

A second safety fetch immediately before pushing detected that official
`upstream/master` had advanced by one commit to
`0b3e77f5dae92c2eb304288ed0d6c8139c3b7fdc` (`Fix problems with
Waitforattackkeepsafedistance`). The push was deliberately aborted before any
remote branch was changed, in accordance with the instruction to report an
upstream change before merging, rebasing, or pulling. Local correction commit
`12b47d8c` is one commit ahead of the published feature branch. The feature is
8 commits ahead and 1 commit behind current official upstream. PR #1 still
points to published head `65ac8666` and has no checks.

Required next decision: inspect the single upstream commit for overlap, then
either rebase/merge it into the feature branch after approval or leave this PR
on its original base and push the correction commits as-is. After that, finish
the still-open correction work and complete a full WotLK worldserver link.

### Upstream integration completed

The user approved syncing the fork. The upstream commit only changed
`WaitForAttackAction.cpp`, `WaitForAttackAction.h`, and `CombatStrategy.h`, with
no overlap with the profession correction files. `origin/master` was
fast-forwarded to official upstream `0b3e77f5`, then `upstream/master` was merged
into the feature branch with merge commit `71a08202`. No rebase, history rewrite,
or force-push was used. The correction commits remain intact and the feature
branch is now based on current official upstream.

## Final correction validation (2026-09-29)

Current official upstream `c1193bcb4fdb9c1e0bc758471022776c141e6f73` was
merged normally as `23996e38`; Git preserved both profession-focus travel and
upstream quest-target behavior. Portable include fixes and immediate live-gate
checks were committed as `1202d0253b9be813d4f0e303bca9ad4066bff944`.

The complete Release WotLK build then completed all 739 candidate-build steps
and linked `bin/x64_Release/mangosd.exe` against CMaNGOS WotLK core
`1cd9d566ae83c1a88f1b057514697055055f419c`. The executable is 28,948,992 bytes
with SHA-256 `98C78755C7FAAEDBF955CD385E4906B8E795295F1E3B4BEA802A890805F6C57B`.
CTest found no registered runtime tests; compile-time profession policy checks
compiled in `libplayerbots.a`. No deployment occurred.

Publication remains pending because the visible repository's `.git` directory
is read-only to this session and the saved GitHub CLI token is invalid. PR #1
has therefore not yet been updated.


## Live canary validation (2026-09-30)

The profession branch was deployed to a Linux CMaNGOS WotLK test server and
validated with the autonomous rollout temporarily raised to 100% for observation.

Test conditions deliberately excluded the two easiest false positives:

- `AiPlayerbot.RndBotCheats` was empty, so profession materials were not supplied
  by the item cheat.
- `AiPlayerbot.RandomBotNonCombatStrategies` did not include `+rpg craft`, so
  the existing optional RPG crafting strategy was not responsible for the
  profession planner output.

The live `.rndbot do <name> profession` diagnostic successfully reported an
active random bot with persisted profession state and a concrete crafting plan.
For example, Beanezoth reported Alchemy 1/75, Herbalism 58/75 and a planned
`Elixir of Minor Defense` batch of 5 with the craft cooldown ready. Live bots
were also observed performing profession-related world activity while the
feature was enabled.

Live testing found two omissions in the new chat diagnostic path:

- commit `6153723b477860a1bfe853bfaf7db9c183ac3c10`
  (`playerbots: register profession chat trigger`) registers the
  `profession` chat trigger;
- commit `2b5a18c00e25f286a7790967bb638278ecf7715d`
  (`playerbots: route profession chat command`) adds `profession` to the
  chat-command handler's supported action list.

The direct console diagnostic worked before those chat fixes. The final whisper
path should be runtime-retested after rebuilding/restarting with both commits.

The Linux deployment used the normal CMaNGOS build and restart path; no database
migration was required. Long-running progression, economy behavior under a
smaller canary, and Classic/TBC build coverage remain follow-up validation work.
