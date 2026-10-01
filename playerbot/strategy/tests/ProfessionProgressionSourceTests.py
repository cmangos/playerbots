"""Source wiring checks; these do not substitute for compiled C++ regressions.

Run without a compiler or a CMaNGOS checkout. Assertions check the production
integration boundaries that the independent policy/component tests cannot see.
"""
import pathlib
import re
import subprocess
import unittest

from ProfessionProgressionComponentTests import extract


REPO = pathlib.Path(__file__).resolve().parents[3]
BASELINE = 'c0100429a9b80b3b571d663399ac8a278d91cfbf'


def source(path):
    return (REPO / path).read_text(encoding='utf-8')


class ProfessionSourceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.crafts = source('playerbot/strategy/values/CraftValues.cpp')
        cls.actions = source('playerbot/strategy/actions/CastCustomSpellAction.cpp')
        cls.plan = extract(cls.crafts, 'ProfessionCraftingPlan ProfessionCraftingPlanValue::Calculate')
        cls.dispatch = extract(cls.actions, 'bool CraftRandomItemAction::Execute')

    def test_score_formula_is_preserved(self):
        baseline = subprocess.check_output(
            ['git', '-C', str(REPO), 'show', BASELINE + ':playerbot/strategy/values/CraftValues.cpp'],
            text=True)
        marker = 'int64 score = candidate.missing.empty()'
        pattern = re.escape(marker) + r'.*?score -=.*?;'
        self.assertEqual(re.search(pattern, baseline, re.S).group(),
                         re.search(pattern, self.plan, re.S).group())

    def test_maintenance_uses_value_event_name_and_single_plan(self):
        maintenance = source('playerbot/strategy/generic/MaintenanceStrategy.cpp')
        self.assertIn('"val::can craft profession"', maintenance)
        self.assertIn('event.getSource() == "can craft profession"', self.dispatch)
        self.assertIn('spellIds.assign(1, professionPlan.spellId)', self.dispatch)
        self.assertIn('bot->HasSpell(professionPlan.spellId)', self.dispatch)
        self.assertLess(self.dispatch.index('CanDispatchProfessionPlan'),
                        self.dispatch.index('spellIds.assign'))

    def test_fresh_readiness_checks_precede_pending_queue(self):
        queue = self.dispatch.index('QueuePendingCraft')
        for check in ('bot->HasSpell(spellId)', 'professionPlan.GetMissingReagents(ai).empty()',
                      'CanCraftSpellValue::HasRequiredTools(pSpellInfo, bot)', 'if (!castCount)'):
            self.assertLess(self.dispatch.index(check), queue)
        readiness = extract(self.crafts, 'bool CanCraftProfessionValue::Calculate')
        for check in ('GetMissingReagents', 'HasPendingCraft', 'IsNonMeleeSpellCasted',
                      'IsCraftLocationReady', 'bot->HasSpell', 'HasRequiredTools'):
            self.assertIn(check, readiness)

    def test_pending_recovery_resets_without_self_deletion(self):
        recovery = extract(self.crafts, 'bool ProfessionCraftingPlanValue::HasPendingCraft')
        self.assertIn('IsPendingCraftExpired', recovery)
        self.assertIn('IsNonMeleeSpellCasted(false)', recovery)
        clear = extract(self.crafts, 'void ProfessionCraftingPlanValue::ClearPendingCraft')
        self.assertIn('RESET_AI_VALUE(bool, "can craft profession")', clear)
        self.assertNotRegex(clear, r'context->ClearValues\s*\(')

    def test_continuation_keeps_target_count_and_ownership(self):
        cast = extract(self.actions, 'bool CastCustomSpellAction::Execute')
        self.assertIn('KeepsPendingCraft(result, castCount)', cast)
        continuation = extract(cast, 'if (castCount > 1)\n        {')
        for token in ('castString(woTarget)', 'formatWorldobject(gameObjectTarget)',
                      '(castCount - 1)', 'QueuePendingCraft(ai, spell, true)'):
            self.assertIn(token, continuation)
        self.assertIn('FinishOpportunity', cast)

    def test_retry_lease_only_renews_after_accepted_cast(self):
        queue = extract(self.crafts, 'void ProfessionCraftingPlanValue::QueuePendingCraft')
        self.assertIn('ShouldRenewCraftLease(pending, spellId, acceptedCast)', queue)
        cast = extract(self.actions, 'bool CastCustomSpellAction::Execute')
        facing = extract(cast, 'if (woTarget != bot &&')
        self.assertIn('QueuePendingCraft(ai, spell)', facing)
        self.assertNotIn('QueuePendingCraft(ai, spell, true)', facing)

    def test_focus_uses_core_predicate_and_cached_local_objects(self):
        focus = extract(self.crafts, 'GameObject* ProfessionCraftingPlanValue::GetCurrentSpellFocus')
        for token in ('TRAVEL_STATUS_WORK', 'TravelDestinationPurpose::CraftingFocus',
                      'MaNGOS::GameObjectFocusCheck', 'matches(focus)', 'matches(nearby)',
                      '"nearest game objects no los"'):
            self.assertIn(token, focus)
        self.assertNotIn('GetGameObjectData', focus)
        self.assertNotIn('GetDestinations', focus)
        # The shared action must still enforce GO type and exact focus ID.
        self.assertIn('GAMEOBJECT_TYPE_SPELL_FOCUS', self.dispatch)
        self.assertIn('spellFocus.focusId != pSpellInfo->RequiresSpellFocus', self.dispatch)

    def test_quantities_use_whole_casts_and_one_inventory_pass(self):
        for token in ('BoundCraftBatch', 'AvailableCraftBatch', 'currentCounts[reagent]',
                      'spell->ReagentCount[reagent] * candidate.craftCount'):
            self.assertIn(token, self.plan)
        self.assertEqual(self.plan.count('GetInventoryItemsCountWithId('), 1)
        self.assertNotIn('std::min<uint32>(\n                spell->ReagentCount', self.plan)

    def test_tools_use_core_category_compatibility_and_normal_usage(self):
        tools = extract(self.crafts, 'bool CanCraftSpellValue::HasRequiredTools')
        self.assertIn('spell->Totem', tools)
        self.assertIn('player->HasItemCount(tool, 1)', tools)
        self.assertIn('player->HasItemTotemCategory(category)', tools)
        usage = extract(source('playerbot/strategy/values/ItemUsageValue.cpp'),
                        'bool ItemUsageValue::IsItemNeededForSkill')
        self.assertIn('"craft tool requirements"', usage)
        self.assertIn('IsTotemCategoryCompatiableWith', usage)

    def test_tool_acquisition_reuses_supply_vendor_and_auction_paths(self):
        supplies = extract(self.crafts, 'std::map<uint32, uint32> ProfessionCraftingPlan::GetMissingSupplies')
        self.assertIn('GetMissingReagents(ai)', supplies)
        self.assertIn('GetMissingTools(ai)', supplies)
        self.assertIn('std::max', supplies)
        sources = extract(self.crafts, 'ProfessionMaterialSources ProfessionCraftingPlan::GetMaterialSources')
        self.assertIn('GetMissingSupplies(ai)', sources)
        self.assertIn('professionPlan.GetMissingSupplies(ai)', source('playerbot/strategy/actions/BuyAction.cpp'))
        self.assertIn('plan.GetMissingSupplies(ai)', source('playerbot/strategy/actions/AhAction.cpp'))
        auction = source('playerbot/strategy/actions/AhAction.cpp')
        self.assertIn('profession::SupplyReserveTarget', auction)
        self.assertIn('missingTools.count(itemId)', auction)
        self.assertIn('plan.required.count(itemId)', auction)
        self.assertIn('reserveTarget(candidate->itemTemplate)', auction)
        self.assertIn('reserveTarget(auction->itemTemplate)', auction)

    def test_tools_request_one_with_live_ownership_and_replacements(self):
        missing = extract(self.crafts, 'std::map<uint32, uint32> ProfessionCraftingPlanValue::GetMissingTools')
        self.assertIn('requirements.NeedsItem', missing)
        self.assertIn('missing[itemId] = 1', missing)
        self.assertNotIn('professionCraftBatchSize', missing)
        self.assertNotIn('professionMaterialTarget', missing)
        needs = extract(self.crafts, 'bool CraftToolRequirements::NeedsItem')
        for token in ('HasItemCount(item->ItemId, 1)', 'HasItemTotemCategory(category)',
                      'IsTotemCategoryCompatiableWith'):
            self.assertIn(token, needs)
        item_usage = source('playerbot/strategy/values/ItemUsageValue.cpp')
        self.assertIn('recipeTools.UsesItem(proto)', item_usage)
        self.assertIn('recipeTools.NeedsItem(proto, bot)', item_usage)

    def test_tool_category_index_is_shared_not_scanned_per_bot(self):
        shared = source('playerbot/strategy/values/SharedValueContext.h')
        header = source('playerbot/strategy/values/CraftValues.h')
        self.assertIn('creators["craft tool items"]', shared)
        self.assertIn('SingleCalculatedValue<CraftToolItemMap*>', header)
        self.assertIn('"profession tool purchases", 30', header)
        choice = extract(self.crafts, 'std::set<uint32> ProfessionToolPurchasesValue::Calculate')
        self.assertIn('GAI_VALUE(CraftToolItemMap*, "craft tool items")', choice)
        self.assertIn('"craft tool requirements"', choice)
        self.assertNotIn('sItemStorage', choice)
        self.assertNotIn('profession crafting plan', choice)
        requirements = extract(self.crafts, 'CraftToolRequirements CraftToolRequirementsValue::Calculate')
        self.assertIn('"craft spells"', requirements)
        self.assertIn('"enchant spells"', requirements)
        self.assertIn('spell->Totem', requirements)
        self.assertIn('spell->TotemCategory', requirements)

    def test_missing_tools_can_request_vendor_before_focus(self):
        travel = extract(self.crafts, 'bool ProfessionCraftingPlanValue::ShouldTravelToSpellFocus')
        for token in ('professionVendorPurchaseLimit', 'GetMissingTools(ai)', 'ShouldTravelToVendor(ai, plan)'):
            self.assertIn(token, travel)
        self.assertLess(travel.index('ShouldTravelToVendor'), travel.index('profession::ShouldTravelToSpellFocus'))
        self.assertNotIn('SetTarget', travel)

    def test_fairness_survives_plan_invalidation(self):
        values = source('playerbot/strategy/values/ValueContext.h')
        header = source('playerbot/strategy/values/CraftValues.h')
        self.assertIn('creators["profession fairness"]', values)
        self.assertIn('ManualSetValue<profession::CraftingFairness&>', header)
        self.assertIn('fairness.Select(candidates', self.plan)
        self.assertIn('candidate.missing.empty() && CanCraftSpellValue::HasRequiredTools', self.plan)
        self.assertNotRegex(self.actions, r'ClearValues\("profession fairness"\)')

    def test_practical_vendor_filter_preserves_ah_fallback(self):
        sources = extract(self.crafts, 'ProfessionMaterialSources ProfessionCraftingPlan::GetMaterialSources')
        self.assertIn('IsPracticalProfessionVendor(entry, itemId, bot, travelInfo)', sources)
        self.assertIn('if (hasPracticalVendor)', sources)
        practical = extract(self.crafts, 'bool IsPracticalProfessionVendor')
        for token in ('IsHostileTo', 'GetNpcVendorItemList', 'GetNpcVendorTemplateItemList',
                      'GetEntryDestinations', 'IsPossible', 'IsLocationLevelValid'):
            self.assertIn(token, practical)
        self.assertNotIn('.GetDestinations(', practical)
        cash = extract(self.crafts, 'bool HasCashVendorStock')
        self.assertIn('item->ExtendedCost', cash)
        self.assertIn('profession::IsCashVendorStock', cash)
        self.assertIn('if (item->item != itemId)', cash)
        self.assertIn('return profession::IsCashVendorStock', cash)

    def test_destination_lookup_is_nonmutating_and_indexed(self):
        lookup = extract(source('playerbot/TravelMgr.h'), 'const DestinationList* GetEntryDestinations')
        self.assertEqual(lookup.count('.find('), 2)
        self.assertNotIn('destinationMap[', lookup)
        self.assertNotIn('for (', lookup)

    def test_common_castnc_route_is_retained(self):
        common = source('playerbot/strategy/generic/ChatCommandHandlerStrategy.h')
        rpg = source('playerbot/strategy/generic/RpgStrategy.cpp')
        self.assertIn('CastNcTrigger = "castnc"', common)
        self.assertIn('CastNcAction = "cast custom nc spell"', common)
        self.assertNotIn('"castnc"', rpg)

    def test_added_production_code_has_no_sample_or_cheat_branches(self):
        diff = subprocess.check_output(['git', '-C', str(REPO), 'diff', BASELINE, '--',
                                       'playerbot/TravelMgr.h', 'playerbot/strategy/actions',
                                       'playerbot/strategy/values'], text=True)
        additions = '\n'.join(line[1:] for line in diff.splitlines()
                              if line.startswith('+') and not line.startswith('+++'))
        additions += source('playerbot/strategy/values/ProfessionCraftingFairness.h')
        self.assertNotRegex(additions, r'(?i)Beanezoth|Feleil|Kedis|Amorniar|Roxanna|192\.168\.|SetSkill\s*\(|AddItem\s*\(|TeleportTo\s*\(|GetAccountId\s*\(|GetGUIDLow\s*\(')
        self.assertNotRegex(additions, r'(?:spellId|skillId|itemId)\s*==\s*[1-9][0-9]+')


if __name__ == '__main__':
    unittest.main(verbosity=2)
