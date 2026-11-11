
#include "playerbot/playerbot.h"
#include "RtiValue.h"
#include "RtiTargetValue.h"

using namespace ai;

RtiValue::RtiValue(PlayerbotAI* ai)
    : ManualSetValue<std::string>(ai, "skull", "rti")
{
}

RtiCcValue::RtiCcValue(PlayerbotAI* ai)
    : ManualSetValue<std::string>(ai, "none", "rti cc")
{
}

CombatOrderTargetValue::CombatOrderTargetValue(PlayerbotAI* ai)
    : ManualSetValue<std::string>(ai, "", "combat order target")
{
}

class OrderCcCheck : public FindNonCcTargetStrategy
{
public:
    OrderCcCheck(PlayerbotAI* ai) : FindNonCcTargetStrategy(ai) {}
    virtual void CheckAttacker(Unit* attacker, ThreatManager* threatManager) override {}
    bool IsCc(Unit* attacker) { return IsCcTarget(attacker); }
};

Player* CombatOrderTargetValue::FindGroupMember(Player* bot, const std::string& name)
{
    Group* group = bot->GetGroup();
    if (!group || name.empty())
        return nullptr;

    std::string wanted = name;
    std::transform(wanted.begin(), wanted.end(), wanted.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    for (Group::member_citerator itr = group->GetMemberSlots().begin(); itr != group->GetMemberSlots().end(); ++itr)
    {
        Player* member = sObjectMgr.GetPlayer(itr->guid);
        if (!member || !member->IsInWorld())
            continue;

        std::string memberName = member->GetName();
        std::transform(memberName.begin(), memberName.end(), memberName.begin(), [](unsigned char c) { return (char)std::tolower(c); });
        if (memberName == wanted)
            return member;
    }

    return nullptr;
}

ObjectGuid CombatOrderTargetValue::FindTarget(PlayerbotAI* ai, bool lowestThreat)
{
    AiObjectContext* context = ai->GetAiObjectContext();
    const std::string order = context->GetValue<std::string>("combat order target")->Get();
    const size_t separator = order.find(':');
    if (separator == std::string::npos)
        return ObjectGuid();

    const std::string mode = order.substr(0, separator);
    Player* bot = ai->GetBot();
    Player* member = FindGroupMember(bot, order.substr(separator + 1));
    if (!member || member == bot || !sServerFacade.IsAlive(member) || !ai->IsSafe(member))
        return ObjectGuid();

    if (mode == "assist")
    {
        if (!member->IsInCombat())
            return ObjectGuid();

        ObjectGuid guid = member->GetVictim() ? member->GetVictim()->GetObjectGuid() : member->GetSelectionGuid();
        if (!guid)
            return ObjectGuid();

        const std::list<ObjectGuid>& possible = context->GetValue<std::list<ObjectGuid>>("possible targets no los")->Get();
        if (std::find(possible.begin(), possible.end(), guid) == possible.end())
            return ObjectGuid();

        Unit* unit = ai->GetUnit(guid);
        if (!unit || sServerFacade.UnitIsDead(unit) || !bot->IsWithinDistInMap(unit, sPlayerbotAIConfig.sightDistance, false))
            return ObjectGuid();

        return guid;
    }

    if (mode == "protect")
    {
        Unit* best = nullptr;
        float bestThreat = 0.0f;
        OrderCcCheck ccCheck(ai);
        std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("attackers")->Get();
        for (const ObjectGuid& guid : attackers)
        {
            Unit* unit = ai->GetUnit(guid);
            if (!unit || sServerFacade.UnitIsDead(unit) || unit->GetVictim() != member || ccCheck.IsCc(unit))
                continue;

            if (lowestThreat)
            {
                float threat = sServerFacade.GetThreatManager(unit).getThreat(bot);
                if (!best || threat < bestThreat)
                {
                    best = unit;
                    bestThreat = threat;
                }
            }
            else if (!best || unit->GetHealth() < best->GetHealth())
            {
                best = unit;
            }
        }

        return best ? best->GetObjectGuid() : ObjectGuid();
    }

    return ObjectGuid();
}
