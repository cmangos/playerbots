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
            ['git', '-C', str(REPO), 'show', 'HEAD:playerbot/strategy/values/CraftValues.cpp'],
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
                      '(castCount - 1)', 'QueuePendingCraft(ai, spell)'):
            self.assertIn(token, continuation)
        self.assertIn('FinishOpportunity', cast)

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
        diff = subprocess.check_output(['git', '-C', str(REPO), 'diff', '--',
                                       'playerbot/TravelMgr.h', 'playerbot/strategy/actions',
                                       'playerbot/strategy/values'], text=True)
        additions = '\n'.join(line[1:] for line in diff.splitlines()
                              if line.startswith('+') and not line.startswith('+++'))
        additions += source('playerbot/strategy/values/ProfessionCraftingFairness.h')
        self.assertNotRegex(additions, r'(?i)Beanezoth|Feleil|Kedis|Amorniar|Roxanna|192\.168\.|SetSkill\s*\(|AddItem\s*\(|TeleportTo\s*\(|GetAccountId\s*\(|GetGUIDLow\s*\(')
        self.assertNotRegex(additions, r'(?:spellId|skillId|itemId)\s*==\s*[1-9][0-9]+')


if __name__ == '__main__':
    unittest.main(verbosity=2)
