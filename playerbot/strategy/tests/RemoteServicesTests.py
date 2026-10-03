"""Compile actual scoped access, patched core predicates and maintenance dispatch.

No realm/database/full build is used. --core-source is an unmodified compatible
CMaNGOS WotLK checkout; its three files are copied to a temporary directory and
the companion patch is applied there only. World doubles cannot establish live
auction settlement, mail persistence or full core ABI/build compatibility.
"""
import argparse
import os
import pathlib
import subprocess
import tempfile
from ProfessionProgressionComponentTests import extract

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default='c++')
parser.add_argument('--core-source', required=True)
parser.add_argument('--baseline-ref', help='Optional audit comparison of unchanged transaction/planner bodies')
args = parser.parse_args()
repo = pathlib.Path(__file__).resolve().parents[3]
core = pathlib.Path(args.core_source)

def source(path):
    return (repo / path).read_text(encoding='utf-8')

access = source('playerbot/RemoteServiceAccess.cpp')
access = access.replace('#include "playerbot/playerbot.h"', '')
access = access.replace('#include "Server/WorldSession.h"', '')
dispatch = source('playerbot/strategy/actions/RemoteServicesAction.cpp')
dispatch = extract(dispatch, 'bool RemoteServicesAction::IsReady') + '\n' + extract(
    dispatch, 'bool RemoteServicesAction::Execute')
ah_route = extract(source('playerbot/strategy/actions/AhAction.cpp'), 'bool AhAction::Execute')
mail_route = extract(source('playerbot/strategy/actions/MailAction.cpp'), 'ObjectGuid MailProcessor::FindMailbox')
bank_route = extract(source('playerbot/strategy/actions/BankAction.cpp'), 'bool BankAction::ExecuteCommand')
dispatch_header = source('playerbot/strategy/actions/RemoteServicesAction.h').replace(
    '#include "playerbot/strategy/Action.h"', '').replace('#include "playerbot/strategy/Value.h"', '')

# These are deliberately the existing transaction implementations, not a new
# auction/mail/bank implementation. Exact bodies must remain unchanged.
for path, markers in {
    'playerbot/strategy/actions/AhAction.cpp': ['bool AhAction::PostItem', 'bool AhBidAction::BidItem'],
    'playerbot/strategy/actions/BankAction.cpp': ['bool BankAction::AutoDeposit', 'bool BankAction::AutoWithdraw'],
}.items():
    if args.baseline_ref:
        baseline = subprocess.check_output(['git', 'show', args.baseline_ref+':'+path], cwd=repo, text=True)
        for marker in markers:
            assert extract(source(path), marker) == extract(baseline, marker), marker
mail = source('playerbot/strategy/actions/MailAction.cpp')
if args.baseline_ref:
    baseline_mail = subprocess.check_output(['git', 'show', args.baseline_ref+':playerbot/strategy/actions/MailAction.cpp'], cwd=repo, text=True)
    assert mail[mail.index('class TakeMailProcessor'):mail.index('class DeleteMailProcessor')] == baseline_mail[
        baseline_mail.index('class TakeMailProcessor'):baseline_mail.index('class DeleteMailProcessor')]
    for path in ('CraftValues.cpp', 'ProfessionProgressionPolicy.h', 'ProfessionProduction.h', 'ProfessionCraftingFairness.h'):
        path='playerbot/strategy/values/'+path
        assert source(path) == subprocess.check_output(['git', 'show', args.baseline_ref+':'+path], cwd=repo, text=True)
