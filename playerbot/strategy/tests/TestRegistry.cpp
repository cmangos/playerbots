#include "playerbot/playerbot.h"
#include "TestRegistry.h"
#include "playerbot/ChatHelper.h"
#include "Globals/ObjectMgr.h"
#include <regex>
#include <sstream>
#include <mutex>
#include <set>
#include "playerbot/TravelMgr.h"
#include <playerbot/TravelNode.h>
#include "Entities/Transports.h"
#include "Server/DBCStores.h"

static std::map<std::string, std::vector<std::string>> sTestRegistry;
static bool sTestsRegistered = false;
static std::recursive_mutex sTestsRegisterMutex;

static std::map<std::string, GuidPosition> sNamedTestLocations;
static std::mutex sNamedLocationsMutex;

struct TeleLoc
{
    std::string name;
    GuidPosition pos;
};

static std::vector<TeleLoc> sTeleLocations;

static void InitTestLocations()
{
    sNamedTestLocations["ironforge_outside"] = GuidPosition(ObjectGuid(), WorldPosition(0, -5150.0f, -856.0f, 508.4f));
    sNamedTestLocations["zg_boss1_room"] = GuidPosition(ObjectGuid(), WorldPosition(309, -12276.0f, -1399.0f, 130.9f));
}

static void InitTeleLocations()
{
    for (auto const& [id, tele] : sObjectMgr.GetGameTeleMap())
    {
        GuidPosition pos(ObjectGuid(), WorldPosition(tele.mapId, tele.position_x, tele.position_y, tele.position_z, tele.orientation));
        sTeleLocations.push_back({ tele.name, pos });
    }
}

static void EnsureLocationsInit()
{
    static std::mutex initMutex;
    std::lock_guard<std::mutex> guard(initMutex);

    static bool initialized = false;
    if (!initialized)
    {
        InitTestLocations();
        InitTeleLocations();
        initialized = true;
    }
}

namespace
{
    using ScenarioParams = std::map<std::string, std::string>;

    std::string NormalizeLocationKey(std::string value)
    {
        const size_t begin = value.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos)
            return "";

        const size_t end = value.find_last_not_of(" \t\r\n");
        value = value.substr(begin, end - begin + 1);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        return value;
    }

    uint32 ParseUintOrDefault(const std::string& value, uint32 fallback)
    {
        if (value.empty())
            return fallback;

        for (char c : value)
            if (!std::isdigit(static_cast<unsigned char>(c)))
                return fallback;

        return static_cast<uint32>(std::stoul(value));
    }

    uint32 ParseMGroupSize(const std::string& line)
    {
        static const uint32 defaultGroupSize = 5;
        std::istringstream iss(line);
        std::string token;

        while (iss >> token)
        {
            if (token.find("size=") != 0)
                continue;

            return ParseUintOrDefault(token.substr(std::string("size=").length()), defaultGroupSize);
        }

        return defaultGroupSize;
    }
}

