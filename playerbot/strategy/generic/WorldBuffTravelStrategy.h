#pragma once
#include "playerbot/strategy/Strategy.h"

namespace ai
{
    enum class WorldBuffTravelStep : uint8
    {
        STEP_STORMWIND = 0,         // Rallying Cry of the Dragonslayer
        STEP_GRYPHON_MASTER,        // Waypoint: travel to Stormwind flight master (Alliance only, Horde skips)
        STEP_BOOTY_BAY,             // Spirit of Zandalar
        STEP_BRAGOK,                // Waypoint: travel to Ratchet flight master
        STEP_THYSSIANA,             // Waypoint: travel to flight master (Alliance: Thyssiana, Horde: Shyn)
        STEP_FEATHERMOON,           // Waypoint: arrived in Feralas, head off the island (Alliance only, Horde skips)
        STEP_FORGOTTEN_COAST,       // Waypoint: reach Zorbin Fandazzle on Forgotten Coast (Alliance only, Horde skips)
        STEP_DM_TRAVEL,             // Travel to Dire Maul North door
        STEP_DM_INSIDE,             // Inside DM North — get tribute buffs
        STEP_DM_EXIT,               // Leave instance back to overworld
        STEP_DM_PORTAL,             // Mages cast Portal (Alliance: Darnassus, Horde: Orgrimmar), everyone takes it
        STEP_FELWOOD,               // Waypoint: arrive in Felwood zone (Alliance: Mishellena, Horde: Brakkar)
        STEP_SONGFLOWER,            // Travel to a Cleansed Songflower GO and use it
        STEP_PORTAL_HOME,           // Mages cast Portal home (Alliance: Stormwind, Horde: Orgrimmar), everyone takes it
        STEP_DONE,                  // All buffs collected
        STEP_COUNT
    };

    // Spell IDs for the world buffs (Classic values)
    enum WorldBuffTravelSpells : uint32
    {
        SPELL_RALLYING_CRY = 22888,    // Rallying Cry of the Dragonslayer
        SPELL_SPIRIT_OF_ZANDALAR = 24425,    // Spirit of Zandalar
        SPELL_DM_TRIBUTE_FENGUS = 22817,    // Fengus' Ferocity (DMT)
        SPELL_DM_TRIBUTE_SLIPKIK = 22820,    // Slip'kik's Savvy (DMT)
        SPELL_DM_TRIBUTE_GORDOK = 22818,    // Mol'dar's Moxie (DMT)
        SPELL_SONGFLOWER = 15366,    // Songflower Serenade
        SPELL_WARCHIEFS_BLESSING = 16609,    // Warchief's Blessing (Horde only)
        SPELL_PORTAL_STORMWIND = 10059,    // Portal: Stormwind (Mage)
        SPELL_PORTAL_DARNASSUS = 11419,    // Portal: Darnassus (Mage)
        SPELL_PORTAL_ORGRIMMAR = 11417,    // Portal: Orgrimmar (Mage)
    };

    // NPC entry IDs for waypoint steps
    enum WorldBuffTravelNpcs : uint32
    {
        NPC_DUNGAR_LONGDRINK = 352,      // Stormwind gryphon master (Alliance)
        NPC_BRAGOK = 16227,    // Ratchet flight master
        NPC_THYSSIANA = 4319,     // Thalanaar flight master (Alliance)
        NPC_ZORBIN_FANDAZZLE = 14637,    // Forgotten Coast, Feralas (Alliance mainland waypoint)
        NPC_MISHELLENA = 12578,    // Felwood waypoint NPC (Alliance)
        NPC_SHYN = 8020,     // Camp Mojache flight master (Horde)
        NPC_BRAKKAR = 11900,    // Felwood waypoint NPC (Horde)
    };

    // Taxi node IDs for flight destinations
    enum WorldBuffTravelTaxiNodes : uint32
    {
        TAXI_NODE_BOOTY_BAY = 19,     // Booty Bay, Stranglethorn
        TAXI_NODE_THALANAAR = 42,     // Thalanaar, Feralas (Alliance)
        TAXI_NODE_FEATHERMOON = 31,     // Feathermoon, Feralas (Alliance)
        TAXI_NODE_CAMP_MOJACHE = 44,     // Camp Mojache, Feralas (Horde)
    };

    // Game object entry IDs
    enum WorldBuffTravelGameObjects : uint32
    {
        GO_DM_NORTH_DOOR = 177192,      // Dire Maul North door (guid 49948)
        GO_BONFIRE_FERALAS = 176318,     // Bonfire near DM, Feralas (Horde waypoint for STEP_FORGOTTEN_COAST)
    };

    constexpr uint32 GO_DM_NORTH_DOOR_GUID = 49948;

    constexpr uint32 GO_SONGFLOWER_ENTRIES[] =
    {
        164886, 171939, 171942, 174594, 174595,
        174596, 174597, 174598, 174712, 174713
    };
    constexpr uint32 GO_SONGFLOWER_ENTRY_COUNT = sizeof(GO_SONGFLOWER_ENTRIES) / sizeof(GO_SONGFLOWER_ENTRIES[0]);

    enum WorldBuffTravelZones : uint32
    {
        ZONE_STORMWIND = 1519,
        ZONE_ORGRIMMAR = 1637,
        AREA_BOOTY_BAY = 35,
        MAP_DIRE_MAUL = 429,
        ZONE_DIRE_MAUL = 2557,
        ZONE_FERALAS = 357,
        ZONE_FELWOOD = 361,
        ZONE_DARNASSUS = 1657,
    };

