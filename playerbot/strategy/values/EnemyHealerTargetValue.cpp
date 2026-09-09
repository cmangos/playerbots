
#include "playerbot/playerbot.h"
#include "EnemyHealerTargetValue.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"

using namespace ai;

ObjectGuid EnemyHealerTargetValue::Calculate()
{
    std::string spell = qualifier;

    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
    Unit* target = ai->GetUnit(ai->GetAiObjectContext()->GetValue<ObjectGuid>("current target")->Get());
    for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); ++i)
    {
        Unit* unit = ai->GetUnit(*i);
        if (!unit || unit == target)
            continue;

        if (sServerFacade.GetDistance2d(bot, unit) > ai->GetRange("spell"))
            continue;

        if (!ai->IsInterruptableSpellCasting(unit, spell))
            continue;

        Spell* spell = unit->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (spell && IsPositiveSpell(spell->m_spellInfo))
            return unit->GetObjectGuid();

        spell = unit->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (spell && IsPositiveSpell(spell->m_spellInfo))
            return unit->GetObjectGuid();
    }

    return NULL;
}
