# Profession reference

[Wiki home](Home.md) | [Profession pipeline](Profession-Pipeline.md) | [Troubleshooting](Troubleshooting.md)

This page lists typical dependencies to inspect. It does not encode a different
algorithm for each profession and does not claim that the same starting recipes
are learned on every realm. The spell book and compatible spell/item/loot data
are authoritative.

## Crafting professions

| Profession | Inputs and prerequisites to inspect | How the shared implementation helps | Remaining natural blockers |
| --- | --- | --- | --- |
| Alchemy | Herbs, vendor reagents and learned potions/elixirs | Direct reagent planning, ordinary acquisition and shared spell casting | Missing herbs/recipes, stock, money or current readiness |
| Blacksmithing | Ore versus bars, learned smelting, explicit/category tools and forge/anvil metadata | Learned bar producers and common tool/focus/acquisition routes | No learned producer, insufficient ore, deeper chain, inaccessible focus or supply |
| Engineering | Bars, cloth/stone, learned components, hammer/spanner categories and recipe focus | Bounded component production, one-copy tools and ready-skill fairness | Nested chain beyond the bound, unavailable materials/tools or rank/recipe |
| Inscription | Milling inputs, pigments, learned inks, parchment and ink-set category | Known processing effects, one real loot cycle, learned ink production and tool/vendor paths | Unsupported loot metadata, random output deficit, missing herbs/ink recipe, tool/stock/money |
| Jewelcrafting | Ore/gems/stone, learned Prospecting, processed components and jeweler-tool category | Known processing effects, learned producers, valid-stack targeting and tool/fairness paths | Missing skill/input/producer, random gem supply, unsupported source or supply |
| Tailoring | Cloth versus bolts, thread and learned bolt producers | Grey learned bolt production can serve a non-grey goal; common vendor and fairness routes | No learned bolt recipe, insufficient cloth/thread, longer chain or training |
| Leatherworking | Leather versus processed leather, learned producers and vendor thread | Existing gathering/ordinary loot, learned inputs and vendor routes | Skinning/loot availability, missing recipe, stock and money |
| Enchanting | Learned item-producing recipes, dust/essences, disenchantable gear and enchant targets | Shared item-craft planning where eligible; existing enchanting/disenchanting maintenance remains separate | Reagent sustainability, useful target/gear, recipe and money availability |
| Cooking | Food inputs, vendor supplies and each spell's real focus requirement | Ordinary loot/acquisition plus CraftingFocus travel/WORK bridge | Missing food, inaccessible real focus, travel/AI activity or rank/recipe |
| First Aid | Cloth, learned bandages and any recipe requirements | Direct recipe/material/batch route and cross-skill fairness | Cloth supply, rank/recipe availability and other readiness gates |

Smelting is a learned recipe path; having Mining alone is not proof that a required
bar is available. A herb stack is not pigment, pigment is not ink, cloth is not a
bolt, and ore is not a gem. The planner must follow a supported learned producer
or acquire the actual required item.

## Enchanting is a separate comparison

`MaintenanceStrategy` already registers `disenchant random item` and
`enchant random item` in addition to the new autonomous profession trigger.
`EnchantSpellsValue` and existing enchant/target logic have their own path.

An observed Enchanting skill increase therefore does not, by itself, prove the
new item-producing planner ran. Determine which action executed, where its dust
or essence came from, what target was used, and whether the input supply is
sustainable. The generic planner is not a universal replacement for existing
enchant-target logic.

## Gathering and Fishing

Mining, Herbalism, Skinning and Fishing already have their own progression and
interaction systems. The profession feature can request practical destinations
as reagent sources, but a gathering skill increase is evidence of that gathering
path, not evidence of `craft random item` -> `castnc` execution.

Gathering supply still depends on the bot owning the appropriate skill, a
practical destination, available objects/creatures and successful normal
interaction. Ordinary looting remains relevant for cloth, food, leather and gear.

## Advanced recipes and ranks

The same metadata-driven path applies after the initial recipes. More advanced
recipes can introduce new tool categories, focus requirements, vendors and nested
inputs. A newly learned requirement is discovered through learned spell metadata;
there is no list of sampled recipes that alone receives tool recognition.

Common rank caps such as 75, 150, 225, 300, 375 and 450 are game/expansion data,
not an unconditional loop in this feature. The core trainer-state predicate and
existing training route decide which next rank or recipe can be learned. Recipes
outside the bounded production graph may require actual direct supply even after
the relevant rank is learned.

## Historical observations versus current guarantees

Earlier population evidence showed Alchemy, Leatherworking, Cooking and Enchanting
increasing while several other crafting skills remained near their starting
values. That established that some normal spell/skill paths worked; it did not
identify a single cause for every bot or validate all later changes.

The current source addresses demonstrated generic dispatch, ownership, focus,
quantity, tool, fairness and bounded-production gaps. The remaining requirement
is to trace each affected profession on the actual rebuilt realm. Use the audit
report for dated evidence and the usage checklist for new observations.
