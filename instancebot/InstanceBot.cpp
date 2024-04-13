#include "InstanceBot.h"
#include "Entities/Player.h"
#include "playerbot/ServerFacade.h"

#include "playerbot/PlayerbotAIConfig.h"

#include <sstream>
#include <boost/algorithm/string.hpp>

#define ALL_CLASSES 0

namespace instancebot {

   struct PlayerData
   {
      std::vector<std::string> bip;
   };

   InstanceBot::InstanceBot(Map* map) :
      mp_map(map)
   {
      m_dungeon_strat = "brd_arena_run";

      // buffs to all
      // # 15123 - Resist Fire
      m_buffstoclass[15123] = { ALL_CLASSES };
      // # 15366 - Songflower Serenade
      m_buffstoclass[15366] = { ALL_CLASSES };
      // # 16609 - Warchief Blessing
      m_buffstoclass[16609] = { ALL_CLASSES };
      // # 22888 - Rallying Cry of the Dragonslayer
      m_buffstoclass[22888] = { ALL_CLASSES };
      // # 24382 - Spirit of Zanza
      m_buffstoclass[24382] = { ALL_CLASSES };
      // # 24425 - Spirit of Zandalar
      m_buffstoclass[24425] = { ALL_CLASSES };

      // #  9206 - Elixir of Giants
      m_itemstoclass[9206] = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DRUID };
      // # 13442 - Mighty Rage Potion
      m_itemstoclass[13442] = { CLASS_WARRIOR };
      // # 13445 - Elixir of Superior Defense
      m_itemstoclass[13445] = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DRUID };
      // # 13446 - Major Healing Potion;
      m_itemstoclass[13446] = { ALL_CLASSES };
      // # 13452 - Elixir of the Mongoose
      m_itemstoclass[13452] = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_HUNTER, CLASS_ROGUE, CLASS_DRUID, CLASS_SHAMAN};
      // # 13454 - Greater Arcane Elixir
      m_itemstoclass[13454] = { CLASS_MAGE, CLASS_WARLOCK, CLASS_PRIEST, CLASS_SHAMAN, CLASS_DRUID};
      // # 13455 - Greater Stoneshield Potion
      m_itemstoclass[13455] = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DRUID};
      // # 13457 - Greater Fire Protection Potion
      m_itemstoclass[13457] = { ALL_CLASSES};
      // # 13511 - Flask of Distilled Wisdom
      m_itemstoclass[13511] = { CLASS_PRIEST, CLASS_SHAMAN, CLASS_DRUID, CLASS_HUNTER };
      // # 13512 - Flask of Supreme Power
      m_itemstoclass[13512] = { CLASS_MAGE, CLASS_WARLOCK, CLASS_SHAMAN, CLASS_DRUID };
      // # 13513 - Flask of Chromatic Resistance
      m_itemstoclass[13513] = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_DRUID };

      // # 17333 - Aqual Quintessence
      m_itemstoclass[17333] = { ALL_CLASSES };

      // 
      // # 20076 - Zandalar Signet of Mojo
      m_itemstoclass[20076] = { CLASS_MAGE, CLASS_WARLOCK, CLASS_PRIEST, CLASS_SHAMAN, CLASS_DRUID };
      // # 20077 - Zandalar Signet of Might
      m_itemstoclass[20077] = { CLASS_WARRIOR, CLASS_PALADIN, CLASS_HUNTER, CLASS_ROGUE, CLASS_DRUID, CLASS_SHAMAN };
      // # 20078 - Zandalar Signet of Serenity
      m_itemstoclass[20078] = { CLASS_PALADIN, CLASS_PRIEST, CLASS_SHAMAN, CLASS_DRUID };
      // # 20748 - Brilliant Mana Oil
      m_itemstoclass[20748] = { CLASS_PALADIN, CLASS_PRIEST, CLASS_SHAMAN, CLASS_DRUID };
      // # 20749 - Brilliant Wizard Oil
      m_itemstoclass[20749] = { CLASS_MAGE, CLASS_WARLOCK, CLASS_PRIEST, CLASS_SHAMAN, CLASS_DRUID };

      // # 21546 - Elixir of Greater Firepower
      m_itemstoclass[21546] = { CLASS_MAGE, CLASS_WARLOCK };
   }

   void InstanceBot::Init()
   {
      
   }

   bool InstanceBot::AddAura(Unit* unit, uint32 spellId)
   {
      // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form    

      SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
      if (!spellInfo)
         return false;

      if (!IsSpellAppliesAura(spellInfo, (1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2)) &&
         !IsSpellHaveEffect(spellInfo, SPELL_EFFECT_PERSISTENT_AREA_AURA))
      {
         return false;
      }

      SpellAuraHolder* holder = CreateSpellAuraHolder(spellInfo, unit, unit);

      for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
      {
         uint8 eff = spellInfo->Effect[i];
         if (eff >= MAX_SPELL_EFFECTS)
            continue;
         if (IsAreaAuraEffect(eff) ||
            eff == SPELL_EFFECT_APPLY_AURA ||
            eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
         {
            int32 basePoints = spellInfo->CalculateSimpleValue(SpellEffectIndex(i));
            int32 damage = basePoints;
            Aura* aur = CreateAura(spellInfo, SpellEffectIndex(i), &damage, &basePoints, holder, unit);
            holder->AddAura(aur, SpellEffectIndex(i));
         }
      }
      if (!unit->AddSpellAuraHolder(holder))
         delete holder;

      return true;
   }

   std::vector<uint32> InstanceBot::NeedGuildBuffs(Player* player)
   {
      std::vector<uint32> retVec;

      LfgRoles botRoles = sLFGMgr.CalculateTalentRoles(player);

      if (!sPlayerbotAIConfig.guildBuffs.empty())
      {
         for (auto& gb : sPlayerbotAIConfig.guildBuffs)
         {
            if (player->GetGuildId() != gb.guildId)
               continue;

            uint8 classId = player->getClass();

            auto it1 = m_buffstoclass.find(gb.spellId);

            if (it1 != m_buffstoclass.end())
            {
               if (std::find(it1->second.begin(), it1->second.end(), ALL_CLASSES) != it1->second.end() ||
                  std::find(it1->second.begin(), it1->second.end(), classId) != it1->second.end())
               {
                  if (player->HasAura(gb.spellId))
                     continue;

                  retVec.push_back(gb.spellId);
               }
            }

            auto it2 = m_itemstoclass.find(gb.spellId);

            if (it2 != m_itemstoclass.end())
            {
               if (std::find(it2->second.begin(), it2->second.end(), ALL_CLASSES) != it2->second.end() ||
                  std::find(it2->second.begin(), it2->second.end(), classId) != it2->second.end())
               {
                  if (player->GetItemByEntry(gb.spellId) == nullptr)
                  {
                     player->StoreNewItemInBestSlots(gb.spellId, 1);
                  }
               }
            }
         }
      }

      return retVec;
   }

   void InstanceBot::OnPlayerEnter(Player* player)
   {
      m_map.insert(std::make_pair(player->GetObjectGuid(), new PlayerData));

      std::ostringstream welcome_message;
      welcome_message << "Welcome to InstanceBot for " <<  mp_map->GetMapName();
      welcome_message << " strat: " << m_dungeon_strat;
      welcome_message << " progress: " << m_progress;

      WorldPacket data;
      ChatHandler::BuildChatPacket(data, CHAT_MSG_BG_SYSTEM_NEUTRAL, welcome_message.str().c_str(), LANG_UNIVERSAL);
      sServerFacade.SendPacket(player, data);


      std::vector<uint32> pbuff = NeedGuildBuffs(player);
      for (uint32 aura : pbuff)
         AddAura(player, aura);

//       if (PlayerbotAI* bot = player->GetPlayerbotAI())
//       {
//          if (Player* master = bot->GetMaster())
//          {
//             std::vector<std::string> lines;
// 
//             if (bot->IsTank(player))
//                lines.push_back("co +rtsc");
//             else
//                lines.push_back("co +threat,+rtsc");
// 
//             lines.push_back("ll -vendor,-consumable");
// 
//             for (auto& l : lines)
//             {
//                ChatHandler::BuildChatPacket(data, CHAT_MSG_WHISPER, l.c_str(), LANG_UNIVERSAL, CHAT_TAG_NONE, master->GetObjectGuid(), master->GetName());
//                //sServerFacade.SendPacket(player, data);
//                bot->HandleMasterIncomingPacket(data);
//             }
//             
//          }
//          
//       }


   }

   void InstanceBot::OnPlayerLeave(Player* player)
   {
      auto it = m_map.find(player->GetObjectGuid());

      if (it == m_map.end())
         return;

      delete it->second;
      m_map.erase(it);
   }

   void InstanceBot::Update(const uint32& t_diff)
   {
      if (m_cooldown < t_diff)
      {
         m_cooldown = 10000;

         CheckBuffs();
      }
      else
      {
         m_cooldown -= t_diff;
      }
   }

   void InstanceBot::CheckBuffs()
   {
      for (auto& el : m_map)
      {
         Player* player = mp_map->GetPlayer(el.first);

         std::vector<uint32> pbuff = NeedGuildBuffs(player);
         for (uint32 aura : pbuff)
            AddAura(player, aura);
      }
   }

   std::list<std::string> InstanceBot::HandlePlayerbotCommand(char const* args, Player* master)
   {
      std::list<std::string> l;


      return l;
   }
}