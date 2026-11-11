
#include "playerbot/playerbot.h"
#include "LootStrategyAction.h"
#include "playerbot/strategy/values/SubStrategyValue.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "LootAction.h"
#include "playerbot/PlayerbotAIAware.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

bool LootStrategyAction::Execute(Event& event)
{
    std::string strategy = event.getParam();
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    LootObjectStack* lootItems = AI_VALUE(LootObjectStack*, "available loot");
    std::set<uint32>& alwaysLootItems = AI_VALUE(std::set<uint32>&, "always loot list");
    std::set<uint32>& skipLootItems = AI_VALUE(std::set<uint32>&, "skip loot list");
    std::set<uint32>& skipGoLootList = AI_VALUE(std::set<uint32>&, "skip go loot list");

    if (strategy == "?")
    {
        {
            std::ostringstream out;
            out << "Loot strategy: ";
            out << AI_VALUE(std::string, "loot strategy");
            ai->TellPlayer(requester, out);
        }

        TellLootList(requester, "always loot list");
        TellLootList(requester, "skip loot list");
        TellGoList(requester, "skip go loot list");
    }
    else if (strategy == "clear")
    {
        alwaysLootItems.clear();
        skipLootItems.clear();
        ai->TellPlayer(requester, "My loot list is now empty");
        return true;
    }    
    else
    {
        std::set<std::string> itemQualifiers = chat->parseItemQualifiers(strategy);
        std::list<ObjectGuid> gos = chat->parseGameobjects(strategy);

        if (itemQualifiers.size() == 0 && gos.size() == 0)
        {
            SET_AI_VALUE(std::string, "loot strategy", strategy);

            std::string lootStrategy = AI_VALUE(std::string, "loot strategy");

            std::ostringstream out;
            out << "Loot strategy set to " << lootStrategy;
            ai->TellPlayer(requester, out);
            return true;
        }

        bool ignore = strategy.size() > 1 && strategy.substr(0, 1) == "!";
        bool remove = strategy.size() > 1 && strategy.substr(0, 1) == "-";
        bool query = strategy.size() > 1 && strategy.substr(0, 1) == "?";
        bool add = !ignore && !remove && !query;
        bool changes = false;
        for (auto& qualifier : itemQualifiers)
        {
            ItemQualifier itemQualifier(qualifier);
            auto itemid = itemQualifier.GetId();
            if (query)
            {
                if (itemQualifier.GetProto())
                {
                    std::ostringstream out;
                    out << (StoreLootAction::IsLootAllowed(itemQualifier, ai) ? "|cFF000000Will loot " : "|c00FF0000Won't loot ") << ChatHelper::formatItem(itemQualifier);
                    ai->TellPlayer(requester, out.str());
                }
            }
            
            if (remove || add)
            {
                std::set<uint32>::iterator j = skipLootItems.find(itemid);
                if (j != skipLootItems.end()) skipLootItems.erase(j);
                changes = true;
            }
            
            if (remove || ignore)
            {
                std::set<uint32>::iterator j = alwaysLootItems.find(itemid);
                if (j != alwaysLootItems.end()) alwaysLootItems.erase(j);
                changes = true;
            }
            
            if (ignore)
            {
                skipLootItems.insert(itemid);
                changes = true;
            }

            if (add)
            {
                alwaysLootItems.insert(itemid);
                changes = true;
            }
        }
            
        for (std::list<ObjectGuid>::iterator i = gos.begin(); i != gos.end(); ++i)
        {
            GameObject *go = ai->GetGameObject(*i);
            if (!go) continue;
            uint32 goId = go->GetGOInfo()->id;

            if (remove || add)
            {
                std::set<uint32>::iterator j = skipGoLootList.find(goId);
                if (j != skipGoLootList.end()) skipGoLootList.erase(j);
                changes = true;
            }

            if (ignore)
            {
                skipGoLootList.insert(goId);
                changes = true;
            }
        }

        if (changes)
        {
            TellLootList(requester, "always loot list");
            TellLootList(requester, "skip loot list");
            TellGoList(requester, "skip go loot list");
            AI_VALUE(LootObjectStack*, "available loot")->Clear();
        }
    }

    return true;
}

