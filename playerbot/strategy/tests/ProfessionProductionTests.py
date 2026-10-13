"""Local C++ regressions of the bounded bridge and actual runtime adapter bodies.

The realm is not started. World doubles test ownership, stack/skill metadata,
loot completion and the real configuration assignments. Full core compilation
and a live inventory/skill trace remain separate validation requirements.
"""
import argparse
import os
import pathlib
import subprocess
import tempfile
from ProfessionProgressionComponentTests import extract

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default='c++')
args = parser.parse_args()
repo = pathlib.Path(__file__).resolve().parents[3]
crafts = (repo / 'playerbot/strategy/values/CraftValues.cpp').read_text()
config = (repo / 'playerbot/PlayerbotAIConfig.cpp').read_text()
start = config.index('    professionProgressionEnabled =')
end = config.index('    //', config.index('    professionAuctionPostLimit =', start))
config_body = config[start:end]
actions = (repo / 'playerbot/strategy/actions/CastCustomSpellAction.cpp').read_text()
cleanup_start = actions.index('    if (professionCraft)', actions.index('bool result = gameObjectTarget ? ai->CastSpell'))
cast_cleanup = extract(actions[cleanup_start:], 'if (professionCraft)')

prefix = r'''
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <string>
#include <ctime>
#include <type_traits>
#include "playerbot/strategy/values/ProfessionProduction.h"
#include "playerbot/strategy/values/ProfessionCraftingFairness.h"
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"
using uint32=uint32_t;using int32=int32_t;using int64=int64_t;using uint8=uint8_t;
using SpellEffectIndex=uint8;using namespace std;using namespace ai;
constexpr uint8 MAX_EFFECT_INDEX=3;
constexpr uint32 SPELL_EFFECT_MILLING=71,SPELL_EFFECT_PROSPECTING=72;
constexpr uint32 ITEM_FLAG_IS_MILLABLE=1,ITEM_FLAG_IS_PROSPECTABLE=2;
constexpr uint32 SKILL_INSCRIPTION=41,SKILL_JEWELCRAFTING=42;
constexpr uint32 LOOT_MILLING=11,LOOT_PROSPECTING=12,HIGHGUID_ITEM=13;
struct ObjectGuid{
 uint32 value=0;ObjectGuid()=default;ObjectGuid(uint32 v):value(v){}
 ObjectGuid(uint32,uint32 entry,uint32):value(entry){}
 explicit operator bool()const{return value!=0;}
 bool operator==(ObjectGuid o)const{return value==o.value;}
 bool operator!=(ObjectGuid o)const{return value!=o.value;}
};
constexpr uint32 SPELL_EFFECT_CREATE_ITEM=73,MAX_SPELL_REAGENTS=8,PLAYERSPELL_REMOVED=9;
constexpr uint32 SKILL_CATEGORY_PROFESSION=43,SKILL_CATEGORY_SECONDARY=44;
struct SpellEntry{uint32 Effect[3]={};int32 quantity=5;
 uint32 EffectItemType[3]={},RequiresSpellFocus=0;int32 Reagent[8]={},ReagentCount[8]={};bool toolsReady=true;
 int32 CalculateSimpleValue(uint8)const{return quantity;}};
struct ItemPrototype{uint32 ItemId=0,Flags=0,RequiredSkillRank=0;};
struct Item{
 ItemPrototype* proto;uint32 count;ObjectGuid guid,owner;bool loot=false;
 uint32 GetEntry()const{return proto->ItemId;}uint32 GetCount()const{return count;}
 ObjectGuid GetOwnerGuid()const{return owner;}ObjectGuid GetObjectGuid()const{return guid;}
 ItemPrototype* GetProto()const{return proto;}bool HasTemporaryLoot()const{return loot;}
};
struct Learned{uint32 state=0;bool disabled=false;};using PlayerSpellMap=map<uint32,Learned>;
bool IsPassiveSpell(uint32){return false;}
struct Player{
 bool trade=false;map<uint32,uint32> skills;vector<Item*> items;
 PlayerSpellMap known;
 const PlayerSpellMap& GetSpellMap(){return known;}
 bool HasSpell(uint32 id){return known.count(id)&&known[id].state!=PLAYERSPELL_REMOVED&&!known[id].disabled;}
 Player* GetTrader(){return trade?this:nullptr;}
 ObjectGuid GetObjectGuid(){return ObjectGuid(900);}
 bool HasSkill(uint32 s){return skills.count(s);}
 uint32 GetSkillValue(uint32 s){return skills[s];}
 Item* GetItemByGuid(ObjectGuid g){for(auto i:items)if(i->guid==g)return i;return nullptr;}
};
struct Server{map<uint32,SpellEntry> spells;
 SpellEntry* LookupSpellInfo(uint32 id){auto i=spells.find(id);return i==spells.end()?nullptr:&i->second;}
}sServerFacade;
struct ProfessionCraftingPlan{
 uint32 spellId=0,skillId=0,processingInputId=0,goalSpellId=0;
 uint32 itemId=0,spellFocusId=0,craftCount=0;
 map<uint32,uint32> required,missing,retained;
 bool IsValid()const{return spellId&&skillId;}
};
struct ProfessionCraftRequest{ProfessionCraftingPlan plan;ObjectGuid inputGuid;bool accepted=false;};
struct ProfessionMaterialSources{};
struct Context{map<string,int32> manual;ProfessionCraftRequest request;int readinessResets=0,planResets=0,sourcesResets=0;
 profession::CraftingFairness fairness;
 vector<uint32> crafts;
 template<class T>auto& Get(string){if constexpr(is_same_v<T,profession::CraftingFairness&>)return fairness;
 else if constexpr(is_same_v<T,vector<uint32>>)return crafts;else return request;}
 void ClearValues(string n){Reset(n);}
 void Reset(string n){if(n=="profession craft request")request={};
 else if(n=="profession crafting plan")++planResets;
 else if(n=="profession material sources")++sourcesResets;
 else if(n=="can craft profession")++readinessResets;else assert(false);}
};
using AiObjectContext=Context;
struct PlayerbotAI{Player* bot;Context context;
 map<uint32,uint32> stock;
 uint32 GetInventoryItemsCountWithId(uint32 id){if(stock.count(id))return stock[id];uint32 total=0;for(auto item:bot->items)if(item->GetEntry()==id)total+=item->count;return total;}
 Player* GetBot(){return bot;}Context* GetAiObjectContext(){return &context;}
 vector<Item*> GetInventoryItems(){return bot->items;}
};
#define AI_VALUE(T,N) (context->Get<T>(N))
#define AI_VALUE2(T,N,K) (context->manual[K])
#define SET_AI_VALUE2(T,N,K,V) (context->manual[K]=(V))
#define RESET_AI_VALUE(T,N) (context->Reset(N))
struct ProfessionCraftingPlanValue{
 Player* bot;PlayerbotAI* ai;AiObjectContext* context;
 ProfessionCraftingPlan Calculate();
 static bool IsEnabledFor(PlayerbotAI*){return true;}
 static bool HasPendingCraft(PlayerbotAI* a){return a->context.manual["pending profession craft"]!=0;}
 static Item* GetProcessingTarget(PlayerbotAI*,const ProfessionCraftingPlan&,ObjectGuid=ObjectGuid());
 static void QueuePlan(PlayerbotAI*,const ProfessionCraftingPlan&,Item*);
 static void QueuePendingCraft(PlayerbotAI*,uint32,bool=false);
 static void ClearPendingCraft(PlayerbotAI*,uint32);
 static void CompleteProcessingLoot(PlayerbotAI*,ObjectGuid);
};
struct ProcessingSource{uint32 inputId,effect;};
using ProcessingSourceMap=map<uint32,vector<ProcessingSource>>;
struct ProcessingSourcesValue{ProcessingSourceMap* Calculate();};
struct LootStoreItem{uint32 itemid;int32 mincountOrRef;uint32 conditionId;bool needs_quest;};
using LootStoreItemList=vector<LootStoreItem>;
struct LootLootGroupAccess{LootStoreItemList ExplicitlyChanced,EqualChanced;};
struct LootTemplateAccess{LootStoreItemList Entries;vector<LootLootGroupAccess> Groups;};
struct Storage{
 map<uint32,ItemPrototype> rows;
 uint32 GetMaxEntry(){return rows.empty()?0:rows.rbegin()->first+1;}
 template<class T>const T* LookupEntry(uint32 id){auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}
}sItemStorage;
struct DropMapValue{static map<pair<uint32,uint32>,LootTemplateAccess> rows;
 static const LootTemplateAccess* GetLootTemplate(ObjectGuid id,uint32 type){auto i=rows.find({id.value,type});return i==rows.end()?nullptr:&i->second;}
};map<pair<uint32,uint32>,LootTemplateAccess> DropMapValue::rows;
struct Config{
 map<string,double> values;
 int GetIntDefault(string k,int d){return values.count(k)?int(values[k]):d;}
 bool GetBoolDefault(string k,bool d){return values.count(k)?values[k]!=0:d;}
 float GetFloatDefault(string k,float d){return values.count(k)?float(values[k]):d;}
};
struct Options{
 bool professionProgressionEnabled;
 uint32 professionProgressionPercent,professionPlanCheckInterval,professionCraftBatchSize,
 professionCraftCooldown,professionMaterialTarget,professionVendorPurchaseLimit,
 professionAhSearchCooldown,professionAhPurchaseLimit,professionAhBudgetPercent,professionAuctionPostLimit;
 float professionAhMaxPriceMultiplier;
 void Load(Config config){
'''

