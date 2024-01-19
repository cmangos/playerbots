#include "../botpch.h"
#include "../ahbot/AhBot.h"
#include "PlayerbotAIConfig.h"
#include "playerbot.h"
#include "RandomPlayerbotFactory.h"
#include "AccountMgr.h"
#include "SystemConfig.h"
#include "PlayerbotFactory.h"
#include "RandomItemMgr.h"
#include "World/WorldState.h"
#include "PlayerbotHelpMgr.h"

#include "TravelMgr.h"

#include <iostream>
#include <numeric>
#include <iomanip>
#include <boost/algorithm/string.hpp>

using namespace std;

std::vector<string> ConfigAccess::GetValues(const std::string& name) const
{
    std::vector<string> values;
    auto const nameLower = boost::algorithm::to_lower_copy(name);
    for (auto entry : m_entries)
        if (entry.first.find(nameLower) != string::npos)
            values.push_back(entry.first);

    return values;
};



INSTANTIATE_SINGLETON_1(PlayerbotAIConfig);

PlayerbotAIConfig::PlayerbotAIConfig()
{
}

template <class T>
void LoadList(string value, T &list)
{
    list.clear();
    vector<string> ids = split(value, ',');
    for (vector<string>::iterator i = ids.begin(); i != ids.end(); i++)
    {
        string string = *i;
        if (string.empty())
            continue;

        uint32 id = atoi(string.c_str());

        list.push_back(id);
    }
}

template <class T>
void LoadListString(string value, T& list)
{
    list.clear();
    vector<string> strings = split(value, ',');
    for (vector<string>::iterator i = strings.begin(); i != strings.end(); i++)
    {
        string string = *i;
        if (string.empty())
            continue;

        list.push_back(string);
    }
}


