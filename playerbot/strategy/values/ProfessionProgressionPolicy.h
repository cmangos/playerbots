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

        constexpr bool IsPendingCraftExpired(
            uint32_t now, uint32_t queuedAt, uint32_t timeout, bool casting)
        {
            return !casting && IsCooldownReady(now, queuedAt, timeout);
        }

        constexpr bool IsNcCraftUseful(bool moving, bool autonomousPending)
        {
            // Execute already stops movement before the normal spell check.
            return !moving || autonomousPending;
        }

        constexpr bool KeepsPendingCraft(bool accepted, uint32_t remainingCasts)
        {
            return accepted && remainingCasts > 1;
        }

        constexpr bool ShouldRenewCraftLease(uint32_t pendingSpell, uint32_t requestedSpell, bool acceptedCast)
        {
            return pendingSpell != requestedSpell || acceptedCast;
        }

        constexpr bool CanDispatchProfessionPlan(bool knownRecipe, bool cooldownReady,
            bool pending, bool casting)
        {
            return knownRecipe && cooldownReady && !pending && !casting;
        }

        constexpr uint32_t BoundCraftBatch(uint32_t batch, uint32_t perCast, uint32_t target)
        {
            return perCast ? std::min(batch, std::max(1u, target / perCast)) : batch;
        }

        constexpr uint32_t AvailableCraftBatch(uint32_t batch, uint32_t available)
        {
            // With no complete cast, keep the acquisition plan. Otherwise do
            // not require five casts' materials before making any progress.
            return available ? std::min(batch, available) : batch;
        }

        constexpr bool IsCashVendorStock(bool matchesItem, bool limitedStock, uint32_t extendedCost)
        {
            // Existing vendor usefulness excludes currency offers; classification
            // must not suppress AH fallback for an unusable cash-source route.
            return matchesItem && !limitedStock && !extendedCost;
        }

        constexpr uint32_t SupplyReserveTarget(bool tool, bool reagent, uint32_t materialTarget)
        {
            return tool && !reagent ? 1 : materialTarget;
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
