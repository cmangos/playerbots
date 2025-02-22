#include "PetActions.h"
#include "playerbot/PlayerbotFactory.h"
#include "playerbot/playerbot.h"
#include <playerbot/ServerFacade.h>
using namespace ai;
#define MANGOSBOT_ONE 1

bool InitializePetAction::Execute(Event& event)
{
    PlayerbotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_LEGENDARY);
    factory.InitPet();
    InitialFamilySkill(bot);
    return true;
}

bool InitializePetAction::isUseful()
{
    // Only for random bots with item cheats enabled
    if ((ai->HasCheat(BotCheatMask::item) && sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId())) ||
        // Or if alt bot and autoLearnTrainerSpells is true
        (!sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()) && sPlayerbotAIConfig.autoLearnTrainerSpells))
    {
        if (bot->GetLevel() >= 10 && bot->getClass() == CLASS_HUNTER)
        {
            bool hasTamedPet = bot->GetPet();
            if (!hasTamedPet)
            {
                std::unique_ptr<QueryResult> queryResult = CharacterDatabase.PQuery("SELECT id, entry, owner "
                    "FROM character_pet WHERE owner = '%u' AND (slot = '%u' OR slot > '%u') ",
                    bot->GetGUIDHigh(), PET_SAVE_AS_CURRENT, PET_SAVE_LAST_STABLE_SLOT);

                if (queryResult)
                {
                    Field* fields = queryResult->Fetch();
                    const uint32 entry = fields[1].GetUInt32();
                    hasTamedPet = ObjectMgr::GetCreatureTemplate(entry);
                }
            }
            return !hasTamedPet;
        }
    }
    return false;
}

bool InitializePetSpellsAction::Execute(Event& event)
{
    PlayerbotFactory factory(bot, bot->GetLevel(), ITEM_QUALITY_LEGENDARY);
    factory.InitPetSpells();
    return true;
}

