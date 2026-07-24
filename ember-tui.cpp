// ember-tui — EmberDragon's terminal IDE. A Binary-Ninja-class multi-view reverse
// engineering UI in the terminal: a function sidebar plus four switchable views of
// the selected function — Pseudocode (decompiled C/C++), Disassembly, Hex, and a
// binary-wide Strings panel. Truecolor dark theme, syntax coloring, full mouse
// (click functions, click tabs, wheel-scroll, click a call to jump), cross-reference
// strip (callers/callees), in-view search, and jump history. From scratch: raw ANSI
// + termios, no ncurses, no deps. Tuned for Ghostty / any truecolor+SGR-mouse term.
//
//   build:  clang++ -std=c++17 -O2 ember-tui.cpp -o ember-tui
//   use:    ember-tui <binary> [--ai none|api|local]
//   keys:   ↑/↓ j/k  function    1-4 / Tab  switch view    g/G  ends
//           ^U/^D PgUp/PgDn  scroll    / search (n/N)    f  filter funcs
//           [ ]  jump back/fwd    Enter  follow call under cursor    q  quit
//   mouse:  click function · click tab · wheel-scroll · click a call to jump
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <cxxabi.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#include "ember.h"
using std::string; using std::vector;
using namespace nx;

// ── palette (truecolor) ──────────────────────────────────────────────────────
#define BG    "\033[48;2;24;24;27m"     // editor background
#define BG2   "\033[48;2;34;34;38m"     // sidebar / chrome
#define BG3   "\033[48;2;44;44;50m"     // hover / tab inactive
#define SELB  "\033[48;2;58;42;14m"     // selected row (ember-warm)
#define MATCH "\033[48;2;74;60;20m"     // search-match line
#define FG    "\033[38;2;212;212;212m"
#define DIM   "\033[38;2;120;120;132m"
#define DIM2  "\033[38;2;78;78;88m"
#define KW    "\033[38;2;86;156;214m"   // keywords (blue)
#define TY    "\033[38;2;78;201;176m"   // types (teal)
#define CMT   "\033[38;2;106;153;85m"   // comments (green)
#define STRC  "\033[38;2;206;145;120m"  // strings (orange)
#define FNC   "\033[38;2;220;220;170m"  // function names (yellow)
#define NUM   "\033[38;2;181;206;168m"  // numbers
#define REG   "\033[38;2;156;220;254m"  // asm registers (light blue)
#define ACC   "\033[38;2;255;138;42m"   // ember accent
#define ACC2  "\033[38;2;255;92;205m"   // ember pink
#define RST   "\033[0m"
#define B     "\033[1m"

enum View { V_PSEUDO=0, V_DISASM=1, V_GRAPH=2, V_HEX=3, V_STR=4, V_LOG=5, V_INFO=6, V_COUNT=7 };
static const char* VIEW_NAME[V_COUNT] = {"Pseudocode","Disassembly","Graph","Hex","Strings","Console","Info"};

struct Func {
    string name; uint64_t addr=0;
    vector<string> code, dis, graph;       // graph = lazily-built CFG text
    bool isClass=false, isStruct=false, isGlobals=false;
    vector<int> callees, callers;          // function indices
    int cScroll=0, dScroll=0, gScroll=0;   // per-func remembered scroll
    string comment;                        // user annotation
};
struct Str { uint64_t va; size_t off; string text; };

// ── globals ──────────────────────────────────────────────────────────────────
static vector<Func> g_fn;
static vector<uint8_t> g_bytes;            // raw binary
static vector<Section> g_sec;
static vector<Str> g_str;
static string g_bin, g_arch="?", g_mode="none";
static string g_tmp, g_dir, g_name, g_srcpath;   // engine dir · binary base name · recompile source
static bool g_cpp=false, g_analyzed=false;       // C++ output? · has Analyze run?

static int sel=0, view=V_PSEUDO;
static int hexScroll=0, strScroll=0;
static string filter, search; int searchIdx=0;
static vector<int> g_hist;                 // jump history (func indices)
static int g_histPos=-1;
static string g_status;
// hex editing
static bool hexEdit=false; static size_t hexCur=0; static int hexNib=0; static bool dirty=false;
// console / log
static vector<string> g_log; static int logScroll=1<<29;
static void logMsg(const string& s){ g_log.push_back(s); logScroll=1<<29; }
// info panel (lazy-loaded from ember-info)
static vector<string> g_info; static int infoScroll=0;

// geometry (set by draw, read by input)
static int TW=80, TH=24, sideW=26, contentX=28, topRow=3, rows=18, gutter=6, listScroll=0;
static struct { int x0,x1,v; } g_tabhit[V_COUNT];
static vector<int> g_visIdx;               // filtered function indices, in list order

// ── term ─────────────────────────────────────────────────────────────────────
static struct termios g_orig;
static void getsz(){ struct winsize ws; if(ioctl(0,TIOCGWINSZ,&ws)==0&&ws.ws_col){ TW=ws.ws_col; TH=ws.ws_row; } }
static void raw(){ tcgetattr(0,&g_orig); struct termios r=g_orig; r.c_lflag&=~(ICANON|ECHO); r.c_cc[VMIN]=1; r.c_cc[VTIME]=0; tcsetattr(0,TCSANOW,&r);
    printf("\033[?1049h\033[?25l\033[?1000h\033[?1006h"); }      // alt-screen, hide cursor, SGR mouse
static void restore(){ printf("\033[?1000l\033[?1006l\033[?25h\033[?1049l\033[0m"); tcsetattr(0,TCSANOW,&g_orig); fflush(stdout);
    if(!g_tmp.empty()){ string q="rm -rf '"; for(char ch:g_tmp){ if(ch=='\'') q+="'\\''"; else q+=ch; } q+="'"; system(q.c_str()); } }
static void at(int r,int c){ printf("\033[%d;%dH", r,c); }

// ── tokens / highlighting ────────────────────────────────────────────────────
static bool isKw(const string& w){ static const char* K[]={"int","long","char","short","void","return","if","else","while","for","do","struct","class","public","private","const","unsigned","signed","goto","break","continue","this","sizeof","switch","case","default","static","enum","typedef","union","float","double","bool","true","false","NULL","nullptr",0}; for(int i=0;K[i];i++) if(w==K[i]) return true; return false; }
static bool isTy(const string& w){ return w=="int"||w=="long"||w=="char"||w=="short"||w=="void"||w=="unsigned"||w=="signed"||w=="float"||w=="double"||w=="bool"||w.find("int64")!=string::npos||w.find("int32")!=string::npos||w.find("uint")!=string::npos||(!w.empty()&&(isupper((unsigned char)w[0])||w.rfind("s_",0)==0||w.rfind("S_",0)==0)); }
static bool knownFn(const string& w){ for(auto&f:g_fn) if(f.name==w) return true; return false; }

// syntax-highlight a C line into ANSI (caller has set the row bg already)
static string hlC(const string& l,const char* bg){
    string o; size_t i=0; size_t c=l.find("//"); size_t cm=l.find("/*");
    size_t cstart=(c==string::npos?cm:(cm==string::npos?c:(c<cm?c:cm)));
    while(i<l.size()){
        if(i==cstart){ o+=CMT; o+=l.substr(i); o+=RST; o+=bg; break; }
        char ch=l[i];
        if(ch=='"'){ size_t j=i+1; while(j<l.size()&&l[j]!='"'){ if(l[j]=='\\')j++; j++; } o+=STRC; o+=l.substr(i,j-i+1); o+=RST; o+=bg; i=j+1; continue; }
        if(isalpha((unsigned char)ch)||ch=='_'){ size_t j=i; while(j<l.size()&&(isalnum((unsigned char)l[j])||l[j]=='_'))j++; string w=l.substr(i,j-i);
            bool callf = j<l.size()&&l[j]=='(';
            const char* col = isKw(w)?KW : (knownFn(w)?ACC : callf?FNC : isTy(w)?TY : FG);
            o+=col; o+=w; o+=RST; o+=bg; i=j; continue; }
        if(isdigit((unsigned char)ch)){ size_t j=i; while(j<l.size()&&(isalnum((unsigned char)l[j])))j++; o+=NUM; o+=l.substr(i,j-i); o+=RST; o+=bg; i=j; continue; }
        o+=FG; o+=ch; i++;
    }
    return o;
}
// highlight a disassembly line "ADDR:  mnem  ops"
static string hlAsm(const string& l,const char* bg){
    string o; size_t colon=l.find(':');
    size_t i=0;
    if(colon!=string::npos && colon>=8){ o+=DIM2; o+=l.substr(0,colon+1); o+=RST; o+=bg; i=colon+1; }
    bool firstWord=true;
    while(i<l.size()){
        char ch=l[i];
        if(ch==';'){ o+=CMT; o+=l.substr(i); o+=RST; o+=bg; break; }
        if(isalpha((unsigned char)ch)||ch=='_'||ch=='.'){ size_t j=i; while(j<l.size()&&(isalnum((unsigned char)l[j])||l[j]=='_'||l[j]=='.'))j++; string w=l.substr(i,j-i);
            const char* col=FG;
            if(firstWord){ col=KW; firstWord=false; }                                   // mnemonic
            else if((w.size()>=2&&(w[0]=='x'||w[0]=='w'||w[0]=='v'||w[0]=='q')&&isdigit((unsigned char)w[1]))||w=="sp"||w=="lr"||w=="wzr"||w=="xzr"||w=="pc"||w=="fp") col=REG;
            else if(knownFn(w)) col=ACC;
            o+=col; o+=w; o+=RST; o+=bg; i=j; continue; }
        if(ch=='#'||ch=='0'){ if(ch=='0'&&i+1<l.size()&&l[i+1]=='x'){ size_t j=i+2; while(j<l.size()&&isxdigit((unsigned char)l[j]))j++; o+=NUM; o+=l.substr(i,j-i); o+=RST; o+=bg; i=j; continue; }
            if(ch=='#'){ size_t j=i+1; while(j<l.size()&&(isalnum((unsigned char)l[j])||l[j]=='x'||l[j]=='-'))j++; o+=NUM; o+=l.substr(i,j-i); o+=RST; o+=bg; i=j; continue; } }
        if(isdigit((unsigned char)ch)){ size_t j=i; while(j<l.size()&&(isalnum((unsigned char)l[j])))j++; o+=NUM; o+=l.substr(i,j-i); o+=RST; o+=bg; i=j; continue; }
        o+=FG; o+=ch; i++;
    }
    return o;
}