bool PlayerbotAIConfig::Initialize()
{
    sLog.outString("Initializing AI Playerbot by ike3, based on the original Playerbot by blueboy");

    if (!config.SetSource(SYSCONFDIR"aiplayerbot.conf"))
    {
        sLog.outString("AI Playerbot is Disabled. Unable to open configuration file aiplayerbot.conf");
        return false;
    }

    enabled = config.GetBoolDefault("AiPlayerbot.Enabled", true);
    if (!enabled)
    {
        sLog.outString("AI Playerbot is Disabled in aiplayerbot.conf");
        return false;
    }

    ConfigAccess* configA = reinterpret_cast<ConfigAccess*>(&config);

    BarGoLink::SetOutputState(config.GetBoolDefault("AiPlayerbot.ShowProgressBars", false));
    globalCoolDown = (uint32) config.GetIntDefault("AiPlayerbot.GlobalCooldown", 500);
    maxWaitForMove = config.GetIntDefault("AiPlayerbot.MaxWaitForMove", 3000);
    expireActionTime = config.GetIntDefault("AiPlayerbot.ExpireActionTime", 5000);
    dispelAuraDuration = config.GetIntDefault("AiPlayerbot.DispelAuraDuration", 2000);
    reactDelay = (uint32) config.GetIntDefault("AiPlayerbot.ReactDelay", 100);
    passiveDelay = (uint32) config.GetIntDefault("AiPlayerbot.PassiveDelay", 4000);
    repeatDelay = (uint32) config.GetIntDefault("AiPlayerbot.RepeatDelay", 5000);
    errorDelay = (uint32) config.GetIntDefault("AiPlayerbot.ErrorDelay", 5000);
    rpgDelay = (uint32) config.GetIntDefault("AiPlayerbot.RpgDelay", 3000);
    sitDelay = (uint32) config.GetIntDefault("AiPlayerbot.SitDelay", 30000);
    returnDelay = (uint32) config.GetIntDefault("AiPlayerbot.ReturnDelay", 7000);
    lootDelay = (uint32)config.GetIntDefault("AiPlayerbot.LootDelayDelay", 750);

    farDistance = config.GetFloatDefault("AiPlayerbot.FarDistance", 20.0f);
    sightDistance = config.GetFloatDefault("AiPlayerbot.SightDistance", 75.0f);
    spellDistance = config.GetFloatDefault("AiPlayerbot.SpellDistance", 25.0f);
    shootDistance = config.GetFloatDefault("AiPlayerbot.ShootDistance", 25.0f);
    healDistance = config.GetFloatDefault("AiPlayerbot.HealDistance", 125.0f);
    reactDistance = config.GetFloatDefault("AiPlayerbot.ReactDistance", 150.0f);
    grindDistance = config.GetFloatDefault("AiPlayerbot.GrindDistance", 75.0f);
    aggroDistance = config.GetFloatDefault("AiPlayerbot.AggroDistance", 22.0f);
    lootDistance = config.GetFloatDefault("AiPlayerbot.LootDistance", 15.0f);
    fleeDistance = config.GetFloatDefault("AiPlayerbot.FleeDistance", 8.0f);
    tooCloseDistance = config.GetFloatDefault("AiPlayerbot.TooCloseDistance", 5.0f);
    meleeDistance = config.GetFloatDefault("AiPlayerbot.MeleeDistance", 1.5f);
    followDistance = config.GetFloatDefault("AiPlayerbot.FollowDistance", 1.5f);
    raidFollowDistance = config.GetFloatDefault("AiPlayerbot.RaidFollowDistance", 5.0f);
    whisperDistance = config.GetFloatDefault("AiPlayerbot.WhisperDistance", 6000.0f);
    contactDistance = config.GetFloatDefault("AiPlayerbot.ContactDistance", 0.5f);
    aoeRadius = config.GetFloatDefault("AiPlayerbot.AoeRadius", 5.0f);
    rpgDistance = config.GetFloatDefault("AiPlayerbot.RpgDistance", 80.0f);

    criticalHealth = config.GetIntDefault("AiPlayerbot.CriticalHealth", 20);
    lowHealth = config.GetIntDefault("AiPlayerbot.LowHealth", 50);
    mediumHealth = config.GetIntDefault("AiPlayerbot.MediumHealth", 70);
    almostFullHealth = config.GetIntDefault("AiPlayerbot.AlmostFullHealth", 90);
    lowMana = config.GetIntDefault("AiPlayerbot.LowMana", 15);
    mediumMana = config.GetIntDefault("AiPlayerbot.MediumMana", 40);

    randomGearMaxLevel = config.GetIntDefault("AiPlayerbot.RandomGearMaxLevel", 500);
    randomGearMaxDiff = config.GetIntDefault("AiPlayerbot.RandomGearMaxDiff", 9);
    randomGearUpgradeEnabled = config.GetBoolDefault("AiPlayerbot.RandomGearUpgradeEnabled", false);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomGearBlacklist", ""), randomGearBlacklist);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomGearWhitelist", ""), randomGearWhitelist);
    randomGearProgression = config.GetBoolDefault("AiPlayerbot.RandomGearProgression", true);
    randomGearLoweringChance = config.GetFloatDefault("AiPlayerbot.RandomGearLoweringChance", 0.15f);
    randomBotMaxLevelChance = config.GetFloatDefault("AiPlayerbot.RandomBotMaxLevelChance", 0.15f);
    randomBotRpgChance = config.GetFloatDefault("AiPlayerbot.RandomBotRpgChance", 0.35f);
    usePotionChance = config.GetFloatDefault("AiPlayerbot.UsePotionChance", 1.0f);
    attackEmoteChance = config.GetFloatDefault("AiPlayerbot.AttackEmoteChance", 0.0f);

    jumpNoCombatChance = config.GetFloatDefault("AiPlayerbot.JumpNoCombatChance", 0.5f);
    jumpMeleeInCombatChance = config.GetFloatDefault("AiPlayerbot.JumpMeleeInCombatChance", 0.5f);
    jumpRandomChance = config.GetFloatDefault("AiPlayerbot.JumpRandomChance", 0.20f);
    jumpInPlaceChance = config.GetFloatDefault("AiPlayerbot.JumpInPlaceChance", 0.50f);
    jumpBackwardChance = config.GetFloatDefault("AiPlayerbot.JumpBackwardChance", 0.10f);
    jumpHeightLimit = config.GetFloatDefault("AiPlayerbot.JumpHeightLimit", 60.f);
    jumpInBg = config.GetBoolDefault("AiPlayerbot.JumpInBg", false);
    jumpWithPlayer = config.GetBoolDefault("AiPlayerbot.JumpWithPlayer", false);
    jumpFollow = config.GetBoolDefault("AiPlayerbot.JumpFollow", true);
    jumpChase = config.GetBoolDefault("AiPlayerbot.JumpChase", true);
    useKnockback = config.GetBoolDefault("AiPlayerbot.UseKnockback", true);

    iterationsPerTick = config.GetIntDefault("AiPlayerbot.IterationsPerTick", 100);

    allowGuildBots = config.GetBoolDefault("AiPlayerbot.AllowGuildBots", true);
    allowMultiAccountAltBots = config.GetBoolDefault("AiPlayerbot.AllowMultiAccountAltBots", true);

    randomBotMapsAsString = config.GetStringDefault("AiPlayerbot.RandomBotMaps", "0,1,530,571");
    LoadList<vector<uint32> >(randomBotMapsAsString, randomBotMaps);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotQuestItems", "6948,5175,5176,5177,5178,16309,12382,13704,11000,22754"), randomBotQuestItems);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotSpellIds", "54197"), randomBotSpellIds);
	LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.PvpProhibitedZoneIds", "2255,656,2361,2362,2363,976,35,2268,3425,392,541,1446,3828,3712,3738,3565,3539,3623,4152,3988,4658,4284,4418,4436,4275,4323"), pvpProhibitedZoneIds);
    
#ifndef MANGOSBOT_ZERO
    // disable pvp near dark portal if event is active
    if (sWorldState.GetExpansion() == EXPANSION_NONE)
        pvpProhibitedZoneIds.insert(pvpProhibitedZoneIds.begin(), 72);
