#include "playerbot/playerbot.h"
#include "WorldBuffTravelActions.h"
#include "ChooseTravelTargetAction.h"
#include "playerbot/TravelMgr.h"


using namespace ai;

static const char* GetStepZoneName(WorldBuffTravelStep step, Player* bot)
{
    bool horde = IsHordeFaction(bot);

    switch (step)
    {
    case WorldBuffTravelStep::STEP_STORMWIND:         return horde ? "Orgrimmar" : "Stormwind";
    case WorldBuffTravelStep::STEP_GRYPHON_MASTER:    return horde ? "" : "Dungar Longdrink";
    case WorldBuffTravelStep::STEP_BOOTY_BAY:         return "Booty Bay";
    case WorldBuffTravelStep::STEP_BRAGOK:            return "Bragok";
    case WorldBuffTravelStep::STEP_THYSSIANA:         return horde ? "Shyn" : "Thyssiana";
    case WorldBuffTravelStep::STEP_FEATHERMOON:       return horde ? "" : "Feathermoon Stronghold";
    case WorldBuffTravelStep::STEP_FORGOTTEN_COAST:   return horde ? "Bonfire" : "Zorbin Fandazzle";
    case WorldBuffTravelStep::STEP_DM_TRAVEL:         return "Dire Maul";
    case WorldBuffTravelStep::STEP_FELWOOD:           return horde ? "Brakkar" : "Mishellena";
    case WorldBuffTravelStep::STEP_SONGFLOWER:        return "Songflower";
    default:                                          return "";
    }
}

static bool IsStepGoSearch(WorldBuffTravelStep step, Player* bot)
{
    bool horde = IsHordeFaction(bot);

    if (step == WorldBuffTravelStep::STEP_BRAGOK ||
        step == WorldBuffTravelStep::STEP_THYSSIANA ||
        step == WorldBuffTravelStep::STEP_FELWOOD)
        return true;

    if (!horde)
    {
        if (step == WorldBuffTravelStep::STEP_GRYPHON_MASTER ||
            step == WorldBuffTravelStep::STEP_FORGOTTEN_COAST)
            return true;
    }

    return false;
}

static uint32 GetStepNpcEntry(WorldBuffTravelStep step, Player* bot)
{
    bool horde = IsHordeFaction(bot);

    switch (step)
    {
    case WorldBuffTravelStep::STEP_GRYPHON_MASTER:    return horde ? 0 : NPC_DUNGAR_LONGDRINK;
    case WorldBuffTravelStep::STEP_BRAGOK:            return NPC_BRAGOK;
    case WorldBuffTravelStep::STEP_THYSSIANA:         return horde ? NPC_SHYN : NPC_THYSSIANA;
    case WorldBuffTravelStep::STEP_FORGOTTEN_COAST:   return horde ? 0 : NPC_ZORBIN_FANDAZZLE;
    case WorldBuffTravelStep::STEP_FELWOOD:           return horde ? NPC_BRAKKAR : NPC_MISHELLENA;
    default:                                          return 0;
    }
}

static const GameObjectData* FindClosestSongflowerSpawn(Player* bot)
{
    const GameObjectData* best = nullptr;
    float bestDistSq = std::numeric_limits<float>::max();
    uint32 botMapId = bot->GetMapId();
    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();

    struct SongflowerFinder
    {
        uint32 mapId;
        float x, y, z;
        const GameObjectData* best;
        float bestDistSq;

        bool operator()(GameObjectDataPair const& dataPair)
        {
            const GameObjectData& data = dataPair.second;
            if (!IsSongflowerEntry(data.id))
                return false;

            if (data.mapid != mapId)
                return false;

            float dx = data.posX - x;
            float dy = data.posY - y;
            float dz = data.posZ - z;
            float distSq = dx * dx + dy * dy + dz * dz;

            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                best = &data;
            }
            return false;
        }
    };

    SongflowerFinder finder{ botMapId, botX, botY, botZ, nullptr, bestDistSq };
    sObjectMgr.DoGOData(finder);
    return finder.best;
}

