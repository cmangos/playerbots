#include "botpch.h"
#include "../../playerbot.h"
#include "UseItemAction.h"

#include "../../PlayerbotAIConfig.h"
#include "DBCStore.h"
#include "../../ServerFacade.h"
#include "../values/ItemUsageValue.h"
#include "../../TravelMgr.h"
#include "CheckMountStateAction.h"

using namespace ai;

SpellCastResult BotUseItemSpell::ForceSpellStart(SpellCastTargets const* targets, Aura* triggeredByAura)
{
    WorldObject* truecaster = GetTrueCaster();
    if (!truecaster)
        truecaster = m_caster;
    m_spellState = SPELL_STATE_TARGETING;
    m_targets = *targets;

    if (triggeredByAura)
        m_triggeredByAuraSpell = triggeredByAura->GetSpellProto();

    // create and add update event for this spell
    SpellEvent* Event = new SpellEvent(this);
    truecaster->m_events.AddEvent(Event, truecaster->m_events.CalculateTime(1));

    SpellCastResult result = PreCastCheck();

    bool failed = result != SPELL_CAST_OK;
    if (result == SPELL_FAILED_BAD_TARGETS && OpenLockCheck())
    {
        failed = false;
        m_IsTriggeredSpell = true;
        m_ignoreCastTime = true;
    }
    if (result == SPELL_FAILED_REAGENTS && itemCheats)
    {
        failed = false;
    }

    if (failed)
    {
        SendCastResult(result);
        finish(false);
        return result;
    }
    else
    {
        Prepare();
        return SPELL_CAST_OK;
    }
}

bool BotUseItemSpell::OpenLockCheck()
{
    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        // for effects of spells that have only one target
        switch (m_spellInfo->Effect[i])
        {
#ifndef MANGOSBOT_TWO
            case SPELL_EFFECT_OPEN_LOCK_ITEM:
#endif
            case SPELL_EFFECT_OPEN_LOCK:
            {
                if (m_caster->GetTypeId() != TYPEID_PLAYER) // only players can open locks, gather etc.
                    return false;

                // we need a go target in case of TARGET_GAMEOBJECT (for other targets acceptable GO and items)
                if (m_spellInfo->EffectImplicitTargetA[i] == TARGET_GAMEOBJECT)
                {
                    if (!m_targets.getGOTarget())
                        return false;
                }

                // get the lock entry
                uint32 lockId;
                if (GameObject* go = m_targets.getGOTarget())
                {
                    // In BattleGround players can use only flags and banners
                    if (((Player*)m_caster)->InBattleGround() &&
                        !((Player*)m_caster)->CanUseBattleGroundObject())
                        return false;

                    lockId = go->GetGOInfo()->GetLockId();
                    if (!lockId)
                        return false;

                    // check if its in use only when cast is finished (called from spell::cast() with strict = false)
                    if (go->IsInUse())
                        return false;

                    if (go->HasFlag(GAMEOBJECT_FLAGS, GO_FLAG_IN_USE))
                        return false;

                    // done in client but we need to recheck anyway
                    if (go->GetGOInfo()->CannotBeUsedUnderImmunity() && m_caster->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE))
                        return false;
                }
                else if (Item* item = m_targets.getItemTarget())
                {
                    // not own (trade?)
                    if (item->GetOwner() != m_caster)
                        return false;

                    lockId = item->GetProto()->LockID;

                    // if already unlocked
                    if (!lockId || item->HasFlag(ITEM_FIELD_FLAGS, ITEM_DYNFLAG_UNLOCKED))
                        return false;
                }
                else
                    return false;

                if (!lockId)                                            // possible case for GO and maybe for items.
                    return false;

                // Get LockInfo
                LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);

                if (!lockInfo)
                    return false;

                // check lock compatibility
                SpellEffectIndex effIdx = SpellEffectIndex(i);
                SpellCastResult res = CanOpenLock(effIdx, lockId, m_effectSkillInfo[effIdx].skillId, m_effectSkillInfo[effIdx].reqSkillValue, m_effectSkillInfo[effIdx].skillValue);
                if (res == SPELL_FAILED_BAD_TARGETS)
                    return true;
            }
        }
    }

    return false;
}

