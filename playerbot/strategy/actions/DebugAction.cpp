#include "playerbot/playerbot.h"
#include "DebugAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include <playerbot/TravelNode.h>
#include "ChooseTravelTargetAction.h"
#include "playerbot/strategy/values/SharedValueContext.h"
#include "playerbot/strategy/values/FreeMoveValues.h"
#include "playerbot/strategy/actions/RpgSubActions.h"
#include "playerbot/LootObjectStack.h"
#include "GameEvents/GameEventMgr.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/PlayerbotHelpMgr.h"
#include "Entities/Transports.h"
#include "MotionGenerators/PathFinder.h"
#include "playerbot/PlayerbotLLMInterface.h"

#ifdef MANGOSBOT_TWO
#include "Vmap/VMapFactory.h"
#else
#include "vmap/VMapFactory.h"
#endif

#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

#include <iomanip>
#include "SayAction.h"
#ifdef GenerateBotTests
#include "playerbot/strategy/tests/TestRegistry.h"
#endif
#include "MotionGenerators/MoveMap.h"

using namespace ai;
using namespace MaNGOS;

bool DebugAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    bool isMod = event.getSource() == ".bot" || (event.getOwner() && event.getOwner()->GetSession() && event.getOwner()->GetSession()->GetSecurity() >= SEC_MODERATOR);

    if (!requester)
    {
        requester = bot;
    }

    std::string text = event.getParam();
    
    if (text == "help" || text.find("help ") == 0)
        return HandleDebugHelp(event, requester, text, isMod);
    else if (text == "avoid scan" && isMod)
        return HandleAvoidScan(event, requester, text);
    else if (text.find("avoid add") == 0 && isMod)
        return HandleAvoidAdd(event, requester, text);
    else if (text.find("mount") == 0)
        return HandleMount(event, requester, text);
    else if (text.find("unmount") == 0)
        return HandleUnmount(event, requester, text);
    else if (text.find("area") == 0 && isMod)
        return HandleArea(event, requester, text);
    else if (text.find("llm ") == 0 && isMod)
        return HandleLLM(event, requester, text);
    else if (text.find("chatreplydo ") == 0 && isMod)
        return HandleChatReplyDo(event, requester, text);
    else if (text.find("monstertalk ") == 0 && isMod)
        return HandleMonsterTalk(event, requester, text);
    else if (text == "gy" && isMod)
        return HandleGY(event, requester, text);
    else if (text == "grid" && isMod)
        return HandleGrid(event, requester, text);
    else if (text.find("test") == 0 && isMod)
        return HandleTest(event, requester, text);
    else if (text.find("values") == 0)
        return HandleValues(event, requester, text);
    else if (text == "gps" && isMod)
        return HandleGPS(event, requester, text);
    else if (text.find("setvalueuin32 ") == 0)
        return HandleSetValueUInt32(event, requester, text);
    else if (text.find("do ") == 0)
        return HandleDo(event, requester, text);
    else if (text.find("trade ") == 0)
        return HandleTrade(event, requester, text);
    else if (text.find("trade") == 0)
        return HandleTrade(event, requester, text);
    else if (text.find("mail") == 0)
        return HandleMail(event, requester, text);
    else if (text.find("poi ") == 0)
        return HandlePOI(event, requester, text);
    else if (text.find("motion") == 0 && isMod)
        return HandleMotion(event, requester, text);
    else if (text.find("transport") == 0 && isMod)
        return HandleTransport(event, requester, text);
    else if (text.find("pointontrans") == 0 && isMod)
        return HandlePointOnTrans(event, requester, text);
    else if (text.find("dotrans") == 0 && isMod)
        return HandleDoTransport(event, requester, text);
    else if (text.find("ontrans") == 0 && isMod)
        return HandleOnTrans(event, requester, text);
    else if (text.find("offtrans") == 0 && isMod)
        return HandleOffTrans(event, requester, text);
    else if (text.find("pathable") == 0 && isMod)
        return HandlePathable(event, requester, text);
    else if (text.find("randomspot") == 0 && isMod)
        return HandleRandomSpot(event, requester, text);
    else if (text.find("printmap") == 0 && isMod)
        return HandlePrintMap(event, requester, text);
    else if (text.find("corpse") == 0)
        return HandleCorpse(event, requester, text);
    else if (text.find("logouttime") == 0)
        return HandleLogoutTime(event, requester, text);
    else if (text.find("level") == 0)
        return HandleLevel(event, requester, text);
    else if (text.find("quest") == 0)
        return HandleQuest(event, requester, text);
    else if (text.find("position") == 0)
        return HandlePosition(event, requester, text);
    else if (text.find("npc") == 0)
        return HandleNPC(event, requester, text);
    else if (text.find("go ") == 0)
        return HandleGO(event, requester, text);
    else if (text.find("item ") == 0)
        return HandleItem(event, requester, text);
    else if (text.find("find ") == 0)
        return HandleFind(event, requester, text);
    else if (text.find("rpg ") == 0)
        return HandleRPG(event, requester, text);
    else if (text.find("rpgtargets") == 0)
        return HandleRPGTargets(event, requester, text);    
    else if (text.find("travel ") == 0)
        return HandleTravel(event, requester, text);
    else if (text.find("print travel") == 0)
        return HandlePrintTravel(event, requester, text);
    else if (text.find("values ") == 0)
        return HandleValues(event, requester, text);
    else if (text.find("loot ") == 0)
        return HandleLoot(event, requester, text);
    else if (text.find("loot") == 0)
        return HandleLoot(event, requester, text);
    else if (text.find("drops ") == 0)
        return HandleDrops(event, requester, text);
    else if (text.find("taxi") == 0)
        return HandleTaxi(event, requester, text);
    else if (text.find("price ") == 0)
        return HandlePrice(event, requester, text);
    else if (text.find("nc") == 0)
        return HandleNC(event, requester, text);
    else if (text.find("add node") == 0 && isMod)
        return HandleAddNode(event, requester, text);
    else if (text.find("rem node") == 0 && isMod)
        return HandleRemNode(event, requester, text);
    else if (text.find("reset node") == 0 && isMod)
        return HandleResetNode(event, requester, text);
    else if (text.find("reset path") == 0 && isMod)
        return HandleResetPath(event, requester, text);
    else if (text.find("gen node") == 0 && isMod)
        return HandleGenNode(event, requester, text);
    else if (text.find("gen path") == 0 && isMod)
        return HandleGenPath(event, requester, text);
    else if (text.find("crop path") == 0 && isMod)
        return HandleCropPath(event, requester, text);
    else if (text.find("save node") == 0 && isMod)
        return HandleSaveNode(event, requester, text);
    else if (text.find("load node") == 0 && isMod)
        return HandleLoadNode(event, requester, text);
    else if (text.find("show node") == 0 && isMod)
        return HandleShowNode(event, requester, text);
    else if (text.find("dspell ") == 0 && isMod)
        return HandleDSpell(event, requester, text);
    else if (text.find("vspell ") == 0 && isMod)
        return HandleVSpell(event, requester, text);
    else if (text.find("aspell ") == 0 && isMod)
        return HandleASpell(event, requester, text);
    else if (text.find("cspell ") == 0 && isMod)
        return HandleCSpell(event, requester, text);
    else if (text.find("fspell ") == 0 && isMod)
        return HandleFSpell(event, requester, text);
    else if (text.find("spell ") == 0 && isMod)
        return HandleSpell(event, requester, text);
    else if (text.find("tspellmap") == 0 && isMod)
        return HandleTSpellMap(event, requester, text);
    else if (text.find("uspellmap") == 0 && isMod)
        return HandleUSpellMap(event, requester, text);
    else if (text.find("dspellmap") == 0 && isMod)
        return HandleDSpellMap(event, requester, text);
    else if (text.find("vspellmap") == 0 && isMod)
        return HandleVSpellMap(event, requester, text);
    else if (text.find("ispellmap") == 0 && isMod)
        return HandleISpellMap(event, requester, text);
    else if (text.find("cspellmap") == 0 && isMod)
        return HandleCSpellMap(event, requester, text);
    else if (text.find("aspellmap") == 0 && isMod)
        return HandleASpellMap(event, requester, text);
    else if (text.find("gspellmap") == 0 && isMod)
        return HandleGSpellMap(event, requester, text);
    else if (text.find("mspellmap") == 0 && isMod)
        return HandleMSpellMap(event, requester, text);
    else if (text.find("soundmap") == 0 && isMod)
        return HandleSoundMap(event, requester, text);
    else if (text.find("sounds") == 0 && isMod)
        return HandleSounds(event, requester, text);
    else if (text.find("dsound") == 0 && isMod)
        return HandleDSound(event, requester, text);
    else if (text.find("sound") == 0 && isMod)
        return HandleSound(event, requester, text);
    else if (text.find("stuck") == 0)
        return HandleStuck(event, requester, text);
    else if (text.find("combat") == 0)
        return HandleCombat(event, requester, text);
    else if (text.find("nodes") == 0)
        return HandleNodes(event, requester, text);
    else if (text.find("activity") == 0)
        return HandleActivity(event, requester, text);
    else if (text.find("transanal") == 0)
        return HandleTransanal(event, requester, text);
    else if (text.find("updownspace") == 0 && isMod)
        return HandleUpdownspace(event, requester, text);
    else if (text.find("patharound") == 0 && isMod)
        return HandlePatharound(event, requester, text);
    else if (text.find("heightforlos") == 0 && isMod)
        return HandleHeightForLos(event, requester, text);

    // Fallback/default behavior
    std::string response = ai->HandleRemoteCommand(text);
    ai->TellPlayer(requester, response);
    return true;
}

bool DebugAction::HandleDebugHelp(Event& event, Player* requester, const std::string& text, bool isMod)
{
    std::string cmd = text;
    if (cmd.find("help ") == 0)
        cmd = cmd.substr(5);
    else
        cmd = "";
    
    while (cmd.size() > 0 && cmd[0] == ' ')
        cmd = cmd.substr(1);
    
    if (cmd.empty())
    {
        ai->TellPlayer(requester, "=== Debug Commands ===");
        ai->TellPlayer(requester, "Usage: debug help <command>");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "General: position, quest, values, level, who, stats, spells");
        ai->TellPlayer(requester, "Movement: route, path, distance, teleport, zone");
        ai->TellPlayer(requester, "Info: target, movement, corpse, logouttime, taxi");
        ai->TellPlayer(requester, "Interaction: npc, go, rpg, travel, loot, trade, mail");
        ai->TellPlayer(requester, "Quest: quest list|complete|drop|add|travel");
        ai->TellPlayer(requester, "Nodes: nodes");
        
        if (isMod)
        {
            ai->TellPlayer(requester, "Mod Only: llm, area, avoid, motion, transport, nc, gps");
        }
        
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Type 'debug help <command>' for detailed help.");
        return true;
    }
    
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    if (cmd == "position" || cmd == "pos")
    {
        ai->TellPlayer(requester, "=== debug position ===");
        ai->TellPlayer(requester, "Get position info and manipulate position-related data.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug position                    - Bot's GPS coordinates");
        ai->TellPlayer(requester, "  debug position zone [loc]          - Zone info (bot or location)");
        ai->TellPlayer(requester, "  debug position teleport [loc]     - Teleport to master or location");
        ai->TellPlayer(requester, "  debug position distance [loc]    - Distance to master or location");
        ai->TellPlayer(requester, "  debug position path <from>|<to>  - Full path with movement breakdown");
        ai->TellPlayer(requester, "  debug position path <loc> full   - Show all path points");
        ai->TellPlayer(requester, "  debug position route <from>|<to> - Route nodes with map/area");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Location formats:");
        ai->TellPlayer(requester, "  x y                    - Coordinates (uses bot's map/z)");
        ai->TellPlayer(requester, "  map x y                - 3 params");
        ai->TellPlayer(requester, "  map x y z              - 4 params");
        ai->TellPlayer(requester, "  LocationName           - Named location (case sensitive)");
        ai->TellPlayer(requester, "  from|to                - Pipe separates start and end");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug position");
        ai->TellPlayer(requester, "  debug position teleport Stormwind");
        ai->TellPlayer(requester, "  debug position distance Orgrimmar");
        ai->TellPlayer(requester, "  debug position path Orgrimmar|Stormwind");
        ai->TellPlayer(requester, "  debug position route Orgrimmar|Stormwind full");
    }
    else if (cmd == "quest")
    {
        ai->TellPlayer(requester, "=== debug quest ===");
        ai->TellPlayer(requester, "Manage and view quest information.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug quest                   - List all quests in bot's log");
        ai->TellPlayer(requester, "  debug quest <id>              - Show quest details");
        ai->TellPlayer(requester, "  debug quest complete <id>     - Complete a quest");
        ai->TellPlayer(requester, "  debug quest drop <id>         - Abandon a quest");
        ai->TellPlayer(requester, "  debug quest add <id>          - Add quest to log");
        ai->TellPlayer(requester, "  debug quest travel <id>       - Show quest objectives");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug quest");
        ai->TellPlayer(requester, "  debug quest 1");
        ai->TellPlayer(requester, "  debug quest complete 1");
    }
    else if (cmd == "route")
    {
        ai->TellPlayer(requester, "=== debug position route ===");
        ai->TellPlayer(requester, "Show travel nodes between two locations.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug position route <from>|<to>");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Output:");
        ai->TellPlayer(requester, "  - List of travel nodes with names, zone, map");
        ai->TellPlayer(requester, "  - Distance between each node");
        ai->TellPlayer(requester, "  - Total route distance");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug position route Orgrimmar|Stormwind");
        ai->TellPlayer(requester, "  debug position route ThunderBluff|Stormwind");
    }
    else if (cmd == "path")
    {
        ai->TellPlayer(requester, "=== debug position path ===");
        ai->TellPlayer(requester, "Show full pathfinding route with movement breakdown.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug position path <loc>              - Summary only");
        ai->TellPlayer(requester, "  debug position path <loc> full          - All points");
        ai->TellPlayer(requester, "  debug position path <from>|<to>        - Between two locations");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Output:");
        ai->TellPlayer(requester, "  - Total distance");
        ai->TellPlayer(requester, "  - Walk/Fly/Transport/Teleport breakdown");
        ai->TellPlayer(requester, "  - Map transitions");
        ai->TellPlayer(requester, "  - Next waypoint coordinates");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Node Types: 0=prepath, 1=walk, 4=transport, 5=flight, 6=teleport");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug position path Stormwind");
        ai->TellPlayer(requester, "  debug position path Orgrimmar|Stormwind full");
    }
    else if (cmd == "distance" || cmd == "dist")
    {
        ai->TellPlayer(requester, "=== debug position distance ===");
        ai->TellPlayer(requester, "Calculate distance to a location.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug position distance [location]");
        ai->TellPlayer(requester, "  Without location: distance to master");
        ai->TellPlayer(requester, "  With location: distance to that point");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug position distance");
        ai->TellPlayer(requester, "  debug position distance Orgrimmar");
    }
    else if (cmd == "teleport")
    {
        ai->TellPlayer(requester, "=== debug position teleport ===");
        ai->TellPlayer(requester, "Teleport bot to a location.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug position teleport [location|coords]");
        ai->TellPlayer(requester, "  Without params: teleport to master");
        ai->TellPlayer(requester, "  With location: teleport to named location");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug position teleport");
        ai->TellPlayer(requester, "  debug position teleport Stormwind");
        ai->TellPlayer(requester, "  debug position teleport -8944 0.5");
    }
    else if (cmd == "zone")
    {
        ai->TellPlayer(requester, "=== debug position zone ===");
        ai->TellPlayer(requester, "Get zone information for a location.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug position zone [location]");
        ai->TellPlayer(requester, "  Without params: bot's current zone");
        ai->TellPlayer(requester, "  With location: zone at that location");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug position zone");
        ai->TellPlayer(requester, "  debug position zone Stormwind");
    }
    else if (cmd == "values")
    {
        ai->TellPlayer(requester, "=== debug values ===");
        ai->TellPlayer(requester, "List and get bot AI values.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug values                    - List all values");
        ai->TellPlayer(requester, "  debug values <name>            - Get value by name");
        ai->TellPlayer(requester, "  debug setvalueuin32 <name> <v> - Set uint32 value");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Examples:");
        ai->TellPlayer(requester, "  debug values");
        ai->TellPlayer(requester, "  debug values last movement");
    }
    else if (cmd == "gps")
    {
        ai->TellPlayer(requester, "=== debug gps ===");
        ai->TellPlayer(requester, "Get detailed position information (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug gps");
        ai->TellPlayer(requester, "  Shows: X, Y, Z, Map, Area, Zone, Orientation");
    }
    else if (cmd == "level")
    {
        ai->TellPlayer(requester, "=== debug level ===");
        ai->TellPlayer(requester, "Get or set bot level.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug level          - Show current level");
        ai->TellPlayer(requester, "  debug level <n>     - Set level to n");
    }
    else if (cmd == "who")
    {
        ai->TellPlayer(requester, "=== debug who ===");
        ai->TellPlayer(requester, "Show detailed bot information.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug who");
        ai->TellPlayer(requester, "  Shows: Name, Level, Class, HP, Mana, Gold, Location");
    }
    else if (cmd == "stats")
    {
        ai->TellPlayer(requester, "=== debug stats ===");
        ai->TellPlayer(requester, "Show bot statistics.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug stats");
        ai->TellPlayer(requester, "  Shows: Gold, Bag slots, Durability, XP, Mana");
    }
    else if (cmd == "spells")
    {
        ai->TellPlayer(requester, "=== debug spells ===");
        ai->TellPlayer(requester, "List bot's known spells.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug spells");
    }
    else if (cmd == "target")
    {
        ai->TellPlayer(requester, "=== debug target ===");
        ai->TellPlayer(requester, "Show bot's current target.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug target");
        ai->TellPlayer(requester, "  Shows: Target name, GUID, type");
    }
    else if (cmd == "movement")
    {
        ai->TellPlayer(requester, "=== debug movement ===");
        ai->TellPlayer(requester, "Show last movement coordinates.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug movement");
    }
    else if (cmd == "mount")
    {
        ai->TellPlayer(requester, "=== debug mount ===");
        ai->TellPlayer(requester, "Mount the bot.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug mount");
    }
    else if (cmd == "unmount")
    {
        ai->TellPlayer(requester, "=== debug unmount ===");
        ai->TellPlayer(requester, "Dismount the bot.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug unmount");
    }
    else if (cmd == "who")
    {
        ai->TellPlayer(requester, "=== debug who ===");
        ai->TellPlayer(requester, "Show detailed bot information.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug who");
        ai->TellPlayer(requester, "  Shows: Name, Level, Class, HP, Mana, Gold, Location");
    }
    else if (cmd == "stats")
    {
        ai->TellPlayer(requester, "=== debug stats ===");
        ai->TellPlayer(requester, "Show bot statistics.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug stats");
        ai->TellPlayer(requester, "  Shows: Gold, Bag slots, Durability, XP, Mana");
    }
    else if (cmd == "spells")
    {
        ai->TellPlayer(requester, "=== debug spells ===");
        ai->TellPlayer(requester, "List bot's known spells.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug spells");
    }
    else if (cmd == "gps" && isMod)
    {
        ai->TellPlayer(requester, "=== debug gps ===");
        ai->TellPlayer(requester, "Get detailed position information (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug gps");
        ai->TellPlayer(requester, "  Shows: X, Y, Z, Map, Area, Zone, Orientation");
    }
    else if (cmd == "llm" && isMod)
    {
        ai->TellPlayer(requester, "=== debug llm ===");
        ai->TellPlayer(requester, "LLM interaction (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug llm <prompt>");
    }
    else if (cmd == "area" && isMod)
    {
        ai->TellPlayer(requester, "=== debug area ===");
        ai->TellPlayer(requester, "Area debugging (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug area");
    }
    else if (cmd == "avoid" && isMod)
    {
        ai->TellPlayer(requester, "=== debug avoid ===");
        ai->TellPlayer(requester, "Avoidance management (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug avoid scan");
        ai->TellPlayer(requester, "  debug avoid add <spellId>");
    }
    else if (cmd == "trade")
    {
        ai->TellPlayer(requester, "=== debug trade ===");
        ai->TellPlayer(requester, "Manage trade window with bot.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug trade       - Show trade status");
        ai->TellPlayer(requester, "  debug trade <item> - Use item in trade");
    }
    else if (cmd == "mail")
    {
        ai->TellPlayer(requester, "=== debug mail ===");
        ai->TellPlayer(requester, "Send mail to bot.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug mail <subject>");
    }
    else if (cmd == "taxi")
    {
        ai->TellPlayer(requester, "=== debug taxi ===");
        ai->TellPlayer(requester, "Show taxi/fly routes.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug taxi");
    }
    else if (cmd == "rpg")
    {
        ai->TellPlayer(requester, "=== debug rpg ===");
        ai->TellPlayer(requester, "Roleplay interaction.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug rpg <action>");
    }
    else if (cmd == "travel")
    {
        ai->TellPlayer(requester, "=== debug travel ===");
        ai->TellPlayer(requester, "Travel to a location.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug travel <location>");
    }
    else if (cmd == "loot")
    {
        ai->TellPlayer(requester, "=== debug loot ===");
        ai->TellPlayer(requester, "Loot information.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug loot");
        ai->TellPlayer(requester, "  debug loot <target>");
    }
    else if (cmd == "npc")
    {
        ai->TellPlayer(requester, "=== debug npc ===");
        ai->TellPlayer(requester, "Interact with NPC.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug npc <npc>");
    }
    else if (cmd == "go" || cmd == "goto")
    {
        ai->TellPlayer(requester, "=== debug go ===");
        ai->TellPlayer(requester, "Go to location or object.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage:");
        ai->TellPlayer(requester, "  debug go <coords>");
        ai->TellPlayer(requester, "  debug go <gameobject>");
    }
    else if (cmd == "corpse")
    {
        ai->TellPlayer(requester, "=== debug corpse ===");
        ai->TellPlayer(requester, "Show corpse location.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug corpse");
    }
    else if (cmd == "nc" || cmd == "nocombat")
    {
        ai->TellPlayer(requester, "=== debug nc ===");
        ai->TellPlayer(requester, "Toggle non-combat mode.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug nc");
    }
    else if (cmd == "motion" && isMod)
    {
        ai->TellPlayer(requester, "=== debug motion ===");
        ai->TellPlayer(requester, "Motion controller (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug motion <params>");
    }
    else if (cmd == "transport" && isMod)
    {
        ai->TellPlayer(requester, "=== debug transport ===");
        ai->TellPlayer(requester, "Transport debugging (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug transport");
    }
    else if (cmd == "do")
    {
        ai->TellPlayer(requester, "=== debug do ===");
        ai->TellPlayer(requester, "Execute bot action directly.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug do <action>");
    }
    else if (cmd == "item")
    {
        ai->TellPlayer(requester, "=== debug item ===");
        ai->TellPlayer(requester, "Use or manage items.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug item <item>");
    }
    else if (cmd == "find")
    {
        ai->TellPlayer(requester, "=== debug find ===");
        ai->TellPlayer(requester, "Find objects or NPCs.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug find <name>");
    }
    else if (cmd == "drops")
    {
        ai->TellPlayer(requester, "=== debug drops ===");
        ai->TellPlayer(requester, "Show loot drops from creatures.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug drops <creature>");
    }
    else if (cmd == "price")
    {
        ai->TellPlayer(requester, "=== debug price ===");
        ai->TellPlayer(requester, "Show item vendor price.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug price <item>");
    }
    else if (cmd == "logouttime")
    {
        ai->TellPlayer(requester, "=== debug logouttime ===");
        ai->TellPlayer(requester, "Show logout timer.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug logouttime");
    }
    else if (cmd == "rpgtargets")
    {
        ai->TellPlayer(requester, "=== debug rpgtargets ===");
        ai->TellPlayer(requester, "Show RPG interaction targets.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug rpgtargets");
    }
    else if (cmd == "poi")
    {
        ai->TellPlayer(requester, "=== debug poi ===");
        ai->TellPlayer(requester, "Show point of interest.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug poi <poiId>");
    }
    else if (cmd == "setvalue" || cmd == "setvalueuin32")
    {
        ai->TellPlayer(requester, "=== debug setvalue ===");
        ai->TellPlayer(requester, "Set bot AI value.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug setvalueuin32 <name> <value>");
    }
    // MOD ONLY - avoid
    else if (cmd == "avoid scan" && isMod)
    {
        ai->TellPlayer(requester, "=== debug avoid scan ===");
        ai->TellPlayer(requester, "Scan for avoidance targets (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug avoid scan");
    }
    else if (cmd == "avoid add" && isMod)
    {
        ai->TellPlayer(requester, "=== debug avoid add ===");
        ai->TellPlayer(requester, "Add avoidance target (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug avoid add <spellId>");
    }
    // MOD ONLY - travel nodes
    else if (cmd == "add node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug add node ===");
        ai->TellPlayer(requester, "Add travel node (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug add node");
    }
    else if (cmd == "rem node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug rem node ===");
        ai->TellPlayer(requester, "Remove travel node (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug rem node");
    }
    else if (cmd == "reset node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug reset node ===");
        ai->TellPlayer(requester, "Reset travel nodes (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug reset node");
    }
    else if (cmd == "reset path" && isMod)
    {
        ai->TellPlayer(requester, "=== debug reset path ===");
        ai->TellPlayer(requester, "Reset travel path (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug reset path");
    }
    else if (cmd == "gen node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug gen node ===");
        ai->TellPlayer(requester, "Generate travel node (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug gen node");
    }
    else if (cmd == "gen path" && isMod)
    {
        ai->TellPlayer(requester, "=== debug gen path ===");
        ai->TellPlayer(requester, "Generate travel path (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug gen path");
    }
    else if (cmd == "crop path" && isMod)
    {
        ai->TellPlayer(requester, "=== debug crop path ===");
        ai->TellPlayer(requester, "Crop travel path (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug crop path");
    }
    else if (cmd == "save node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug save node ===");
        ai->TellPlayer(requester, "Save travel node (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug save node");
    }
    else if (cmd == "load node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug load node ===");
        ai->TellPlayer(requester, "Load travel node (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug load node");
    }
    else if (cmd == "show node" && isMod)
    {
        ai->TellPlayer(requester, "=== debug show node ===");
        ai->TellPlayer(requester, "Show travel nodes (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug show node");
    }
    else if (cmd == "print travel" && isMod)
    {
        ai->TellPlayer(requester, "=== debug print travel ===");
        ai->TellPlayer(requester, "Print travel info (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug print travel");
    }
    // MOD ONLY - transport
    else if (cmd == "pointontrans" && isMod)
    {
        ai->TellPlayer(requester, "=== debug pointontrans ===");
        ai->TellPlayer(requester, "Point on transport (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug pointontrans");
    }
    else if (cmd == "dotrans" && isMod)
    {
        ai->TellPlayer(requester, "=== debug dotrans ===");
        ai->TellPlayer(requester, "Do transport action (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug dotrans");
    }
    else if (cmd == "ontrans" && isMod)
    {
        ai->TellPlayer(requester, "=== debug ontrans ===");
        ai->TellPlayer(requester, "On transport (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug ontrans");
    }
    else if (cmd == "offtrans" && isMod)
    {
        ai->TellPlayer(requester, "=== debug offtrans ===");
        ai->TellPlayer(requester, "Off transport (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug offtrans");
    }
    // MOD ONLY - map
    else if (cmd == "pathable" && isMod)
    {
        ai->TellPlayer(requester, "=== debug pathable ===");
        ai->TellPlayer(requester, "Check if location is pathable (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug pathable");
    }
    else if (cmd == "randomspot" && isMod)
    {
        ai->TellPlayer(requester, "=== debug randomspot ===");
        ai->TellPlayer(requester, "Show random spot (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug randomspot");
    }
    else if (cmd == "printmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug printmap ===");
        ai->TellPlayer(requester, "Print map info (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug printmap");
    }
    // MOD ONLY - spells
    else if (cmd == "dspell" && isMod)
    {
        ai->TellPlayer(requester, "=== debug dspell ===");
        ai->TellPlayer(requester, "Debug spell (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug dspell <spellId>");
    }
    else if (cmd == "vspell" && isMod)
    {
        ai->TellPlayer(requester, "=== debug vspell ===");
        ai->TellPlayer(requester, "Validate spell (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug vspell <spellId>");
    }
    else if (cmd == "aspell" && isMod)
    {
        ai->TellPlayer(requester, "=== debug aspell ===");
        ai->TellPlayer(requester, "Add spell (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug aspell <spellId>");
    }
    else if (cmd == "cspell" && isMod)
    {
        ai->TellPlayer(requester, "=== debug cspell ===");
        ai->TellPlayer(requester, "Cast spell (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug cspell <spellId>");
    }
    else if (cmd == "fspell" && isMod)
    {
        ai->TellPlayer(requester, "=== debug fspell ===");
        ai->TellPlayer(requester, "Fake spell (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug fspell <spellId>");
    }
    else if (cmd == "spell" && isMod)
    {
        ai->TellPlayer(requester, "=== debug spell ===");
        ai->TellPlayer(requester, "Spell action (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug spell <spellId>");
    }
    // MOD ONLY - spell maps
    else if (cmd == "tspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug tspellmap ===");
        ai->TellPlayer(requester, "Test spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug tspellmap");
    }
    else if (cmd == "uspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug uspellmap ===");
        ai->TellPlayer(requester, "Use spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug uspellmap");
    }
    else if (cmd == "dspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug dspellmap ===");
        ai->TellPlayer(requester, "Debug spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug dspellmap");
    }
    else if (cmd == "vspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug vspellmap ===");
        ai->TellPlayer(requester, "Validate spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug vspellmap");
    }
    else if (cmd == "ispellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug ispellmap ===");
        ai->TellPlayer(requester, "Info spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug ispellmap");
    }
    else if (cmd == "cspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug cspellmap ===");
        ai->TellPlayer(requester, "Cast spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug cspellmap");
    }
    else if (cmd == "aspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug aspellmap ===");
        ai->TellPlayer(requester, "Add spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug aspellmap");
    }
    else if (cmd == "gspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug gspellmap ===");
        ai->TellPlayer(requester, "Get spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug gspellmap");
    }
    else if (cmd == "mspellmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug mspellmap ===");
        ai->TellPlayer(requester, "Modify spell map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug mspellmap");
    }
    // MOD ONLY - sounds
    else if (cmd == "soundmap" && isMod)
    {
        ai->TellPlayer(requester, "=== debug soundmap ===");
        ai->TellPlayer(requester, "Sound map (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug soundmap");
    }
    else if (cmd == "sounds" && isMod)
    {
        ai->TellPlayer(requester, "=== debug sounds ===");
        ai->TellPlayer(requester, "List sounds (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug sounds");
    }
    else if (cmd == "dsound" && isMod)
    {
        ai->TellPlayer(requester, "=== debug dsound ===");
        ai->TellPlayer(requester, "Debug sound (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug dsound");
    }
    else if (cmd == "sound" && isMod)
    {
        ai->TellPlayer(requester, "=== debug sound ===");
        ai->TellPlayer(requester, "Play sound (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug sound <soundId>");
    }
    // MOD ONLY - other
    else if (cmd == "chatreplydo" && isMod)
    {
        ai->TellPlayer(requester, "=== debug chatreplydo ===");
        ai->TellPlayer(requester, "Chat reply action (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug chatreplydo <text>");
    }
    else if (cmd == "monstertalk" && isMod)
    {
        ai->TellPlayer(requester, "=== debug monstertalk ===");
        ai->TellPlayer(requester, "Monster talk (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug monstertalk <text>");
    }
    else if (cmd == "gy" && isMod)
    {
        ai->TellPlayer(requester, "=== debug gy ===");
        ai->TellPlayer(requester, "Graveyard info (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug gy");
    }
    else if (cmd == "grid" && isMod)
    {
        ai->TellPlayer(requester, "=== debug grid ===");
        ai->TellPlayer(requester, "Grid info (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug grid");
    }
    else if (cmd == "test" && isMod)
    {
        ai->TellPlayer(requester, "=== debug test ===");
        ai->TellPlayer(requester, "Testing command (mod only).");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug test");
    }
    else if (cmd == "nodes")
    {
        ai->TellPlayer(requester, "=== debug nodes ===");
        ai->TellPlayer(requester, "Debug travel node connection - why bots get stuck.");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Usage: debug nodes");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "This command checks:");
        ai->TellPlayer(requester, "  - Nodes within 100y of bot");
        ai->TellPlayer(requester, "  - If pathfinding works to each node");
        ai->TellPlayer(requester, "  - What getNode() returns (the actual function used)");
        ai->TellPlayer(requester, "  - Why pathfinding fails if it does");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Output:");
        ai->TellPlayer(requester, "  - Step-by-step pathfinding results");
        ai->TellPlayer(requester, "  - Path type and point count");
        ai->TellPlayer(requester, "  - Distance reached vs total distance");
        ai->TellPlayer(requester, "  - Suggested fixes");
    }
    else
    {
        ai->TellPlayer(requester, "Unknown command: " + cmd);
        ai->TellPlayer(requester, "Type 'debug help' for list of commands.");
    }
    
    return true;
}

