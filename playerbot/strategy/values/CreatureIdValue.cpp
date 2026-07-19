
#include "playerbot/playerbot.h"
#include "CreatureIdValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#ifdef MANGOSBOT_TWO
#include "Entities/Vehicle.h"
#endif

using namespace ai;

CreatureIdValue::CreatureIdValue(PlayerbotAI* ai) : CalculatedValue<uint32>(ai, "creature id", 10), Qualified()
{
}

uint32 CreatureIdValue::Calculate()
{
    std::string namepart = qualifier;
    ItemIds itemIds = ChatHelper::parseItems(namepart);

    PlayerbotChatHandler handler(bot);
    uint32 extractedCreatureId = handler.extractCreatureId(namepart);
    if (extractedCreatureId)
    {
        const CreatureInfo* pCreatureInfo = sServerFacade.LookupCreatureInfo(extractedCreatureId);
        if (pCreatureInfo)
            return extractedCreatureId;
    }

    return 0;
}
