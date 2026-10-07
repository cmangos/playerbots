"""Run actual travel lifecycle bodies against clock/context doubles; no realm."""
import argparse
import pathlib
import subprocess
import tempfile
from ProfessionProgressionComponentTests import extract

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default='c++')
args = parser.parse_args()
repo = pathlib.Path(__file__).resolve().parents[3]
travel = (repo / 'playerbot/TravelMgr.cpp').read_text()
actions = (repo / 'playerbot/strategy/actions/ChooseTravelTargetAction.cpp').read_text()
header = (repo / 'playerbot/TravelMgr.h').read_text()
assert travel.count('statusTime != 0 && GetTimeLeft() <= 0 && !IsForced()') == 1
prefix = r"""
#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <type_traits>
using uint32=uint32_t; using int32=int32_t;
constexpr uint32 HOUR=3600,TRAVEL_ACTIVITY=1;
struct WorldTimer {inline static uint32 now=0; static uint32 getMSTime(){return now;}};
struct Config {uint32 maxWaitForMove=100;} sPlayerbotAIConfig;
enum class TravelStatus {TRAVEL_STATUS_NONE,TRAVEL_STATUS_PREPARE,TRAVEL_STATUS_EXPIRED,TRAVEL_STATUS_READY,TRAVEL_STATUS_TRAVEL,TRAVEL_STATUS_WORK,TRAVEL_STATUS_COOLDOWN};
struct Context {int clears=0;void ClearValues(const std::string& name){assert(name=="no active travel destinations");++clears;}};
struct PlayerbotAI {Context ctx;bool allowed=true;bool AllowActivity(uint32){return allowed;} Context* GetAiObjectContext(){return &ctx;}void* GetMaster(){return nullptr;}void TellDebug(void*,const char*,const char*){}};
struct Destination {uint32 GetExpireDelay(){return 1000;}uint32 GetCooldownDelay(){return 2000;}};
struct TravelTarget {PlayerbotAI* ai;Destination* tDestination;TravelStatus m_status=TravelStatus::TRAVEL_STATUS_NONE;uint32 startTime=0,statusTime=0;bool forced=false;
void SetStatus(TravelStatus);bool IsActive();bool IsForced()const{return forced;}TravelStatus GetStatus(){return m_status;}uint32 GetMaxTravelTime(){return 1000;}
"""
for name in ('GetTimeLeft', 'GetExpiredTime'):
    prefix += next(line.strip() for line in header.splitlines() if 'int32 '+name+'()' in line)+'\n'
prefix += r"""
};
struct Player {bool battleground=false;bool InBattleGround(){return battleground;}};
struct ChooseTravelTargetAction {PlayerbotAI* ai;Player* bot;TravelTarget* target;bool canMove=true;bool isUseful();
template<class T>T Get(const std::string& name){if constexpr(std::is_same_v<T,bool>){if(name=="can move around")return canMove;assert(name=="travel target active");return target->IsActive();}else{return target;}}};
struct ResetTargetAction:ChooseTravelTargetAction {bool isUseful();};
#define AI_VALUE(T,name) Get<T>(name)
"""
bodies = '\n'.join((extract(travel,'void TravelTarget::SetStatus'), extract(travel,'bool TravelTarget::IsActive'),extract(actions,'bool ChooseTravelTargetAction::isUseful'),extract(actions,'bool ResetTargetAction::isUseful')))
checks = r"""
int main(){
PlayerbotAI ai;Destination dest;TravelTarget target{&ai,&dest};int cases=0;
for(auto status:{TravelStatus::TRAVEL_STATUS_READY,TravelStatus::TRAVEL_STATUS_TRAVEL,TravelStatus::TRAVEL_STATUS_WORK,TravelStatus::TRAVEL_STATUS_COOLDOWN}){
 for(bool forced:{false,true})for(bool unlimited:{false,true})for(int elapsed:{99,100,101}){
  WorldTimer::now=500;target.SetStatus(status);target.statusTime=unlimited?0:100;target.forced=forced;ai.ctx.clears=0;WorldTimer::now+=elapsed;
  bool expired=!forced&&!unlimited&&elapsed>=100;
  assert(target.IsActive()==!expired);assert(ai.ctx.clears==(expired?1:0));assert(target.GetStatus()==(expired?TravelStatus::TRAVEL_STATUS_EXPIRED:status));
  assert(target.IsActive()==!expired);assert(ai.ctx.clears==(expired?1:0));++cases;
 }
}
for(auto status:{TravelStatus::TRAVEL_STATUS_NONE,TravelStatus::TRAVEL_STATUS_PREPARE,TravelStatus::TRAVEL_STATUS_EXPIRED}){target.SetStatus(status);WorldTimer::now+=10000;ai.ctx.clears=0;assert(!target.IsActive());assert(!ai.ctx.clears);++cases;}
WorldTimer::now=0xfffffff0;target.forced=false;target.SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);target.statusTime=100;WorldTimer::now+=100;assert(!target.IsActive());++cases;
Player bot;ResetTargetAction reset;reset.ai=&ai;reset.bot=&bot;reset.target=&target;
assert(reset.isUseful());++cases;
target.SetStatus(TravelStatus::TRAVEL_STATUS_TRAVEL);assert(!reset.isUseful());++cases;
WorldTimer::now+=target.statusTime;assert(reset.isUseful());++cases;
bot.battleground=true;assert(!reset.isUseful());bot.battleground=false;++cases;
ai.allowed=false;assert(!reset.isUseful());ai.allowed=true;++cases;
reset.canMove=false;assert(!reset.isUseful());reset.canMove=true;++cases;
target.SetStatus(TravelStatus::TRAVEL_STATUS_PREPARE);assert(!reset.isUseful());++cases;
std::cout<<"PASS "<<cases<<" travel lifecycle cases\n";
}
"""
with tempfile.TemporaryDirectory(prefix='travel-lifecycle-') as tmp:
    cpp=pathlib.Path(tmp)/'test.cpp'; exe=pathlib.Path(tmp)/'test'
    cpp.write_text(prefix+bodies+checks)
    subprocess.run([args.compiler,'-std=c++17','-Wall','-Wextra','-Werror',str(cpp),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