void DebugAction::FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target, std::list<ObjectGuid> otherTargets, std::list<ObjectGuid> missTargets, WorldPosition source, WorldPosition dest, bool forceDest)
{
    SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(spellId);
    {
        uint32 castFlags = CAST_FLAG_UNKNOWN2;

        if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_USES_RANGED_SLOT))
            castFlags |= CAST_FLAG_AMMO;                        // arrows/bullets visual

        WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));

        data << truecaster->GetPackGUID();                      //truecaster

        if (caster)
            data << caster->GetPackGUID();                          //m_caster->GetPackGUID();     
        else
            data << ObjectGuid();

        data << uint32(spellId);                                // spellId
        data << uint16(castFlags);                              // cast flags
        data << uint32(1000.0f);                                   // delay?

        SpellCastTargets m_targets;

        data << m_targets;

        //projectile info
        if (castFlags & CAST_FLAG_AMMO)
        {
            data << uint32(5996);
            data << uint32(INVTYPE_AMMO);
        }

        if(caster)
            caster->GetMap()->MessageBroadcast(caster, data);
        else
            truecaster->GetMap()->MessageBroadcast(truecaster, data);
    }
    {
        uint32 castFlags = CAST_FLAG_UNKNOWN9;



        if (spellInfo && spellInfo->HasAttribute(SPELL_ATTR_USES_RANGED_SLOT))
            castFlags |= CAST_FLAG_AMMO;                        // arrows/bullets visual
        if (spellInfo && HasPersistentAuraEffect(spellInfo))
            castFlags |= CAST_FLAG_PERSISTENT_AA;

        WorldPacket data(SMSG_SPELL_GO, 53);                    // guess size

        data << truecaster->GetPackGUID();                      //truecaster

        if (caster)
            data << caster->GetPackGUID();                          //m_caster->GetPackGUID();     
        else
            data << ObjectGuid();

        data << uint32(spellId);                                // spellId
        data << uint16(castFlags);                              // cast flags

        //WriteSpellGoTargets
        uint32 hits = otherTargets.size() + (target ? 1 : 0);

        data << uint8(hits);                                      // Hits                   

        if (target)
            data << target;

        //Hit targets here.                   
        for (auto otherTarget : otherTargets)
            data << otherTarget;

        data << (uint8)missTargets.size();                         //miss

        for (auto missTarget : missTargets)
        {
            data << missTarget;

            data << uint8(SPELL_MISS_RESIST);    //Miss condition
            data << uint8(SPELL_MISS_NONE);    //Miss condition
        }

        SpellCastTargets m_targets;

        if ((spellInfo && spellInfo->Targets & TARGET_FLAG_DEST_LOCATION) || forceDest)
            m_targets.setDestination(dest.getX(), dest.getY(), dest.getZ());
        if ((spellInfo && spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION) || forceDest)
            m_targets.setSource(source.getX(), source.getY(), source.getZ());
        if (!forceDest && target)
            if(!spellInfo || !(spellInfo->Targets & TARGET_FLAG_DEST_LOCATION && spellInfo->Targets & TARGET_FLAG_SOURCE_LOCATION))
                m_targets.setUnitTarget(ai->GetUnit(target));

        data << m_targets;

        //projectile info
        if (castFlags & CAST_FLAG_AMMO)
        {
            data << uint32(5996);
            data << uint32(INVTYPE_AMMO);
        }

        if (caster)
            caster->GetMap()->MessageBroadcast(caster, data);
        else
            truecaster->GetMap()->MessageBroadcast(truecaster, data);
    }
}

void DebugAction::addAura(uint32 spellId, Unit* target)
{
    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellInfo)
        return;

    if (!IsSpellAppliesAura(spellInfo, (1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2)) &&
        !IsSpellHaveEffect(spellInfo, SPELL_EFFECT_PERSISTENT_AREA_AURA))
    {
        return;
    }

    SpellAuraHolder* holder = CreateSpellAuraHolder(spellInfo, target, target);

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        uint8 eff = spellInfo->Effect[i];
        if (eff >= MAX_SPELL_EFFECTS)
            continue;
        if (IsAreaAuraEffect(eff) ||
            eff == SPELL_EFFECT_APPLY_AURA ||
            eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        {
            int32 basePoints = spellInfo->CalculateSimpleValue(SpellEffectIndex(i));
            int32 damage = 0; // no damage cos caster doesnt exist
            Aura* aur = CreateAura(spellInfo, SpellEffectIndex(i), &damage, &basePoints, holder, target);
            holder->AddAura(aur, SpellEffectIndex(i));
        }
    }
    if (!target->AddSpellAuraHolder(holder))
        delete holder;

    return;
}

// Handler implementations

bool DebugAction::HandleAvoidScan(Event& event, Player* requester, const std::string& text)
{
    PathFinder path(requester);
    for (float x = -100.0f; x < 100.0f; x += 2.0f)
        for (float y = -100.0f; y < 100.0f; y += 2.0f)
        {
            WorldPosition p(requester);
            p.setX(p.getX() + x);
            p.setY(p.getY() + y);
            p.setZ(p.getHeight(bot->GetInstanceId()));
            Creature* wpCreature = requester->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0.0, TEMPSPAWN_TIMED_DESPAWN, 20000.0f);
            if(path.getArea(p.getMapId(), p.getX(), p.getY(), p.getZ()) == 12)
                ai->AddAura(wpCreature, 246);
            if (path.getArea(p.getMapId(), p.getX(), p.getY(), p.getZ()) == 13)
                ai->AddAura(wpCreature, 1130);
        }
    return true;
}

bool DebugAction::HandleAvoidAdd(Event& event, Player* requester, const std::string& text)
{
    PathFinder pathfinder(bot);
    WorldPosition point(requester);
    uint32 area, radius;
    std::vector<std::string> args = { "12", "5" };
    if (text.length() > std::string("avoid add").size())
    {
        args = ChatHelper::splitString(text.substr(std::string("avoid add").size() + 1), " ");
        if (args.size() == 1)
            args.push_back("5");
    }
    area = stoi(args[0]);
    radius = stoi(args[1]);
    pathfinder.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), area, radius);
    return true;
}

bool DebugAction::HandleMount(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;
    out << (bot->IsMounted() ? "mounted" : "not mounted");
    out << (bot->IsTaxiFlying() ? ", taxi flying" : ", not taxi flying");
    out << ", mount speed: " << AI_VALUE2(uint32, "current mount speed", "self target");
    out << ", mount id: " << bot->GetMountID();        
    if(bot->GetMountInfo())
        out << ", mount info: " << bot->GetMountInfo()->Name;
    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return true;
}

bool DebugAction::HandleUnmount(Event& event, Player* requester, const std::string& text)
{
    ai->Unmount();
    return true;
}

bool DebugAction::HandleArea(Event& event, Player* requester, const std::string& text)
{
    WorldPosition point(requester);
    AreaTableEntry const* area = point.GetArea();
    std::ostringstream out;
    out << point.getAreaName(true, false); 
    out << "," << area->team << " (" << (area->team != FACTION_GROUP_MASK_ALLIANCE ? (area->team != FACTION_GROUP_MASK_HORDE ? "neutral" : "horde") : "alliance") << ")";
    ai->TellPlayerNoFacing(requester, out, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return true;
}

bool DebugAction::HandleLLM(Event& event, Player* requester, const std::string& text)
{
    Player* player = bot;
    std::map<std::string, std::string> jsonFill;
    jsonFill["<prompt>"] = text.substr(4);
    jsonFill["<context>"] = "";
    jsonFill["<pre prompt>"] = "";
    jsonFill["<post prompt>"] = "";
    std::string json = BOT_TEXT2(sPlayerbotAIConfig.llmApiJson, jsonFill);
    std::vector<std::string> debugLines = { json };
    std::string response = PlayerbotLLMInterface::Generate(json, sPlayerbotAIConfig.llmGenerationTimeout, sPlayerbotAIConfig.llmMaxSimultaniousGenerations, debugLines);
    for(auto line : debugLines)
        ai->TellPlayerNoFacing(requester, line, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    ai->TellPlayerNoFacing(requester, response, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    return true;
}

bool DebugAction::HandleChatReplyDo(Event& event, Player* requester, const std::string& text)
{
    ChatReplyAction::ChatReplyDo(bot, CHAT_MSG_WHISPER, requester->GetGUIDLow(), 0, text.substr(12), "", requester->GetName());
    return true;
}

bool DebugAction::HandleMonsterTalk(Event& event, Player* requester, const std::string& text)
{
    GuidPosition guidP = GuidPosition(requester->GetSelectionGuid(), requester);
    Player* player = requester;
    if (!guidP.IsUnit())
        return false;
    Action* action = context->GetAction("rpg ai chat");
    if (!action)
        return false;
    RpgAIChatAction* rpgAction = (RpgAIChatAction*)action;
    if (!rpgAction)
        return false;
    std::string line = text.substr(12);
    rpgAction->ManualChat(guidP, line);
    return true;
}

bool DebugAction::HandleGY(Event& event, Player* requester, const std::string& text)
{
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        if (!sMapStore.LookupEntry(i))
            continue;

        uint32 mapId = sMapStore.LookupEntry(i)->MapID;

        Map* map = sMapMgr.FindMap(mapId);

        if (!map)
            continue;

        GraveyardManager* gy = &map->GetGraveyardManager();

        if (!gy)
            continue;

        for (uint32 x = 0; x < TOTAL_NUMBER_OF_CELLS_PER_MAP; x++)
        {
            for (uint32 y = 0; y < TOTAL_NUMBER_OF_CELLS_PER_MAP; y++)
            {
                CellPair c(x, y);

                WorldPosition pos(mapId, c);

                if (!pos.isValid())
                {
                    continue;
                }

                pos.setZ(pos.getHeight(bot->GetInstanceId()));

                const uint32 zoneId = sTerrainMgr.GetZoneId(mapId, pos.getX(), pos.getY(), pos.getZ());
                const uint32 areaId = sTerrainMgr.GetAreaId(mapId, pos.getX(), pos.getY(), pos.getZ());

                WorldSafeLocsEntry const* graveyard = nullptr;
                if (areaId != 0)
                {
                    WorldSafeLocsEntry const* ClosestGrave;
                    ClosestGrave = gy->GetClosestGraveYard(pos.getX(), pos.getY(), pos.getZ(), mapId, ALLIANCE);
                    ClosestGrave = gy->GetClosestGraveYard(pos.getX(), pos.getY(), pos.getZ(), mapId, HORDE);
                }
            }
        }
    }
    return true;
}

bool DebugAction::HandleGrid(Event& event, Player* requester, const std::string& text)
{
    WorldPosition botPos = bot;
    std::string loaded = botPos.getMap(bot->GetInstanceId())->IsLoaded(botPos.getX(), botPos.getY()) ? "loaded" : "unloaded";

    std::ostringstream out;

    out << "Map: " << botPos.getMapId() << " " << botPos.getAreaName() << " Grid: " << botPos.getGridPair().x_coord << "," << botPos.getGridPair().y_coord << " [" << loaded << "] Cell: " << botPos.getCellPair().x_coord << "," << botPos.getCellPair().y_coord;

    bot->Whisper(out.str().c_str(), LANG_UNIVERSAL, event.getOwner()->GetObjectGuid());

    return true;
}

bool DebugAction::HandleTest(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 4)
    {
        param = text.substr(5);
    }

    if (param.empty())
    {
        ai->TellPlayer(requester, "=== debug test ===");
        ai->TellPlayer(requester, "Usage: debug test <command>");
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "  locations          - List all test locations");
        ai->TellPlayer(requester, "  locate <name>      - Test location resolution");
        ai->TellPlayer(requester, "  [clear expired]  - Clear expired AI values");
        return true;
    }

#ifdef GenerateBotTests 
    if (param == "locations")
    {
        ai->TellPlayer(requester, "=== Test Locations ===");
        auto tests = TestRegistry::GetAvailableTests();
        ai->TellPlayer(requester, "Available tests: " + std::to_string(tests.size()));
        return true;
    }

    if (param.find("locate ") == 0)
    {
        std::string locationName = param.substr(7);
        GuidPosition loc;
        if (TestRegistry::ParseLocation(locationName, loc))
        {
            std::ostringstream out;
            out << locationName << " -> map=" << loc.mapid << " pos=(" << std::fixed << std::setprecision(2) << loc.coord_x << ", " << loc.coord_y << ", " << loc.coord_z << ")";
            ai->TellPlayer(requester, out.str());
        }
        else
        {
            ai->TellPlayer(requester, "Failed to resolve location: " + locationName);
        }
        return true;
    }
#endif

    return true;
}