assert 'HandleAuctionSellItem(packet)' in source('playerbot/strategy/actions/AhAction.cpp')
assert 'HandleAuctionPlaceBid(packet)' in source('playerbot/strategy/actions/AhAction.cpp')
assert 'HandleMailTakeItem(packet)' in mail and 'HandleMailTakeMoney(packet)' in mail
assert 'bot->CanBankItem' in source('playerbot/strategy/actions/BankAction.cpp')
assert 'bot->CanStoreItem' in source('playerbot/strategy/actions/BankAction.cpp')
assert 'RemoteServiceAccess::IsUsableNow(ai)' in extract(source('playerbot/strategy/actions/BankAction.cpp'), 'bool BankAction::Execute')
assert 'RemoteServiceAccess access(ai, RemoteService::Mail)' in extract(mail, 'bool MailAction::Execute')
assert '"val::remote services ready"' in source('playerbot/strategy/generic/MaintenanceStrategy.cpp')
assert 'creators["remote services"]' in source('playerbot/strategy/actions/ActionContext.h')
assert 'creators["remote services ready"]' in source('playerbot/strategy/values/ValueContext.h')
assert 'GetBoolDefault("AiPlayerbot.RandomBotRemoteServices", false)' in source('playerbot/PlayerbotAIConfig.cpp')
triggers = source('playerbot/strategy/triggers/RpgTriggers.cpp')
for trigger in ('RpgAHSellTrigger', 'RpgAHBuyTrigger', 'RpgGetMailTrigger', 'RpgBankDepositTrigger', 'RpgBankWithdrawTrigger'):
    assert 'RemoteServiceAccess::IsEnabledFor(ai)' in extract(triggers, 'bool '+trigger+'::IsActive')
travel=source('playerbot/strategy/values/TravelValues.cpp')
for marker in ('case TravelDestinationPurpose::AH:', 'case TravelDestinationPurpose::Mail:', 'else if (name == "profession auction house")'):
    assert 'RemoteServiceAccess::IsEnabledFor(ai)' in travel[travel.index(marker):travel.index(marker)+240]