static GameObject* FindNearbyPortalGO(PlayerbotAI* ai, Player* bot,
    const std::list<ObjectGuid>& gos, const char* destKeyword)
{
    for (auto& guid : gos)
    {
        GameObject* go = ai->GetGameObject(guid);
        if (!go)
            continue;

        GameObjectInfo const* goInfo = go->GetGOInfo();
        if (!goInfo || goInfo->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        uint32 spellId = goInfo->spellcaster.spellId;
        const SpellEntry* const pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS &&
            pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS &&
            pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        if (destKeyword && destKeyword[0])
        {
            std::string name = goInfo->name;
            if (name.find(destKeyword) == std::string::npos)
                continue;
        }

        return go;
    }
    return nullptr;
}

static bool SummonPlayerToSummoner(Player* summoner, Player* target, PlayerbotAI* summonerAI)
{
    if (!summoner || !target || summoner == target)
        return false;

    if (summoner->IsBeingTeleported() || target->IsBeingTeleported())
        return false;

    float x, y, z;
    summoner->GetPosition(x, y, z);

    if (target->isRealPlayer())
    {
        target->SetSummonPoint(summoner->GetMapId(), x, y, z, summoner->GetObjectGuid());

        WorldPacket data(SMSG_SUMMON_REQUEST, 8 + 4 + 4);
        data << summoner->GetObjectGuid();
        data << uint32(summoner->GetZoneId());
        data << uint32(MAX_PLAYER_SUMMON_DELAY * IN_MILLISECONDS);
        target->GetSession()->SendPacket(data);
    }
    else
    {
        target->TeleportTo(summoner->GetMapId(), x, y, z, summoner->GetOrientation());
    }

    return true;
}

// Apply a buff to the bot and also share it with any real players in the group
void WorldBuffTravelApplyAction::ApplyBuffToSelfAndRealPlayers(uint32 spellId)
{
    PlayerbotAI::AddAura(bot, spellId);

    std::vector<Player*> members = ai->GetPlayersInGroup();
    for (Player* member : members)
    {
        if (!member || member == bot)
            continue;

        if (!ai->IsRealPlayer(member))
            continue;

        if (member->HasAura(spellId))
            continue;

        if (member->GetMapId() != bot->GetMapId())
            continue;

        if (bot->GetDistance(member) > 100.0f)
            continue;

        PlayerbotAI::AddAura(member, spellId);
    }
}

bool WorldBuffTravelApplyAction::TakeFlightFromMaster(uint32 npcEntry, uint32 destTaxiNode)
{
    if (bot->IsTaxiFlying())
        return true;

    std::list<ObjectGuid> npcs = AI_VALUE(std::list<ObjectGuid>, "nearest npcs no los");
    for (auto& guid : npcs)
    {
        Unit* unit = ai->GetUnit(guid);
        if (!unit || !unit->IsCreature())
            continue;

        if (unit->GetEntry() != npcEntry)
            continue;

        Creature* flightMaster = bot->GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_FLIGHTMASTER);
        if (!flightMaster)
            continue;

        bot->GetSession()->SendLearnNewTaxiNode(flightMaster);

        uint32 srcNode = sObjectMgr.GetNearestTaxiNode(
            flightMaster->GetPositionX(), flightMaster->GetPositionY(),
            flightMaster->GetPositionZ(), flightMaster->GetMapId(), bot->GetTeam());

        if (!srcNode)
            return false;

        uint32 savedMoney = bot->GetMoney();
        bot->SetMoney(savedMoney + 100000);

        bool didFly = bot->ActivateTaxiPathTo({ srcNode, destTaxiNode }, flightMaster, 0);

        bot->SetMoney(savedMoney);

        return didFly;
    }

    return false;
}