bool DebugAction::HandleGPS(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;
    WorldPosition botPos(bot);
    out << bot->GetName() << ",";
    out << botPos.getAreaName() << ",";

    out << "\"node = sTravelNodeMap.addNode(WorldPosition(" << botPos.getMapId() << std::fixed << std::setprecision(2);
    out << ',' << botPos.coord_x << "f";
    out << ',' << botPos.coord_y << "f";
    out << ',' << botPos.coord_z << "f";
    out << "), \\\"c1" << botPos.getAreaName() << "\\\", true, false);\"";

    //node = sTravelNodeMap.addNode(WorldPosition(1, -3626.39f, 917.37f, 150.13f), "c1-Dire maul", true, false);

    if (!sPlayerbotAIConfig.isLogOpen("gps.csv"))
        sPlayerbotAIConfig.openLog("gps.csv", "a", true);
    sPlayerbotAIConfig.log("gps.csv", out.str().c_str());

    return true;
}

bool DebugAction::HandleSetValueUInt32(Event& event, Player* requester, const std::string& text)
{
    std::vector<std::string> args = Qualified::getMultiQualifiers(text.substr(14), ",");

    if (args.size() == 1)
    {
        RESET_AI_VALUE(uint32, args[0]);
        return true;
    }
    else if (args.size() == 2)
    {
        SET_AI_VALUE(uint32, args[0], stoi(args[1]));
        return true;
    }
    else if (args.size() == 3)
    {
        SET_AI_VALUE2(uint32, args[0], args[1], stoi(args[2]));
        return true;
    }

    return false;
}

bool DebugAction::HandleDo(Event& event, Player* requester, const std::string& text)
{
    return ai->DoSpecificAction(text.substr(3), Event(), true);
}

bool DebugAction::HandleTrade(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 6)
    {
        param = text.substr(6);
    }

    if (param.find("stop") == 0)
    {
        if (bot->GetTradeData())
            bot->TradeCancel(true);
    }
    return true;
}

bool DebugAction::HandleMail(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 5)
    {
        param = text.substr(6);
    }

    bool doAction = ai->DoSpecificAction("mail", Event("debug", param.empty() ? "?" : param), true);
    return doAction;
}

bool DebugAction::HandlePOI(Event& event, Player* requester, const std::string& text)
{        
    WorldPosition botPos = WorldPosition(bot);

    WorldPosition poiPoint = botPos;
    std::string name = "bot";

    std::vector<std::string> args = Qualified::getMultiQualifiers(text.substr(4), " ");

    DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, args[0]);
    TravelDestination* dest = nullptr;

    if (!dests.empty())
    {
        WorldPosition botPos(bot);

        dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });
    }

    if (dest)
    {
        std::list<uint8> chancesToGoFar = { 10,50,90 }; //Closest map, grid, cell.
        WorldPosition* point = dest->GetNextPoint(botPos, chancesToGoFar);
        if (point)
        {
            poiPoint = *point;
            name = dest->GetTitle();
        }
    }

    if (args.size() == 1)
    {
        args.push_back("99");
    }
    else if (args.size() == 2)
    {
        args.push_back("6");
    }
    else if (args.size() == 3)
    {
        args.push_back("0");
    }

    ai->Poi(poiPoint.coord_x, poiPoint.coord_y, name, nullptr, stoi(args[1]), stoi(args[2]), stoi(args[3]));

    return true;
}

bool DebugAction::HandleMotion(Event& event, Player* requester, const std::string& text)
{
    Unit* requesterTarget = nullptr;
    if (requester->GetSelectionGuid())
    {
        requesterTarget = ai->GetUnit(requester->GetSelectionGuid());
    }

    Unit* motionBot = bot;
    Unit* motionTarget = requesterTarget;

    if (text.find("motion") != 0)
    {
        if (requesterTarget)
        {
            motionBot = requesterTarget;
            motionTarget = requester;
        }
        else
        {
            motionBot = requester;
            motionTarget = bot;
        }
    }

    MotionMaster* mm = motionBot->GetMotionMaster();

    MovementGeneratorType type = mm->GetCurrentMovementGeneratorType();

    std::string sType = "TODO"; // GetMoveTypeStr(type);

    Unit* cTarget = sServerFacade.GetChaseTarget(motionBot);
    float cAngle = sServerFacade.GetChaseAngle(motionBot);
    float cOffset = sServerFacade.GetChaseOffset(motionBot);
    std::string cTargetName = cTarget ? cTarget->GetName() : "none";

    std::string motionName = motionBot->GetName();

    ai->TellPlayer(requester, motionName + " :" + sType + " (" + cTargetName + " a:" + std::to_string(cAngle) + " o:" + std::to_string(cOffset) + ")");

    if (!requesterTarget)
    {
        requesterTarget = requester;
    }

    if (text.size() > 7)
    {
        std::string cmd = text.substr(7);

        if (cmd == "clear")
            mm->Clear();
        else if (cmd == "reset")
            mm->Clear(true);
        else if (cmd == "clearall")
            mm->Clear(false, true);
        else if (cmd == "expire")
            mm->MovementExpired();
        else if (cmd == "flee")
            mm->MoveFleeing(motionTarget, 10);
        else if (cmd == "followmain")
            mm->MoveFollow(motionTarget, 5, 0, true, true);
        else if (cmd == "follow")
            mm->MoveFollow(motionTarget, 5, 0);
        else if (cmd == "dist")
            mm->DistanceYourself(10);
        else if (cmd == "update")
            mm->UpdateMotion(10);
        else if (cmd == "chase")
            mm->MoveChase(motionTarget, 5, 0);
        else if (cmd == "fall")
            mm->MoveFall();
        else if (cmd == "formation")
        {
            FormationSlotDataSPtr form = std::make_shared<FormationSlotData>(0, bot->GetObjectGuid(), nullptr, SpawnGroupFormationSlotType::SPAWN_GROUP_FORMATION_SLOT_TYPE_STATIC);
            mm->MoveInFormation(form);
        }

        std::string sType = "TODO"; // GetMoveTypeStr(type);
        ai->TellPlayer(requester, "new:" + sType);
    }
    return true;
}

bool DebugAction::HandleTransport(Event& event, Player* requester, const std::string& text)
{
    std::vector<GenericTransport*> transports;

    for (auto trans : WorldPosition(bot).getTransports())
        transports.push_back(trans);

    WorldPosition botPos(bot);

    //Closest transport last = below in chat.
    std::sort(transports.begin(), transports.end(), [botPos](GenericTransport* i, GenericTransport* j) { return botPos.distance(i) > botPos.distance(j); });

    for (auto trans : transports)
    {
        GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(trans->GetEntry());
        std::ostringstream out;

        std::string transportName = trans->GetName();
        if (transportName.empty())
            transportName = data->name;

        out << transportName << " (" << trans->GetEntry() << ")";

        if (bot->GetTransport() == trans)
            out << "[inside]";

        if (WorldPosition(bot).isOnTransport(trans))
        {
            out << "[ontop]";
        }

        out << " dist:" << std::fixed << std::setprecision(2) << botPos.distance(trans) << " offset: " << (botPos - trans).print(2, true);

        ai->TellPlayer(requester, out);
    }

    return true;
}

bool DebugAction::HandlePointOnTrans(Event& event, Player* requester, const std::string& text)
{

    uint32 radius = 20;
    std::string param;

    if (text.length() > std::string("pointontrans").size())
    {
        param = text.substr(std::string("pointontrans").size() + 1);
        radius = stoi(param);
    }

    //Get transport
    std::vector<GenericTransport*> transports;

    for (auto trans : WorldPosition(bot).getTransports())
        transports.push_back(trans);

    WorldPosition botPos(bot);

    std::sort(transports.begin(), transports.end(), [botPos](GenericTransport* i, GenericTransport* j) { return botPos.distance(i) > botPos.distance(j); });

    GenericTransport* transport = transports.back();

    GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(transport->GetEntry());
    std::string transportName = transport->GetName();
    if (transportName.empty())
        transportName = data->name;

    std::ostringstream out;
    out << "found: " << transportName << " (" << transport->GetEntry() << ")";
    ai->TellPlayer(requester, out);

    //Get point on transport and path.
    std::vector<WorldPosition> path;

    WorldPosition pointOnTrans = WorldPosition(bot).RandomPointOnTrans(transport, radius, bot, path);

    if (!pointOnTrans)
    {
        ai->TellPlayer(requester, "No points found.");
        return false;
    }

    WorldPosition transPos(transport);
    transPos.SetTranpotHeightToFloor(transport->GetEntry());

    out << "point at offset: " << (pointOnTrans - transPos).print(1, true);
    out << " path size: " << path.size();

    //Place WP at point.
    GenericTransport* botTrans = bot->GetTransport();
    bot->SetTransport(nullptr);

    Creature* wpCreature = bot->SummonCreature(2334, pointOnTrans.getX(), pointOnTrans.getY(), pointOnTrans.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
    transport->AddPassenger(wpCreature, true);
    wpCreature->NearTeleportTo(pointOnTrans.getX(), pointOnTrans.getY(), pointOnTrans.getZ(), wpCreature->GetOrientation());
    ai->AddAura(wpCreature, 246);
    bot->SetTransport(botTrans);

    //Check if we have good path.
    if (path.size())
    {
        WorldPosition pathEnd = path.back();
        pathEnd.CalculatePassengerPosition(transport);

        if (pathEnd.distance(pointOnTrans) < 1.0f)
        {
            out << " ON TRANS!!";
            ai->AddAura(wpCreature, 1130);
        }
    }

    ai->TellPlayer(requester, out);

    return true;
    /*
    uint32 radius = 20;

    for (float x = radius * -1.0f; x < radius; x += 1.0f)
    {
        for (float y = radius * -1.0f; y < radius; y += 1.0f)
        {
            if (x * x + y * y > radius * radius)
                continue;

            WorldPosition botPos(bot);
            WorldPosition transPos(transport);

            WorldPosition pos = transPos + WorldPosition(0, x, y);

            transPos.SetTranpotHeightToFloor(transport->GetEntry());

            bool onTrans = pos.SetOnTransport(transport, 0.5, -0.5);

            if (!onTrans)
                continue;

            Player* pathBot = bot;
            GenericTransport* botTrans = bot->GetTransport();

            bot->SetTransport(nullptr);

            Creature* wpCreature = bot->SummonCreature(2334, pos.getX(), pos.getY(), pos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
            transport->AddPassenger(wpCreature, true);
            wpCreature->NearTeleportTo(pos.getX(), pos.getY(), pos.getZ(), wpCreature->GetOrientation());
            ai->AddAura(wpCreature, 246);

            bot->SetTransport(transport);
            std::unique_ptr<PathFinder> pathfinder = std::make_unique<PathFinder>(bot, true);
            WorldPosition tStart = bot, tEnd = pos;
            tStart.CalculatePassengerOffset(transport);
            tEnd.CalculatePassengerOffset(transport);
            pathfinder->calculate(tStart.getVector3(), tEnd.getVector3(), false);

            if (!(pathfinder->getPathType() & PATHFIND_NORMAL))
                continue;

            std::vector<WorldPosition> path = pos.fromPointsArray(pathfinder->getPath());

            if (path.size() < 2 || path.front().distance(path[1]) > 10.0f)
                continue;

            ai->TellPlayer(ai->GetMaster(), "Height diff:" + std::to_string(pos.getZ() - transPos.getZ()));

            WorldPosition rp = path.back();
            rp.CalculatePassengerPosition(transport);

            if (rp.distance(pos) < 1.0f)
                ai->AddAura(wpCreature, 1130);

            bot->SetTransport(botTrans);
        }
    }

    return true;
    */
}

bool DebugAction::HandleDoTransport(Event& event, Player* requester, const std::string& text)
{
    std::string param;

    if (text.length() > std::string("dotrans").size())
        param = text.substr(std::string("dotrans").size() + 1);

    WorldPosition botPos(bot), transPos(bot);

    uint32 entry = 0;
    float distance = FLT_MAX;

    for (auto trans : botPos.getTransports())
    {
        float dist = botPos.distance(trans);

        if (dist < distance)
        {
            distance = dist;
            entry = trans->GetEntry();
            transPos = trans;
        }
    }

    MovementAction::UseTransport(ai, entry, transPos, requester, param.find("tele") != std::string::npos);

    return true;
}

bool DebugAction::HandleOnTrans(Event& event, Player* requester, const std::string& text)
{
    uint32 radius = 20;

    if (text.length() > std::string("ontrans").size())
    {
        radius = stoi(text.substr(std::string("ontrans").size() + 1));
    }

    WorldPosition botPos(bot);

    float distance = FLT_MAX;

    GenericTransport* transport = nullptr;
    for (auto trans : botPos.getTransports())
    {
        float dist = botPos.distance(trans);
        if (dist < distance)
        {
            transport = trans;
            distance = dist;
        }
    }

    if (!transport)
    {
        return false;
    }

    GenericTransport* botTrans = bot->GetTransport();

    std::vector<WorldPosition> path;

    WorldPosition transPos = botPos.RandomPointOnTrans(transport, 20.0f, bot, path);

    if (!transPos)
        return false;

    bot->SetTransport(botTrans);

    if (path.empty())
    {
        ai->TellPlayer(requester, "No point, trying to get near transport first.");
        path = WorldPosition(transport).getPathStepFrom(botPos, bot);

        if (path.empty())
            return false;
    }
    else
    {
        ai->TellPlayer(requester, "Found point on transport, trying to walk there.");
        transport->AddPassenger(bot, true);

        ai->StopMoving();

        if (!bot->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = bot->GetMotionMaster()->top())
                movgen->Interrupt(*bot);

        bot->SendHeartBeat();

        if (!bot->GetMotionMaster()->empty())
            if (MovementGenerator* movgen = bot->GetMotionMaster()->top())
                movgen->Reset(*bot);
    }

    if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        for (auto& p : path)
        {
            Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            transport->AddPassenger(wpCreature, true);

            wpCreature->NearTeleportTo(p.getX(), p.getY(), p.getZ(), wpCreature->GetOrientation());

            ai->AddAura(wpCreature, 246);

            if (p == path.back())
                ai->AddAura(wpCreature, 1130);
        }
    }

    bot->GetMotionMaster()->Clear();

    std::vector<G3D::Vector3> pointPath = transPos.toPointsArray(path);
#ifndef MANGOSBOT_TWO
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false, false);
#else
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false);
#endif  

    return true;
}

bool DebugAction::HandleOffTrans(Event& event, Player* requester, const std::string& text)
{
    if (!bot->GetTransport())
    {
        ai->TellPlayer(requester, "Bot not on transport.");
        return false;
    }

    GenericTransport* transport = bot->GetTransport();

    WorldPosition exitPos(requester);

    transport->RemovePassenger(bot);

    bot->NearTeleportTo(bot->m_movementInfo.pos.x, bot->m_movementInfo.pos.y, bot->m_movementInfo.pos.z, bot->m_movementInfo.pos.o);

    std::vector<WorldPosition> path = WorldPosition(bot).getPathStepFrom(exitPos, bot, false);

    if (path.empty())
    {
        ai->TellPlayer(requester, "No path to exitPos.");
        return false;
    }

    if (exitPos.sqDistance(path.back()) > 5.0f)
    {
        ai->TellPlayer(requester, "Path doesn't get to exit pos.");
        return false;
    }

    bot->GetMotionMaster()->Clear();

    std::vector<G3D::Vector3> pointPath = exitPos.toPointsArray(path);

#ifndef MANGOSBOT_TWO
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false, false);
#else
    bot->GetMotionMaster()->MovePath(pointPath, FORCED_MOVEMENT_RUN, false);
#endif  

    return true;
}