prefix = r'''
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>
#include "playerbot/RemoteServiceAccess.h"
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"
using uint32=uint32_t; using int32=int32_t; using uint8=uint8_t;
using namespace ai; using namespace std;
int checks=0;void check(bool v){++checks;if(!v){cerr<<"FAILED check "<<checks<<"\n";abort();}}
uint32 clockNow=10000;
#define time(...) clockNow
#define DEBUG_LOG(...) ((void)0)
constexpr int UNIT_NPC_FLAG_NONE=0,UNIT_NPC_FLAG_AUCTIONEER=1,GAMEOBJECT_TYPE_MAILBOX=2;
constexpr int TYPEID_UNIT=1,ALLIANCE=0,HORDE=1;
constexpr int CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_AUCTION=1;
constexpr int FACTION_GROUP_MASK_ALLIANCE=1,FACTION_GROUP_MASK_HORDE=2;
namespace CreatureTypeFlags{constexpr int INTERACT_ONLY_WITH_CREATOR=1;}
struct ObjectGuid {int value=0,kind=0; bool operator==(ObjectGuid v)const{return value==v.value&&kind==v.kind;}
 bool operator!=(ObjectGuid v)const{return !(*this==v);}explicit operator bool()const{return value!=0;}
 bool IsGameObject()const{return kind==1;}bool IsAnyTypeCreature()const{return kind==2;}};
struct CreatureInfo{bool HasFlag(int)const{return true;}};
struct Unit{virtual ~Unit()=default;virtual int GetTypeId()const{return TYPEID_UNIT;}int GetFaction()const{return 0;}};
struct Creature:Unit{CreatureInfo info;ObjectGuid owner;CreatureInfo* GetCreatureInfo(){return &info;}ObjectGuid GetOwnerGuid(){return owner;}};
struct GameObject{ObjectGuid guid;int GetGoType()const{return GAMEOBJECT_TYPE_MAILBOX;}ObjectGuid GetObjectGuid()const{return guid;}};
struct AuctionHouseEntry{uint32 houseId;};
struct FactionTemplateEntry{int factionGroupMask=0;};
struct FactionStore{FactionTemplateEntry* LookupEntry(int){return nullptr;}}sFactionTemplateStore;
struct AuctionStore{AuctionHouseEntry entry;AuctionHouseEntry const* LookupEntry(uint32 id){entry.houseId=id;return &entry;}}sAuctionHouseStore;
struct World{bool shared=false;bool getConfig(int){return shared;}}sWorld;
struct Config{bool randomBotRemoteServices=true;}sPlayerbotAIConfig;
enum class BotState{BOT_STATE_NON_COMBAT};
struct Event{string name,param;Event(string n="",string p=""):name(n),param(p){}
 Player* getOwner(){return nullptr;}string getParam(){return param;}};
struct ProfessionCraftingPlan{};
struct Context{map<string,int32> manual;map<string,bool> flags;vector<string> clears;list<ObjectGuid> npcs,objects;
 template<class T>T Get(const string& name){if constexpr(is_same_v<T,bool>)return flags[name];else if constexpr(is_same_v<T,ProfessionCraftingPlan>)return {};else if constexpr(is_same_v<T,list<ObjectGuid>>)return npcs;else return T{};}
 template<class T>T* GetValue(const string&){return &objects;}
 void ClearValues(const string& n){clears.push_back(n);}void Reset(const string& n){clears.push_back(n);}};
#define AI_VALUE(T,N) context->Get<T>(N)
#define AI_VALUE2(T,N,K) context->manual[K]
#define SET_AI_VALUE2(T,N,K,V) (context->manual[K]=(V))
#define RESET_AI_VALUE(T,N) context->Reset(N)
using AiObjectContext=Context;
class Player:public Unit{
public:
 PlayerbotAI* ai=nullptr;ObjectGuid guid;bool random=true,free=true,alive=true,combat=false,taxi=false,teleport=false,casting=false,trade=false,looting=false,gm=false,npc=false,mailbox=false;
 int mode=0,team=ALLIANCE,security=0;Creature creature;
 Player(int id):guid{id,0}{}int GetTypeId()const override{return 2;}
 ObjectGuid GetObjectGuid()const{return guid;}PlayerbotAI* GetPlayerbotAI(){return ai;}
 bool IsAlive(){return alive;}bool IsInCombat(){return combat;}bool IsTaxiFlying(){return taxi;}
 bool IsBeingTeleported(){return teleport;}bool IsNonMeleeSpellCasted(bool){return casting;}
 void* GetTradeData(){return trade?this:nullptr;}ObjectGuid GetLootGuid(){return {looting?1:0,0};}
 int GetAuctionAccessMode(){return mode;}void SetAuctionAccessMode(int v){mode=v;}int GetTeam(){return team;}
 Creature* GetNPCIfCanInteractWith(ObjectGuid,int){return npc?&creature:nullptr;}
 void* GetGameObjectIfCanInteractWith(ObjectGuid,int){return mailbox?this:nullptr;}
};
struct ItemPrototype{uint32 ItemId=0;};struct Item{ItemPrototype proto;ItemPrototype* GetProto(){return &proto;}};
enum class IterateItemsMask{ITERATE_ITEMS_IN_BANK,ITERATE_ITEMS_IN_BAGS};
class PlayerbotAI{
public:
 Player* bot;Context ctx;bool real=false,master=false,pending=false,buyProfession=false;
 int deposits=0,withdrawals=0;vector<string> calls;set<string> strategies={"rpg vendor","rpg bank"};list<Item*> parsed;GameObject go;
 PlayerbotAI(Player* p):bot(p){p->ai=this;}Player* GetBot(){return bot;}bool IsRealPlayer(){return real;}
 bool HasRealPlayerMaster(){return master;}Context* GetAiObjectContext(){return &ctx;}
 bool HasStrategy(string n,BotState){return strategies.count(n)!=0;}
 bool DoSpecificAction(string n,Event e,bool){calls.push_back(n+":"+e.param);return true;}
 void TellPlayerNoFacing(Player*,const char*){}list<Item*> InventoryParseItems(string,IterateItemsMask){return parsed;}
 GameObject* GetGameObject(ObjectGuid v){return v==go.guid?&go:nullptr;}
};
struct RandomMgr{mutex m_ahActionMutex;bool IsRandomBot(Player* p){return p->random;}bool IsFreeBot(Player* p){return p->free;}}sRandomPlayerbotMgr;
struct ChatHandler{Player* p;ChatHandler(Player* v):p(v){}bool FindCommand(const char*){return p->gm;}};
struct AuctionHouseMgr{static AuctionHouseEntry const* GetAuctionHouseEntry(Unit*);};
struct WorldSession{Player* p;Player* GetPlayer()const{return p;}
 bool CheckMailBox(ObjectGuid)const;AuctionHouseEntry const* GetCheckedAuctionHouseForAuctioneer(ObjectGuid)const;};
struct AhAction{PlayerbotAI* ai;Player* bot;Context* context;int transactions=0;bool throwTransaction=false;
 AhAction(PlayerbotAI* a):ai(a),bot(a->bot),context(&a->ctx){}
 Player* GetMaster(){return nullptr;}bool Execute(Event&);
 bool ExecuteCommand(Player*,string,Unit* u){++transactions;if(throwTransaction)throw 1;
  return WorldSession{bot}.GetCheckedAuctionHouseForAuctioneer(u==bot?bot->guid:ObjectGuid{8,2})!=nullptr;}};
struct MailProcessor{static ObjectGuid FindMailbox(PlayerbotAI*);};
struct ProfessionCraftingPlanValue{
 static bool HasPendingCraft(PlayerbotAI* a){return a->pending;}
 static bool ShouldTravelToAuctionHouse(PlayerbotAI* a,const ProfessionCraftingPlan&){return a->buyProfession;}};
struct BankAction{PlayerbotAI* ai;Player* bot;BankAction(PlayerbotAI* a):ai(a),bot(a->bot){}
 bool ExecuteCommand(Player*,const string&,Unit*);void ListItems(Player*){}
 bool Withdraw(Player*,uint32){++ai->withdrawals;return true;}
 bool Deposit(Player*,Item*){++ai->deposits;return true;}
 bool AutoDeposit(){++ai->deposits;return true;}bool AutoWithdraw(){++ai->withdrawals;return true;}};
namespace ai {
class Action{protected:PlayerbotAI* ai;Context* context;Player* bot;
public:Action(PlayerbotAI* a,string):ai(a),context(&a->ctx),bot(a->bot){}
 virtual bool Execute(Event&){return false;}virtual bool isUseful(){return false;}};
class BoolCalculatedValue{protected:PlayerbotAI* ai;
public:BoolCalculatedValue(PlayerbotAI* a,string,int):ai(a){}virtual bool Calculate(){return false;}};
}
'''

