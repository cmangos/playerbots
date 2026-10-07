# Usage

[Wiki home](Home.md) | [Configuration](Configuration.md) | [Troubleshooting](Troubleshooting.md)

## Before observing progression

1. Build a compatible core with this PlayerBots source. Remote services additionally
   need the companion core patch. See [Build and tests](Build-and-Tests.md).
2. Put the canonical configuration in the server's active `aiplayerbot.conf`.
   Start with participation at 10% and remote services off unless you intend to
   validate that separate feature.
3. Use the server's normal controlled configuration/startup procedure. This guide
   is not a claim that any particular installed binary has already been updated.
4. Select online, autonomous bots and check their reported participation.

No extra client addon is required for autonomous progression. The code runs
inside the server's PlayerBots module. Optional UI addons can help inspect bots,
but cannot implement the core remote-access checks by themselves.

## Profession diagnostic command

Server console or an authorized console-command transport:

```text
rndbot do <bot> profession
```

Through the normal in-game administrative interface, where the realm uses the
standard dot-prefixed form:

```text
.rndbot do <bot> profession
```

For a bot you can normally command by whisper, the chat action is `profession`.
Existing command security still applies. The diagnostic action requests private
output to its requester. Transporting that output through a headless console
depends on the existing administrative interface; the feature does not add a
new general-purpose remote API.

## Reading the response

| Response | Interpretation |
| --- | --- |
| Progression enabled, participation 10%, bot active/inactive | Global switch plus this bot's actual eligibility/cohort result. |
| Profession/secondary skills | Current skill values reported by normal bot state. Gathering values alone do not prove this crafting path. |
| Missing tool | A selected compatible tool purchase is still needed, normally one item. |
| No skill-up recipe currently planned | No selected viable root candidate at that calculation; investigate skills, known spells and caps. |
| Plan and cast count | Next execution step and requested casts, not completed output. |
| Prerequisite for another spell | The execution step supplies the displayed root goal. |
| Input item | Processing uses this item type; dispatch still needs an actual eligible stack. |
| Spell focus | Required focus metadata; it does not confirm arrival or a usable nearby object. |
| Craft cooldown ready/waiting | Elapsed-time gate for a new autonomous job. |
| Missing materials | Fresh deficit for the execution step's direct reagents. |
| Acquisition counts and AH cooldown | Classified possible sources and search timing, not proof of stock, affordability or completed travel. |

The command does not print a complete candidate-score table, the entire fairness
ledger, every core cast result or all travel-state fields. If those are needed,
use existing debug facilities or read-only instrumentation. Do not describe them
as available in `profession` when they are not.

## Observe a normal crafting cycle

For each selected bot record:

1. Skill value/max, known root/producer recipes, real inventory and tools.
2. The diagnostic plan, deficits, focus requirement and cooldown.
3. Acquisition/travel changes through existing diagnostics or logs.
4. The actual cast and any failure result, including resolved object/item targets.
5. Inventory before/after, natural skill before/after, and the subsequent plan.

Successful evidence is a real cast that consumes legitimate inputs and creates
real output, followed by normal cleanup/replanning. Natural skill gain depends
on the core's recipe difficulty and chance; one successful cast need not increase
skill. A queued `castnc` line alone does not prove completion.

For Milling/Prospecting, observe the owned input stack, normal loot opening and
loot release, then the real pigment/gem counts. Never credit hypothetical drops
to the planner just to make the next recipe appear ready.

## Validate fairness

Choose a participating bot with at least two learned root skills whose execution
steps have usable materials and tools. Record the selected goal and readiness
over more than one 300-second opportunity, allowing for normal AI activity.

The initially higher-scoring recipe should remain preferred normally. A competing
ready skill that continues waiting can later receive an opportunity. A pending
batch must not be interrupted by a competing job. Missing inputs or a skill that
ceases to be ready do not qualify for the same wait-age guarantee.

There is no fixed sequence of profession names and no promise of equal skill
gains or exactly equal casts per hour.

## Validate tools and advanced recipes

For a newly learned recipe, check explicit item tools and category-compatible
replacements. Tool acquisition should request one missing non-consumable tool
and stop requesting it after a compatible item is actually owned.

Advanced recipes still require known producers, rank/level requirements and real
inputs. A chain deeper than the bounded production bridge, an unavailable AH
alternative, or a missing trainer recipe remains a legitimate blocker.

## Optional remote-service observation

Use the [remote-service checklist](Remote-Services.md#usage-and-observation).
Keep participation at 10%; remote access does not require raising it. Record real
gold, bank/bag quantities, mail and auction entries, rather than testing with
free stock or forced skill changes.

## Operations excluded from these diagnostics

Do not use full bot AI reset as an observation shortcut. An earlier live reset
produced an unresolved `std::bad_alloc`; this build does not establish its cause
or repair it. Teleports, free items/money/recipes and direct skill edits also
invalidate a natural-progression trace.