// ── loading ──────────────────────────────────────────────────────────────────
static string readFile(const string& p){ FILE* f=fopen(p.c_str(),"rb"); if(!f) return ""; string s; char b[65536]; size_t n; while((n=fread(b,1,sizeof b,f))>0) s.append(b,n); fclose(f); return s; }
static string baseName(const string& p){ size_t s=p.find_last_of('/'); return s==string::npos?p:p.substr(s+1); }
// file offset where the selected function's bytes begin (vaddr -> fileoff via sections)
static size_t hexBase(){ Func& f=g_fn[sel]; if(f.addr) for(auto& s:g_sec) if(f.addr>=s.vaddr&&f.addr<s.vaddr+s.size) return s.fileoff+(f.addr-s.vaddr); return 0; }
// write edited bytes back to the binary, preserving a one-time .bak of the original
static void saveBytes(){ string bak=g_bin+".bak"; struct stat st;
    if(stat(bak.c_str(),&st)!=0){ FILE* o=fopen(g_bin.c_str(),"rb"); if(o){ FILE* b=fopen(bak.c_str(),"wb"); if(b){ char buf[65536]; size_t n; while((n=fread(buf,1,sizeof buf,o))>0) fwrite(buf,1,n,b); fclose(b);} fclose(o);} }
    FILE* f=fopen(g_bin.c_str(),"wb"); if(!f){ g_status="write FAILED (permissions?)"; return; }
    fwrite(g_bytes.data(),1,g_bytes.size(),f); fclose(f); dirty=false;
    g_status="wrote "+std::to_string(g_bytes.size())+" bytes to "+baseName(g_bin)+"  (.bak kept)"; }
static vector<string> splitLines(const string& t){ vector<string> v; size_t i=0; while(i<=t.size()){ size_t e=t.find('\n',i); string ln=t.substr(i,(e==string::npos?t.size():e)-i); if(!(e==string::npos&&ln.empty())) v.push_back(ln); if(e==string::npos)break; i=e+1; } return v; }

static string nameOf(const string& l){ string t=l; size_t a=t.find_first_not_of(" \t"); if(a!=string::npos)t=t.substr(a);
    if(t.rfind("class ",0)==0||t.rfind("struct ",0)==0){ size_t s=t.find(' ')+1, e=t.find_first_of(" {:",s); return t.substr(s, e==string::npos?string::npos:e-s); }
    size_t par=t.find('('); if(par!=string::npos){ size_t s=t.find_last_of(" *&",par); return t.substr(s==string::npos?0:s+1, par-(s==string::npos?0:s+1)); }
    return t; }

// parse decompiled C into top-level function blocks (returns them, globals first)
static vector<Func> parseCText(const string& text){
    vector<Func> out; vector<string> lines=splitLines(text);
    Func globals; globals.name="· data / globals"; globals.isGlobals=true;
    int depth=0; Func cur; bool inBlk=false;
    for(auto& l:lines){ int op=0,cl=0; for(char c:l){ if(c=='{')op++; else if(c=='}')cl++; }
        if(!inBlk && depth==0 && op>cl && l.find('{')!=string::npos){ cur=Func(); cur.name=nameOf(l); string tl=l; size_t a=tl.find_first_not_of(" \t");
            cur.isClass=(a!=string::npos&&tl.compare(a,6,"class ")==0); cur.isStruct=(a!=string::npos&&tl.compare(a,7,"struct ")==0);
            cur.code.push_back(l); inBlk=true; depth+=op-cl; continue; }
        if(inBlk){ cur.code.push_back(l); depth+=op-cl; if(depth<=0){ out.push_back(cur); inBlk=false; depth=0; } continue; }
        if(l.find_first_not_of(" \t")!=string::npos) globals.code.push_back(l);
    }
    if(!globals.code.empty()) out.insert(out.begin(), globals);
    return out;
}
static void parseC(const string& text){ g_fn=parseCText(text); }
// attach disassembly blocks (asm has `label:` then `ADDR:  insn` lines)
static void parseAsm(const string& text){
    std::map<string, std::pair<uint64_t,vector<string>>> m;
    vector<string> lines=splitLines(text); string curName; uint64_t curAddr=0; vector<string> curLines;
    auto flush=[&](){ if(!curName.empty()){ m[curName]={curAddr,curLines}; } curName.clear(); curLines.clear(); curAddr=0; };
    for(auto& l:lines){ if(l.empty())continue;
        // label line: identifier ending in ':' with no spaces
        size_t cp=l.find(':');
        if(cp!=string::npos && cp+1==l.size() && l.find(' ')==string::npos && (isalpha((unsigned char)l[0])||l[0]=='_')){ flush(); curName=l.substr(0,cp); continue; }
        if(!curName.empty()){ curLines.push_back(l);
            if(curAddr==0){ size_t c=l.find(':'); if(c!=string::npos){ curAddr=strtoull(l.substr(0,c).c_str(),0,16); } } }
    }
    flush();
    for(auto& f:g_fn){ auto it=m.find(f.name); if(it!=m.end()){ f.addr=it->second.first; f.dis=it->second.second; } }
}
// build call graph from disassembly bl/b targets
static void buildXrefs(){
    vector<std::pair<uint64_t,int>> starts; for(int i=0;i<(int)g_fn.size();i++) if(g_fn[i].addr) starts.push_back({g_fn[i].addr,i});
    std::sort(starts.begin(),starts.end());
    auto resolve=[&](uint64_t a)->int{ // function whose start == a (exact), else containing range
        for(auto&s:starts) if(s.first==a) return s.second;
        int hit=-1; for(size_t k=0;k<starts.size();k++){ if(starts[k].first<=a && (k+1==starts.size()||a<starts[k+1].first)){ hit=starts[k].second; } } return hit==-1?-1:-2; };
    std::map<string,int> byName; for(int i=0;i<(int)g_fn.size();i++) if(!g_fn[i].isGlobals) byName[g_fn[i].name]=i;
    for(int i=0;i<(int)g_fn.size();i++){ for(auto& l:g_fn[i].dis){
        size_t cp=l.find(':'); if(cp==string::npos)continue; string rest=l.substr(cp+1);
        size_t a=rest.find_first_not_of(" \t"); if(a==string::npos)continue;
        size_t e=rest.find_first_of(" \t",a); string mn=rest.substr(a,e==string::npos?string::npos:e-a);
        if(mn!="bl"&&mn!="b") continue;
        // operand: first token after the mnemonic — either 0x<addr> or a symbol name
        size_t o=rest.find_first_not_of(" \t",e==string::npos?rest.size():e); if(o==string::npos)continue;
        size_t oe=rest.find_first_of(" \t,",o); string op=rest.substr(o,oe==string::npos?string::npos:oe-o);
        int j=-1;
        if(op.rfind("0x",0)==0) j=resolve(strtoull(op.c_str(),0,16));
        else { auto it=byName.find(op); if(it!=byName.end()) j=it->second; }
        if(j<0||j==i) continue;
        if(std::find(g_fn[i].callees.begin(),g_fn[i].callees.end(),j)==g_fn[i].callees.end()) g_fn[i].callees.push_back(j);
        if(std::find(g_fn[j].callers.begin(),g_fn[j].callers.end(),i)==g_fn[j].callers.end()) g_fn[j].callers.push_back(i);
    } }
}
static void scanStrings(){
    size_t i=0,n=g_bytes.size();
    auto va=[&](size_t off)->uint64_t{ for(auto&s:g_sec) if(off>=s.fileoff&&off<s.fileoff+s.size) return s.vaddr+(off-s.fileoff); return 0; };
    while(i<n){ if(g_bytes[i]>=0x20&&g_bytes[i]<0x7f){ size_t j=i; string s; while(j<n&&g_bytes[j]>=0x20&&g_bytes[j]<0x7f){ s+=(char)g_bytes[j]; j++; }
            if(s.size()>=4){ g_str.push_back({va(i),i,s}); if(g_str.size()>=200000)break; } i=j; } else i++; }
}

