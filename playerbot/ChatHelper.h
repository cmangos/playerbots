#pragma once
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

typedef std::set<uint32> ItemIds;
typedef std::set<uint32> SpellIds;

namespace ai
{
    class ItemQualifier;

    class WorldPosition;
    class GuidPosition;

    class ChatHelper : public PlayerbotAIAware
    {
    public:
        ChatHelper(PlayerbotAI* ai);

    public:
        static std::string formatMoney(uint32 copper);
        static uint32 parseMoney(std::string& text);

        static std::string formatQuest(Quest const* quest);

        static std::string formatItem(ItemQualifier& itemQualifier, int count = 0, int total = 0);
        static std::string formatItem(ItemPrototype const * proto, int count = 0, int total = 0);
        static std::string formatItem(Item* item, int count = 0, int total = 0);
        static std::string formatQItem(uint32 itemId);
        static ItemIds parseItems(const std::string& text, bool validate = false);
        static std::vector<uint32> parseItemsUnordered(const std::string& text, bool validate = false);
        static std::set<std::string> parseItemQualifiers(std::string& text);
        static uint32 parseItemQuality(std::string text);
        static bool parseItemClass(std::string text, uint32* itemClass, uint32* itemSubClass);
        static uint32 parseSlot(std::string text);

        static std::string formatSpell(SpellEntry const *sInfo);
        static std::string formatSpell(uint32 spellId) {const SpellEntry* const spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId); if (!spellInfo) return ""; return formatSpell(spellInfo);};
        uint32 parseSpell(std::string& text);

        static std::string formatGameobject(GameObject* go);
        static std::list<ObjectGuid> parseGameobjects(std::string& text);

        static std::string formatWorldobject(WorldObject* wo);

        static std::string formatWorldEntry(int32 entry);
        static std::list<int32> parseWorldEntries(std::string& text);

        static std::string formatQuestObjective(std::string name, int available, int required);

        static std::string formatValue(std::string type, std::string code, std::string name, std::string color = "0000FFFF");
        static std::string parseValue(std::string type, std::string& text);

        static std::string formatChat(ChatMsg chat);
        static ChatMsg parseChat(std::string& text);

        static std::string specName(Player* player);
        static std::string formatClass(Player* player, int spec);
        static std::string formatClass(uint8 cls);

        static std::string formatRace(uint8 race);

        static std::string formatSkill(uint32 skill);
        uint32 parseSkill(std::string& text);

        static std::string formatAngle(float angle);
        static std::string formatWorldPosition(WorldPosition& pos);
        static std::string formatGuidPosition(GuidPosition& guidP);

        static std::string formatBoolean(bool flag);       
       
        static bool parseable(std::string text);

        void eraseAllSubStr(std::string& mainStr, const std::string& toErase);

        static void PopulateSpellNameList();
        static std::vector<uint32> SpellIds(const std::string& name);

        static std::vector<std::string> splitString(const std::string& text, const std::string& delimiter);
        static std::vector<std::string> findSubstringsBetween(const std::string& input, const std::string& start, const std::string& end, bool includeDelimiters = false);
        static void replaceSubstring(std::string& str, const std::string& oldStr, const std::string& newStr);

    private:
        static std::map<std::string, uint32> consumableSubClasses;
        static std::map<std::string, uint32> tradeSubClasses;
        static std::map<std::string, uint32> itemQualities;
        static std::map<std::string, uint32> projectileSubClasses;
        static std::map<std::string, std::pair<uint32, uint32>> itemClasses;
        static std::map<std::string, uint32> slots;
        static std::map<std::string, uint32> skills;
        static std::map<std::string, ChatMsg> chats;
        static std::map<uint8, std::string> classes;
        static std::map<uint8, std::string> races;
        static std::map<uint8, std::map<uint8, std::string> > specs;
        static std::unordered_map<std::string, std::vector<uint32>> spellIds;
    };
};