bodies = '\n'.join(extract(crafts, marker) for marker in (
    'ProcessingSourceMap* ProcessingSourcesValue::Calculate',
    'bool ProcessingInput(',
    'Item* ProfessionCraftingPlanValue::GetProcessingTarget',
    'void ProfessionCraftingPlanValue::QueuePlan',
    'void ProfessionCraftingPlanValue::QueuePendingCraft',
    'void ProfessionCraftingPlanValue::ClearPendingCraft',
    'void ProfessionCraftingPlanValue::CompleteProcessingLoot',
    'ProfessionCraftingPlan ProfessionCraftingPlanValue::Calculate'))
bodies += r'''
void CastCleanup(PlayerbotAI* ai,uint32 spell,bool result,int castCount){
 AiObjectContext* context=ai->GetAiObjectContext();
 bool professionCraft=true;
 auto& professionRequest=context->request;
 bool processingCraft=professionRequest.plan.processingInputId;
 bool completesProfessionGoal=!professionRequest.plan.goalSpellId||professionRequest.plan.goalSpellId==spell;
''' + cast_cleanup + '\n}\n'

checks = r'''
int main(){
 int cases=0;auto check=[&](bool b){assert(b);++cases;};
 using namespace ai::profession;
 // Synthetic IDs describe roles, not real professions or sampled spells.
 vector<ProductionRecipe> recipes(3);
 recipes[0].spellId=101;recipes[0].itemId=201;recipes[0].reagents={{202,1},{205,1}};
 recipes[1].spellId=102;recipes[1].itemId=202;recipes[1].reagents={{203,1}};
 recipes[2].spellId=103;recipes[2].itemId=203;recipes[2].processing=true;recipes[2].reagents={{204,5}};
 ProducerIndex index{{202,{1}},{203,{2}}};map<uint32,uint32> stock;
 auto inventory=[&](uint32 id){return stock[id];};
 auto select=[&](uint32 batch=5){return NextProductionStep(recipes,index,0,batch,5,inventory);};
 auto step=select();check(step.recipe==2&&!step.ready&&step.casts==1);
 check(step.required.at(204)==5&&step.retained.at(202)==5&&step.retained.at(203)==5);
 stock[204]=4;check(!select().ready);stock[204]=5;check(select().ready);
 // No pigment is credited by selection or casting. Only observed loot advances.
 check(stock[203]==0&&select().recipe==2);
 stock[203]=2;step=select();check(step.recipe==1&&step.casts==2&&step.ready);
 stock[202]=5;stock[205]=5;step=select();check(step.recipe==0&&step.ready);
 stock.clear();stock[203]=7;recipes[1].yield=2;step=select();check(step.recipe==1&&step.casts==3);
 recipes[1].maxCasts=1;check(select().casts==1);
 recipes[1].maxCasts=5;recipes[1].toolsReady=false;check(!select().ready);
 recipes[1].toolsReady=true;
 index.erase(203);stock.clear();step=select();check(step.recipe==1&&step.required.at(203)==3&&!step.ready);
 // Unknown producer cannot be inferred or granted; direct sourcing remains.
 index.clear();step=select();check(step.recipe==0&&step.required.at(202)==5);
 // Cycles and third prerequisites terminate at a real missing input.
 index={{202,{1}},{203,{2}},{201,{0}}};recipes[2].processing=false;recipes[2].reagents={{201,1}};
 check(select().recipe==2&&!select().ready);
 ProductionRecipe fourth;fourth.spellId=104;fourth.itemId=204;fourth.reagents={{206,1}};
 recipes.push_back(fourth);recipes[2].reagents={{204,1}};index[204]={3};
 check(select().recipe==2&&select().required.count(204)&&!select().required.count(206));
 // Alternative inputs use available stock, without fixed herb/item priorities.
 recipes[2].processing=true;recipes[2].reagents={{204,5}};
 fourth.spellId=103;fourth.itemId=203;fourth.processing=true;fourth.reagents={{207,5}};
 recipes.push_back(fourth);index[203]={2,4};stock={{207,5}};
 check(select().recipe==4&&select().ready);
 // The inherited fairness ledger sees the goal identity across prerequisites.
 CraftingFairness ledger;vector<CraftCandidate> goals={{41,101,1000035000,true},{42,901,1000015000,true}};
 check(ledger.Select(goals,100,false)==0);
 check(ledger.Select(goals,399,false)==0);
 check(ledger.Select(goals,400,false)==1);
 check(ledger.Select(goals,800,true)==1);
 // Exhaustive input counts/batch quantities remain bounded and truthful.
 for(uint32 batch=1;batch<=20;++batch)for(uint32 count=0;count<=50;++count){
   recipes[1].yield=1;recipes[1].maxCasts=5;stock={{203,count}};index[203]={2};
   auto s=select(batch);check(s.casts>=1&&s.casts<=5);
   if(s.ready&&s.recipe==1)check(s.required.at(203)<=count);
 }
 Player bot;bot.skills[SKILL_INSCRIPTION]=1;PlayerbotAI ai{&bot,{}, {}};
 ItemPrototype herb{301,ITEM_FLAG_IS_MILLABLE,1},ore{302,ITEM_FLAG_IS_PROSPECTABLE,1};
 SpellEntry mill;mill.Effect[1]=SPELL_EFFECT_MILLING;sServerFacade.spells[401]=mill;
 Item item{&herb,5,ObjectGuid(501),bot.GetObjectGuid(),false};bot.items={&item};
 ProfessionCraftingPlan plan;plan.spellId=401;plan.skillId=SKILL_INSCRIPTION;plan.processingInputId=301;
 check(ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan)==&item);
 item.count=4;check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));item.count=5;
 herb.RequiredSkillRank=2;check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));herb.RequiredSkillRank=1;
 herb.Flags=0;check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));herb.Flags=ITEM_FLAG_IS_MILLABLE;
 item.loot=true;check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));item.loot=false;
 item.owner=ObjectGuid(901);check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));item.owner=bot.GetObjectGuid();
 bot.trade=true;check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));bot.trade=false;
 check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan,ObjectGuid(502)));
 // Aggregate inventory is insufficient if no individual stack has enough.
 Item small=item;small.count=3;small.guid=ObjectGuid(502);item.count=2;bot.items={&item,&small};
 check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));item.count=5;
 mill.quantity=3;sServerFacade.spells[401]=mill;item.count=3;
 check(ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan)==&item);
 mill.quantity=0;sServerFacade.spells[401]=mill;check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,plan));
 mill.quantity=5;sServerFacade.spells[401]=mill;item.count=5;
 SpellEntry prospect;prospect.Effect[0]=SPELL_EFFECT_PROSPECTING;sServerFacade.spells[402]=prospect;
 Item metal{&ore,5,ObjectGuid(503),bot.GetObjectGuid(),false};bot.items.push_back(&metal);
 auto jc=plan;jc.spellId=402;jc.skillId=SKILL_JEWELCRAFTING;jc.processingInputId=302;
 check(!ProfessionCraftingPlanValue::GetProcessingTarget(&ai,jc));bot.skills[SKILL_JEWELCRAFTING]=1;
 check(ProfessionCraftingPlanValue::GetProcessingTarget(&ai,jc)==&metal);
 ProfessionCraftingPlanValue::QueuePlan(&ai,plan,&item);
 check(ai.context.manual["pending profession craft"]==401&&ai.context.request.inputGuid==item.guid);
 ProfessionCraftingPlanValue::CompleteProcessingLoot(&ai,item.guid);
 check(ai.context.request.plan.IsValid()); // No accepted cast, no completion.
 ai.context.request.accepted=true;ProfessionCraftingPlanValue::CompleteProcessingLoot(&ai,metal.guid);
 check(ai.context.request.plan.IsValid()&&ai.context.planResets==0);
 ProfessionCraftingPlanValue::ClearPendingCraft(&ai,402);check(ai.context.request.plan.IsValid());
 ProfessionCraftingPlanValue::CompleteProcessingLoot(&ai,item.guid);
 check(!ai.context.request.plan.IsValid()&&!ai.context.manual["pending profession craft"]);
 check(ai.context.planResets==1&&ai.context.sourcesResets==1);
 // Missing responses have the same safe bounded request reset as ordinary craft.
 ProfessionCraftingPlanValue::QueuePlan(&ai,plan,&item);
 ProfessionCraftingPlanValue::ClearPendingCraft(&ai,401);check(!ai.context.request.inputGuid);
 // Exercise the actual post-cast block, including snapshot reset ordering.
 auto ordinary=plan;ordinary.processingInputId=0;ordinary.goalSpellId=999;
 ai.context.fairness.ownerSkill=41;ai.context.fairness.ownerSpell=999;
 ProfessionCraftingPlanValue::QueuePlan(&ai,ordinary,nullptr);CastCleanup(&ai,401,true,1);
 check(ai.context.fairness.ownerSpell==999&&!ai.context.request.plan.IsValid());
 ordinary.goalSpellId=401;ProfessionCraftingPlanValue::QueuePlan(&ai,ordinary,nullptr);
 CastCleanup(&ai,401,true,1);check(!ai.context.fairness.ownerSpell);
 ProfessionCraftingPlanValue::QueuePlan(&ai,ordinary,nullptr);CastCleanup(&ai,401,true,2);
 check(ai.context.request.plan.IsValid()&&ai.context.manual["pending profession craft"]==401);
 ProfessionCraftingPlanValue::QueuePlan(&ai,plan,&item);CastCleanup(&ai,401,true,1);
 check(ai.context.request.accepted&&ai.context.request.inputGuid==item.guid&&ai.context.manual["pending profession craft"]==401);
 CastCleanup(&ai,401,false,1);check(!ai.context.request.plan.IsValid());
 sItemStorage.rows={{301,herb},{302,ore}};
 DropMapValue::rows[{301,LOOT_MILLING}].Entries={{601,1,0,false},{602,-4,0,false},{603,1,7,false},{604,1,0,true}};
 DropMapValue::rows[{302,LOOT_PROSPECTING}].Groups.push_back({{{605,1,0,false}},{{606,1,0,false}}});
 ProcessingSourcesValue shared;auto outputs=shared.Calculate();
 check(outputs->size()==3&&outputs->at(601)[0].inputId==301);
 check(outputs->at(605)[0].effect==SPELL_EFFECT_PROSPECTING&&outputs->count(606));
 check(!outputs->count(602)&&!outputs->count(603)&&!outputs->count(604));delete outputs;
 Options opt;Config c;opt.Load(c);check(opt.professionProgressionEnabled&&opt.professionProgressionPercent==10);
 check(opt.professionCraftBatchSize==5&&opt.professionMaterialTarget==20&&opt.professionAhSearchCooldown==600);
 c.values["AiPlayerbot.ProfessionProgression.CanaryPercent"]=27;opt.Load(c);check(opt.professionProgressionPercent==27);
 c.values["AiPlayerbot.ProfessionProgressionPercent"]=10;opt.Load(c);check(opt.professionProgressionPercent==10);
 c.values["AiPlayerbot.ProfessionProgressionPercent"]=-1;opt.Load(c);check(opt.professionProgressionPercent==0);
 c.values["AiPlayerbot.ProfessionProgressionPercent"]=101;opt.Load(c);check(opt.professionProgressionPercent==100);
 c.values["AiPlayerbot.ProfessionProgression.CraftBatchSize"]=8;opt.Load(c);check(opt.professionCraftBatchSize==8);
 c.values["AiPlayerbot.ProfessionCraftBatchSize"]=2;opt.Load(c);check(opt.professionCraftBatchSize==2);
 c.values["AiPlayerbot.ProfessionProgression.Enabled"]=0;opt.Load(c);check(!opt.professionProgressionEnabled);
 c.values["AiPlayerbot.ProfessionProgressionEnabled"]=1;opt.Load(c);check(opt.professionProgressionEnabled);
 cout<<"PASS: "<<cases<<" production demand, processing ownership/metadata, fairness and config cases\n";
}
'''