// ── CFG (basic-block) recovery + terminal box rendering ──────────────────────
static void buildGraph(Func& f){ if(!f.graph.empty()||f.dis.empty())return;
    struct In { uint64_t a; string mn, ops, raw; };
    vector<In> ins; uint64_t lo=~0ull, hi=0;
    for(auto& l:f.dis){ size_t cp=l.find(':'); if(cp==string::npos)continue; uint64_t a=strtoull(l.substr(0,cp).c_str(),0,16);
        string rest=l.substr(cp+1); size_t s=rest.find_first_not_of(" \t"); if(s==string::npos)continue;
        size_t e=rest.find_first_of(" \t",s); string mn=rest.substr(s,e==string::npos?string::npos:e-s);
        string ops = e==string::npos?"":rest.substr(rest.find_first_not_of(" \t",e)==string::npos?e:rest.find_first_not_of(" \t",e));
        ins.push_back({a,mn,ops,l}); lo=std::min(lo,a); hi=std::max(hi,a); }
    if(ins.empty())return;
    auto isCond=[](const string& m){ return m.rfind("b.",0)==0||m=="cbz"||m=="cbnz"||m=="tbz"||m=="tbnz"; };
    auto isUncond=[](const string& m){ return m=="b"; };
    auto isTerm=[](const string& m){ return m=="ret"||m=="br"||m=="brk"; };
    auto target=[&](const In& I)->uint64_t{ size_t h=I.ops.rfind("0x"); if(h==string::npos)return 0; return strtoull(I.ops.c_str()+h,0,16); };
    // leaders
    std::map<uint64_t,bool> leader; leader[ins[0].a]=true;
    for(size_t i=0;i<ins.size();i++){ const In& I=ins[i]; if(isCond(I.mn)||isUncond(I.mn)){ uint64_t t=target(I); if(t>=lo&&t<=hi)leader[t]=true; if(i+1<ins.size())leader[ins[i+1].a]=true; } else if(isTerm(I.mn)&&i+1<ins.size()) leader[ins[i+1].a]=true; }
    // emit boxes as TAGGED PLAIN lines: \x01 header · \x02 instruction · \x03 footer · "" blank
    int bn=0;
    for(size_t i=0;i<ins.size();){ uint64_t bstart=ins[i].a; size_t j=i;
        char hdr[80]; snprintf(hdr,sizeof hdr,"\x01""bb%d  0x%llx",bn,(unsigned long long)bstart);
        f.graph.push_back(hdr);
        vector<uint64_t> succ; bool ended=false;
        for(; j<ins.size(); j++){ if(j>i && leader.count(ins[j].a)){ break; }
            const In& I=ins[j]; string body=I.raw.substr(I.raw.find(':')+1); size_t b0=body.find_first_not_of(" \t");
            f.graph.push_back(string("\x02")+(b0==string::npos?body:body.substr(b0)));
            if(isCond(I.mn)){ uint64_t t=target(I); uint64_t fall=(j+1<ins.size())?ins[j+1].a:0; succ={t,fall}; j++; ended=true; break; }
            if(isUncond(I.mn)){ succ={target(I)}; j++; ended=true; break; }
            if(isTerm(I.mn)){ succ={}; j++; ended=true; break; } }
        if(!ended && j<ins.size()) succ={ins[j].a};
        char foot[160];
        if(succ.size()==2) snprintf(foot,sizeof foot,"\x03""true → 0x%llx    false ↓ 0x%llx",(unsigned long long)succ[0],(unsigned long long)succ[1]);
        else if(succ.size()==1) snprintf(foot,sizeof foot,"\x03""→ 0x%llx",(unsigned long long)succ[0]);
        else snprintf(foot,sizeof foot,"\x03""return");
        f.graph.push_back(foot); f.graph.push_back("");
        i=j; bn++; }
}

static string shq(const string& s);   // fwd (defined with the actions)
// ── drawing ──────────────────────────────────────────────────────────────────
static void padBG(int w,const char* bg){ string s; for(int i=0;i<w;i++) s+=' '; printf("%s%s%s",bg,s.c_str(),RST); }
static string clip(const string& s,int w){ if((int)s.size()<=w) return s; if(w<=1) return s.substr(0,w>0?w:0); return s.substr(0,w-1)+"…"; }