bool DebugAction::HandlePathable(Event& event, Player* requester, const std::string& text)
{
    uint32 radius = 10;

    if (text.length() > std::string("pathable").size())
        radius = stoi(text.substr(std::string("pathable").size() + 1));

    GenericTransport* transport = nullptr;
    for (auto trans : WorldPosition(bot).getTransports(bot->GetInstanceId()))
        if (!transport || WorldPosition(bot).distance(trans) < WorldPosition(bot).distance(transport))
            transport = trans;

    for (float x = radius * -1.0f; x < radius; x += 1.0f)
    {
        for (float y = radius * -1.0f; y < radius; y += 1.0f)
        {
            if (x * x + y * y > radius * radius)
                continue;

            WorldPosition botPos(bot);
            WorldPosition pos = botPos + WorldPosition(0, x, y, 3.0f);

            Player* pathBot = bot;
            GenericTransport* botTrans = bot->GetTransport();
            GenericTransport* trans = botTrans ? botTrans : transport;

            if (!pos.isOnTransport(trans)) //When trying to calculate a position off the transport, act like the bot is off the transport.
                pathBot = nullptr;
            else
                bot->SetTransport(trans);

            std::vector<WorldPosition> path = pos.getPathFrom(botPos, pathBot); //Use full pathstep to get proper paths on to transports.

            if (path.empty())
                continue;

            pos = path.back();

            bool onTrans = pos.isOnTransport(trans);

            if (onTrans) //Summon creature needs to be Summoned on offset coordinates.
            {
                pos.CalculatePassengerOffset(trans);
                bot->SetTransport(trans);
            }
            else //Generate wp off transport so it doesn't spawn on transport.
                bot->SetTransport(nullptr);

            Creature* wpCreature = bot->SummonCreature(2334, pos.getX(), pos.getY(), pos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
            ai->AddAura(wpCreature, 246);

            if (onTrans)
                ai->AddAura(wpCreature, 1130);

            bot->SetTransport(botTrans);
        }
    }

    return false;
}

bool DebugAction::HandleRandomSpot(Event& event, Player* requester, const std::string& text)
{
    uint32 radius = 10;
    if(text.length() > std::string("randomspot").size())
        radius = stoi(text.substr(std::string("randomspot").size()+1));

    WorldPosition botPos(bot);

    PathFinder pathfinder(bot);

    if (bot->GetTransport())
        botPos.CalculatePassengerOffset(bot->GetTransport());

    pathfinder.ComputePathToRandomPoint(botPos.getVector3(), radius);
    PointsArray points = pathfinder.getPath();
    std::vector<WorldPosition> path = botPos.fromPointsArray(points);

    if (path.empty())
        return false;
   
    Creature* wpCreature = bot->SummonCreature(6, path.back().getX(), path.back().getY(), path.back().getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
    wpCreature->SetObjectScale(0.5f);

    if (bot->GetTransport())
        bot->GetTransport()->AddPassenger(wpCreature,true);


    for (auto& p : path)
    {
        Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        ai->AddAura(wpCreature, 246);
        if (bot->GetTransport())
            bot->GetTransport()->AddPassenger(wpCreature,true);
    }
    return true;
}

bool DebugAction::HandlePrintMap(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.printMap();
    sTravelNodeMap.printNodeStore();
    return true;
}

bool DebugAction::HandleCorpse(Event& event, Player* requester, const std::string& text)
{

    Corpse* corpse = bot->GetCorpse();

    if (!corpse)
    {
        ai->TellPlayerNoFacing(requester, "no corpse");
        return true;
    }


    std::ostringstream out;

    if(corpse->GetType() == CORPSE_BONES)
        out << "CORPSE_BONES";

    if (corpse->GetType() == CORPSE_RESURRECTABLE_PVE)
        out << "CORPSE_RESURRECTABLE_PVE";

    if (corpse->GetType() == CORPSE_RESURRECTABLE_PVP)
        out << "CORPSE_RESURRECTABLE_PVP";

    out << ",p:" << WorldPosition(corpse).print();

    out << ",time: " << corpse->GetGhostTime();

    ai->TellPlayerNoFacing(requester, out);

    return true;
}

bool DebugAction::HandleLogoutTime(Event& event, Player* requester, const std::string& text)
{
    int32 time = sRandomPlayerbotMgr.GetValueValidTime(bot->GetGUIDLow(), "add");

    int32 min = 0, hr = 0;

    if (time > 3600)
    {
        hr = floor(time / 3600);
        time = time % 3600;
    }
    if (time > 60)
    {
        min = floor(time / 60);
        time = time % 60;
    }

    std::ostringstream out;

    out << "Logout in: " << hr << ":" << min << ":" << time;

    bot->Whisper(out.str().c_str(), LANG_UNIVERSAL, event.getOwner()->GetObjectGuid());

    return true;
}

bool DebugAction::HandleLevel(Event& event, Player* requester, const std::string& text)
{
    uint32 level = bot->GetLevel();
    uint32 nextLevelXp = bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP);
    uint32 xp = bot->GetUInt32Value(PLAYER_XP);
    float flevel = ai->GetLevelFloat();

    std::ostringstream out;

    out << "Level: " << level << ", xp:" << xp << "/" << nextLevelXp << " :" || flevel;

    bot->Whisper(out.str().c_str(), LANG_UNIVERSAL, event.getOwner()->GetObjectGuid());

    return true;
}

bool DebugAction::HandleQuest(Event& event, Player* requester, const std::string& text)
{
    std::string param = text.substr(5); // skip "quest"
    while (param.size() > 0 && param[0] == ' ')
        param = param.substr(1);

    std::string subcmd;
    std::string questParam;

    // Parse subcommand
    size_t spacePos = param.find(' ');
    if (spacePos != std::string::npos)
    {
        subcmd = param.substr(0, spacePos);
        questParam = param.substr(spacePos + 1);
        while (questParam.size() > 0 && questParam[0] == ' ')
            questParam = questParam.substr(1);
    }
    else
    {
        subcmd = param;
    }

    // Handle 'list' subcommand
    if (subcmd == "list")
    {
        std::ostringstream out;
        out << "=== Quest Log ===";
        int count = 0;
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = bot->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
            std::string name = pQuest ? pQuest->GetTitle() : "Unknown";
            QuestStatus status = bot->GetQuestStatus(questId);
            std::string statusStr = (status == QUEST_STATUS_COMPLETE) ? "[COMPLETE]" : "[INCOMPLETE]";
            
            out << "\n" << questId << ": " << statusStr << " " << name;
            count++;
        }
        if (count == 0)
            out << "\nNo quests in log";
        ai->TellPlayer(requester, out.str());
        return true;
    }

    // Parse quest ID - try both subcmd and questParam
    uint32 questId = 0;

    // First try questParam (if provided with subcommand like "complete 123")
    if (!questParam.empty())
    {
        try
        {
            questId = std::stoul(questParam);
        }
        catch (...)
        {
            if (questParam.find("|Hquest:") != std::string::npos)
            {
                size_t start = questParam.find(":") + 1;
                size_t end = questParam.find(":", start);
                if (end != std::string::npos)
                {
                    try
                    {
                        questId = std::stoul(questParam.substr(start, end - start));
                    }
                    catch (...) {}
                }
            }
        }
    }

    // If no questId yet, try subcmd directly (for "quest 745" syntax)
    if (!questId && !subcmd.empty())
    {
        try
        {
            questId = std::stoul(subcmd);
        }
        catch (...)
        {
            if (subcmd.find("|Hquest:") != std::string::npos)
            {
                size_t start = subcmd.find(":") + 1;
                size_t end = subcmd.find(":", start);
                if (end != std::string::npos)
                {
                    try
                    {
                        questId = std::stoul(subcmd.substr(start, end - start));
                    }
                    catch (...) {}
                }
            }
        }
    }

    // Handle 'complete' subcommand
    if (subcmd == "complete")
    {
        if (!questId)
        {
            ai->TellPlayer(requester, "Usage: quest complete <questId>");
            return true;
        }

        // Find the quest in bot's quest log
        bool found = false;
        bool completed = false;

        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuestId = bot->GetQuestSlotQuestId(slot);
            if (!logQuestId)
                continue;

            if (logQuestId != questId)
                continue;

            found = true;

            if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
            {
                completed = true;
                break;
            }

            Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
            if (!pQuest)
                break;

            // Add required items
            for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
            {
                uint32 id = pQuest->ReqItemId[x];
                uint32 count = pQuest->ReqItemCount[x];
                if (!id || !count)
                    continue;

                uint32 curItemCount = bot->GetItemCount(id, true);
                if (curItemCount >= count)
                    continue;

                ItemPosCountVec dest;
                uint8 msg = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
                if (msg == EQUIP_ERR_OK)
                {
                    Item* item = bot->StoreNewItem(dest, id, true);
                    bot->SendNewItem(item, count - curItemCount, true, false);
                }
            }

            // Complete kill objectives
            for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
            {
                int32 creature = pQuest->ReqCreatureOrGOId[i];
                uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

                if (uint32 spell_id = pQuest->ReqSpell[i])
                {
                    for (uint16 z = 0; z < creaturecount; ++z)
                        bot->CastedCreatureOrGO(creature, ObjectGuid((creature > 0 ? HIGHGUID_UNIT : HIGHGUID_GAMEOBJECT), uint32(std::abs(creature)), 1u), spell_id);
                }
                else if (creature > 0)
                {
                    if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature))
                        for (uint16 z = 0; z < creaturecount; ++z)
                            bot->KilledMonster(cInfo, nullptr);
                }
                else if (creature < 0)
                {
                    for (uint16 z = 0; z < creaturecount; ++z)
                        bot->CastedCreatureOrGO(-creature, ObjectGuid(), 0);
                }
            }

            // Complete reputation
            if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
            {
                uint32 repValue = pQuest->GetRepObjectiveValue();
                uint32 curRep = bot->GetReputationMgr().GetReputation(repFaction);
                if (curRep < repValue)
                {
#ifndef MANGOSBOT_ONE
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
#else
                    if (FactionEntry const* factionEntry = sFactionStore.LookupEntry<FactionEntry>(repFaction))
#endif
                        bot->GetReputationMgr().SetReputation(factionEntry, repValue);
                }
            }

            // Complete money requirement
            int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
            if (ReqOrRewMoney < 0)
                bot->ModifyMoney(-ReqOrRewMoney);

            bot->CompleteQuest(questId);
            completed = true;
            break;
        }

        if (completed)
        {
            std::ostringstream out;
            out << "Quest " << questId << " completed!";
            ai->TellPlayer(requester, out.str());
        }
        else if (found)
        {
            std::ostringstream out;
            out << "Quest " << questId << " already completed.";
            ai->TellPlayer(requester, out.str());
        }
        else
        {
            std::ostringstream out;
            out << "Quest " << questId << " not found in quest log.";
            ai->TellPlayer(requester, out.str());
        }
        return true;
    }

    // Handle 'drop' subcommand (abandon quest)
    if (subcmd == "drop")
    {
        if (!questId)
        {
            ai->TellPlayer(requester, "Usage: quest drop <questId>");
            return true;
        }

        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuestId = bot->GetQuestSlotQuestId(slot);
            if (!logQuestId || logQuestId != questId)
                continue;

            bot->SetQuestSlot(slot, 0);
            bot->SetQuestSlotState(slot, QUEST_STATE_NONE);
            
            std::ostringstream out;
            out << "Quest " << questId << " dropped.";
            ai->TellPlayer(requester, out.str());
            return true;
        }

        std::ostringstream out;
        out << "Quest " << questId << " not found in quest log.";
        ai->TellPlayer(requester, out.str());
        return true;
    }

    // Handle 'add' subcommand (add quest to log)
    if (subcmd == "add")
    {
        if (!questId)
        {
            ai->TellPlayer(requester, "Usage: quest add <questId>");
            return true;
        }

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
        if (!pQuest)
        {
            std::ostringstream out;
            out << "Quest " << questId << " not found in database.";
            ai->TellPlayer(requester, out.str());
            return true;
        }

        // Find empty slot
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            if (bot->GetQuestSlotQuestId(slot))
                continue;

            bot->SetQuestSlot(slot, questId);
            bot->SetQuestSlotState(slot, QUEST_STATE_NONE);
            
            std::ostringstream out;
            out << "Quest " << questId << " (" << pQuest->GetTitle() << ") added.";
            ai->TellPlayer(requester, out.str());
            return true;
        }

        ai->TellPlayer(requester, "Quest log is full");
        return true;
    }

    // Handle 'travel' subcommand (show travel info for quest)
    if (subcmd == "travel")
    {
        if (!questId)
        {
            ai->TellPlayer(requester, "Usage: quest travel <questId>");
            return true;
        }

        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
        if (!pQuest)
        {
            std::ostringstream out;
            out << "Quest " << questId << " not found in database.";
            ai->TellPlayer(requester, out.str());
            return true;
        }

        std::ostringstream out;
        out << "=== Quest: " << questId << " ===";
        out << "\n" << pQuest->GetTitle();
        
        // Show objectives
        out << "\n--- Objectives ---";
        for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
        {
            int32 creature = pQuest->ReqCreatureOrGOId[i];
            uint32 count = pQuest->ReqCreatureOrGOCount[i];
            if (!creature || !count)
                continue;

            std::string targetType = (creature > 0) ? "Kill" : "Collect";
            out << "\n" << targetType << " " << abs(creature) << " x" << count;
        }
        for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
        {
            uint32 item = pQuest->ReqItemId[x];
            uint32 count = pQuest->ReqItemCount[x];
            if (!item || !count)
                continue;

            out << "\nCollect item " << item << " x" << count;
        }

        ai->TellPlayer(requester, out.str());
        return true;
    }

    // Handle 'info' or just quest ID - show info about quest
    if (!subcmd.empty() && questId)
    {
        Quest const* pQuest = sObjectMgr.GetQuestTemplate(questId);
        if (!pQuest)
        {
            std::ostringstream out;
            out << "Quest " << questId << " not found in database.";
            ai->TellPlayer(requester, out.str());
            return true;
        }

        // Check if in quest log
        bool inLog = false;
        QuestStatus status = QUEST_STATUS_NONE;
        for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            if (bot->GetQuestSlotQuestId(slot) == questId)
            {
                inLog = true;
                status = bot->GetQuestStatus(questId);
                break;
            }
        }

        std::ostringstream out;
        out << "=== Quest: " << questId << " ===";
        out << "\n" << pQuest->GetTitle();
        out << "\nLevel: " << pQuest->GetQuestLevel();
        out << "\nStatus: ";
        if (!inLog)
            out << "NOT IN LOG";
        else if (status == QUEST_STATUS_COMPLETE)
            out << "COMPLETE";
        else
            out << "IN PROGRESS";

        ai->TellPlayer(requester, out.str());
        return true;
    }

    // No valid subcommand - show usage
    ai->TellPlayer(requester, "Usage: quest <complete|drop|add|list|travel> [questId]");
    return true;
}

PositionTarget DebugAction::ParseLocation(const std::string& param, Player* bot)
{
    PositionTarget result;
    
    if (param.empty())
        return result;

    std::istringstream iss(param);
    std::vector<float> numbers;
    float num;
    while (iss >> num)
    {
        numbers.push_back(num);
    }

    bool isAllNumbers = numbers.size() > 0 && param.find_first_not_of("0123456789.-+ ") == std::string::npos;
    
    if (isAllNumbers && numbers.size() <= 4)
    {
        result.valid = true;
        if (numbers.size() == 2)
        {
            result.mapId = bot->GetMapId();
            result.x = numbers[0];
            result.y = numbers[1];
            result.z = bot->GetPositionZ();
        }
        else if (numbers.size() == 3)
        {
            result.mapId = (uint32)numbers[0];
            result.x = numbers[1];
            result.y = numbers[2];
            result.z = bot->GetPositionZ();
        }
        else if (numbers.size() == 4)
        {
            result.mapId = (uint32)numbers[0];
            result.x = numbers[1];
            result.y = numbers[2];
            result.z = numbers[3];
        }
        return result;
    }

    GameTele const* tele = sObjectMgr.GetGameTele(param);
    if (tele)
    {
        result.valid = true;
        result.mapId = tele->mapId;
        result.x = tele->position_x;
        result.y = tele->position_y;
        result.z = tele->position_z;
        result.name = param;
        return result;
    }

    return result;
}

bool DebugAction::HandlePosition(Event& event, Player* requester, const std::string& text)
{
    std::string param = text.substr(8); // skip "position"
    while (param.size() > 0 && param[0] == ' ')
        param = param.substr(1);

    // Handle subcommands
    if (param.substr(0, 4) == "zone")
    {
        std::string zoneParam = param.size() > 4 ? param.substr(4) : "";
        while (zoneParam.size() > 0 && zoneParam[0] == ' ')
            zoneParam = zoneParam.substr(1);

        uint32 mapId = bot->GetMapId();
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();

        if (!zoneParam.empty())
        {
            PositionTarget target = ParseLocation(zoneParam, bot);
            if (target.valid)
            {
                mapId = target.mapId;
                x = target.x;
                y = target.y;
            }
            else
            {
                ai->TellPlayer(requester, "Usage: position zone [x y | map x y | location]");
                return true;
            }
        }

        uint32 areaId = sTerrainMgr.GetAreaId(mapId, x, y, bot->GetPositionZ());
        const AreaTableEntry* area = GetAreaEntryByAreaID(areaId);
        std::ostringstream out;
        out << "Zone: " << areaId;
        if (area)
        {
            out << " (" << area->area_name[0] << ")";
            if (area->zone)
            {
                const AreaTableEntry* zone = GetAreaEntryByAreaID(area->zone);
                if (zone)
                    out << " Zone: " << zone->area_name[0];
            }
        }
        ai->TellPlayer(requester, out.str());
        return true;
    }

    // Check travel nodes near bot or location
    if (param.substr(0, 5) == "nodes")
    {
        std::string nodeParam = param.size() > 5 ? param.substr(5) : "";
        while (nodeParam.size() > 0 && nodeParam[0] == ' ')
            nodeParam = nodeParam.substr(1);

        WorldPosition pos(bot);
        if (!nodeParam.empty())
        {
            PositionTarget target = ParseLocation(nodeParam, bot);
            if (target.valid)
            {
                pos = WorldPosition(target.mapId, target.x, target.y, target.z, 0);
            }
        }

        std::vector<TravelNode*> nodes = sTravelNodeMap.getNodes(pos);
        std::ostringstream out;
        out << "Travel nodes near " << pos.getAreaName() << " (" << uint32(pos.getX()) << "," << uint32(pos.getY()) << "," << uint32(pos.getZ()) << "): " << nodes.size();
        
        if (nodes.size() > 0)
        {
            out << " - closest: " << uint32(nodes[0]->getDistance(pos)) << "y";
            if (nodes.size() > 1)
                out << ", 2nd: " << uint32(nodes[1]->getDistance(pos)) << "y";
        }
        ai->TellPlayer(requester, out.str());
        return true;
    }

    if (param.substr(0, 8) == "teleport")
    {
        std::string teleParam = param.substr(8);
        while (teleParam.size() > 0 && teleParam[0] == ' ')
            teleParam = teleParam.substr(1);

        if (teleParam.empty())
        {
            Player* master = ai->GetMaster();
            if (master)
            {
                bot->TeleportTo(master->GetMapId(), master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(), master->GetOrientation());
                ai->TellPlayer(requester, "Teleported to master");
            }
            else
            {
                ai->TellPlayer(requester, "No master found");
            }
            return true;
        }

        PositionTarget target = ParseLocation(teleParam, bot);
        if (target.valid)
        {
            bot->TeleportTo(target.mapId, target.x, target.y, target.z, bot->GetOrientation());
            std::ostringstream out;
            if (!target.name.empty())
                out << "Teleported to " << target.name;
            else if (target.mapId == bot->GetMapId())
                out << "Teleported to " << target.x << ", " << target.y << " on map " << target.mapId;
            else
                out << "Teleported to map " << target.mapId << " at " << target.x << ", " << target.y << ", " << target.z;
            ai->TellPlayer(requester, out.str());
            return true;
        }

        ai->TellPlayer(requester, "Usage: position teleport [x y | map x y | map x y z | location]");
        return true;
    }

    if (param.substr(0, 8) == "distance" || param.substr(0, 4) == "dist")
    {
        std::string distParam = param.size() > 8 ? param.substr(8) : (param.size() > 4 ? param.substr(4) : "");
        while (distParam.size() > 0 && distParam[0] == ' ')
            distParam = distParam.substr(1);

        if (distParam.empty())
        {
            Player* master = ai->GetMaster();
            if (master)
            {
                float dist = bot->GetDistance(master);
                std::ostringstream out;
                out << "Distance to master: " << dist << " yards";
                ai->TellPlayer(requester, out.str());
            }
            else
            {
                ai->TellPlayer(requester, "No master found");
            }
            return true;
        }

        PositionTarget target = ParseLocation(distParam, bot);
        if (target.valid)
        {
            float dx = bot->GetPositionX() - target.x;
            float dy = bot->GetPositionY() - target.y;
            float dz = bot->GetPositionZ() - target.z;
            float dist = sqrt(dx*dx + dy*dy + dz*dz);
            std::ostringstream out;
            out << "Distance to " << (target.name.empty() ? std::string("position") : target.name) << ": " << dist << " yards";
            ai->TellPlayer(requester, out.str());
            return true;
        }

        ai->TellPlayer(requester, "Usage: position distance [x y | map x y | map x y z | location]");
        return true;
    }

    if (param.substr(0, 4) == "path")
    {
        std::string pathParam = param.size() > 4 ? param.substr(4) : "";
        while (pathParam.size() > 0 && pathParam[0] == ' ')
            pathParam = pathParam.substr(1);

        if (pathParam.empty())
        {
            LastMovement& moveData = *ai->GetAiObjectContext()->GetValue<LastMovement&>("last movement");
            std::ostringstream out;
            out << "Last movement: ";
            WorldPosition().printWKT(moveData.lastPath.getPointPath(), out, 1);
            ai->TellPlayer(requester, out.str());
            return true;
        }

        std::string targetParam;
        WorldPosition startPos(bot);
        
        size_t pipePos = pathParam.find('|');
        if (pipePos == std::string::npos)
            pipePos = pathParam.find("->");
        
        bool showFull = false;
        if (pipePos != std::string::npos)
        {
            std::string startStr = pathParam.substr(0, pipePos);
            targetParam = pathParam.substr(pipePos + 1);
            while (targetParam.size() > 0 && targetParam[0] == ' ')
                targetParam = targetParam.substr(1);
            
            if (targetParam.find("full") != std::string::npos)
            {
                showFull = true;
                size_t fullPos = targetParam.find("full");
                targetParam = targetParam.substr(0, fullPos);
                while (targetParam.size() > 0 && targetParam[targetParam.size() - 1] == ' ')
                    targetParam = targetParam.substr(0, targetParam.size() - 1);
            }
            
            PositionTarget startTarget = ParseLocation(startStr, bot);
            if (startTarget.valid)
            {
                startPos = WorldPosition(startTarget.mapId, startTarget.x, startTarget.y, startTarget.z);
            }
            else
            {
                ai->TellPlayer(requester, "Usage: position path <from>|<to>");
                return true;
            }
        }
        else
        {
            targetParam = pathParam;
        }

        PositionTarget target = ParseLocation(targetParam, bot);
        if (target.valid)
        {
            WorldPosition endPos(target.mapId, target.x, target.y, target.z);
            TravelPath fullPath = sTravelNodeMap.getFullPath(startPos, endPos, bot);
            
            if (fullPath.empty())
            {
                ai->TellPlayer(requester, "No path found");
                return true;
            }

            float totalDist = 0.0f;
            float walkDist = 0.0f, flightDist = 0.0f, transportDist = 0.0f, teleportCount = 0;
            uint32 prevMapId = startPos.getMapId();
            std::set<uint32> maps;
            maps.insert(prevMapId);
            WorldPosition prevPos = startPos;
            bool debugMove = ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT);
            
            std::ostringstream out;
            out << "Path (" << fullPath.getPath().size() << " points):";
            ai->TellPlayer(requester, out.str());
            
            if (showFull)
            {
                for (size_t i = 0; i < fullPath.getPath().size(); i++)
                {
                    PathNodePoint& p = fullPath.getPath()[i];
                    float dist = prevPos.distance(p.point);
                    totalDist += dist;
                    
                    if (p.point.getMapId() != prevMapId)
                    {
                        maps.insert(p.point.getMapId());
                    }
                    
                    switch (p.type)
                    {
                        case PathNodeType::NODE_PATH:
                        case PathNodeType::NODE_PREPATH:
                            walkDist += dist;
                            break;
                        case PathNodeType::NODE_FLIGHTPATH:
                            flightDist += dist;
                            break;
                        case PathNodeType::NODE_TRANSPORT:
                            transportDist += dist;
                            break;
                        case PathNodeType::NODE_TELEPORT:
                        case PathNodeType::NODE_STATIC_PORTAL:
                            teleportCount++;
                            break;
                        default:
                            break;
                    }
                    
                    std::ostringstream ptOut;
                    ptOut << "  " << i << ". " << p.point.getX() << "," << p.point.getY() << "," << p.point.getZ() 
                          << " m" << p.point.getMapId() << " type:" << (int)p.type;
                    ai->TellPlayer(requester, ptOut.str());
                    
                    prevPos = p.point;
                    prevMapId = p.point.getMapId();
                }
            }
            else
            {
                for (size_t i = 0; i < fullPath.getPath().size(); i++)
                {
                    PathNodePoint& p = fullPath.getPath()[i];
                    float dist = prevPos.distance(p.point);
                    totalDist += dist;
                    
                    if (p.point.getMapId() != prevMapId)
                    {
                        maps.insert(p.point.getMapId());
                    }
                    
                    switch (p.type)
                    {
                        case PathNodeType::NODE_PATH:
                        case PathNodeType::NODE_PREPATH:
                            walkDist += dist;
                            break;
                        case PathNodeType::NODE_FLIGHTPATH:
                            flightDist += dist;
                            break;
                        case PathNodeType::NODE_TRANSPORT:
                            transportDist += dist;
                            break;
                        case PathNodeType::NODE_TELEPORT:
                        case PathNodeType::NODE_STATIC_PORTAL:
                            teleportCount++;
                            break;
                        default:
                            break;
                    }
                    
                    if (debugMove && i < 20)
                    {
                        bot->SummonCreature(2334, p.point.getX(), p.point.getY(), p.point.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 20000.0f);
                    }
                    
                    prevPos = p.point;
                    prevMapId = p.point.getMapId();
                }
            }
            
            if (debugMove && !fullPath.getPath().empty())
            {
                bot->SummonCreature(6, fullPath.getFront().getX(), fullPath.getFront().getY(), fullPath.getFront().getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 30000.0f);
            }

            std::ostringstream summary;
            summary << "Total: " << totalDist << " yards";
            ai->TellPlayer(requester, summary.str());
            
            std::ostringstream breakdown;
            breakdown << "Walk: " << walkDist << "y, Fly: " << flightDist << "y, Transport: " << transportDist << "y, Teleport: " << teleportCount << "x";
            ai->TellPlayer(requester, breakdown.str());
            
            if (maps.size() > 1)
            {
                std::ostringstream mapInfo;
                mapInfo << "Maps: ";
                for (auto m : maps)
                    mapInfo << m << " ";
                ai->TellPlayer(requester, mapInfo.str());
            }
            
            std::ostringstream next;
            next << "Next Point: " << fullPath.getFront().getX() << " " << fullPath.getFront().getY() << " " << fullPath.getFront().getZ();
            ai->TellPlayer(requester, next.str());
            return true;
        }

        ai->TellPlayer(requester, "Usage: position path [<from> |] <to> [full]");
        return true;
    }

    if (param.substr(0, 5) == "route")
    {
        std::string routeParam = param.size() > 5 ? param.substr(5) : "";
        while (routeParam.size() > 0 && routeParam[0] == ' ')
            routeParam = routeParam.substr(1);

        if (routeParam.empty())
        {
            ai->TellPlayer(requester, "Usage: position route <from> | <to>");
            return true;
        }

        std::string targetParam;
        WorldPosition startPos(bot);
        
        size_t pipePos = routeParam.find('|');
        if (pipePos == std::string::npos)
            pipePos = routeParam.find("->");
        
        if (pipePos != std::string::npos)
        {
            std::string startStr = routeParam.substr(0, pipePos);
            targetParam = routeParam.substr(pipePos + 1);
            while (targetParam.size() > 0 && targetParam[0] == ' ')
                targetParam = targetParam.substr(1);
            
            PositionTarget startTarget = ParseLocation(startStr, bot);
            if (startTarget.valid)
            {
                startPos = WorldPosition(startTarget.mapId, startTarget.x, startTarget.y, startTarget.z);
            }
            else
            {
                ai->TellPlayer(requester, "Usage: position route <from>|<to>");
                return true;
            }
        }
        else
        {
            targetParam = routeParam;
        }

        PositionTarget target = ParseLocation(targetParam, bot);
        if (target.valid)
        {
            WorldPosition endPos(target.mapId, target.x, target.y, target.z);
            
            std::vector<WorldPosition> beginPath, endPath;
            TravelNodeRoute route = sTravelNodeMap.getRoute(startPos, endPos, beginPath, endPath, bot);
            
            if (route.isEmpty())
            {
                ai->TellPlayer(requester, "No route found");
                return true;
            }

            std::ostringstream out;
            out << "Route (" << route.getNodes().size() << " nodes):";
            ai->TellPlayer(requester, out.str());
            
            float totalDist = 0.0f;
            WorldPosition prevPos = startPos;
            bool debugMove = ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT);
            
            for (size_t i = 0; i < route.getNodes().size(); i++)
            {
                TravelNode* node = route.getNodes()[i];
                WorldPosition nodePos = *node->getPosition();
                float dist = prevPos.distance(nodePos);
                totalDist += dist;
                
                uint32 areaId = sTerrainMgr.GetAreaId(nodePos.getMapId(), nodePos.getX(), nodePos.getY(), nodePos.getZ());
                const AreaTableEntry* area = GetAreaEntryByAreaID(areaId);
                std::string areaName = "Unknown";
                if (area)
                {
                    areaName = area->area_name[0];
                }
                
                std::ostringstream nodeOut;
                nodeOut << "  " << (i + 1) << ". " << node->getName() << " [" << areaName << ", m" << nodePos.getMapId() << "] (" << dist << " yd)";
                ai->TellPlayer(requester, nodeOut.str());
                
                if (debugMove)
                {
                    bot->SummonCreature(2334, nodePos.getX(), nodePos.getY(), nodePos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 20000.0f);
                }
                
                prevPos = nodePos;
            }
            
            if (debugMove && !route.getNodes().empty())
            {
                bot->SummonCreature(6, route.getNodes().front()->getPosition()->getX(), 
                    route.getNodes().front()->getPosition()->getY(), 
                    route.getNodes().front()->getPosition()->getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 30000.0f);
            }

            std::ostringstream summary;
            summary << "Total: " << totalDist << " yards";
            ai->TellPlayer(requester, summary.str());
            return true;
        }

        ai->TellPlayer(requester, "Usage: position route <from> | <to>");
        return true;
    }

    if (param.substr(0, 5) == "water")
    {
        std::string waterParam = param.size() > 5 ? param.substr(5) : "";
        while (waterParam.size() > 0 && waterParam[0] == ' ')
            waterParam = waterParam.substr(1);

        float x, y, z;
        uint32 mapId;

        if (!waterParam.empty())
        {
            PositionTarget target = ParseLocation(waterParam, bot);
            if (target.valid)
            {
                x = target.x;
                y = target.y;
                z = target.z;
                mapId = target.mapId;
            }
            else
            {
                ai->TellPlayer(requester, "Usage: position water [x y | map x y | map x y z | location]");
                return true;
            }
        }
        else
        {
            x = bot->GetPositionX();
            y = bot->GetPositionY();
            z = bot->GetPositionZ();
            mapId = bot->GetMapId();
        }

        Map* map = sMapMgr.FindMap(mapId);
        if (!map)
        {
            ai->TellPlayer(requester, "Map not found");
            return true;
        }

        const TerrainInfo* terrain = map->GetTerrain();
        if (!terrain)
        {
            ai->TellPlayer(requester, "Terrain not loaded");
            return true;
        }

        float groundLevel = 0.0f;
        float waterLevel = terrain->GetWaterLevel(x, y, z, &groundLevel);

        std::ostringstream out;
        out << "Pos: " << x << "," << y << "," << z << " m" << mapId;
        ai->TellPlayer(requester, out.str());

        std::ostringstream groundOut;
        groundOut << "Ground: " << groundLevel;
        ai->TellPlayer(requester, groundOut.str());

        std::ostringstream waterOut;
        waterOut << "Water: " << waterLevel;
        ai->TellPlayer(requester, waterOut.str());

        if (waterLevel > groundLevel)
        {
            float waterDepth = waterLevel - groundLevel;
            if (z >= waterLevel)
            {
                ai->TellPlayer(requester, "State: Above water surface");
            }
            else if (z >= groundLevel)
            {
                std::ostringstream inWater;
                inWater << "State: In water (depth: " << waterDepth << " yards)";
                ai->TellPlayer(requester, inWater.str());
            }
            else
            {
                ai->TellPlayer(requester, "State: Underwater (swimming/flying below surface)");
            }
        }
        else
        {
            ai->TellPlayer(requester, "State: Not in water (no water at this location)");
        }

        if (z < groundLevel - 0.5f)
        {
            float underGround = groundLevel - z;
            std::ostringstream underground;
            underground << "Underground: Yes (" << underGround << " yards below ground)";
            ai->TellPlayer(requester, underground.str());
        }
        else
        {
            ai->TellPlayer(requester, "Underground: No");
        }

        return true;
    }

    // If param is empty, show bot's position
    if (param.empty())
    {
        std::ostringstream out;
        out << bot->GetPositionX() << " " << bot->GetPositionY() << " " << bot->GetPositionZ() << " " << bot->GetMapId() << " " << bot->GetOrientation();
        uint32 area = sServerFacade.GetAreaId(bot);
        if (const AreaTableEntry* areaEntry = GetAreaEntryByAreaID(area))
        {
            if (AreaTableEntry const* zoneEntry = areaEntry->zone ? GetAreaEntryByAreaID(areaEntry->zone) : areaEntry)
                out << " |" << zoneEntry->area_name[0] << "|";
        }
        ai->TellPlayer(requester, out.str());
        return true;
    }

    // Try to find player by name
    if (Player* target = sObjectMgr.GetPlayer(param.c_str()))
    {
        std::ostringstream out;
        out << target->GetName() << ": " << target->GetPositionX() << " " << target->GetPositionY() << " " << target->GetPositionZ() << " " << target->GetMapId();
        ai->TellPlayer(requester, out.str());
        return true;
    }

    // Try to find creature by name (search nearby)
    std::list<Unit*> targets;
    AnyUnitInObjectRangeCheck u_check(bot, 100000.0f);
    UnitListSearcher<AnyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(bot, searcher, 100000.0f);
    
    for (Unit* u : targets)
    {
        if (u && u->GetTypeId() == TYPEID_UNIT)
        {
            Creature* c = (Creature*)u;
            if (c->GetName() == param)
            {
                std::ostringstream out;
                out << c->GetName() << ": " << c->GetPositionX() << " " << c->GetPositionY() << " " << c->GetPositionZ() << " " << c->GetMapId();
                ai->TellPlayer(requester, out.str());
                return true;
            }
        }
    }

    // Try parsing as guid or link
    uint32 guid = 0;
    try { guid = std::stoul(param); } catch (...) {}

    if (guid > 0)
    {
        if (GameObject* go = bot->GetMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, guid)))
        {
            std::ostringstream out;
            out << "GO " << guid << ": " << go->GetPositionX() << " " << go->GetPositionY() << " " << go->GetPositionZ() << " " << go->GetMapId();
            ai->TellPlayer(requester, out.str());
            return true;
        }
        if (Unit* u = bot->GetMap()->GetUnit(ObjectGuid(HIGHGUID_UNIT, guid)))
        {
            std::ostringstream out;
            out << "Unit " << guid << ": " << u->GetPositionX() << " " << u->GetPositionY() << " " << u->GetPositionZ() << " " << u->GetMapId();
            ai->TellPlayer(requester, out.str());
            return true;
        }
    }

    ai->TellPlayer(requester, "Usage: position [zone|teleport|distance|path|<name/guid>]");
    return true;
}

