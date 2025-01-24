
#include "playerbot/playerbot.h"
#include "GenericActions.h"
#include "playerbot/PlayerbotFactory.h"

using namespace ai;

bool MeleeAction::isUseful()
{
    // do not allow if can't attack from vehicle
    if (ai->IsInVehicle() && !ai->IsInVehicle(false, false, true))
        return false;

    return true;
}

bool UpdateStrategyDependenciesAction::Execute(Event& event)
{
    if (!strategiesToAdd.empty() || !strategiesToRemove.empty())
    {
        // Strategies to add
        for (const StrategyToUpdate* strategy : strategiesToAdd)
        {
            std::stringstream changeStr;
            changeStr << "+" << strategy->name;
            ai->ChangeStrategy(changeStr.str(), strategy->state);
        }

        // Strategies to remove
        for (const StrategyToUpdate* strategy : strategiesToRemove)
        {
            std::stringstream changeStr;
            changeStr << "-" << strategy->name;
            ai->ChangeStrategy(changeStr.str(), strategy->state);
        }

        return true;
    }

    return false;
}

bool UpdateStrategyDependenciesAction::isUseful()
{
    if (!strategiesToUpdate.empty())
    {
        strategiesToAdd.clear();
        strategiesToRemove.clear();
        for (const StrategyToUpdate& strategy : strategiesToUpdate)
        {
            // Ignore if the strategies required are not found
            bool requiredStrategyMissing = false;
            for (const std::string& strategyRequired : strategy.strategiesRequired)
            {
                // Check if the strategy required has any aliases
                std::vector<std::string> strategyRequiredAliases = { strategyRequired };
                if (strategyRequired.find("/") != std::string::npos)
                {
                    strategyRequiredAliases.clear();
                    std::string alias;
                    std::stringstream ss(strategyRequired);
                    while (std::getline(ss, alias, '/'))
                    {
                        strategyRequiredAliases.push_back(alias);
                    }
                }

                bool synonymFound = false;
                for (const std::string& strategyRequiredAlias : strategyRequiredAliases)
                {
                    if (ai->HasStrategy(strategyRequiredAlias, strategy.state))
                    {
                        synonymFound = true;
                        break;
                    }
                }

                if (!synonymFound)
                {
                    requiredStrategyMissing = true;
                    break;
                }
            }

            if (requiredStrategyMissing)
            {
                // Check if we need to remove the strategy
                if (ai->HasStrategy(strategy.name, strategy.state))
                {
                    strategiesToRemove.emplace_back(&strategy);
                }
            }
            else
            {
                // Check if we need to add the strategy
                if (!ai->HasStrategy(strategy.name, strategy.state))
                {
                    strategiesToAdd.emplace_back(&strategy);
                }
            }
        }
    }

    return !strategiesToAdd.empty() || !strategiesToRemove.empty();
}