void LootStrategyAction::TellLootList(Player* requester, const std::string& name)
{
    std::set<uint32>& alwaysLootItems = AI_VALUE(std::set<uint32>&, name);
    std::ostringstream out;
    out << "My " << name << ":";

    for (std::set<uint32>::iterator i = alwaysLootItems.begin(); i != alwaysLootItems.end(); i++)
    {
        ItemPrototype const *proto = sItemStorage.LookupEntry<ItemPrototype>(*i);
        if (!proto)
        {
            continue;
        }

        out << " " << chat->formatItem(proto);
    }
    
    ai->TellPlayer(requester, out);
}

void LootStrategyAction::TellGoList(Player* requester, const std::string& name)
{
    std::set<uint32>& skipGoItems = AI_VALUE(std::set<uint32>&, name);
    std::ostringstream out;
    out << "My " << name << ":";

    for (std::set<uint32>::iterator i = skipGoItems.begin(); i != skipGoItems.end(); i++)
    {
        uint32 id = *i;
        GameObjectInfo const *proto = sGOStorage.LookupEntry<GameObjectInfo>(id);
        if (!proto)
            continue;

        out << " |cFFFFFF00|Hfound:" << 0 << ":" << id << ":" <<  "|h[" << proto->name << "]|h|r";
    }
    ai->TellPlayer(requester, out);
}

static const char* const kAllLootTokens = "equip,quest,skill,disenchant,use,vendor,trash";

static std::string PrefixTokens(const std::string& tokens, char operation)
{
    std::string out;
    for (const std::string& token : StrSplit(tokens, ","))
    {
        if (!out.empty())
            out += ",";
        out += operation + token;
    }
    return out;
}

bool CollectAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    std::string param = event.getParam();
    std::transform(param.begin(), param.end(), param.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    std::replace(param.begin(), param.end(), ',', ' ');

    std::string lootChanges;
    std::string strategyChanges;
    float distance = 0.0f;
    bool clear = false;
    bool tellSkin = false;
    bool invalid = false;

    auto addLootTokens = [&lootChanges](const std::string& tokens)
    {
        lootChanges += (lootChanges.empty() ? "" : ",") + PrefixTokens(tokens, '+');
    };
    auto addStrategy = [&strategyChanges](const std::string& strategy)
    {
        strategyChanges += (strategyChanges.empty() ? "" : ",") + strategy;
    };

    std::istringstream stream(param);
    std::string token;
    while (stream >> token)
    {
        if (token == "none" || token == "nothing")
        {
            clear = true;
            break;
        }
        else if (token == "all")
        {
            addLootTokens(kAllLootTokens);
            addStrategy("+loot");
            addStrategy("+gather");
            tellSkin = true;
        }
        else if (token == "combat")
            addStrategy("+loot");
        else if (token == "loot")
            addLootTokens(kAllLootTokens);
        else if (token == "quest")
            addLootTokens("quest");
        else if (token == "profession" || token == "skill")
            addLootTokens("skill");
        else if (token == "objects" || token == "nearby")
            addStrategy("+gather");
        else if (token == "skin")
            tellSkin = true;
        else if (token.compare(0, 9, "distance:") == 0)
        {
            char* end = nullptr;
            distance = strtof(token.c_str() + 9, &end);
            if (end == token.c_str() + 9 || *end != '\0' || !(distance >= 1.0f))
                invalid = true;
        }
        else
            invalid = true;
    }

    if (invalid)
    {
        std::ostringstream out;
        out << "Collect <collectable(s)>: none, all, combat, loot, quest, profession, objects, distance:<1-" << (int)sPlayerbotAIConfig.lootDistanceMax << ">";
        ai->TellPlayer(requester, out);
        return true;
    }

    bool changed = false;

    if (clear)
    {
        SET_AI_VALUE(std::string, "loot strategy", PrefixTokens(kAllLootTokens, '-'));
        ai->ChangeStrategy("-loot,-gather", BotState::BOT_STATE_NON_COMBAT);
        AI_VALUE(LootObjectStack*, "available loot")->Clear();
        changed = true;
    }
    else
    {
        if (!lootChanges.empty())
        {
            SET_AI_VALUE(std::string, "loot strategy", lootChanges);
            changed = true;
        }
        if (!strategyChanges.empty())
        {
            ai->ChangeStrategy(strategyChanges, BotState::BOT_STATE_NON_COMBAT);
            changed = true;
        }
    }

    if (distance >= 1.0f)
    {
        bool capped = distance > sPlayerbotAIConfig.lootDistanceMax;
        distance = std::min(distance, sPlayerbotAIConfig.lootDistanceMax);
        SET_AI_VALUE2(float, "range", "loot", distance);
        changed = true;
        if (capped)
            ai->TellPlayer(requester, "Loot distance is capped at " + std::to_string((int)sPlayerbotAIConfig.lootDistanceMax));
    }

    if (changed && !sPlayerbotAIConfig.bExplicitDbStoreSave)
        sPlayerbotDbStore.Save(ai);

    if (tellSkin)
        ai->TellPlayer(requester, "I skin corpses whenever I have the skill.");

    TellCollectState(requester);
    return true;
}