static void draw(){
    getsz();
    sideW = TW/4<22?22:(TW/4>38?38:TW/4); contentX=sideW+2; topRow=3; rows=TH-topRow-2; if(rows<1)rows=1;
    // filtered function index list
    g_visIdx.clear(); for(int i=0;i<(int)g_fn.size();i++){ if(filter.empty()){ g_visIdx.push_back(i); continue; }
        string nm=g_fn[i].name; string fl=filter; for(char&c:nm)c=tolower(c); for(char&c:fl)c=tolower(c); if(nm.find(fl)!=string::npos) g_visIdx.push_back(i); }
    if(g_visIdx.empty()) g_visIdx.push_back(0);
    if(sel>=(int)g_fn.size())sel=g_fn.size()-1; if(sel<0)sel=0;
    // ensure sel is in visible list; find its list position
    int selPos=-1; for(int p=0;p<(int)g_visIdx.size();p++) if(g_visIdx[p]==sel){ selPos=p; break; }
    if(selPos<0){ sel=g_visIdx[0]; selPos=0; }

    printf("%s\033[2J",BG);
    // ── header (row 1) ──
    at(1,1); printf("%s%s%s 🔥🐉 EmberDragon %s%s%s  %s%s  %s%s[%s]%s  %s%sfuncs %d%s  %s%sai %s%s",
        BG2,ACC,B, RST,BG2,FG, clip(g_bin,28).c_str(),RST, BG2,TY,g_arch.c_str(),RST, BG2,DIM,(int)g_fn.size(),RST, BG2,DIM,g_mode.c_str(),RST);
    { int used=18+30+6+12+8; if(used<TW){ at(1,used); padBG(TW-used,BG2);} }
    // ── tab bar (row 2) ──
    at(2,1); printf("%s",BG2); int col=2; printf(" ");
    for(int v=0; v<V_COUNT; v++){ bool act=(v==view); char lbl[40]; snprintf(lbl,sizeof lbl," %d %s ",v+1,VIEW_NAME[v]);
        g_tabhit[v].x0=col; g_tabhit[v].v=v;
        if(act) printf("%s%s%s%s%s",ACC, "\033[48;2;58;42;14m",B,lbl,RST);
        else    printf("%s%s%s%s",BG3,DIM,lbl,RST);
        col+=(int)strlen(lbl); g_tabhit[v].x1=col; printf("%s ",BG2); col++; }
    if(col<TW) padBG(TW-col+1,BG2);

    Func& f=g_fn[sel];
    // ── sidebar (function list) ──
    if(selPos<listScroll) listScroll=selPos; if(selPos>=listScroll+rows) listScroll=selPos-rows+1;
    if(listScroll>(int)g_visIdx.size()-rows) listScroll=(int)g_visIdx.size()-rows; if(listScroll<0)listScroll=0;
    for(int r=0;r<rows;r++){ int p=listScroll+r; at(topRow+r,1);
        if(p<(int)g_visIdx.size()){ Func& b=g_fn[g_visIdx[p]]; bool s=(g_visIdx[p]==sel);
            const char* ic = b.isGlobals?"·":b.isClass?"◆":b.isStruct?"▢":"ƒ";
            const char* bg = s?SELB:BG2;
            char addr[20]=""; if(b.addr) snprintf(addr,sizeof addr,"%llx",(unsigned long long)(b.addr&0xfffff));
            int nameW = sideW-3-(int)strlen(addr)-(addr[0]?1:0); if(nameW<3)nameW=3;
            string nm=clip(b.name,nameW);
            printf("%s%s %s ", bg, s?ACC:DIM, ic);
            printf("%s%-*s", s?FG B:FG, nameW, nm.c_str());
            if(addr[0]) printf("%s%s %s",bg,DIM2,addr);
            printf("%s",RST);
            // pad
            int drawn=1+1+1+1+nameW+(addr[0]?1+(int)strlen(addr):0); if(drawn<sideW){ printf("%s",bg); padBG(sideW-drawn,bg);}
        } else { padBG(sideW,BG2); }
        // divider
        printf("%s%s│%s",BG,DIM2,RST);
    }

    // ── content pane ──
    int cw=TW-contentX+1; if(cw<10)cw=10;
    gutter=6;
    auto drawScrollableLines=[&](const vector<string>& lines,int& scroll,bool asmv){
        int n=(int)lines.size(); if(scroll>n-1)scroll=n>0?n-1:0; if(scroll<0)scroll=0;
        // search target line for highlight
        for(int r=0;r<rows;r++){ int li=scroll+r; at(topRow+r,contentX);
            if(li<n){ const string& raw=lines[li];
                bool mt = !search.empty() && [&]{ string a=raw,b=search; for(char&c:a)c=tolower(c); for(char&c:b)c=tolower(c); return a.find(b)!=string::npos; }();
                const char* bg = mt?MATCH:BG;
                printf("%s%s%4d %s",bg,mt?ACC:DIM2,li+1,RST);
                string body=clip(raw,cw-gutter);
                printf("%s%s",bg, asmv?hlAsm(body,bg).c_str():hlC(body,bg).c_str());
                // pad to pane end
                int pad=cw-gutter-(int)body.size(); if(pad>0){ printf("%s",bg); padBG(pad,bg);} printf("%s",RST);
            } else { at(topRow+r,contentX); padBG(cw,BG); }
        }
    };
    if(view==V_PSEUDO)       drawScrollableLines(f.code,f.cScroll,false);
    else if(view==V_DISASM){ if(f.dis.empty()){ at(topRow,contentX); printf("%s%s  (no disassembly — library/imported)%s",BG,DIM,RST); for(int r=1;r<rows;r++){at(topRow+r,contentX);padBG(cw,BG);} }
                             else drawScrollableLines(f.dis,f.dScroll,true); }
    else if(view==V_GRAPH){ buildGraph(f);
        if(f.graph.empty()){ at(topRow,contentX); printf("%s%s  (no CFG — needs disassembly)%s",BG,DIM,RST); for(int r=1;r<rows;r++){at(topRow+r,contentX);padBG(cw,BG);} }
        else { int n=(int)f.graph.size(); if(f.gScroll>n-1)f.gScroll=n>0?n-1:0; if(f.gScroll<0)f.gScroll=0;
            for(int r=0;r<rows;r++){ at(topRow+r,contentX); int li=f.gScroll+r;
                if(li<n){ const string& g=f.graph[li]; printf("%s ",BG);
                    if(g.empty()) padBG(cw-1,BG);
                    else if(g[0]=='\x01'){ printf("%s┌─ %s%s ",ACC,g.c_str()+1,DIM2); int u=4+(int)strlen(g.c_str()+1)+1; for(int x=u;x<cw-1;x++)printf("─"); printf("%s",RST); }
                    else if(g[0]=='\x02'){ string body=clip(g.c_str()+1,cw-4); printf("%s│ %s",DIM2,hlAsm(body,BG).c_str()); int pad=cw-3-(int)body.size(); if(pad>0){printf("%s",BG);padBG(pad,BG);} printf("%s",RST); }
                    else if(g[0]=='\x03'){ string body=clip(g.c_str()+1,cw-4); printf("%s└─▶ %s",TY,body.c_str()); int pad=cw-4-(int)body.size(); if(pad>0){printf("%s",BG);padBG(pad,BG);} printf("%s",RST); }
                    else { printf("%s%s",FG,clip(g,cw-1).c_str()); }
                    printf("%s",RST);
                } else padBG(cw,BG);
            }
        }
    }
    else if(view==V_HEX){
        // hex dump starting at this function's file offset (vaddr->off via sections)
        size_t base=hexBase(); uint64_t vbase=f.addr?f.addr:0;
        static const char* HEXD="0123456789abcdef";
        if(hexEdit){ if(hexCur<base)hexCur=base; long row=((long)hexCur-(long)base)/16;
            if(row<hexScroll)hexScroll=row; if(row>=hexScroll+rows)hexScroll=row-rows+1; if(hexScroll<0)hexScroll=0; }
        size_t start=base + (size_t)hexScroll*16;
        for(int r=0;r<rows;r++){ at(topRow+r,contentX); size_t off=start+(size_t)r*16;
            if(off<g_bytes.size()){
                string asciis; printf("%s%s%012llx  %s",BG,DIM2,(unsigned long long)(vbase+(off-base)),RST);
                for(int k=0;k<16;k++){ if(off+k<g_bytes.size()){ unsigned char c=g_bytes[off+k];
                        bool cur = hexEdit && (off+k)==hexCur;
                        if(cur){ printf("%s",BG);
                            printf("%s%c%s",(hexNib==0?"\033[7m" ACC:ACC),HEXD[c>>4],RST BG);
                            printf("%s%c%s ",(hexNib==1?"\033[7m" ACC:ACC),HEXD[c&15],RST BG); }
                        else printf("%s%s%02x %s",BG,(c==0?DIM2:NUM),c,RST);
                        asciis+=(c>=0x20&&c<0x7f)?(char)c:'.'; }
                    else { printf("%s   ",BG); asciis+=' '; }
                    if(k==7) printf("%s ",BG); }
                printf("%s%s %s%s",BG,DIM,asciis.c_str(),RST);
                int drawn=14+16*3+1+1+16; if(drawn<cw)padBG(cw-drawn,BG); printf("%s",RST);
            } else padBG(cw,BG);
        }
    } else if(view==V_STR){
        int n=(int)g_str.size(); if(strScroll>n-1)strScroll=n>0?n-1:0; if(strScroll<0)strScroll=0;
        for(int r=0;r<rows;r++){ at(topRow+r,contentX); int li=strScroll+r;
            if(li<n){ Str& s=g_str[li]; bool mt=!search.empty()&&[&]{string a=s.text,b=search;for(char&c:a)c=tolower(c);for(char&c:b)c=tolower(c);return a.find(b)!=string::npos;}();
                const char* bg=mt?MATCH:BG; if(s.va) printf("%s%s%012llx  ",bg,DIM2,(unsigned long long)s.va); else printf("%s%s+%010llx  ",bg,DIM2,(unsigned long long)s.off);
                string t=clip(s.text,cw-15); printf("%s%s",STRC,t.c_str()); printf("%s",RST);
                int drawn=14+(int)t.size(); if(drawn<cw){printf("%s",bg);padBG(cw-drawn,bg);} printf("%s",RST);
            } else padBG(cw,BG);
        }
    } else if(view==V_LOG){ // Console
        int n=(int)g_log.size(); int maxs=n-rows; if(maxs<0)maxs=0; if(logScroll>maxs)logScroll=maxs; if(logScroll<0)logScroll=0;
        for(int r=0;r<rows;r++){ at(topRow+r,contentX); int li=logScroll+r;
            if(li<n){ const string& s=g_log[li]; const char* col=FG;
                if(s.rfind("$ ",0)==0||s.rfind("EmberRun",0)==0||s.rfind("recompile",0)==0||s.rfind("patch",0)==0) col=ACC;
                else if(s.find("FAIL")!=string::npos||s.find("error")!=string::npos||s.find("denied")!=string::npos) col=ACC2;
                else if(s.rfind("  ",0)==0) col=DIM;
                string t=clip(s,cw-2); printf("%s%s %s",BG,col,t.c_str()); int u=1+(int)t.size(); if(u<cw){printf("%s",BG);padBG(cw-u,BG);} printf("%s",RST);
            } else padBG(cw,BG);
        }
    } else { // V_INFO — static binary intel from ember-info
        if(g_info.empty()){ string self=g_dir+"ember-info"; struct stat st; string tool=(stat(self.c_str(),&st)==0)?self:"ember-info";
            FILE* p=popen((shq(tool)+" "+shq(g_bin)+" 2>/dev/null").c_str(),"r");
            if(p){ char b[65536]; string acc; size_t n; while((n=fread(b,1,sizeof b,p))>0)acc.append(b,n); pclose(p);
                for(size_t i=0;i<acc.size();){ size_t e=acc.find('\n',i); g_info.push_back(acc.substr(i,(e==string::npos?acc.size():e)-i)); if(e==string::npos)break; i=e+1; } }
            if(g_info.empty()) g_info.push_back("(ember-info unavailable)"); }
        int n=(int)g_info.size(); if(infoScroll>n-1)infoScroll=n>0?n-1:0; if(infoScroll<0)infoScroll=0;
        for(int r=0;r<rows;r++){ at(topRow+r,contentX); int li=infoScroll+r;
            if(li<n){ const string& s=g_info[li]; const char* col=FG;
                if(s.find("SECTIONS")!=string::npos||s.find("IMPORTS")!=string::npos||s.find("SYMBOLS")!=string::npos||s.find("LIBRARIES")!=string::npos||s.find("ember-info")!=string::npos) col=ACC;
                else if(s.find("⚠")!=string::npos||s.find("packed")!=string::npos) col=ACC2;
                else if(s.find("SEGMENT")!=string::npos||s.rfind("──",0)==0) col=DIM2;
                string t=clip(s,cw-2); printf("%s%s %s",BG,col,t.c_str()); int u=1+(int)t.size(); if(u<cw){printf("%s",BG);padBG(cw-u,BG);} printf("%s",RST);
            } else padBG(cw,BG);
        }
    }

    // ── xref strip (row TH-1) — shows the function's comment (if any) then callers/callees ──
    at(TH-1,1); printf("%s",BG2);
    if(!f.comment.empty()){ string cm=" // "+f.comment; printf("%s%s",CMT,clip(cm,TW-1).c_str()); int u=(int)clip(cm,TW-1).size(); if(u<TW)padBG(TW-u,BG2); printf("%s",RST); }
    else { string xr;
        if(!f.callees.empty()){ xr+=" →calls "; int c=0; for(int j:f.callees){ if(c++>=6){xr+="…";break;} xr+=g_fn[j].name; xr+=" "; } }
        if(!f.callers.empty()){ xr+=" ←from "; int c=0; for(int j:f.callers){ if(c++>=6){xr+="…";break;} xr+=g_fn[j].name; xr+=" "; } }
        if(xr.empty()) xr=" (no cross-references)";
        printf("%s%s",DIM,clip(xr,TW-1).c_str()); { int u=(int)clip(xr,TW-1).size(); if(u<TW)padBG(TW-u,BG2);} printf("%s",RST); }

    // ── footer (row TH) ──
    at(TH,1); printf("%s",BG2);
    if(!g_status.empty()){ printf("%s %s",ACC,clip(g_status,TW-2).c_str()); int u=(int)clip(g_status,TW-2).size()+1; if(u<TW)padBG(TW-u,BG2); }
    else { string hint = hexEdit ? " HEX EDIT · 0-9 a-f type · hjkl/arrows move · w write · i exit"
                        : (view==V_HEX ? " i edit bytes · w write · r run · p patch · ` term · d lldb · 1-7 view · q quit"
                        : (view==V_LOG ? " a analyze · r run · R recompile · p patch · ` terminal · d lldb · 1-7 view · m rename · q quit"
                                       : " ↑↓ func · 1-7 view · m rename · a analyze · r run · p patch · ` term · d lldb · / search · q quit"));
        printf("%s%s",hexEdit?ACC:DIM,clip(hint,TW-20).c_str());
        string right; if(dirty) right="●unsaved "; if(!search.empty()) right+="/"+search+" "; if(!filter.empty()) right+="filter:"+filter+" ";
        int hl2=(int)clip(hint,TW-20).size(); int rp=TW-hl2-(int)right.size(); if(rp>0)padBG(rp,BG2); printf("%s%s",dirty?ACC:ACC2,right.c_str()); }
    printf("%s",RST);
    fflush(stdout);
}

