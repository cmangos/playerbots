# Troubleshooting

[Wiki home](Home.md) | [Usage](Usage.md) | [Code reference](Code-Reference.md)

## Locate the first failed stage

```text
participation -> known recipe -> selected step -> inputs/tools
-> acquisition -> travel/focus -> readiness -> dispatch -> real cast
-> inventory/loot -> natural skill -> cleanup/cooldown -> replan
```

Gather evidence in that order. Fixing a later stage cannot supply a missing
recipe, money or raw material at an earlier stage. Bot/profession names are labels
for observations, not valid special-case fixes.

## Symptom table

| Symptom | First things to verify | Relevant code |
| --- | --- | --- |
| Bot inactive | Global switch, 10% cohort, free-bot status, active master | `IsEnabledFor` |
| No plan | Learned/disabled spells, skill-up eligibility, skill cap, effect metadata | `CraftSpellsValue`, `SpellGivesSkillUp`, planner |
| Final recipe needs a crafted input | Producer is learned, indexed and within two prerequisite edges | `NextProductionStep` |
| Only ore/cloth/herbs owned | Actual bar/bolt/pigment recipe/input metadata; raw items are not their processed output | Production index |
| Missing tool persists | Explicit tools/category compatibility, practical supply, selected AH alternative, live ownership | Tool requirements/purchases and core category checks |
| Missing materials persist | Gathering skill/destination, usable vendor stock, real money, affordable AH listings | Material sources, BuyAction, AhAction |
| One skill keeps winning | Other skill's next step is truly ready, its age persists, pending ownership and observation interval | Fairness ledger |
| All reagents owned but cannot craft | Tools, bag usage, cooldown, current/pending cast, focus WORK/object resolution | `CanCraftProfessionValue` |
| Travel does not start | Existing active target, request gates, purpose destinations and current source classification | Travel values/actions |
| WORK but no cast | Exact focus object, maintenance strategy/trigger, selected-plan event source and common castnc route | Focus resolver, craft/cast actions |
| Pending never clears | Active cast, accepted processing input, matching loot completion or lease expiry | Pending/request lifecycle |
| AH bought but bags unchanged | Ordinary delivery mail and collection waiting policy | AH/core mail and MailAction |
| Cast accepted but no skill gain | Actual completion, recipe difficulty/chance, rank cap and observed skill | Core spell/skill rules |
| At 75/150/etc. | Level, trainable ranks/recipes, reachable trade trainer, budgets and RPG training strategy | Trainer values/actions |
| Remote option enabled but still travels | Compatible bridge, fallback log and eligibility; old targets can remain until normal expiry | RemoteServiceAccess |

## Inscription and Jewelcrafting

Investigate each prerequisite separately instead of treating the profession as
one indivisible failure:

- Is the skill-up root recipe known and does it appear in candidate discovery?
- Is its next material directly available, or does it need a learned producer?
- For ink, is pigment available and is the ink recipe learned?
- For pigments/gems, is Milling/Prospecting known, with a valid real input stack
  and supported loot metadata?
- Is the required ink-set/jeweler-tool category present or legitimately sourced?
- Are vendor consumables such as parchment genuinely in stock and affordable?
- Can the current root compete fairly once its next step is ready?

The generic bridge can solve reachable learned chains. It cannot guarantee raw
herbs/ore, particular random drops, unknown producer recipes or nonexistent supply.

## Trainer and rank progression

Trainer requests are independent of the selected profession plan. Existing
`TrainableSpellsValue` calls the core trainer-state predicate; trade-trainer travel
checks train cost and skill-training money. `RpgMaintenanceStrategy` supplies the
`rpg train` route, and the action uses `TrainerAction`.

The existing training implementation also has its own `AutoTrainSpells` and
gold-cheat behavior. Do not enable free training or cheats to validate legitimate
profession progression. This feature does not replace those rules, and source
coverage does not prove every rank transition has been observed live.

## Stale travel lifecycle

The current `TravelTarget::IsActive` checks deadlines on activity reads, so expiry
does not depend exclusively on a movement action calling `CheckStatus`. An
expired non-forced, timed target becomes `EXPIRED` and clears the cached absence
of available destinations. Forced/unlimited targets retain their distinct rules.

`ResetTargetAction` still has generic battleground/selection/PREPARE safety gates.
An active valid target can legitimately reject resetting; the expiry correction
addresses the stale-active case. `reset travel target` is a different action from
full AI `reset`, and is not necessary for a read-only trace.

Check cached values and actual target status/time together. A negative displayed
deadline alone does not prove which gates are currently blocking a request.

## Full AI reset allocation crash

The earlier `rndbot do <bot> reset` crash with `std::bad_alloc` remains a separate
unresolved finding. The observed timing does not establish the allocation site or
prove a profession cause. Do not reproduce it on a live realm as a recovery tool.
Safe investigation needs an existing core/backtrace, logs and source analysis.

## Useful evidence record

| Field | Record |
| --- | --- |
| Version | Actual source revision, built/installed binary identity and loaded configuration |
| Bot | Diagnostic label, active/inactive result and autonomous/master state |
| Skills | Value/max before and after |
| Spells | Root and producer IDs, names, learned state and DBC thresholds |
| Inventory | Direct inputs, owned processing stack, tools and output before/after |
| Plan | Goal, step, quantities, missing supplies and persistence over time |
| Movement | Purpose, target, deadline, arrival/WORK state and usable object |
| Execution | Readiness failure, dispatch, actual cast result and completion |
| Lifecycle | Pending lease/request, processing loot, cooldown and subsequent plan |

Use read-only diagnostics where possible. The current `profession` command covers
only part of this record; do not invent missing output fields.