void TestRegistry::GenerateMovementTestsImpl(int maxTests, float minDist, float maxDist)
{
    if (sTeleLocations.size() < 2)
        return;

    std::string gmInvisible = "gm visible off";
    std::string gmVisible = "cleanup gm visible on";
    std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    std::string needAboveGRound = "monitor underground => fail \"Bot is underground at <current position>\"";
    std::string needCanReachNode = "monitor can not reach nodes => fail \"Bot cannot reach travel network at <current position>\"";

    int count = 0;
    for (size_t i = 0; i < sTeleLocations.size() && count < maxTests; ++i)
    {
        for (size_t j = 0; j < sTeleLocations.size() && count < maxTests; ++j)
        {
            if (i == j)
                continue;

            WorldPosition startPos = sTeleLocations[i].pos;
            WorldPosition endPos = sTeleLocations[j].pos;

            if (!startPos || !endPos)
                continue;

            if (startPos.isBg() || endPos.isBg())
                continue;

            if (startPos.isDungeon() && startPos.getMapEntry()->IsRaid())
                continue;

            if (endPos.isDungeon() && endPos.getMapEntry()->IsRaid())
                continue;

            float dist = startPos.distance(endPos);

            if (dist < minDist || dist > maxDist)
                continue;

            std::string prefix;
            int timeoutSecs = dist + 600;
            if (dist < 500)
            {
                prefix = "short";
            }
            else if (dist < 2000)
            {
                prefix = "medium";
            }
            else
            {
                prefix = "far";
            }

            std::string startName = sTeleLocations[i].name;
            std::transform(startName.begin(), startName.end(), startName.begin(), ::tolower);
            std::replace(startName.begin(), startName.end(), '\'', '_');
            std::string endName = sTeleLocations[j].name;
            std::transform(endName.begin(), endName.end(), endName.begin(), ::tolower);
            std::replace(endName.begin(), endName.end(), '\'', '_');


            std::ostringstream testName;
            testName << "movement_" << prefix << "_" << startName << "_" << endName;

            std::ostringstream timeout;
            timeout << "monitor time > " << timeoutSecs << " => fail \"Timeout: bot did not reach destination after <time elapsed> (traveled <distance traveled> / wanted <distance wanted>)\"";

            std::ostringstream reachCheck;
            reachCheck << "monitor distance to " << endName << " < 100 => pass \"Bot arrived at " << endName << " after <time elapsed>\"";

            std::vector<std::string> script = {
                gmInvisible,
                needAlive,
                needAboveGRound,
                needCanReachNode,
                timeout.str(),
                reachCheck.str(),
                "teleport " + startName,
                "set destination " + endName,
                "observe",
                gmVisible
            };

            RegisterTest(testName.str(), script);
            count++;

            if ((j % 10) == 0 && i < sTeleLocations.size())
                i++;
        }
    }
}

namespace
{
    std::string SanitizeLocationName(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        std::replace(value.begin(), value.end(), ' ', '_');
        std::replace(value.begin(), value.end(), '\'', '_');
        return value;
    }

    // Register a named location. If the name is taken by a different position, add _2, _3, ...
    std::string RegisterUniqueDockLocation(std::string name, const WorldPosition& pos)
    {
        name = SanitizeLocationName(name);

        GuidPosition existing;
        std::string key = name;
        uint32 suffix = 1;

        while (TestRegistry::LookupNamedLocation(key, existing))
        {
            WorldPosition oldPos(existing);
            sLog.outString("[TRANSPORTGEN] loc '%s' exists at (%.0f,%.0f,%.0f m%u), new (%.0f,%.0f,%.0f m%u)", key.c_str(), oldPos.getX(), oldPos.getY(), oldPos.getZ(), oldPos.getMapId(), pos.getX(), pos.getY(), pos.getZ(), pos.getMapId());
            // 3D distance: elevator top/bottom docks can sit right on top of each other in 2D
            // (fDist is horizontal-only) but are far apart vertically.
            if (oldPos.getMapId() == pos.getMapId() && oldPos.distance(pos) < 50.0f)
                return key;

            key = name + "_" + std::to_string(++suffix);
        }

        TestRegistry::RegisterNamedLocation(key, GuidPosition(ObjectGuid(), pos));
        sLog.outString("[TRANSPORTGEN] loc registered: '%s' at (%.0f,%.0f,%.0f m%u)", key.c_str(), pos.getX(), pos.getY(), pos.getZ(), pos.getMapId());
        return key;
    }

    // Compare with all separators stripped: tele/node names arrive as "grom_gol_base_camp",
    // "gromgolbasecamp" or "Grom'gol Base Campdock" depending on the source.
    std::string CompactDockName(const std::string& name)
    {
        std::string compact;
        for (char c : name)
        {
            if (c == ' ' || c == '\'' || c == '_')
                continue;
            compact += (char)::tolower(c);
        }

        return compact;
    }

