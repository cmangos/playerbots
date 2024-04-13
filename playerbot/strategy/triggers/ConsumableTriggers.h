#pragma once
#include "../triggers/DungeonTriggers.h"
#include "../actions/UseItemAction.h"

namespace ai
{
//    class MissingBuffTrigger : public Trigger
//    {
//    public:
//       MissingBuffTrigger(PlayerbotAI* ai, uint32 itemId, uint32 spellId, std::string name = "missing buff")
//          : Trigger(ai, name, 5)
//          , m_itemId(itemId)
//          , m_spellId(spellId)
//       {}
// 
//       bool IsActive() override
//       {
//          if (!bot->HasAura(m_spellId))
//          {
//             return bot->GetItemByEntry(m_itemId) != nullptr;
//          }
// 
//          return false;
//       }
// 
//    private:
//       uint32 m_itemId;
//       uint32 m_spellId;
//    };

   
#define ITEM_ELIXIR_OF_SUPERIOR_DEFENSE 13445
#define BUFF_ELIXIR_OF_SUPERIOR_DEFENSE 11348

#define ITEM_ELIXIR_OF_THE_MONGOOSE 13452
#define BUFF_ELIXIR_OF_THE_MONGOOSE 17538

#define ITEM_GREATER_ARCANE_ELIXIR 13454
#define BUFF_GREATER_ARCANE_ELIXIR 17539

#define ITEM_GREATER_STONESHIELD_POTION 13455
#define BUFF_GREATER_STONESHIELD_POTION 17540


#define ITEM_FLASK_OF_THE_TITANS 13510
#define BUFF_FLASK_OF_THE_TITANS 17626

#define ITEM_FLASK_OF_DISTILLED_WISDOM 13511
#define BUFF_FLASK_OF_DISTILLED_WISDOM 17627

#define ITEM_FLASK_OF_SUPREME_POWER 13512
#define BUFF_FLASK_OF_SUPREME_POWER 17628

#define ITEM_FLASK_OF_CHROMATIC_RESISTANCE 13513
#define BUFF_FLASK_OF_CHROMATIC_RESISTANCE 17629