void CollectAction::TellCollectState(Player* requester)
{
    std::vector<std::string> tokens = StrSplit(AI_VALUE(std::string, "loot strategy"), ",");
    auto has = [&tokens](const char* name) { return std::find(tokens.begin(), tokens.end(), name) != tokens.end(); };

    std::vector<std::string> items;
    if (has("trash"))
        items.push_back("all loot");
    else
    {
        if (has("equip") || has("use") || has("vendor") || has("disenchant"))
            items.push_back("useful");
        if (has("quest"))
            items.push_back("quest");
        if (has("skill"))
            items.push_back("profession");
    }

    bool combat = ai->HasStrategy("loot", BotState::BOT_STATE_NON_COMBAT);
    bool objects = ai->HasStrategy("gather", BotState::BOT_STATE_NON_COMBAT);

    std::ostringstream out;
    if (items.empty() && !objects)
        out << "I'm collecting nothing";
    else
    {
        out << "I'm collecting ";
        for (size_t i = 0; i < items.size(); ++i)
            out << (i ? ", " : "") << items[i];
        if (!items.empty())
            out << (combat ? " items after combat" : " items");
        if (objects)
            out << (items.empty() ? "" : " and ") << "nearby objects";
    }

    out << ". Loot distance: " << (int)ai->GetRange("loot");
    ai->TellPlayer(requester, out);
}

static GameObject* FindLinkedGameObject(PlayerbotAI* ai, Player* bot, const std::string& text)
{
    for (ObjectGuid const& guid : ChatHelper::parseGameobjects(text))
    {
        GameObject* go = ai->GetGameObject(guid);
        if (go && sServerFacade.isSpawned(go) && go->GetMapId() == bot->GetMapId() && bot->GetDistance(go) <= sPlayerbotAIConfig.sightDistance)
            return go;
    }

    return nullptr;
}

static GameObject* ResolveLinkedGameObject(PlayerbotAI* ai, Player* bot, Player* requester, const std::string& verb, const std::string& text)
{
    if (ChatHelper::parseGameobjects(text).empty())
    {
        if (text.find_first_not_of(' ') == std::string::npos)
            ai->TellPlayer(requester, "Usage: " + verb + " [object link]. Say 'survey' to see what I can collect around here.");
        return nullptr;
    }

    GameObject* go = FindLinkedGameObject(ai, bot, text);
    if (!go)
        ai->TellPlayer(requester, "I can't see that object from here.");

    return go;
}

bool SurveyAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    std::map<uint32, std::pair<float, GameObject*>> nearestByEntry;
    for (ObjectGuid const& guid : AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los"))
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go)
            continue;

        LootObject loot(bot, guid);
        if (loot.IsEmpty() || !loot.IsLootPossible(bot))
            continue;

        float distance = bot->GetDistance(go);
        auto existing = nearestByEntry.find(go->GetEntry());
        if (existing == nearestByEntry.end() || distance < existing->second.first)
            nearestByEntry[go->GetEntry()] = std::make_pair(distance, go);
    }

    if (nearestByEntry.empty())
    {
        ai->TellPlayer(requester, "I see nothing to collect around here.");
        return true;
    }

    std::vector<std::pair<float, GameObject*>> sorted;
    for (auto const& entry : nearestByEntry)
        sorted.push_back(entry.second);

    std::sort(sorted.begin(), sorted.end(), [](const std::pair<float, GameObject*>& a, const std::pair<float, GameObject*>& b) { return a.first < b.first; });

    std::string line;
    size_t shown = 0;
    for (auto const& entry : sorted)
    {
        std::string link = chat->formatGameobject(entry.second);
        if (shown && line.size() + link.size() > 220)
            break;
        line += link;
        ++shown;
    }

    if (shown < sorted.size())
        line += " +" + std::to_string(sorted.size() - shown) + " more";

    ai->TellPlayer(requester, line);
    return true;
}