// If bot is a warlock at a regrouping step, summon far away group members and sync their step.
bool WorldBuffTravelApplyAction::TrySummonFarAwayMembers(WorldBuffTravelStep step)
{
    if (bot->getClass() != CLASS_WARLOCK)
        return false;

    if (step != WorldBuffTravelStep::STEP_BOOTY_BAY &&
        step != WorldBuffTravelStep::STEP_DM_TRAVEL &&
        step != WorldBuffTravelStep::STEP_DM_PORTAL &&
        step != WorldBuffTravelStep::STEP_SONGFLOWER &&
        step != WorldBuffTravelStep::STEP_PORTAL_HOME)
        return false;

    const Group* group = bot->GetGroup();
    if (!group)
        return false;

    static std::map<ObjectGuid, time_t> lastSummonAttempt;
    time_t now = time(nullptr);
    ObjectGuid botGuid = bot->GetObjectGuid();
    auto it = lastSummonAttempt.find(botGuid);
    if (it != lastSummonAttempt.end() && now < it->second)
        return true;

    uint32 cooldownSeconds = urand(1, 5);
    lastSummonAttempt[botGuid] = now + cooldownSeconds;

    uint8 warlockStep = static_cast<uint8>(step);
    uint32 pendingCount = 0;

    std::vector<Player*> botsToSummon;
    std::vector<Player*> realPlayersToSummon;

    const Group::MemberSlotList& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); ++itr)
    {
        if (itr->guid == bot->GetObjectGuid())
            continue;

        Player* member = sObjectMgr.GetPlayer(itr->guid);
        if (!member)
        {
            ++pendingCount;
            continue;
        }

        if (!member->IsAlive())
            continue;

        bool needsSummonByStep = false;
        PlayerbotAI* memberAI = member->GetPlayerbotAI();
        if (memberAI)
        {
            uint8 memberStep = memberAI->GetAiObjectContext()->GetValue<uint8>("world buff travel step")->Get();
            if (memberStep > warlockStep)
                continue;
            if (memberStep < warlockStep)
                needsSummonByStep = true;
        }

        bool differentMap = member->GetMapId() != bot->GetMapId();
        bool farAway = !differentMap && bot->GetDistance(member) > PORTAL_REGROUP_DISTANCE;

        if (!needsSummonByStep && !differentMap && !farAway)
            continue;

        if (member->IsBeingTeleported())
        {
            ++pendingCount;
            continue;
        }

        if (member->isRealPlayer())
            realPlayersToSummon.push_back(member);
        else
            botsToSummon.push_back(member);
    }

    Player* toSummon = nullptr;
    {
        std::vector<Player*> allToSummon;
        allToSummon.insert(allToSummon.end(), realPlayersToSummon.begin(), realPlayersToSummon.end());
        allToSummon.insert(allToSummon.end(), botsToSummon.begin(), botsToSummon.end());
        if (!allToSummon.empty())
            toSummon = allToSummon[urand(0, allToSummon.size() - 1)];
    }

    bool didSummon = false;
    if (toSummon)
    {
        if (SummonPlayerToSummoner(bot, toSummon, ai))
        {
            ai->TellPlayer(GetMaster(), "Summoning " + std::string(toSummon->GetName()) + " to the group!");

            // Sync the summoned member's step to the warlock's step
            PlayerbotAI* memberAI = toSummon->GetPlayerbotAI();
            if (memberAI)
            {
                memberAI->GetAiObjectContext()->GetValue<uint8>("world buff travel step")->Set(warlockStep);

                TravelTarget* memberTravel = memberAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();
                if (memberTravel)
                {
                    memberTravel->SetForced(false);
                    memberTravel->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);
                }
            }

            didSummon = true;
        }
    }

    uint32 remainingToSummon = botsToSummon.size() + realPlayersToSummon.size();
    if (didSummon && remainingToSummon > 0)
        --remainingToSummon;

    if (!didSummon && pendingCount == 0 && remainingToSummon == 0)
        lastSummonAttempt.erase(botGuid);

    return didSummon || (remainingToSummon > 0) || (pendingCount > 0);
}

