# Configuration

[Wiki home](Home.md) | [Usage](Usage.md) | [Complete option reference](../PROFESSION_PROGRESSION.md)

Options belong in the active `aiplayerbot.conf`, not `mangosd.conf`. These are
canonical names consistent with the surrounding PlayerBots configuration style.
Configuration is read through `PlayerbotAIConfig::Initialize`; this feature adds
no file watcher or independent hot-reload command. Editing a file does not prove
that a running process has read it.

## Default feature block

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
AiPlayerbot.RandomBotRemoteServices = 0
```

## Meaning and units

| Option suffix after `AiPlayerbot.` | Default | Meaning |
| --- | --- | --- |
| `ProfessionProgressionEnabled` | `1` | Enables this autonomous progression policy for eligible bots. |
| `ProfessionProgressionPercent` | `10` | Stable cohort percentage, clamped to 0-100; not craft speed. |
| `ProfessionPlanCheckInterval` | `60` | Planning cache interval in seconds, minimum 10. Shared cache semantics normally reevaluate after half this interval. |
| `ProfessionCraftBatchSize` | `5` | Maximum ordinary casts per batch, minimum 1. Actual stock can reduce this. |
| `ProfessionCraftCooldown` | `30` | Seconds since the latest accepted autonomous cast before a new job, minimum 1. Owned continuations are distinct. |
| `ProfessionMaterialTarget` | `20` | Routine material demand/reserve bound, minimum 1; neither free stock nor a required minimum inventory. |
| `ProfessionVendorPurchaseLimit` | `10` | Successful profession-supply purchase operations per interaction, minimum 1; a pack can contain several units. |
| `ProfessionAHSearchCooldown` | `600` | Seconds between profession AH search attempts, minimum 60. Empty/rejected searches count. |
| `ProfessionAHPurchaseLimit` | `2` | Successful buyout listings per interaction, minimum 0. Zero disables this buying path. |
| `ProfessionAHBudgetPercent` | `10` | Percentage of current money allowed for an interaction, clamped 0-100 and capped by free trade-skill money. |
| `ProfessionAHMaxPriceMultiplier` | `1.25` | Unit-price ceiling relative to existing bot-buy valuation, minimum 0.1. |
| `ProfessionAuctionPostLimit` | `3` | Successful postings of the current plan's output per interaction, minimum 1. Other existing selling remains separate. |
| `RandomBotRemoteServices` | `0` | Optional remote personal services for eligible random bots; requires the companion core bridge. |

For clamps, examples and all legacy aliases, see the
[complete option reference](../PROFESSION_PROGRESSION.md). Canonical keys take
precedence over historical dotted fallback names, including an explicit value of
zero. New configuration should use the names above.

## Common setups

### Keep profession participation at 10%

Use the default block. No global `+rpg craft` addition is required. Keep the
existing maintenance, travel and appropriate RPG acquisition/trainer strategies;
disabling those systems can remove actions the planner expects to reuse.

### Disable autonomous progression

```ini
AiPlayerbot.ProfessionProgressionEnabled = 0
```

This preserves real skills and items. It does not switch off every pre-existing
gathering/fishing/RPG/economy action. Remote services have a separate switch.

### Disable profession AH purchases

```ini
AiPlayerbot.ProfessionAHPurchaseLimit = 0
```

Alternatively set the AH budget percentage to zero. Setting vendor purchase
limit or auction post limit to zero does not disable them: both clamp to one.

### Enable remote services separately

```ini
AiPlayerbot.RandomBotRemoteServices = 1
```

This needs the compatible core bridge and rebuilt binaries. Without it the loader
logs a fallback and ordinary world access remains active. It does not alter the
10% profession cohort: remote services can apply to all otherwise eligible random
bots. Read [Remote services](Remote-Services.md) before using this option.

## Policy constants without configuration options

- Fairness opportunities use 300 seconds.
- Prerequisite exploration permits at most two prerequisite edges.
- Processing uses one real cast/loot cycle at a time.
- Remote maintenance attempts use a 60-second deadline.
- Ordinary remote AH attempts use a separate 600-second deadline.
- Pending craft recovery uses `max(120 seconds, expireActionTime / 1000 + 60)`.

These are not additional configuration keys. Do not invent similarly named
settings and expect the loader to recognize them.