bool InitializePetSpellsAction::isUseful()
{
    // Only for random bots
    if (sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()) ||
        // Or if alt bot and autoLearnTrainerSpells is true
        (!sPlayerbotAIConfig.IsInRandomAccountList(bot->GetSession()->GetAccountId()) && sPlayerbotAIConfig.autoLearnTrainerSpells))
    {
        if (bot->getClass() == CLASS_HUNTER && bot->GetLevel() >= 10)
        {
            bool hasTamedPet = bot->GetPet();
            if (hasTamedPet)
            {

                HunterPetBuild currentBuild = HunterPetBuild(bot);
                HunterPetBuild* proposedBuild;
                std::string proposedBuildLink = "";
                std::string currentBuildLink = currentBuild.GetBuildLink();
                HunterPetBuildPath hpbp;
                switch (sPlayerbotDbStore.PetHasBuilds(bot->GetPet()->GetCharmInfo()->GetPetNumber()))
                {
                    case 0:
                        return true;
                    case 1:
                        hpbp = sPlayerbotDbStore.LoadPetBuildPath(bot->GetPet()->GetCharmInfo()->GetPetNumber());
                        proposedBuild = GetBestPremadeBuild(hpbp.id);
                        proposedBuildLink = proposedBuild->GetBuildLink();
                        return !(proposedBuildLink == currentBuildLink);
                    case 2:
                        proposedBuildLink = sPlayerbotDbStore.LoadPetBuildLink(bot->GetPet()->GetCharmInfo()->GetPetNumber());
                        return !(proposedBuildLink == currentBuildLink);
                    default:
                        // incorrect value returned
                        return false;
                }
            }
            return false;
        }
#ifndef MANGOSBOT_TWO
        // Warlock pets should auto learn spells in WOTLK
        else if (bot->getClass() == CLASS_WARLOCK)
        {
            // Only initialize if warlock has the pet summoned
            Pet* pet = bot->GetPet();
            if (pet)
            {
                constexpr uint32 PET_IMP = 416;
                constexpr uint32 PET_FELHUNTER = 417;
                constexpr uint32 PET_VOIDWALKER = 1860;
                constexpr uint32 PET_SUCCUBUS = 1863;
                constexpr uint32 PET_FELGUARD = 17252;

                //      pet type                    pet level  pet spell id
                std::map<uint32, std::vector<std::pair<uint32, uint32>>> spellList;

                // Imp spells
                {
                    // Blood Pact
                    spellList[PET_IMP].push_back(std::pair(4, 6307));
                    spellList[PET_IMP].push_back(std::pair(14, 7804));
                    spellList[PET_IMP].push_back(std::pair(26, 7805));
                    spellList[PET_IMP].push_back(std::pair(38, 11766));
                    spellList[PET_IMP].push_back(std::pair(50, 11767));
                    spellList[PET_IMP].push_back(std::pair(62, 27268));
                    spellList[PET_IMP].push_back(std::pair(74, 47982));

                    // Fire Shield
                    spellList[PET_IMP].push_back(std::pair(14, 2947));
                    spellList[PET_IMP].push_back(std::pair(24, 8316));
                    spellList[PET_IMP].push_back(std::pair(34, 8317));
                    spellList[PET_IMP].push_back(std::pair(44, 11770));
                    spellList[PET_IMP].push_back(std::pair(54, 11771));
                    spellList[PET_IMP].push_back(std::pair(64, 27269));
                    spellList[PET_IMP].push_back(std::pair(76, 47983));

                    // Firebolt
                    spellList[PET_IMP].push_back(std::pair(1, 3110));
                    spellList[PET_IMP].push_back(std::pair(8, 7799));
                    spellList[PET_IMP].push_back(std::pair(18, 7800));
                    spellList[PET_IMP].push_back(std::pair(28, 7801));
                    spellList[PET_IMP].push_back(std::pair(38, 7802));
                    spellList[PET_IMP].push_back(std::pair(48, 11762));
                    spellList[PET_IMP].push_back(std::pair(58, 11763));
                    spellList[PET_IMP].push_back(std::pair(68, 27267));
                    spellList[PET_IMP].push_back(std::pair(78, 47964));

                    // Phase Shift
                    spellList[PET_IMP].push_back(std::pair(12, 4511));
                }

                // Felhunter spells
                {
                    // Devour Magic
                    spellList[PET_FELHUNTER].push_back(std::pair(30, 19505));
                    spellList[PET_FELHUNTER].push_back(std::pair(38, 19731));
                    spellList[PET_FELHUNTER].push_back(std::pair(46, 19734));
                    spellList[PET_FELHUNTER].push_back(std::pair(54, 19736));
                    spellList[PET_FELHUNTER].push_back(std::pair(62, 27276));
                    spellList[PET_FELHUNTER].push_back(std::pair(70, 27277));
                    spellList[PET_FELHUNTER].push_back(std::pair(77, 48011));

                    // Paranoia
                    spellList[PET_FELHUNTER].push_back(std::pair(42, 19480));

                    // Spell Lock
                    spellList[PET_FELHUNTER].push_back(std::pair(36, 19244));
                    spellList[PET_FELHUNTER].push_back(std::pair(52, 19647));

                    // Tainted Blood
                    spellList[PET_FELHUNTER].push_back(std::pair(32, 19478));
                    spellList[PET_FELHUNTER].push_back(std::pair(40, 19655));
                    spellList[PET_FELHUNTER].push_back(std::pair(48, 19656));
                    spellList[PET_FELHUNTER].push_back(std::pair(56, 19660));
                    spellList[PET_FELHUNTER].push_back(std::pair(64, 27280));
                }

                // Voidwalker spells
                {
                    // Consume Shadows
                    spellList[PET_VOIDWALKER].push_back(std::pair(18, 17767));
                    spellList[PET_VOIDWALKER].push_back(std::pair(26, 17850));
                    spellList[PET_VOIDWALKER].push_back(std::pair(34, 17851));
                    spellList[PET_VOIDWALKER].push_back(std::pair(42, 17852));
                    spellList[PET_VOIDWALKER].push_back(std::pair(50, 17853));
                    spellList[PET_VOIDWALKER].push_back(std::pair(58, 17854));
                    spellList[PET_VOIDWALKER].push_back(std::pair(66, 27272));
                    spellList[PET_VOIDWALKER].push_back(std::pair(73, 47987));
                    spellList[PET_VOIDWALKER].push_back(std::pair(78, 47988));

                    // Sacrifice
                    spellList[PET_VOIDWALKER].push_back(std::pair(16, 7812));
                    spellList[PET_VOIDWALKER].push_back(std::pair(24, 19438));
                    spellList[PET_VOIDWALKER].push_back(std::pair(32, 19440));
                    spellList[PET_VOIDWALKER].push_back(std::pair(40, 19441));
                    spellList[PET_VOIDWALKER].push_back(std::pair(48, 19442));
                    spellList[PET_VOIDWALKER].push_back(std::pair(56, 19443));
                    spellList[PET_VOIDWALKER].push_back(std::pair(64, 27273));
                    spellList[PET_VOIDWALKER].push_back(std::pair(72, 47985));
                    spellList[PET_VOIDWALKER].push_back(std::pair(79, 47986));

                    // Suffering
                    spellList[PET_VOIDWALKER].push_back(std::pair(24, 17735));
                    spellList[PET_VOIDWALKER].push_back(std::pair(36, 17750));
                    spellList[PET_VOIDWALKER].push_back(std::pair(48, 17751));
                    spellList[PET_VOIDWALKER].push_back(std::pair(60, 17752));
                    spellList[PET_VOIDWALKER].push_back(std::pair(63, 27271));
                    spellList[PET_VOIDWALKER].push_back(std::pair(69, 33701));
                    spellList[PET_VOIDWALKER].push_back(std::pair(75, 47989));
                    spellList[PET_VOIDWALKER].push_back(std::pair(80, 47990));

                    // Torment
                    spellList[PET_VOIDWALKER].push_back(std::pair(10, 3716));
                    spellList[PET_VOIDWALKER].push_back(std::pair(20, 7809));
                    spellList[PET_VOIDWALKER].push_back(std::pair(30, 7810));
                    spellList[PET_VOIDWALKER].push_back(std::pair(40, 7811));
                    spellList[PET_VOIDWALKER].push_back(std::pair(50, 11774));
                    spellList[PET_VOIDWALKER].push_back(std::pair(60, 11775));
                    spellList[PET_VOIDWALKER].push_back(std::pair(70, 27270));
                    spellList[PET_VOIDWALKER].push_back(std::pair(80, 47984));
                }

                // Succubus spells
                {
                    // Lash of Pain
                    spellList[PET_SUCCUBUS].push_back(std::pair(20, 7814));
                    spellList[PET_SUCCUBUS].push_back(std::pair(28, 7815));
                    spellList[PET_SUCCUBUS].push_back(std::pair(36, 7816));
                    spellList[PET_SUCCUBUS].push_back(std::pair(44, 11778));
                    spellList[PET_SUCCUBUS].push_back(std::pair(52, 11779));
                    spellList[PET_SUCCUBUS].push_back(std::pair(60, 11780));
                    spellList[PET_SUCCUBUS].push_back(std::pair(68, 27274));
                    spellList[PET_SUCCUBUS].push_back(std::pair(74, 47991));
                    spellList[PET_SUCCUBUS].push_back(std::pair(80, 47992));

                    // Lesser Invisibility
                    spellList[PET_SUCCUBUS].push_back(std::pair(32, 7870));

                    // Seduction
                    spellList[PET_SUCCUBUS].push_back(std::pair(26, 6358));

                    // Soothing Kiss
                    spellList[PET_SUCCUBUS].push_back(std::pair(22, 6360));
                    spellList[PET_SUCCUBUS].push_back(std::pair(34, 7813));
                    spellList[PET_SUCCUBUS].push_back(std::pair(46, 11784));
                    spellList[PET_SUCCUBUS].push_back(std::pair(58, 11785));
                    spellList[PET_SUCCUBUS].push_back(std::pair(70, 27275));
                }

                // Felguard spells
                {
                    // Anguish
                    spellList[PET_FELGUARD].push_back(std::pair(50, 33698));
                    spellList[PET_FELGUARD].push_back(std::pair(60, 33699));
                    spellList[PET_FELGUARD].push_back(std::pair(69, 33700));
                    spellList[PET_FELGUARD].push_back(std::pair(78, 47993));

                    // Avoidance
                    spellList[PET_FELGUARD].push_back(std::pair(60, 32233));

                    // Cleave
                    spellList[PET_FELGUARD].push_back(std::pair(50, 30213));
                    spellList[PET_FELGUARD].push_back(std::pair(60, 30219));
                    spellList[PET_FELGUARD].push_back(std::pair(68, 30223));
                    spellList[PET_FELGUARD].push_back(std::pair(76, 47994));

                    // Demonic Frenzy
                    spellList[PET_FELGUARD].push_back(std::pair(56, 32850));

                    // Intercept
                    spellList[PET_FELGUARD].push_back(std::pair(52, 30151));
                    spellList[PET_FELGUARD].push_back(std::pair(61, 30194));
                    spellList[PET_FELGUARD].push_back(std::pair(69, 30198));
                    spellList[PET_FELGUARD].push_back(std::pair(79, 47996));
                }

                // Get the appropriate spell list by level and type
                const auto& petSpellListItr = spellList.find(pet->GetEntry());
                if (petSpellListItr != spellList.end())
                {
                    const auto& petSpellList = petSpellListItr->second;
                    for (const auto& pair : petSpellListItr->second)
                    {
                        const uint32& levelRequired = pair.first;
                        const uint32& spellID = pair.second;

                        // Check if the pet is missing the spells for it's current level
                        if (pet->GetLevel() >= levelRequired && !pet->HasSpell(spellID))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
#endif
}

bool SetPetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string command = event.getParam();
    HunterPetBuild* hpb = new HunterPetBuild(bot);

    // Extract the command and the parameters
    std::string parameter;
    size_t spacePos = command.find_first_of(" ");
    if (spacePos != std::string::npos)
    {
        parameter = command.substr(spacePos + 1);
        command = command.substr(0, spacePos);
    }

    Pet* pet = bot->GetPet();
    if (pet)
    {
        if (command == "autocast")
        {
            const std::string& spellName = parameter;
            if (!spellName.empty())
            {
                const uint32 spellId = AI_VALUE2(uint32, "spell id", spellName);
                if (pet->HasSpell(spellId) && IsAutocastable(spellId))
                {
                    auto IsAutocastActive = [&pet, &spellId]() -> bool
                        {
                            for (AutoSpellList::iterator i = pet->m_autospells.begin(); i != pet->m_autospells.end(); ++i)
                            {
                                if (*i == spellId)
                                {
                                    return true;
                                }
                            }

                            return false;
                        };

                    const bool autocastActive = IsAutocastActive();
                    pet->ToggleAutocast(spellId, !autocastActive);

                    std::ostringstream out;
                    out << (autocastActive ? "Disabling" : "Enabling") << " pet autocast for ";
                    out << ChatHelper::formatSpell(sServerFacade.LookupSpellInfo(spellId));
                    ai->TellPlayer(GetMaster(), out);

                    return true;
                }
                else
                {
                    ai->TellPlayer(requester, "I can't set to autocast that spell.");
                }
            }
            else
            {
                ai->TellPlayer(requester, "Please specify a pet spell to set the autocast.");
            }
        }
        else if (command == "aggressive")
        {
            // Send pet action packet
            const ObjectGuid& petGuid = pet->GetObjectGuid();
            const ObjectGuid& targetGuid = ObjectGuid();
            const uint8 flag = ACT_REACTION;
            const uint32 spellId = REACT_AGGRESSIVE;
            const uint32 command = (flag << 24) | spellId;

            WorldPacket data(CMSG_PET_ACTION);
            data << petGuid;
            data << command;
            data << targetGuid;
            bot->GetSession()->HandlePetAction(data);

            ai->TellPlayer(requester, "Setting pet to aggressive mode");
            return true;
        }
        else if (command == "defensive")
        {
            // Send pet action packet
            const ObjectGuid& petGuid = pet->GetObjectGuid();
            const ObjectGuid& targetGuid = ObjectGuid();
            const uint8 flag = ACT_REACTION;
            const uint32 spellId = REACT_DEFENSIVE;
            const uint32 command = (flag << 24) | spellId;

            WorldPacket data(CMSG_PET_ACTION);
            data << petGuid;
            data << command;
            data << targetGuid;
            bot->GetSession()->HandlePetAction(data);

            ai->TellPlayer(requester, "Setting pet to defensive mode");
            return true;
        }
        else if (command == "passive")
        {
            // Send pet action packet
            const ObjectGuid& petGuid = pet->GetObjectGuid();
            const ObjectGuid& targetGuid = ObjectGuid();
            const uint8 flag = ACT_REACTION;
            const uint32 spellId = REACT_PASSIVE;
            const uint32 command = (flag << 24) | spellId;

            WorldPacket data(CMSG_PET_ACTION);
            data << petGuid;
            data << command;
            data << targetGuid;
            bot->GetSession()->HandlePetAction(data);

            ai->TellPlayer(requester, "Setting pet to passive mode");
            return true;
        }
        else if (command == "follow")
        {
            // Send pet action packet
            const ObjectGuid& petGuid = pet->GetObjectGuid();
            const ObjectGuid& targetGuid = ObjectGuid();
            const uint8 flag = ACT_COMMAND;
            const uint32 spellId = COMMAND_FOLLOW;
            const uint32 command = (flag << 24) | spellId;

            WorldPacket data(CMSG_PET_ACTION);
            data << petGuid;
            data << command;
            data << targetGuid;
            bot->GetSession()->HandlePetAction(data);

            ai->TellPlayer(requester, "Setting pet to follow me");
            return true;
        }
        else if (command == "stay")
        {
            // Send pet action packet
            const ObjectGuid& petGuid = pet->GetObjectGuid();
            const ObjectGuid& targetGuid = ObjectGuid();
            const uint8 flag = ACT_COMMAND;
            const uint32 spellId = COMMAND_STAY;
            const uint32 command = (flag << 24) | spellId;

            WorldPacket data(CMSG_PET_ACTION);
            data << petGuid;
            data << command;
            data << targetGuid;
            bot->GetSession()->HandlePetAction(data);

            ai->TellPlayer(requester, "Setting pet to stay in place");
            return true;
        }
        else if (command == "attack")
        {
            if (requester->GetTarget())
            {
                // Send pet action packet
                const ObjectGuid& petGuid = pet->GetObjectGuid();
                const ObjectGuid& targetGuid = requester->GetTarget()->GetObjectGuid();
                const uint8 flag = ACT_COMMAND;
                const uint32 spellId = COMMAND_ATTACK;
                const uint32 command = (flag << 24) | spellId;

                WorldPacket data(CMSG_PET_ACTION);
                data << petGuid;
                data << command;
                data << targetGuid;
                bot->GetSession()->HandlePetAction(data);

                ai->TellPlayer(requester, "Sending pet to attack");
                return true;
            }
            else
            {
                ai->TellPlayer(requester, "Please select a target to attack");
            }
        }
        else if (command == "dismiss")
        {
            if (pet->getPetType() == HUNTER_PET)
            {
                if (ai->DoSpecificAction("dismiss pet", event, true))
                {
                    ai->ChangeStrategy("-pet", BotState::BOT_STATE_COMBAT);
                    ai->ChangeStrategy("-pet", BotState::BOT_STATE_NON_COMBAT);
                    ai->TellPlayer(requester, "Dismissing pet");
                    return true;
                }
                else
                {
                    ai->TellPlayer(requester, "I can't dismiss my pet");
                }
            }
            else
            {
                // Send pet action packet
                const ObjectGuid& petGuid = pet->GetObjectGuid();
                const ObjectGuid& targetGuid = ObjectGuid();
                const uint8 flag = ACT_COMMAND;
                const uint32 spellId = COMMAND_DISMISS;
                const uint32 command = (flag << 24) | spellId;

                WorldPacket data(CMSG_PET_ACTION);
                data << petGuid;
                data << command;
                data << targetGuid;
                bot->GetSession()->HandlePetAction(data);

                ai->ChangeStrategy("+pet", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("+pet", BotState::BOT_STATE_NON_COMBAT);
                ai->ChangeStrategy("-pet", BotState::BOT_STATE_COMBAT);
                ai->ChangeStrategy("-pet", BotState::BOT_STATE_NON_COMBAT);
                ai->TellPlayer(requester, "Dismissing pet");

                return true;
            }
        }
        else if (command == "build")
        {
            if (bot->getClass() == CLASS_HUNTER)
            {
                std::ostringstream out;
                if (parameter == "" || parameter == " ")
                {
                    listCurrentPath(&out);
                    ai->TellPlayer(requester, out.str());
                }
                else if (parameter.find("list ") != std::string::npos)
                {
                    listPremadePaths(getPremadePaths(parameter.substr(5)), &out);
                    ai->TellPlayer(requester, out.str());
                }
                else if (parameter.find("list") != std::string::npos)
                {
                    listPremadePaths(getPremadePaths(""), &out);
                    ai->TellPlayer(requester, out);
                }
                else if (parameter.find("get tp") != std::string::npos || parameter.find("get training points") != std::string::npos)
                {
                    out << bot->GetPet()->GetName() << " has " << hpb->CalculateTrainingPoints(bot) << " total training points.";
                    ai->TellPlayer(requester, out);
                }
                else if (parameter.find("update") != std::string::npos)
                {
                    uint32 petNumber = pet->GetCharmInfo()->GetPetNumber();
                    int savedType = sPlayerbotDbStore.PetHasBuilds(pet->GetCharmInfo()->GetPetNumber());
                    switch (savedType)
                    {
                        case 0:
                            ai->TellPlayer(requester, "No build saved. Please run 'pet build set random', 'pet build set <path number>', 'pet build set <path name>', or 'pet build <build link>'");
                            break;
                        case 1:
                        {
                            HunterPetBuild currentBuild = HunterPetBuild(bot);
                            std::string currentBuildString = currentBuild.GetBuildLink();
                            HunterPetBuildPath path = sPlayerbotDbStore.LoadPetBuildPath(pet->GetCharmInfo()->GetPetNumber());
                            HunterPetBuild newBuild = *GetBestPremadeBuild(path.id);
                            std::string buildLink = newBuild.GetBuildLink();
                            if (currentBuildString != buildLink)
                            {
                                std::vector<std::string> outGroup;
                                newBuild.ApplyBuild(bot, outGroup);
                            }
                            break;
                        }
                        case 2:
                        {
                            HunterPetBuild currentBuild = HunterPetBuild(bot);
                            std::string currentBuildString = currentBuild.GetBuildLink();
                            std::string buildLink = sPlayerbotDbStore.LoadPetBuildLink(pet->GetCharmInfo()->GetPetNumber());
                            if (currentBuildString != buildLink)
                            {
                                HunterPetBuild newBuild = HunterPetBuild(buildLink);
                                std::vector<std::string> outGroup;
                                newBuild.ApplyBuild(bot, outGroup);
                            }
                            break;
                        }
                    }
                }
                else if (parameter.find("set ") != std::string::npos)
                {
                    if (parameter.substr(4).find("random") != std::string::npos)
                    {
                        std::vector<HunterPetBuildPath*> paths = getPremadePaths("");
                        if (paths.size() > 0)
                        {
                            out.str("");
                            out.clear();

                            if (paths.size() > 1)
                                out << "Found " << paths.size() << " possible pet builds for this family to choose from. ";

                            HunterPetBuildPath* path = PickPremadePath(paths, sRandomPlayerbotMgr.IsRandomBot(bot));
                            HunterPetBuild newBuild = *GetBestPremadeBuild(path->id);
                            std::string buildLink = newBuild.GetBuildLink();
                            std::vector<std::string> outGroup;
                            newBuild.ApplyBuild(bot, outGroup);

                            if (newBuild.GetTPCostOfBuild() > 0)
                            {
                                out << "Apply spec " << "|h|cffffffff" << path->name;
                                sPlayerbotDbStore.SavePetBuildPath(pet->GetCharmInfo()->GetPetNumber(), pet->GetCreatureInfo()->Family, path->id);
                                ai->TellPlayer(requester, out);

                            }
                        }
                    }
                    else
                    {
                        if (IsValidBuildId(parameter.substr(4)))
                        {
                            parameter = parameter.substr(4);
                            int buildId = stoi(parameter);
                            std::vector<HunterPetBuildPath*> paths = getPremadePaths(buildId);
                            if (paths.size() > 0)
                            {
                                out.str("");
                                out.clear();

                                HunterPetBuildPath* path = PickPremadePath(paths, false);
                                HunterPetBuild newBuild = *GetBestPremadeBuild(path->id);
                                std::string buildLink = newBuild.GetBuildLink();
                                std::vector<std::string> outGroup;
                                newBuild.ApplyBuild(bot, outGroup);

                                if (newBuild.GetTPCostOfBuild() > 0)
                                {
                                    out << "Apply spec " << "|h|cffffffff" << path->name;
                                    sPlayerbotDbStore.SavePetBuildPath(pet->GetCharmInfo()->GetPetNumber(), pet->GetCreatureInfo()->Family, path->id);
                                    ai->TellPlayer(requester, out);
                                }
                            }
                        }
                        else if (IsValidBuildName(parameter.substr(4)))
                        {
                            std::string buildName = parameter.substr(4);
                            std::vector<HunterPetBuildPath*> paths = getPremadePaths(buildName);

                            if (paths.size() > 1)
                            {
                                std::ostringstream multiplePathsFoundString;
                                multiplePathsFoundString << paths.size() << " paths found. A random one will be picked. Or you can use pet build list " << buildName << " to view build ids + name and select a build that way.";
                                ai->TellPlayer(requester, multiplePathsFoundString);
                            }

                            HunterPetBuildPath* path = PickPremadePath(paths, false);
                            HunterPetBuild newBuild = *GetBestPremadeBuild(path->id);
                            std::string buildLink = newBuild.GetBuildLink();
                            std::vector<std::string> outGroup;
                            newBuild.ApplyBuild(bot, outGroup);

                            if (newBuild.GetTPCostOfBuild() > 0)
                            {
                                out << "Apply spec " << "|h|cffffffff" << path->name;
                                sPlayerbotDbStore.SavePetBuildPath(pet->GetCharmInfo()->GetPetNumber(), pet->GetCreatureInfo()->Family, path->id);
                                ai->TellPlayer(requester, out);
                            }
                        }
                        else if (hpb->CheckBuildLink(parameter.substr(4), pet->GetCreatureInfo()->Family, &out))
                        {
                            HunterPetBuild newBuild(parameter.substr(4));
                            std::string buildLink = newBuild.GetBuildLink();

                            if (newBuild.CheckBuild(hpb->CalculateTrainingPoints(bot), &out))
                            {
                                std::vector<std::string> outGroup;
                                newBuild.ApplyBuild(bot, outGroup);
                                sPlayerbotDbStore.SavePetBuildLink(pet->GetCharmInfo()->GetPetNumber(), buildLink);
                                ai->TellPlayer(requester, out);
                            }
                        }
                        else
                        {
                            ai->TellPlayer(requester, "Invalid build id, name or build link provided.");
                        }
                    }
                }
                else if (parameter.find("get current build link") != std::string::npos)
                {
                    HunterPetBuild hp = HunterPetBuild(bot);
                    ai->TellPlayer(requester, hp.GetBuildLink());
                }
                else if (parameter.find("get planned build link") != std::string::npos)
                {

                    uint32 petNumber = pet->GetCharmInfo()->GetPetNumber();
                    int savedType = sPlayerbotDbStore.PetHasBuilds(pet->GetCharmInfo()->GetPetNumber());
                    switch (savedType)
                    {
                        case 0:
                            ai->TellPlayer(requester, "No build saved. Please run 'pet build set random', 'pet build set <path number>', 'pet build set <path name>', or 'pet build <build link>'");
                            break;
                        case 1:
                        {
                            HunterPetBuild currentBuild = HunterPetBuild(bot);
                            std::string currentBuildString = currentBuild.GetBuildLink();
                            HunterPetBuildPath path = sPlayerbotDbStore.LoadPetBuildPath(pet->GetCharmInfo()->GetPetNumber());
                            HunterPetBuild newBuild = *GetBestPremadeBuild(path.id);
                            std::stringstream nextLink;
                            nextLink << "Next Link in Path: " << newBuild.GetBuildLink();
                            ai->TellPlayer(requester, nextLink.str());
                            std::stringstream finalBuildLink;
                            finalBuildLink << "Final Link in Path: " << path.hunterPetBuild.back().GetBuildLink();
                            ai->TellPlayer(requester, finalBuildLink.str());
                            break;
                        }
                        case 2:
                        {
                            HunterPetBuild currentBuild = HunterPetBuild(bot);
                            std::string currentBuildString = currentBuild.GetBuildLink();
                            std::string buildLink = sPlayerbotDbStore.LoadPetBuildLink(pet->GetCharmInfo()->GetPetNumber());
                            ai->TellPlayer(requester, buildLink);
                            break;
                        }
                    }
                }
            }
            else
            {
                ai->TellPlayer(requester, "pet build and its sub commands are hunter only commands.");
            }
        }
        else if (command == "spells")
        {
            std::vector<std::string> spells = GetCurrentPetSpellNames(bot->GetPet());
            for (std::string spell : spells)
            {
                ai->TellPlayer(requester, spell, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL);
            }
        }
        else
        {
            ai->TellPlayer(requester, "Please specify a pet command (Like autocast).");
        }
    }
    else if (command == "call")
    {
        if (bot->getClass() == CLASS_HUNTER || bot->getClass() == CLASS_WARLOCK)
        {
            ai->ChangeStrategy("+pet", BotState::BOT_STATE_COMBAT);
            ai->ChangeStrategy("+pet", BotState::BOT_STATE_NON_COMBAT);
            ai->TellPlayer(requester, "Calling my pet");
            return true;
        }
        else
        {
            ai->TellPlayer(requester, "I can't call any pets");
        }
    }
    else
    {
        ai->TellPlayer(requester, "I don't have any pets");
    }

    return false;
}

bool SetPetAction::AutoSelectBuild(Player* bot, std::ostringstream* out)
{
    HunterPetBuild currentBuild = HunterPetBuild(bot);
    HunterPetBuildPath path = sPlayerbotDbStore.LoadPetBuildPath(bot->GetPet()->GetCharmInfo()->GetPetNumber());
    if (path.id >= 0)
    {
        HunterPetBuild* build = GetBestPremadeBuild(path.id);
        if (currentBuild.GetBuildLink() != build->GetBuildLink())
        {
            std::vector<std::string> outGroup;
            build->ApplyBuild(bot, outGroup);
            return true;
        }
    }
    else
    {
        std::string buildLink = sPlayerbotDbStore.LoadPetBuildLink(bot->GetPet()->GetCharmInfo()->GetPetNumber());
        if (buildLink != "")
        {
            HunterPetBuild build = HunterPetBuild(buildLink);

        }
    }
    return false;
}

std::vector<HunterPetBuildPath*> SetPetAction::getPremadePaths(std::string findName)
{
    std::vector<HunterPetBuildPath*> ret;
    for (auto& path : sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths)
    {
        if (findName.empty() || path.name.find(findName) != std::string::npos)
        {
            ret.push_back(&path);
        }
    }

    return ret;
}

std::vector<HunterPetBuildPath*> SetPetAction::getPremadePaths(int id)
{
    std::vector<HunterPetBuildPath*> ret;
    for (auto& path : sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths)
    {
        if (path.id == id)
        {
            ret.push_back(&path);
        }
    }

    return ret;
}

void SetPetAction::listPremadePaths(std::vector<HunterPetBuildPath*> paths, std::ostringstream* out)
{
    if (paths.size() == 0)
    {
        *out << "No predefined paths for family found.";
    }

    *out << "|h|cffffffff";

    for (auto path : paths)
    {
        *out << "id: " << path->id << " Name:" << path->name << ", ";
    }

    out->seekp(-2, out->cur);
    *out << ".";
}

HunterPetBuildPath* SetPetAction::PickPremadePath(std::vector<HunterPetBuildPath*> paths, bool useProbability)
{
    int totalProability = 0;
    int curProbability = 0;

    if (paths.size() == 1)
        return paths[0];

    for (auto path : paths)
    {
        totalProability += useProbability ? path->probability : 1;
    }

    totalProability = irand(0, totalProability);

    for (auto path : paths)
    {
        curProbability += (useProbability ? path->probability : 1);
        if (curProbability >= totalProability)
            return path;
    }

    return paths[0];
}

HunterPetBuild* SetPetAction::GetBestPremadeBuild(int specId)
{
    HunterPetBuildPath* path = getPremadePath(specId);
    for (auto& build : path->hunterPetBuild)
    {
        if (build.tpCost >= build.CalculateTrainingPoints(bot))
            return &build;
    }
    if (path->hunterPetBuild.size())
        return &path->hunterPetBuild.back();

    return &sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].baseBuild;
}

HunterPetBuildPath* SetPetAction::getPremadePath(int id)
{
    for (auto& path : sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths)
    {
        if (id == path.id)
            return &path;
    }

    if (sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths.empty())
        return nullptr;

    return &sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths[0];
}

void SetPetAction::listCurrentPath(std::ostringstream* out)
{
    std::string petBuildName = sPlayerbotDbStore.GetPetBuildName(bot->GetPet()->GetCharmInfo()->GetPetNumber());
    *out << "|h|cffffffff" << petBuildName;
}

HunterPetBuildPath* InitializePetSpellsAction::PickPremadePath(std::vector<HunterPetBuildPath*> paths, bool useProbability)
{
    int totalProability = 0;
    int curProbability = 0;

    if (paths.size() == 1)
        return paths[0];

    for (auto path : paths)
    {
        totalProability += useProbability ? path->probability : 1;
    }

    totalProability = irand(0, totalProability);

    for (auto path : paths)
    {
        curProbability += (useProbability ? path->probability : 1);
        if (curProbability >= totalProability)
            return path;
    }

    return paths[0];
}

HunterPetBuild* InitializePetSpellsAction::GetBestPremadeBuild(int specId)
{
    HunterPetBuildPath* path = getPremadePath(specId);
    for (auto& build : path->hunterPetBuild)
    {
        if (build.tpCost >= build.CalculateTrainingPoints(bot))
            return &build;
    }
    if (path->hunterPetBuild.size())
        return &path->hunterPetBuild.back();

    return &sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].baseBuild;
}

HunterPetBuildPath* InitializePetSpellsAction::getPremadePath(int id)
{
    for (auto& path : sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths)
    {
        if (id == path.id)
            return &path;
    }

    if (sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths.empty())
        return nullptr;

    return &sPlayerbotAIConfig.familyPetBuilds[bot->GetPet()->GetCreatureInfo()->Family].hunterPetBuildPaths[0];
}

void InitializePetAction::InitialFamilySkill(Player* bot)
{
    uint32 level = bot->GetPet()->GetLevel();
    uint32 family = bot->GetPet()->GetCreatureInfo()->Family;
    HunterPetBuild hunterPetBuild = HunterPetBuild();
    hunterPetBuild.InitializeStartingPetSpells(bot, level, family);
}

bool SetPetAction::IsValidBuildId(std::string buildId)
{
    std::istringstream iss(buildId);
    int buildIdNum;
    iss >> buildIdNum;
    if (!(iss.eof() && !iss.fail()))
        return false;

    return getPremadePath(buildIdNum);
}

bool SetPetAction::IsValidBuildName(std::string buildName)
{
    std::vector<HunterPetBuildPath*> paths = getPremadePaths(buildName);
    return paths.size() > 0;
}

std::vector<std::string> SetPetAction::GetCurrentPetSpellNames(Pet* pet)
{
    std::vector<std::string> spells;
    uint32 petGuid = pet->GetCharmInfo()->GetPetNumber();

    PetSpellMap creatureSpellList = pet->m_spells;

    for (auto& creatureSpell : creatureSpellList)
    {
        SpellEntry const* spell = sServerFacade.LookupSpellInfo(creatureSpell.first);
        if (creatureSpell.second.state != PetSpellState::PETSPELL_REMOVED)
        {
            std::stringstream spellNameStream;
            spellNameStream << spell->SpellName[0] << " " << spell->Rank[0];
            std::string spellName = spellNameStream.str();
            if (spellName.find("Tamed Pet Passive (DND)") == std::string::npos)
            {
                spells.push_back(spellName);
            }
        }
    }

    if (spells.empty())
    {
        spells.push_back("No spells for pet.");
    }
    return spells;
}