    // Docks of these locations are horde territory (guards kill an alliance bot).
    bool IsHordeDockName(const std::string& name)
    {
        std::string compact = CompactDockName(name);
        return compact.find("orgrimmar") != std::string::npos || compact.find("undercity") != std::string::npos ||
               compact.find("gromgol") != std::string::npos || compact.find("thunderbluff") != std::string::npos ||
               compact.find("warsonghold") != std::string::npos || compact.find("vengeancelanding") != std::string::npos ||
               compact.find("tirisfal") != std::string::npos || compact.find("durotar") != std::string::npos;
    }

    bool IsAllianceDockName(const std::string& name)
    {
        std::string compact = CompactDockName(name);
        return compact.find("stormwind") != std::string::npos || compact.find("ironforge") != std::string::npos ||
               compact.find("menethil") != std::string::npos || compact.find("auberdine") != std::string::npos ||
               compact.find("theramore") != std::string::npos || compact.find("darnassus") != std::string::npos ||
               compact.find("valiance") != std::string::npos || compact.find("ruttheran") != std::string::npos ||
               compact.find("exodar") != std::string::npos;
    }
}

void TestRegistry::GenerateTransportTestsImpl(int maxTests)
{
    // Transport test generator, driven by the SAVED travel node store (ai_playerbot_travelnode*
    // tables in the world DB - the same snapshot TravelNodeMap::loadNodeStore loads at startup).
    //
    // Structure (built by TravelNodeMap::generateTransportNodes):
    //  - every boat/zeppelin/elevator/tram has "stop" nodes linked over transport-type paths
    //    (type=3, object = transport GO entry) - one row per ride leg;
    //  - every stop node is paired with a ground "dock" node over a transport link with
    //    object=0; its stored path is exactly two points: {stop pos, dock world pos};
    //  - dock nodes connect to the world through normal walk links (type=1).
    //
    // The runtime node map is NOT used: bot update threads crop dock nodes over time
    // (manageNodes/cropUselessNode), the DB keeps them. Test shape (world -> transport -> world):
    // teleport the bot a few points into the start dock's walk path towards the world, send it
    // to the mirrored point at the other dock; the travel AI waits, boards, rides, disembarks.
    std::string gmInvisible = "gm visible off";
    std::string gmVisible = "cleanup gm visible on";
    std::string needAlive = "monitor bot dead => abort \"Bot died test interupted\"";
    std::string needCanReachNode = "monitor can not reach nodes => fail \"Bot cannot reach travel network at <current position>\"";

    struct DbNode { std::string name; WorldPosition pos; };
    struct DbLink { uint32 to = 0; uint64 object = 0; float distance = 0; float extraCost = 0; std::vector<WorldPosition> path; };

    std::map<uint32, DbNode> dbNodes;
    std::multimap<uint32, DbLink> linksFrom;
    std::map<uint32, std::pair<uint32, std::vector<WorldPosition>>> dockWalkPath; // dock id -> (first walk-target node, points)
    std::map<uint32, std::set<uint32>> dockWalkTargets; // dock id -> all type=1 targets (docks, stops and world nodes)

    auto result = WorldDatabase.PQuery("SELECT id, name, map_id, x, y, z FROM ai_playerbot_travelnode");
    if (!result)
    {
        sLog.outString("[TRANSPORTGEN] travel node store missing in world DB - generation skipped");
        return;
    }

    do
    {
        Field* f = result->Fetch();
        DbNode node;
        node.name = f[1].GetCppString();
        node.pos = WorldPosition(f[2].GetUInt32(), f[3].GetFloat(), f[4].GetFloat(), f[5].GetFloat());
        dbNodes[f[0].GetUInt32()] = node;
    } while (result->NextRow());

    result = WorldDatabase.PQuery("SELECT node_id, to_node_id, object, distance, extra_cost FROM ai_playerbot_travelnode_link WHERE type=3");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            DbLink link;
            link.to = f[1].GetUInt32();
            link.object = f[2].GetUInt64();
            link.distance = f[3].GetFloat();
            link.extraCost = f[4].GetFloat();
            linksFrom.insert(std::make_pair(f[0].GetUInt32(), link));
        } while (result->NextRow());
    }

    // Stored path points for all transport links. Ride legs carry the ride waypoints; dock
    // links carry {stop pos, dock world pos}.
    result = WorldDatabase.PQuery(
        "SELECT p.node_id, p.to_node_id, p.nr, p.map_id, p.x, p.y, p.z "
        "FROM ai_playerbot_travelnode_path p JOIN ai_playerbot_travelnode_link l "
        "ON l.node_id=p.node_id AND l.to_node_id=p.to_node_id "
        "WHERE l.type=3 ORDER BY p.node_id, p.to_node_id, p.nr");
    if (result)
    {
        do
        {
            Field* f = result->Fetch();
            uint32 from = f[0].GetUInt32();
            uint32 to = f[1].GetUInt32();
            WorldPosition point(f[3].GetUInt32(), f[4].GetFloat(), f[5].GetFloat(), f[6].GetFloat());

            auto range = linksFrom.equal_range(from);
            for (auto it = range.first; it != range.second; ++it)
            {
                if (it->second.to == to)
                {
                    it->second.path.push_back(point);
                    break;
                }
            }
        } while (result->NextRow());
    }

    // Dock nodes: endpoints of the object=0 transport links. Their walk links (type=1) lead
    // into the world - keep the shortest one's path to walk a few points in from the dock.
    std::ostringstream dockIdList;
    std::set<uint32> dockIds;
    for (auto const& [fromId, link] : linksFrom)
    {
        if (link.object != 0)
            continue;

        for (uint32 id : { fromId, link.to })
        {
            if (dockIds.insert(id).second)
            {
                if (!dockIdList.str().empty())
                    dockIdList << ",";
                dockIdList << id;
            }
        }
    }

    if (!dockIdList.str().empty())
    {
        result = WorldDatabase.PQuery((std::string(
            "SELECT l.node_id, l.distance, l.to_node_id, p.nr, p.map_id, p.x, p.y, p.z "
            "FROM ai_playerbot_travelnode_link l JOIN ai_playerbot_travelnode_path p "
            "ON p.node_id=l.node_id AND p.to_node_id=l.to_node_id "
            "WHERE l.type=1 AND l.node_id IN (") + dockIdList.str() + ") ORDER BY l.node_id, l.distance DESC, p.nr").c_str());

        if (result)
        {
            uint32 lastDock = 0;
            do
            {
                Field* f = result->Fetch();
                uint32 dockId = f[0].GetUInt32();
                uint32 walkTarget = f[2].GetUInt32();
                WorldPosition point(f[4].GetUInt32(), f[5].GetFloat(), f[6].GetFloat(), f[7].GetFloat());

                dockWalkTargets[dockId].insert(walkTarget); // for the cul-de-sac check below

                if (dockId != lastDock) // rows are ordered by distance: first link = longest walk out
                {
                    lastDock = dockId;
                    dockWalkPath[dockId].first = walkTarget;
                    dockWalkPath[dockId].second.push_back(point);
                }
                else
                    dockWalkPath[dockId].second.push_back(point);
            } while (result->NextRow());
        }
    }

    // A point a few steps into the dock's walk path towards the world: safer to teleport to
    // (and to end a run at) than the raw node coordinate. Prefer the point FARTHEST out (still
    // <= 45 yd): the parked transport occupies the dock area, and a walk-in point inside that
    // footprint leaves the bot on top of the (off-mesh) platform - pathfinding from there is
    // persistently blocked. Never fall back to the dock node itself (it is path point 0).
    //
    // Every returned point is snapped to the nearest ground poly (small extents): a bot that
    // settles even ~0.8 yd above the only nearby poly (RFK bottom dock: node z -45.32, ground
    // z -44.53) cannot seed a path (PATHDIAG: path=1 to every node) and freezes in place.
    auto snapToMesh = [](WorldPosition pos) -> WorldPosition
    {
        WorldPosition fallback = pos;
        pos.loadMapAndVMap(0);
        if (pos.ClosestCorrectPoint(3.0f, 3.0f, 0))
            return pos;
        return fallback;
    };

    auto walkInPoint = [&](uint32 dockId) -> WorldPosition
    {
        auto walk = dockWalkPath.find(dockId);
        auto nodeIt = dbNodes.find(dockId);
        if (walk == dockWalkPath.end() || nodeIt == dbNodes.end())
            return snapToMesh(nodeIt != dbNodes.end() ? nodeIt->second.pos : WorldPosition());

        WorldPosition const& dockPos = nodeIt->second.pos;
        WorldPosition best;
        float bestD = -1.0f;
        for (WorldPosition const& point : walk->second.second)
        {
            float d = dockPos.fDist(point);
            if (d >= 10.0f && d <= 45.0f && d > bestD)
            {
                bestD = d;
                best = point;
            }
        }

        if (bestD > 0.0f)
            return snapToMesh(best);

        // No point in the band: fall back to the dock node itself. (The farthest stored path
        // point proved worse: the RFK bottom dock's only walk points sit in a dead pocket under
        // the parked platform footprint - pathfinding from there never starts.)
        return snapToMesh(dockPos);
    };

    auto nearestDockLink = [&](uint32 nodeId) -> uint32
    {
        uint32 dockId = 0;
        float bestDist = -1.0f;
        auto range = linksFrom.equal_range(nodeId);
        for (auto it = range.first; it != range.second; ++it)
        {
            if (it->second.object != 0 || !dbNodes.count(it->second.to))
                continue;

            if (bestDist < 0.0f || it->second.distance < bestDist)
            {
                bestDist = it->second.distance;
                dockId = it->second.to;
            }
        }

        return dockId;
    };

    int count = 0;
    int docksMissed = 0, degenerate = 0, skippedLog = 0;

    for (auto it = linksFrom.begin(); it != linksFrom.end(); ++it)
    {
        if (count >= maxTests)
            break;

        uint32 fromId = it->first;
        DbLink const& ride = it->second;
        if (ride.object == 0) // dock link, not a ride leg
            continue;

        uint32 entry = (uint32)ride.object;

        GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(entry);
        if (!data)
        {
            docksMissed++;
            if (skippedLog < 300)
            {
                skippedLog++;
                sLog.outString("[TRANSPORTGEN] skip no-goinfo: node '%s' entry=%u", dbNodes.count(fromId) ? dbNodes[fromId].name.c_str() : "?", entry);
            }
            continue;
        }

        uint32 startDockId = nearestDockLink(fromId);
        uint32 endDockId = nearestDockLink(ride.to);
        if (!startDockId || !endDockId || startDockId == endDockId)
        {
            docksMissed++;
            if (skippedLog < 300)
            {
                skippedLog++;
                sLog.outString("[TRANSPORTGEN] skip dock: '%s' -> '%s' entry=%u startDock=%d endDock=%d",
                    dbNodes.count(fromId) ? dbNodes[fromId].name.c_str() : "?",
                    dbNodes.count(ride.to) ? dbNodes[ride.to].name.c_str() : "?", entry, startDockId, endDockId);
            }
            continue;
        }

        // Cul-de-sac check: a START dock whose only walk links lead to other docks/stops (its
        // sibling dock) can never be walked out of - live pathfinding confirms the stored link is
        // synthetic (RFK bottom docks: mesh island, path=1 to every node; Freewind dock_2 pair:
        // same "traveled 0 m" failure). A start dock must reach a real world node on foot.
        // Arrival-only legs (world -> cul-de-sac dock) stay: the bot never needs to walk out.
        bool startWalkOut = false;
        for (uint32 target : dockWalkTargets[startDockId])
        {
            if (!dockIds.count(target)) // dockIds = stops + docks (endpoints of object=0 links)
            {
                startWalkOut = true;
                break;
            }
        }
        if (!startWalkOut)
        {
            docksMissed++;
            if (skippedLog < 300)
            {
                skippedLog++;
                sLog.outString("[TRANSPORTGEN] skip cul-de-sac start dock: '%s' -> '%s' entry=%u startDock=%d",
                    dbNodes.count(fromId) ? dbNodes[fromId].name.c_str() : "?",
                    dbNodes.count(ride.to) ? dbNodes[ride.to].name.c_str() : "?", entry, startDockId);
            }
            continue;
        }

        WorldPosition startDockPos = walkInPoint(startDockId);
        WorldPosition endDockPos = walkInPoint(endDockId);

        if (startDockPos.isBg() || endDockPos.isBg())
        {
            docksMissed++;
            continue;
        }

        // Same rule as the movement generator: no generated rides into raid maps (ICC lifts etc.).
        if ((startDockPos.isDungeon() && startDockPos.getMapEntry()->IsRaid()) ||
            (endDockPos.isDungeon() && endDockPos.getMapEntry()->IsRaid()))
        {
            docksMissed++;
            continue;
        }

        // A leg shorter than the pass threshold would pass instantly without any ride.
        // Vertical transport (elevators) can travel far in Z while the docks sit on top of
        // each other horizontally, so only skip when the points are close in BOTH 2D and Z.
        if (startDockPos.sqDistance2d(endDockPos) < 100.0f * 100.0f && std::abs(startDockPos.getZ() - endDockPos.getZ()) < 30.0f)
        {
            degenerate++;
            if (skippedLog < 300)
            {
                skippedLog++;
                sLog.outString("[TRANSPORTGEN] skip degenerate: '%s' -> '%s' entry=%u dist2d=%.0f dz=%.0f",
                    dbNodes[startDockId].name.c_str(), dbNodes[endDockId].name.c_str(), entry,
                    sqrt(startDockPos.sqDistance2d(endDockPos)), std::abs(startDockPos.getZ() - endDockPos.getZ()));
            }
            continue;
        }

        // Ride time: the stored extra cost is seconds for elevators but junk (0.003) for boats
        // and 0 for some lines, so fall back to ride path length / transport move speed.
        // Timeout = ride + one full transport cycle + slack, so a just-missed transport is
        // waited out, not failed.
        float rideSecs = ride.extraCost;
        if (rideSecs < 5.0f && ride.path.size() > 1)
        {
            float rideDist = 0.0f;
            for (size_t i = 1; i < ride.path.size(); ++i)
                rideDist += ride.path[i - 1].distance(ride.path[i]);

            float moveSpeed = (float)data->moTransport.moveSpeed;
            if (moveSpeed > 0.0f)
                rideSecs = rideDist / moveSpeed;
        }

        uint32 period = 0;
        if (TransportTemplate const* tmpl = sTransportMgr.GetTransportTemplate(entry))
            period = tmpl->pathTime / 1000; // pathTime is milliseconds
        else if (TransportAnimation const* anim = sTransportMgr.GetTransportAnimInfo(entry))
            period = anim->TotalTime / 1000;

        uint32 timeoutSecs = (uint32)(rideSecs + period + 300.0f);

        // A missed transport is waited out, one full cycle at minimum.
        if (timeoutSecs < 900)
            timeoutSecs = 900;

        // Name the points after the nearest game tele; fall back to the transport name.
        std::string startName;
        std::string endName;
        float bestStart = 300.0f;
        float bestEnd = 300.0f;
            for (auto const& [id, tele] : sObjectMgr.GetGameTeleMap())
            {
                if (tele.mapId != startDockPos.getMapId() && tele.mapId != endDockPos.getMapId())
                    continue;

                float dStart = startDockPos.getMapId() == tele.mapId ? startDockPos.fDist(WorldPosition(tele.mapId, tele.position_x, tele.position_y, tele.position_z)) : -1.0f;
                float dEnd = endDockPos.getMapId() == tele.mapId ? endDockPos.fDist(WorldPosition(tele.mapId, tele.position_x, tele.position_y, tele.position_z)) : -1.0f;

                if (dStart >= 0.0f && dStart < bestStart)
                {
                    bestStart = dStart;
                    startName = SanitizeLocationName(tele.name);
                }

                if (dEnd >= 0.0f && dEnd < bestEnd)
                {
                    bestEnd = dEnd;
                    endName = SanitizeLocationName(tele.name);
                }
            }

            std::string transportName = SanitizeLocationName(data->name);
            if (startName.empty())
                startName = transportName;

            if (endName.empty())
                endName = transportName + "_" + std::to_string(entry);
            // Always under a _dock key: a bare tele name would silently re-point every existing
            // teleport/distance-to use of that name (e.g. 'teleport menethil') at the pier.
            startName = RegisterUniqueDockLocation(startName + "_dock", startDockPos);
            endName = RegisterUniqueDockLocation(endName + "_dock", endDockPos);

            std::string testName = "movement_transport_" + transportName + "_" + startName + "_" + endName;

            uint32 dedupe = 2;
            while (HasTest(testName))
                testName = "movement_transport_" + transportName + "_" + startName + "_" + endName + "_" + std::to_string(dedupe++);

            std::ostringstream reachCheck;
            reachCheck << "monitor distance to " << endName << " < 100 => pass \"Bot arrived at " << endName << " after <time elapsed>\"";

            std::ostringstream timeout;
            timeout << "monitor time > " << timeoutSecs << " => fail \"Timeout: bot did not reach the other side after <time elapsed> (traveled <distance traveled> / wanted <distance wanted>)\"";

            std::string factionReq;
            if (IsHordeDockName(startName) || IsHordeDockName(endName))
                factionReq = "require bot is faction=horde";
            else if (IsAllianceDockName(startName) || IsAllianceDockName(endName))
                factionReq = "require bot is faction=alliance";

            std::vector<std::string> script = {
                gmInvisible,
                needAlive,
                needCanReachNode,
                timeout.str(),
                reachCheck.str()
            };

            if (!factionReq.empty())
                script.push_back(factionReq);

            script.push_back("teleport " + startName);
            script.push_back("set destination " + endName);
            script.push_back("observe");
            script.push_back(gmVisible);

            RegisterTest(testName, script);

            if (skippedLog < 300)
            {
                skippedLog++;
                sLog.outString("[TRANSPORTGEN] test: %s start=(%.0f,%.0f,%.0f) end=(%.0f,%.0f,%.0f) ride=%.0fs timeout=%u", testName.c_str(), startDockPos.getX(), startDockPos.getY(), startDockPos.getZ(), endDockPos.getX(), endDockPos.getY(), endDockPos.getZ(), rideSecs, timeoutSecs);
            }

            count++;
    }

    sLog.outString("[TRANSPORTGEN] nodes=%zu rideLegs=%d tests=%d docksMissed=%d degenerate=%d", dbNodes.size(), (int)linksFrom.size(), count, docksMissed, degenerate);
}

