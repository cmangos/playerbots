# Profession economy implementation log

Last updated: 2026-09-29 (Europe/Amsterdam)

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