void WorldBuffTravelApplyAction::ApplyBuffsForStep(WorldBuffTravelStep step)
{
    bool horde = IsHordeFaction(bot);

    switch (step)
    {
    case WorldBuffTravelStep::STEP_STORMWIND:
        ApplyBuffToSelfAndRealPlayers(SPELL_RALLYING_CRY);
        ai->TellPlayer(GetMaster(), "Gained Rallying Cry of the Dragonslayer!");
        break;
    case WorldBuffTravelStep::STEP_GRYPHON_MASTER:
        // Alliance only
        if (!TakeFlightFromMaster(NPC_DUNGAR_LONGDRINK, TAXI_NODE_BOOTY_BAY))
        {
            ai->TellPlayer(GetMaster(), "Failed to take flight from Dungar Longdrink.");
            return;
        }
        ai->TellPlayer(GetMaster(), "Taking flight to Booty Bay!");
        break;
    case WorldBuffTravelStep::STEP_BOOTY_BAY:
        ApplyBuffToSelfAndRealPlayers(SPELL_SPIRIT_OF_ZANDALAR);
        ai->TellPlayer(GetMaster(), "Gained Spirit of Zandalar!");
        break;
    case WorldBuffTravelStep::STEP_BRAGOK:
        if (horde)
        {
            if (!TakeFlightFromMaster(NPC_BRAGOK, TAXI_NODE_CAMP_MOJACHE))
            {
                ai->TellPlayer(GetMaster(), "Failed to take flight from Bragok.");
                return;
            }
            ai->TellPlayer(GetMaster(), "Taking flight to Camp Mojache!");
        }
        else
        {
            if (!TakeFlightFromMaster(NPC_BRAGOK, TAXI_NODE_THALANAAR))
            {
                ai->TellPlayer(GetMaster(), "Failed to take flight from Bragok.");
                return;
            }
            ai->TellPlayer(GetMaster(), "Taking flight to Thalanaar!");
        }
        break;
    case WorldBuffTravelStep::STEP_THYSSIANA:
        if (horde)
        {
            // Horde only
            ai->TellPlayer(GetMaster(), "Arrived at Shyn in Camp Mojache. Heading to Dire Maul...");
        }
        else
        {
            if (!TakeFlightFromMaster(NPC_THYSSIANA, TAXI_NODE_FEATHERMOON))
            {
                ai->TellPlayer(GetMaster(), "Failed to take flight from Thyssiana.");
                return;
            }
            ai->TellPlayer(GetMaster(), "Taking flight to Feathermoon Stronghold!");
        }
        break;
    case WorldBuffTravelStep::STEP_FEATHERMOON:
        // Alliance only
        ai->TellPlayer(GetMaster(), "Arrived at Feathermoon Stronghold. Heading to Feralas mainland...");
        break;
    case WorldBuffTravelStep::STEP_FORGOTTEN_COAST:
        if (horde)
            ai->TellPlayer(GetMaster(), "Reached Dire Maul in Feralas. Heading to Dire Maul North...");
        else
            ai->TellPlayer(GetMaster(), "Reached Feralas mainland. Heading to Dire Maul...");
        break;
    case WorldBuffTravelStep::STEP_DM_TRAVEL:
        ApplyBuffToSelfAndRealPlayers(SPELL_DM_TRIBUTE_FENGUS);
        ApplyBuffToSelfAndRealPlayers(SPELL_DM_TRIBUTE_SLIPKIK);
        ApplyBuffToSelfAndRealPlayers(SPELL_DM_TRIBUTE_GORDOK);
        ai->TellPlayer(GetMaster(), "Entered Dire Maul North. Gained all Dire Maul Tribute buffs!");
        break;
    case WorldBuffTravelStep::STEP_FELWOOD:
    {
        const char* npcName = horde ? "Brakkar" : "Mishellena";
        ai->TellPlayer(GetMaster(), std::string("Arrived at ") + npcName + ". Searching for a Songflower...");
        break;
    }
    case WorldBuffTravelStep::STEP_SONGFLOWER:
        ApplyBuffToSelfAndRealPlayers(SPELL_SONGFLOWER);
        ai->TellPlayer(GetMaster(), "Gained Songflower Serenade!");
        break;
    default:
        break;
    }
}