    constexpr uint32 AREATRIGGER_DM_NORTH = 3193;

    constexpr float PORTAL_REGROUP_DISTANCE = 100.0f;

    inline bool IsHordeFaction(Player* player)
    {
        return player->GetTeam() == HORDE;
    }

    inline uint32 GetHomeZone(Player* player)
    {
        return IsHordeFaction(player) ? ZONE_ORGRIMMAR : ZONE_STORMWIND;
    }

    inline uint32 GetDMPortalDestZone(Player* player)
    {
        return IsHordeFaction(player) ? ZONE_ORGRIMMAR : ZONE_DARNASSUS;
    }

    inline uint32 GetDMPortalSpell(Player* player)
    {
        return IsHordeFaction(player) ? SPELL_PORTAL_ORGRIMMAR : SPELL_PORTAL_DARNASSUS;
    }

    inline uint32 GetHomePortalSpell(Player* player)
    {
        return IsHordeFaction(player) ? SPELL_PORTAL_ORGRIMMAR : SPELL_PORTAL_STORMWIND;
    }

    inline const char* GetDMPortalKeyword(Player* player)
    {
        return IsHordeFaction(player) ? "Orgrimmar" : "Darnassus";
    }

    inline const char* GetHomePortalKeyword(Player* player)
    {
        return IsHordeFaction(player) ? "Orgrimmar" : "Stormwind";
    }

    inline bool IsSongflowerEntry(uint32 entry)
    {
        for (uint32 i = 0; i < GO_SONGFLOWER_ENTRY_COUNT; ++i)
            if (GO_SONGFLOWER_ENTRIES[i] == entry)
                return true;
        return false;
    }

    inline bool HasAllDMTributeBuffs(Player* player)
    {
        return player->HasAura(SPELL_DM_TRIBUTE_FENGUS)
            && player->HasAura(SPELL_DM_TRIBUTE_SLIPKIK)
            && player->HasAura(SPELL_DM_TRIBUTE_GORDOK);
    }

    inline bool IsAllianceOnlyStep(WorldBuffTravelStep step)
    {
        return step == WorldBuffTravelStep::STEP_GRYPHON_MASTER
            || step == WorldBuffTravelStep::STEP_FEATHERMOON;
    }

    inline bool AreAllGroupMembersNearby(Player* player)
    {
        Group* group = player->GetGroup();
        if (!group)
            return true;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            if (member == player)
                continue;

            if (!member)
                return false;

            if (!member->IsAlive())
                continue;

            if (member->GetMapId() != player->GetMapId())
                return false;

            if (member->IsTaxiFlying())
                return false;

            if (player->GetDistance(member) > PORTAL_REGROUP_DISTANCE)
                return false;
        }
        return true;
    }

    inline WorldBuffTravelStep GetFirstNeededStep(Player* player)
    {
        bool horde = IsHordeFaction(player);

        if (!player->HasAura(SPELL_RALLYING_CRY))
            return WorldBuffTravelStep::STEP_STORMWIND;

        if (!player->HasAura(SPELL_SPIRIT_OF_ZANDALAR))
        {
            return horde ? WorldBuffTravelStep::STEP_BOOTY_BAY
                : WorldBuffTravelStep::STEP_GRYPHON_MASTER;
        }

        if (!HasAllDMTributeBuffs(player))
            return WorldBuffTravelStep::STEP_BRAGOK;

        uint32 zoneId = player->GetZoneId();
        uint32 portalDestZone = GetDMPortalDestZone(player);
        if (zoneId != ZONE_FELWOOD && zoneId != portalDestZone)
        {
            if (!player->HasAura(SPELL_SONGFLOWER))
                return WorldBuffTravelStep::STEP_DM_INSIDE;
        }

        if (!player->HasAura(SPELL_SONGFLOWER))
            return WorldBuffTravelStep::STEP_FELWOOD;

        uint32 homeZone = GetHomeZone(player);
        if (zoneId != homeZone)
            return WorldBuffTravelStep::STEP_PORTAL_HOME;

        return WorldBuffTravelStep::STEP_DONE;
    }

    class WorldBuffTravelMultiplier : public Multiplier
    {
    public:
        WorldBuffTravelMultiplier(PlayerbotAI* ai) : Multiplier(ai, "world buff travel") {}
        float GetValue(Action* action) override;
    };

    class WorldBuffTravelStrategy : public Strategy
    {
    public:
        WorldBuffTravelStrategy(PlayerbotAI* ai) : Strategy(ai) {}
        int GetType() override { return STRATEGY_TYPE_NONCOMBAT; }
        std::string getName() override { return "wbuff travel"; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "wbuff travel"; }
        virtual std::string GetHelpDescription() {
            return "This strategy makes bots travel to specific zones in a scripted sequence to collect world buffs.";
        }
        virtual std::vector<std::string> GetRelatedStrategies() { return { "wbuff", "travel" }; }
#endif

    protected:
        void InitNonCombatTriggers(std::list<TriggerNode*>& triggers) override;
        void InitNonCombatMultipliers(std::list<Multiplier*>& multipliers) override;
        void OnStrategyAdded(BotState state) override;
        void OnStrategyRemoved(BotState state) override;
    };
}