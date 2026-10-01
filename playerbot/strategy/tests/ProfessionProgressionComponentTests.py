"""Offline component tests of production bodies with minimal world test doubles.

No realm, database, service, or in-game action is started. The source extraction
keeps the tested predicate/resolver bodies identical to the selected checkout;
these tests complement, rather than replace, full server integration tests.
"""
import argparse
import os
import pathlib
import re
import subprocess
import tempfile

def extract(text, marker):
    start = text.index(marker)
    opening = text.index('{', start)
    depth = 0
    for i in range(opening, len(text)):
        if text[i] == '{': depth += 1
        if text[i] == '}': depth -= 1
        if depth == 0: return text[start:i + 1]
    raise ValueError('Unclosed body: ' + marker)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--core-source', type=pathlib.Path, required=True)
    parser.add_argument('--baseline-ref')
    parser.add_argument('--compiler', default='c++')
    args = parser.parse_args()
    repo = pathlib.Path(__file__).resolve().parents[3]
    def source(path):
        if args.baseline_ref:
            return subprocess.check_output(['git', '-C', str(repo), 'show', args.baseline_ref + ':' + path], text=True)
        return (repo / path).read_text()
    crafts = source('playerbot/strategy/values/CraftValues.cpp')
    usage = source('playerbot/strategy/values/ItemUsageValue.cpp')
    focus = extract((args.core_source / 'src/game/Grids/GridNotifiers.h').read_text(), 'class GameObjectFocusCheck')
    compatibility = extract((args.core_source / 'src/game/Server/DBCStores.cpp').read_text(), 'bool IsTotemCategoryCompatiableWith')
    resolver = extract(crafts, 'GameObject* ProfessionCraftingPlanValue::GetCurrentSpellFocus')
    readiness = extract(crafts, 'bool CanCraftSpellValue::Calculate')
    tools = extract(crafts, 'bool CanCraftSpellValue::HasRequiredTools') if 'bool CanCraftSpellValue::HasRequiredTools' in crafts else ''
    vendor = extract(crafts, 'bool HasCashVendorStock') if 'bool HasCashVendorStock' in crafts else ''
    travel_header = source('playerbot/TravelMgr.h')
    destinations = extract(travel_header, 'const DestinationList* GetEntryDestinations') if 'const DestinationList* GetEntryDestinations' in travel_header else ''
    tool_usage = extract(usage, 'bool ItemUsageValue::IsItemNeededForSkill')
    skill_constants = '\n'.join('constexpr uint32 ' + name + '=' + str(i+1) + ';' for i,name in enumerate(sorted(set(re.findall(r'SKILL_[A-Z_]+',tool_usage)))))
    prefix = r'''
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"
namespace profession = ai::profession;
using namespace std;
using uint32=uint32_t; using int32=int32_t; using ObjectGuid=uint32;
constexpr uint32 GAMEOBJECT_TYPE_SPELL_FOCUS=8;
struct TotemCategoryEntry { uint32 categoryType,categoryMask; };
struct CategoryStore { map<uint32,TotemCategoryEntry> rows; const TotemCategoryEntry* LookupEntry(uint32 id) { auto i=rows.find(id); return i==rows.end()?nullptr:&i->second; } } sTotemCategoryStore;
bool IsTotemCategoryCompatiableWith(uint32,uint32);
struct GameObjectInfo { uint32 type=8; struct { uint32 focusId=0,dist=10; } spellFocus; };
struct WorldObject { float x=0; uint32 map=1; virtual ~WorldObject()=default; };
struct GameObject:WorldObject { GameObjectInfo info; bool spawned=true;
const GameObjectInfo* GetGOInfo()const{return &info;} bool IsSpawned()const{return spawned;}
bool IsWithinDistInMap(const WorldObject* p,float d)const{return map==p->map && abs(x-p->x)<d;} };
struct Player:WorldObject { set<uint32> items,categories; uint32 GetInstanceId(){return 0;}
bool HasItemCount(uint32 id,uint32){return items.count(id);} bool HasItemTotemCategory(uint32 id){for(auto c:categories)if(IsTotemCategoryCompatiableWith(c,id))return true;return false;} };
struct WorldPosition {virtual ~WorldPosition()=default;};
struct GuidPosition:WorldPosition {GameObject* go=nullptr; GameObject* GetGameObject(uint32){return go;}};
enum class TravelStatus {TRAVEL_STATUS_WORK,TRAVEL_STATUS_TRAVEL};
enum class TravelDestinationPurpose {CraftingFocus,Vendor};
struct Destination {TravelDestinationPurpose purpose=TravelDestinationPurpose::CraftingFocus;TravelDestinationPurpose GetPurpose(){return purpose;}};
using DestinationList = vector<Destination*>;
struct VendorItem {uint32 item,maxcount,ExtendedCost;};
struct VendorItemData {vector<VendorItem*> m_items;};
struct TravelTarget { TravelStatus status=TravelStatus::TRAVEL_STATUS_WORK; Destination* destination=nullptr; WorldPosition* position=nullptr;
TravelStatus GetStatus(){return status;} Destination* GetDestination(){return destination;} WorldPosition* GetPosition(){return position;} };
struct CraftToolRequirements {set<uint32> items,categories;};
struct SpellEntry {array<uint32,2> Totem{},TotemCategory{};};
struct ItemPrototype {uint32 ItemId=0,TotemCategory=0;};
struct ServerFacade {map<uint32,SpellEntry> spells; const SpellEntry* LookupSpellInfo(uint32 id){auto i=spells.find(id);return i==spells.end()?nullptr:&i->second;}} sServerFacade;
struct AiObjectContext { TravelTarget* travel=nullptr; list<ObjectGuid> nearby; CraftToolRequirements tools; uint32 reagentCasts=1;
template<class T>T Get(const string&){if constexpr(is_same_v<T,TravelTarget*>)return travel;else if constexpr(is_same_v<T,list<ObjectGuid>>)return nearby;else if constexpr(is_same_v<T,CraftToolRequirements>)return tools;else return T{};}
template<class T>T Get(const string&,uint32){return static_cast<T>(reagentCasts);} };
struct PlayerbotAI {Player* bot;AiObjectContext* context;map<ObjectGuid,GameObject*> objects;
Player* GetBot(){return bot;} AiObjectContext* GetAiObjectContext(){return context;} GameObject* GetGameObject(ObjectGuid id){auto i=objects.find(id);return i==objects.end()?nullptr:i->second;} bool HasSkill(uint32){return false;} };
#define AI_VALUE(T,name) context->Get<T>(name)
#define AI_VALUE2(T,name,q) context->Get<T>(name,q)
struct ProfessionCraftingPlan {uint32 spellId=1,skillId=1,spellFocusId=4;bool IsValid()const{return spellId&&skillId;}};
struct ProfessionCraftingPlanValue {static GameObject* GetCurrentSpellFocus(PlayerbotAI*,const ProfessionCraftingPlan&);};
struct CanCraftSpellValue {Player* bot; AiObjectContext* context;string qualifier="900001"; string getQualifier(){return qualifier;}bool Calculate();static bool HasRequiredTools(const SpellEntry*,Player*);};
struct ItemUsageValue {PlayerbotAI* ai;AiObjectContext* context;bool IsItemNeededForSkill(const ItemPrototype*);};
'''
    extra_declarations = ('struct TravelMgr { map<TravelDestinationPurpose,map<int32,DestinationList>> destinationMap;\n' + destinations + '\n};\n') if destinations else ''
    driver = r'''
int main(){
Player bot; AiObjectContext context;PlayerbotAI ai{&bot,&context,{}};
Destination destination; GuidPosition point;TravelTarget target;target.destination=&destination;target.position=&point;context.travel=&target;
GameObject far,near,wrong;far.x=127;near.x=3;wrong.x=2;far.info.spellFocus.focusId=near.info.spellFocus.focusId=4;wrong.info.spellFocus.focusId=7;
point.go=&far;ai.objects={{1,&near},{2,&wrong}};context.nearby={2,1};ProfessionCraftingPlan plan;
uint32 checks=0;
auto check=[&](bool ok,const char* label){++checks;if(!ok){cerr<<"FAIL: "<<label<<'\n';exit(1);}};
check(ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan)==&near,"far selected spawn must fall back to the in-range exact focus");
near.spawned=false;check(!ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan),"despawned and mismatched focuses must be rejected");
near.spawned=true;near.map=2;check(!ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan),"another-map focus must be rejected");
near.map=1;target.status=TravelStatus::TRAVEL_STATUS_TRAVEL;check(!ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan),"a traveling bot must not bypass the WORK gate");
target.status=TravelStatus::TRAVEL_STATUS_WORK;destination.purpose=TravelDestinationPurpose::Vendor;check(!ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan),"vendor WORK must not impersonate crafting-focus WORK");
destination.purpose=TravelDestinationPurpose::CraftingFocus;point.go=&near;check(ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan)==&near,"valid selected focus must preserve the direct path");
near.info.type=1;check(!ProfessionCraftingPlanValue::GetCurrentSpellFocus(&ai,plan),"same ID on an incorrect GO type must not pass");
SpellEntry spell;spell.Totem[0]=900010;spell.TotemCategory[0]=800001;sServerFacade.spells[900001]=spell;
sTotemCategoryStore.rows={{800001,{24,3}},{800002,{24,7}},{800003,{25,7}},{800004,{24,1}}};
CanCraftSpellValue ready{&bot,&context};
check(!ready.Calculate(),"complete reagents do not replace a missing required tool");bot.items.insert(900010);
check(!ready.Calculate(),"a direct tool does not replace a missing tool category");bot.categories.insert(800003);
check(!ready.Calculate(),"a category with the wrong type must be rejected");bot.categories.insert(800004);
check(!ready.Calculate(),"a category missing required mask bits must be rejected");bot.categories.insert(800002);
check(ready.Calculate(),"a compatible higher-category tool must work without a hard-coded item ID");context.reagentCasts=0;
check(!ready.Calculate(),"tools do not bypass reagent requirements");
context.tools.items={900010};context.tools.categories={800001};ItemUsageValue usage{&ai,&context};ItemPrototype exact{900010,0},replacement{900020,800002},unrelated{900030,800003};
check(usage.IsItemNeededForSkill(&exact),"known direct tools must use normal inventory retention");
check(usage.IsItemNeededForSkill(&replacement),"unknown-name compatible tools must use normal vendor/retention policy");
check(!usage.IsItemNeededForSkill(&unrelated),"unrelated tool categories must not be bought or retained");
'''
    if tools:
        driver += r'''
check(!CanCraftSpellValue::HasRequiredTools(nullptr,&bot),"missing spell metadata must reject readiness");
check(!CanCraftSpellValue::HasRequiredTools(&spell,nullptr),"missing player must reject readiness");
'''
    if vendor and destinations:
        driver += r'''
check(!HasCashVendorStock(nullptr,12345),"a missing vendor stock list is not a source");
VendorItemData stock;
check(!HasCashVendorStock(&stock,12345),"empty vendor stock is not a source");
VendorItem currency{12345,0,123},limited{12345,1,0},unrelatedStock{12346,0,0},cash{12345,0,0};
stock.m_items={&currency};check(!HasCashVendorStock(&stock,12345),"currency stock must allow AH fallback");
stock.m_items={&limited};check(!HasCashVendorStock(&stock,12345),"limited stock is outside the existing vendor index");
stock.m_items={&unrelatedStock};check(!HasCashVendorStock(&stock,12345),"the exact requested reagent must be sold");
stock.m_items={&cash};check(HasCashVendorStock(&stock,12345),"cash stock must retain normal vendor acquisition");
stock.m_items={&currency,&cash};check(!HasCashVendorStock(&stock,12345),"a later cash offer must not bypass BuyItem's first matching currency slot");
stock.m_items={&cash,&currency};check(HasCashVendorStock(&stock,12345),"a first matching cash offer must preserve vendor sourcing");
TravelMgr travel;travel.destinationMap[TravelDestinationPurpose::Vendor][6789]={&destination};
check(travel.GetEntryDestinations(TravelDestinationPurpose::Vendor,6789)->front()==&destination,"indexed lookup must return the existing destination");
check(!travel.GetEntryDestinations(TravelDestinationPurpose::Vendor,6790),"missing entry lookup must fail without creating a destination");
check(!travel.GetEntryDestinations(TravelDestinationPurpose::CraftingFocus,6789),"missing purpose lookup must fail without creating an index");
check(travel.destinationMap.size()==1 && travel.destinationMap.at(TravelDestinationPurpose::Vendor).size()==1,"read-only lookups must not mutate the shared index");
'''
    driver += 'cout<<"PASS: "<<checks<<" production-body focus/tool/vendor/index cases with core focus/category predicates\\n";\n}\n'
    code = prefix + compatibility + '\nnamespace MaNGOS {\n' + focus + ';\n}\n' + skill_constants + '\n' + resolver + '\n' + readiness + '\n' + tools + '\n' + tool_usage + '\n' + vendor + '\n' + extra_declarations + '\n' + driver
    with tempfile.TemporaryDirectory(prefix='profession-components-', dir=repo) as tmp:
        cpp=pathlib.Path(tmp)/'tests.cpp';binary=pathlib.Path(tmp)/('tests.exe' if os.name == 'nt' else 'tests')
        cpp.write_text(code)
        subprocess.run([args.compiler,'-std=c++17','-DMANGOSBOT_TWO','-Wall','-Wextra','-I',str(repo),str(cpp),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)

if __name__=='__main__':main()