void WorldBuffTravelApplyAction::AdvanceStep()
{
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    travelTarget->SetForced(false);
    travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);

    uint8 step = AI_VALUE(uint8, "world buff travel step");
    step++;

    // Horde: skip Alliance-only steps
    bool horde = IsHordeFaction(bot);
    while (horde && step < static_cast<uint8>(WorldBuffTravelStep::STEP_DONE) &&
        IsAllianceOnlyStep(static_cast<WorldBuffTravelStep>(step)))
    {
        step++;
    }

    WorldBuffTravelStep nextNeeded = GetFirstNeededStep(bot);
    if (static_cast<uint8>(nextNeeded) > step)
    {
        ai->TellPlayer(GetMaster(), "Already have upcoming buffs, skipping ahead...");
        step = static_cast<uint8>(nextNeeded);
    }

    context->GetValue<uint8>("world buff travel step")->Set(step);
}

bool WorldBuffTravelApplyAction::Execute(Event& event)
{
    uint8 rawStep = AI_VALUE(uint8, "world buff travel step");
    WorldBuffTravelStep step = static_cast<WorldBuffTravelStep>(rawStep);

    if (IsHordeFaction(bot) && IsAllianceOnlyStep(step))
    {
        AdvanceStep();
        return true;
    }

    if (step != WorldBuffTravelStep::STEP_STORMWIND &&
        step != WorldBuffTravelStep::STEP_GRYPHON_MASTER &&
        step != WorldBuffTravelStep::STEP_BOOTY_BAY &&
        step != WorldBuffTravelStep::STEP_BRAGOK &&
        step != WorldBuffTravelStep::STEP_THYSSIANA &&
        step != WorldBuffTravelStep::STEP_FEATHERMOON &&
        step != WorldBuffTravelStep::STEP_FORGOTTEN_COAST &&
        step != WorldBuffTravelStep::STEP_DM_TRAVEL &&
        step != WorldBuffTravelStep::STEP_FELWOOD &&
        step != WorldBuffTravelStep::STEP_SONGFLOWER)
        return false;

    if (TrySummonFarAwayMembers(step))
    {
        ai->TellPlayer(GetMaster(), "Waiting for summoned members to arrive before applying buffs...");
        return true;
    }

    ApplyBuffsForStep(step);
    AdvanceStep();
    return true;
}

