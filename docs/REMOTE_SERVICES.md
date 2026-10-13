# Remote personal services for random bots

See the [profession build wiki](wiki/Home.md) for the component map, operating
guide and build/test reference for the complete feature.

This is an access option, not a new economy or profession planner. It lets an
autonomous random bot use its personal bank, mailbox and faction Auction House
without traveling to an NPC or mailbox. The existing item selection, reserve,
purchase budget, price checks, auction deposits/cuts, mail delivery/COD checks and
bank/bag capacity remain authoritative. No items or money are generated.

## Enablement and eligibility

```ini
AiPlayerbot.RandomBotRemoteServices = 1
AiPlayerbot.ProfessionProgressionEnabled = 1
AiPlayerbot.ProfessionProgressionPercent = 10
```

`RandomBotRemoteServices` is a Boolean; its default is **0**. Enabling it also
requires the [companion core patch](../patches/cmangos-wotlk-remote-services.patch).
Without that bridge, PlayerBots retains ordinary world interactions and travel,
even if this option is set. Do not enable GM commands or bot cheat flags instead.
The configuration loader logs this fallback when remote services are requested
without the compatible bridge.

Remote access applies to free random bots with artificial bot sessions and no
real-player master. It is independent of profession participation: all eligible
random bots may access services, while the existing profession planner still runs
only for its configured 10% cohort. Player-controlled bots, human players and
non-random alts retain ordinary access. Combat, death, taxi travel, teleportation,
active spells, trade, loot and pending autonomous crafting defer remote maintenance.

This does not enable remote guild-bank access, vendors, trainers, gathering or
crafting focuses. Recipes still require actual reagents, tools and appropriate
real world crafting objects. Existing active travel targets expire normally;
new AH/mail/bank RPG requests are suppressed for eligible remote bots.

## Existing actions and policies

A maintenance action checks at most once per minute and calls the existing
`ah`, `ah bid`, `mail take` and personal-bank helpers. The maintenance strategy
must be present, as it already must be for autonomous profession crafting.
The normal `rpg vendor`/`rpg bank` policy gates remain in effect. Remote manual
`ah`, `ah bid`, `mail` and `bank` commands use the same access option.

The dispatcher withdraws needed bank items, collects mail when the existing mail
policy requests it, makes eligible profession purchases, performs eligible
ordinary AH transactions and deposits eligible bank stock. Ordinary AH attempts
are limited to once per ten minutes per bot, including empty/rejected attempts.
Profession searches keep `ProfessionAHSearchCooldown`, purchase limits and budgets.
The shared auction mutex prevents overlapping bot auction transactions.

Remote access does **not** mean immediate collection: the existing one-to-four-hour
mail waiting policy still applies, including its existing guild-sharing/money
exceptions. It does not change the current gathering/vendor/AH source preference,
sell all gathered materials automatically or implement relisting/bank cooldowns.

## Core bridge and manual build

CMaNGOS validates auctioneer/mailbox access inside its normal session handlers.
The companion patch adds one narrowly scoped exception to each existing self-player
endpoint, plus a capability marker in `WorldSession.h`. Authorization is confined
to the owning bot, the selected service and the calling thread, and disappears
when the synchronous action returns or throws. Session security is never elevated.
The auction scope temporarily selects ordinary own-faction access and restores the
prior mode on exit; the core still determines auction pools and realm sharing.
Normal NPC/mailbox access and existing GM command permissions are unchanged.

The patch was checked against upstream WotLK core
`2fc8161eeddfce737766d449d0ff94f48b3df2e0`. This is not a verification of any live
core revision. Review/apply it to an **isolated compatible core checkout**, with
this PlayerBots checkout used by that build. Do not apply it over a dirty live
deployment or assume that updating only the PlayerBots module enables the bridge.

For an isolated Linux source/build, set these paths to your actual staging paths:

```sh
CORE=/path/to/isolated/mangos-wotlk
PLAYERBOTS=/path/to/this/playerbots
BUILD=/path/to/isolated/build

git -C "$CORE" apply --check "$PLAYERBOTS/patches/cmangos-wotlk-remote-services.patch"
git -C "$CORE" apply "$PLAYERBOTS/patches/cmangos-wotlk-remote-services.patch"
cmake -S "$CORE" -B "$BUILD" -DBUILD_PLAYERBOTS=ON \
  -DFETCHCONTENT_SOURCE_DIR_PLAYERBOTS="$PLAYERBOTS" \
  -DFETCHCONTENT_UPDATES_DISCONNECTED=ON
cmake --build "$BUILD" --target mangosd --parallel 2
```

Retain the other configuration options required by your existing core build.
Do not set the capability marker manually; all three core changes are required.
There is no automatic patch application, deployment, installation or restart.

## Validation

```sh
python3 -B "$PLAYERBOTS/playerbot/strategy/tests/RemoteServicesTests.py" \
  --compiler c++ --core-source /path/to/unpatched/compatible/mangos-wotlk
git -C "$PLAYERBOTS" diff --check
```

The test applies the patch to temporary copies only. It compiles actual scoped
authorization, core access/faction predicates and dispatch bodies with world doubles,
including eligibility, service/player/thread isolation, nesting, exception cleanup,
opt-out/unpatched fallback, busy-state guards and empty-attempt throttling. It
checks that dispatch still uses the real AH/mail/bank handlers. The audit also runs
`--baseline-ref 4541a89e` to confirm unchanged transaction and profession-planner
bodies; that optional comparison avoids imposing a historical planner on future
development. This does not replace a full compatible-core build or real transactions.

After building and deploying a compatible version, choose eligible random bots away from
service NPCs, record real bag/bank/mail/gold/auction state, and verify:

- A worthwhile selected stack becomes a real faction auction and pays its deposit.
- A budget-approved purchase deducts actual gold and creates normal delivery mail.
- Delivered mail becomes real bag items/money only when the existing policy allows it.
- Personal-bank deposits/withdrawals conserve quantities and respect capacity.
- Ordinary player sessions and controlled bots cannot use the scoped exception.
- Remote maintenance waits during combat/casts/trade/loot/pending profession work.
- Profession participation remains 10%, and focus-dependent crafting still uses
  the correct real object. Disabling remote access restores ordinary travel.

The current patch has only lightweight validation; full build and live behavior
remain unverified. No live configuration, databases, binaries or services changed.