vector<uint32> ParseItems(const string& text)
{
    vector<uint32> itemIds;

    uint8 pos = 0;
    while (true)
    {
        int i = text.find("Hitem:", pos);
        if (i == -1)
        {
            break;
        }

        pos = i + 6;
        int endPos = text.find(':', pos);
        if (endPos == -1)
        {
            break;
        }

        string idC = text.substr(pos, endPos - pos);
        uint32 id = atol(idC.c_str());
        pos = endPos;
        if (id)
        {
            itemIds.push_back(id);
        }
    }

    return itemIds;
}

bool IsFoodOrDrink(ItemPrototype const* proto, uint32 spellCategory)
{
    return proto->Class == ITEM_CLASS_CONSUMABLE && 
            (proto->SubClass == ITEM_SUBCLASS_FOOD || proto->SubClass == ITEM_SUBCLASS_CONSUMABLE) && 
            proto->Spells[0].SpellCategory == spellCategory;
}

bool IsFood(ItemPrototype const* proto)
{
    return IsFoodOrDrink(proto, 11);
}

bool IsDrink(ItemPrototype const* proto)
{
    return IsFoodOrDrink(proto, 59);
}

bool UseItemAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    string name = event.getParam();
    if (name.empty())
    {
        name = getName();
    }

    list<Item*> items = AI_VALUE2(list<Item*>, "inventory items", name);
    list<ObjectGuid> gos = chat->parseGameobjects(name);

    if (gos.empty())
    {
        if (items.size() > 1)
        {
            list<Item*>::iterator i = items.begin();
            Item* item = *i++;
            Item* itemTarget = *i;

            // Check if the items are in the correct order
            const vector<uint32> itemIds = ParseItems(name);
            if(!itemIds.empty() && item->GetEntry() != *itemIds.begin())
            {
                // Swap items to match command order
                Item* tmp = item;
                item = itemTarget;
                itemTarget = tmp;
            }

            ItemPrototype const* proto = item->GetProto();
            if(item->IsPotion() || IsFood(proto) || IsDrink(proto) || itemTarget->GetProto() == proto)
            {
                return UseItemAuto(requester, item);
            }
            else
            {
                return UseItemOnItem(requester, item, itemTarget);
            }
        }
        else if (!items.empty())
        {
            if (requester && ai->HasActivePlayerMaster() && !selfOnly && requester->GetSelectionGuid())
            {
                Unit* target = ai->GetUnit(requester->GetSelectionGuid());

                if (target != ai->GetBot())
                   return UseItemOnTarget(requester, *items.begin(), target);
            }

            return UseItemAuto(requester, *items.begin());
        }
    }
    else
    {
        if (items.empty())
        {
            return UseGameObject(requester, *gos.begin());
        }
        else
        {
            return UseItemOnGameObject(requester, *items.begin(), *gos.begin());
        }
    }

    ai->TellPlayer(requester, "No items or game objects available");
    return false;
}

bool UseItemAction::isPossible()
{
    return getName() == "use" || (AI_VALUE2(uint32, "item count", getName()) > 0 || ai->HasCheat(BotCheatMask::item));
}

bool UseItemAction::UseGameObject(Player* requester, ObjectGuid guid)
{
    GameObject* go = ai->GetGameObject(guid);
    if (!go || !sServerFacade.isSpawned(go)
#ifdef CMANGOS
        || go->IsInUse()
#endif
        || go->GetGoState() != GO_STATE_READY)
        return false;

    std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
    *packet << guid;
    bot->GetSession()->QueuePacket(std::move(packet));
    
   ostringstream out; out << "Using " << chat->formatGameobject(go);
   ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
   return true;
}

bool UseItemAction::UseItemAuto(Player* requester, Item* item)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 spell_index = 0;
    uint8 cast_count = 1;
    uint32 spellId = 0;
    ObjectGuid item_guid = item->GetObjectGuid();
#ifdef MANGOSBOT_ZERO
    uint16 targetFlag = TARGET_FLAG_SELF;
#else
    uint32 targetFlag = TARGET_FLAG_SELF;
