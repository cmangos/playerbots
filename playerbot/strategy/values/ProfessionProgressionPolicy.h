#pragma once

#include <algorithm>
#include <cstdint>

namespace ai
{
    namespace profession
    {
        struct AssignmentPair
        {
            uint16_t first = 0;
            uint16_t second = 0;
        };

        // Inputs must already be normalized so zero means "not a valid primary
        // profession". Persisted character skills are authoritative whenever
        // they exist. A stored companion is retained for a one-skill character
        // only when the actual skill was part of the stored assignment.
        constexpr AssignmentPair ReconcileAssignments(
            uint16_t storedFirst, uint16_t storedSecond, uint16_t actualFirst, uint16_t actualSecond)
        {
            if (actualFirst && actualSecond && actualFirst != actualSecond)
                return {actualFirst, actualSecond};

            if (actualFirst)
            {
                bool actualWasStored = storedFirst == actualFirst || storedSecond == actualFirst;
                uint16_t storedCompanion = storedFirst == actualFirst ? storedSecond : storedFirst;
                if (!actualWasStored || storedCompanion == actualFirst)
                    storedCompanion = 0;

                return {actualFirst, storedCompanion};
            }

            if (storedFirst == storedSecond)
                storedSecond = 0;

            return {storedFirst, storedSecond};
        }

        constexpr uint32_t CanaryBucket(uint32_t guidLow)
        {
            return (guidLow * 2654435761u) % 100u;
        }

        constexpr bool IsInCanary(uint32_t guidLow, uint32_t percent)
        {
            return percent > 0 && CanaryBucket(guidLow) < std::min<uint32_t>(percent, 100u);
        }

        constexpr bool IsCooldownReady(uint32_t now, uint32_t lastAttempt, uint32_t cooldown)
        {
            return !lastAttempt || now < lastAttempt || now - lastAttempt >= cooldown;
        }

        constexpr uint32_t AuctionPurchaseCapacity(
            uint32_t currentCount, uint32_t requiredNow, uint32_t materialTarget, uint32_t maxStack)
        {
            uint32_t boundedReserve = std::min(materialTarget, maxStack);
            uint32_t requiredTotal = currentCount + requiredNow;
            uint32_t maximumTotal = std::max(requiredTotal, boundedReserve);
            return maximumTotal > currentCount ? maximumTotal - currentCount : 0;
        }

        constexpr bool IsReasonableAuctionStack(
            uint32_t stackCount, uint32_t currentCount, uint32_t requiredNow,
            uint32_t materialTarget, uint32_t maxStack)
        {
            return stackCount > 0 &&
                stackCount <= AuctionPurchaseCapacity(currentCount, requiredNow, materialTarget, maxStack);
        }

        constexpr bool ShouldTravelForSources(
            bool featureEnabledForBot, bool validPlan, bool hasSource, bool hasBudget,
            bool bagSpaceAvailable, bool activeRealPlayerMaster)
        {
            return featureEnabledForBot && validPlan && hasSource && hasBudget &&
                bagSpaceAvailable && !activeRealPlayerMaster;
        }

        constexpr bool ShouldTravelToAuctionHouse(
            bool featureEnabledForBot, bool validPlan, bool hasAuctionMaterial,
            bool ahBuyingEnabled, bool hasBudget, bool cooldownReady,
            bool bagSpaceAvailable, bool activeRealPlayerMaster)
        {
            return ShouldTravelForSources(featureEnabledForBot, validPlan, hasAuctionMaterial,
                       hasBudget, bagSpaceAvailable, activeRealPlayerMaster) &&
                ahBuyingEnabled && cooldownReady;
        }

        constexpr bool ShouldTravelToSpellFocus(
            bool featureEnabledForBot, bool validPlan, uint32_t spellFocusId,
            bool hasMissingReagents, bool cooldownReady, bool activeRealPlayerMaster)
        {
            return featureEnabledForBot && validPlan && spellFocusId &&
                !hasMissingReagents && cooldownReady && !activeRealPlayerMaster;
        }

        constexpr bool IsCraftLocationReady(uint32_t spellFocusId, bool atMatchingSpellFocus)
        {
            return !spellFocusId || atMatchingSpellFocus;
        }
    }
}