using namespace ai;

std::string TestRegistry::GetBotCreationRequirement(const std::string& testName)
{
    std::vector<std::string> script = GetTestScript(testName);

    for (const std::string& line : script)
    {
        if (line.find("require bot is") != 0)
            continue;

        std::string params = line.substr(std::string("require bot is ").length());
        return params;
    }

    return "";
}

uint32 TestRegistry::ExpectedBotSpawnCount(const std::string& testName)
{
    std::vector<std::string> script = GetTestScript(testName);

    uint32 expectedBots = 1;

    for (const std::string& line : script)
    {
        if (line.find("spawn") == 0)
        {
            expectedBots++;
            continue;
        }

        if (line.find("mgroup") == 0)
        {
            expectedBots = std::max(expectedBots, ParseMGroupSize(line));
        }
    }

    return expectedBots;
}

void TestRegistry::RegisterTest(const std::string& name, const std::vector<std::string>& script)
{
    sTestRegistry[name] = script;
}

void TestRegistry::RegisterNamedLocation(const std::string& name, const GuidPosition& pos)
{
    EnsureLocationsInit();

    std::string key = NormalizeLocationKey(name);

    std::lock_guard<std::mutex> guard(sNamedLocationsMutex);
    sNamedTestLocations[key] = pos;
}