#endif

    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.RandomBotQuestIds", "7848,3802,5505,6502,7761,9378"), randomBotQuestIds);
    LoadList<list<uint32> >(config.GetStringDefault("AiPlayerbot.ImmuneSpellIds", ""), immuneSpellIds);

    botAutologin = config.GetBoolDefault("AiPlayerbot.BotAutologin", false);
    randomBotAutologin = config.GetBoolDefault("AiPlayerbot.RandomBotAutologin", true);
    minRandomBots = config.GetIntDefault("AiPlayerbot.MinRandomBots", 50);
    maxRandomBots = config.GetIntDefault("AiPlayerbot.MaxRandomBots", 200);
    randomBotUpdateInterval = config.GetIntDefault("AiPlayerbot.RandomBotUpdateInterval", 1);
    randomBotCountChangeMinInterval = config.GetIntDefault("AiPlayerbot.RandomBotCountChangeMinInterval", 1 * 1800);
    randomBotCountChangeMaxInterval = config.GetIntDefault("AiPlayerbot.RandomBotCountChangeMaxInterval", 2 * 3600);
    loginBoostPercentage = config.GetFloatDefault("AiPlayerbot.LoginBoostPercentage", 90);
    randomBotTimedLogout = config.GetBoolDefault("AiPlayerbot.RandomBotTimedLogout", true);
    randomBotTimedOffline = config.GetBoolDefault("AiPlayerbot.RandomBotTimedOffline", false);
    minRandomBotInWorldTime = config.GetIntDefault("AiPlayerbot.MinRandomBotInWorldTime", 1 * 1800);
    maxRandomBotInWorldTime = config.GetIntDefault("AiPlayerbot.MaxRandomBotInWorldTime", 6 * 3600);
    minRandomBotRandomizeTime = config.GetIntDefault("AiPlayerbot.MinRandomBotRandomizeTime", 6 * 3600);
    maxRandomBotRandomizeTime = config.GetIntDefault("AiPlayerbot.MaxRandomRandomizeTime", 24 * 3600);
    minRandomBotChangeStrategyTime = config.GetIntDefault("AiPlayerbot.MinRandomBotChangeStrategyTime", 1800);
    maxRandomBotChangeStrategyTime = config.GetIntDefault("AiPlayerbot.MaxRandomBotChangeStrategyTime", 2 * 3600);
    minRandomBotReviveTime = config.GetIntDefault("AiPlayerbot.MinRandomBotReviveTime", 60);
    maxRandomBotReviveTime = config.GetIntDefault("AiPlayerbot.MaxRandomReviveTime", 300);
    randomBotTeleportDistance = config.GetIntDefault("AiPlayerbot.RandomBotTeleportDistance", 1000);
    randomBotTeleportNearPlayer = config.GetBoolDefault("AiPlayerbot.RandomBotTeleportNearPlayer", false);
    randomBotTeleportNearPlayerMaxAmount = config.GetIntDefault("AiPlayerbot.RandomBotTeleportNearPlayerMaxAmount", 0);
    randomBotTeleportNearPlayerMaxAmountRadius = config.GetFloatDefault("AiPlayerbot.RandomBotTeleportNearPlayerMaxAmountRadius", 0.0f);
    randomBotsPerInterval = config.GetIntDefault("AiPlayerbot.RandomBotsPerInterval", 3);
    randomBotsMaxLoginsPerInterval = config.GetIntDefault("AiPlayerbot.RandomBotsMaxLoginsPerInterval", randomBotsPerInterval);
    minRandomBotsPriceChangeInterval = config.GetIntDefault("AiPlayerbot.MinRandomBotsPriceChangeInterval", 2 * 3600);
    maxRandomBotsPriceChangeInterval = config.GetIntDefault("AiPlayerbot.MaxRandomBotsPriceChangeInterval", 48 * 3600);
    randomBotJoinLfg = config.GetBoolDefault("AiPlayerbot.RandomBotJoinLfg", true);
    randomBotJoinBG = config.GetBoolDefault("AiPlayerbot.RandomBotJoinBG", true);
    randomBotAutoJoinBG = config.GetBoolDefault("AiPlayerbot.RandomBotAutoJoinBG", false);
    randomBotBracketCount = config.GetIntDefault("AiPlayerbot.RandomBotBracketCount", 3);
    logInGroupOnly = config.GetBoolDefault("AiPlayerbot.LogInGroupOnly", true);
    logValuesPerTick = config.GetBoolDefault("AiPlayerbot.LogValuesPerTick", false);
    fleeingEnabled = config.GetBoolDefault("AiPlayerbot.FleeingEnabled", true);
    summonAtInnkeepersEnabled = config.GetBoolDefault("AiPlayerbot.SummonAtInnkeepersEnabled", true);
    randomBotMinLevel = config.GetIntDefault("AiPlayerbot.RandomBotMinLevel", 1);
    randomBotMaxLevel = config.GetIntDefault("AiPlayerbot.RandomBotMaxLevel", 255);
    randomBotLoginAtStartup = config.GetBoolDefault("AiPlayerbot.RandomBotLoginAtStartup", true);
    randomBotTeleLevel = config.GetIntDefault("AiPlayerbot.RandomBotTeleLevel", 5);
    openGoSpell = config.GetIntDefault("AiPlayerbot.OpenGoSpell", 6477);

    randomChangeMultiplier = config.GetFloatDefault("AiPlayerbot.RandomChangeMultiplier", 1.0);

    randomBotCombatStrategies = config.GetStringDefault("AiPlayerbot.RandomBotCombatStrategies", "-threat,+custom::say");
    randomBotNonCombatStrategies = config.GetStringDefault("AiPlayerbot.RandomBotNonCombatStrategies", "+custom::say");
    randomBotReactStrategies = config.GetStringDefault("AiPlayerbot.RandomBotReactStrategies", "");
    randomBotDeadStrategies = config.GetStringDefault("AiPlayerbot.RandomBotDeadStrategies", "");
    combatStrategies = config.GetStringDefault("AiPlayerbot.CombatStrategies", "");
    nonCombatStrategies = config.GetStringDefault("AiPlayerbot.NonCombatStrategies", "+return,+delayed roll");
    reactStrategies = config.GetStringDefault("AiPlayerbot.ReactStrategies", "");
    deadStrategies = config.GetStringDefault("AiPlayerbot.DeadStrategies", "");

    commandPrefix = config.GetStringDefault("AiPlayerbot.CommandPrefix", "");
    commandSeparator = config.GetStringDefault("AiPlayerbot.CommandSeparator", "\\\\");

    commandServerPort = config.GetIntDefault("AiPlayerbot.CommandServerPort", 0);
    perfMonEnabled = config.GetBoolDefault("AiPlayerbot.PerfMonEnabled", false);

    randomBotLoginWithPlayer = config.GetBoolDefault("AiPlayerbot.RandomBotLoginWithPlayer", false);

    sLog.outString("Loading Race/Class probabilities");

    classRaceProbabilityTotal = 0;

    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        //Set race defaults
        if (race > 0)
        {
            int rProb = config.GetIntDefault("AiPlayerbot.ClassRaceProb.0." + to_string(race), 100);

            for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
            {
                classRaceProbability[cls][race] = rProb;
            }
        }
    }

    //Class overrides
    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        int cProb = config.GetIntDefault("AiPlayerbot.ClassRaceProb." + to_string(cls), -1);

        if (cProb >= 0)
        {
            for (uint32 race = 1; race < MAX_RACES; ++race)
            {
                classRaceProbability[cls][race] = cProb;
            }
        }
    }

    RandomPlayerbotFactory factory(0);

    //Race Class overrides
    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            int rcProb = config.GetIntDefault("AiPlayerbot.ClassRaceProb." + to_string(cls) + "." + to_string(race), -1);
            if (rcProb >= 0)
                classRaceProbability[cls][race] = rcProb;

            if (!factory.isAvailableRace(cls, race))
                classRaceProbability[cls][race] = 0;
            else
                classRaceProbabilityTotal += classRaceProbability[cls][race];
        }
    }

    sLog.outString("Loading TalentSpecs");

    for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
    {
        classSpecs[cls] = ClassSpecs(1 << (cls - 1));
        for (uint32 spec = 0; spec < MAX_LEVEL; ++spec)
        {
            ostringstream os; os << "AiPlayerbot.PremadeSpecName." << cls << "." << spec;
            string specName = config.GetStringDefault(os.str().c_str(), "");
            if (!specName.empty())
            {
                ostringstream os; os << "AiPlayerbot.PremadeSpecProb." << cls << "." << spec;
                int probability = config.GetIntDefault(os.str().c_str(), 100);

                TalentPath talentPath(spec, specName, probability);

                for (int level = 10; level <= 100; level++)
                {
                    ostringstream os; os << "AiPlayerbot.PremadeSpecLink." << cls << "." << spec << "." << level;
                    string specLink = config.GetStringDefault(os.str().c_str(), "");
                    specLink = specLink.substr(0, specLink.find("#", 0));;
                    specLink = specLink.substr(0, specLink.find(" ", 0));;

                    if (!specLink.empty())
                    {
                        ostringstream out;

                        //Ignore bad specs.
                        if (!classSpecs[cls].baseSpec.CheckTalentLink(specLink, &out))
                        {
                            sLog.outErrorDb("Error with premade spec link: %s", specLink.c_str());
                            sLog.outErrorDb("%s", out.str().c_str());
                            continue;
                        }

                        TalentSpec linkSpec(&classSpecs[cls].baseSpec, specLink);

                        if (!linkSpec.CheckTalents(level, &out))
                        {
                            sLog.outErrorDb("Error with premade spec: %s", specLink.c_str());
                            sLog.outErrorDb("%s", out.str().c_str());
                            continue;
                        }


                        talentPath.talentSpec.push_back(linkSpec);
                    }
                }

                //Only add paths that have atleast 1 spec.
                if(talentPath.talentSpec.size() > 0)
                    classSpecs[cls].talentPath.push_back(talentPath);
            }
        }
    }

    botCheats.clear();
    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.BotCheats", "taxi,item,breath"), botCheats);

    botCheatMask = 0;

    if (std::find(botCheats.begin(), botCheats.end(), "taxi") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::taxi;
    if (std::find(botCheats.begin(), botCheats.end(), "gold") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::gold;
    if (std::find(botCheats.begin(), botCheats.end(), "health") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::health;
    if (std::find(botCheats.begin(), botCheats.end(), "mana") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::mana;
    if (std::find(botCheats.begin(), botCheats.end(), "power") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::power;
    if (std::find(botCheats.begin(), botCheats.end(), "item") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::item;
    if (std::find(botCheats.begin(), botCheats.end(), "cooldown") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::cooldown;
    if (std::find(botCheats.begin(), botCheats.end(), "repair") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::repair;
    if (std::find(botCheats.begin(), botCheats.end(), "movespeed") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::movespeed;
    if (std::find(botCheats.begin(), botCheats.end(), "attackspeed") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::attackspeed;
    if (std::find(botCheats.begin(), botCheats.end(), "breath") != botCheats.end())
        botCheatMask |= (uint32)BotCheatMask::breath;

    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.AllowedLogFiles", ""), allowedLogFiles);
    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.DebugFilter", "add gathering loot,check values,emote,check mount state"), debugFilter);

    worldBuffs.clear();

    //Get all config values starting with AiPlayerbot.WorldBuff
    vector<string> values = configA->GetValues("AiPlayerbot.WorldBuff");
   
    if (values.size())
    {
        sLog.outString("Loading WorldBuffs");
        BarGoLink wbuffBar(values.size());

        for (auto value : values)
        {
            vector<string> ids = split(value, '.');
            vector<uint32> params = { 0,0,0,0,0 };

            //Extract faction, class, spec, minlevel, maxlevel
            for (uint8 i = 0; i < 5; i++)
                if (ids.size() > i + 2)
                    params[i] = stoi(ids[i + 2]);

            //Get list of buffs for this combination.
            list<uint32> buffs;
            LoadList<list<uint32>>(config.GetStringDefault(value, ""), buffs);

            //Store buffs for later application.
            for (auto buff : buffs)
            {
                worldBuff wb = { buff, params[0], params[1], params[2], params[3], params[4] };
                worldBuffs.push_back(wb);
            }

            wbuffBar.step();
        }
    }

    randomBotAccountPrefix = config.GetStringDefault("AiPlayerbot.RandomBotAccountPrefix", "rndbot");
    randomBotAccountCount = config.GetIntDefault("AiPlayerbot.RandomBotAccountCount", 50);
    deleteRandomBotAccounts = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotAccounts", false);
    randomBotGuildCount = config.GetIntDefault("AiPlayerbot.RandomBotGuildCount", 20);
    deleteRandomBotGuilds = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotGuilds", false);

    //arena
    randomBotArenaTeamCount = config.GetIntDefault("AiPlayerbot.RandomBotArenaTeamCount", 20);
    deleteRandomBotArenaTeams = config.GetBoolDefault("AiPlayerbot.DeleteRandomBotArenaTeams", false);

    //cosmetics (by lidocain)
    randomBotShowCloak = config.GetBoolDefault("AiPlayerbot.RandomBotShowCloak", false);
    randomBotShowHelmet = config.GetBoolDefault("AiPlayerbot.RandomBotShowHelmet", false);

	//SPP switches
    enableGreet = config.GetBoolDefault("AiPlayerbot.EnableGreet", false);
	disableRandomLevels = config.GetBoolDefault("AiPlayerbot.DisableRandomLevels", false);
    instantRandomize = config.GetBoolDefault("AiPlayerbot.InstantRandomize", true);
    randomBotRandomPassword = config.GetBoolDefault("AiPlayerbot.RandomBotRandomPassword", true);
    playerbotsXPrate = config.GetFloatDefault("AiPlayerbot.XPRate", 1.0f);
    botActiveAlone = config.GetIntDefault("AiPlayerbot.botActiveAlone", 10);
    diffWithPlayer = config.GetIntDefault("AiPlayerbot.DiffWithPlayer", 100);
    diffEmpty = config.GetIntDefault("AiPlayerbot.DiffEmpty", 200);
    RandombotsWalkingRPG = config.GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG", false);
    RandombotsWalkingRPGInDoors = config.GetBoolDefault("AiPlayerbot.RandombotsWalkingRPG.InDoors", false);
    minEnchantingBotLevel = config.GetIntDefault("AiPlayerbot.minEnchantingBotLevel", 60);
    randombotStartingLevel = config.GetIntDefault("AiPlayerbot.randombotStartingLevel", 5);
    gearscorecheck = config.GetBoolDefault("AiPlayerbot.GearScoreCheck", false);
    levelCheck = config.GetIntDefault("AiPlayerbot.LevelCheck", 30);
	randomBotPreQuests = config.GetBoolDefault("AiPlayerbot.PreQuests", true);
    randomBotSayWithoutMaster = config.GetBoolDefault("AiPlayerbot.RandomBotSayWithoutMaster", false);
    randomBotInvitePlayer = config.GetBoolDefault("AiPlayerbot.RandomBotInvitePlayer", true);
    randomBotGroupNearby = config.GetBoolDefault("AiPlayerbot.RandomBotGroupNearby", true);
    randomBotRaidNearby = config.GetBoolDefault("AiPlayerbot.RandomBotRaidNearby", true);
    randomBotGuildNearby = config.GetBoolDefault("AiPlayerbot.RandomBotGuildNearby", true);
    inviteChat = config.GetBoolDefault("AiPlayerbot.InviteChat", true);
    guildFeedbackRate = config.GetFloatDefault("AiPlayerbot.GuildFeedbackRate", 100.0f);
    guildSuggestRate = config.GetFloatDefault("AiPlayerbot.GuildSuggestRate", 100.0f);
    guildRepliesRate= config.GetFloatDefault("AiPlayerbot.GuildRepliesRate", 100.0f);
    
    randomBotFormGuild = config.GetBoolDefault("AiPlayerbot.RandomBotFormGuild", true);
    
    boostFollow = config.GetBoolDefault("AiPlayerbot.BoostFollow", false);
    turnInRpg = config.GetBoolDefault("AiPlayerbot.TurnInRpg", false);
    globalSoundEffects = config.GetBoolDefault("AiPlayerbot.GlobalSoundEffects", false);
    nonGmFreeSummon = config.GetBoolDefault("AiPlayerbot.NonGmFreeSummon", false);
    
    //SPP automation
    autoPickReward = config.GetStringDefault("AiPlayerbot.AutoPickReward", "no");
    autoEquipUpgradeLoot = config.GetBoolDefault("AiPlayerbot.AutoEquipUpgradeLoot", false);
    syncQuestWithPlayer = config.GetBoolDefault("AiPlayerbot.SyncQuestWithPlayer", false);
    syncQuestForPlayer = config.GetBoolDefault("AiPlayerbot.SyncQuestForPlayer", false);
    autoTrainSpells = config.GetStringDefault("AiPlayerbot.AutoTrainSpells", "no");
    autoPickTalents = config.GetStringDefault("AiPlayerbot.AutoPickTalents", "no");
    autoLearnTrainerSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnTrainerSpells", false);
    autoLearnQuestSpells = config.GetBoolDefault("AiPlayerbot.AutoLearnQuestSpells", false);
    autoDoQuests = config.GetBoolDefault("AiPlayerbot.AutoDoQuests", false);
    syncLevelWithPlayers = config.GetBoolDefault("AiPlayerbot.SyncLevelWithPlayers", false);
    syncLevelMaxAbove = config.GetIntDefault("AiPlayerbot.SyncLevelMaxAbove", 5);
    syncLevelNoPlayer = config.GetIntDefault("AiPlayerbot.SyncLevelNoPlayer", randombotStartingLevel);
    tweakValue = config.GetIntDefault("AiPlayerbot.TweakValue", 0);
    talentsInPublicNote = config.GetBoolDefault("AiPlayerbot.TalentsInPublicNote", false);
    respawnModNeutral = config.GetFloatDefault("AiPlayerbot.RespawnModNeutral", 10.0f);
    respawnModHostile = config.GetFloatDefault("AiPlayerbot.RespawnModHostile", 5.0f);
    respawnModThreshold = config.GetIntDefault("AiPlayerbot.RespawnModThreshold", 10);
    respawnModMax = config.GetIntDefault("AiPlayerbot.RespawnModMax", 18);
    respawnModForPlayerBots = config.GetBoolDefault("AiPlayerbot.RespawnModForPlayerBots", false);
    respawnModForInstances = config.GetBoolDefault("AiPlayerbot.RespawnModForInstances", false);

    // Gear progression system
    gearProgressionSystemEnabled = config.GetBoolDefault("AiPlayerbot.GearProgressionSystem.Enable", false);

    // Gear progression phase
    for (uint8 phase = 0; phase < MAX_GEAR_PROGRESSION_LEVEL; phase++)
    {
        ostringstream os; os << "AiPlayerbot.GearProgressionSystem." << std::to_string(phase) << ".MinItemLevel";
        gearProgressionSystemItemLevels[phase][0] = config.GetIntDefault(os.str().c_str(), 9999999);
        os.str(""); os << "AiPlayerbot.GearProgressionSystem." << std::to_string(phase) << ".MaxItemLevel";
        gearProgressionSystemItemLevels[phase][1] = config.GetIntDefault(os.str().c_str(), 9999999);

        // Gear progression class
        for (uint8 cls = 1; cls < MAX_CLASSES; cls++)
        {
            // Gear progression spec
            for (uint8 spec = 0; spec < 4; spec++)
            {
                // Gear progression slot
                for (uint8 slot = 0; slot < SLOT_EMPTY; slot++)
                {
                    ostringstream os; os << "AiPlayerbot.GearProgressionSystem." << std::to_string(phase) << "." << std::to_string(cls) << "." << std::to_string(spec) << "." << std::to_string(slot);
                    gearProgressionSystemItems[phase][cls][spec][slot] = config.GetIntDefault(os.str().c_str(), -1);
                }
            }
        }
    }

    sLog.outString("Loading free bots.");
    selfBotLevel = config.GetIntDefault("AiPlayerbot.SelfBotLevel", 1);
    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.ToggleAlwaysOnlineAccounts", ""), toggleAlwaysOnlineAccounts);
    LoadListString<list<string>>(config.GetStringDefault("AiPlayerbot.ToggleAlwaysOnlineChars", ""), toggleAlwaysOnlineChars);

    for (string& nm : toggleAlwaysOnlineAccounts)
        transform(nm.begin(), nm.end(), nm.begin(), ::toupper);

    for (string& nm : toggleAlwaysOnlineChars)
    {
        transform(nm.begin(), nm.end(), nm.begin(), ::tolower);
        nm[0] = toupper(nm[0]);
    }

    loadFreeAltBotAccounts();

    targetPosRecalcDistance = config.GetFloatDefault("AiPlayerbot.TargetPosRecalcDistance", 0.1f),

    sLog.outString("Loading area levels.");
    sTravelMgr.loadAreaLevels();
    sLog.outString("Loading spellIds.");
    ChatHelper::PopulateSpellNameList();

    RandomPlayerbotFactory::CreateRandomBots();
    PlayerbotFactory::Init();
    sRandomItemMgr.Init();
    auctionbot.Init();
    sRandomItemMgr.InitAfterAhBot();
    sPlayerbotTextMgr.LoadBotTexts();
    sPlayerbotTextMgr.LoadBotTextChance();
    sPlayerbotHelpMgr.LoadBotHelpTexts();

    if (sPlayerbotAIConfig.autoDoQuests)
    {
        sLog.outString("Loading Quest Detail Data...");
        sTravelMgr.LoadQuestTravelTable();
    }

    if (sPlayerbotAIConfig.randomBotJoinBG)
        sRandomPlayerbotMgr.LoadBattleMastersCache();

    sLog.outString("---------------------------------------");
    sLog.outString("        AI Playerbot initialized       ");
    sLog.outString("---------------------------------------");
    sLog.outString();

    return true;
}

bool PlayerbotAIConfig::IsInRandomAccountList(uint32 id)
{
    return find(randomBotAccounts.begin(), randomBotAccounts.end(), id) != randomBotAccounts.end();
}

bool PlayerbotAIConfig::IsFreeAltBot(uint32 guid)
{
    for (auto bot : freeAltBots)
        if (bot.second == guid)
            return true;

    return false;
}

bool PlayerbotAIConfig::IsInRandomQuestItemList(uint32 id)
{
    return find(randomBotQuestItems.begin(), randomBotQuestItems.end(), id) != randomBotQuestItems.end();
}

bool PlayerbotAIConfig::IsInPvpProhibitedZone(uint32 id)
{
	return find(pvpProhibitedZoneIds.begin(), pvpProhibitedZoneIds.end(), id) != pvpProhibitedZoneIds.end();
}

string PlayerbotAIConfig::GetValue(string name)
{
    ostringstream out;

    if (name == "GlobalCooldown")
        out << globalCoolDown;
    else if (name == "ReactDelay")
        out << reactDelay;

    else if (name == "SightDistance")
        out << sightDistance;
    else if (name == "SpellDistance")
        out << spellDistance;
    else if (name == "ReactDistance")
        out << reactDistance;
    else if (name == "GrindDistance")
        out << grindDistance;
    else if (name == "LootDistance")
        out << lootDistance;
    else if (name == "FleeDistance")
        out << fleeDistance;

    else if (name == "CriticalHealth")
        out << criticalHealth;
    else if (name == "LowHealth")
        out << lowHealth;
    else if (name == "MediumHealth")
        out << mediumHealth;
    else if (name == "AlmostFullHealth")
        out << almostFullHealth;
    else if (name == "LowMana")
        out << lowMana;

    else if (name == "IterationsPerTick")
        out << iterationsPerTick;

    return out.str();
}

void PlayerbotAIConfig::SetValue(string name, string value)
{
    istringstream out(value, istringstream::in);

    if (name == "GlobalCooldown")
        out >> globalCoolDown;
    else if (name == "ReactDelay")
        out >> reactDelay;

    else if (name == "SightDistance")
        out >> sightDistance;
    else if (name == "SpellDistance")
        out >> spellDistance;
    else if (name == "ReactDistance")
        out >> reactDistance;
    else if (name == "GrindDistance")
        out >> grindDistance;
    else if (name == "LootDistance")
        out >> lootDistance;
    else if (name == "FleeDistance")
        out >> fleeDistance;

    else if (name == "CriticalHealth")
        out >> criticalHealth;
    else if (name == "LowHealth")
        out >> lowHealth;
    else if (name == "MediumHealth")
        out >> mediumHealth;
    else if (name == "AlmostFullHealth")
        out >> almostFullHealth;
    else if (name == "LowMana")
        out >> lowMana;

    else if (name == "IterationsPerTick")
        out >> iterationsPerTick;
}

void PlayerbotAIConfig::loadFreeAltBotAccounts()
{
    bool allCharsOnline = (selfBotLevel > 3);

    freeAltBots.clear();

    auto results = LoginDatabase.PQuery("SELECT username, id FROM account where username not like '%s%%'", randomBotAccountPrefix.c_str());
    if (results)
    {
        do
        {
            bool accountAlwaysOnline = allCharsOnline;

            Field* fields = results->Fetch();
            string accountName = fields[0].GetString();
            uint32 accountId = fields[1].GetUInt32();

            if (std::find(toggleAlwaysOnlineAccounts.begin(), toggleAlwaysOnlineAccounts.end(), accountName) != toggleAlwaysOnlineAccounts.end())
                accountAlwaysOnline = !accountAlwaysOnline;                       

            auto result = CharacterDatabase.PQuery("SELECT name, guid FROM characters WHERE account = '%u'", accountId);
            if (!result)
                continue;

            do
            {
                bool charAlwaysOnline = allCharsOnline;

                Field* fields = result->Fetch();
                string charName = fields[0].GetString();
                uint32 guid = fields[1].GetUInt32();

                uint32 always = sRandomPlayerbotMgr.GetValue(guid, "always");

                if (always == 2)
                    continue;

                if (std::find(toggleAlwaysOnlineChars.begin(), toggleAlwaysOnlineChars.end(), charName) != toggleAlwaysOnlineChars.end())
                    charAlwaysOnline = !charAlwaysOnline;

                if(charAlwaysOnline || accountAlwaysOnline || always)
                    freeAltBots.push_back(make_pair(accountId, guid));

            } while (result->NextRow());
        

        } while (results->NextRow());
    }
}

std::string PlayerbotAIConfig::GetTimestampStr()
{
    time_t t = time(nullptr);
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
    char buf[20];
    snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:%02d", aTm->tm_year + 1900, aTm->tm_mon + 1, aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec);
    return std::string(buf);
}

bool PlayerbotAIConfig::openLog(string fileName, char const* mode)
{
    if (!hasLog(fileName))
        return false;
     
    auto logFileIt = logFiles.find(fileName);
    if (logFileIt == logFiles.end())
    {
        logFiles.insert(make_pair(fileName, make_pair(nullptr, false)));
        logFileIt = logFiles.find(fileName);
    }

    FILE* file = logFileIt->second.first;
    bool fileOpen = logFileIt->second.second;

    if (fileOpen) //close log file
        fclose(file);

    string m_logsDir = sConfig.GetStringDefault("LogsDir");
    if (!m_logsDir.empty())
    {
        if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
            m_logsDir.append("/");
    }


    file = fopen((m_logsDir + fileName).c_str(), mode);
    fileOpen = true;

    logFileIt->second.first = file;
    logFileIt->second.second = fileOpen;
    
    return true;
}

void PlayerbotAIConfig::log(string fileName, const char* str, ...)
{
    if (!str)
        return;

    std::lock_guard<std::mutex> guard(m_logMtx);

    if (!isLogOpen(fileName))
        if (!openLog(fileName, "a"))
            return;

    FILE* file = logFiles.find(fileName)->second.first;

    va_list ap;
    va_start(ap, str);
    vfprintf(file, str, ap);
    fprintf(file, "\n");
    va_end(ap);
    fflush(file);

    fflush(stdout);
}

void PlayerbotAIConfig::logEvent(PlayerbotAI* ai, string eventName, string info1, string info2)
{
    if (hasLog("bot_events.csv"))
    {
        Player* bot = ai->GetBot();

        ostringstream out;
        out << sPlayerbotAIConfig.GetTimestampStr() << "+00,";
        out << bot->GetName() << ",";
        out << eventName << ",";
        out << std::fixed << std::setprecision(2);
        WorldPosition(bot).printWKT(out);

        out << to_string(bot->getRace()) << ",";
        out << to_string(bot->getClass()) << ",";
        float subLevel = ((float)bot->GetLevel() + (bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP) ? ((float)bot->GetUInt32Value(PLAYER_XP) / (float)bot->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)) : 0));

        out << subLevel << ",";

        out << "\"" << info1 << "\",";
        out << "\"" << info2 << "\"";

        log("bot_events.csv", out.str().c_str());
    }
};

void PlayerbotAIConfig::logEvent(PlayerbotAI* ai, string eventName, ObjectGuid guid, string info2)
{
    string info1 = "";

    Unit* victim;
    if (guid)
    {
        victim = ai->GetUnit(guid);
        if (victim)
            info1 = victim->GetName();
    }

    logEvent(ai, eventName, info1, info2);
};

bool PlayerbotAIConfig::CanLogAction(PlayerbotAI* ai, string actionName, bool isExecute, string lastActionName)
{
    bool forRpg = (actionName.find("rpg") == 0) && ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT);

    if (!forRpg)
    {
        if (isExecute && !ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT))
            return false;

        if (!isExecute && !ai->HasStrategy("debug action", BotState::BOT_STATE_NON_COMBAT))
            return false;

        if ((lastActionName == actionName) && (actionName == "melee"))
        {
            return false;
        }
    }

    return std::find(debugFilter.begin(), debugFilter.end(), actionName) == debugFilter.end(); 
}