bool DebugAction::HandleNPC(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    GuidPosition guidP = GuidPosition(requester->GetSelectionGuid(), requester);

    if (text.size() > 4)
    {
        std::string link = text.substr(4);

        if (!link.empty())
        {
            std::list<int32> entries = chat->parseWorldEntries(link);
            if (!entries.empty())
            {

                int32 entry = entries.front();

                for (auto cre : WorldPosition(bot).getCreaturesNear(0.0f, entry))
                {
                    guidP = GuidPosition(cre);
                    break;
                }
            }
        }
    }

    if (!guidP)
        return false;

    if (guidP.GetWorldObject(bot->GetInstanceId()))
        out << chat->formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId()));
    
    out << " (e:" << guidP.GetEntry();
    
    if (guidP.GetUnit(bot->GetInstanceId()))
        out << ",level:" << guidP.GetUnit(bot->GetInstanceId())->GetLevel();
        
    out << ") ";

    guidP.printWKT(out);

    out << "[a:" << guidP.GetArea()->area_name[0]; 

    if (guidP.GetArea() && guidP.getAreaLevel())
        out << " level: " << guidP.getAreaLevel();
    if (guidP.GetArea()->zone && GetAreaEntryByAreaID(guidP.GetArea()->zone))
    {
        out << " z:" << GetAreaEntryByAreaID(guidP.GetArea()->zone)->area_name[0];
        if (sTravelMgr.GetAreaLevel(guidP.GetArea()->zone))
            out << " level: " << sTravelMgr.GetAreaLevel(guidP.GetArea()->zone);
    }

    out << "] ";

    uint16 event_id = sGameEventMgr.GetGameEventId<Creature>(guidP.GetCounter());

    if (event_id)
        out << " event:" << event_id << (sGameEventMgr.IsActiveEvent(event_id) ? " active" : " inactive");

    uint16 topPoolId = sPoolMgr.IsPartOfTopPool<Creature>(guidP.GetCounter());

    if (topPoolId)
        out << " pool:" << topPoolId << (sGameEventMgr.GetGameEventId<Pool>(topPoolId) ? " event" : " nonevent");

    ai->TellPlayerNoFacing(requester, out);

    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_GOSSIP))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_GOSSIP");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_QUESTGIVER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_QUESTGIVER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_VENDOR))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_VENDOR");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_FLIGHTMASTER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_FLIGHTMASTER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_TRAINER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_TRAINER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_SPIRITHEALER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_SPIRITHEALER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_SPIRITGUIDE))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_SPIRITGUIDE");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_INNKEEPER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_INNKEEPER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_BANKER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_BANKER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_PETITIONER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_PETITIONER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_TABARDDESIGNER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_TABARDDESIGNER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_BATTLEMASTER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_BATTLEMASTER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_AUCTIONEER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_AUCTIONEER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_STABLEMASTER))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_STABLEMASTER");
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_REPAIR))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_REPAIR");
#ifdef MANGOSBOT_ZERO
    if (guidP.HasNpcFlag(UNIT_NPC_FLAG_OUTDOORPVP))
        ai->TellPlayerNoFacing(requester, "UNIT_NPC_FLAG_OUTDOORPVP");
#endif


    std::ostringstream out2;
    FactionTemplateEntry const* requestFaction = sFactionTemplateStore.LookupEntry(requester->GetFaction());
    FactionTemplateEntry const* objectFaction = nullptr;
    if(guidP.GetCreatureTemplate() && guidP.GetCreatureTemplate()->Faction)
        objectFaction = sFactionTemplateStore.LookupEntry(guidP.GetCreatureTemplate()->Faction);
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    std::map<ReputationRank, std::string> rep;

    rep[REP_HATED] = "hated";
    rep[REP_HOSTILE] = "hostile";
    rep[REP_UNFRIENDLY] = "unfriendly";
    rep[REP_NEUTRAL] = "neutral";
    rep[REP_FRIENDLY] = "friendly";
    rep[REP_HONORED] = "honored";
    rep[REP_REVERED] = "revered";
    rep[REP_EXALTED] = "exalted";

    if (objectFaction)
    {
        ReputationRank reactionRequest = PlayerbotAI::GetFactionReaction(requestFaction, objectFaction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, objectFaction);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, objectFaction);

        out2 << " faction:" << guidP.GetCreatureTemplate()->Faction << " reaction me: " << rep[reactionRequest] << ",alliance: " << rep[reactionHum] << " ,horde: " << rep[reactionOrc];
    }

    ai->TellPlayerNoFacing(requester, out2);

    return true;
}

bool DebugAction::HandleGO(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 4)
        return false;

    GuidPosition guidP;

    std::string link = text.substr(3);

    if (!link.empty())
    {
        std::list<ObjectGuid> gos = chat->parseGameobjects(link);
        if (!gos.empty())
        {
            for (auto go : gos)
            {
                guidP = GuidPosition(go, bot);
                break;
            }
        }
    }

    if (!guidP)
        return false;

    if (!guidP.IsGameObject())
        return false;

    if (guidP.GetWorldObject(bot->GetInstanceId()))
        out << chat->formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId()));

    out << " (e:" << guidP.GetEntry();

    if (guidP.GetUnit(bot->GetInstanceId()))
        out << ",level:" << guidP.GetUnit(bot->GetInstanceId())->GetLevel();

    out << ") ";

    guidP.printWKT(out);


    ai->TellPlayerNoFacing(requester, out);

    std::ostringstream out2;
    
    FactionTemplateEntry const* requestFaction = sFactionTemplateStore.LookupEntry(requester->GetFaction());
    FactionTemplateEntry const* objectFaction = sFactionTemplateStore.LookupEntry(guidP.GetGameObjectInfo()->faction);
    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    std::map<ReputationRank, std::string> rep;

    rep[REP_HATED] = "hated";
    rep[REP_HOSTILE] = "hostile";
    rep[REP_UNFRIENDLY] = "unfriendly";
    rep[REP_NEUTRAL] = "neutral";
    rep[REP_FRIENDLY] = "friendly";
    rep[REP_HONORED] = "honored";
    rep[REP_REVERED] = "revered";
    rep[REP_EXALTED] = "exalted";

    if (objectFaction)
    {
        ReputationRank reactionRequest = PlayerbotAI::GetFactionReaction(requestFaction, objectFaction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, objectFaction);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, objectFaction);

        out2 << " faction:" << guidP.GetGameObjectInfo()->faction << " reaction me: " << rep[reactionRequest] << ",alliance: " << rep[reactionHum] << " ,horde: " << rep[reactionOrc];
    }

    ai->TellPlayerNoFacing(requester, out2);

    std::unordered_map<uint32, std::string>  types;
    types[GAMEOBJECT_TYPE_DOOR] = "GAMEOBJECT_TYPE_DOOR";
    types[GAMEOBJECT_TYPE_BUTTON] = "GAMEOBJECT_TYPE_BUTTON";
    types[GAMEOBJECT_TYPE_QUESTGIVER] = "GAMEOBJECT_TYPE_QUESTGIVER";
    types[GAMEOBJECT_TYPE_CHEST] = "GAMEOBJECT_TYPE_CHEST";
    types[GAMEOBJECT_TYPE_BINDER] = "GAMEOBJECT_TYPE_BINDER";
    types[GAMEOBJECT_TYPE_GENERIC] = "GAMEOBJECT_TYPE_GENERIC";
    types[GAMEOBJECT_TYPE_TRAP] = "GAMEOBJECT_TYPE_TRAP";
    types[GAMEOBJECT_TYPE_CHAIR] = "GAMEOBJECT_TYPE_CHAIR";
    types[GAMEOBJECT_TYPE_SPELL_FOCUS] = "GAMEOBJECT_TYPE_SPELL_FOCUS";
    types[GAMEOBJECT_TYPE_TEXT] = "GAMEOBJECT_TYPE_TEXT";
    types[GAMEOBJECT_TYPE_GOOBER] = "GAMEOBJECT_TYPE_GOOBER";
    types[GAMEOBJECT_TYPE_TRANSPORT] = "GAMEOBJECT_TYPE_TRANSPORT";
    types[GAMEOBJECT_TYPE_AREADAMAGE] = "GAMEOBJECT_TYPE_AREADAMAGE";
    types[GAMEOBJECT_TYPE_CAMERA] = "GAMEOBJECT_TYPE_CAMERA";
    types[GAMEOBJECT_TYPE_MAP_OBJECT] = "GAMEOBJECT_TYPE_MAP_OBJECT";
    types[GAMEOBJECT_TYPE_MO_TRANSPORT] = "GAMEOBJECT_TYPE_MO_TRANSPORT";
    types[GAMEOBJECT_TYPE_DUEL_ARBITER] = "GAMEOBJECT_TYPE_DUEL_ARBITER";
    types[GAMEOBJECT_TYPE_FISHINGNODE] = "GAMEOBJECT_TYPE_FISHINGNODE";
    types[GAMEOBJECT_TYPE_SUMMONING_RITUAL] = "GAMEOBJECT_TYPE_SUMMONING_RITUAL";
    types[GAMEOBJECT_TYPE_MAILBOX] = "GAMEOBJECT_TYPE_MAILBOX";
#ifndef MANGOSBOT_TWO
    types[GAMEOBJECT_TYPE_AUCTIONHOUSE] = "GAMEOBJECT_TYPE_AUCTIONHOUSE";
#endif
    types[GAMEOBJECT_TYPE_GUARDPOST] = "GAMEOBJECT_TYPE_GUARDPOST";
    types[GAMEOBJECT_TYPE_SPELLCASTER] = "GAMEOBJECT_TYPE_SPELLCASTER";
    types[GAMEOBJECT_TYPE_MEETINGSTONE] = "GAMEOBJECT_TYPE_MEETINGSTONE";
    types[GAMEOBJECT_TYPE_FLAGSTAND] = "GAMEOBJECT_TYPE_FLAGSTAND";
    types[GAMEOBJECT_TYPE_FISHINGHOLE] = "GAMEOBJECT_TYPE_FISHINGHOLE";
    types[GAMEOBJECT_TYPE_FLAGDROP] = "GAMEOBJECT_TYPE_FLAGDROP";
    types[GAMEOBJECT_TYPE_MINI_GAME] = "GAMEOBJECT_TYPE_MINI_GAME";
#ifndef MANGOSBOT_TWO
    types[GAMEOBJECT_TYPE_LOTTERY_KIOSK] = "GAMEOBJECT_TYPE_LOTTERY_KIOSK";
#endif
    types[GAMEOBJECT_TYPE_CAPTURE_POINT] = "GAMEOBJECT_TYPE_CAPTURE_POINT";
    types[GAMEOBJECT_TYPE_AURA_GENERATOR] = "GAMEOBJECT_TYPE_AURA_GENERATOR";
#ifdef MANGOSBOT_TWO
    types[GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY] = "GAMEOBJECT_TYPE_DUNGEON_DIFFICULTY";
    types[GAMEOBJECT_TYPE_BARBER_CHAIR] = "GAMEOBJECT_TYPE_BARBER_CHAIR";
    types[GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING] = "GAMEOBJECT_TYPE_DESTRUCTIBLE_BUILDING";
    types[GAMEOBJECT_TYPE_GUILD_BANK] = "GAMEOBJECT_TYPE_GUILD_BANK";
    types[GAMEOBJECT_TYPE_TRAPDOOR] = "GAMEOBJECT_TYPE_TRAPDOOR";
#endif

    ai->TellPlayerNoFacing(requester, types[guidP.GetGameObjectInfo()->type]);

    if (guidP.GetGameObject(bot->GetInstanceId()))
    {
        GameObject* object = guidP.GetGameObject(bot->GetInstanceId());

        GOState state = object->GetGoState();

        std::ostringstream out;

        out << "state:";

        if (state == GO_STATE_ACTIVE)
            out << "GO_STATE_ACTIVE";
        if (state == GO_STATE_READY)
            out << "GO_STATE_READY";
        if (state == GO_STATE_ACTIVE_ALTERNATIVE)
            out << "GO_STATE_ACTIVE_ALTERNATIVE";

        out << (object->IsInUse() ? ", in use" : ", not in use");

        LootState lootState = object->GetLootState();

        out << " lootState:";

        if (lootState == GO_NOT_READY)
            out << "GO_NOT_READY";
        if (lootState == GO_READY)
            out << "GO_READY";
        if (lootState == GO_ACTIVATED)
            out << "GO_ACTIVATED";
        if (lootState == GO_JUST_DEACTIVATED)
            out << "GO_JUST_DEACTIVATED";

        ai->TellPlayerNoFacing(requester, out);
    }

    return true;
}

bool DebugAction::HandleFind(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() <= 5)
        return false;

    std::string link = text.substr(5);

    if (link.empty())
        return false;

    if (!Qualified::isValidNumberString(link))
        return false;

    uint32 entry = stoi(link);

    Creature* creature = nullptr;
    MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(*bot, entry, true, false, 1000.0f, true);
    MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(creature, creature_check);
    Cell::VisitGridObjects(bot, searcher, 1000.0f);

    if (!creature)
        return false;

    out << "Found: ";
    out << chat->formatWorldobject(creature);
    out << " at distance ";
    out << bot->GetDistance(creature);

    ai->TellPlayerNoFacing(requester, out);

    return true;
}