bool WorldBuffTravelSetTargetAction::Execute(Event& event)
{
    uint8 rawStep = AI_VALUE(uint8, "world buff travel step");
    WorldBuffTravelStep step = static_cast<WorldBuffTravelStep>(rawStep);

    bool horde = IsHordeFaction(bot);

    const char* name = GetStepZoneName(step, bot);
    if (!name || !name[0])
    {
        // DM_PORTAL: regroup at the DM North
        if (step == WorldBuffTravelStep::STEP_DM_PORTAL)
        {
            uint32 destZone = GetDMPortalDestZone(bot);
            if (bot->GetZoneId() == destZone)
                return false;

            GameObjectData const* goData = sObjectMgr.GetGOData(GO_DM_NORTH_DOOR_GUID);
            if (!goData)
            {
                ai->TellPlayer(GetMaster(), "Cannot find Dire Maul North door spawn data");
                return false;
            }

            ai->TellPlayer(GetMaster(), "Regrouping at Dire Maul North for portal...");
            return MoveTo(goData->mapid, goData->posX, goData->posY, goData->posZ);
        }

        // PORTAL_HOME: regroup at the nearest Songflower
        if (step == WorldBuffTravelStep::STEP_PORTAL_HOME)
        {
            uint32 homeZone = GetHomeZone(bot);
            if (bot->GetZoneId() == homeZone)
                return false;

            const GameObjectData* goData = FindClosestSongflowerSpawn(bot);
            if (!goData)
            {
                ai->TellPlayer(GetMaster(), "Cannot find any Songflower spawn to regroup at");
                return false;
            }

            const char* portalName = GetHomePortalKeyword(bot);
            ai->TellPlayer(GetMaster(), std::string("Regrouping at Songflower waiting for Portal: ") + portalName + "...");
            return MoveTo(goData->mapid, goData->posX, goData->posY, goData->posZ);
        }

        return false;
    }

    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    if (travelTarget->IsForced() &&
        travelTarget->GetStatus() != TravelStatus::TRAVEL_STATUS_NONE &&
        travelTarget->GetStatus() != TravelStatus::TRAVEL_STATUS_EXPIRED &&
        travelTarget->GetStatus() != TravelStatus::TRAVEL_STATUS_COOLDOWN)
        return false;

    if (step == WorldBuffTravelStep::STEP_DM_TRAVEL)
    {
        GameObjectData const* goData = sObjectMgr.GetGOData(GO_DM_NORTH_DOOR_GUID);
        if (!goData)
        {
            ai->TellPlayer(GetMaster(), "Cannot find Dire Maul North door spawn data");
            return false;
        }

        ai->TellPlayer(GetMaster(), "Traveling to Dire Maul North for world buffs");
        return MoveTo(goData->mapid, goData->posX, goData->posY, goData->posZ);
    }

    if (step == WorldBuffTravelStep::STEP_SONGFLOWER)
    {
        const GameObjectData* goData = FindClosestSongflowerSpawn(bot);
        if (!goData)
        {
            ai->TellPlayer(GetMaster(), "Cannot find any Songflower spawn data");
            return false;
        }

        ai->TellPlayer(GetMaster(), "Traveling to nearest Songflower for world buffs");
        return MoveTo(goData->mapid, goData->posX, goData->posY, goData->posZ);
    }

    if (step == WorldBuffTravelStep::STEP_FORGOTTEN_COAST && horde)
    {
        const GameObjectData* best = nullptr;
        float bestDistSq = std::numeric_limits<float>::max();

        struct BonfireFinder
        {
            uint32 mapId;
            float x, y, z;
            const GameObjectData* best;
            float bestDistSq;

            bool operator()(GameObjectDataPair const& dataPair)
            {
                const GameObjectData& data = dataPair.second;
                if (data.id != GO_BONFIRE_FERALAS)
                    return false;

                if (data.mapid != mapId)
                    return false;

                float dx = data.posX - x;
                float dy = data.posY - y;
                float dz = data.posZ - z;
                float distSq = dx * dx + dy * dy + dz * dz;

                if (distSq < bestDistSq)
                {
                    bestDistSq = distSq;
                    best = &data;
                }
                return false;
            }
        };

        BonfireFinder finder{ bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                              bot->GetPositionZ(), nullptr, bestDistSq };
        sObjectMgr.DoGOData(finder);

        if (!finder.best)
        {
            ai->TellPlayer(GetMaster(), "Cannot find Bonfire spawn data in Feralas");
            return false;
        }

        ai->TellPlayer(GetMaster(), "Traveling to the Dire Maul in Feralas");
        return MoveTo(finder.best->mapid, finder.best->posX, finder.best->posY, finder.best->posZ);
    }

    if (step == WorldBuffTravelStep::STEP_FELWOOD)
    {
        uint32 zoneId = bot->GetZoneId();
        uint32 portalDestZone = GetDMPortalDestZone(bot);
        if (zoneId != portalDestZone && zoneId != ZONE_FELWOOD)
        {
            TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
            travelTarget->SetForced(false);
            travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);

            GameObjectData const* goData = sObjectMgr.GetGOData(GO_DM_NORTH_DOOR_GUID);
            if (!goData)
            {
                ai->TellPlayer(GetMaster(), "Cannot find Dire Maul North door");
                return false;
            }

            const char* portalName = horde ? "Orgrimmar" : "Darnassus";
            ai->TellPlayer(GetMaster(), std::string("Regrouping at Dire Maul North before portal to ") + portalName + "...");
            return MoveTo(goData->mapid, goData->posX, goData->posY, goData->posZ);
        }
    }

    PlayerTravelInfo info(bot);

    bool zones = !IsStepGoSearch(step, bot);
    bool npcs = IsStepGoSearch(step, bot);

    DestinationList dests = ChooseTravelTargetAction::FindDestination(
        info, name, zones, npcs, false, false, false, false);

    if (dests.empty())
    {
        ai->TellPlayer(GetMaster(), std::string("Cannot find travel destination for ") + name);
        return false;
    }

    TravelDestination* dest = dests.front();
    WorldPosition center(bot);
    std::list<uint8> chancesToGoFar = { 10, 50, 90 };
    WorldPosition* point = dest->GetNextPoint(center, chancesToGoFar);

    if (!point)
        return false;

    travelTarget->SetTarget(dest, point);
    travelTarget->SetForced(true);
    travelTarget->SetExpireIn(HOUR * IN_MILLISECONDS);
    travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_READY);

    RESET_AI_VALUE(GuidPosition, "rpg target");
    RESET_AI_VALUE(ObjectGuid, "attack target");
    RESET_AI_VALUE(bool, "travel target active");

    ai->TellPlayer(GetMaster(), std::string("Traveling to ") + name + " for world buffs");
    return true;
}