planner_world = r'''
Options sPlayerbotAIConfig;
struct ObjectMgr{static const ItemPrototype* GetItemPrototype(uint32 id){return sItemStorage.LookupEntry<ItemPrototype>(id);}};
struct SkillLineEntry{uint32 categoryId;};
struct SkillStore{map<uint32,SkillLineEntry> rows;const SkillLineEntry* LookupEntry(uint32 id){auto i=rows.find(id);return i==rows.end()?nullptr:&i->second;}}sSkillLineStore;
struct SkillLineAbilityEntry{uint32 skillId,min_value;};
using SkillLineAbilityMap=multimap<uint32,SkillLineAbilityEntry*>;
using SkillLineAbilityMapBounds=pair<SkillLineAbilityMap::const_iterator,SkillLineAbilityMap::const_iterator>;
struct SpellMgr{SkillLineAbilityMap rows;SkillLineAbilityMapBounds GetSkillLineAbilityMapBoundsBySpellId(uint32 id){return rows.equal_range(id);}}sSpellMgr;
struct ShouldCraftSpellValue{static set<uint32> useful;static bool SpellGivesSkillUp(uint32 id,Player*){return useful.count(id);}};
set<uint32> ShouldCraftSpellValue::useful;
struct CanCraftSpellValue{static bool HasRequiredTools(const SpellEntry* s,Player*){return s&&s->toolsReady;}};
ProcessingSourceMap sharedProcessing;
#define GAI_VALUE(T,N) (&sharedProcessing)
'''
planner_checks = r'''
 // Compile and execute the whole production planner adapter with world doubles.
 // A grey learned ink producer remains usable for a useful final recipe.
 sPlayerbotAIConfig.Load(Config{});
 sServerFacade.spells[711]={};sServerFacade.spells[712]={};
 auto& ink=sServerFacade.spells[711];ink.Effect[0]=SPELL_EFFECT_CREATE_ITEM;ink.EffectItemType[0]=701;
 ink.quantity=1;ink.Reagent[0]=601;ink.ReagentCount[0]=1;
 auto& scroll=sServerFacade.spells[712];scroll.Effect[0]=SPELL_EFFECT_CREATE_ITEM;scroll.EffectItemType[0]=702;
 scroll.quantity=1;scroll.Reagent[0]=701;scroll.ReagentCount[0]=1;scroll.Reagent[1]=703;scroll.ReagentCount[1]=1;
 SkillLineAbilityEntry inkSkill{SKILL_INSCRIPTION,15},scrollSkill{SKILL_INSCRIPTION,35};
 sSpellMgr.rows={{711,&inkSkill},{712,&scrollSkill}};
 sSkillLineStore.rows[SKILL_INSCRIPTION]={SKILL_CATEGORY_PROFESSION};
 bot.known={{401,{}},{711,{}},{712,{}}};ai.context.crafts={711,712};
 ai.context.manual["pending profession craft"]=0;ai.context.request={};ai.context.fairness={};
 ShouldCraftSpellValue::useful={712};sharedProcessing={{601,{{301,SPELL_EFFECT_MILLING}}}};
 ai.stock.clear();bot.items={&item};item.count=5;ai.stock[703]=5;
 ProfessionCraftingPlanValue planner{&bot,&ai,&ai.context};
 auto planned=planner.Calculate();
 check(planned.spellId==401&&planned.goalSpellId==712&&planned.processingInputId==301);
 check(planned.required.at(301)==5&&planned.missing.empty()&&planned.craftCount==1);
 check(planned.retained.count(601)&&planned.retained.count(701)&&planned.retained.count(703));
 ProfessionCraftingPlanValue::QueuePlan(&ai,planned,&item);ai.stock[601]=2;
 check(planner.Calculate().spellId==401); // Pending loot/cast still owns its snapshot.
 ProfessionCraftingPlanValue::ClearPendingCraft(&ai,401);
 planned=planner.Calculate();check(planned.spellId==711&&planned.goalSpellId==712&&planned.craftCount==2);
 check(planned.required.at(601)==2&&planned.missing.empty());
 ai.stock[701]=5;planned=planner.Calculate();check(planned.spellId==712&&planned.missing.empty());
 ai.stock[701]=0;ai.stock[601]=0;item.count=4;planned=planner.Calculate();
 check(planned.spellId==401&&planned.missing.at(301)==1); // Real raw reagent demand.
 bot.known.erase(401);planned=planner.Calculate();check(planned.spellId==711&&planned.missing.count(601));
 bot.known.erase(711);ai.context.crafts={712};planned=planner.Calculate();
 check(planned.spellId==712&&planned.missing.count(701)); // No invented ink recipe.
'''
checks = checks.replace(' cout<<"PASS:', planner_checks + '\n cout<<"PASS:')
code = prefix + config_body + '\n}};\n' + planner_world + bodies + '\n' + checks
with tempfile.TemporaryDirectory(prefix='profession-production-') as tmp:
    cpp = pathlib.Path(tmp) / 'tests.cpp'
    binary = pathlib.Path(tmp) / ('tests.exe' if os.name == 'nt' else 'tests')
    cpp.write_text(code)
    subprocess.run([args.compiler, '-std=c++17', '-DMANGOSBOT_TWO', '-Wall', '-Wextra',
                    '-Werror', '-I', str(repo), str(cpp), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