#endif
    uint32 glyphIndex = 0;
    uint8 unk_flags = 0;

    ItemPrototype const* proto = item->GetProto();
    bool isDrink = IsDrink(proto);
    bool isFood = IsFood(proto);

    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        // wrong triggering type
        if (proto->Spells[i].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
            continue;

        if (proto->Spells[i].SpellId > 0)
        {
            spell_index = i;
        }
    }

    //Temporary fix for starting quests:
    if (uint32 questid = item->GetProto()->StartQuest)
    {
        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (qInfo)
        {
            WorldPacket packet(CMSG_QUESTGIVER_ACCEPT_QUEST, 8 + 4 + 4);
            packet << item_guid;
            packet << questid;
            packet << uint32(0);
            bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);
            ostringstream out; out << "Got quest " << chat->formatQuest(qInfo);
            ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            return true;
        }
    }

#ifdef MANGOSBOT_ZERO
    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << spell_index;
#endif
#ifdef MANGOSBOT_ONE
    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << spell_index << cast_count << item_guid;
#endif
#ifdef MANGOSBOT_TWO
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetProto()->Spells[i].SpellId > 0)
        {
            spellId = item->GetProto()->Spells[i].SpellId;
            break;
        }
    }

    WorldPacket packet(CMSG_USE_ITEM, 1 + 1 + 1 + 4 + 8 + 4 + 1 + 8 + 1);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << unk_flags;
#endif

    packet << targetFlag;
    packet.appendPackGUID(bot->GetObjectGuid());

    SetDuration(sPlayerbotAIConfig.globalCoolDown);
    if (isFood || isDrink)
    {
        if (sServerFacade.IsInCombat(bot))
            return false;

        bot->addUnitState(UNIT_STAND_STATE_SIT);
        ai->InterruptSpell();
        ai->StopMoving();

        float hp = bot->GetHealthPercent();
        float mp = bot->GetPower(POWER_MANA) * 100.0f / bot->GetMaxPower(POWER_MANA);
        float p;
        if (isDrink && isFood)
        {
            p = min(hp, mp);
            TellConsumableUse(requester, item, "Feasting", p);
        }
        else if (isDrink)
        {
            p = mp;
            TellConsumableUse(requester, item, "Drinking", p);
        }
        else if (isFood)
        {
            p = hp;
            TellConsumableUse(requester, item, "Eating", p);
        }
        
        bool waitForWellFed = false;
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (proto->Spells[i].SpellId > 0)
            {
                if (SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(proto->Spells[i].SpellId))
                {
                    for (uint8 j = 0; j < MAX_SPELL_EFFECTS; ++j)
                    {
                        if (spellInfo->Effect[j] == SPELL_EFFECT_APPLY_AURA && spellInfo->EffectApplyAuraName[j] == SPELL_AURA_PERIODIC_TRIGGER_SPELL)
                        {
                            waitForWellFed = true;
                            break;
                        }
                    }
                }
            }
        }

        if(!bot->IsInCombat())
        {
            const float multiplier = bot->InBattleGround() ? 20000.0f : 27000.0f;
            const float duration = waitForWellFed ? 15000.0f : multiplier * ((100 - p) / 100.0f);
            SetDuration(duration);
        }
    }

    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;

   //return UseItem(item, ObjectGuid(), nullptr);
}

bool UseItemAction::UseItemOnGameObject(Player* requester, Item* item, ObjectGuid go)
{
   return UseItem(requester, item, go, nullptr);
}

bool UseItemAction::UseItemOnItem(Player* requester, Item* item, Item* itemTarget)
{
    return UseItem(requester, item, ObjectGuid(), itemTarget);
}

bool UseItemAction::UseItemOnTarget(Player* requester, Item* item, Unit* target)
{
    return UseItem(requester, item, ObjectGuid(), nullptr, target);
}

bool UseItemAction::UseItem(Player* requester, Item* item, ObjectGuid goGuid, Item* itemTarget, Unit* unitTarget)
{
    if (bot->CanUseItem(item) != EQUIP_ERR_OK)
        return false;

    if (bot->IsNonMeleeSpellCasted(true))
        return false;

    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    uint8 spell_index = 0;
    uint8 cast_count = 1;
    uint32 spellId = 0;

#ifndef MANGOSBOT_TWO
    ObjectGuid item_guid = item->GetObjectGuid();
#else
    ObjectGuid item_guid = item->GetObjectGuid();
#endif

    uint32 glyphIndex = 0;
    uint8 unk_flags = 0;

#ifdef MANGOSBOT_ZERO
    uint16 targetFlag = TARGET_FLAG_SELF;
#else
    uint32 targetFlag = TARGET_FLAG_SELF;
#endif

    if (itemTarget)
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (item->GetProto()->Spells[i].SpellId > 0)
            {
                spellId = item->GetProto()->Spells[i].SpellId;
                spell_index = i;
                break;
            }
        }
    }

