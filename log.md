# Profession economy implementation log

Last updated: 2026-09-29 (Europe/Amsterdam)

## Repository checkpoint

- Local branch: `feature/playerbot-profession-economy`
- Base commit: `99e6f15eb154bec2c8602e8b425fef87e67de501`
- Current source tip preceding the diagnostics/log commit:
  `1430cec5` (`playerbots: integrate profession Auction House economy`)
- This file is part of the following diagnostics commit; use `git rev-parse HEAD`
  for its post-amend hash.
- Official upstream: `upstream` -> `https://github.com/cmangos/playerbots.git`
- Writable fork: `origin` -> `https://github.com/goakiller900/playerbots.git`
- The branch has not been pushed and no pull request exists yet.

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

## Implemented but not yet committed

- `profession` chat diagnostic showing rollout state, persisted skills, selected
  recipe, and missing materials.
- This implementation log.

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

The compile objects are under `.validation-tools/` and must not be committed.

## Remaining work

1. Commit the chat diagnostic and this log.
2. Run final repository/configuration validation and review the complete branch
   diff.
3. Fetch remote metadata again and verify the branch is based on the intended
   upstream commit.
4. Push without force to `origin/feature/playerbot-profession-economy`.
5. Create (but do not merge) the requested pull request targeting the normal
   development branch, then record its URL here.

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
