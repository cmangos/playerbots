
#include "playerbot/playerbot.h"
#include <stdarg.h>
#include "AiObjectContext.h"
#include "Action.h"
#include "Entities/Unit.h"
#include <vector>

using namespace ai;

int NextAction::size(NextAction** actions)
{
    if (!actions)
        return 0;

    int size;
    for (size=0; actions[size]; )
        size++;
    return size;
}

NextAction** NextAction::clone(NextAction** actions)
{
    if (!actions)
        return NULL;

    int size = NextAction::size(actions);

    NextAction** res = new NextAction*[size + 1];
    for (int i=0; i<size; i++)
        res[i] = new NextAction(*actions[i]);
    res[size] = NULL;
    return res;
}

NextAction** NextAction::merge(NextAction** left, NextAction** right)
{
    int leftSize = NextAction::size(left);
    int rightSize = NextAction::size(right);

    NextAction** res = new NextAction*[leftSize + rightSize + 1];
    for (int i = 0; i < leftSize; i++)
        res[i] = new NextAction(*left[i]);
    for (int i = 0; i < rightSize; i++)
        res[leftSize + i] = new NextAction(*right[i]);
    res[leftSize + rightSize] = NULL;

    return res;
}

//NextAction** NextAction::array(uint8 nil, ...)
NextAction** NextAction::array(uint32 n, ...)
{
    va_list vl;
    va_start(vl, n);

    std::vector<NextAction*> tmp;
    NextAction* cur = nullptr;
    while ((cur = va_arg(vl, NextAction*)))
        tmp.push_back(cur);

    va_end(vl);

    NextAction** res = new NextAction*[tmp.size() + 1];
    for (size_t i = 0; i < tmp.size(); ++i)
        res[i] = tmp[i];
    res[tmp.size()] = nullptr;

    return res;
}

void NextAction::destroy(NextAction** actions)
{
    if (!actions)
        return;

    for (int i=0; actions[i]; i++)
        delete actions[i];

    delete[] actions;
}

Value<Unit*>* Action::GetTargetValue()
{
    return context->GetValue<Unit*>(GetTargetName());
}

Unit* Action::GetTarget()
{
    return GetTargetValue()->Get();
}