# Profession progression configuration

For the complete architecture, usage and troubleshooting guide, see the
[profession build wiki](wiki/Home.md).

This guide describes the autonomous profession settings in the feature branch.
Place the options in your active `aiplayerbot.conf`. The block below keeps
participation at **10%** and uses the current defaults for all other settings.
The latest source changes still require a full core build and live validation
before their behavior can be confirmed on a running realm.

Optional [remote AH, mail and personal-bank access](REMOTE_SERVICES.md) is a
separate setting and requires its companion core bridge. It does not change
profession participation or crafting requirements.

```ini
AiPlayerbot.ProfessionProgressionEnabled = 1
AiPlayerbot.ProfessionProgressionPercent = 10
AiPlayerbot.ProfessionPlanCheckInterval = 60
AiPlayerbot.ProfessionCraftBatchSize = 5
AiPlayerbot.ProfessionCraftCooldown = 30
AiPlayerbot.ProfessionMaterialTarget = 20
AiPlayerbot.ProfessionVendorPurchaseLimit = 10
AiPlayerbot.ProfessionAHSearchCooldown = 600
AiPlayerbot.ProfessionAHPurchaseLimit = 2
AiPlayerbot.ProfessionAHBudgetPercent = 10
AiPlayerbot.ProfessionAHMaxPriceMultiplier = 1.25
AiPlayerbot.ProfessionAuctionPostLimit = 3
```

## Participation

| Option | Default | Accepted range | Effect |
| --- | --- | --- | --- |
| `AiPlayerbot.ProfessionProgressionEnabled` | `1` | Boolean `0` / `1` | Enables autonomous profession planning and its material/economy behavior for eligible bots. |
| `AiPlayerbot.ProfessionProgressionPercent` | `10` | `0`–`100` | Selects a stable percentage of eligible free bots to participate. `0` disables participation; `100` includes every otherwise eligible bot. |

Participation uses the existing deterministic bot-identity hash. It is stable
across ordinary updates and restarts, rather than rolling a new chance on every
AI update. It does not target names, accounts, realms or specific professions.
Ten percent means an approximate fraction of otherwise eligible bots, not an
exact promise of 150 active participants in a population of 1,500. Bots with an
active real-player master are excluded from autonomous progression.

Disabling these options does not erase learned professions, inventory or natural
skill gains. Existing gathering, fishing, optional RPG crafting and unrelated
economy systems have their own behavior. Corrective profession-assignment
metadata preservation also remains active independently of this switch.

## Planning and crafting

| Option | Default | Minimum | Effect |
| --- | --- | --- | --- |
| `AiPlayerbot.ProfessionPlanCheckInterval` | `60` seconds | `10` seconds | Sets the cached planning value's check interval. Higher values reduce periodic recipe work; lower values notice changed circumstances sooner. |
| `AiPlayerbot.ProfessionCraftBatchSize` | `5` casts | `1` | Maximum ordinary recipe casts planned together. This counts spell casts, not the number of output items. |
| `AiPlayerbot.ProfessionCraftCooldown` | `30` seconds | `1` second | Minimum elapsed time since the most recent accepted autonomous profession cast before another job can start. |
| `AiPlayerbot.ProfessionMaterialTarget` | `20` units | `1` | Bounds routine reagent demand/reserves and helps limit how large a crafting batch can be. It does not grant materials or require a full reserve before crafting. |

The shared PlayerBots `CalculatedValue` implementation evaluates periodic caches
at half their configured interval. Consequently, `ProfessionPlanCheckInterval =
60` normally permits a periodic recalculation after about **30 seconds**. Accepted
casts and request cleanup can invalidate the plan sooner. This option is neither
a guaranteed action frequency nor a hard minimum between recalculations.

A batch of five casts may create more than five items if the recipe creates
multiple items per cast. Available materials can reduce the batch: a bot able to
cast twice may proceed with two casts instead of waiting for all five. The
material target also reduces batches where a recipe consumes several units per
cast. For example, a recipe using six units per cast with a target of 20 normally
plans at most three casts. A single valid cast remains possible when its reagent
requirement exceeds the target; the target does not make the recipe impossible.

Tools are non-consumable prerequisites and normally need **one** compatible item,
regardless of batch size or material target. Item stack limits and live inventory
counts also constrain retention/acquisition; the material target is not a rigid
global cap on all items in the bot's bags.

Already-owned batch continuations retain their request and can proceed without
starting a competing job. Milling and Prospecting use **one processing cast at a
time**, then wait for normal loot handling and a fresh inventory observation.
Their random outputs are not assumed to exist when a cast is merely accepted.

## Vendor acquisition

| Option | Default | Minimum | Effect |
| --- | --- | --- | --- |
| `AiPlayerbot.ProfessionVendorPurchaseLimit` | `10` operations | `1` | Limits successful profession-supply purchase operations during one vendor interaction. |

This is an operation limit, not an item-unit limit. A vendor may sell several
units in one purchase pack. Purchases still require a usable vendor, real stock,
sufficient legitimate money and bag space. Existing trade-skill budget rules
remain authoritative. Tools target one compatible item rather than a full stack.

Setting this option to `0` does **not** disable vendor purchases: the loader
clamps it to `1`. There is no separate profession vendor-disable option in the
current configuration.

## Auction House acquisition and selling