bool FindObjectAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    GameObject* go = ResolveLinkedGameObject(ai, bot, requester, "find", event.getParam());
    if (!go)
        return false;

    float angle = go->GetAngle(bot);
    float distance = std::min(INTERACTION_DISTANCE * 0.5f, bot->GetDistance(go));
    float x = go->GetPositionX() + cos(angle) * distance;
    float y = go->GetPositionY() + sin(angle) * distance;
    float z = go->GetPositionZ();
    bot->UpdateAllowedPositionZ(x, y, z);
    WorldPosition spot(go->GetMapId(), x, y, z);

    ai->Reset();
    RESET_AI_VALUE(ObjectGuid, "fetch target");
    ai->ChangeStrategy("+stay,-follow,-wander,-guard,-passive,-fetch", BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+stay,-follow,-wander,-guard,-passive", BotState::BOT_STATE_COMBAT);

    SetPosition(spot);
    SetPosition(spot, "stay");
    MEM_AI_VALUE(WorldPosition, "master position")->Reset();

    ai->TellPlayerNoFacing(requester, "Going to " + chat->formatGameobject(go) + ", I'll wait there. Say 'follow' to call me back.");
    return true;
}

bool GetObjectAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    GameObject* go = ResolveLinkedGameObject(ai, bot, requester, "get", event.getParam());
    if (!go)
        return false;

    ObjectGuid guid = go->GetObjectGuid();
    LootObject loot(bot, guid);
    if (loot.IsEmpty() || !loot.IsLootPossible(bot))
    {
        std::string reason = "there is nothing in it I can take";
        if (go->IsInUse() || go->GetGoState() == GO_STATE_ACTIVE)
            reason = "someone is already using it";
        else if (loot.skillId != SKILL_NONE && !ai->HasSkill((SkillType)loot.skillId))
            reason = "I don't have the skill for it";
        else if (loot.skillId != SKILL_NONE && loot.reqSkillValue > uint32(bot->GetSkillValue(loot.skillId)))
            reason = "my skill is too low";
        else if (loot.skillId == SKILL_MINING && !bot->HasItemCount(2901, 1))
            reason = "I need a mining pick";
        else if (loot.reqItem && !bot->HasItemCount(loot.reqItem, 1))
            reason = "I don't have the key";

        ai->TellPlayer(requester, "I can't collect " + chat->formatGameobject(go) + ": " + reason + ".");
        return false;
    }

    if (!ai->HasStrategy("fetch", BotState::BOT_STATE_NON_COMBAT))
    {
        std::string mode;
        for (const char* movementMode : { "follow", "stay", "guard", "wander" })
        {
            if (ai->HasStrategy(movementMode, BotState::BOT_STATE_NON_COMBAT))
            {
                mode = movementMode;
                break;
            }
        }
        SET_AI_VALUE2(std::string, "manual string", "fetch mode", mode);
    }

    time_t now = time(0);
    SET_AI_VALUE(ObjectGuid, "fetch target", guid);
    SET_AI_VALUE(ObjectGuid, "fetch requester", requester->GetObjectGuid());
    SET_AI_VALUE2(time_t, "manual time", "fetch start", now);
    SET_AI_VALUE2(time_t, "manual time", "fetch tick", now);
    SET_AI_VALUE2(bool, "manual bool", "fetch opened", false);
    ai->ChangeStrategy("+fetch,-follow,-stay,-guard,-wander", BotState::BOT_STATE_NON_COMBAT);

    ai->TellPlayerNoFacing(requester, "Getting " + chat->formatGameobject(go) + ", be right back.");
    return true;
}
