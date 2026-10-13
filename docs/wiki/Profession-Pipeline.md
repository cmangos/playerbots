# Profession pipeline

[Wiki home](Home.md) | [Architecture](Architecture.md) | [Troubleshooting](Troubleshooting.md)

## 1. Eligibility

`ProfessionCraftingPlanValue::IsEnabledFor` requires progression to be enabled,
nonzero participation, a free bot and no active player master. A deterministic
identity hash chooses the configured cohort. At 10%, the approximate participating
fraction is stable across updates; it is not a fresh 10% chance to craft each tick.

This gate does not globally disable existing gathering, fishing, enchanting,
optional RPG crafting or economy behavior outside this feature.

## 2. Discover learned recipes

The planner reuses `CraftSpellsValue`, which enumerates learned, enabled,
non-passive crafting spells using their effects. Skill-line metadata associates
recipes with the bot's actual professions/secondary skills, and
`SpellGivesSkillUp` filters root goals.

Candidate discovery is not a scan of every trainer recipe followed by free
learning. A recipe absent from the spell book cannot be used as a producer.
The existing enumeration has effect-shape limitations; it is not a universal
adapter for every possible custom spell. WotLK processing spells are added through
their Milling/Prospecting effects and processing-input metadata.

## 3. Calculate honest quantities

Each reagent requirement is multiplied by the proposed number of casts. The
configured batch and material target bound demand. Whole-cast availability can
reduce a batch to the amount the bot can execute now.

For example, with a batch limit of five and three usable units of a reagent
consumed one per cast, the bot can plan three casts. A spell producing two items
per cast can create six output items. Batch count means casts, not output units.

Tools are prerequisites rather than consumed batch reagents. Explicit tool item
requirements and compatible totem/tool categories are checked against core
metadata. A missing non-consumable tool targets one compatible item, not 20 copies.
The current tool-selection fallback can choose one metadata-compatible alternative
without guaranteeing that this exact alternative is on the AH.

## 4. Resolve bounded prerequisites

The planner builds an output-to-producer index from the bot's learned recipes.
`NextProductionStep` can descend through **two prerequisite edges** from a root
goal, rejecting ancestor cycles. This is bounded dependency exploration, not
unrestricted arbitrary-depth crafting.

Conceptual chains supported when the bot's metadata and learned spells permit them:

| Demand | Possible next step | Still required |
| --- | --- | --- |
| Metal bar | Learned smelting recipe | Real ore, mining/recipe requirements and any required focus |
| Cloth bolt | Learned bolt recipe | Actual cloth |
| Engineering component | Learned component recipe | Its inputs, tools and any focus |
| Ink | Learned ink recipe | Pigment and any tool requirement |
| Pigment | Known Milling effect | A valid real herb stack and supported loot metadata |
| Gem | Known Prospecting effect | A valid real ore stack and supported loot metadata |

The planner may use a grey producer to satisfy a skill-up goal. It does not award
the producer a fake skill gain. A ready prerequisite is preferred; otherwise a
stable metadata order exposes genuine missing raw inputs to acquisition.

For processing, expected loot identifies a possible source. It does **not** count
as inventory. One cast targets one actual eligible stack, the request retains its
input identity, and matching normal loot completion permits fresh observation.
The next step may still need another processing cast because the output is random.

## 5. Score goals and bound monopolization

The preserved scoring formula is:

```text
(next step has all direct reagents ? 1,000,000,000 : 0)
+ root recipe DBC min_value * 1,000
- next step missing reagent units * 10
```

DBC `min_value` is a skill-up threshold used by the existing helper; it must not
be presented as the trainer's required skill. Material completeness also does
not prove that tools, location or spell execution are ready.

`CraftingFairness` keeps an independent ready-since timestamp per runtime skill:

1. Scores choose normally when no ready skill is overdue.
2. A selected ready skill can retain an opportunity for up to 300 seconds.
3. Another ready skill waiting at least 300 seconds can take priority. The oldest
   wait wins; scores break equal-age ties.
