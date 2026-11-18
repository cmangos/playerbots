#pragma once

#include "Action.h"
#include <memory>
#include <vector>

namespace ai
{
class Queue
{
public:
    void Push(ActionNode* action, float relevance, bool skipPrerequisites, const Event& event);
    ActionNode* Pop(ActionBasket* selection);
    ActionBasket* Peek() const;
    int Size() const { return static_cast<int>(actions.size()); }
    void Clear() { actions.clear(); }
    void RemoveExpired();

private:
    std::vector<std::unique_ptr<ActionBasket>> actions;
};
}
