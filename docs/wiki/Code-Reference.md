# Code reference

[Wiki home](Home.md) | [Architecture](Architecture.md) | [Build and tests](Build-and-Tests.md)

All paths below are relative links to the code in this repository. Core bridge
targets live in the separate CMaNGOS repository and are represented by the patch.

## Planning and policy

| File | Main responsibility / entry points |
| --- | --- |
| [CraftValues.h](../../playerbot/strategy/values/CraftValues.h) | Plan, source, tool, request and processing structures; cached/manual AI value declarations. |
| [CraftValues.cpp](../../playerbot/strategy/values/CraftValues.cpp) | `ProfessionCraftingPlanValue::Calculate`, source classification, live tools/reagents, focus resolution, pending recovery and processing targets. |
| [ProfessionProgressionPolicy.h](../../playerbot/strategy/values/ProfessionProgressionPolicy.h) | Small pure policies: participation, batches, cooldowns, pending leases, supply bounds, dispatch/location and acquisition gates. |
| [ProfessionCraftingFairness.h](../../playerbot/strategy/values/ProfessionCraftingFairness.h) | Per-runtime-skill ready-age ledger, owner opportunity and `FinishOpportunity`. |
| [ProfessionProduction.h](../../playerbot/strategy/values/ProfessionProduction.h) | Learned producer structures and depth-bounded `NextProductionStep`. |
| [ValueContext.h](../../playerbot/strategy/values/ValueContext.h) | Registration of AI values so existing strategies/actions can request them by name. |

## Dispatch and observation

| File | Main responsibility / entry points |
| --- | --- |
| [MaintenanceStrategy.cpp](../../playerbot/strategy/generic/MaintenanceStrategy.cpp) | `can craft profession` -> `craft random item`; remote readiness -> remote maintenance. Craft relevance is 1.2, remote-service relevance 1.1 at this snapshot. |
| [CastCustomSpellAction.cpp](../../playerbot/strategy/actions/CastCustomSpellAction.cpp) | Selected-plan dispatch in `CraftRandomItemAction`, focus/item targets, normal cast request, continuations and cleanup. |
| [ChatCommandHandlerStrategy.cpp](../../playerbot/strategy/generic/ChatCommandHandlerStrategy.cpp) | Common profession diagnostic and `castnc` routing. |
| [ChatTriggerContext.h](../../playerbot/strategy/triggers/ChatTriggerContext.h) | Chat trigger registration, including profession/cast command names. |
| [ProfessionStatusAction.cpp](../../playerbot/strategy/actions/ProfessionStatusAction.cpp) | Private status response with skills, selected execution step, prerequisite goal, deficits and routes. |
| [LootAction.cpp](../../playerbot/strategy/actions/LootAction.cpp) | Existing loot handling; matching processing-loot lifecycle feeds request completion. |
| [RpgStrategy.cpp](../../playerbot/strategy/generic/RpgStrategy.cpp) | Base RPG toggles, training strategy and optional craft/spell/item bundle. |
| [RpgSubActions.h](../../playerbot/strategy/actions/RpgSubActions.h) | Existing wrappers, including `RpgCraftAction` and `RpgTrainAction`. |

The 1.2/1.1 relevance values are action priorities, not billion-point recipe
scores. They cannot establish a universal priority over quests/combat, whose
actions, multipliers, states and prerequisites have their own engine rules.

## Travel, economy and training

| File | Main responsibility / entry points |
| --- | --- |
| [TravelMgr.cpp](../../playerbot/TravelMgr.cpp) | Destination behavior, CraftingFocus and generic deadline expiry on `TravelTarget::IsActive`. |
| [TravelValues.cpp](../../playerbot/strategy/values/TravelValues.cpp) | Purpose indexes, needs/should-travel values, profession/trainer requests and remote-service suppression. |
| [ChooseTravelTargetAction.cpp](../../playerbot/strategy/actions/ChooseTravelTargetAction.cpp) | Generic/named destination requests, source-driven gathering/vendor/AH requests and reset gates. |
| [TravelStrategy.cpp](../../playerbot/strategy/generic/TravelStrategy.cpp) | Existing travel request triggers/priorities, including trade trainers. |
| [ItemUsageValue.cpp](../../playerbot/strategy/values/ItemUsageValue.cpp) | Bounded chain-input reserves, metadata-derived tool usefulness and ordinary use/equip/sell policy. |
| [BuyAction.cpp](../../playerbot/strategy/actions/BuyAction.cpp) | Normal vendor transactions plus bounded missing profession/tool purchases. |
| [AhAction.cpp](../../playerbot/strategy/actions/AhAction.cpp) | Existing posting/buyout methods, profession budget/price/quantity limits and optional scoped access. |
| [MailAction.cpp](../../playerbot/strategy/actions/MailAction.cpp) | Existing mailbox finding and mail policies/handlers; scoped self-mailbox endpoint. |
| [BankAction.cpp](../../playerbot/strategy/actions/BankAction.cpp) | Existing personal-bank movement and automatic stock handling; successful command aggregation. |
| [TrainerValues.cpp](../../playerbot/strategy/values/TrainerValues.cpp) | Shared trainer map, core trainability, suitable trainer entries and costs. |
| [TrainerAction.cpp](../../playerbot/strategy/actions/TrainerAction.cpp) | Existing recipe/rank learning, costs and training policy. |

