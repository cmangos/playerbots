#include "DatabaseStrategyContext.h"
#include "DatabaseStrategy.h"
#include "playerbot/RandomPlayerbotMgr.h"

using namespace ai;

DatabaseStrategyContext::DatabaseStrategyContext()
{
    for (auto strategyRecord : sRandomPlayerbotMgr.GetStrategyRecords())
    {
        creators[(*strategyRecord).Name] = [strategyRecord](PlayerbotAI* ai) -> Strategy*
        {
            return new DatabaseStrategy(ai, strategyRecord);
        };
    }
}