   inline std::pair<uint32, uint32> GetBestFlask(PlayerbotAI* ai)
   {
      switch (ai->GetBot()->getClass())
      {
         case CLASS_WARRIOR: return { ITEM_FLASK_OF_THE_TITANS, BUFF_FLASK_OF_THE_TITANS };
         case CLASS_PALADIN: 
            if (ai->GetTalentSpec() == PlayerTalentSpec::TALENT_SPEC_PALADIN_HOLY)
               return { ITEM_FLASK_OF_DISTILLED_WISDOM, BUFF_FLASK_OF_DISTILLED_WISDOM };
            else
               return { ITEM_FLASK_OF_THE_TITANS, BUFF_FLASK_OF_THE_TITANS };
         case CLASS_HUNTER: return { ITEM_FLASK_OF_DISTILLED_WISDOM, BUFF_FLASK_OF_DISTILLED_WISDOM };
         case CLASS_ROGUE: return { ITEM_FLASK_OF_THE_TITANS, BUFF_FLASK_OF_THE_TITANS };
         case CLASS_PRIEST: 
            if (ai->GetTalentSpec() == PlayerTalentSpec::TALENT_SPEC_PRIEST_SHADOW)
               return { ITEM_FLASK_OF_SUPREME_POWER, BUFF_FLASK_OF_SUPREME_POWER };
            else
               return { ITEM_FLASK_OF_DISTILLED_WISDOM, BUFF_FLASK_OF_DISTILLED_WISDOM };
         case CLASS_SHAMAN:
            if (ai->GetTalentSpec() == PlayerTalentSpec::TALENT_SPEC_SHAMAN_ELEMENTAL)
               return { ITEM_FLASK_OF_SUPREME_POWER, BUFF_FLASK_OF_SUPREME_POWER };
            else if (ai->GetTalentSpec() == PlayerTalentSpec::TALENT_SPEC_SHAMAN_RESTORATION)
               return { ITEM_FLASK_OF_DISTILLED_WISDOM, BUFF_FLASK_OF_DISTILLED_WISDOM };
            else
               return { ITEM_FLASK_OF_THE_TITANS, BUFF_FLASK_OF_THE_TITANS };
         break;

         case CLASS_MAGE: return { ITEM_FLASK_OF_SUPREME_POWER, BUFF_FLASK_OF_SUPREME_POWER };
         case CLASS_WARLOCK: return { ITEM_FLASK_OF_SUPREME_POWER, BUFF_FLASK_OF_SUPREME_POWER };
         case CLASS_DRUID: 
            if (ai->GetTalentSpec() == PlayerTalentSpec::TALENT_SPEC_DRUID_BALANCE)
               return { ITEM_FLASK_OF_SUPREME_POWER, BUFF_FLASK_OF_SUPREME_POWER };
            else if (ai->GetTalentSpec() == PlayerTalentSpec::TALENT_SPEC_DRUID_RESTORATION)
               return { ITEM_FLASK_OF_DISTILLED_WISDOM, BUFF_FLASK_OF_DISTILLED_WISDOM };
            else
               return { ITEM_FLASK_OF_THE_TITANS, BUFF_FLASK_OF_THE_TITANS };
      }

      // hp always good
      return { ITEM_FLASK_OF_THE_TITANS, BUFF_FLASK_OF_THE_TITANS };
   }


//    struct Consumable {
//       uint32 itemId;
//       uint32 spellId;
//       std::vector<
//    };
   inline void CreateMissingFlaskToConsumableUseTrigger(map<string, std::function<Trigger* (PlayerbotAI*)>>& creators)
   {
      creators["use flask"] = [](PlayerbotAI* ai) -> Trigger*
         {
            std::pair<uint32, uint32> flask = GetBestFlask(ai);

            std::ostringstream ss;
            ss << "missing buff " << flask.second;

            return new ItemBuffReadyTrigger(ai, ss.str().c_str(), flask.first, flask.second);
         };
   }

   inline void CreateBuffToConsumableUseTrigger(map<string, std::function<Trigger*(PlayerbotAI*)>>& creators, uint32 itemId, uint32 buffId)
   {
      std::ostringstream ss;
      ss << "use potion " << itemId;
      creators[ss.str().c_str()] = [itemId, buffId](PlayerbotAI* ai) -> Trigger*
         {
            std::ostringstream ss;
            ss << "missing buff " << buffId;

            return new ItemBuffReadyTrigger(ai, ss.str().c_str(), itemId, buffId);
         };
   }

   inline void CreateUseFlaskAction(map<string, std::function<Action* (PlayerbotAI*)>>& creators)
   {
      creators["use flask"] = [](PlayerbotAI* ai) -> Action*
         {
            std::pair<uint32, uint32> flask = GetBestFlask(ai);
            std::ostringstream itemlink;
            itemlink << "|cffffffff|Hitem:" << flask.first << ":0:0:0|h[Whatever]|h|r";

            return new UseItemAction(ai, itemlink.str().c_str(), true);
         };
   }

   inline void CreateUseConsumableAction(map<string, std::function<Action* (PlayerbotAI*)>>& creators, uint32 itemId)
   {
      std::ostringstream ss;
      ss << "use potion " << itemId;
      creators[ss.str().c_str()] = [itemId](PlayerbotAI* ai) -> Action*
         {
            std::ostringstream itemlink;
            itemlink << "|cffffffff|Hitem:" << itemId << ":0:0:0|h[Whatever]|h|r";

            return new UseItemAction(ai, itemlink.str().c_str(), true);
         };
   }

   inline void InitMissingConsumableTrigger(std::list<TriggerNode*>& triggers, uint32 itemId)
   {
      std::ostringstream ss;
      ss << "use potion " << itemId;
      triggers.push_back(new TriggerNode(
         ss.str().c_str(),
         NextAction::array(0, new NextAction(ss.str().c_str(), ACTION_HIGH), NULL)));
   }

}