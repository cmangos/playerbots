# PlayerBots profession build wiki

This wiki explains the profession additions in this fork, how they fit into
PlayerBots, and how to configure, use and validate them. It covers autonomous
profession progression and the optional remote personal-services feature.

**Source snapshot:** `13039f29` on `feature/playerbot-profession-economy`, including
upstream `14ccaa25`. This identifies the code reviewed for these pages; it does
not identify a binary installed on any realm. The newest production bridge and
remote-service changes still require a full compatible-core build and live
validation. Earlier successful profession casts do not validate every newer path.

## Contents

| Page | What it explains |
| --- | --- |
| [Architecture](Architecture.md) | The responsibilities of the planner, AI values, actions, travel and game core. |
| [Profession pipeline](Profession-Pipeline.md) | Recipe selection, fairness, prerequisite production, tools, focuses and craft ownership. |
| [Profession reference](Profession-Reference.md) | Dependencies to inspect for each crafting profession, gathering and advanced recipes. |
| [Configuration](Configuration.md) | Every feature option, defaults, examples and interactions. |
| [Usage](Usage.md) | How to enable progression, inspect a bot and verify a real result. |
| [Remote services](Remote-Services.md) | Optional bank/AH/mail access, eligibility, permissions and existing economy rules. |
| [Troubleshooting](Troubleshooting.md) | How to locate a blocker without confusing a plan with a successful cast. |
| [Build and tests](Build-and-Tests.md) | Isolated build instructions, lightweight checks and validation limits. |
| [Code reference](Code-Reference.md) | Source files, important functions and extension boundaries. |
| [Changes and limits](Changes-and-Limits.md) | What the merged build changes, upstream additions and remaining validation gaps. |

The shorter [configuration guide](../PROFESSION_PROGRESSION.md),
[remote-service guide](../REMOTE_SERVICES.md) and
[audit report](../../profession-audit/REPORT.md) remain available. The audit is a
historical evidence log; use this wiki and current source for present behavior.

## What this build adds

- A cached profession planner for eligible free bots with known recipes.
- Bounded sharing of crafting opportunities between ready runtime skills.
- Up to two prerequisite production steps through learned recipes and supported
  processing metadata, including WotLK Milling and Prospecting.
- Reagent reserves, partial batches and tool acquisition through existing systems.
- A maintenance route into the shared crafting action and ordinary game spells.
- Real spell-focus travel and validation, without enabling the entire optional
  `rpg craft` strategy.
- Optional remote personal-bank, own-faction AH and mailbox access.
- Regression coverage and private profession diagnostics.

## What it does not promise

The feature does not grant items, money, recipes or natural skill points. It does
not guarantee that every profession can advance on every database: learned
recipes, raw inputs, usable objects, money, level and trainer availability still
matter. There is no arbitrary-depth production planner, targeted worldwide
mob-drop farming, or failed-auction/bank-cooldown lifecycle.

The remote-service bridge deliberately relaxes **service proximity** when enabled.
It does not relax crafting tools, reagents, spell focuses, auction fees, mail
delivery or storage capacity. Remote services default to **off**.

## Quick start

For an existing compatible PlayerBots installation, use these canonical names in
the active `aiplayerbot.conf`:

```ini
AiPlayerbot.ProfessionProgressionEnabled = 1
AiPlayerbot.ProfessionProgressionPercent = 10
AiPlayerbot.RandomBotRemoteServices = 0
```

Inspect a bot through the existing administrative command interface:

```text
rndbot do <bot> profession
```

`<bot>` is a placeholder for an online bot name. Names are diagnostic addresses,
not implementation conditions. For console versus in-game command formatting,
see [Usage](Usage.md). A bot reported inactive at 10% may simply be outside the
stable participating cohort.

## Portability rule

Craft decisions derive from learned spells, skill metadata, inventory, tools,
focus objects and current runtime state. There are no sampled-name, account,
realm or sampled-recipe exceptions in the new progression policy. Stable
participation hashes the bot's identity; it does not compare against a special
GUID list. Database and expansion metadata determine which paths are possible.
