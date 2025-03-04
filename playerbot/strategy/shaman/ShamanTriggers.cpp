
#include "playerbot/playerbot.h"
#include "ShamanTriggers.h"
#include "ShamanActions.h"

using namespace ai;

std::list<std::string> ShamanWeaponTrigger::spells;

bool ShamanWeaponTrigger::IsActive()
{
    if (spells.empty())
    {
        spells.push_back("frostbrand weapon");
        spells.push_back("rockbiter weapon");
        spells.push_back("flametongue weapon");
        spells.push_back("earthliving weapon");
        spells.push_back("windfury weapon");
    }

    for (std::list<std::string>::iterator i = spells.begin(); i != spells.end(); ++i)
    {
        uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
        if (!spellId)
            continue;

        if (AI_VALUE2(Item*, "item for spell", spellId))
            return true;
    }

    return false;
}

bool ShockTrigger::IsActive()
{
    Unit* target = GetTarget();
    return SpellTrigger::IsActive() 
        && target
        && !ai->HasAura("frost shock", target, false, true)
        && !ai->HasAura("earth shock", target, false, true)
        && !ai->HasAura("flame shock", target, false, true)
        && !HasMaxDebuffs();
}

bool FlameShockOnTargetTrigger::IsActive()
{
    return SpellCanBeCastedTrigger::IsActive() && ai->HasAura("flame shock", GetTarget(), false, true);
}

bool MaelstromWeaponTrigger::IsActive()
{
    return ai->HasAura("maelstrom weapon", bot, true);
}

Player* GetLowestHealthPlayer(Group* group)
{
    float lowestHealth = 100.0f;
    Player* lowestHealthPlayer = nullptr;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (Player* player = ref->getSource())
        {
            float health = player->GetHealthPercent();
            if (health > 0.0f && health < lowestHealth)
            {
                lowestHealth = health;
                lowestHealthPlayer = player;
            }
        }
    }

    return lowestHealthPlayer;
}

bool LowestHpEarthShieldTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group) return false;

    Player* lowestHealthPlayer = GetLowestHealthPlayer(group);
    return lowestHealthPlayer && bot != lowestHealthPlayer && !ai->HasAura("earth shield", lowestHealthPlayer, false, true);
}

bool EarthShieldTrigger::IsActive()
{
    Group* group = bot->GetGroup();
    if (!group) return false;

    Player* lowestHealthPlayer = GetLowestHealthPlayer(group);
    return lowestHealthPlayer && bot == lowestHealthPlayer && !ai->HasAura("earth shield", lowestHealthPlayer, false, true);
}
