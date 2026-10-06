# Changes and limits

[Wiki home](Home.md) | [Code reference](Code-Reference.md) | [Build and tests](Build-and-Tests.md)

## Feature history represented by this source

| Area | Current behavior | Why it matters |
| --- | --- | --- |
| Profession diagnostics | Registered/routed command with private requester output | A diagnostic action existing in source is insufficient without its trigger/route. |
| Common castnc dispatch | Shared command route independent of optional RPG crafting | Autonomous plans can reach the ordinary noncombat spell action without enabling the experimental bundle. |
| Selected-plan dispatch | Maintenance executes only its selected step | A blocked root cannot silently execute a different random craft and misrepresent progression. |
| Pending ownership | Request snapshot, continuation lease, accepted-cast renewal and expiry | Competing jobs are blocked without indefinitely renewing failed retries. |
| Focus handoff | Real CraftingFocus WORK state and exact usable object resolution | Reaching a crafting destination can hand off to the common action. |
| Partial quantities | Whole-cast, bounded batches from real stock | A partial executable batch need not wait for a full batch; no material availability is invented. |
| Tool metadata | Explicit items/categories, one-copy demand and common purchasing | Learned recipes and compatible upgraded tools are handled generically. |
| Practical supply | Vendor feasibility and AH fallback | A theoretical vendor entry does not permanently hide the AH path. |
| Ready-skill fairness | Separate ready-age ledger and 300-second opportunities | A repeated high-scoring goal cannot automatically monopolize every ready skill forever. |
| Bounded production | Learned producers, grey intermediates, processing input/loot ownership | Direct inventory demand can be satisfied through reachable recipe prerequisites. |
| Travel expiry | Activity reads evaluate timed non-forced target deadlines | Stopped movement need not leave an expired target active indefinitely. |
| Empty outgoing chat | Post-parse empty messages return before reply/history handling | A separate generic chat defect remains separate from craft policy. |
| Remote services | Opt-in scoped personal-service access through ordinary transactions | Access is changed without implementing a second transaction/economy engine. |

## Upstream synchronized at `13039f29`

The local merge includes seven upstream commits through `14ccaa25`, adding
12 changed files with 523 insertions and 72 deletions relative to the prior fork
head. These upstream additions are distinct from the local remote-service patch.

| Commit | Area |
| --- | --- |
| `4532d664` | Instance test generation improvements |
| `dcb2e15e` | Additional optional bot action/state/heartbeat logging |
| `54f2209e` | Taxi and zone-update debug commands |
| `689eb574` | Test bot-creation failure feedback and retry handling |
| `eda0c9e3` | Combat stance initialization and melee positioning |
| `b60d39d1` | Free-bot login/master/teleport crash correction |
| `14ccaa25` | Quest test corrections |

They do not directly modify the profession planner, scoring or prerequisite
bridge, although common AI, movement and diagnostics can affect observations.
Taxi/zone debug commands can change game state; their presence does not make
them read-only profession diagnostics.

Optional logging keys added upstream include `bot_events.csv`,
`bot_reactions.csv`, `bot_states.csv` and `bot_heartbeat.csv`. Their emission is
conditional on existing logging/debug paths; simply naming a CSV does not
guarantee complete coverage of every action. Inspect the actual log configuration
and call-site gates before relying on them for a live trace.

## Remaining limits

- The latest combined source has lightweight validation, not a recorded full
  compatible-core build or complete live profession matrix.
- Prerequisite exploration is bounded. Chains beyond two prerequisite edges,
  unknown producers and unsupported effect shapes need real direct supply or
  separately justified work.
- Processing indexes simple eligible loot entries/groups. Conditional, quest-only
  and reference-based loot are not guessed to be usable sources for every bot.
- Processing yields remain random; even a valid known source can fail to provide
  the currently needed item in one cast.
- Tool AH selection does not search every compatible alternative indefinitely.
- Gathering/vendor/AH supply can genuinely be absent, inaccessible or unaffordable.
- Trainer travel/learning remains subject to existing strategies, money and core
  rules; every rank transition has not been demonstrated live.
- The `profession` diagnostic is a status summary, not a full execution profiler.
- Full AI reset `std::bad_alloc` remains unresolved and should be investigated
  independently with safe crash evidence.
- Failed auction relisting, reason-tagged bank cooldowns, liquidation cycles and
  cross-bot material transfer were not implemented.
- The remote bridge is supplied for a compatible WotLK core; other expansions or
  changed core access interfaces require a separate compatibility review.

## Contribution boundary

A useful next change starts with an observed generic path and the smallest proven
failure. Reuse existing actions, document the invariant, consider large-population
cost and test failure/cleanup as well as success. Do not interpret every limitation
above as approval for a new subsystem or a reason to relax core crafting rules.
