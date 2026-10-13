"""Compile the actual profession-readiness body with its real class boundary.

World doubles complement, rather than replace, full core compilation. Optional
--baseline-ref verifies that the earlier unqualified processing call is rejected.
"""
import argparse
import os
import pathlib
import subprocess
import tempfile
from ProfessionProgressionComponentTests import extract

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', default='c++')
parser.add_argument('--baseline-ref')
args = parser.parse_args()
repo = pathlib.Path(__file__).resolve().parents[3]
path = 'playerbot/strategy/values/CraftValues.cpp'
source = subprocess.check_output(['git', 'show', args.baseline_ref + ':' + path], cwd=repo, text=True) if args.baseline_ref else (repo / path).read_text()
body = extract(source, 'bool CanCraftProfessionValue::Calculate()')
prefix = r'''
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include "playerbot/strategy/values/ProfessionProgressionPolicy.h"
namespace profession = ai::profession;
using uint32=uint32_t;using uint8=uint8_t;
struct PlayerbotAI;
struct Item {};
struct GameObject {};
struct ProfessionCraftingPlan {
    uint32 spellId=800001,skillId=800002,processingInputId=0;
    std::map<uint32,uint32> missing;
    bool IsValid()const{return spellId&&skillId;}
    std::map<uint32,uint32> GetMissingReagents(PlayerbotAI*)const{return missing;}
};
struct SpellEntry {uint32 RequiresSpellFocus=0;};
struct Server {SpellEntry spell;bool exists=true;
    const SpellEntry* LookupSpellInfo(uint32){return exists?&spell:nullptr;}
} sServerFacade;
struct Player {bool known=true,casting=false,tools=true;
    bool HasSpell(uint32){return known;}
    bool IsNonMeleeSpellCasted(bool){return casting;}
};
struct Context {ProfessionCraftingPlan plan;uint8 bags=0;
    template<class T>T Get(const std::string&){
        if constexpr(std::is_same_v<T,ProfessionCraftingPlan>)return plan;
        else {static_assert(std::is_same_v<T,uint8>);return bags;}
    }
};
struct PlayerbotAI {bool enabled=true,cooldown=true,pending=false,focus=false,input=true;uint32 inputChecks=0;};
struct ProfessionCraftingPlanValue {
    static bool IsEnabledFor(PlayerbotAI* ai){return ai->enabled;}
    static bool IsCraftCooldownReady(PlayerbotAI* ai){return ai->cooldown;}
    static bool HasPendingCraft(PlayerbotAI* ai){return ai->pending;}
    static GameObject* GetCurrentSpellFocus(PlayerbotAI* ai,const ProfessionCraftingPlan&){static GameObject object;return ai->focus?&object:nullptr;}
    static Item* GetProcessingTarget(PlayerbotAI* ai,const ProfessionCraftingPlan&){static Item item;++ai->inputChecks;return ai->input?&item:nullptr;}
};
struct CanCraftSpellValue {static bool HasRequiredTools(const SpellEntry*,Player* bot){return bot->tools;}};
struct CanCraftProfessionValue {Player* bot;PlayerbotAI* ai;Context* context;bool Calculate();};
#define AI_VALUE(T,name) context->Get<T>(name)
'''
driver = r'''
int main(){
    Player bot;PlayerbotAI ai;Context context;CanCraftProfessionValue value{&bot,&ai,&context};
    uint32 checks=0;auto check=[&](bool expected){++checks;assert(value.Calculate()==expected);};
    check(true);assert(ai.inputChecks==0);
    context.plan.processingInputId=800003;ai.input=false;check(false);ai.input=true;check(true);
    context.plan.missing[800004]=1;check(false);context.plan.missing.clear();
    bot.tools=false;check(false);bot.tools=true;
    context.bags=81;check(false);context.bags=80;check(true);context.bags=0;
    ai.pending=true;check(false);ai.pending=false;
    bot.casting=true;check(false);bot.casting=false;
    bot.known=false;check(false);bot.known=true;
    ai.cooldown=false;check(false);ai.cooldown=true;
    ai.enabled=false;check(false);ai.enabled=true;
    context.plan.skillId=0;check(false);context.plan.skillId=800002;
    sServerFacade.exists=false;check(false);sServerFacade.exists=true;
    sServerFacade.spell.RequiresSpellFocus=800005;check(false);ai.focus=true;check(true);
    context.plan.processingInputId=0;ai.input=false;check(true);
    std::cout<<"PASS: "<<checks<<" actual profession-readiness cases, class-scoped processing helper\n";
}
'''
with tempfile.TemporaryDirectory(prefix='profession-readiness-') as tmp:
    cpp = pathlib.Path(tmp) / 'readiness.cpp'
    binary = pathlib.Path(tmp) / ('readiness.exe' if os.name == 'nt' else 'readiness')
    cpp.write_text(prefix + '\n' + body + '\n' + driver)
    result = subprocess.run([args.compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror', '-I', str(repo), str(cpp), '-o', str(binary)], capture_output=True, text=True)
    if args.baseline_ref:
        assert result.returncode != 0 and 'GetProcessingTarget' in result.stderr, result.stderr
        print('PASS: baseline readiness body rejected the unqualified processing helper')
    else:
        if result.returncode:
            raise RuntimeError(result.stderr or result.stdout)
        subprocess.run([str(binary)], check=True)
