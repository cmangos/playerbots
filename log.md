# Profession progression change log

This change log summarizes the profession feature. For configuration, architecture,
usage and validation, see the [profession wiki](docs/wiki/Home.md) and the
[technical review](profession-audit/REPORT.md).

## Profession state preservation

- Reconcile stored assignments with the professions the character actually knows.
- Preserve earned skill/rank values during random-bot initialization.
- Keep assigned primary and secondary skills at value 1; retain orphan cleanup.
- Include Inscription where WotLK supports it.
- Initialize only genuinely missing profession lines at their starting rank.

## Autonomous crafting

- Select a learned progression goal using cached spell and skill metadata.
- Preserve the reagent-completeness, skill-threshold and deficit scoring model.
- Derive whole-cast quantities from real inventory and bounded demand.
- Dispatch through MaintenanceStrategy into the shared CraftRandomItemAction.
- Route castnc through the common noncombat spell action independently of the
  optional experimental RPG crafting bundle.
- Recheck learned spell, materials, tools, focus and current casting before dispatch.
- Preserve batch continuation requests; expire abandoned, non-progressing retries.

## Tools, location and acquisition

- Recognize explicit tools and compatible tool categories from learned recipes.
- Request one missing non-consumable tool through existing vendor/AH actions.
- Travel to actual matching spell focuses through TravelMgr and its WORK handoff.
- Resolve a matching local focus with the core predicate and cached object list.
- Classify practical gathering/vendor supply and use AH fallback where appropriate.
- Bound AH searches, quantities, spending and output postings through existing policy.

## Fairness and intermediate production

- Preserve waiting time independently of plan-cache invalidation.
- Give competing ready skills bounded 300-second opportunities without named
  profession rotation or changes to score coefficients.
- Resolve learned producers through at most two prerequisite edges, rejecting cycles.
- Permit grey intermediates that supply a useful progression goal.
- Recognize WotLK Milling/Prospecting effects and eligible item/loot metadata.
- Process one real stack/cast/loot cycle before observing the resulting inventory.

## Optional remote personal services

- Default RandomBotRemoteServices to disabled.
- Reuse existing AH/mail/personal-bank actions and transaction policies.
- Require the compatible companion core bridge for remote access.
- Scope access to the current bot, service and thread; restore it on exit.
- Preserve costs, delivery, capacity and busy-state restrictions.
- Throttle attempts even when no transaction succeeds.

## Separate integration corrections

- Evaluate expired timed, non-forced travel targets on activity reads.
- Reject empty outgoing chat before reply/history handling.
- Restore test-delivery cleanup within TestContext::Reset.
- Use a named ItemQualifier for the existing non-const diagnostic formatter.
- Qualify the processing-target helper through ProfessionCraftingPlanValue.

## Configuration and documentation

Canonical AiPlayerbot options use the surrounding configuration style, with legacy
fallbacks and explicit-value precedence. Profession participation defaults to 10%.
The wiki covers architecture, configuration, commands, dependencies, tests and limits.

## Validation scope

Recorded lightweight results and the successful WotLK build of profession revision
be2fa8a4 are summarized in [REPORT.md](profession-audit/REPORT.md).
The later upstream Netherspite merge is present in the branch but was not part of
that build. Documentation edits do not establish a new full-build result.

Full Classic/TBC compatibility, sustained latest-revision progress for every
profession/rank and live remote transactions remain validation gaps. Real supply,
known producers, tools, money and usable locations remain prerequisites.