bool WorldBuffTravelDMBuffedAction::Execute(Event& event)
{
    ai->TellPlayer(GetMaster(), "Got all DM Tribute buffs! Leaving the instance...");
    context->GetValue<uint8>("world buff travel step")->Set(
        static_cast<uint8>(WorldBuffTravelStep::STEP_DM_EXIT));
    return true;
}

bool WorldBuffTravelDMExitedAction::Execute(Event& event)
{
    const char* portalName = IsHordeFaction(bot) ? "Orgrimmar" : "Darnassus";
    ai->TellPlayer(GetMaster(), std::string("Exited Dire Maul. Waiting for Portal: ") + portalName + "...");
    context->GetValue<uint8>("world buff travel step")->Set(
        static_cast<uint8>(WorldBuffTravelStep::STEP_DM_PORTAL));
    return true;
}

bool WorldBuffTravelDMCastPortalAction::isUseful()
{
    return bot->getClass() == CLASS_MAGE;
}

bool WorldBuffTravelDMCastPortalAction::Execute(Event& event)
{
    if (bot->IsNonMeleeSpellCasted(false, false, true))
        return true;

    const char* keyword = GetDMPortalKeyword(bot);
    uint32 portalSpell = GetDMPortalSpell(bot);

    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    if (FindNearbyPortalGO(ai, bot, gos, keyword))
    {
        ai->TellPlayer(GetMaster(), std::string("Portal to ") + keyword + " is already open!");
        return true;
    }

    if (bot->IsMounted())
    {
        ai->Unmount();
        return true;
    }

    if (ai->CastSpell(portalSpell, bot))
    {
        ai->TellPlayer(GetMaster(), std::string("Casting Portal: ") + keyword + " for the group!");
        return true;
    }

    ai->TellPlayer(GetMaster(), std::string("Cannot cast Portal: ") + keyword + " yet...");
    return false;
}