tests = r'''
int main(){
 Player a(100),b(900);PlayerbotAI ai(&a),bi(&b);WorldSession session{&a};Event event;
#if CMANGOS_PLAYERBOT_REMOTE_SERVICES == 1
 check(RemoteServiceAccess::IsEnabledFor(&ai));check(!RemoteServiceAccess::IsEnabledFor(nullptr));
 check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));
 check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)==nullptr);check(!session.CheckMailBox(a.guid));
 a.mode=-1;
 {RemoteServiceAccess outer(&ai,RemoteService::Auction);check(a.mode==0);
  check(RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));
  check(!RemoteServiceAccess::IsAllowed(&b,RemoteService::Auction));
  check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Mail));
  check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)->houseId==1);
  a.team=HORDE;check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)->houseId==6);
  sWorld.shared=true;check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)->houseId==1);sWorld.shared=false;
  check(!session.CheckMailBox(a.guid));
  bool otherThread=true;thread t([&]{otherThread=RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction);});t.join();check(!otherThread);
  {RemoteServiceAccess inner(&bi,RemoteService::Mail);check(RemoteServiceAccess::IsAllowed(&b,RemoteService::Mail));check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));}
  check(RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));
 }
 check(a.mode==-1);check(a.security==0);a.mode=0;
 try{RemoteServiceAccess access(&ai,RemoteService::Mail);check(session.CheckMailBox(a.guid));
     check(!session.CheckMailBox({55,1}));check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)==nullptr);throw 1;
 }catch(int){}
 check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Mail));check(!session.CheckMailBox(a.guid));
 for(int state=0;state<10;++state){
  switch(state){case 0:sPlayerbotAIConfig.randomBotRemoteServices=false;break;case 1:ai.real=true;break;case 2:ai.master=true;break;
   case 3:a.random=false;break;case 4:a.free=false;break;case 5:a.alive=false;break;case 6:a.combat=true;break;
   case 7:a.casting=true;break;case 8:a.trade=true;break;case 9:a.looting=true;break;}
  {RemoteServiceAccess access(&ai,RemoteService::Mail);check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Mail));check(!session.CheckMailBox(a.guid));}
  sPlayerbotAIConfig.randomBotRemoteServices=true;ai.real=ai.master=false;
  a.random=a.free=a.alive=true;a.combat=a.casting=a.trade=a.looting=false;
 }
 a.taxi=true;check(!RemoteServicesAction::IsReady(&ai));a.taxi=false;
 a.teleport=true;check(!RemoteServicesAction::IsReady(&ai));a.teleport=false;
 ai.pending=true;check(!RemoteServicesAction::IsReady(&ai));ai.pending=false;
 {RemoteServiceAccess access(&ai,RemoteService::Mail);sPlayerbotAIConfig.randomBotRemoteServices=false;check(!session.CheckMailBox(a.guid));}sPlayerbotAIConfig.randomBotRemoteServices=true;
 // An ordinary real mailbox/auctioneer remains a valid nonremote interaction.
 a.mailbox=true;a.npc=true;check(session.CheckMailBox({9,1}));check(session.GetCheckedAuctionHouseForAuctioneer({9,2})!=nullptr);a.mailbox=a.npc=false;
 a.gm=true;check(session.CheckMailBox(a.guid));check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)!=nullptr);a.gm=false;
 // Actual AH dispatcher works with no nearby NPC and releases both mutex and
 // scoped authorization when an action throws or the mutex is already held.
 AhAction ah(&ai);check(ah.Execute(event));check(ah.transactions==1);
 check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));
 sRandomPlayerbotMgr.m_ahActionMutex.lock();check(!ah.Execute(event));sRandomPlayerbotMgr.m_ahActionMutex.unlock();
 check(ah.transactions==1);ah.throwTransaction=true;
 try{ah.Execute(event);check(false);}catch(int){}
 check(sRandomPlayerbotMgr.m_ahActionMutex.try_lock());sRandomPlayerbotMgr.m_ahActionMutex.unlock();
 check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));ah.throwTransaction=false;
 sPlayerbotAIConfig.randomBotRemoteServices=false;check(!ah.Execute(event));sPlayerbotAIConfig.randomBotRemoteServices=true;
 check(!MailProcessor::FindMailbox(&ai));
 {RemoteServiceAccess mailAccess(&ai,RemoteService::Mail);check(MailProcessor::FindMailbox(&ai)==a.guid);}
 ai.go.guid={999,1};ai.ctx.objects={ai.go.guid};check(MailProcessor::FindMailbox(&ai)==ai.go.guid);ai.ctx.objects.clear();
 // Actual bank command aggregation reports successful existing operations.
 Item item;item.proto.ItemId=77;BankAction manualBank(&ai);
 check(!manualBank.ExecuteCommand(nullptr,"item",&a));ai.parsed={&item};
 check(manualBank.ExecuteCommand(nullptr,"item",&a));check(manualBank.ExecuteCommand(nullptr,"-item",&a));
 ai.deposits=ai.withdrawals=0;ai.parsed.clear();
 RemoteServicesAction action(&ai);check(action.IsReady(&ai));
 check(!action.Execute(event));check(!action.IsReady(&ai));check(ai.ctx.manual["last remote services"]==10000);
 ai.ctx.flags["can get mail"]=true;ai.ctx.flags["should get mail"]=true;
 ai.ctx.flags["should bank withdraw"]=true;ai.ctx.flags["should bank deposit"]=true;
 ai.ctx.flags["can ah buy"]=true;ai.ctx.flags["can ah sell"]=true;ai.ctx.flags["should ah sell"]=true;
 clockNow=10060;check(action.Execute(event));check(ai.withdrawals==1&&ai.deposits==1);
 check(find(ai.calls.begin(),ai.calls.end(),"mail:take")!=ai.calls.end());
 check(find(ai.calls.begin(),ai.calls.end(),"ah bid:vendor")==ai.calls.end()); // Empty first AH attempt is throttled too.
 ai.calls.clear();clockNow=10600;check(action.Execute(event));
 check(find(ai.calls.begin(),ai.calls.end(),"ah:vendor")!=ai.calls.end());
 check(find(ai.calls.begin(),ai.calls.end(),"ah bid:vendor")!=ai.calls.end());
 ai.buyProfession=true;ai.calls.clear();clockNow=11200;check(action.Execute(event));
 check(find(ai.calls.begin(),ai.calls.end(),"ah bid:profession")!=ai.calls.end());
 check(find(ai.calls.begin(),ai.calls.end(),"ah bid:vendor")==ai.calls.end());
 check(!ai.ctx.clears.empty());
 ai.ctx.flags["should get mail"]=false;ai.ctx.flags["should bank withdraw"]=false;
 ai.ctx.flags["should bank deposit"]=false;ai.ctx.flags["can ah sell"]=false;ai.ctx.flags["can ah buy"]=false;
 ai.buyProfession=false;ai.calls.clear();clockNow=11260;check(!action.Execute(event));check(ai.calls.empty());
 clockNow=100;check(action.IsReady(&ai)); // Clock rollback cannot lock maintenance forever.
#else
 check(!RemoteServiceAccess::IsEnabledFor(&ai));
 check(!session.CheckMailBox(a.guid));check(session.GetCheckedAuctionHouseForAuctioneer(a.guid)==nullptr);
 {RemoteServiceAccess access(&ai,RemoteService::Auction);check(!RemoteServiceAccess::IsAllowed(&a,RemoteService::Auction));}
 RemoteServicesAction action(&ai);check(!action.IsReady(&ai));check(!action.Execute(event));check(ai.calls.empty());
#endif
 cout<<"PASS: "<<checks<<" remote service checks, bridge="<<CMANGOS_PLAYERBOT_REMOTE_SERVICES<<"\n";
}
'''

