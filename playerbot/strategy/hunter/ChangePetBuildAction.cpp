#include "C:/Users/edmun/OneDrive/Desktop/WoW Private Server/WoW Classic/server/server-bin/src/modules/PlayerBots/CMakeFiles/playerbots.dir/Debug/cmake_pch.hxx"
#include "ChangePetBuildAction.h"

using namespace ai;

bool ChangePetBuildAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::ostringstream out; 
    HunterPetBuild(bot);
    std::string param = event.getParam();

    if (!param.empty())
    {
        if (param.find("list ") != std::string::npos)
        {
            listPremadePaths(getPremadePaths(param.substr(5)), &out);
        }
        else if (param.find("list") != std::string::npos)
        {
            listPremadePaths(getPremadePaths(""), &out);
        }
    }

    return true;
}

bool ChangePetBuildAction::isUseful()
{
    if (bot->getClass() == CLASS_HUNTER && bot->GetPet())
    {
        return true;
    }
    return false;
}

std::vector<HunterPetBuildPath*> ChangePetBuildAction::getPremadePaths(std::string findName)
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

void ChangePetBuildAction::listPremadePaths(std::vector<HunterPetBuildPath*> paths, std::ostringstream* out)
{
    if (paths.size() == 0)
    {
        *out << "No predefined paths for family found.";
    }

    *out << "|h|cffffffff";

    for (auto path : paths)
    {
        *out << path->name << " (" << path->name << "), ";
    }

    out->seekp(-2, out->cur);
    *out << ".";
}