#include "playerbot/playerbot.h"
#include "PartyMemberToSoulstone.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

class IsTargetOfSoulstoneSpell : public SpellEntryPredicate
{
public:
    virtual bool Check(SpellEntry const* spell) override
    {
        for (int i = 0; i < 3; i++)
        {
            if(spell->Effect[i] == SPELL_EFFECT_SELF_RESURRECT)
                return true;
        }
        return false;
    }
};

// Finds a healer or tank in range without an active soulstone buff
// and not already being targeted by another warlock's cast
class FindSoulstoneTarget : public FindPlayerPredicate, public PlayerbotAIAware
{
public:
    FindSoulstoneTarget(PlayerbotAI* ai, PartyMemberValue* value) : PlayerbotAIAware(ai), value(value) {}

    virtual bool Check(Unit* unit) override
    {
        Player* player = dynamic_cast<Player*>(unit);
        if (!player || !player->IsAlive() || player->InBattleGround())
            return false;

        if (sServerFacade.GetDistance2d(ai->GetBot(), player) > ai->GetRange("spell"))
            return false;

        // Skip DPS — only healers and tanks are worth stoning
        if (!ai->IsHeal(player) && !ai->IsTank(player))
            return false;

        // Skip players who already have a soulstone buff
        if (ai->HasAura("soulstone resurrection", player))
            return false;

        // Skip players another warlock is already mid-cast on
        return !value->IsTargetOfSpellCast(player, predicate);
    }

private:
    PartyMemberValue* value;
    IsTargetOfSoulstoneSpell predicate;
};

static bool HasOtherWarlockInGroup(Player* bot, Group* group)
{
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->getSource();
        if (!member || member == bot)
            continue;
        if (member->getClass() == CLASS_WARLOCK)
            return true;
    }
    return false;
}

static bool BotIsAlreadySoulstoned(Player* bot)
{
    Unit::SpellAuraHolderMap const& auras = bot->GetSpellAuraHolderMap();
    for (auto const& pair : auras)
    {
        SpellEntry const* spellInfo = pair.second->GetSpellProto();
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo->Effect[i] == SPELL_EFFECT_SELF_RESURRECT)
                return true;
        }
    }
    return false;
}

Unit* PartyMemberToSoulstone::Calculate()
{
    // If a master player has manually assigned revive targets, defer to that
    if (!AI_VALUE(std::list<ObjectGuid>, "revive targets").empty())
        return nullptr;

    Group* group = bot->GetGroup();

    if (group)
    {
        // FindPartyMember automatically prioritizes healers over tanks
        FindSoulstoneTarget finder(ai, this);
        Unit* target = FindPartyMember(finder);

        if (!target)
            return nullptr;

        // Multiple warlocks in group may target same player. Each roll D20 to stagger casts.
        if (HasOtherWarlockInGroup(bot, group))
        {
            if (urand(1, 20) != 20)
                return nullptr;
        }

        return target;
    }
    else
    {
        if (BotIsAlreadySoulstoned(bot))
            return nullptr;

        return ai->GetBot();
    }
}