bool TestRegistry::HasTest(const std::string& name)
{
    EnsureTestsRegistered();
    return sTestRegistry.find(name) != sTestRegistry.end();
}

std::vector<std::string> TestRegistry::GetTestScript(const std::string& name)
{
    EnsureTestsRegistered();
    auto it = sTestRegistry.find(name);
    if (it != sTestRegistry.end())
        return it->second;
    return {};
}

std::vector<std::string> TestRegistry::GetAvailableTests()
{
    EnsureTestsRegistered();
    std::vector<std::string> tests;
    for (const auto& pair : sTestRegistry)
        tests.push_back(pair.first);
    return tests;
}

void TestRegistry::GenerateMovementTests(int maxTests, float minDist, float maxDist)
{
    EnsureLocationsInit();
    GenerateMovementTestsImpl(maxTests, minDist, maxDist);
}

void TestRegistry::GenerateTransportTests(int maxTests)
{
    GenerateTransportTestsImpl(maxTests);
}

bool TestRegistry::LookupNamedLocation(const std::string& name, GuidPosition& out)
{
    EnsureLocationsInit();

    std::string lowerName = NormalizeLocationKey(name);

    std::lock_guard<std::mutex> guard(sNamedLocationsMutex);

    auto it = sNamedTestLocations.find(lowerName);
    if (it != sNamedTestLocations.end())
    {
        out = it->second;
        return true;
    }

    for (const auto& entry : sNamedTestLocations)
    {
        if (NormalizeLocationKey(entry.first) != lowerName)
            continue;

        out = entry.second;
        return true;
    }

    return false;
}