| Option | Default | Accepted range | Effect |
| --- | --- | --- | --- |
| `AiPlayerbot.ProfessionAHSearchCooldown` | `600` seconds | At least `60` seconds | Minimum interval between this bot's profession-material AH search attempts. |
| `AiPlayerbot.ProfessionAHPurchaseLimit` | `2` auctions | At least `0` | Maximum successful auction buyouts during one profession buying interaction. `0` disables profession AH buying. |
| `AiPlayerbot.ProfessionAHBudgetPercent` | `10` | `0`–`100` | Caps one profession buying interaction's budget as a percentage of the bot's current money, also limited by its existing free trade-skill budget. `0` disables profession AH buying. |
| `AiPlayerbot.ProfessionAHMaxPriceMultiplier` | `1.25` | At least `0.1` | Maximum accepted unit price as a multiplier of `ItemUsageValue::GetBotBuyPrice` for that item and bot. |
| `AiPlayerbot.ProfessionAuctionPostLimit` | `3` stacks | At least `1` | Limits successful postings of the currently planned output item during one AH posting interaction. |

AH search attempts are recorded even when no suitable listing is bought, so an
empty or expensive AH cannot cause this profession path to rescan every AI tick.
The cooldown applies to profession-material searches; it is not a global throttle
on every existing AH feature.

The actual buying budget is:

```text
minimum(existing free trade-skill money,
        current bot money × ProfessionAHBudgetPercent / 100)
```

For example, a bot holding 100 gold with only 6 gold available for trade skills
has at most a 6-gold budget at 10%. This is a per-interaction allowance, not a
daily spending limit. Each successful buyout reduces the remaining budget.

The purchase limit counts **auction listings**, not individual material units.
Buying one affordable stack of ten items counts as one purchase. Whole auction
stacks must fit the bounded demand/reserve policy. The bot does not buy from its
own auctions, bid without a buyout, or accept arbitrary amounts just because the
unit price is cheap.

At `ProfessionAHMaxPriceMultiplier = 1.25`, the unit-price ceiling is 125% of the
existing bot-buy valuation. This valuation is not a guaranteed current market
average. The auction's total buyout must also fit the remaining budget, and normal
buyout handling remains responsible for payment and item delivery.

The posting limit applies only to the output item identified by the **current
plan**, which may be an intermediate while production is in progress. It does
not cap every AH posting made by the bot, guarantee a sale, or bypass existing
item-usage, deposit and profitability rules. `0` is clamped to `1`; disabling
profession AH buying does not independently disable ordinary AH selling.

To disable only profession AH buying, set either:

```ini
AiPlayerbot.ProfessionAHPurchaseLimit = 0
```

or:

```ini
AiPlayerbot.ProfessionAHBudgetPercent = 0
```

## Behavior these settings do not override

Progression still needs known recipes, legitimate materials, real tools and a
usable crafting location. Normal core spell casts create the items and award
natural skill gains. Setting participation to 100% or increasing a budget does
not grant recipes, gold, tools, items or profession ranks.

The planner can request up to two prerequisite production steps using learned
recipes and known processing effects. Examples include Milling → pigment → ink
for a final recipe, learned cloth bolts, smelted bars and engineering components.
It does not implement unlimited recursive production or arbitrary targeted
mob-drop farming. Longer chains and unsupported processing loot metadata can
still require direct gathering, vendor/AH acquisition or ordinary loot.

Existing bounded fairness gives waiting **ready** skills an opportunity after
five minutes. This duration is currently a policy constant, not another setting.
It does not make an unavailable reagent appear or guarantee a share of quest or
combat activity. Recipe scores choose a profession goal; the existing AI action
priorities still decide when an action can execute. Globally enabling `rpg craft`
is not required for the autonomous path.

## Existing configuration compatibility

The loader reads older dotted names as fallbacks. A canonical option always wins
when both names are present, including when its value is `0`.

| Canonical suffix after `AiPlayerbot.` | Legacy suffix after `AiPlayerbot.ProfessionProgression.` |
| --- | --- |
| `ProfessionProgressionEnabled` | `Enabled` |
| `ProfessionProgressionPercent` | `CanaryPercent` |
| `ProfessionPlanCheckInterval` | `PlanCheckInterval` |
| `ProfessionCraftBatchSize` | `CraftBatchSize` |
| `ProfessionCraftCooldown` | `CraftCooldown` |
| `ProfessionMaterialTarget` | `MaterialTarget` |
| `ProfessionVendorPurchaseLimit` | `VendorPurchaseLimit` |
| `ProfessionAHSearchCooldown` | `AHSearchCooldown` |
| `ProfessionAHPurchaseLimit` | `AHPurchaseLimit` |
| `ProfessionAHBudgetPercent` | `AHBudgetPercent` |
| `ProfessionAHMaxPriceMultiplier` | `AHMaxPriceMultiplier` |
| `ProfessionAuctionPostLimit` | `AuctionPostLimit` |

The historical percentage spelling is retained only for reading old files.
Use the canonical participation names in new configurations. Settings are loaded
through `PlayerbotAIConfig::Initialize`; this feature adds no automatic file
watcher or independent hot-reload mechanism.

## Checking a bot

The `profession` diagnostic reports participation, the bot's skills, its selected
execution step, any prerequisite goal/input, missing tools/materials and available
acquisition routes. On a server using the existing random-bot command interface:

```text
rndbot do <bot> profession
```

Check that the bot is reported active before expecting autonomous work at 10%.
Missing inputs, tools, money, recipes, an occupied pending cast or travel state
can still prevent progress. A recipe appearing in the plan is not proof that a
cast succeeded; verify actual inventory and natural skill changes over time.

Implementation and test evidence are in [the audit report](../profession-audit/REPORT.md).
Configuration loading is in [PlayerbotAIConfig.cpp](../playerbot/PlayerbotAIConfig.cpp),
with the distributed defaults in [aiplayerbot.conf.dist.in](../playerbot/aiplayerbot.conf.dist.in).
