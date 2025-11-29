//add here most rarely modified headers to speed up debug build compilation
#include "Server/WorldSocket.h"
#include "Common.h"

// Core game systems
#include "Maps/MapManager.h"
#include "Log/Log.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Server/SQLStorages.h"
#include "Server/Opcodes.h"
#include "Globals/SharedDefines.h"
#include "Guilds/GuildMgr.h"
#include "Globals/ObjectMgr.h"
#include "DBScripts/ScriptMgr.h"

// Heavy game headers (frequently used, rarely modified)
#include "Spells/Spell.h"
#include "Spells/SpellMgr.h"
#include "Spells/SpellAuras.h"
#include "Server/WorldPacket.h"
#include "Loot/LootMgr.h"
#include "Entities/GossipDef.h"
#include "Chat/Chat.h"
#include "World/World.h"
#include "Entities/Unit.h"
#include "MotionGenerators/MotionMaster.h"
#include "Guilds/Guild.h"
#include "Entities/Player.h"
#include "Groups/Group.h"
#include "Database/DatabaseEnv.h"

// Grid system (used in many playerbot files)
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

// Boost headers (used across multiple files)
#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

// STL headers
#include <stack>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <memory>
#include <regex>
#include <numeric>

// Playerbot core
#include "playerbot/playerbot.h"

// Playerbot AI framework (included 40-60+ times each, rarely modified)
#include "playerbot/strategy/AiObject.h"
#include "playerbot/strategy/Value.h"
#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/Trigger.h"
#include "playerbot/strategy/Strategy.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/strategy/AiObjectContext.h"
