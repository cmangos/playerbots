
#include "playerbot/playerbot.h"
#include "AutoLearnSpellAction.h"
#include "playerbot/ServerFacade.h"
#include "Entities/Item.h"
#include <Mails/Mail.h>

using namespace ai;

bool AutoLearnSpellAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();

    std::ostringstream out;

    LearnSpells(&out);

    if (!out.str().empty())
    {
        const std::string& temp = out.str();
        out.seekp(0);
        out << temp;
        out.seekp(-2, out.cur);
        out << ".";

        std::map<std::string, std::string> args;
        args["%spells"] = out.str();
        ai->TellPlayer(requester, BOT_TEXT2("auto_learn_spell", args), PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, false);
    }

    return true;
}

void AutoLearnSpellAction::LearnSpells(std::ostringstream* out)
{
    BroadcastHelper::BroadcastLevelup(ai, bot);

    if (sPlayerbotAIConfig.autoLearnTrainerSpells)
        LearnTrainerSpells(out);

    if (sPlayerbotAIConfig.autoLearnQuestSpells)
        LearnQuestSpells(out);

    if (!ai->HasActivePlayerMaster()) //Hunter spells for pets.
    {
        if (bot->getClass() == CLASS_HUNTER && bot->GetLevel() >= 10)
        {
            bot->learnSpell(5149, false); //Beast training
            bot->learnSpell(883, false); //Call pet
            bot->learnSpell(982, false); //Revive pet
            bot->learnSpell(6991, false); //Feed pet
            bot->learnSpell(1515, false); //Tame beast
        }
    }
}

void AutoLearnSpellAction::LearnTrainerSpells(std::ostringstream* out)
{
    bot->learnDefaultSpells();

    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
        if (!co)
            continue;

        if (co->TrainerType != TRAINER_TYPE_CLASS && co->TrainerType != TRAINER_TYPE_TRADESKILLS)
            continue;

        if (co->TrainerType == TRAINER_TYPE_CLASS && co->TrainerClass != bot->getClass())
            continue;

        uint32 trainerId = co->TrainerTemplateId;
        if (!trainerId)
            trainerId = co->Entry;

        TrainerSpellData const* trainer_spells = sObjectMgr.GetNpcTrainerTemplateSpells(trainerId);
        if (!trainer_spells)
            trainer_spells = sObjectMgr.GetNpcTrainerSpells(trainerId);

        if (!trainer_spells)
            continue;

        for (TrainerSpellMap::const_iterator itr = trainer_spells->spellList.begin(); itr != trainer_spells->spellList.end(); ++itr)
        {
            TrainerSpell const* tSpell = &itr->second;

            if (!tSpell)
                continue;

            uint32 reqLevel = 0;

            reqLevel = tSpell->isProvidedReqLevel ? tSpell->reqLevel : std::max(reqLevel, tSpell->reqLevel);
            TrainerSpellState state = bot->GetTrainerSpellState(tSpell, reqLevel);
            if (state != TRAINER_SPELL_GREEN)
                continue;

            if (co->TrainerType == TRAINER_TYPE_TRADESKILLS)
            {
                SpellEntry const* spell = sServerFacade.LookupSpellInfo(tSpell->spell);
                if (spell)
                {
                    std::string SpellName = spell->SpellName[0];
                    if (spell->Effect[EFFECT_INDEX_1] == SPELL_EFFECT_SKILL_STEP)
                    {
                        uint32 skill = spell->EffectMiscValue[EFFECT_INDEX_1];

                        if (skill)
                        {
                            SkillLineEntry const* pSkill = sSkillLineStore.LookupEntry(skill);
                            if (pSkill)
                            {
                                if (SpellName.find("Apprentice") != std::string::npos && pSkill->categoryId == SKILL_CATEGORY_PROFESSION || pSkill->categoryId == SKILL_CATEGORY_SECONDARY)
                                    continue;
                            }
                        }
                    }
                }

            }

            LearnSpell(tSpell->spell, out);
        }
    }
}