// ── navigation ───────────────────────────────────────────────────────────────
static void jumpTo(int idx,bool record){ if(idx<0||idx>=(int)g_fn.size())return;
    if(record){ if(g_histPos<(int)g_hist.size()-1) g_hist.resize(g_histPos+1); g_hist.push_back(sel); g_histPos=(int)g_hist.size()-1; }
    sel=idx; filter.clear(); }
static void histBack(){ if(g_histPos>=0&&g_histPos<(int)g_hist.size()){ int t=g_hist[g_histPos]; g_hist[g_histPos]=sel; g_histPos--; sel=t; } }
static void histFwd(){ if(g_histPos+1<(int)g_hist.size()){ g_histPos++; int t=g_hist[g_histPos]; g_hist[g_histPos]=sel; sel=t; } }

// the active scrollable view's lines + scroll ref (for search / scroll)
static vector<string>* activeLines(int** scrollp){ Func& f=g_fn[sel];
    if(view==V_PSEUDO){ *scrollp=&f.cScroll; return &f.code; }
    if(view==V_DISASM){ *scrollp=&f.dScroll; return &f.dis; }
    return nullptr; }

static void doSearch(int dir){ // search within current view (or strings)
    auto match=[&](const string& s){ string a=s,b=search; for(char&c:a)c=tolower(c); for(char&c:b)c=tolower(c); return !b.empty()&&a.find(b)!=string::npos; };
    if(view==V_STR){ int n=(int)g_str.size(); for(int s=1;s<=n;s++){ int i=((strScroll+dir*s)%n+n)%n; if(match(g_str[i].text)){ strScroll=i; return; } } g_status="no match"; return; }
    int* sp; vector<string>* L=activeLines(&sp); if(!L||L->empty()){ g_status="nothing to search here"; return; }
    int n=(int)L->size(); for(int s=1;s<=n;s++){ int i=(((*sp)+dir*s)%n+n)%n; if(match((*L)[i])){ *sp=i; return; } } g_status="no match";
}
static void scrollContent(int d){ Func& f=g_fn[sel];
    if(view==V_PSEUDO) f.cScroll+=d; else if(view==V_DISASM) f.dScroll+=d;
    else if(view==V_GRAPH) f.gScroll+=d; else if(view==V_HEX) hexScroll+=d;
    else if(view==V_STR) strScroll+=d; else if(view==V_LOG) logScroll+=d; else infoScroll+=d; }

// click a token in the content pane (pseudo/disasm) -> if it's a known function, jump
static void clickToken(int mx,int my){ if(view!=V_PSEUDO&&view!=V_DISASM)return;
    int r=my-topRow; if(r<0||r>=rows)return; int* sp; vector<string>* L=activeLines(&sp); if(!L)return;
    int li=*sp+r; if(li<0||li>=(int)L->size())return; const string& raw=(*L)[li];
    int srcCol=(mx-contentX)-gutter; if(srcCol<0||srcCol>=(int)raw.size())return;
    // expand identifier under srcCol
    int a=srcCol,b=srcCol; auto idc=[&](char c){return isalnum((unsigned char)c)||c=='_';};
    if(!idc(raw[srcCol]))return; while(a>0&&idc(raw[a-1]))a--; while(b+1<(int)raw.size()&&idc(raw[b+1]))b++;
    string w=raw.substr(a,b-a+1); for(int i=0;i<(int)g_fn.size();i++) if(g_fn[i].name==w){ jumpTo(i,true); g_status="→ "+w; return; }
}
// click a name in the xref strip -> jump
static void clickXref(int mx){ Func& f=g_fn[sel]; string xr;
    vector<std::pair<int,int>> spans; // (startcol,funcidx) — rebuild same string as draw
    int col=0; auto emit=[&](const string& s){ col+=s.size(); };
    string strip; if(!f.callees.empty()){ strip+=" →calls "; for(int c=0;c<(int)f.callees.size()&&c<6;c++){ spans.push_back({(int)strip.size(),f.callees[c]}); strip+=g_fn[f.callees[c]].name; strip+=" "; } }
    if(!f.callers.empty()){ strip+=" ←from "; for(int c=0;c<(int)f.callers.size()&&c<6;c++){ spans.push_back({(int)strip.size(),f.callers[c]}); strip+=g_fn[f.callers[c]].name; strip+=" "; } }
    int p=mx-1; for(int i=(int)spans.size()-1;i>=0;i--){ if(p>=spans[i].first){ int j=spans[i].second; int end=spans[i].first+(int)g_fn[j].name.size(); if(p<end){ jumpTo(j,true); g_status="→ "+g_fn[j].name; } return; } }
    (void)emit;
}