4. A pending continuation keeps ownership through its cleanup.
5. Disappeared or unready candidates lose their readiness age. Expired owners do
   not immediately reacquire ahead of an already waiting skill.

Here, fairness readiness means usable inputs, tools and a valid processing stack
where needed. It does **not** require local spell-focus readiness. A focus journey
can therefore own an opportunity, but its ownership is bounded. Five minutes is
not a guaranteed execution deadline; periodic evaluation, several ready skills
and other AI activity still affect timing.

There is no fixed rotation by profession name and no Cooking/Engineering/etc.
exception. Plan invalidation preserves the ledger and avoids rotation on every
AI update.

## 6. Acquire missing supplies

After selecting the production step, source classification applies to its actual
missing reagents and missing tool purchases:

```text
practical gathering source for a skill the bot owns
    -> otherwise practical vendor
    -> otherwise AH fallback
```

This is the present preference. The proposed AH-first surplus/retry economy was
not implemented. Intermediate production happens before this classification
when a learned producer can supply the demand.

Gathering reuses skill-specific TravelMgr destinations. Vendor selection checks
practical availability; a vendor entry somewhere in the database is insufficient.
Buying uses normal stock, money, purchase pack and capacity checks. AH acquisition
uses cooldowns, bounded stacks, the existing trade-skill budget and bot-buy price.

Ordinary looting can supply cloth, meat or other materials. The feature does not
construct targeted farming routes from arbitrary creature-drop probabilities.

## 7. Reach a real crafting focus

A focus-dependent plan requests `CraftingFocus` travel through TravelMgr. On the
autonomous maintenance route, focus resolution requires that destination's
`WORK` state, then validates a real object with the core `GameObjectFocusCheck`.

If the chosen spawn is unavailable, the resolver can reuse the cached nearby
object list to find a matching real focus. The shared crafting action still
checks `GAMEOBJECT_TYPE_SPELL_FOCUS` and the exact required focus ID.

Arrival alone is not enough. The required usable object must resolve at the bot's
actual location. No object is spawned and no teleport is used. A locally present
focus without the autonomous travel/WORK handoff is not automatically equivalent
to satisfying this particular maintenance route.

## 8. Dispatch the selected step

`CanCraftProfessionValue` checks participation, plan validity, cooldown, live
reagents, bag usage, pending/current cast state, learned spell, tools, processing
input and location. Its maintenance trigger invokes `craft random item`.

The action rechecks volatile conditions, resolves the real focus or processing
item, records a request and generates `castnc`. Common command routing reaches
`cast custom nc spell`, which uses the existing core spell path.

This separation prevents a cached ready plan from bypassing a tool or inventory
change. The maintenance path does not choose an unrelated fallback recipe.

## 9. Preserve ownership and recover

The pending lease prevents competing autonomous requests during a batch.
An accepted cast can renew its lease. Requeueing the same spell without an
accepted cast does not extend the lease indefinitely.

Expiry uses at least 120 seconds, or the configured action-expiry duration plus
60 seconds if larger. Active non-melee casting protects an in-flight request.
Matching cleanup resets the request and calculated values without deleting a
value while its own calculation is running.

Ordinary accepted casts update the last-craft timestamp, invalidate planning and
may queue the remaining count under the same ownership. Processing retains its
request until the matching loot lifecycle is observed. Failure/expiry can clear
ownership without claiming any output or skill change.

## 10. Observe, reserve and train

Only the game core produces items and natural skill gains. An accepted cast is
not a measurement of a completed cast; confirm real inventory and skill state.
`ItemUsageValue` retains bounded inputs for the selected chain and lets ordinary
use/equip/sell/bank policy handle surplus.

At rank caps or when new recipes are needed, existing trainer logic remains
responsible. Trainability comes from core level/skill/prerequisite rules; travel
and training use existing money and strategy gates. No additional rank-cap
override or free recipe path was added.
