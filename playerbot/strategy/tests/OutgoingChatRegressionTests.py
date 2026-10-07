"""Execute the production post-parse chat guard with empty/nonempty text."""
import argparse
import pathlib
import subprocess
import tempfile
parser=argparse.ArgumentParser()
parser.add_argument('--compiler',default='c++')
args=parser.parse_args()
repo=pathlib.Path(__file__).resolve().parents[3]
source=(repo/'playerbot/PlayerbotAI.cpp').read_text()
start=source.index('            // Empty/unsupported chat payloads')
end=source.index('            bool isAiChat',start)
guard=source[start:end]
assert source.rfind('#endif',0,start)>source.index('void PlayerbotAI::HandleBotOutgoingPacket')
assert end<source.index('            if (m_recordIncommingMessages)',end)
assert 'MANGOS_ASSERT(!message.empty())' not in source
cpp='#include <cassert>\n#include <string>\n#include <iostream>\nint recorded=0,replies=0;\nvoid Handle(const std::string& message){\n'+guard+'++recorded;++replies;}\n'
cpp+='int main(){Handle("");assert(recorded==0&&replies==0);for(auto s:{"hello"," ","profession","\\xC3\\xA9"}){Handle(s);}assert(recorded==4&&replies==4);Handle("");assert(recorded==4&&replies==4);std::cout<<"PASS 6 outgoing chat cases\\n";}\n'
with tempfile.TemporaryDirectory(prefix='outgoing-chat-') as tmp:
    path=pathlib.Path(tmp)/'test.cpp';exe=pathlib.Path(tmp)/'test'
    path.write_text(cpp)
    subprocess.run([args.compiler,'-std=c++17','-Wall','-Wextra','-Werror',str(path),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