#ifdef MANGOSBOT_ZERO
    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << spell_index;
#elif MANGOSBOT_ONE
    WorldPacket packet(CMSG_USE_ITEM);
    packet << bagIndex << slot << spell_index << cast_count << item_guid;
#elif MANGOSBOT_TWO
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (item->GetProto()->Spells[i].SpellId > 0)
        {
            spellId = item->GetProto()->Spells[i].SpellId;
            break;
        }
    }

    WorldPacket packet(CMSG_USE_ITEM, 1 + 1 + 1 + 4 + 8 + 4 + 1 + 8 + 1);
    packet << bagIndex << slot << cast_count << spellId << item_guid << glyphIndex << unk_flags;
#endif

    bool targetSelected = false;

    map<string, string> replyArgs;
    replyArgs["%target"] = chat->formatItem(item);
    ostringstream replyStr; replyStr << BOT_TEXT("use_command");

    if ((int)item->GetProto()->Stackable > 1)
    {
        uint32 count = item->GetCount();
        if (count > 1)
        {
            replyArgs["%amount"] = count;
            replyStr << " " << BOT_TEXT("use_command_remaining");
        }
        else
        {
            replyStr << " " << BOT_TEXT("use_command_last");
        }
    }

    if (goGuid)
    {
        GameObject* go = ai->GetGameObject(goGuid);
        if (!go || !sServerFacade.isSpawned(go))
        {
            return false;
        }

        targetFlag = TARGET_FLAG_GAMEOBJECT;
        packet << targetFlag;
        packet.appendPackGUID(goGuid.GetRawValue());

        replyArgs["%gameobject"] = chat->formatGameobject(go);
        replyStr << " " << BOT_TEXT("use_command_target_go");

        targetSelected = true;
    }

    if (itemTarget)
    {
#ifndef MANGOSBOT_ZERO
        if (item->GetProto()->Class == ITEM_CLASS_GEM)
        {
            bool fit = SocketItem(requester, itemTarget, item) || SocketItem(requester, itemTarget, item, true);
            if (!fit)
            {
                ai->TellPlayer(requester, BOT_TEXT("use_command_socket_error"), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            }

            return fit;
        }
        else
        {
#endif
            targetFlag = TARGET_FLAG_ITEM;
            packet << targetFlag;
            packet.appendPackGUID(itemTarget->GetObjectGuid());

            replyArgs["%item"] = chat->formatItem(itemTarget);
            replyStr << " " << BOT_TEXT("command_target_item");

            targetSelected = true;
#ifndef MANGOSBOT_ZERO
        }
#endif
    }

    if (!targetSelected && !selfOnly)
    {
        // Prevent using a item target spell on a unit target
        bool needsItemTarget = false;
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
        {
            spellId = item->GetProto()->Spells[i].SpellId;
            if (spellId > 0)
            {
                const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
                if (pSpellInfo->Targets & TARGET_FLAG_ITEM)
                {
                    needsItemTarget = true;
                    unitTarget = nullptr;
                }

                break;
            }
        }
        
        if(!needsItemTarget)
        {
            if (unitTarget)
            {
                if (item->IsTargetValidForItemUse(unitTarget))
                {
                    targetFlag = TARGET_FLAG_UNIT;
                    packet << targetFlag << unitTarget->GetObjectGuid().WriteAsPacked();

                    replyArgs["%unit"] = unitTarget->GetName();
                    replyStr << " " << BOT_TEXT("command_target_unit");

                    targetSelected = true;
                }
            }
            else
            {
                if (!targetSelected && requester && ai->HasActivePlayerMaster())
                {
                    ObjectGuid requesterSelection = requester->GetSelectionGuid();
                    if (requesterSelection)
                    {
                        Unit* unit = ai->GetUnit(requesterSelection);
                        if (unit && item->IsTargetValidForItemUse(unit))
                        {
                            targetFlag = TARGET_FLAG_UNIT;
                            packet << targetFlag << requesterSelection.WriteAsPacked();

                            replyArgs["%unit"] = unit->GetName();
                            replyStr << " " << BOT_TEXT("command_target_unit");

                            targetSelected = true;
                        }
                    }
                }
            }
        }
    }

    if (unitTarget)
    {
        uint32 spellid = 0;
        for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (item->GetProto()->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
            {
                spellid = item->GetProto()->Spells[i].SpellId;
                spell_index = i;
                break;
            }
        }

        if (SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid))
        {
            if (!bot->IsSpellReady(spellid, item->GetProto()))
                return false;

            if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
            {
                targetFlag = TARGET_FLAG_DEST_LOCATION;
                Position pos = unitTarget->GetPosition();
                SpellCastTargets targets;
                targets.setDestination(pos.x, pos.y, pos.z);
                targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;

                const auto spell = new Spell(bot, spellInfo, false, bot->GetObjectGuid());
                spell->m_targets = targets;
                if (spell->CheckRange(true) != SPELL_CAST_OK)
                {
                    delete spell;
                    return false;
                }

                delete spell;

#ifdef MANGOSBOT_ZERO
                bot->CastItemUseSpell(item, targets, spell_index);
#elif MANGOSBOT_ONE
                bot->CastItemUseSpell(item, targets, 1, spell_index);
#elif MANGOSBOT_TWO
                bot->CastItemUseSpell(item, targets, 1, 0, spellid);
#endif

                return true;
            }
        }
    }

    if (uint32 questid = item->GetProto()->StartQuest)
    {
        Quest const* qInfo = sObjectMgr.GetQuestTemplate(questid);
        if (qInfo)
        {
            WorldPacket packet(CMSG_QUESTGIVER_ACCEPT_QUEST, 8 + 4 + 4);
            packet << item_guid;
            packet << questid;
            packet << uint32(0);
            bot->GetSession()->HandleQuestgiverAcceptQuestOpcode(packet);

            map<string, string> args;
            args["%quest"] = chat->formatQuest(qInfo);
            ai->TellPlayerNoFacing(requester, BOT_TEXT2("use_command_quest_accepted", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
            return true;
        }
    }

    //bot->clearUnitState(UNIT_STAT_CHASE);
    //bot->clearUnitState(UNIT_STAT_FOLLOW);

    if (sServerFacade.isMoving(bot))
    {
        ai->StopMoving();
    }

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
    {
        spellId = item->GetProto()->Spells[i].SpellId;
        if (!spellId)
            continue;

        bool canCast = false;

        if (unitTarget && ai->CanCastSpell(spellId, unitTarget, 0, false))
            canCast = true;

        if (!canCast && goGuid)
        {
            GameObject* go = ai->GetGameObject(goGuid);

            if (go)
            {
                if (ai->CanCastSpell(spellId, go, 0, false))
                    canCast = true;
            }
        }
        if (!canCast && ai->CanCastSpell(spellId, bot, 0, false))
            canCast = true;

        if (!canCast)
            return false;

        const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (pSpellInfo->Targets & TARGET_FLAG_ITEM)
        {
            Item* itemForSpell = AI_VALUE2(Item*, "item for spell", spellId);
            if (!itemForSpell)
                continue;

            if (itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                continue;

            if (bot->GetTrader())
            {
                if (selfOnly)
                    return false;

                targetFlag = TARGET_FLAG_TRADE_ITEM;
                packet << targetFlag << (uint8)1 << (uint64)TRADE_SLOT_NONTRADED;
                targetSelected = true;
                replyStr << " " << BOT_TEXT("command_target_trade");
            }
            else
            {
                targetFlag = TARGET_FLAG_ITEM;
                packet << targetFlag;
                packet.appendPackGUID(itemForSpell->GetObjectGuid());
                targetSelected = true;

                replyArgs["%item"] = chat->formatItem(itemForSpell);
                replyStr << " " << BOT_TEXT("command_target_item");
            }

            Spell *spell = new Spell(bot, pSpellInfo, false);
            ai->WaitForSpellCast(spell);
            delete spell;
        }
        break;
    }

    if (!targetSelected)
    {
        targetFlag = TARGET_FLAG_SELF;
        packet << targetFlag;
        packet.appendPackGUID(bot->GetObjectGuid());
        targetSelected = true;
        replyStr << " " << BOT_TEXT("command_target_self");
    }

    if (!spellId)
       return false;

    SetDuration(sPlayerbotAIConfig.globalCoolDown);
    ai->TellPlayerNoFacing(requester, BOT_TEXT2(replyStr.str(), replyArgs), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

    bot->GetSession()->HandleUseItemOpcode(packet);
    return true;
}

void UseItemAction::TellConsumableUse(Player* requester, Item* item, string action, float percent)
{
    ostringstream out;
    out << action << " " << chat->formatItem(item);
    if ((int)item->GetProto()->Stackable > 1) out << "/x" << item->GetCount();
    out << " (" << round(percent) << "%)";
    ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
}

#ifndef MANGOSBOT_ZERO
bool UseItemAction::SocketItem(Player* requester, Item* item, Item* gem, bool replace)
{
   WorldPacket* const packet = new WorldPacket(CMSG_SOCKET_GEMS);
   *packet << item->GetObjectGuid();

   bool fits = false;
   for (uint32 enchant_slot = SOCK_ENCHANTMENT_SLOT; enchant_slot < SOCK_ENCHANTMENT_SLOT + MAX_GEM_SOCKETS; ++enchant_slot)
   {
      uint8 SocketColor = item->GetProto()->Socket[enchant_slot - SOCK_ENCHANTMENT_SLOT].Color;
      GemPropertiesEntry const* gemProperty = sGemPropertiesStore.LookupEntry(gem->GetProto()->GemProperties);
      if (gemProperty && (gemProperty->color & SocketColor))
      {
         if (fits)
         {
            *packet << ObjectGuid();
            continue;
         }

         uint32 enchant_id = item->GetEnchantmentId(EnchantmentSlot(enchant_slot));
         if (!enchant_id)
         {
            *packet << gem->GetObjectGuid();
            fits = true;
            continue;
         }

         SpellItemEnchantmentEntry const* enchantEntry = sSpellItemEnchantmentStore.LookupEntry(enchant_id);
         if (!enchantEntry || !enchantEntry->GemID)
         {
            *packet << gem->GetObjectGuid();
            fits = true;
            continue;
         }

         if (replace && enchantEntry->GemID != gem->GetProto()->ItemId)
         {
            *packet << gem->GetObjectGuid();
            fits = true;
            continue;
         }

      }

      *packet << ObjectGuid();
   }

   if (fits)
   {
      ostringstream out; out << "Socketing " << chat->formatItem(item);
      out << " with " << chat->formatItem(gem);
      ai->TellPlayer(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);

      bot->GetSession()->HandleSocketOpcode(*packet);
   }
   return fits;
}

#endif

bool UseItemIdAction::Execute(Event& event)
{
    uint32 itemId;
    Unit* target = nullptr;
    GameObject* go = nullptr;

    if (getQualifier().empty())
    {
        itemId = GetItemId();
        target = GetTarget();
        string params = event.getParam();
        list<ObjectGuid> gos = chat->parseGameobjects(params);
        if (!gos.empty())
            GameObject* go = ai->GetGameObject(*gos.begin());
    }
    else
    {
        vector<string> params = getMultiQualifiers(getQualifier(), ",");
        itemId = stoi(params[0]);
        if (params.size() > 1)
        {
            list<GuidPosition> guidPs = AI_VALUE(list<GuidPosition>, params[1]);
            if (!guidPs.empty())
            {
                GuidPosition guidP = *guidPs.begin();
                if (guidP.IsGameObject())
                    go = guidP.GetGameObject();
                else
                    target = guidP.GetUnit();

            }
        }
    }

    return CastItemSpell(itemId, target, go);
}

bool UseItemIdAction::isPossible()
{
    uint32 itemId = GetItemId();

    if (!itemId)
        return false;

    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

    if (!proto)
        return false;

    if (HasSpellCooldown(itemId))
        return false;

    if (!ai->HasCheat(BotCheatMask::item) && !bot->HasItemCount(itemId, 1))
        return false;

    uint32 spellCount = 0;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
#ifdef MANGOSBOT_ZERO
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
#else
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
#endif
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
        if (!spellInfo)
        {
            continue;
        }

        spellCount++;        
    }


    return spellCount;
}

bool UseItemIdAction::HasSpellCooldown(const uint32 itemId)
{
    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

    if (!proto)
        return false;

    uint32 spellId = 0;
    for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        if (proto->Spells[i].SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
        {
            continue;
        }

        if (proto->Spells[i].SpellId > 0)
        {
            if (!sServerFacade.IsSpellReady(bot, proto->Spells[i].SpellId))
                return true;

            if (!sServerFacade.IsSpellReady(bot, proto->Spells[i].SpellId, itemId))
                return true;
        }
    }

    return false;
}

bool UseItemIdAction::CastItemSpell(uint32 itemId, Unit* target, GameObject* goTarget)
{
    ItemPrototype const* proto = sObjectMgr.GetItemPrototype(itemId);

    if (!proto)
        return false;

    Item* item = nullptr;

    if (!ai->HasCheat(BotCheatMask::item)) //If bot has no item cheat it needs an item to cast.
    {
        list<Item*> items = AI_VALUE2(list<Item*>, "inventory items", chat->formatQItem(itemId));

        if (items.empty())
            return false;

        item = items.front();
    }

    SpellCastTargets targets;
    if (target)
    {
        targets.setUnitTarget(target);
        targets.setDestination(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
    }
    if (goTarget)
    {
        targets.setGOTarget(goTarget);
        targets.m_targetMask = TARGET_FLAG_GAMEOBJECT;
        targets.setDestination(goTarget->GetPositionX(), goTarget->GetPositionY(), goTarget->GetPositionZ());
    }
    else
        targets.m_targetMask = TARGET_FLAG_SELF;
        
    // use triggered flag only for items with many spell casts and for not first cast
    int count = 0;

    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
    {
        _Spell const& spellData = proto->Spells[i];

        // no spell
        if (!spellData.SpellId)
            continue;

        // wrong triggering type
#ifdef MANGOSBOT_ZERO
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
#else
        if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
#endif
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
        if (!spellInfo)
        {
            continue;
        }

        if (spellInfo->Targets & TARGET_FLAG_DEST_LOCATION)
            targets.m_targetMask = TARGET_FLAG_DEST_LOCATION;

        BotUseItemSpell* spell = new BotUseItemSpell(bot, spellInfo, (count > 0) ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);

        Item* tItem = nullptr;       

        if (item)
        {
            spell->SetCastItem(item);
            item->SetUsedInSpell(true);
        }

        spell->m_clientCast = true;

        bool result = (spell->ForceSpellStart(&targets) == SPELL_CAST_OK);

        if (!result)
            return false;

        bot->RemoveSpellCooldown(*spellInfo, false);
        bot->AddCooldown(*spellInfo, proto, false);

        ++count;
    }

    return count;
}

bool UseItemIdAction::isUseful()
{
    if (!getQualifier().empty() && (getMultiQualifierStr(getQualifier(), 0, ",").find_first_not_of("0123456789") != std::string::npos))
    {
        sLog.outError("UseItemIdAction::isUseful with qualifier %s, should be number.", getQualifier().c_str());
        return false;
    }

    const ItemPrototype* proto = sObjectMgr.GetItemPrototype(GetItemId());
    if (proto)
    {
        set<uint32>& skipSpells = AI_VALUE(set<uint32>&, "skip spells list");
        if(!skipSpells.empty())
        {
            for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
            {
                const _Spell& spellData = proto->Spells[i];
                if (spellData.SpellId)
                {
                    if (skipSpells.find(spellData.SpellId) != skipSpells.end())
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    return false;
}

bool UseSpellItemAction::isUseful()
{
   return AI_VALUE2(bool, "spell cast useful", getName());
}

bool UseHearthStoneAction::Execute(Event& event)
{
    if (bot->IsMoving())
    {
        ai->StopMoving();
    }

    if (AI_VALUE2(uint32, "current mount speed", "self target"))
    {
        if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
            return false;

        if (bot->IsMounted())
        {
            WorldPacket emptyPacket;
            bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
            bot->UpdateSpeed(MOVE_RUN, true);
            bot->UpdateSpeed(MOVE_RUN, false);

            if (bot->IsFlying())
                bot->GetMotionMaster()->MoveFall();
        }
    }

    ai->RemoveShapeshift();

    const bool used = UseItemAction::Execute(event);
    if (used)
    {
        RESET_AI_VALUE(bool, "combat::self target");
        RESET_AI_VALUE(WorldPosition, "current position");
    }

    return used;
}

bool UseHearthStoneAction::isUseful() 
{
    if (!sServerFacade.IsSpellReady(bot, 8690))
        return false;

    if (bot->InBattleGround())
        return false;

    //Do not HS in dungeons when master is inside the dungeon or dead.
    if (ai->GetMaster() && !WorldPosition(bot).isOverworld() && (bot->GetMapId() == ai->GetMaster()->GetMapId() || !ai->GetMaster()->IsAlive()))
        return false;

    if (bot->IsFlying() && WorldPosition(bot).currentHeight() > 10.0f)
        return false;

    return true;
}

bool UseRandomRecipeAction::isUseful()
{
   return !bot->IsInCombat() && !ai->HasActivePlayerMaster() && !bot->InBattleGround();
}

bool UseRandomRecipeAction::Execute(Event& event)
{
    list<Item*> recipes = AI_VALUE2(list<Item*>, "inventory items", "recipe");   
    string recipeName = "";
    for (auto& recipe : recipes)
    {
        recipeName = recipe->GetProto()->Name1;
        if (!urand(0, 10))
            break;
    }

    if (recipeName.empty())
        return false;

    Event rEvent = Event(name, recipeName);

    return UseItemAction::Execute(rEvent);
}

bool UseRandomQuestItemAction::isUseful()
{
    return !ai->HasActivePlayerMaster() && !bot->InBattleGround() && !bot->IsTaxiFlying();
}

bool UseRandomQuestItemAction::Execute(Event& event)
{
    Unit* unitTarget = nullptr;
    ObjectGuid goTarget = ObjectGuid();

    list<Item*> questItems = AI_VALUE2(list<Item*>, "inventory items", "quest");
    if (questItems.empty())
        return false;

    Item* item = nullptr;
    for (uint8 i = 0; i< 5;i++)
    {
        auto itr = questItems.begin();
        std::advance(itr, urand(0, questItems.size()- 1));
        Item* questItem = *itr;

        ItemPrototype const* proto = questItem->GetProto();

        if (proto->StartQuest)
        {
            Quest const* qInfo = sObjectMgr.GetQuestTemplate(proto->StartQuest);
            if (bot->CanTakeQuest(qInfo, false))
            {
                item = questItem;
                break;
            }
        }
        /*
        uint32 spellId = proto->Spells[0].SpellId;
        if (spellId)
        {
            SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(spellId);

            list<ObjectGuid> npcs = AI_VALUE(list<ObjectGuid>, ("nearest npcs"));
            for (auto& npc : npcs)
            {
                Unit* unit = ai->GetUnit(npc);
                if (ai->CanCastSpell(spellId, unit, 0, false))
                {
                    item = questItem;
                    unitTarget = unit;
                    break;
                }
            }

            list<ObjectGuid> gos = AI_VALUE(list<ObjectGuid>, ("nearest game objects"));
            for (auto& go : gos)
            {
                GameObject* gameObject = ai->GetGameObject(go);
                GameObjectInfo const* goInfo = gameObject->GetGOInfo();
                if (!goInfo->GetLockId())
                    continue;

                LockEntry const* lock = sLockStore.LookupEntry(goInfo->GetLockId());

                for (uint8 i = 0; i < MAX_LOCK_CASE; ++i)
                {
                    if (!lock->Type[i])
                        continue;
                    if (lock->Type[i] != LOCK_KEY_ITEM)
                        continue;

                    if (lock->Index[i] == proto->ItemId)
                    {
                        item = questItem;
                        goTarget = go;
                        unitTarget = nullptr;
                        break;
                    }
                }               
            }
        }
        */
    }

    if (!item)
        return false;

    bool used = UseItem(nullptr, item, goTarget, nullptr, unitTarget);
    if (used)
    {
        SetDuration(sPlayerbotAIConfig.globalCoolDown);
    }

    return used;
}