# Old PartyBot compatibility (`AiPlayerbot.PartyBotOnly`)

This document describes how the PlayerBots module covers the use case of the **deprecated in-core PlayerBot** (the "old playerbot" or "partybot", built with `BUILD_DEPRECATED_PLAYERBOT`, sources in `src/game/PlayerBot/`, docs in `doc/PlayerBot/`). It lists the configuration, what the compatibility mode changes, and every old command with its counterpart in this module.

Contents

1. [What the old system was](#1-what-the-old-system-was)
2. [Configuration reference](#2-configuration-reference)
3. [What `PartyBotOnly = 1` does](#3-what-partybotonly--1-does)
4. [Getting started (old workflow)](#4-getting-started-old-workflow)
5. [Command reference: old command to module command](#5-command-reference-old-command-to-module-command)
6. [Native commands added for old-system parity](#6-native-commands-added-for-old-system-parity)
7. [Known differences and limits](#7-known-differences-and-limits)
8. [Expansions](#8-expansions)
9. [Testing and debugging](#9-testing-and-debugging)

---

## 1. What the old system was

- Bots were **real player characters from the master's own account** (up to 9), logged in with `.bot add <character>` and controlled with whispers or party chat (`/t BOTNAME ...`, `/p ...`). There was no random bot population, no travel or questing agenda: a bot only followed, fought, looted and did what its master said.
- Orders used a fixed syntax such as `orders combat tank`, `follow far`, `pull test`, `survey`, `get [object]`, `process mill [herbs]`, `auction add [item]`.
- Extras: a "bot guy" NPC that sold bots, per-bot summon costs, a set of loot-collection flags (`collect`), and gameobject fetching (`survey`, `find`, `get`).

The PlayerBots module already shares the core-level login plumbing of the old system (`PlayerbotHolder`, `PlayerbotLoginQueryHolder`), so bots added by a player are the same kind of characters. What the module lacked for a partybot user was:

- a quiet profile (player-added bots used to run the full strategy tree),
- the old command spellings,
- a handful of features (object fetching, mill/prospect, combat orders by name, resistance orders, costs, auction commands).

All of these are now covered by the switch described below and by new native commands that work in every mode.

---

## 2. Configuration reference

All settings live in `aiplayerbot.conf` (template: `aiplayerbot.conf.dist.in`). A restart is required after changing `PartyBotOnly`. `.rndbot reload` re-reads the file at runtime, but bots that are already online keep the agenda they were created with.

| Key | Default | Meaning |
|---|---|---|
| `AiPlayerbot.PartyBotOnly` | `0` | Master switch for the compatibility mode. See [section 3](#3-what-partybotonly--1-does). `0` leaves every behaviour unchanged. |
| `AiPlayerbot.BotAddCost` | `0` | Copper charged to a **non-GM player** for each bot added with `.bot add` or `.bot login`. Charged once the bot is actually online. Never applies to random bots, autologin or console adds. Negative values are treated as `0`. |
| `AiPlayerbot.BotSummonCost` | `0` | Copper charged to a **non-GM player** each time the `summon` command brings a bot to them. Applies on top of `AiPlayerbot.NonGmFreeSummon`. Meeting stone rituals and short-range unstuck teleports are free. |
| `AiPlayerbot.LootDistanceMax` | `50.0` | Upper limit (yards) for the per-bot loot distance a player can set with `collect ... distance:N` or `range loot`. |

Existing settings that interact with the mode:

| Key | Interaction |
|---|---|
| `AiPlayerbot.AllowMultiAccountAltBots` | Ignored while `PartyBotOnly = 1`: only the requesting player's own account is allowed. |
| `AiPlayerbot.NonCombatStrategies` | Filtered for player-added bots while `PartyBotOnly = 1` (see section 3). |
| `AiPlayerbot.NonGmFreeSummon` | Lets non-GM players summon without a meeting stone or innkeeper. `BotSummonCost` still applies. |
| `AiPlayerbot.SightDistance` | Search radius used by `survey`, `find` and `get`. |
| `AiPlayerbot.LootDistance`, `AiPlayerbot.GroupMemberLootDistance` | Default loot distances. A per-bot value set with `collect`/`range loot` overrides them. |
| `AiPlayerbot.CommandSeparator` | Separator for multiple commands in one message. The old-wording translation does not rely on it. |

Money values are in copper: `10000` = 1 gold, `100` = 1 silver.

### Example: classic partybot setup

```
AiPlayerbot.Enabled = 1
AiPlayerbot.PartyBotOnly = 1
AiPlayerbot.BotAddCost = 0
AiPlayerbot.BotSummonCost = 0
```

---

## 3. What `PartyBotOnly = 1` does

One switch, four effects.

### 3.1 No bot population

The random bot system is switched off completely: spawning, login churn, wandering, guild, quest and battleground agendas, free bots and offline-group bots. After startup `.rndbot stats` reports `0 online` until a player adds a bot. The player-facing bot machinery (sessions, per-bot updates, `.bot add`) keeps working normally.

### 3.2 Own-account adds only

`.bot add` / `.bot login` accept only characters of the **requesting player's own account**, like the old system. Characters of other accounts, random-account characters and multi-account alt bots are refused with `Not in your account`. `.bot create` brings the new character online immediately (subject to `BotAddCost`).

### 3.3 Quiet agenda for player-added bots

A bot that has a real player as master gets this non-combat set:

- class strategies (talent spec, `dps assist` / `tank assist`, `cure`, `buff`, `boost` where the class has them), `wbuff`, `avoid mobs`, `dungeon`
- `racials`, `nc`, `food`, `follow` (or `wander` when configured), `default`, `quest` (quest sharing), `loot`, `collision`, `buff`, `mount`
- `return` and `delayed roll` from `AiPlayerbot.NonCombatStrategies`
- `rpg jump` and `follow jump` if the corresponding jump options are enabled

Not loaded: `grind`, `travel`, `tfish`, `gather`, `maintenance`, `free`, `bg`, `lfg`, `start duel`, `attack tagged`, `dksquest`, every `rpg ...` strategy except `rpg jump`, duels and emotes.

Strategies **saved in the database** for the character are filtered the same way when the bot logs in, so an alt that was once a random bot does not bring its old agenda back. Removals (`-strategy`) are never filtered. You can still switch a strategy on by hand (`nc +grind`); it is filtered again at the next login.

Random bots, bots without a real player master and every run with `PartyBotOnly = 0` are unaffected.

### 3.4 Old command wording

The old whisper spellings listed in [section 5](#5-command-reference-old-command-to-module-command) are rewritten to module commands. The rewrite is plain text only; permission checks run afterwards exactly as for any other command. The native module commands from [section 6](#6-native-commands-added-for-old-system-parity) work in **all** modes.

---

## 4. Getting started (old workflow)

1. Set `AiPlayerbot.PartyBotOnly = 1` and restart the server.
2. Log in with your account and add bots from your own characters: `.bot add Name` (several: `.bot add Name1,Name2`).
3. Whisper or use party chat: `/t Name follow`, `/p attack`. Party chat commands are obeyed by every bot in the group, as in the old system.
4. Give orders: `orders combat tank`, `orders delay 3`, `pull test`, `survey`, ...
5. `.bot remove Name` (or `logout`) when done.

Bot ownership, grouping, invitations and master detection are unchanged from the standard module behaviour.

---

## 5. Command reference: old command to module command

Legend: **alias** = the old spelling is rewritten only while `PartyBotOnly = 1`; **native** = the same words already work in every mode; **changed** = works but behaves differently from the old bot.

### 5.1 Lifecycle

| Old | Module | Notes |
|---|---|---|
| `.bot add <char>` | `.bot add <char>` | native. With `PartyBotOnly = 1`: own account only. |
| `.bot remove <char>` | `.bot remove` / `logout` | native |
| Auto-accept invites, rejoin group | handled by the module | native |
| Bot guy NPC (bots for sale) | not implemented | See [section 7](#7-known-differences-and-limits). |
| Cost per bot / per summon | `AiPlayerbot.BotAddCost`, `AiPlayerbot.BotSummonCost` | see [6.7](#67-costs) |

### 5.2 Movement and basic orders

| Old | Module | Notes |
|---|---|---|
| `attack`, `assist`, `stay`, `follow` | same words | native |
| `follow far` / `follow near` | `range follow <distance>` | alias. Moves the follow distance by 0.5 yd per command. `range follow ?` shows the value. |
| `follow reset`, `follow auto`, `follow info` | `range follow ...` | the module has no automatic variable follow distance; use `range follow` directly |
| `pull` | `pull` | native |
| `pull test` | `pull capability` | alias. Reports whether the bot can pull and with which ability, even if the `pull` strategy is off ("I can pull with serpent sting."). |
| `pull ready` | `pull readiness` | alias. Checks your selected target: range, line of sight, ability. |
| `neutralize` | `cast <crowd control spell>` | alias, class aware: mage Polymorph, priest Shackle Undead, druid Hibernate, hunter Scare Beast. Other classes get no reply. Needs the spell to be known. |
| `reset` | `reset` | native: resets states and orders |
| `spells`, `cast`, `c` | same | native |
| `stats` | `stats` | native |

### 5.3 Combat orders

All `orders combat ...` spellings are **alias** and need `PartyBotOnly = 1`. The first old-style order after a restore point saves the bot's current combat setup, so `resumeorders` can bring it back.

| Old | Module | Notes |
|---|---|---|
| `orders combat tank` / `maintank` | `co +tank` | Primary orders replace each other, like the old bot. |
| `orders combat heal` / `mainheal` | `co +heal` | |
| `orders combat notmainheal` | `co +offheal` | closest module equivalent |
| `orders combat assist` | `co +dps assist` | |
| `orders combat passive` | `co +passive` | |
| `orders combat nodispell` | `co +heal` | **changed.** The old order was a *secondary* order (heal, never dispel). Here it adds `heal` without clearing other roles and does not stop dispelling. |
| `orders combat assist <NAME>` | `order assist <NAME>` | see [6.1](#61-named-assist-and-protect) |
| `orders combat protect <NAME>` | `order protect <NAME>` | see [6.1](#61-named-assist-and-protect) |
| `orders combat resistfire` / `resistfrost` / `resistnature` / `resistshadow` | `resist <school>` | see [6.2](#62-resistance-orders). Also accepted with a space: `orders combat resist fire`. |
| `orders combat reset` | clears primary orders, the named assist/protect target and the resist order | |
| `orders delay <n>` | `wait for attack time <n>` | The module accepts a wider range than the old 0 to 10 seconds. |
| `orders` (show orders) | `order`, `co ?` | different spelling, not translated. `order` reports the named assist/protect state, `co ?` lists all combat strategies. |
| `resumeorders` | `co restore` | alias. Restores the snapshot taken by the first order. |

Native module equivalents work in every mode: `co +tank`, `co -tank`, `co ?`, `co save`, `co restore`, `order ...`, `resist ...`, `wait for attack time N`.

### 5.4 Looting and gameobjects

| Old | Module | Notes |
|---|---|---|
| `collect` | `collect` | native, see [6.3](#63-collect) |
| `collect <flags> distance:N` | same | native |
| `survey` | `survey` | native, see [6.4](#64-survey-find-get) |
| `find [object]` | `find [object]` | native |
| `get [object]` | `get [object]` | native |

### 5.5 Items, professions and services

| Old | Module | Notes |
|---|---|---|
| `use [item]`, `u` | same | native (items, quest items, containers, on target or object) |
| `equip [item]`, `e` | same | native. `equip auto/info` toggles are not provided; the module decides gear upgrades itself. |
| `sell [item]` | `sell` / `s` | native (see also below) |
| `sell all` | `sell` of junk | alias, see below |
| `buy [item]`, `drop [item]` | `buy`, `destroy` / `d` | native |
| `repair`, `repair all` | `repair` | native |
| `craft <profession>` | `craft <profession>` | native: lists learnt recipes as links. Accepts `alchemy a`, `blacksmithing b`, `cooking c`, `engineering e`, `firstaid f`, `leatherworking l`, `magic m` (enchanting), `smelting s`, `tailoring t`. |
| `craft [recipe]` | `craft [recipe]` | native |
| `process disenchant [item]` (`process d`) | `cast 13262 [item]` | alias |
| `process mill [item]` (`process m`) | `mill [item]` | alias, see [6.5](#65-mill-and-prospect) |
| `process prospect [item]` (`process p`) | `prospect [item]` | alias, see [6.5](#65-mill-and-prospect) |
| `enchant [spell][item]`, `nt` | enchanting through trade and item-for-spell actions | native, behaviour unverified against the old flow |
| `auction`, `auction add`, `auction remove` | `auction ...` | native, see [6.6](#66-auction) |
| `mail inbox/getcash/getitem/delete` | `mail ?`, `mail take`, `mail delete`, `mail read` | native, different spelling. Note: a mail with both money and items is handled money first; check the result. |
| `bank`, `bank deposit/withdraw` | `bank [+item]` / `bank [-item]` | native |
| `talent ...`, `skill ...` | `talents`, `trainer`, `skill` | native, different spelling |
| `quest ...` | `q`, `quests`, accept/drop/reward | native, different spelling |
| `pet ...` | `pet ...` | native (call, dismiss, stay, follow, attack, autocast, aggressive/defensive/passive). Taming is not available. |
| Money request while trading (`10g`, `6g500s25c`) | `10g` | native: whisper the amount with a trade window open |
| `help` | `help` | native |

`sell all` (alias): sells **only junk** to a vendor in range: grey items and items the bot cannot use that have a sell price. It never sells equipped items, quest items or anything the bot could use or still needs. Replies `Sold N items for <money>`, `Nothing to sell.` or `I need a vendor nearby.` The module's own `sell` family is unchanged.

---

## 6. Native commands added for old-system parity

These commands work in every mode (`PartyBotOnly = 0` as well). Only the old spellings in the tables above need the switch.

### 6.1 Named assist and protect

```
order assist <name>
order protect <name>
order clear
order
```

- **assist**: the bot attacks the target of the named group member while that member is in combat.
- **protect**: the bot attacks a non-crowd-controlled attacker whose victim is the named member (damage dealers pick the lowest-health attacker, tanks the one with the lowest own threat). Healers do not take the protect role.
- Adds `dps assist` unless the bot already has a tank or dps assist strategy.
- `order` on its own reports the current state, `order clear` removes it.
- The setting is saved per bot and is part of `co save` / `co restore`.
- The named member must be in the bot's group.

### 6.2 Resistance orders

```
resist fire | frost | nature | shadow
resist none        (also: clear, reset, off)
resist
```

Per class:

| Class | fire | frost | nature | shadow |
|---|---|---|---|---|
| Paladin | `aura fire` | `aura frost` | none | `aura shadow` |
| Shaman | Fire Resistance Totem | Frost Resistance Totem | Nature Resistance Totem | none |
| Hunter | none | none | `aspect wild` | none |
| Other classes | gear only | gear only | gear only | gear only |

- The automatic aura / totem / aspect choice is saved before the order and **restored** when you clear or switch it.
- **Gear:** if the bot has a saved outfit named `<school>`, `<school>res`, `resist<school>` or `<school>resist` (for example `fire` or `fireres`, created with the `outfit` command), it equips it. The current gear is first saved as the outfit `noresist` and put back when the order is cleared. No gear swap happens in combat; the strategy part still applies and the reply says gear will not change.
- While a resist outfit is worn, automatic gear upgrades are paused so the resist set is not replaced.
- The order is saved per bot and is part of `co save` / `co restore`.

### 6.3 `collect`

```
collect
collect [none | all | combat | loot | quest | profession | objects | skin] [distance:N]
```

Flags are **additive**, as in the old bot. `collect` alone reports the current state.

| Flag | Effect |
|---|---|
| `combat` | loot after combat (non-combat `loot` strategy) |
| `loot` | grab all loot from corpses |
| `quest` | only quest items |
| `profession` | profession-related loot |
| `objects` | gather objects (non-combat `gather` strategy) |
| `skin` | informational; skinning follows the bot's skill |
| `all` | everything above |
| `none` | removes loot and gather behaviour |
| `distance:N` | per-bot loot distance in yards, capped by `AiPlayerbot.LootDistanceMax`. Same as `range loot N`. |

### 6.4 `survey`, `find`, `get`

```
survey
find [gameobject link]
get  [gameobject link]
```

- **survey** lists, in one line of clickable links and nearest first, the objects within `AiPlayerbot.SightDistance` that the bot could actually collect: ore and herbs it has the skill (and pick) for, chests, and quest objects it needs. Without any: `I see nothing to collect around here.`
- **find** walks to a spot about 2.5 yards from the chosen object and **stays** there until you give a movement order (`follow`, `stay`, ...).
- **get** checks the bot can collect the object (the reply names the reason if not: no skill, skill too low, no mining pick, no key, someone else is using it, nothing to take), then:
  1. stops following (remembering whether it was following, staying, guarding or wandering),
  2. walks there, gathers or opens the object and loots it,
  3. goes back to the remembered movement mode (follow your master by default).
  
  `get` ends when the object is no longer collectable, after it has been opened once (multi-use ore veins and herbs continue until empty), or after **90 seconds** out of combat (`I could not get [..], coming back.`). Fights on the way pause it. Giving `follow`, `stay`, `guard` or `wander` cancels it.
- In party chat every bot in range answers `survey`.

### 6.5 `mill` and `prospect`

```
mill [herb link]
prospect [ore link]
```

Without a link the bot picks its largest qualifying stack. Requirements: the bot knows the spell (Inscription / Jewelcrafting), the item is flagged millable / prospectable, the required skill is high enough, and a **single stack of at least 5**. Not usable in combat or while flying; the bot dismounts and leaves shapeshift forms first.

Availability: prospecting exists from TBC, milling from WotLK. In older versions the bot replies that it is not possible in this version of the game.

### 6.6 `auction`

```
auction                             (or: auction list)
auction add [item link] [price]
auction remove [item link]          (or: auction remove #<id> / <id>)
```

- **list** works anywhere: id, item and count, current bid (or minimum bid), buyout, time left and auction house, up to 15 lines.
- **add** needs an auctioneer in range. The price is the buyout for the whole stack; a bare number means **gold** (`5` = 5g, `5g50s`, `50s`, `5g 50s`). The starting bid is 95% of the buyout. Without a price the bot uses the same automatic pricing as `ah vendor`. The bot checks it can pay the deposit first.
- **remove** needs an auctioneer in range and cancels the bot's own auctions through the normal client path (deposit and mail rules apply). An item link cancels every matching auction **that has no bid**; an id cancels exactly one. Auctions with a bid are refused.
- Differences to the old bot: no queue that waits until the next auctioneer is met, no clickable auction link (use the `#id` from the list).

### 6.7 Costs

`AiPlayerbot.BotAddCost` and `AiPlayerbot.BotSummonCost` (section 2).

- **Add:** the cost is checked when you type `.bot add` (money already reserved for other bots that are still logging in counts), and taken when the bot is online with the message `Paid <amount> to add <bot>.` If you cannot pay: `You need <amount> to add a bot.` and nothing happens. A bot that logs in while you can no longer pay is logged out again.
- **Summon:** taken when the summon goes through; `You need <amount> to summon me.` otherwise.
- GM accounts are always free. Random bots, autologin, `.always` and console adds are free.
- With `PartyBotOnly = 1`, `.bot create` charges the add cost as well; if you cannot pay, the character is created but stays offline.

### 6.8 Pull checks and combat order snapshots

- `pull capability` / `pull readiness` are the native names behind `pull test` / `pull ready`.
- `co save` stores the bot's combat role strategies and attack delay, `co restore` brings them back, `co ?` lists the combat strategies.

---

## 7. Known differences and limits

- **Bot guy NPC and bot shop:** not implemented. It needs creature gossip and menu flows the module does not have. Use `.bot add`, optionally with `BotAddCost`.
- **`nodispell`** is approximated (see 5.3).
- **`follow auto/reset/info`:** no automatic variable follow distance.
- **`equip auto on/off/once`:** not provided; the module's usage calculator decides on gear.
- **Pet taming:** not available (factory-managed pets only).
- **Auction queue / auction links:** see 6.6.
- **Runtime flip of `PartyBotOnly`:** does not retire bots that are already online or change their agenda; restart the server.
- **Old wording needs the mode:** `orders combat ...`, `resumeorders`, `follow near/far`, `pull test/ready`, `neutralize`, `process ...` and `sell all` are only understood while `PartyBotOnly = 1`. Their native counterparts always work.
- **One command per bot and tick:** chat commands are single slots; two commands in the same instant for the same bot can overwrite each other. The old-order translations are single commands for exactly this reason.
- **Verified in game:** the commands were verified on live bots through the console; paths that need a real client master (agenda of an own-account alt after `.bot add`, `sell all` next to a vendor, real `get` return to the master, outfit swaps, costs, auction add/remove at an auctioneer, `neutralize` with the spell known) are covered by code review and should be checked on your server.

---

## 8. Expansions

All commands are expansion independent except:

| Feature | Vanilla | TBC | WotLK |
|---|---|---|---|
| `prospect` | not available | yes | yes |
| `mill` | not available | not available | yes |
| `auction` times | 8 h posts | 12 h posts | 12 h posts; sold auctions stay listed while payment is pending (shown as pending, not cancellable) |

Resist orders use the resistance strategies the class has in that expansion; combinations without a spell reply `no <school> resist spell for my class` and use gear only.

---

## 9. Testing and debugging

### 9.1 Quick checks (player side)

1. `.rndbot stats` shows `0 online` after the startup ramp.
2. `.bot add` of a character from another account answers `Not in your account`.
3. `.bot add` of your own alt; `/t Name nc ?` shows no `grind`, `travel`, `rpg ...`.
4. `orders combat tank`, then `co ?` (tank strategy present), `resumeorders` (original setup back).
5. `survey`, then `get [link]` on a nearby ore, herb or chest.
6. `pull test`, `orders delay 3`, `resist fire`, `resist none`, `auction`.

### 9.2 Console tools (RA / GM)

| Command | Use |
|---|---|
| `.rndbot cmd <bot> <command>` | Run a chat command on a bot as if parsed from chat (skips the old-wording translation). |
| `.rndbot cmd <bot> chat <text>` | Run `<text>` through the **real chat handler** of the bot, including the old-wording translation and permission checks. Pair with the two commands below to read the reply. |
| `.rndbot record <bot> enable` | Capture the bot's next reply. Re-enable before each command. |
| `.rndbot read <bot>` | Print the captured reply (wait about two seconds after the command). |
| `.rndbot reload` | Re-read the configuration (does not retire online bots). |
| `.rndbot debug <bot> why` / `history` | Why the bot does what it does; recent actions (enable with `.rndbot history on`). |

To test the old wording without a client: start with `PartyBotOnly = 0` and a few random bots, set the switch to `1` in the file, run `.rndbot reload`, then use `.rndbot cmd <bot> chat orders combat tank`.

### 9.3 Reading failures

- No reply to an old spelling: check the switch (`AiPlayerbot.PartyBotOnly = 1`) and that you are the bot's master or allowed to command it.
- `get` seems stuck: it pauses during fights and gives up after 90 seconds out of combat.
- `sell all` says `I need a vendor nearby.`: stand next to the vendor (interaction range) and target nothing special.
