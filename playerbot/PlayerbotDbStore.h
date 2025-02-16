#ifndef _PlayerbotDbStore_H
#define _PlayerbotDbStore_H

#include "Common.h"
#include "PlayerbotAIBase.h"
#include "PlayerbotMgr.h"

class PlayerbotDbStore
{
public:
    PlayerbotDbStore() {}
    virtual ~PlayerbotDbStore() {}
    static PlayerbotDbStore& instance()
    {
        static PlayerbotDbStore instance;
        return instance;
    }

    void Save(PlayerbotAI *ai, std::string preset = "");
    void Load(PlayerbotAI *ai, std::string preset = "");
    void Reset(PlayerbotAI *ai, std::string preset = "");

    void SavePetBuildLink(uint64 petGuid, std::string buildLink);
    std::string LoadPetBuildLink(uint64 petGuid);

    void SavePetBuildPath(uint64 petGuid, uint32 petFamilyId, int buildNo);
    HunterPetBuildPath LoadPetBuildPath(uint64 petGuid);

    int PetHasBuilds(uint64 petGuid);

    std::string GetPetBuildName(uint64 petGuid);

private:
    void SaveValue(uint64 guid, std::string preset, std::string key, std::string value);
    std::string FormatStrategies(std::string type, std::list<std::string_view> strategies);
};

#define sPlayerbotDbStore PlayerbotDbStore::instance()

#endif
