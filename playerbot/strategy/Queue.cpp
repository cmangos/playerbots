
#include "playerbot/playerbot.h"
#include "Queue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include <algorithm>

using namespace ai;

void Queue::Push(ActionNode* action, float relevance, bool skipPrerequisites, const Event& event)
{
    std::unique_ptr<ActionNode> node(action);
    if (!node)
        return;

    for (const auto& basket : actions)
    {
        if (node->getName() != basket->getAction()->getName())
            continue;

        if (relevance > basket->getRelevance())
        {
            basket->setRelevance(relevance);
            basket->setEvent(event);
        }
        return;
    }

    actions.push_back(std::make_unique<ActionBasket>(std::move(node), relevance, skipPrerequisites, event));
}

ActionNode* Queue::Pop(ActionBasket* selection)
{
    auto const iter = std::find_if(actions.begin(), actions.end(),
        [selection](const auto& basket) { return basket.get() == selection; });
    if (iter == actions.end())
        return nullptr;

    ActionNode* action = (*iter)->releaseAction();
    actions.erase(iter);
    return action;
}

ActionBasket* Queue::Peek() const
{
    float max = -400;
    ActionBasket* selection = nullptr;
    for (const auto& basket : actions)
    {
        if (basket->getRelevance() > max)
        {
            max = basket->getRelevance();
            selection = basket.get();
        }
    }
    return selection;
}

void Queue::RemoveExpired()
{
    auto const expireTime = sPlayerbotAIConfig.expireActionTime;
    if (!expireTime || actions.empty())
        return;

    time_t const now = time(nullptr);
    auto const expired = std::remove_if(actions.begin(), actions.end(), [now, expireTime](const auto& basket)
    {
        if (!basket->isExpired(now, expireTime / 1000))
            return false;

        sLog.outDebug("Action %s is expired", basket->getAction()->getName().c_str());
        return true;
    });
    actions.erase(expired, actions.end());
}