with tempfile.TemporaryDirectory(prefix='playerbots-remote-services-') as tmp:
    tmp = pathlib.Path(tmp)
    subprocess.run(['git', 'init', '-q', str(tmp)], check=True)
    for path in ('src/game/Server/WorldSession.h', 'src/game/AuctionHouse/AuctionHouseHandler.cpp', 'src/game/Mails/MailHandler.cpp'):
        target=tmp/path;target.parent.mkdir(parents=True,exist_ok=True)
        target.write_bytes((core/path).read_bytes())
    patch=repo/'patches/cmangos-wotlk-remote-services.patch'
    subprocess.run(['git', '-C', str(tmp), 'apply', '--check', str(patch)], check=True)
    subprocess.run(['git', '-C', str(tmp), 'apply', str(patch)], check=True)
    assert '#define CMANGOS_PLAYERBOT_REMOTE_SERVICES 1' in (tmp/'src/game/Server/WorldSession.h').read_text()
    core_methods=extract((tmp/'src/game/Mails/MailHandler.cpp').read_text(), 'bool WorldSession::CheckMailBox')+'\n'
    core_methods+=extract((tmp/'src/game/AuctionHouse/AuctionHouseHandler.cpp').read_text(), 'AuctionHouseEntry const* WorldSession::GetCheckedAuctionHouseForAuctioneer')+'\n'
    core_methods+=extract((core/'src/game/AuctionHouse/AuctionHouseMgr.cpp').read_text(), 'AuctionHouseEntry const* AuctionHouseMgr::GetAuctionHouseEntry')+'\n'
    (tmp/'dispatch-header.h').write_text(dispatch_header)
    cpp=tmp/'tests.cpp';cpp.write_text(prefix+'\n#include "dispatch-header.h"\n'+access+'\n'+core_methods+ah_route+'\n'+mail_route+'\n'+bank_route+'\n'+dispatch+'\n'+tests)
    for bridge in (1,0):
        binary=tmp/('tests'+str(bridge)+('.exe' if os.name=='nt' else ''))
        subprocess.run([args.compiler,'-std=c++17','-Wall','-Wextra','-Werror','-pthread',
                        '-DENABLE_PLAYERBOTS',f'-DCMANGOS_PLAYERBOT_REMOTE_SERVICES={bridge}',
                        '-I',str(repo),'-I',str(repo/'playerbot'),str(cpp),'-o',str(binary)],check=True)
        subprocess.run([str(binary)],check=True)
print('PASS: companion patch applies; normal handlers and source wiring' +
      ('; unchanged transaction/planner bodies against '+args.baseline_ref if args.baseline_ref else ''))