bool DebugAction::HandleItem(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 4)
        return false;

    GuidPosition guidP;

    std::string link = text.substr(3);

    if (link.empty())
        return false;

        ItemIds items = chat->parseItems(link);
        if (items.empty())
            return false;

    uint32 itemId = *items.begin();

    Item* item = bot->GetItemByEntry(itemId);

    ItemQualifier qualifier;
    ItemPrototype const* proto;
    if (item)
    {
        qualifier = ItemQualifier(item);
        proto = item->GetProto();
    }
    else
    {
        qualifier = ItemQualifier(itemId);
        proto = qualifier.GetProto();
    }


    if (item)
        out << chat->formatItem(item, item->GetCount());
    else
        out << chat->formatItem(qualifier);

    out << " (id:" << itemId;

    out << ",reqlev:" << proto->RequiredLevel << " ilev:" << proto->ItemLevel;

    out << ") ";
    

    ai->TellPlayerNoFacing(requester, out);

    std::ostringstream out2;

    // Print class and subclass string equivalents
    static const std::unordered_map<uint32, std::string> itemClassNames = {
        { ITEM_CLASS_CONSUMABLE, "Consumable" },
        { ITEM_CLASS_CONTAINER, "Container" },
        { ITEM_CLASS_WEAPON, "Weapon" },
        { ITEM_CLASS_GEM, "Gem" },
        { ITEM_CLASS_ARMOR, "Armor" },
        { ITEM_CLASS_REAGENT, "Reagent" },
        { ITEM_CLASS_PROJECTILE, "Projectile" },
        { ITEM_CLASS_TRADE_GOODS, "Trade Goods" },
        { ITEM_CLASS_GENERIC, "Generic" },
        { ITEM_CLASS_RECIPE, "Recipe" },
        { ITEM_CLASS_MONEY, "Money" },
        { ITEM_CLASS_QUIVER, "Quiver" },
        { ITEM_CLASS_QUEST, "Quest" },
        { ITEM_CLASS_KEY, "Key" },
        { ITEM_CLASS_PERMANENT, "Permanent" },
        { ITEM_CLASS_MISC, "Miscellaneous" },
#ifdef MANGOSBOT_TWO
        { ITEM_CLASS_GLYPH, "Glyph" }
#endif
    };

    static const std::unordered_map<uint32, std::unordered_map<uint32, std::string>> itemSubclassNames = {
        { ITEM_CLASS_CONSUMABLE, {
            { 0, "Consumable" }, { 1, "Potion" }, { 2, "Elixir" }, { 3, "Flask" }, { 4, "Scroll" }, { 5, "Food" },
            { 6, "Item Enhancement" }, { 7, "Bandage" }, { 8, "Other" }
        }},
        { ITEM_CLASS_CONTAINER, {
            { 0, "Container" }, { 1, "Soul Container" }, { 2, "Herb Container" }, { 3, "Enchanting Container" },
            { 4, "Engineering Container" }, { 5, "Gem Container" }, { 6, "Mining Container" },
            { 7, "Leatherworking Container" }, { 8, "Inscription Container" }
        }},
        { ITEM_CLASS_WEAPON, {
            { 0, "Axe (1H)" }, { 1, "Axe (2H)" }, { 2, "Bow" }, { 3, "Gun" }, { 4, "Mace (1H)" }, { 5, "Mace (2H)" },
            { 6, "Polearm" }, { 7, "Sword (1H)" }, { 8, "Sword (2H)" }, { 9, "Obsolete" }, { 10, "Staff" },
            { 11, "Exotic (1H)" }, { 12, "Exotic (2H)" }, { 13, "Fist Weapon" }, { 14, "Miscellaneous" },
            { 15, "Dagger" }, { 16, "Thrown" }, { 17, "Spear" }, { 18, "Crossbow" }, { 19, "Wand" }, { 20, "Fishing Pole" }
        }},
        { ITEM_CLASS_GEM, {
            { 0, "Red" }, { 1, "Blue" }, { 2, "Yellow" }, { 3, "Purple" }, { 4, "Green" }, { 5, "Orange" },
            { 6, "Meta" }, { 7, "Simple" }, { 8, "Prismatic" }
        }},
        { ITEM_CLASS_ARMOR, {
            { 0, "Miscellaneous" }, { 1, "Cloth" }, { 2, "Leather" }, { 3, "Mail" }, { 4, "Plate" }, { 5, "Buckler" },
            { 6, "Shield" }, { 7, "Libram" }, { 8, "Idol" }, { 9, "Totem" }, { 10, "Sigil" }
        }},
        { ITEM_CLASS_REAGENT, {
            { 0, "Reagent" }
        }},
        { ITEM_CLASS_PROJECTILE, {
            { 0, "Wand" }, { 1, "Bolt" }, { 2, "Arrow" }, { 3, "Bullet" }, { 4, "Thrown" }
        }},
        { ITEM_CLASS_TRADE_GOODS, {
            { 0, "Trade Goods" }, { 1, "Parts" }, { 2, "Explosives" }, { 3, "Devices" }, { 4, "Jewelcrafting" },
            { 5, "Cloth" }, { 6, "Leather" }, { 7, "Metal & Stone" }, { 8, "Meat" }, { 9, "Herb" },
            { 10, "Elemental" }, { 11, "Other" }, { 12, "Enchanting" }, { 13, "Material" },
            { 14, "Armor Enchantment" }, { 15, "Weapon Enchantment" }
        }},
        { ITEM_CLASS_GENERIC, {
            { 0, "Generic" }
        }},
        { ITEM_CLASS_RECIPE, {
            { 0, "Book" }, { 1, "Leatherworking Pattern" }, { 2, "Tailoring Pattern" }, { 3, "Engineering Schematic" },
            { 4, "Blacksmithing" }, { 5, "Cooking Recipe" }, { 6, "Alchemy Recipe" }, { 7, "First Aid Manual" },
            { 8, "Enchanting Formula" }, { 9, "Fishing Manual" }, { 10, "Jewelcrafting Recipe" }
        }},
        { ITEM_CLASS_MONEY, {
            { 0, "Money" }
        }},
        { ITEM_CLASS_QUIVER, {
            { 0, "Quiver0" }, { 1, "Quiver1" }, { 2, "Quiver" }, { 3, "Ammo Pouch" }
        }},
        { ITEM_CLASS_QUEST, {
            { 0, "Quest" }
        }},
        { ITEM_CLASS_KEY, {
            { 0, "Key" }, { 1, "Lockpick" }
        }},
        { ITEM_CLASS_PERMANENT, {
            { 0, "Permanent" }
        }},
        { ITEM_CLASS_MISC, {
            { 0, "Junk" }, { 1, "Reagent" }, { 2, "Pet" }, { 3, "Holiday" }, { 4, "Other" }, { 5, "Mount" }
        }},
#ifdef MANGOSBOT_TWO
        { ITEM_CLASS_GLYPH, {
            { 1, "Glyph Warrior" }, { 2, "Glyph Paladin" }, { 3, "Glyph Hunter" }, { 4, "Glyph Rogue" },
            { 5, "Glyph Priest" }, { 6, "Glyph Death Knight" }, { 7, "Glyph Shaman" }, { 8, "Glyph Mage" },
            { 9, "Glyph Warlock" }, { 11, "Glyph Druid" }
        }}
#endif
    };



    out2 << "class: ";
    auto classIt = itemClassNames.find(proto->Class);
    if (classIt != itemClassNames.end())
        out2 << classIt->second;
    else
        out2 << "Unknown(" << proto->Class << ")";

    out2 << ", subclass: ";
    auto subclassMapIt = itemSubclassNames.find(proto->Class);
    if (subclassMapIt != itemSubclassNames.end())
    {
        auto subIt = subclassMapIt->second.find(proto->SubClass);
        if (subIt != subclassMapIt->second.end())
            out2 << subIt->second;
        else
            out2 << "Unknown(" << proto->SubClass << ")";
    }
    else
    {
        out2 << "Unknown(" << proto->SubClass << ")";
    }

    ai->TellPlayerNoFacing(requester, out2);

    std::ostringstream out3;

    //Print the items contained inside item.


    ai->TellPlayerNoFacing(requester, out3);
    
    return true;
}

bool DebugAction::HandleRPG(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 4)
        return false;

    GuidPosition guidP;

    std::string link = text.substr(3);

    if (!link.empty())
    {
        std::list<ObjectGuid> gos = chat->parseGameobjects(link);
        if (!gos.empty())
        {
            for (auto go : gos)
            {
                guidP = GuidPosition(go,bot);
                guidP.updatePosition(bot->GetInstanceId());

                if(guidP)
                    break;
            }
        }
    }

    if (!guidP)
        return false;

    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    if (guidP.GetWorldObject(bot->GetInstanceId()))
        ai->TellPlayerNoFacing(requester, "Setting Rpg target to " + ChatHelper::formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId())));

    return true;
}

bool DebugAction::HandleRPGTargets(Event& event, Player* requester, const std::string& text)
{
    Action* action = ai->GetAiObjectContext()->GetAction("choose rpg target");

    if (!action)
        return false;

    ChooseRpgTargetAction* targetAction = static_cast<ChooseRpgTargetAction*>(action);

    if (!targetAction)
        return false;

    targetAction->GetTargets(requester, true);

    return true;
}

bool DebugAction::HandleTravel(Event& event, Player* requester, const std::string& text)
{
    WorldPosition botPos = WorldPosition(bot);

    std::string destination = text.substr(7);

    DestinationList dests = ChooseTravelTargetAction::FindDestination(bot, destination);
    TravelDestination* dest = nullptr;

    if (!dests.empty())
    {
        WorldPosition botPos(bot);

        dest = *std::min_element(dests.begin(), dests.end(), [botPos](TravelDestination* i, TravelDestination* j) {return i->DistanceTo(botPos) < j->DistanceTo(botPos); });
    }

    if (dest)
    {
        std::list<uint8> chancesToGoFar = { 10,50,90 }; //Closest map, grid, cell.
        WorldPosition* point = dest->GetNextPoint(botPos, chancesToGoFar);

        if (!point)
            return false;

        std::vector<WorldPosition> beginPath, endPath;
        TravelNodeRoute route = sTravelNodeMap.getRoute(botPos, *point, beginPath, endPath, bot);

        std::ostringstream out; out << "Traveling to " << dest->GetTitle() << ": ";

        for (auto node : route.getNodes())
        {
            out << node->getName() << ", ";
        }

        ai->TellPlayerNoFacing(requester, out.str());

        return true;
    }
    else
    {
        ai->TellPlayerNoFacing(requester, "Destination " + destination + " not found.");
        return true;
    }
}

bool DebugAction::HandlePrintTravel(Event& event, Player* requester, const std::string& text)
{
    if (!sPlayerbotAIConfig.isLogOpen("travel.csv"))
        sPlayerbotAIConfig.openLog("travel.csv", "w", true);

    if (!sPlayerbotAIConfig.isLogOpen("travelActive.csv"))
        sPlayerbotAIConfig.openLog("travelActive.csv", "w", true);

    PlayerTravelInfo info(bot);

    std::vector<std::type_index> types = { typeid(QuestTravelDestination), typeid(RpgTravelDestination), typeid(ExploreTravelDestination), typeid(GrindTravelDestination), typeid(BossTravelDestination), typeid(GatherTravelDestination) };

    for (auto& type : types)
    {
        for (auto& dest : sTravelMgr.GetDestinations(info, (uint32)TravelDestinationPurpose::None, {}, true, 0))
        {
            bool isPossible = dest->IsPossible(info);
            bool isActive = dest->IsActive(bot, info);
            for (auto& point : dest->GetPoints())
            {
                GuidPosition* guidP = dynamic_cast<GuidPosition*>(point);

                if (guidP && guidP->IsEventUnspawned()) //Skip points that are not spawned due to events.
                {
                    continue;
                }

                std::ostringstream out;

                out << type.name() << ",";
                out << (uint32)((EntryTravelDestination*)dest)->GetPurpose() << ",";
                out << "\"";
                if (type == typeid(QuestTravelDestination))
                {
                    uint32 questId = ((QuestTravelDestination*)dest)->GetQuestId();
                    Quest const* quest = sObjectMgr.GetQuestTemplate(questId);
                    out << quest->GetTitle();
                }
                else if (type != typeid(ExploreTravelDestination))
                {
                    if (((EntryTravelDestination*)dest)->GetCreatureInfo())
                        out << ((EntryTravelDestination*)dest)->GetCreatureInfo()->Name;
                    else if (((EntryTravelDestination*)dest)->GetGoInfo())
                        out << ((EntryTravelDestination*)dest)->GetGoInfo()->name;
                    else
                        out << "";
                }
                else
                {
                    out << dest->GetTitle();
                }
                out << "\",";
                out << std::to_string(dest->GetEntry()) << ",";
                out << "\"" << dest->GetTitle() << "\",";
                out << isPossible << ",";
                out << isActive << ",";
                point->printWKT(out);
                out << point->getAreaLevel();

                //sPlayerbotAIConfig.log("travel.csv", out.str().c_str());
                if(isActive)
                    sPlayerbotAIConfig.log("travelActive.csv", out.str().c_str());
            }

            if (dest->GetPoints().empty())
                sLog.outError("Destination %s has no points!", dest->GetTitle().c_str());
        }
    }
    return true;
}

bool DebugAction::HandleValues(Event& event, Player* requester, const std::string& text)
{
    //    ai->TellPlayerNoFacing(requester, ai->GetAiObjectContext()->FormatValues(text.substr(7)));

    std::string param = "";
    if (text.length() > 6)
    {
        param = text.substr(7);
    }

    std::set<std::string> names = context->GetValues();
    std::vector<std::string> values;
    for (auto name : names)
    {
        UntypedValue* value = context->GetUntypedValue(name);
        if (!value)
        {
            continue;
        }

        if (!param.empty() && name.find(param) == std::string::npos)
        {
            continue;
        }

        std::string text = value->Format();
        if (text == "?")
        {
            continue;
        }

        values.push_back(name + "=" + text);
    }

    std::string valuestring = sPlayerbotHelpMgr.makeList(values, "[<part>]");

    std::vector<std::string> lines = Qualified::getMultiQualifiers(valuestring, "\n");
    for (auto& line : lines)
    {
        ai->TellPlayerNoFacing(requester, line, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
    }

    return true;
}

bool DebugAction::HandleLoot(Event& event, Player* requester, const std::string& text)
{
    std::string param = "";
    if (text.length() > 6)
        param = text.substr(5);

    bool doAction = ai->DoSpecificAction("add all loot", Event("debug", param), true);

    if (doAction)
        ai->TellPlayerNoFacing(requester, "Added new loot");
    else
        ai->TellPlayerNoFacing(requester, "No new loot");

    LootObjectStack* loots = AI_VALUE(LootObjectStack*, "available loot");

    for (int i = 0; i < 2000; i++)
    {
        LootObject loot = loots->GetLoot();

        if (loot.IsEmpty())
            break;

        WorldObject* wo = ai->GetWorldObject(loot.guid);

        if (wo)
            ai->TellPlayerNoFacing(requester, chat->formatWorldobject(wo) + " " + (loot.IsLootPossible(bot) ? "can loot" : "can not loot"));
        else
            ai->TellPlayerNoFacing(requester,
                                   std::to_string(loot.guid) + " " + (loot.IsLootPossible(bot) ? "can loot" : "can not loot") + " " + std::to_string(loot.guid.GetEntry()));

        if (loot.guid.IsGameObject())
        {
            GameObject* go = ai->GetGameObject(loot.guid);

            if (go->ActivateToQuest(bot))
                ai->TellPlayerNoFacing(requester, std::to_string(go->GetGoType()) + " for quest");
            else
                ai->TellPlayerNoFacing(requester, std::to_string(go->GetGoType()));
        }

        loots->Remove(loot.guid);
    }

    ai->DoSpecificAction("add all loot", Event(), true);

    return true;
}

bool DebugAction::HandleDrops(Event& event, Player* requester, const std::string& text)
{
    std::string textSubstr;
    std::ostringstream out;
    for (auto entry : chat->parseWorldEntries(textSubstr = text.substr(6)))
    {
        std::list<uint32> itemIds = GAI_VALUE2(std::list<uint32>, "entry loot list", entry);

        if (itemIds.empty())
            out << chat->formatWorldEntry(entry) << " no drops found.";
        else
            out << chat->formatWorldEntry(entry) << " " << std::to_string(itemIds.size()) << " drops found:";

        ai->TellPlayerNoFacing(requester, out);
        out.str("");
        out.clear();

        std::vector<std::pair<uint32, float>> chances;

        for (auto itemId : itemIds)
        {
            std::vector<std::string> qualifiers = { std::to_string(entry) , std::to_string(itemId) };
            std::string qualifier = Qualified::MultiQualify(qualifiers, " ");
            float chance = GAI_VALUE2(float, "loot chance", qualifier);
            if (chance > 0 && sObjectMgr.GetItemPrototype(itemId))
            {
                chances.push_back(std::make_pair(itemId, chance));
            }
        }

        std::sort(chances.begin(), chances.end(), [](std::pair<uint32, float> i, std::pair<int32, float> j) {return i.second > j.second; });

        chances.resize(std::min(20, (int)chances.size()));

        for (auto chance : chances)
        {
            out << chat->formatItem(sObjectMgr.GetItemPrototype(chance.first), 0, 0) << ": " << chance.second << "%";
            ai->TellPlayerNoFacing(requester, out);
            out.str("");
            out.clear();
        }
    }
    return true;
}

bool DebugAction::HandleTaxi(Event& event, Player* requester, const std::string& text)
{
    for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
    {
        if (!bot->m_taxi.IsTaximaskNodeKnown(i))
            continue;

        TaxiNodesEntry const* taxiNode = sTaxiNodesStore.LookupEntry(i);

        std::ostringstream out;

        out << taxiNode->name[0];

        ai->TellPlayerNoFacing(requester, out);
    }
    return true;
}

bool DebugAction::HandlePrice(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    if (text.size() < 7)
        return false;

    std::string link = text.substr(6);
    ItemIds ids = ChatHelper::parseItems(link);

    for (auto id : ids)
    {
        std::ostringstream out;

        const ItemPrototype* proto = sObjectMgr.GetItemPrototype(id);
        if (!proto)
            continue;

        out << ChatHelper::formatItem(proto);
        out << " Buy from vendor: " << ChatHelper::formatMoney(proto->BuyPrice);
        out << " Sell to vendor: " << ChatHelper::formatMoney(proto->SellPrice);
        out << " Median buyout from AH (price per item): " << ChatHelper::formatMoney(ItemUsageValue::GetAHMedianBuyoutPricePerItem(proto));
        out << " Lowest AH listing buyout (price per item): " << ChatHelper::formatMoney(ItemUsageValue::GetAHListingLowestBuyoutPricePerItem(proto));
        out << " Sell to AH: " << ChatHelper::formatMoney(ItemUsageValue::GetBotAHSellMinPrice(proto)) << " to " << ChatHelper::formatMoney(ItemUsageValue::GetBotAHSellMaxPrice(proto));

        ai->TellPlayerNoFacing(requester, out);
    }

    return true;
}

bool DebugAction::HandleNC(Event& event, Player* requester, const std::string& text)
{
    std::ostringstream out;

    std::string name;
    if (text.size() > 3)
        name = text.substr(3);

    std::list<TriggerNode*> triggerNodes;
    std::set<std::string> strategies;
    context->GetSupportedStrategies(strategies);
    for (auto& strategyName : strategies)
    {
        if (!name.empty() && strategyName.find(name) == std::string::npos)
            continue;

        Strategy* strategy = context->GetStrategy(strategyName);

        ai->TellPlayerNoFacing(requester, "s:" + strategyName + (ai->HasStrategy(strategyName, BotState::BOT_STATE_NON_COMBAT) ? " [active]" : ""), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);

        strategy->InitTriggers(triggerNodes, BotState::BOT_STATE_NON_COMBAT);

        //Loop over all triggers of this strategy.
        for (auto& triggerNode : triggerNodes)
        {
            Trigger* trigger = context->GetTrigger(triggerNode->getName());

            if (trigger)
            {
                ai->TellPlayerNoFacing(requester, "t: " + triggerNode->getName() + (trigger->IsActive() ? " [active]" : ""), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);
                triggerNode->setTrigger(trigger);

                Trigger* trigger = triggerNode->getTrigger();

                NextAction** nextActions = triggerNode->getHandlers();

                bool isRpg = false;

                //Loop over all actions triggered by this trigger and check if any is an 'rpg action'.
                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                {
                    NextAction* nextAction = nextActions[i];

                    Action* action = ai->GetAiObjectContext()->GetAction(nextAction->getName());

                    if (action)
                    {
                        std::ostringstream out;
                        out << "a:  ";
                        out << nextAction->getName();
                        out << triggerNode->getName();
                        out << (action->isUseful() ? " [usefull]" : "");
                        out << (action->isPossible() ? " [possible]" : "");
                        out << "(";
                        out << std::setprecision(3);
                        out << nextAction->getRelevance();
                        out << ")";

                        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);
                    }
                    else
                    {
                        std::ostringstream out;
                        out << "a:  [unknown]";
                        out << nextAction->getName();
                        out << triggerNode->getName();
                        out << " (";
                        out << std::setprecision(3);
                        out << nextAction->getRelevance();
                        out << ")";

                        ai->TellPlayerNoFacing(requester, out.str(), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false, false);
                    }
                    
                }
                NextAction::destroy(nextActions);
            }
        }

        for (std::list<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
        {
            TriggerNode* trigger = *i;
            delete trigger;
        }

        triggerNodes.clear();
    }        

    return true;
}

bool DebugAction::HandleAddNode(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);

    std::string name = "USER:" + text.substr(9);

    TravelNode* startNode = sTravelNodeMap.addNode(pos, name, false, false);

    for (auto& endNode : sTravelNodeMap.getNodes(pos, 2000))
    {
        endNode->setLinked(false);
    }

    ai->TellPlayerNoFacing(requester, "Node " + name + " created.");
    
    sTravelNodeMap.setHasToGen();

    return true;
}

bool DebugAction::HandleRemNode(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);

    TravelNode* startNode = sTravelNodeMap.getNode(pos, nullptr, 50);

    if (!startNode)
        return false;

    if (startNode->isImportant())
    {
        ai->TellPlayerNoFacing(requester, "Node can not be removed.");
    }
    sTravelNodeMap.m_nMapMtx.lock();
    sTravelNodeMap.removeNode(startNode);
    ai->TellPlayerNoFacing(requester, "Node removed.");
    sTravelNodeMap.m_nMapMtx.unlock();

    sTravelNodeMap.setHasToGen();

    return true;
}

bool DebugAction::HandleResetNode(Event& event, Player* requester, const std::string& text)
{
    for (auto& node : sTravelNodeMap.getNodes())
        node->setLinked(false);
    return true;
}

bool DebugAction::HandleResetPath(Event& event, Player* requester, const std::string& text)
{
    for (auto& node : sTravelNodeMap.getNodes())
        for (auto& path : *node->getLinks())
            node->removeLinkTo(path.first, true);
    return true;
}

bool DebugAction::HandleGenNode(Event& event, Player* requester, const std::string& text)
{
    //Pathfinder
    sTravelNodeMap.generateNodes();
    return true;
}

bool DebugAction::HandleGenPath(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.generatePaths(false);
    return true;
}

bool DebugAction::HandleCropPath(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.removeUselessPaths();
    return true;
}

bool DebugAction::HandleSaveNode(Event& event, Player* requester, const std::string& text)
{
    sTravelNodeMap.printNodeStore();
    sTravelNodeMap.saveNodeStore(true);
    return true;
}

bool DebugAction::HandleLoadNode(Event& event, Player* requester, const std::string& text)
{
    std::thread t([] {if (sTravelNodeMap.removeNodes())
        sTravelNodeMap.loadNodeStore(); });

    t.detach();

    return true;
}

bool DebugAction::HandleShowNode(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);

    std::vector<TravelNode*> nodes = sTravelNodeMap.getNodes(pos, 500);

    for (auto& node : nodes)
    {
        for (auto& l : *node->getLinks())
        {
            Unit* start = nullptr;
            std::list<ObjectGuid> units;

            uint32 time = 60 * IN_MILLISECONDS;

            std::vector<WorldPosition> ppath = l.second->getPath();

            for (auto p : ppath)
            {
                Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 20000.0f);
                //addAura(246, wpCreature);
                units.push_back(wpCreature->GetObjectGuid());

                if(!start)
                    ai->AddAura(wpCreature, 1130);
                else
                    ai->AddAura(wpCreature, 246);
                    

                if (!start)
                    start = wpCreature;
            }
        }
    }
    return true;
}

bool DebugAction::HandleDSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    Unit* prev = bot;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);
        WorldPosition botPos1 = botPos;

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

        FakeSpell(spellEffect, wpCreature, wpCreature, prev->GetObjectGuid(), {}, {}, botPos, botPos);

        prev = wpCreature;
    }
    return true;
}

bool DebugAction::HandleVSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    Unit* prev = bot;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);
        WorldPosition botPos1 = botPos;

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);            

        if (wpCreature)
        {
            WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4);        // visual effect on guid
            data << wpCreature->GetObjectGuid();
            data << uint32(spellEffect);                               // index from SpellVisualKit.dbc
            wpCreature->SendMessageToSet(data, true);                
        }
    }
    return true;
}

