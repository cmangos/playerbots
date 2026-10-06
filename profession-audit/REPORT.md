# Profession progression technical review

## Scope and validation boundary

This report consolidates the profession feature's architecture, demonstrated
integration failures, fixes and remaining limits. The current branch includes
upstream through 45bed519. Full WotLK compilation and startup were verified for
profession revision be2fa8a4 before the subsequent upstream Netherspite merge.
That build result does not validate later upstream changes, every profession/rank
or live remote-service transactions.

Profession participation remains 10% by default. Remote services remain separately
disabled by default. Runtime behavior uses learned spells, skills, inventory,
item/tool/focus metadata and request state; sampled characters are observations,
not implementation conditions.

## Earlier population evidence

An earlier approximately five-hour observation showed materially different results:

| Profession | Earlier highest observed skill | Interpretation |
| --- | --- | --- |
| Alchemy | 25/75 | Repeated natural crafting was possible. |
| Enchanting | 12/75 | Progress existed; existing enchant/disenchant paths also contribute. |
| Leatherworking | 17/75 | Progress existed with genuine creature-material/vendor dependencies. |
| Cooking | 55/75 | Population progress did not prove every focus-travel path worked. |
| Blacksmithing | 1/75 | Direct bar supply, producers, tools and focus required investigation. |
| Engineering | 2/75 | Most observed characters remained at 1; components/tools and arbitration mattered. |
| Inscription | 1/75 | Tools, processing, pigment/ink production and legitimate supply were separate issues. |
| Jewelcrafting | 1/75 | Tools, learned candidates, gem/ore supply and arbitration required investigation. |
| Tailoring | 1/75 | Cloth-to-bolt production and competing ready goals required investigation. |

These are historical observations, not a current measurement or proof that one
failure explains every character. Later short observations recorded gains in
additional crafting professions, but interruptions and delayed saves prevented
complete attribution. Gathering/fishing gains alone do not validate this planner.

## Generic failures and corrections

| Path | Demonstrated failure | Correction and reuse |
| --- | --- | --- |
| Assignment/init | Stored metadata could disagree with real professions; skill-1 cleanup removed legitimate assignments; randomization synthesized profession values. | Reconcile actual skills, preserve earned values/assigned secondary lines, initialize only missing lines through PlayerbotFactory. |
| Command routing | A diagnostic or cast action existed without the common trigger/route needed to execute it. | Register profession diagnostics and route castnc through ChatCommandHandlerStrategy. |
| Maintenance dispatch | Autonomous work could depend on optional RPG crafting or fall through to another recipe. | Maintenance calls the shared CraftRandomItemAction for only the selected execution step. |
| Continuations | Releasing after an accepted cast allowed another batch to compete; repeated unaccepted retries could renew indefinitely. | Retain the request through its continuation; renew only for accepted progress and recover expired work. |
| Cache cleanup | Cleanup could delete the readiness value while its own Calculate method was executing. | Reset that value and matching request/cache state rather than deleting the active object. |
| Spell focus | Focus travel reached WORK without autonomous execution; the selected spawn could be distant/unavailable despite a usable local match. | Resolve a real focus through the core predicate and cached nearby-object list, then reuse ordinary crafting/casting. |
| Quantity | A full configured batch could hide a smaller executable batch. | Calculate whole-cast availability and reduce batch demand without inventing stock. |
| Tools | Learned explicit/category tools were recognized inconsistently and missing tools did not reach acquisition. | Share core-compatible readiness; derive learned requirements and one-copy supply through existing vendor/AH policy. |
| Vendor fallback | An indexed vendor entry could suppress AH despite an unusable offer/destination. | Check practical cash stock and travel/faction feasibility while retaining normal purchase checks. |
| Arbitration | A repeated high-scoring ready goal could indefinitely exclude another ready skill. | Add a separate bounded ready-age ledger; preserve scores and pending continuations. |
| Intermediate demand | Direct missing-reagent planning could not reach learned grey producers or processing inputs. | Index learned producers, bound prerequisite exploration to two edges and reuse existing spell/loot actions. |

## Why the optional RPG bundle is not globally enabled

RpgCraftStrategy registers crafting, random spell casting and random item use.
Enabling that entire experimental bundle would introduce unrelated behavior into
normal autonomous bots and would not supply demand, fairness or continuation state.

The crafting implementation itself is reused: RpgCraftAction delegates to
CraftRandomItemAction. Profession maintenance reaches the same action through
can craft profession, and the common castnc route reaches CastCustomSpellAction.
Ordinary RPG vendor, gathering, trainer and economy behavior remains relevant.

## Planner and fairness invariants

The preserved score is reagent completeness for the next execution step, plus the
root DBC min_value threshold multiplied by 1000, minus missing units multiplied
by 10. Completeness contributes 1,000,000,000. The threshold is not necessarily
trainer-required skill or economic value.

Ready waiting time lives separately from the calculated plan. A competing ready
runtime skill can receive a 300-second opportunity, with oldest wait first and
score as an equal-age tie-breaker. Materials/tools and valid processing input
establish fairness readiness; focus travel may still be needed. Pending batches
retain execution until cleanup. This is neither per-update rotation nor a promise
of successful casting within exactly five minutes.

## Intermediate production and profession-specific limits

Known ordinary recipes provide output-to-producer relationships. Two prerequisite
edges allow chains such as root item to ink to pigment processing. Cycles are
rejected; grey producers can supply a useful root without claiming a skill gain.