bool WorldBuffTravelDMTakePortalAction::Execute(Event& event)
{
    if (bot->IsNonMeleeSpellCasted(false, false, true))
        return true;

    if (bot->IsBeingTeleported())
        return true;

    const char* keyword = GetDMPortalKeyword(bot);

    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    GameObject* portalGO = FindNearbyPortalGO(ai, bot, gos, keyword);
    if (portalGO)
    {
        float dist = bot->GetDistance(portalGO);
        if (dist > INTERACTION_DISTANCE)
            return MoveTo(portalGO->GetMapId(), portalGO->GetPositionX(),
                portalGO->GetPositionY(), portalGO->GetPositionZ());

        std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
        *packet << portalGO->GetObjectGuid();
        bot->GetSession()->QueuePacket(std::move(packet));

        ai->TellPlayer(GetMaster(), std::string("Taking the portal to ") + keyword + "!");
        context->GetValue<uint8>("world buff travel step")->Set(
            static_cast<uint8>(WorldBuffTravelStep::STEP_FELWOOD));
        return true;
    }

    ai->TellPlayer(GetMaster(), std::string("Waiting for a mage to open Portal: ") + keyword + "...");
    return false;
}

bool WorldBuffTravelCastPortalAction::isUseful()
{
    return bot->getClass() == CLASS_MAGE;
}

bool WorldBuffTravelCastPortalAction::Execute(Event& event)
{
    if (bot->IsNonMeleeSpellCasted(false, false, true))
        return true;

    const char* keyword = GetHomePortalKeyword(bot);
    uint32 portalSpell = GetHomePortalSpell(bot);

    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    if (FindNearbyPortalGO(ai, bot, gos, keyword))
    {
        ai->TellPlayer(GetMaster(), std::string("Portal to ") + keyword + " is already open!");
        return true;
    }

    if (bot->IsMounted())
    {
        ai->Unmount();
        return true;
    }

    if (ai->CastSpell(portalSpell, bot))
    {
        ai->TellPlayer(GetMaster(), std::string("Casting Portal: ") + keyword + " for the group!");
        return true;
    }

    ai->TellPlayer(GetMaster(), std::string("Cannot cast Portal: ") + keyword + " yet...");
    return false;
}

bool WorldBuffTravelTakePortalAction::Execute(Event& event)
{
    if (bot->IsNonMeleeSpellCasted(false, false, true))
        return true;

    if (bot->IsBeingTeleported())
        return true;

    const char* keyword = GetHomePortalKeyword(bot);

    std::list<ObjectGuid> gos = AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    GameObject* portalGO = FindNearbyPortalGO(ai, bot, gos, keyword);
    if (portalGO)
    {
        float dist = bot->GetDistance(portalGO);
        if (dist > INTERACTION_DISTANCE)
            return MoveTo(portalGO->GetMapId(), portalGO->GetPositionX(),
                portalGO->GetPositionY(), portalGO->GetPositionZ());

        std::unique_ptr<WorldPacket> packet(new WorldPacket(CMSG_GAMEOBJ_USE));
        *packet << portalGO->GetObjectGuid();
        bot->GetSession()->QueuePacket(std::move(packet));

        ai->TellPlayer(GetMaster(), "Taking the portal home!");
        context->GetValue<uint8>("world buff travel step")->Set(
            static_cast<uint8>(WorldBuffTravelStep::STEP_DONE));
        return true;
    }

    ai->TellPlayer(GetMaster(), std::string("Waiting for a mage to open Portal: ") + keyword + "...");
    return false;
}

bool WorldBuffTravelFinishAction::Execute(Event& event)
{
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    travelTarget->SetForced(false);

    // Horde: Apply Warchief's Blessing last when getting back to Orgrimmar.
    if (IsHordeFaction(bot) && !bot->HasAura(SPELL_WARCHIEFS_BLESSING))
    {
        PlayerbotAI::AddAura(bot, SPELL_WARCHIEFS_BLESSING);
        ai->TellPlayer(GetMaster(), "Gained Warchief's Blessing!");
    }

    ai->TellPlayer(GetMaster(), "World buff collection complete!");
    ai->ChangeStrategy("-wbuff travel", BotState::BOT_STATE_NON_COMBAT);
    return true;
}