// ── actions (Run · Recompile · Patch-from-pseudocode · Terminal · Debug) ─────
static string shq(const string& s){ string o="'"; for(char c:s){ if(c=='\'')o+="'\\''"; else o+=c; } return o+"'"; }
static string dirOf(const string& p){ size_t s=p.find_last_of('/'); return s==string::npos?string("."):p.substr(0,s); }
static void capInto(const string& cmd,const string& prefix){ FILE* p=popen((cmd+" 2>&1").c_str(),"r"); if(!p){ logMsg("  (spawn failed)"); return; }
    string acc; char b[4096]; size_t n; while((n=fread(b,1,sizeof b,p))>0){ acc.append(b,n); size_t nl; while((nl=acc.find('\n'))!=string::npos){ logMsg(prefix+acc.substr(0,nl)); acc.erase(0,nl+1); if(g_log.size()>50000){ pclose(p); return; } } }
    if(!acc.empty()) logMsg(prefix+acc); pclose(p); }

static void runEmberRun(){ view=V_LOG;
    bool macho = g_bytes.size()>=4 && g_bytes[0]==0xCF&&g_bytes[1]==0xFA&&g_bytes[2]==0xED&&g_bytes[3]==0xFE;
    string nxrt="/usr/local/bin/nxrt"; struct stat st;
    string cmd = macho ? shq(g_bin) : ((stat(nxrt.c_str(),&st)==0?shq(nxrt):string("nxrt"))+" "+shq(g_bin));
    logMsg("EmberRun: "+baseName(g_bin)+(macho?"":" via NXRT"));
    logMsg("$ "+cmd);
    capInto(cmd+" </dev/null","  ");
    logMsg("EmberRun: done"); g_status="ran "+baseName(g_bin); }

// Analyze: the deep OFFLINE pass (ember-collapse — NO AI). Declares locals, names
// vars, structures control flow -> readable, compilable source. AI is the separate
// -ai build (swaps ember-collapse for ember-claude); it is deliberately NOT here.
static void runAnalyze(){ view=V_LOG;
    string tool=g_dir+"ember-collapse"; struct stat st; if(stat(tool.c_str(),&st)!=0) tool="ember-collapse";
    string outp=g_tmp+"/analyzed_"+g_name+".c";
    logMsg("analyze: deep offline pass (ember-collapse, no AI)…");
    system((shq(tool)+" < "+shq(g_srcpath)+" > "+shq(outp)+" 2>/dev/null").c_str());
    string txt=readFile(outp);
    if(txt.empty()){ logMsg("analyze: produced nothing (is ember-collapse installed?)"); return; }
    g_cpp = txt.find("#include <c")!=string::npos && txt.find(".h>")==string::npos ? true : (txt.find("std::")!=string::npos||g_cpp);
    g_srcpath=outp; g_analyzed=true;
    // swap in the analyzed pseudocode WITHOUT losing disasm / xrefs / graph. Analyze
    // RENAMES functions (sub_ -> verifyPassword) + vars, so match POSITIONALLY (defs keep
    // order; forward-decls are `;`-terminated -> land in globals) and adopt the new names.
    auto nf=parseCText(txt);
    vector<int> gi,ni; for(int i=0;i<(int)g_fn.size();i++) if(!g_fn[i].isGlobals) gi.push_back(i);
    for(int i=0;i<(int)nf.size();i++) if(!nf[i].isGlobals) ni.push_back(i);
    int upd=0,renamed=0;
    if(gi.size()==ni.size()){ for(size_t k=0;k<gi.size();k++){ Func& d=g_fn[gi[k]]; Func& s2=nf[ni[k]];
            d.code=s2.code; d.cScroll=0; upd++; if(d.name!=s2.name){ d.name=s2.name; renamed++; } } }
    else { std::map<string,vector<string>> code; for(auto&f:nf) code[f.name]=f.code;   // fallback: name-match code only
        for(auto&f:g_fn){ auto it=code.find(f.name); if(it!=code.end()){ f.code=it->second; f.cScroll=0; upd++; } } }
    for(auto&f:g_fn) if(f.isGlobals){ for(auto&n:nf) if(n.isGlobals){ f.code=n.code; break; } break; }
    logMsg("analyze: done -> readable source ("+std::to_string(upd)+" fn rewritten, "+std::to_string(renamed)+" behavior-named). recompile/patch now use it.");
    g_status="analyzed — "+std::to_string(upd)+" fn, "+std::to_string(renamed)+" named"; view=V_PSEUDO; }

static void runRecompile(){ view=V_LOG;
    if(!g_analyzed) logMsg("recompile: tip — press 'a' to Analyze first for readable, compilable source");
    string out=g_tmp+"/recompiled_"+g_name;
    string cc = g_cpp?"clang++ -std=c++17 -O0":"clang -std=c11 -O0";
    logMsg("recompile: "+string(g_cpp?"C++17 ":"C11 ")+baseName(g_srcpath)+" -> "+baseName(out));
    capInto(cc+" "+shq(g_srcpath)+" -o "+shq(out),"  ");
    struct stat st; if(stat(out.c_str(),&st)==0){ logMsg("recompile: OK -> "+out); g_status="recompiled OK"; }
    else { logMsg("recompile: FAILED — press 'a' to Analyze first (remaining errors are decompiler type-recovery gaps)"); g_status="recompile failed"; } }

static string symBase(const string& s){ string n=s; if(!n.empty()&&n[0]=='_')n=n.substr(1);
    if(n.rfind("_Z",0)==0||n.rfind("__Z",0)==0){ int st=0; char* d=abi::__cxa_demangle(n.c_str(),0,0,&st); if(st==0&&d){ string r=d; free(d); size_t p=r.find('('); if(p!=string::npos)r=r.substr(0,p); size_t sp=r.rfind(' '); if(sp!=string::npos)r=r.substr(sp+1); return r; } }
    return n; }

static void patchFromPseudo(){ view=V_LOG; Func& f=g_fn[sel];
    if(f.isGlobals||f.addr==0){ logMsg("patch: select a function with a code address first"); return; }
    string triple = g_arch=="arm64"?"arm64-apple-macos":g_arch=="x86-64"?"x86_64-apple-macos":"";
    if(triple.empty()){ logMsg("patch: only arm64 / x86-64 supported (this is "+g_arch+")"); return; }
    // locate __text + this function's original slot
    uint64_t textVa=0; size_t textOff=0,textSz=0, origOff=SIZE_MAX;
    for(auto&s:g_sec) if(s.seg=="__TEXT"&&s.name=="__text"){ textVa=s.vaddr; textOff=s.fileoff; textSz=s.size; }
    if(textVa&&f.addr>=textVa&&f.addr<textVa+textSz) origOff=textOff+(f.addr-textVa);
    if(origOff==SIZE_MAX){ logMsg("patch: '"+f.name+"' isn't in __text"); return; }
    uint64_t endVa=textVa+textSz; for(auto&g:g_fn) if(g.addr>f.addr&&g.addr<endVa) endVa=g.addr;
    size_t origLen=(size_t)(endVa-f.addr);
    // compile the whole (edited) source so callees/globals resolve
    string obj=g_tmp+"/.patchfn.o", errf=g_tmp+"/.patchfn.err";
    string cmd="clang++ -std=c++17 -w -c -Os -fno-inline -fno-stack-protector -fno-asynchronous-unwind-tables -target "+triple+" "+shq(g_srcpath)+" -o "+shq(obj)+" 2>"+shq(errf);
    logMsg("patch: assembling "+f.name+" from pseudocode…");
    if(system(cmd.c_str())!=0){ string e=readFile(errf); size_t nl=e.find('\n'); if(nl!=string::npos)e=e.substr(0,nl); logMsg(string("patch: source didn't compile — press 'a' to Analyze first")+(g_analyzed?" (remaining errors are decompiler type-recovery gaps)":"")+". "+(e.empty()?"":"("+e+")")); return; }
    // find F's bytes: its symbol -> next symbol in nm -n order
    string ob=readFile(obj); vector<uint8_t> o(ob.begin(),ob.end());
    size_t toff,tsz; uint64_t tva; if(!machoText(o.data(),o.size(),toff,tsz,tva)){ logMsg("patch: no __text in recompiled object"); return; }
    string nmout; { FILE* p=popen(("nm -n "+shq(obj)+" 2>/dev/null").c_str(),"r"); if(p){ char ln[1024]; while(fgets(ln,sizeof ln,p))nmout+=ln; pclose(p);} }
    vector<std::pair<uint64_t,string>> syms;
    for(size_t i=0;i<nmout.size();){ size_t e=nmout.find('\n',i); string ln=nmout.substr(i,(e==string::npos?nmout.size():e)-i); i=e==string::npos?nmout.size():e+1;
        if(ln.size()<19||ln[0]==' ')continue; uint64_t a=strtoull(ln.substr(0,16).c_str(),0,16); size_t sp=ln.find(' ',18); if(sp==string::npos)continue; syms.push_back({a,ln.substr(sp+1)}); }
    uint64_t fva=UINT64_MAX, nextva=tva+tsz;
    for(auto&s:syms) if(symBase(s.second)==f.name){ fva=s.first; break; }
    if(fva==UINT64_MAX){ logMsg("patch: couldn't find '"+f.name+"' in the object (inlined/renamed? give it a unique name)"); return; }
    for(auto&s:syms) if(s.first>fva&&s.first<nextva) nextva=s.first;
    size_t fb=toff+(size_t)(fva-tva), fe=toff+(size_t)(nextva-tva);
    if(fe>o.size()||fe<=fb){ logMsg("patch: bad function bounds in object"); return; }
    vector<uint8_t> nb(o.begin()+fb,o.begin()+fe);
    while(!nb.empty()&&nb.back()==0) nb.pop_back(); if(g_arch=="arm64") while(nb.size()%4) nb.push_back(0);
    if(nb.empty()){ logMsg("patch: recompiled '"+f.name+"' is empty"); return; }
    if(nb.size()>origLen){ char e[220]; snprintf(e,sizeof e,"patch: recompiled %s is %d bytes — won't fit the %d-byte slot (in-place patch must be same-or-smaller; trampolines are future work).",f.name.c_str(),(int)nb.size(),(int)origLen); logMsg(e); return; }
    for(size_t i=0;i<nb.size();i++) g_bytes[origOff+i]=nb[i];
    static const uint8_t A64NOP[4]={0x1f,0x20,0x03,0xd5}; bool x86=(g_arch=="x86-64");
    for(size_t i=nb.size();i<origLen;i++) g_bytes[origOff+i]= x86?0x90:A64NOP[i%4];
    dirty=true; view=V_HEX; hexEdit=true; hexCur=origOff; hexNib=0;
    char b[240]; snprintf(b,sizeof b,"patched %s: %d bytes into a %d-byte slot (NOP-padded). ⚠ inter-function calls aren't relocated — verify in Disasm, then w to write (.bak kept).",f.name.c_str(),(int)nb.size(),(int)origLen);
    logMsg(b); g_status="patched "+f.name+" — w to write"; }