| Profession | Metadata-derived support | Remaining legitimate blockers |
| --- | --- | --- |
| Blacksmithing | Learned bar/smelting producers, tool categories and real focus travel. | Missing ore, unknown smelting, unavailable supply, tools/focus or rank. |
| Engineering | Learned components, bounded nested inputs, partial casts and shared fairness. | Longer chains, missing materials/tools, unknown recipes or focus. |
| Inscription | Known Milling, actual herb-stack/loot handling, learned ink producers, parchment/tool buying. | Herbs, random pigment deficits, unsupported loot metadata, money or unknown ink/recipe. |
| Jewelcrafting | Known Prospecting, real ore stacks/loot, tool/category checks and learned producers. | Ore/gem supply, unknown recipes, random outputs or deeper chains. |
| Tailoring | Learned grey bolt recipes and retained cloth/intermediates. | Ordinary-loot/AH cloth supply, thread, unknown producers or advanced dependencies. |
| Alchemy | Direct learned recipes and ordinary reagent acquisition/casting. | Genuine herb/vendor supply, money, rank and useful candidates. |
| Leatherworking | Existing skinning/loot/vendor routes and learned recipes. | Actual leather supply, thread, tools, rank or recipe availability. |
| Enchanting | Existing enchant/disenchant maintenance and eligible item-producing crafts. | Reagent sustainability, useful targets, gear, tools or recipes. |
| Cooking | Shared learned-craft policy and real focus travel/validation. | Meat/other inputs, usable focus, recipes and normal scheduling. |

Processing metadata identifies possible output, never guaranteed inventory. A
known Milling/Prospecting effect targets a valid real stack, performs one cast,
retains its input identity and waits for matching normal loot completion before
replanning. Conditional, quest-only or referenced loot is not guessed to be usable.

## Acquisition, training and performance

Missing inputs/tools reach practical gathering, then vendor, then AH fallback.
No arbitrary worldwide creature-drop farming is added. Purchases remain subject
to actual money, stock, quantities, delivery and storage. AH searches count empty
attempts, use cooldowns, bounded stacks and existing valuation/trade-skill budgets.
Needed chain inputs are retained through ordinary item-usage policy.

Trainer metadata, travel, costs, trainability and rpg train remain the rank/recipe
learning path. No candidate does not inherently disable an independent trainer
request. Full progression through every rank to 450 is not established.

Recipe/requirement caches, shared tool/processing indexes, per-plan inventory
memoization, bounded prerequisite depth and slow AH searches control repeated
work. The fairness map contains currently ready runtime skills. Focus fallback
uses local object caches. The inherited CalculatedValue interval semantics permit
recalculation at approximately half the configured interval; events can invalidate
sooner. These controls are not a measured population-wide performance benchmark.

## Independent travel and chat corrections

TravelTarget::IsActive now evaluates timed, non-forced expiry without requiring a
movement action to call CheckStatus. This prevents indefinitely stale activity
from suppressing quest, vendor, AH, trainer, RPG or profession travel. Forced and
unlimited targets and reset safety gates remain intact.

Empty parsed outgoing chat returns before reply/history processing. This is a
separate generic correction with its own fixture.

The recorded full-AI-reset bad_alloc remains a separate unresolved issue; neither
its cause nor a fix is established by profession progression. The deployment-only
AH money-delivery adjustment is not included in this feature's source changes.

## Optional remote services

RandomBotRemoteServices defaults to zero and requires the compatible WotLK core
bridge supplied in patches/cmangos-wotlk-remote-services.patch. It reuses existing
personal-bank, AH and mail actions. Access is scoped to the current bot, service
and thread and restored on exit; ordinary security, transactions and capacity
remain authoritative. Busy states and pending profession work defer maintenance.
Without the bridge, normal world access remains active. Live remote transactions
and compatibility with other cores/expansions remain unverified.

## Recorded validation

| Check | Result |
| --- | --- |
| Policy/regression executable | PASS, including 1,050,000 quantity combinations and lifecycle/fairness cases. |
| Production fixtures | 2,087 cases PASS. |
| Component fixtures | 54 cases PASS. |
| Complete profession-readiness fixture | 17 cases PASS; pre-fix unqualified-helper baseline rejected at compilation. |
| Source integration | 23 checks PASS. |
| Travel lifecycle | 59 cases PASS. |
| Outgoing chat | 6 cases PASS. |
| Remote bridge/fallback | 84 bridge and 7 fallback checks PASS. |
| Full WotLK target | Profession revision be2fa8a4 compiled/linked against compatible core based on 2cce0b2e, with the optional bridge included. |
| Installed startup | Built, installed and actual running executable hashes matched; realm/world initialized and game listeners were reachable. |

Full compilation exposed two class/API boundaries that lightweight fixtures had
not covered: a temporary ItemQualifier could not bind to formatItem's non-const
reference, and CanCraftProfessionValue needed an explicitly qualified call to
ProfessionCraftingPlanValue::GetProcessingTarget. Named-local formatting and the
existing static helper qualification corrected them. The complete-readiness
fixture now compiles its actual body with the correct class boundary.

World doubles and source checks do not establish scheduler fairness, full live
market behavior or sustained natural skill gains. The later Netherspite merge
was not part of the recorded full build. Classic/TBC full builds, every advanced
chain/rank and live remote transactions remain outside that evidence.

## Reproduction and remaining work

Use [the test reference](../playerbot/strategy/tests/README.profession-progression.md)
and [build guide](../docs/wiki/Build-and-Tests.md) with a compatible isolated core.
Inspect rndbot do <bot> profession, then compare actual inventory, casts, loot,
skill/rank, pending cleanup and replanning over time. Gathering gains or a queued
command alone are insufficient proof. No full reset, synthetic skill/item grant
or focus bypass is needed for observation.

Remaining limitations are genuine acquisition shortages, unknown producers,
chains beyond two edges, unsupported loot/effects, imperfect tool alternatives
on the AH, incomplete full-rank/live coverage and the independent reset crash.
Failed-auction retry/bank-cooldown cycles and cross-bot transfers are not implemented.