void AutoLearnSpellAction::LearnQuestSpells(std::ostringstream* out)
{
    //CreatureInfo const* co = sCreatureStorage.LookupEntry<CreatureInfo>(id);
    ObjectMgr::QuestMap const& questTemplates = sObjectMgr.GetQuestTemplates();
    for (ObjectMgr::QuestMap::const_iterator i = questTemplates.begin(); i != questTemplates.end(); ++i)
    {
        uint32 questId = i->first;
        Quest const* quest = i->second.get();

        if (!quest->GetRequiredClasses() || quest->IsRepeatable())
            continue;

        if (!bot->SatisfyQuestClass(quest, false) ||
            quest->GetMinLevel() > bot->GetLevel() ||
            !bot->SatisfyQuestRace(quest, false))
            continue;

        if (quest->GetRewSpellCast() > 0)
        {
            if(LearnSpell(quest->GetRewSpellCast(), out))
                GetClassQuestItem(quest, out);
        }
        else if (quest->GetRewSpell() > 0)
        {
            if(LearnSpell(quest->GetRewSpell(), out))
                GetClassQuestItem(quest, out);
        }
    }
}
/**
* Attempts to add the quest item
* If the bot's bag is full report to player.
*/
void AutoLearnSpellAction::GetClassQuestItem(Quest const* quest, std::ostringstream* out)
{
    if (quest->GetRewItemsCount() > 0)
    {
        for (uint32 i = 0; i < quest->GetRewItemsCount(); i++)
        {            
            ItemPosCountVec itemVec;
            ItemPrototype const* itemP = sObjectMgr.GetItemPrototype(quest->RewItemId[i]);
            InventoryResult result = bot->CanStoreNewItem(NULL_BAG, NULL_SLOT, itemVec, itemP->ItemId, quest->RewItemCount[i]);
            if (result == EQUIP_ERR_OK)
            {
                bot->StoreNewItemInInventorySlot(itemP->ItemId, quest->RewItemCount[i]);
                *out << "Got " << chat->formatItem(itemP, 1, 1) << " from " << quest->GetTitle();
            }
            else if (result == EQUIP_ERR_INVENTORY_FULL)
            {
                MailDraft draft("Item(s) from quest reward", quest->GetTitle());
                Item* item = item->CreateItem(itemP->ItemId, quest->RewItemCount[i]);
                draft.AddItem(item);
                draft.SendMailTo(MailReceiver(bot), MailSender(bot));
                *out << "Could not add item " << chat->formatItem(itemP) << " from " << quest->GetTitle() << ". " << bot->GetName() << "'s inventory is full.";
            }
        }
    }
}

std::string formatSpell(SpellEntry const* sInfo)
{
    std::ostringstream out;
    std::string rank = sInfo->Rank[0];

    if (rank.empty())
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << "]|h|r";
    else
        out << "|cffffffff|Hspell:" << sInfo->Id << "|h[" << sInfo->SpellName[LOCALE_enUS] << " " << rank << "]|h|r";
    return out.str();
}

bool AutoLearnSpellAction::LearnSpell(uint32 spellId, std::ostringstream* out)
{
    SpellEntry const* proto = sServerFacade.LookupSpellInfo(spellId);

    if (!proto)
        return false;

    bool learned = false;
    for (int j = 0; j < 3; ++j)
    {
        if (proto->Effect[j] == SPELL_EFFECT_LEARN_SPELL)
        {
            uint32 learnedSpell = proto->EffectTriggerSpell[j];

            if (!bot->HasSpell(learnedSpell))
            {
                bot->learnSpell(learnedSpell, false);
                SpellEntry const* spellInfo = sServerFacade.LookupSpellInfo(learnedSpell);
                *out << formatSpell(spellInfo) << ", ";
            }
            learned = true;
        }
    }

    if (!learned && !bot->HasSpell(spellId)) {
        bot->learnSpell(spellId, false);
        *out << formatSpell(proto) << ", ";

        learned = bot->HasSpell(spellId);
    }

    return learned;
}