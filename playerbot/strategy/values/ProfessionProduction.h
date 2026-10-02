#pragma once

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <vector>

namespace ai { namespace profession
{
    // Metadata for learned recipes only. Processing yields are unknown: one
    // real cast followed by real loot and a new inventory observation.
    struct ProductionRecipe
    {
        uint32_t spellId = 0;
        uint32_t itemId = 0;
        uint32_t yield = 1;
        uint32_t maxCasts = std::numeric_limits<uint32_t>::max();
        bool processing = false;
        bool toolsReady = true;
        std::map<uint32_t, uint32_t> reagents;
    };

    struct ProductionStep
    {
        std::size_t recipe = 0;
        uint32_t casts = 1;
        bool ready = false;
        std::map<uint32_t, uint32_t> required;
        std::map<uint32_t, uint32_t> retained;
    };

    using ProducerIndex = std::map<uint32_t, std::vector<std::size_t>>;

    // Strictly bounded demand expansion, not an arbitrary crafting graph.
    // Root -> learned intermediate -> learned processing is sufficient for
    // ink demand; the same rule also admits bolts, bars and components.
    inline ProductionStep NextProductionStep(const std::vector<ProductionRecipe>& recipes,
        const ProducerIndex& producers, std::size_t root, uint32_t casts, uint32_t batchLimit,
        const std::function<uint32_t(uint32_t)>& inventory,
        std::set<uint32_t> ancestors = {}, unsigned depth = 0)
    {
        const ProductionRecipe& recipe = recipes.at(root);
        ProductionStep result;
        result.recipe = root;
        result.casts = recipe.processing ? 1 : std::max(1u, casts);
        result.ready = recipe.toolsReady;
        ancestors.insert(recipe.spellId);
        for (const auto& reagent : recipe.reagents)
        {
            result.required[reagent.first] = reagent.second * result.casts;
            if (inventory(reagent.first) < result.required[reagent.first])
                result.ready = false;
        }
        result.retained = result.required;
        if (result.ready || depth >= 2)
            return result;

        bool found = false;
        ProductionStep prerequisite;
        for (const auto& reagent : result.required)
        {
            uint32_t owned = inventory(reagent.first);
            if (owned >= reagent.second)
                continue;
            auto sources = producers.find(reagent.first);
            if (sources == producers.end())
                continue;
            for (std::size_t source : sources->second)
            {
                const ProductionRecipe& producer = recipes.at(source);
                if (ancestors.count(producer.spellId))
                    continue;
                uint32_t deficit = reagent.second - owned;
                uint32_t count = 1 + (deficit - 1) / std::max(1u, producer.yield);
                count = std::min(count, std::max(1u, batchLimit));
                count = std::min(count, std::max(1u, producer.maxCasts));
                // Use a partial executable batch before requesting more input.
                uint32_t available = count;
                for (const auto& input : producer.reagents)
                    available = std::min(available, inventory(input.first) / input.second);
                if (available)
                    count = available;
                ProductionStep step = NextProductionStep(recipes, producers, source,
                    count, batchLimit, inventory, ancestors, depth + 1);
                for (const auto& reserve : result.retained)
                    step.retained[reserve.first] = std::max(step.retained[reserve.first], reserve.second);
                // A ready prerequisite wins. Otherwise retain a stable metadata
                // order and expose its real raw inputs to existing acquisition.
                if (!found || (step.ready && !prerequisite.ready))
                {
                    prerequisite = step;
                    found = true;
                }
            }
        }
        return found ? prerequisite : result;
    }
}}