bool DebugAction::HandleASpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    Unit* prev = bot;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);
        WorldPosition botPos1 = botPos;

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 5000.0f + i * 100.0f);
        wpCreature->SetObjectScale(0.5f);

        if (wpCreature)
        {
            addAura(spellEffect, wpCreature);
        }
    }
    return true;
}

bool DebugAction::HandleCSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));

    std::list<ObjectGuid> units;

    for (float i = 0; i < 60; i++)
    {
        float ang = i / 60 * M_PI_F * 4;
        float dist = i / 60 * 30;

        WorldPosition botPos(bot);

        botPos.setX(botPos.getX() + cos(ang) * dist);
        botPos.setY(botPos.getY() + sin(ang) * dist);
        botPos.setZ(botPos.getHeight(bot->GetInstanceId()) + 2);

        Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        units.push_back(wpCreature->GetObjectGuid());
    }

    WorldPosition botPos(bot);

    if(urand(0,1))
        FakeSpell(spellEffect, bot, bot, ObjectGuid(), units, {}, botPos, botPos);
    else
        FakeSpell(spellEffect, bot, bot, units.front(), units, {}, botPos, botPos);

    return true;
}

bool DebugAction::HandleFSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(7));
    {
        WorldPacket data(SMSG_SPELL_START, (8 + 8 + 4 + 2 + 4));

        data.Initialize(SMSG_SPELL_START);
        data << bot->GetPackGUID();
        data << bot->GetPackGUID();
        data << uint32(spellEffect);
        data << uint16(0);
        data << uint32(0);
        data << uint16(2);
        data << requester->GetObjectGuid();
        bot->SendMessageToSet(data, true);
    }

    {
        WorldPacket data(SMSG_SPELL_GO, 53);                    // guess size
        data << bot->GetPackGUID();
        data << bot->GetPackGUID();
        data << uint32(spellEffect);  // spellID
        data << uint8(0) << uint8(1);   // flags
        data << uint8(1);			   // amount of targets
        data << requester->GetObjectGuid();
        data << uint8(0);
        data << uint16(2);
        data << requester->GetObjectGuid();
        bot->SendMessageToSet(data, true);
    }

    return true;
}

bool DebugAction::HandleSpell(Event& event, Player* requester, const std::string& text)
{
    uint32 spellEffect = stoi(text.substr(6));
    requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), spellEffect);
    return true;
}

bool DebugAction::HandleTSpellMap(Event& event, Player* requester, const std::string& text)
{
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                std::ostringstream out;
                out << "effect ";
                out << effect;

                const std::string& Cname = out.str();

                wpCreature->MonsterSay(Cname.c_str(), 0, requester);
            }
        }
    }
    return true;
}

bool DebugAction::HandleUSpellMap(Event& event, Player* requester, const std::string& text)
{
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(effect, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
        }
    }
    return true;
}

bool DebugAction::HandleDSpellMap(Event& event, Player* requester, const std::string& text)
{
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            FakeSpell(effect, bot, nullptr, ObjectGuid(), {}, {}, botPos, botPos, true);
        }
    }
    return true;
}

bool DebugAction::HandleVSpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<WorldPacket> datMap;
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);
                                   
            if (wpCreature)
            {
                WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 8 + 4);        // visual effect on guid
                data << wpCreature->GetObjectGuid();
                data << uint32(effect); ;                               // index from SpellVisualKit.dbc
                //wpCreature->SendMessageToSet(data, true);
                datMap.push_back(data);

                //wpCreature->MonsterMoveWithSpeed(botPos.getX(), botPos.getY()+80, botPos.getZ(), 8.0f,true,true);
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }
    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }

    return true;
}

bool DebugAction::HandleISpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<WorldPacket> datMap;
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                WorldPacket data(SMSG_PLAY_SPELL_IMPACT, 8 + 4);        // visual effect on player
                data << wpCreature->GetObjectGuid();
                data << uint32(effect);                                 // index from SpellVisualKit.dbc
                //wpCreature->SendMessageToSet(data, true);
                datMap.push_back(data);
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }
    for (auto dat : datMap)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        bot->SendMessageToSet(dat, true);
    }

    return true;
}

bool DebugAction::HandleCSpellMap(Event& event, Player* requester, const std::string& text)
{
    Creature* wpCreature = nullptr;
    Creature* lCreature = nullptr;
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature && lCreature)
            {
                wpCreature->CastSpell(lCreature, effect, TRIGGERED_OLD_TRIGGERED);
            }

            lCreature = wpCreature;
        }
    }
    return true;
}

bool DebugAction::HandleASpellMap(Event& event, Player* requester, const std::string& text)
{
    Creature* wpCreature = nullptr;

    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 spellEffect = stoi(text.substr(10));
            uint32 effect = dx + dy * 10 + spellEffect * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            if (wpCreature)
            {
                addAura(effect, wpCreature);
            }
        }
    }
    return true;
}

bool DebugAction::HandleGSpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<ObjectGuid> all_targets;// = { bot->GetObjectGuid(), master->GetObjectGuid() };
    //vector<ObjectGuid> all_dummies = { bot->GetObjectGuid(), master->GetObjectGuid() };

    /*list<ObjectGuid> a_targets = *context->GetValue<std::list<ObjectGuid> >("all targets");
    for (auto t : a_targets)
    {
        all_targets.push_back(t);
    }
    */


    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            all_targets.push_back(wpCreature->GetObjectGuid());
        }
    }

    all_targets.push_back(requester->GetObjectGuid());
    all_targets.push_back(bot->GetObjectGuid());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (uint32 i = 0; i < 2; i++)
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;

                uint32 i = dx + dy * 10;
                std::list<ObjectGuid> hits, miss;

                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(effect);

                if (spellInfo && spellInfo->speed > 0)
                    for (auto tar : all_targets)
                    {
                        //hits.push_back(tar);

                        switch (urand(0, 10))
                        {
                        case 0:
                            hits.push_back(tar);
                            break;
                        case 1:
                            miss.push_back(tar);
                            break;
                        case 2:
                        case 3:
                            break;
                        }
                    }

                Unit* realCaster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* caster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* target = ai->GetUnit(all_targets[i + 1]);

                if (!realCaster)
                    realCaster = bot;

                if (!caster)
                    caster = bot;

                if (!target)
                    target = requester;

                FakeSpell(effect, realCaster, caster, target->GetObjectGuid(), hits, miss, WorldPosition(caster), WorldPosition(target));

                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    return true;
}

bool DebugAction::HandleMSpellMap(Event& event, Player* requester, const std::string& text)
{
    std::vector<ObjectGuid> all_targets;

    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(2334, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            all_targets.push_back(wpCreature->GetObjectGuid());
        }
    }

    all_targets.push_back(requester->GetObjectGuid());
    all_targets.push_back(bot->GetObjectGuid());

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (uint32 i = 0; i < 2; i++)
        for (int32 dx = 0; dx < 10; dx++)
        {
            for (int32 dy = 0; dy < 10; dy++)
            {
                uint32 spellEffect = stoi(text.substr(10));
                uint32 effect = dx + dy * 10 + spellEffect * 100;

                uint32 i = dx + dy * 10;
                std::list<ObjectGuid> hits, miss;

                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(effect);

                    for (auto tar : all_targets)
                    {
                        //hits.push_back(tar);

                        switch (urand(0, 10))
                        {
                        case 0:
                            hits.push_back(tar);
                            break;
                        case 1:
                            miss.push_back(tar);
                            break;
                        case 2:
                        case 3:
                            break;
                        }
                    }

                Unit* realCaster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* caster = ai->GetUnit(all_targets[i]);//ai->GetUnit(all_targets[urand(0, all_targets.size() - 1)]);
                Unit* target = ai->GetUnit(all_targets[i + 1]);

                if (!realCaster)
                    realCaster = bot;

                if (!caster)
                    caster = bot;

                if (!target)
                    target = requester;

                requester->GetSession()->SendPlaySpellVisual(caster->GetObjectGuid(), 5036);
                FakeSpell(effect, realCaster, caster, target->GetObjectGuid(), hits, miss, WorldPosition(caster), WorldPosition(target));

                std::this_thread::sleep_for(std::chrono::milliseconds(20));
            }
        }
    return true;
}

bool DebugAction::HandleSoundMap(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffects = stoi(text.substr(9));
    for (int32 dx = 0; dx < 10; dx++)
    {
        for (int32 dy = 0; dy < 10; dy++)
        {
            uint32 effect = dx + dy * 10 + soundEffects * 100;
            WorldPosition botPos(bot);

            botPos.setX(botPos.getX() + (dx - 5) * 5);
            botPos.setY(botPos.getY() + (dy - 5) * 5);
            botPos.setZ(botPos.getHeight(bot->GetInstanceId()));

            Creature* wpCreature = bot->SummonCreature(6, botPos.getX(), botPos.getY(), botPos.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            wpCreature->PlayDistanceSound(effect);
        }
    }

    return true;
}

bool DebugAction::HandleSounds(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffects = stoi(text.substr(6));

    for (uint32 i = 0; i < 100; i++)
    {
        bot->PlayDistanceSound(i + soundEffects * 100);
        bot->Say(std::to_string(i + soundEffects * 100), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return true;
}

bool DebugAction::HandleDSound(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffect = stoi(text.substr(7));
    bot->PlayDirectSound(soundEffect);
    return true;
}

bool DebugAction::HandleSound(Event& event, Player* requester, const std::string& text)
{
    uint32 soundEffect = stoi(text.substr(6));
    bot->PlayDistanceSound(soundEffect);
    return true;
}

bool DebugAction::HandleStuck(Event& event, Player* requester, const std::string& text)
{
    bool shouldReset = (text.find("reset") != std::string::npos);

    ai->TellPlayer(requester, "=== Stuck Bot Diagnostic ===");

    WorldPosition botPos(bot);
    std::ostringstream posOut;
    posOut << "Pos: " << botPos.getX() << "," << botPos.getY() << "," << botPos.getZ() << " (" << botPos.getAreaName() << ")";
    ai->TellPlayer(requester, posOut.str());

    bool isMoving = bot->IsMoving();
    ai->TellPlayer(requester, std::string("IsMoving: ") + (isMoving ? "yes" : "no"));

    bool isMounted = bot->IsMounted();
    ai->TellPlayer(requester, std::string("IsMounted: ") + (isMounted ? "yes" : "no"));

    bool isTaxiFlying = bot->IsTaxiFlying();
    ai->TellPlayer(requester, std::string("IsTaxiFlying: ") + (isTaxiFlying ? "yes" : "no"));

    bool isInCombat = bot->IsInCombat();
    ai->TellPlayer(requester, std::string("IsInCombat: ") + (isInCombat ? "yes" : "no"));

    bool isDead = !bot->IsAlive();
    ai->TellPlayer(requester, std::string("IsDead: ") + (isDead ? "yes" : "no"));

    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    bool canFreeMove = false;
    if (travelTarget)
    {
        std::ostringstream ss;
        ss << "Travel Target: ";
        switch (travelTarget->GetStatus())
        {
            case TravelStatus::TRAVEL_STATUS_NONE: ss << "NONE"; break;
            case TravelStatus::TRAVEL_STATUS_PREPARE: ss << "PREPARE"; break;
            case TravelStatus::TRAVEL_STATUS_WORK: ss << "WORK"; break;
            case TravelStatus::TRAVEL_STATUS_TRAVEL: ss << "TRAVEL"; break;
            case TravelStatus::TRAVEL_STATUS_READY: ss << "READY"; break;
            case TravelStatus::TRAVEL_STATUS_EXPIRED: ss << "EXPIRED"; break;
            case TravelStatus::TRAVEL_STATUS_COOLDOWN: ss << "COOLDOWN"; break;
            default: ss << "UNKNOWN"; break;
        }
        ss << " (" << travelTarget->GetTimeLeft() / 1000 << "s left)";
        ai->TellPlayer(requester, ss.str());

        if (travelTarget->GetPosition())
        {
            std::ostringstream ss2;
            ss2 << "Target: " << travelTarget->GetPosition()->getX() << "," 
                << travelTarget->GetPosition()->getY() << "," << travelTarget->GetPosition()->getZ() 
                << " (" << travelTarget->GetPosition()->getAreaName() << ")";
            ai->TellPlayer(requester, ss2.str());

            float dist = botPos.distance(*travelTarget->GetPosition());
            std::ostringstream ss3;
            ss3 << "Distance to target: " << uint32(dist) << "y";
            ai->TellPlayer(requester, ss3.str());
        }

        std::ostringstream ss4;
        ss4 << "Retry: " << travelTarget->GetRetryCount(true) << " (move), " << travelTarget->GetRetryCount(false) << " (target)";
        ai->TellPlayer(requester, ss4.str());

        canFreeMove = CanFreeMoveValue::CanFreeMoveTo(ai, travelTarget->GetPosStr());
    }

    bool canMoveAround = AI_VALUE(bool, "can move around");
    ai->TellPlayer(requester, std::string("can move around: ") + (canMoveAround ? "true" : "FALSE!"));

    bool travelTargetActive = AI_VALUE(bool, "travel target active");
    ai->TellPlayer(requester, std::string("travel target active: ") + (travelTargetActive ? "true" : "false"));

    bool travelTargetTraveling = AI_VALUE(bool, "travel target traveling");
    ai->TellPlayer(requester, std::string("travel target traveling: ") + (travelTargetTraveling ? "true" : "FALSE!"));

    if (travelTarget && travelTarget->GetPosition())
    {
        ai->TellPlayer(requester, std::string("can free move to target: ") + (canFreeMove ? "true" : "FALSE!"));
        
        bool differentMap = (travelTarget->GetPosition()->getMapId() != bot->GetMapId());
        if (differentMap)
        {
            ai->TellPlayer(requester, ">>> TARGET IS ON DIFFERENT MAP! Needs taxi/transport.");
            ai->TellPlayer(requester, "    Bot map: " + std::to_string(bot->GetMapId()) + ", Target map: " + std::to_string(travelTarget->GetPosition()->getMapId()));
        }
    }

    uint32 posLastChange = MEM_AI_VALUE(WorldPosition, "current position")->LastChangeDelay();
    std::ostringstream ss;
    ss << "Position last changed: " << posLastChange << "s ago";
    ai->TellPlayer(requester, ss.str());

    bool isStuck = (posLastChange > 60 && travelTargetActive);
    if (isStuck)
    {
        ai->TellPlayer(requester, ">>> BOT IS STUCK! Position unchanged for > 60s with active travel target!");
    }

    Group* group = bot->GetGroup();
    if (group)
    {
        bool isLeader = group->IsLeader(bot->GetObjectGuid());
        ai->TellPlayer(requester, std::string("In group, is leader: ") + (isLeader ? "yes" : "no"));

        if (!isLeader)
        {
            bool hasFollow = ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT);
            bool hasStay = ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT);
            ai->TellPlayer(requester, std::string("Has follow strategy: ") + (hasFollow ? "yes" : "no"));
            ai->TellPlayer(requester, std::string("Has stay strategy: ") + (hasStay ? "yes" : "no"));
        }
    }

    if (shouldReset && travelTarget)
    {
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, ">>> Resetting travel target...");
        travelTarget->SetStatus(TravelStatus::TRAVEL_STATUS_NONE);
        travelTarget->SetForced(false);
        RESET_AI_VALUE(TravelTarget*, "travel target");
        ai->TellPlayer(requester, ">>> Travel target reset! Bot should find new target.");
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== Movement Blockers ===");

    bool isTrading = bot->GetTradeData() != nullptr;
    ai->TellPlayer(requester, std::string("IsTrading: ") + (isTrading ? "TRUE - BLOCKS MOVEMENT!" : "no"));

    bool groupReady = AI_VALUE(bool, "group ready");
    ai->TellPlayer(requester, std::string("Group ready: ") + (groupReady ? "yes" : "NO - BLOCKS MOVEMENT!"));

    bool castNcActive = AI_VALUE2(bool, "trigger active", "castnc");
    ai->TellPlayer(requester, std::string("CastNC active: ") + (castNcActive ? "TRUE - BLOCKS MOVEMENT!" : "no"));

    bool hasWander = ai->HasStrategy("wander", BotState::BOT_STATE_NON_COMBAT);
    if (hasWander)
    {
        float dist = AI_VALUE2(float, "distance", "master target");
        float wanderMax = ai->GetRange("wandermax");
        bool wanderTooFar = dist > wanderMax;
        ai->TellPlayer(requester, std::string("Wander strategy: yes (dist=") + std::to_string((int)dist) + ", max=" + std::to_string((int)wanderMax) + ") " + (wanderTooFar ? "- BLOCKS MOVEMENT!" : ""));
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== Path/Rout e Status ===");
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");
    bool pathEmpty = lastMove.lastPath.empty();
    ai->TellPlayer(requester, std::string("lastPath empty: ") + (pathEmpty ? "YES" : "no"));
    if (!pathEmpty)
    {
        ai->TellPlayer(requester, "lastPath points: " + std::to_string(lastMove.lastPath.getPath().size()));
        ai->TellPlayer(requester, "lastPath end: " + std::to_string(lastMove.lastPath.getBack().getX()) + "," + 
            std::to_string(lastMove.lastPath.getBack().getY()) + "," + 
            std::to_string(lastMove.lastPath.getBack().getZ()));
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== Other Status ===");

    bool hasStay = ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT);
    ai->TellPlayer(requester, std::string("Has stay strategy: ") + (hasStay ? "yes" : "no"));

    bool hasFollow = ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT);
    ai->TellPlayer(requester, std::string("Has follow strategy: ") + (hasFollow ? "yes" : "no"));

    bool hasGuard = ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT);
    ai->TellPlayer(requester, std::string("Has guard strategy: ") + (hasGuard ? "yes" : "no"));

    //uint32 auraCount = bot->GetAuraCount();
    //ai->TellPlayer(requester, "Aura count: " + std::to_string(auraCount));

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== Suggested Actions ===");
    if (!canMoveAround)
    {
        ai->TellPlayer(requester, "- Check: is bot in combat? Is bot dead? Is bot rooted?");
    }
    if (travelTarget && travelTarget->GetStatus() == TravelStatus::TRAVEL_STATUS_READY && !isMoving)
    {
        ai->TellPlayer(requester, "- Try: .rndbot cmd <bot> reset travel target");
    }
    if (posLastChange > 120)
    {
        ai->TellPlayer(requester, "- Bot may need teleportation: .rndbot debug <bot> position teleport");
    }
    if (pathEmpty && travelTargetActive)
    {
        ai->TellPlayer(requester, "- Try: .rndbot debug <bot> position route <destination>");
    }

    return true;
}

bool DebugAction::HandleCombat(Event& event, Player* requester, const std::string& text)
{
    ai->TellPlayer(requester, "=== Combat State Diagnostic ===");

    bool unitFlagInCombat = bot->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
    ai->TellPlayer(requester, std::string("UNIT_FLAG_IN_COMBAT: ") + (unitFlagInCombat ? "SET" : "clear"));

    bool isInCombat = bot->IsInCombat();
    ai->TellPlayer(requester, std::string("IsInCombat(): ") + (isInCombat ? "true" : "false"));

    ai->TellPlayer(requester, "CMaNGOS attackers: " + std::to_string(bot->getAttackers().size()));

    Unit* victim = bot->GetVictim();
    ai->TellPlayer(requester, std::string("victim: ") + (victim ? victim->GetName() : "none"));

    ai->TellPlayer(requester, "--- BotAI State ---");

    Unit* aiTarget = *ai->GetAiObjectContext()->GetValue<Unit*>("current target");
    if (aiTarget)
    {
        std::ostringstream targetOut;
        targetOut << "current target: " << aiTarget->GetName()
                  << " (" << aiTarget->GetObjectGuid().GetCounter() << ")";
        ai->TellPlayer(requester, targetOut.str());

        bool isInvalid = ai->GetAiObjectContext()->GetValue<bool>("invalid target", "current target")->Get();
        ai->TellPlayer(requester, std::string("invalid: ") + (isInvalid ? "YES" : "no"));
    }
    else
    {
        ai->TellPlayer(requester, "current target: none");
    }

    bool hasAttackers = ai->GetAiObjectContext()->GetValue<bool>("has attackers")->Get();
    ai->TellPlayer(requester, std::string("has attackers: ") + (hasAttackers ? "true" : "false"));

    ai->TellPlayer(requester, "Selection: " + std::to_string(bot->GetSelectionGuid().GetCounter()));

    return true;
}