// suspend the alt-screen and hand the real tty to an interactive program (shell/lldb)
static void suspendTo(const string& cmd){
    printf("\033[?1000l\033[?1006l\033[?25h\033[?1049l\033[0m"); fflush(stdout);
    tcsetattr(0,TCSANOW,&g_orig);
    printf("\n\033[38;2;255;138;42m🔥🐉 EmberDragon — dropped to: %s   (exit to return)\033[0m\n",cmd.c_str()); fflush(stdout);
    int rc=system(cmd.c_str()); (void)rc;
    struct termios r=g_orig; r.c_lflag&=~(ICANON|ECHO); r.c_cc[VMIN]=1; r.c_cc[VTIME]=0; tcsetattr(0,TCSANOW,&r);
    printf("\033[?1049h\033[?25l\033[?1000h\033[?1006h"); fflush(stdout); }
static string resolveLldb(){ FILE* f=popen("xcrun -f lldb 2>/dev/null","r"); if(f){ char b[1024]; string o; size_t n; while((n=fread(b,1,sizeof b,f))>0)o.append(b,n); pclose(f); while(!o.empty()&&(o.back()=='\n'||o.back()==' '))o.pop_back(); struct stat st; if(!o.empty()&&stat(o.c_str(),&st)==0)return o; } return "lldb"; }
// program-wide rename: whole-word replace old name -> new across every function's
// name, pseudocode, disasm (call sites included); CFG rebuilds lazily.
static void renameFn(int idx,const string& nn){ if(idx<0||idx>=(int)g_fn.size())return; string on=g_fn[idx].name;
    if(on==nn||nn.empty()) return;
    auto wr=[&](string& s){ for(size_t p=0;(p=s.find(on,p))!=string::npos;){ bool lb=p==0||!(isalnum((unsigned char)s[p-1])||s[p-1]=='_'); size_t e=p+on.size(); bool rb=e>=s.size()||!(isalnum((unsigned char)s[e])||s[e]=='_'); if(lb&&rb){ s.replace(p,on.size(),nn); p+=nn.size(); } else p+=on.size(); } };
    for(auto&f:g_fn){ if(f.name==on)f.name=nn; for(auto&l:f.code)wr(l); for(auto&l:f.dis)wr(l); f.graph.clear(); }
    logMsg("rename: "+on+" -> "+nn); g_status="renamed "+on+" -> "+nn; }
static void openTerminal(){ suspendTo("cd "+shq(dirOf(g_bin))+"; ${SHELL:-/bin/zsh} -i"); g_status="back from terminal"; }
static void openDebugger(){ suspendTo(resolveLldb()+" "+shq(g_bin)); g_status="back from lldb"; }

