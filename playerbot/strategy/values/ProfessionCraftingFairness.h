#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <map>
#include <vector>

namespace ai
{
    namespace profession
    {
        struct CraftCandidate
        {
            uint32_t skillId = 0;
            uint32_t spellId = 0;
            int64_t score = 0;
            bool ready = false;
        };

        // Runtime skill IDs only. Scores remain the planner's responsibility.
        // An opportunity bounds a stuck focus journey as well as a busy skill;
        // it does not claim that a cast or skill gain actually occurred.
        struct CraftingFairness
        {
            static constexpr uint32_t OpportunitySeconds = 300;
            static constexpr std::size_t NoCandidate = std::numeric_limits<std::size_t>::max();

            std::map<uint32_t, uint32_t> readySince;
            uint32_t ownerSkill = 0;
            uint32_t ownerSpell = 0;
            uint32_t ownerSince = 0;

            std::size_t Select(const std::vector<CraftCandidate>& candidates, uint32_t now, bool pending)
            {
                std::size_t best = NoCandidate;
                std::size_t owner = NoCandidate;
                std::map<uint32_t, bool> readySkills;
                for (std::size_t i = 0; i < candidates.size(); ++i)
                {
                    const CraftCandidate& candidate = candidates[i];
                    if (best == NoCandidate || candidate.score > candidates[best].score)
                        best = i;
                    if (candidate.skillId == ownerSkill && candidate.spellId == ownerSpell)
                        owner = i;
                    if (candidate.ready)
                    {
                        readySkills[candidate.skillId] = true;
                        auto inserted = readySince.emplace(candidate.skillId, now);
                        if (now < inserted.first->second)
                            inserted.first->second = now;
                    }
                }
                for (auto it = readySince.begin(); it != readySince.end();)
                    if (!readySkills.count(it->first))
                        it = readySince.erase(it);
                    else
                        ++it;

                // Replanning may observe consumed materials mid-batch. A
                // pending continuation still owns its recipe until cleanup.
                if (pending)
                    return owner != NoCandidate ? owner : best;
                if (owner != NoCandidate && candidates[owner].ready &&
                    now >= ownerSince && now - ownerSince < OpportunitySeconds)
                    return owner;

                // An expired/invalid owner has spent its opportunity, even if
                // its queued cast never ran. Do not immediately grant it again
                // ahead of another skill that waited through that opportunity.
                auto previous = readySince.find(ownerSkill);
                if (previous != readySince.end())
                    previous->second = now;
                ownerSkill = ownerSpell = ownerSince = 0;

                std::size_t overdue = NoCandidate;
                uint32_t oldest = 0;
                for (std::size_t i = 0; i < candidates.size(); ++i)
                {
                    const CraftCandidate& candidate = candidates[i];
                    if (!candidate.ready)
                        continue;
                    uint32_t since = readySince.at(candidate.skillId);
                    if (now - since < OpportunitySeconds)
                        continue;
                    if (overdue == NoCandidate || since < oldest ||
                        (since == oldest && candidate.score > candidates[overdue].score))
                    {
                        overdue = i;
                        oldest = since;
                    }
                }
                std::size_t selected = overdue != NoCandidate ? overdue : best;
                if (selected != NoCandidate && candidates[selected].ready)
                {
                    ownerSkill = candidates[selected].skillId;
                    ownerSpell = candidates[selected].spellId;
                    ownerSince = now;
                    readySince[ownerSkill] = now;
                }
                return selected;
            }

            void FinishOpportunity(uint32_t now)
            {
                auto owner = readySince.find(ownerSkill);
                if (owner != readySince.end())
                    owner->second = now;
                ownerSkill = ownerSpell = ownerSince = 0;
            }
        };
    }
}