## Remote services and configuration

| File | Main responsibility / entry points |
| --- | --- |
| [RemoteServiceAccess.h](../../playerbot/RemoteServiceAccess.h) / [implementation](../../playerbot/RemoteServiceAccess.cpp) | Eligibility, busy-state gates, thread/player/service scope and temporary faction access mode. |
| [RemoteServicesAction.cpp](../../playerbot/strategy/actions/RemoteServicesAction.cpp) | Slow maintenance order, per-bot deadlines and reuse of existing AH/mail/bank actions. |
| [RpgTriggers.cpp](../../playerbot/strategy/triggers/RpgTriggers.cpp) | Existing RPG target policies with redundant remote service interactions suppressed. |
| [ActionContext.h](../../playerbot/strategy/actions/ActionContext.h) | Action registration for shared dispatch. |
| [Core bridge patch](../../patches/cmangos-wotlk-remote-services.patch) | WorldSession capability marker and scoped AH/mail core access exceptions. |
| [PlayerbotAIConfig.cpp](../../playerbot/PlayerbotAIConfig.cpp) / [header](../../playerbot/PlayerbotAIConfig.h) | Canonical options, legacy fallback parsing, clamps and unsupported-core notice. |
| [Common defaults](../../playerbot/aiplayerbot.conf.dist.in), [TBC defaults](../../playerbot/aiplayerbot.conf.dist.in.tbc), [WotLK defaults](../../playerbot/aiplayerbot.conf.dist.in.wotlk) | Distributed option descriptions/defaults. |

## Performance and cache boundaries

| Work | Boundary |
| --- | --- |
| Known recipe discovery | Existing cached spell-book value; not a full database scan each AI tick. |
| Plan calculation | Configured cache interval; normally half-interval evaluation in shared CalculatedValue, with event-driven invalidation. |
| Prerequisite exploration | At most two prerequisite edges; still branches over candidate learned producers within that bound. |
| Inventory quantities | Per-plan item-count memoization and fresh checks at dispatch; stale plan data cannot authorize a cast alone. |
| Tool categories | Shared item metadata index plus cached bot requirements/purchase selection. |
| Milling/Prospecting sources | Shared output-to-input loot metadata index; bot planning restricts processing consideration to demanded outputs. |
| Material routes | Cached source values and existing gathering/vendor/destination indexes. |
| Focus fallback | Existing cached local GameObject list and core focus predicate, not a global spawn scan per tick. |
| Fairness | Small per-bot map of currently ready runtime skills; persists across plan invalidation. |
| AH | Configured profession search deadline, separate ordinary remote deadline and bounded purchases. |
| Remote dispatcher | Explicit 60-second attempt gate, even when no transaction succeeds. |

These boundaries reduce repeated work for large bot populations, including roughly
1,500 active bots. They are source-level design properties, not a measured CPU/RAM
benchmark for every realm. Watch actual action rate, plan invalidation, AH scan
cost, metadata size and busy/pending behavior before raising participation.

## Extending this build

Before adding a mechanism, check whether an existing generic action can perform
it. Prefer a planner/value exposing demand to an existing action. Keep common
actions available independently of optional strategy bundles where appropriate.

Use spell effects, item/tool categories, skill lines, loot metadata and runtime
ownership. Do not add sampled bot names, selected GUIDs, realm assumptions or
recipe IDs merely to make an observation pass. Existing historical spell handling
in shared PlayerBots code is not permission to add new diagnostic-target hacks.

Keep a new economy lifecycle, arbitrary production-depth expansion, unresolved
AI-reset crash and unrelated AHBot behavior separate until evidence justifies a
specific integration. Test state transitions and failed paths as well as one
successful recipe.
