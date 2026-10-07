# Remote services

[Wiki home](Home.md) | [Configuration](Configuration.md) | [Detailed bridge guide](../REMOTE_SERVICES.md)

## Purpose

`AiPlayerbot.RandomBotRemoteServices` allows eligible random bots to use their
personal bank, own-faction AH and mailbox without traveling to those services.
It defaults to **0** and is separate from profession progression.

This feature does not implement the proposed surplus-to-AH, failed-listing retry,
bank-cooldown and final-liquidation lifecycle. It preserves the existing item
selection and economy policies while changing service access.

## Eligibility and timing

Eligibility requires a compatible core bridge, the enabled option, an artificial
bot AI/session, random-bot membership, free-bot status and no real-player master.
Player-controlled bots and ordinary player sessions retain their existing access.

Usability checks defer work during death, combat, taxi travel, teleportation,
non-melee casts, trade and active loot. Autonomous maintenance also waits for
pending profession crafting. Manual remote actions use the shared eligibility
and busy-state checks; the maintenance pending-craft gate is in the dispatcher.

The cached maintenance readiness value has its own short cache, but an explicit
per-bot attempt timestamp limits actual maintenance to once per 60 seconds. Even
an empty attempt advances that timestamp. Ordinary AH work has an additional
600-second attempt deadline. Profession AH searches retain their own configured
cooldown, budget and limits.

## Maintenance order

| Step | Existing mechanism | Gate/policy retained |
| --- | --- | --- |
| Withdraw needed bank stock | `BankAction::AutoWithdraw` | `rpg bank`, should-withdraw, actual inventory/capacity |
| Collect mail | `mail` with `take` | Existing can/should-get-mail and delivery/attachment policies |
| Buy profession supplies | `ah bid` with `profession` | Current plan, supply classification, budget, prices and cooldown |
| Ordinary AH work | `ah` / `ah bid` with `vendor` | Existing item usage and ordinary attempt deadline |
| Deposit eligible stock | `BankAction::AutoDeposit` | `rpg bank`, should-deposit and real bank capacity |

Mail still follows the existing one-to-four-hour waiting behavior and its
exceptions. Remote access does not mean immediate collection of every delivered
attachment. If a profession purchase is requested that cycle, ordinary buying is
not also issued by this dispatcher.

## Scoped authorization

`RemoteServiceAccess` is a synchronous RAII scope. It records the owning player
and service in thread-local state, restores the previous scope on exit and does
not elevate session security or grant GM status.

For auction scope, it temporarily selects the existing own-faction access mode
and restores the previous mode. The core still determines auction pools and any
realm-wide sharing configuration. A shared auction mutex bounds concurrent bot
auction transactions; RAII releases it even when an action throws.

The [companion patch](../../patches/cmangos-wotlk-remote-services.patch) modifies:

- `WorldSession.h`: a capability marker identifying the compatible bridge.
- `GetCheckedAuctionHouseForAuctioneer`: a scoped exception for the owning bot's
  existing self-player endpoint.
- `CheckMailBox`: the corresponding scoped mailbox exception.

Ordinary checks remain in force outside the scope. Personal-bank movement reuses
the existing core inventory helpers; no guild-bank exception is added.

## Transaction boundaries

Normal money, deposits/cuts, buyout handling, mail delivery/COD rules and bag/bank
capacity remain authoritative. The new code does not construct fake auctions,
create attachments, bypass delivery time or grant material balances.

Remote access suppresses redundant new AH/mail/service RPG travel requests for
eligible bots. It does not erase every previously active travel target on enablement.
Normal lifecycle expiry still applies. Vendor, trainer, gathering and crafting-focus
travel remain world interactions.

## Unsupported-core behavior

With the option enabled but no compatible bridge, configuration loading logs the
fallback and remote eligibility stays false. Normal service/travel paths remain
available. Setting the capability macro manually cannot substitute for the two
core access changes or compatible linkage.

This bridge was checked against WotLK core `2fc8161e`. That is a reference version,
not a promise that every core/expansion can apply it unchanged. The PlayerBots
opt-out path must compile and behave normally without the bridge.

## Usage and observation

1. Apply/review the bridge in an isolated compatible core and build with this
   PlayerBots source; see [Build and tests](Build-and-Tests.md).
2. Set `RandomBotRemoteServices = 1` in the active configuration through the
   normal controlled server procedure.
3. Choose an eligible free random bot away from the relevant NPC/mailbox.
4. Record real bags, bank, money, delivered mail and auctions.
5. Verify permitted transactions and conservation of items/money with normal fees.
6. Verify controlled bots and player sessions cannot gain scoped access, and that
   busy/pending maintenance waits.
7. Disable the option and verify ordinary world access is used again.

Existing manual commands are still `ah`, `ah bid`, `mail` and `bank`. Their
parameters/security remain the existing command interfaces; this feature adds no
new unrestricted service API. Read the detailed guide for the full validation list.
