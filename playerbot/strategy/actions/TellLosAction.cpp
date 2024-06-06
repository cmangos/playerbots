
#include "playerbot/playerbot.h"
#include "TellLosAction.h"

using namespace ai;

bool TellLosAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();

    if (param.empty() || param == "targets")
    {
        ListUnits(requester, "--- Targets ---", *context->GetValue<std::list<ObjectGuid> >("possible targets"));
        ListUnits(requester, "--- Targets (All) ---", *context->GetValue<std::list<ObjectGuid> >("all targets"));
    }

    if (param.empty() || param == "npcs")
    {
        ListUnits(requester, "--- NPCs ---", *context->GetValue<std::list<ObjectGuid> >("nearest npcs"));
    }

    if (param.empty() || param == "corpses")
    {
        ListUnits(requester, "--- Corpses ---", *context->GetValue<std::list<ObjectGuid> >("nearest corpses"));
    }

    if (param.empty() || ChatHelper::startswith(param, "gos") || ChatHelper::startswith(param, "game objects"))
    {
        std::vector<LosModifierStruct> mods;

        if (ChatHelper::startswith(param, "gos"))
        {
            mods = ParseLosModifiers(param.substr(3));
        }
        else if (ChatHelper::startswith(param, "game objects"))
        {
           mods = ParseLosModifiers(param.substr(12));
        }

        TellGameObjects(requester, "--- Game objects ---", FilterGameObjects(requester, GoGuidListToObjList(ai, *context->GetValue<std::list<ObjectGuid> >("nearest game objects no los")), mods), mods);
    }

    if (param.empty() || param == "players")
    {
        ListUnits(requester, "--- Friendly players ---", *context->GetValue<std::list<ObjectGuid> >("nearest friendly players"));
    }

    return true;
}

void TellLosAction::ListUnits(Player* requester, std::string title, std::list<ObjectGuid> units)
{
    ai->TellPlayer(requester, title);

    for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
    {
        Unit* unit = ai->GetUnit(*i);
        if (unit)
            ai->TellPlayer(requester, unit->GetName());
    }

}

std::list<GameObject*> TellLosAction::GoGuidListToObjList(PlayerbotAI* ai, const std::list<ObjectGuid>& gos)
{
   std::list<GameObject*> gameobjects;

   for (const ObjectGuid& guid : gos)
   {
      if (GameObject* go = ai->GetGameObject(guid))
      {
         gameobjects.push_back(go);
      }
   }

   return gameobjects;
}

std::list<GameObject*> TellLosAction::FilterGameObjects(Player* requester, const std::list<GameObject*>& gos, const std::vector<LosModifierStruct>& mods)
{
   std::list<GameObject*> gameobjects = gos;

   for (const LosModifierStruct& mod : mods)
   {
      switch (mod.typ)
      {
      case LosModifierType::FilterName:
         {
            auto it = gameobjects.begin();
            while (it != gameobjects.end())
            {
               if ((*it)->GetGOInfo()->name != mod.param)
               {
                  it = gameobjects.erase(it);
                  continue;
               }
               ++it;
            }
         }
         break;
      case LosModifierType::SortRange:
         {
            std::vector<std::pair<float, GameObject*>> distanceObjectPairs;

            for (GameObject* obj : gameobjects)
            {
               float distance = requester->GetDistance(obj);
               distanceObjectPairs.emplace_back(distance, obj);
            }

            std::sort(distanceObjectPairs.begin(), distanceObjectPairs.end(),
               [](const std::pair<float, GameObject*>& a, const std::pair<float, GameObject*>& b)
               {
                  return a.first < b.first;
               });

            gameobjects.clear();
            for (const auto& pair : distanceObjectPairs)
            {
               gameobjects.push_back(pair.second);
            }
         }
         break;
      case LosModifierType::FilterFirst:
         {
            if (gameobjects.size())
            {
               gameobjects.erase(++gameobjects.begin(), gameobjects.end());
            }
         }
         break;
      default:
         break;
      }
   }

   return gameobjects;
}

void TellLosAction::TellGameObjects(Player* requester, std::string title, const std::list<GameObject*>& gos, const std::vector<LosModifierStruct>& mods)
{
   ai->TellPlayer(requester, title);

   bool bShowRange = std::find_if(mods.begin(), mods.end(), [](const LosModifierStruct& el){ return el.typ == LosModifierType::ShowRange; }) != mods.end();

   for (GameObject* go : gos)
   {  
      std::ostringstream ss;

      ss << chat->formatGameobject(go);

      if (bShowRange)
      {
         float distance = sqrtf(requester->GetDistance(go));

         ss << " " << distance << "m";
      }


      ai->TellPlayer(requester, ss.str());
   }
}


std::vector<LosModifierStruct> TellLosAction::ParseLosModifiers(const std::string& str)
{
   std::vector<LosModifierStruct> mods;

   std::vector<std::string> params = ChatHelper::splitString(str, "|");

   for (std::string param : params)
   {
      std::string s = ChatHelper::trim(param);

      if (ChatHelper::startswith(s, "filter:name"))
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::FilterName, s.substr(12)});
      }
      else if (ChatHelper::startswith(s, "sort:range"))
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::SortRange, ""});
      }
      else if (ChatHelper::startswith(s, "filter:first"))
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::FilterFirst, ""});
      }
      else if (ChatHelper::startswith(s, "show:range"))
      {
         mods.emplace_back(LosModifierStruct{ LosModifierType::ShowRange, ""});
      }
   }

   return mods;
}