int main(int argc,char** argv){
    string bin;
    for(int i=1;i<argc;i++){ string a=argv[i]; if(a=="--ai"&&i+1<argc) g_mode=argv[++i]; else bin=a; }
    if(bin.empty()){ fprintf(stderr,"usage: ember-tui <binary> [--ai none|api|local]\n"); return 2; }
    g_bin=bin;
    fprintf(stderr,"🔥🐉 EmberDragon — decompiling %s ...\n", bin.c_str());

    // run the pipeline -> export c/ + asm/. Resolve the REAL executable dir so the
    // sibling `emberdragon` is found even when launched as a bare name via $PATH.
    string self=argv[0];
#ifdef __APPLE__
    { char buf[4096]; uint32_t sz=sizeof buf; if(_NSGetExecutablePath(buf,&sz)==0){ char rp[4096]; if(realpath(buf,rp)) self=rp; } }
#endif
    string dir=self.substr(0,self.find_last_of('/')+1);
    if(dir.empty()){ // no slash & couldn't resolve: search $PATH for emberdragon
        const char* pe=getenv("PATH"); string P=pe?pe:""; size_t i=0;
        while(i<P.size()){ size_t e=P.find(':',i); string d=P.substr(i,(e==string::npos?P.size():e)-i);
            struct stat st; if(!d.empty()&&stat((d+"/emberdragon").c_str(),&st)==0){ dir=d+"/"; break; } if(e==string::npos)break; i=e+1; }
        if(dir.empty()) dir="./";
    }
    char tmpl[]="/tmp/ember-tui.XXXXXX"; char* td=mkdtemp(tmpl); g_tmp = td?td:"/tmp/ember-tui.x";
    g_dir=dir;
    string cmd = shq(dir+"emberdragon")+" "+shq(bin)+" --ai "+g_mode+" --export --asm --out "+shq(g_tmp)+" >/dev/null 2>&1";
    system(cmd.c_str());
    string name=baseName(bin); g_name=name;
    struct stat sst;
    g_srcpath = (stat((g_tmp+"/cpp/"+name+".cpp").c_str(),&sst)==0) ? g_tmp+"/cpp/"+name+".cpp" : g_tmp+"/c/"+name+".c";
    string ctext=readFile(g_tmp+"/c/"+name+".c"); if(ctext.empty()) ctext=readFile(g_tmp+"/cpp/"+name+".cpp");
    string atext=readFile(g_tmp+"/asm/"+name+".s");
    string manifest=readFile(g_tmp+"/manifest.json"); { size_t a=manifest.find("\"arch\""); if(a!=string::npos){ size_t q=manifest.find('"',manifest.find(':',a)); if(q!=string::npos){ size_t q2=manifest.find('"',q+1); g_arch=manifest.substr(q+1,q2-q-1);} } }
    if(ctext.empty()){ fprintf(stderr,"ember-tui: nothing decompiled (need a Mach-O / .o)\n"); restore(); return 1; }
    g_cpp = g_srcpath.size()>4 && g_srcpath.substr(g_srcpath.size()-4)==".cpp";
    parseC(ctext);
    if(!atext.empty()) parseAsm(atext);
    buildXrefs();
    // raw bytes + sections for hex/strings
    { string raw=readFile(bin); g_bytes.assign(raw.begin(),raw.end()); machoSections((const uint8_t*)g_bytes.data(),g_bytes.size(),g_sec); }
    scanStrings();
    if(g_fn.empty()){ fprintf(stderr,"ember-tui: no functions\n"); restore(); return 1; }

    getsz(); raw();
    while(true){
        draw();
        unsigned char ch; if(read(0,&ch,1)!=1) break;
        g_status.clear();
        // ── hex edit mode: keys are captured for byte editing ──
        if(view==V_HEX && hexEdit){
            size_t sz=g_bytes.size(), base=hexBase();
            auto mvL=[&]{ if(hexNib==1)hexNib=0; else if(hexCur>base){hexCur--;hexNib=1;} };
            auto mvR=[&]{ if(hexNib==0)hexNib=1; else if(hexCur+1<sz){hexCur++;hexNib=0;} };
            if(ch=='i'||ch=='\t'){ hexEdit=false; g_status="hex: read-only (i to edit)"; }
            else if(ch=='q'){ break; }
            else if(ch=='w'){ saveBytes(); }
            else if(ch=='h'){ mvL(); } else if(ch=='l'){ mvR(); }
            else if(ch=='j'){ if(hexCur+16<sz)hexCur+=16; }
            else if(ch=='k'){ if(hexCur>=base+16)hexCur-=16; }
            else if(ch>='1'&&ch<='7'){ view=ch-'1'; hexEdit=false; }
            else if(isxdigit(ch)){ int v=(ch<='9')?ch-'0':(tolower(ch)-'a'+10);
                if(hexNib==0){ g_bytes[hexCur]=(uint8_t)((g_bytes[hexCur]&0x0f)|(v<<4)); hexNib=1; }
                else { g_bytes[hexCur]=(uint8_t)((g_bytes[hexCur]&0xf0)|v); hexNib=0; if(hexCur+1<sz)hexCur++; }
                dirty=true; g_status="edited (unsaved) — w to write, i to exit"; }
            else if(ch=='\033'){ unsigned char a; if(read(0,&a,1)==1 && (a=='['||a=='O')){ unsigned char b2; if(read(0,&b2,1)==1){
                if(b2=='A'){ if(hexCur>=base+16)hexCur-=16; } else if(b2=='B'){ if(hexCur+16<sz)hexCur+=16; }
                else if(b2=='C'){ mvR(); } else if(b2=='D'){ mvL(); } } } }
            continue;
        }
        if(ch=='q') break;
        else if(ch=='i'){ view=V_HEX; hexEdit=true; hexNib=0; if(hexCur<hexBase()||hexCur>=g_bytes.size())hexCur=hexBase(); g_status="HEX EDIT — type 0-9 a-f · arrows/hjkl move · w write · i exit"; }
        else if(ch=='a'){ runAnalyze(); }                  // Analyze (deep offline pass, NO AI)
        else if(ch=='r'){ runEmberRun(); }                 // Run (EmberRun)
        else if(ch=='R'){ runRecompile(); }                // Recompile source -> binary
        else if(ch=='p'){ patchFromPseudo(); }             // Patch function from pseudocode
        else if(ch=='`'||ch=='t'){ openTerminal(); }       // drop to a shell
        else if(ch=='d'){ openDebugger(); }                // lldb on the binary
        else if(ch=='w'){ if(dirty) saveBytes(); }         // write patched/edited bytes
        else if(ch>='1'&&ch<='7'){ view=ch-'1'; }
        else if(ch=='\t'){ view=(view+1)%V_COUNT; }
        else if(ch=='k'){ sel--; }
        else if(ch=='j'){ sel++; }
        else if(ch=='g'){ sel=g_visIdx.empty()?0:g_visIdx.front(); }
        else if(ch=='G'){ sel=g_visIdx.empty()?(int)g_fn.size()-1:g_visIdx.back(); }
        else if(ch=='['){ histBack(); }
        else if(ch==']'){ histFwd(); }
        else if(ch==4){ scrollContent(rows/2); }   // ^D
        else if(ch==21){ scrollContent(-rows/2); }  // ^U
        else if(ch=='\r'||ch=='\n'){ Func& f=g_fn[sel]; if(!f.callees.empty()) jumpTo(f.callees[0],true); }
        else if(ch=='n'){ if(!search.empty()) doSearch(+1); }
        else if(ch=='N'){ if(!search.empty()) doSearch(-1); }
        else if(ch=='/'){ printf("\033[?25h"); at(TH,1); printf("%s%s search: %s",BG2,ACC,RST); fflush(stdout);
            tcsetattr(0,TCSANOW,&g_orig); char fb[128]; if(fgets(fb,sizeof fb,stdin)){ search=fb; if(!search.empty()&&search.back()=='\n')search.pop_back(); }
            struct termios r=g_orig; r.c_lflag&=~(ICANON|ECHO); r.c_cc[VMIN]=1; r.c_cc[VTIME]=0; tcsetattr(0,TCSANOW,&r); printf("\033[?25l");
            if(!search.empty()) doSearch(+1); }
        else if(ch=='f'){ printf("\033[?25h"); at(TH,1); printf("%s%s filter: %s",BG2,ACC,RST); fflush(stdout);
            tcsetattr(0,TCSANOW,&g_orig); char fb[128]; if(fgets(fb,sizeof fb,stdin)){ filter=fb; if(!filter.empty()&&filter.back()=='\n')filter.pop_back(); }
            struct termios r=g_orig; r.c_lflag&=~(ICANON|ECHO); r.c_cc[VMIN]=1; r.c_cc[VTIME]=0; tcsetattr(0,TCSANOW,&r); printf("\033[?25l"); }
        else if(ch=='m'){ printf("\033[?25h"); at(TH,1); printf("%s%s rename '%s' to: %s",BG2,ACC,g_fn[sel].name.c_str(),RST); fflush(stdout);
            tcsetattr(0,TCSANOW,&g_orig); char fb[128]; string nn; if(fgets(fb,sizeof fb,stdin)){ nn=fb; while(!nn.empty()&&(nn.back()=='\n'||nn.back()==' '))nn.pop_back(); }
            struct termios r=g_orig; r.c_lflag&=~(ICANON|ECHO); r.c_cc[VMIN]=1; r.c_cc[VTIME]=0; tcsetattr(0,TCSANOW,&r); printf("\033[?25l");
            string id; for(char c:nn) if(isalnum((unsigned char)c)||c=='_')id+=c; if(!id.empty()) renameFn(sel,id); else g_status="rename: need a valid identifier"; }
        else if(ch==';'){ printf("\033[?25h"); at(TH,1); printf("%s%s comment on '%s': %s",BG2,ACC,g_fn[sel].name.c_str(),RST); fflush(stdout);
            tcsetattr(0,TCSANOW,&g_orig); char fb[256]; if(fgets(fb,sizeof fb,stdin)){ string c=fb; if(!c.empty()&&c.back()=='\n')c.pop_back(); g_fn[sel].comment=c; g_status=c.empty()?"comment cleared":"comment set"; }
            struct termios r=g_orig; r.c_lflag&=~(ICANON|ECHO); r.c_cc[VMIN]=1; r.c_cc[VTIME]=0; tcsetattr(0,TCSANOW,&r); printf("\033[?25l"); }
        else if(ch=='\033'){ unsigned char a; if(read(0,&a,1)!=1) continue;
            if(a=='['||a=='O'){ unsigned char b2; if(read(0,&b2,1)!=1) continue;
                if(b2=='A'){ sel--; } else if(b2=='B'){ sel++; }
                else if(b2=='C'){ view=(view+1)%V_COUNT; } else if(b2=='D'){ view=(view+V_COUNT-1)%V_COUNT; }
                else if(b2=='5'){ unsigned char z; if(read(0,&z,1)){} scrollContent(-rows/2); }
                else if(b2=='6'){ unsigned char z; if(read(0,&z,1)){} scrollContent(rows/2); }
                else if(b2=='<'){ // SGR mouse: ESC [ < b ; x ; y (M|m)
                    string seq; unsigned char z; while(read(0,&z,1)==1){ if(z=='M'||z=='m'){ seq+=(char)z; break;} seq+=(char)z; }
                    int btn=0,mx=0,my=0; char mm=0; if(sscanf(seq.c_str(),"%d;%d;%d%c",&btn,&mx,&my,&mm)>=3){
                        if(btn==64){ if(mx<=sideW){ sel--; } else scrollContent(-3); }       // wheel up
                        else if(btn==65){ if(mx<=sideW){ sel++; } else scrollContent(3); }    // wheel down
                        else if(mm=='M' && (btn&0x43)==0){ // left press
                            if(my==2){ for(int v=0;v<V_COUNT;v++) if(mx>=g_tabhit[v].x0&&mx<g_tabhit[v].x1){ view=g_tabhit[v].v; break; } }
                            else if(my>=topRow && my<topRow+rows && mx<=sideW){ int p=listScroll+(my-topRow); if(p<(int)g_visIdx.size()) sel=g_visIdx[p]; }
                            else if(my==TH-1){ clickXref(mx); }
                            else if(my>=topRow && my<topRow+rows && mx>contentX){ clickToken(mx,my); }
                        }
                    }
                }
            }
        }
        if(sel<0)sel=0; if(sel>=(int)g_fn.size())sel=(int)g_fn.size()-1;
    }
    restore();
    return 0;
}