bool DebugAction::HandleNodes(Event& event, Player* requester, const std::string& text)
{
    WorldPosition pos(bot);
    ai->TellPlayer(requester, "=== Debug: getRoute Pathfinding Analysis ===");
    ai->TellPlayer(requester, "Bot position: " + pos.print());
    
    bool inCombat = bot->IsInCombat();
    bool inWater = bot->IsInWater();
    bool isUnderWater = bot->IsUnderwater();
    ai->TellPlayer(requester, "Bot state: combat=" + std::string(inCombat ? "YES" : "NO") + 
                   " water=" + std::string(inWater ? "YES" : "NO") + 
                   " underwater=" + std::string(isUnderWater ? "YES" : "NO"));

    std::vector<TravelNode*> allNodes = sTravelNodeMap.getNodes(pos, -1.0f);
    ai->TellPlayer(requester, "Total nodes on map " + std::to_string(pos.getMapId()) + ": " + std::to_string(allNodes.size()));

    if (allNodes.empty())
    {
        ai->TellPlayer(requester, ">>> No nodes at all on this map!");
        return true;
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== getRoute Step 1: Find START nodes (closest 5) ===");

    std::sort(allNodes.begin(), allNodes.end(), [pos](TravelNode* i, TravelNode* j) { 
        return i->getPosition()->sqDistance(pos) < j->getPosition()->sqDistance(pos); 
    });

    std::vector<TravelNode*> startNodes;
    uint32 startNr = std::min(5u, (uint32)allNodes.size());
    for (uint32 i = 0; i < startNr; i++)
        startNodes.push_back(allNodes[i]);

    ai->TellPlayer(requester, "Testing pathfinding to each of the 5 closest nodes:");

    int startNodesWithPath = 0;
    for (uint32 i = 0; i < startNodes.size(); i++)
    {
        auto& node = startNodes[i];
        
        TravelNodeMap::PathFindResult result = sTravelNodeMap.testPathToLoop(pos, *node->getPosition(), bot, 0, {}, "debug");
        
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Start Node " + std::to_string(i+1) + ": " + node->getName());
        ai->TellPlayer(requester, "  Distance: " + std::to_string(pos.distance(*node->getPosition())) + "y, PathType: " + 
            (result.type == PATHFIND_NORMAL ? "NORMAL" : 
             result.type == PATHFIND_INCOMPLETE ? "INCOMPLETE" : 
             result.type == PATHFIND_NOPATH ? "NOPATH" : "UNKNOWN"));
        
        if (result.path.size() > 0)
        {
            ai->TellPlayer(requester, "  Path points: " + std::to_string(result.path.size()) + ", stops " + 
                std::to_string(result.path.back().distance(bot)) + "y from bot");
        }
        
        if (node->getPosition()->isPathTo(result.path))
        {
            ai->TellPlayer(requester, "  >>> FULL PATH to node");
            startNodesWithPath++;
        }
        else
        {
            ai->TellPlayer(requester, "  >> NO FULL PATH");
        }
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== getRoute Step 2: Check network routes ===");

    int nodesWithRoute = 0;
    for (uint32 i = 0; i < startNodes.size(); i++)
    {
        auto& startNode = startNodes[i];
        
        float dist = pos.distance(*startNode->getPosition());
        
        std::unique_ptr<PathFinder> pathfinder = std::make_unique<PathFinder>(bot);
        pathfinder->setAreaCost(NAV_AREA_WATER, 10.0f);
        pathfinder->setAreaCost(12, 5.0f);
        pathfinder->setAreaCost(13, 20.0f);
        pathfinder->calculate(pos.getVector3(), startNode->getPosition()->getVector3(), false);
        
        PointsArray points = pathfinder->getPath();
        bool hasFullPath = false;
        if (!points.empty())
        {
            Vector3 lastVec = points.back();
            WorldPosition lastPoint(bot->GetMapId(), lastVec.x, lastVec.y, lastVec.z);
            if (dist - pos.distance(lastPoint) <= 1.0f)
                hasFullPath = true;
        }

        if (!hasFullPath)
            continue;

        std::vector<TravelNode*> reachable = startNode->getNodeMap(false, {}, false);
        
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, "Start: " + startNode->getName() + " (has path)");
        ai->TellPlayer(requester, "  Can reach: " + std::to_string(reachable.size()) + " nodes in network");
        
        if (reachable.size() > 0)
        {
            std::string nodeList;
            uint32 count = std::min(10u, (uint32)reachable.size());
            for (uint32 j = 0; j < count; j++)
            {
                nodeList += reachable[j]->getName();
                if (j < count - 1)
                    nodeList += ", ";
            }
            if (reachable.size() > count)
                nodeList += "...";
            ai->TellPlayer(requester, "  Nodes: " + nodeList);
        }
        
        if (reachable.size() > 10)
        {
            nodesWithRoute++;
            ai->TellPlayer(requester, "  >>> HAS NETWORK ROUTES");
        }
        else
        {
            ai->TellPlayer(requester, "  >> LIMITED NETWORK (only " + std::to_string(reachable.size()) + " nodes)");
        }
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== getRoute Step 3: Try actual route (like bot uses) ===");

    WorldPosition targetPos = pos;
    targetPos.coord_x += 100.0f;
    targetPos.coord_y += 100.0f;

    std::vector<WorldPosition> beginPath, endPath;
    TravelNodeRoute route = sTravelNodeMap.getRoute(pos, targetPos, beginPath, endPath, bot);

    if (!route.isEmpty())
    {
        ai->TellPlayer(requester, ">>> ROUTE FOUND!");
        ai->TellPlayer(requester, "  Nodes in route: " + std::to_string(route.getNodes().size()));
    }
    else
    {
        ai->TellPlayer(requester, ">>> NO ROUTE FOUND");
        ai->TellPlayer(requester, "  This means bot cannot travel using the node network");
    }

    ai->TellPlayer(requester, "");
    ai->TellPlayer(requester, "=== Summary ===");
    ai->TellPlayer(requester, "Start nodes with full path: " + std::to_string(startNodesWithPath) + "/5");
    ai->TellPlayer(requester, "Start nodes with network routes: " + std::to_string(nodesWithRoute));

    if (startNodesWithPath == 0)
    {
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, ">>> PROBLEM: Bot cannot path to ANY start node!");
        ai->TellPlayer(requester, "  - Bot may be in water, indoors, void, or no navmesh");
    }
    else if (nodesWithRoute == 0)
    {
        ai->TellPlayer(requester, "");
        ai->TellPlayer(requester, ">>> PROBLEM: Start nodes have no network routes!");
        ai->TellPlayer(requester, "  - Try: debug gen path (mod only)");
    }

    return true;
}

bool DebugAction::HandleActivity(Event& event, Player* requester, const std::string& text)
{
    std::string action = ai->HandleRemoteCommand("action");

    bool isActive = ai->AllowActivity();

    std::string stateName;
    switch (ai->GetState())
    {
        case BotState::BOT_STATE_COMBAT: stateName = "combat"; break;
        case BotState::BOT_STATE_NON_COMBAT: stateName = "non-combat"; break;
        case BotState::BOT_STATE_DEAD: stateName = "dead"; break;
        case BotState::BOT_STATE_REACTION: stateName = "reaction"; break;
        default: stateName = "unknown"; break;
    }

    std::string priorityTypeName;
    switch (ai->GetPriorityType())
    {
        case ActivePiorityType::IS_REAL_PLAYER: priorityTypeName = "real_player"; break;
        case ActivePiorityType::HAS_REAL_PLAYER_MASTER: priorityTypeName = "has_master"; break;
        case ActivePiorityType::IN_GROUP_WITH_REAL_PLAYER: priorityTypeName = "group_with_player"; break;
        case ActivePiorityType::IN_BATTLEGROUND: priorityTypeName = "battleground"; break;
        case ActivePiorityType::IN_INSTANCE: priorityTypeName = "instance"; break;
        case ActivePiorityType::VISIBLE_FOR_PLAYER: priorityTypeName = "visible_for_player"; break;
        case ActivePiorityType::IS_ALWAYS_ACTIVE: priorityTypeName = "always_active"; break;
        case ActivePiorityType::IN_COMBAT: priorityTypeName = "combat"; break;
        case ActivePiorityType::IN_BG_QUEUE: priorityTypeName = "bg_queue"; break;
        case ActivePiorityType::IN_LFG: priorityTypeName = "lfg"; break;
        case ActivePiorityType::NEARBY_PLAYER: priorityTypeName = "nearby_player"; break;
        case ActivePiorityType::PLAYER_FRIEND: priorityTypeName = "friend"; break;
        case ActivePiorityType::PLAYER_GUILD: priorityTypeName = "guild"; break;
        case ActivePiorityType::NO_PATH: priorityTypeName = "no_path"; break;
        case ActivePiorityType::IN_ACTIVE_AREA: priorityTypeName = "active_area"; break;
        case ActivePiorityType::IN_ACTIVE_MAP: priorityTypeName = "active_map"; break;
        case ActivePiorityType::IN_INACTIVE_MAP: priorityTypeName = "inactive_map"; break;
        case ActivePiorityType::IN_EMPTY_SERVER: priorityTypeName = "empty_server"; break;
        default: priorityTypeName = "unknown"; break;
    }

    auto bracket = ai->GetPriorityBracket(ai->GetPriorityType());
    float activityPct = sRandomPlayerbotMgr.getActivityPercentage();

    std::ostringstream out;
    out << "State: " << stateName << ", Active: " << (isActive ? "yes" : "no");
    out << ", PriorityType: " << priorityTypeName;
    out << ", Bracket: " << bracket.first << "-" << bracket.second;
    out << ", Activity%: " << std::fixed << std::setprecision(1) << activityPct;
    if (!action.empty())
        out << ", Action: " << action;

    ai->TellPlayer(requester, out.str());
    return true;
}

bool DebugAction::HandleTransanal(Event& event, Player* requester, const std::string& text)
{
    sLog.outString("Starting transport analysis...");

    sPlayerbotAIConfig.openLog("transportanalysis.csv", "w", true);

    uint32 shiftx = 0;
    uint32 shifty = 0;

    for (auto& [mapId, map] : sMapMgr.Maps())
    {
        for (auto& transport : WorldPosition(map->GetId(), 1, 1).getTransports())
        {
            std::string transportName = transport->GetName();
            GameObjectInfo const* data = sGOStorage.LookupEntry<GameObjectInfo>(transport->GetEntry());
            if (transportName.empty())
                transportName = data->name;

            sLog.outString("transport %s", transportName.c_str());

            MMAP::MMapManager* mmap = MMAP::MMapFactory::createOrGetMMapManager();
            if (!mmap->GetModelNavMeshQuery(transport->GetDisplayId()))
            {
                sLog.outString("%d has no ModelNavMeshQuery", transport->GetDisplayId());

                continue;
            }

            BarGoLink bar(18 * 80 * 80 * 80);

            shifty += 1;

            shiftx = 0;

            for (float rotation = 0.0f; rotation <= 160.0f; rotation += 20.0f)
            {
                shiftx += 1;
                float rad = rotation * M_PI / 180.0f;

                transport->UpdatePosition(0, 0, 0, rad);

                WorldPosition transPos(transport);
                transPos.SetTranpotHeightToFloor(transport->GetEntry());

                std::vector<WorldPosition> hitPoints;

                sLog.outString("rotation %d", (int32)rotation);

                for (float x = -50.0f; x <= 50.0f; x += 2.0f)
                {
                    for (float y = -50.0f; y <= 50.0f; y += 2.0f)
                    {
                        for (float z = -50.0f; z <= 50.0f; z += 2.0f)
                        {
                            bar.step();
                            WorldPosition testPos = transPos + WorldPosition(0, x, y, z);

                            float hx = x, hy = y, hz = z - 50;

#ifndef MANGOSBOT_TWO
                            bool hasHit = map->GetHitPosition(testPos.getX(), testPos.getY(), testPos.getZ(), hx, hy, hz, 0.0f);
#else
                            bool hasHit = map->GetHitPosition(testPos.getX(), testPos.getY(), testPos.getZ(), hx, hy, hz, 0, 0.0f);
#endif

                            if (!hasHit)
                                continue;

                            WorldPosition hitPos(0, hx, hy, hz);

                            hasHit = false;

                            for (auto& hp : hitPoints)
                                if (hp.sqDistance(hitPos) < 0.1)
                                {
                                    hasHit = true;
                                    break;
                                }

                            if (!hasHit)
                                hitPoints.push_back(hitPos);
                        }
                    }
                }

                if (hitPoints.empty())
                    continue;

                WorldSession* session = new WorldSession(0, NULL, SEC_PLAYER,
#ifdef MANGOSBOT_TWO
                    2,
                    0,
                    LOCALE_enUS,
                    "",
                    0,
                    0,
                    false);
#endif
#ifdef MANGOSBOT_ONE
                    2, 0, LOCALE_enUS, "", 0, 0, false);
#endif
#ifdef MANGOSBOT_ZERO
                    0, LOCALE_enUS, "", 0);
#endif

                    session->SetNoAnticheat();

                    Player* tempPlayer = new Player(session);

                    tempPlayer->Create(sObjectMgr.GeneratePlayerLowGuid(), "test", 1, 1, 0,
                        0, // skinColor,
                        0,
                        0,
                        0, // hairColor,
                        0,
                        0);
                    tempPlayer->AddToWorld();
                    tempPlayer->SetMap(map.get());
                    tempPlayer->SetTransport(transport);
                    tempPlayer->SetPosition(transPos.getX(), transPos.getY(), transPos.getZ(), 0);

                    std::vector<WorldPosition> rhitPoints = hitPoints;

                    std::reverse(rhitPoints.begin(), rhitPoints.end());

                    std::vector<std::vector<WorldPosition>> layers;
                    std::unordered_map<WorldPosition, uint32> found;

                    sLog.outString("hitting done");

                    BarGoLink bar1(hitPoints.size());

                    layers.push_back(hitPoints);

                    for (auto& start : hitPoints)
                    {
                        if (found[start])
                            continue;

                        bar1.step();
                        for (auto& end : rhitPoints)
                        {
                            if (end == start)
                                continue;

                            if (found[end])
                                continue;

                            std::unique_ptr<PathFinder> pathfinder = std::make_unique<PathFinder>(tempPlayer, true);

                            WorldPosition tStart = start, tEnd = end;
                            tStart.CalculatePassengerOffset(transport);
                            tEnd.CalculatePassengerOffset(transport);
                            //tStart.CalculatePassengerPosition(transport);
                            //tEnd.CalculatePassengerPosition(transport);

                            pathfinder->calculate(tStart.getVector3(), tEnd.getVector3(), false);

                            if (pathfinder->getPathType() != PATHFIND_NORMAL)
                                continue;

                            if (pathfinder->getPath().size() < 3)
                                continue;

                            std::vector<WorldPosition> path = start.fromPointsArray(pathfinder->getPath());

                            std::set<uint32> inLayer;

                            std::vector<WorldPosition> realPath = {start, end};

                            for (auto& p : path)
                            {
                                if (p == start)
                                    continue;

                                if (p == end)
                                    continue;

                                if (std::find(realPath.begin(), realPath.end(), p) != realPath.end())
                                    continue;

                                WorldPosition rp = p;
                                rp.CalculatePassengerPosition(transport);
                                //rp.CalculatePassengerOffset(transport);

                                WorldPosition realHp;

                                for (auto& hp : hitPoints)
                                {
                                    if (rp.distance(hp) < 2)
                                    {
                                        realHp = hp;
                                        break;
                                    }
                                }

                                if (realHp)
                                {
                                    if (found[realHp])
                                        inLayer.insert(found[realHp]);
                                    else
                                        realPath.push_back(realHp);
                                }
                            }

                            if (realPath.size() < 3)
                                continue;

                            if (inLayer.empty())
                            {
                                layers.push_back(realPath);
                                for (auto& hp : realPath)
                                    found[hp] = layers.size();
                            }
                            else
                            {
                                std::vector<std::vector<WorldPosition>> newLayers = {hitPoints, realPath};

                                for (uint32 i = 1; i < layers.size(); i++)
                                {
                                    if (inLayer.find(i) != inLayer.end())
                                        newLayers[1].insert(newLayers[1].end(), layers[i].begin(), layers[i].end());
                                    else
                                        newLayers.push_back(layers[i]);
                                }

                                layers = newLayers;
                                found.clear();

                                for (uint32 i = 1; i < layers.size(); i++)
                                {
                                    for (auto& p : layers[i])
                                    {
                                        found[p] = i;
                                    }
                                }
                            }
                        }
                    }

                    int layerNr = 0;
                    for (auto& layer : layers)
                    {
                        float minZ = FLT_MAX, maxZ = -FLT_MAX;
                        for (auto& p : layer)
                        {
                            float relZ = p.getZ() - transPos.getZ();
                            if (relZ < minZ)
                                minZ = relZ;
                            if (relZ > maxZ)
                                maxZ = relZ;
                        }

                        std::ostringstream out;

                        out << transport->GetEntry() << ",";
                        out << transportName << ",";
                        out << rotation << ",";
                        out << transport->GetOrientation() << ",";
                        out << rotation << ",";
                        out << layerNr << ",";
                        out << minZ << ",";
                        out << maxZ << ",";

                        out << "\"MULTILINESTRING(";
                        bool first = true;
                        for (auto& p : layer)
                        {
                            float relX = p.getX() * 10 + shiftx * 1100;
                            float relY = p.getY() * 10 + shifty * 1100;
                            float relZ = p.getZ() * 10;
                            if (!first)
                                out << ",";
                            out << "(" << relX << " " << relY << " " << relZ << ",";
                            out << relX + relZ * 0.2 << " " << relY + relZ + 0.8 << " " << relZ << ")";
                            first = false;
                        }
                        out << ")\"";

                        sPlayerbotAIConfig.log("transportanalysis.csv", out.str().c_str());

                        layerNr++;
                    }

                    sLog.outString("pathfinding done");

                    tempPlayer->RemoveFromWorld();
                    delete tempPlayer;
                    delete session;
            }

            transport->Update(100);

            transport->UpdatePosition(5000, 5000, 0, 0);
        }
    }

    sLog.outString("Transport analysis complete.");
    return true;
}

bool DebugAction::HandleUpdownspace(Event& event, Player* requester, const std::string& text)
{
    float startHeightOffset = 0.0f;

    if (text.length() > std::string("updownspace").size())
    {
        startHeightOffset = atof(text.substr(std::string("updownspace").size() + 1).c_str());
    }

    float botX = bot->GetPositionX();
    float botY = bot->GetPositionY();
    float botZ = bot->GetPositionZ();
    float startZ = botZ + startHeightOffset;

    std::ostringstream out;
    out << "Bot pos: (" << botX << ", " << botY << ", " << botZ << ") offset: " << startHeightOffset << " startZ: " << startZ << "\n";

    float destX_down = botX;
    float destY_down = botY;
    float destZ_down = startZ - 50.0f;
    float vmapDestX_down = botX, vmapDestY_down = botY, vmapDestZ_down = startZ - 50.0f;

    VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(bot->GetMapId(), botX, botY, startZ, destX_down, destY_down, destZ_down, vmapDestX_down, vmapDestY_down, vmapDestZ_down, 0);
    bool vmapHitDown = (vmapDestX_down != botX || vmapDestY_down != botY || fabs(vmapDestZ_down - (startZ - 50.0f)) > 0.1f);

#ifndef MANGOSBOT_TWO
    bool combinedHitDown = bot->GetMap()->GetHitPosition(botX, botY, startZ, destX_down, destY_down, destZ_down, 0);
#else
    bool combinedHitDown = bot->GetMap()->GetHitPosition(botX, botY, startZ, destX_down, destY_down, destZ_down, 0, 0);
#endif

    if (vmapHitDown)
        out << "VMAP DOWN hit at z: " << vmapDestZ_down << "\n";
    else
        out << "VMAP DOWN no hit\n";

    if (combinedHitDown)
    {
        bool isDynamic = (destX_down != vmapDestX_down || destY_down != vmapDestY_down || fabs(destZ_down - vmapDestZ_down) > 0.1f);
        if (isDynamic)
            out << "OBJECT DOWN hit at z: " << destZ_down << "\n";
    }

    float destX_up = botX;
    float destY_up = botY;
    float destZ_up = startZ + 50.0f;
    float vmapDestX_up = botX, vmapDestY_up = botY, vmapDestZ_up = startZ + 50.0f;

    VMAP::VMapFactory::createOrGetVMapManager()->getObjectHitPos(bot->GetMapId(), botX, botY, startZ, destX_up, destY_up, destZ_up, vmapDestX_up, vmapDestY_up, vmapDestZ_up, 0);
    bool vmapHitUp = (vmapDestX_up != botX || vmapDestY_up != botY || fabs(vmapDestZ_up - (startZ + 50.0f)) > 0.1f);

#ifndef MANGOSBOT_TWO
    bool combinedHitUp = bot->GetMap()->GetHitPosition(botX, botY, startZ, destX_up, destY_up, destZ_up, 0);
#else
    bool combinedHitUp = bot->GetMap()->GetHitPosition(botX, botY, startZ, destX_up, destY_up, destZ_up, 0, 0);
#endif

    if (vmapHitUp)
        out << "VMAP UP hit at z: " << vmapDestZ_up << "\n";
    else
        out << "VMAP UP no hit\n";

    if (combinedHitUp)
    {
        bool isDynamic = (destX_up != vmapDestX_up || destY_up != vmapDestY_up || fabs(destZ_up - vmapDestZ_up) > 0.1f);
        if (isDynamic)
            out << "OBJECT UP hit at z: " << destZ_up << "\n";
    }

    ai->TellPlayer(requester, out.str());
    return true;
}

bool DebugAction::HandlePatharound(Event& event, Player* requester, const std::string& text)
{
    float distance = 20.0f;

    if (text.length() > std::string("patharound").size())
    {
        distance = atof(text.substr(std::string("patharound").size() + 1).c_str());
    }

    WorldPosition botPos(bot);
    float botX = botPos.getX();
    float botY = botPos.getY();
    float botZ = botPos.getZ();

    std::ostringstream out;
    out << "Patharound: distance = " << distance << " from (" << botX << ", " << botY << ", " << botZ << ")\n";

    uint32 pathCount = 0;
    uint32 successCount = 0;

    if (bot->GetTransport())
        botPos.CalculatePassengerOffset(bot->GetTransport());

    for (float angle = 0.0f; angle < 360.0f; angle += 20.0f)
    {
        float rad = angle * M_PI / 180.0f;

        WorldPosition target(bot);
        target += WorldPosition(0, distance * cos(rad), distance * sin(rad));

        if (bot->GetTransport())
            target.CalculatePassengerOffset(bot->GetTransport());

        PathFinder pathfinder(bot, true);
        pathfinder.calculate(botPos.getVector3(), target.getVector3(), false);

        if (!(pathfinder.getPathType() & PATHFIND_NORMAL))
        {
            continue;
        }

        std::vector<WorldPosition> path = botPos.fromPointsArray(pathfinder.getPath());

        if (path.size() < 2 || path.front().distance(path[1]) > 10.0f)
        {
            continue;
        }

        bool reachedTarget = false;

        for (size_t i = 0; i < path.size(); ++i)
        {
            WorldPosition p = path[i];

            //if (bot->GetTransport())
            //    p.CalculatePassengerPosition(bot->GetTransport());

            Creature* wpCreature = bot->SummonCreature(2334, p.getX(), p.getY(), p.getZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 10000.0f);

            ai->AddAura(wpCreature, 246);

            if (i == path.size() - 1)
            {
                float finalDist = target.distance(p);
                if (finalDist < 5.0f)
                {
                    reachedTarget = true;
                    ai->AddAura(wpCreature, 1130);
                }
            }
        }

        pathCount++;
        if (reachedTarget)
            successCount++;
    }

    out << "Path attempts: " << pathCount << ", reached target: " << successCount;
    ai->TellPlayer(requester, out.str());
    return true;
}

bool DebugAction::HandleHeightForLos(Event& event, Player* requester, const std::string& text)
{
    Player* master = GetMaster();
    if (!master)
    {
        ai->TellPlayer(requester, "No master found");
        return true;
    }

    WorldPosition masterPos(master);
    WorldPosition botPos(bot);

    std::ostringstream out;
    out << "Bot: (" << botPos.print() << ")\n";
    out << "Master: (" << masterPos.print() << ")\n";

    bool initialLos = botPos.IsInLineOfSight(masterPos, 1.5f);
    out << "Initial LoS (1.5f offset): " << (initialLos ? "YES" : "NO") << "\n";

    if (initialLos)
    {
        out << "Already have LoS at offset 1.5f both sides. Total height: 3.0f";
        ai->TellPlayer(requester, out.str());
        return true;
    }

    float heightStep = 0.5f;
    float maxHeight = 50.0f;

    float bestBotHeight = 0.0f;

    out << "Searching for bot height...\n";
    for (float botHeight = 0.0f; botHeight <= maxHeight; botHeight += heightStep)
    {
        bool los = botPos.IsInLineOfSight(masterPos, botHeight);
        if (los)
        {
            out << "Found LoS with bot +" << botHeight;
            bestBotHeight = botHeight;
            break;
        }
    }

    if (bestBotHeight == maxHeight)
    {
        out << "No LoS found within " << maxHeight << " height range";
    }

    ai->TellPlayer(requester, out.str());
    return true;
}