void TestRegistry::StartTest(PlayerbotAI* ai, const std::string& testName)
{
    std::string strategyName = "test::" + testName;
    ai->ChangeStrategy("+" + strategyName, BotState::BOT_STATE_NON_COMBAT);
    ai->ChangeStrategy("+" + strategyName, BotState::BOT_STATE_COMBAT);
    ai->ChangeStrategy("+" + strategyName, BotState::BOT_STATE_DEAD);
    ai->GetAiObjectContext()->GetValue<bool>("manual bool", "is running test")->Set(true);
}

bool TestRegistry::ParseLocation(const std::string& str, GuidPosition& out)
{
    std::string normalized = NormalizeLocationKey(str);
    if (normalized.empty())
        return false;

    if (LookupNamedLocation(normalized, out))
        return true;

    out = GuidPosition(GuidPosition::CreationMask::UNKNOWN, normalized);

    if (out)
        return true;

    return false;
}

void TestRegistry::EnsureTestsRegistered()
{
    // Generated on demand, but lazily: TestAction ctors (per bot) and RA commands can hit this
    // concurrently while sTestRegistry is written - guard the whole init.
    std::lock_guard<std::recursive_mutex> guard(sTestsRegisterMutex);

    if (sTestsRegistered)
        return;

    sTestsRegistered = true;

    RegisterMoveTests();
    RegisterSpawnTests();
    RegisterRandomizeTests();
    RegisterInstanceTests();
    RegisterBankTests();
    RegisterQuestDkStartTests();
    RegisterQuestSuiteTests();
    RegisterTeleportTests();
}

void TestRegistry::EnsureLocationsInit()
{
    ::EnsureLocationsInit();
}