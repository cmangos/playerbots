
#include "playerbot/playerbot.h"
#include "GuildValues.h"

using namespace ai;


uint8 PetitionSignsValue::Calculate()
{
    if (bot->GetGuildId())
       return 0;

    std::list<Item*> petitions = AI_VALUE2(std::list<Item*>, "inventory items", chat->formatQItem(5863));

    if (petitions.empty())
        return 0;

    auto result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = '%u'", petitions.front()->GetObjectGuid().GetCounter());

    return result ? (uint8)result->GetRowCount() : 0;
};
