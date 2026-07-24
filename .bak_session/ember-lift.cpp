// nxlift — lift decoded x86-64 -> readable C-ish pseudocode WITH control-flow
// structuring. Per function: split into basic blocks, lift each (stack slots ->
// locals, fold mov/add/imul -> expressions, calls recover args), then recover
// real if/else and while from the block graph (back-edges -> while; forward
// conditionals -> if). Anything irreducible falls back to labeled goto (correct,
// just less pretty). Next layer up: struct/class/variable + data-template recovery.
//
// build:  clang++ -std=c++17 -O2 nxlift.cpp -o nxlift
// use:    nxlift <mach-o> [--from 0xADDR] [--to 0xADDR]
#include "ember.h"
#include <map>
#include <set>
#include <algorithm>
#include <cxxabi.h>
using namespace nx;
using std::string; using std::vector; using std::map; using std::set;

// when __cxa_demangle fails (mingw's libc++ ABI gap on `St3__1` / `[abi:...]` symbols), pull the first real
// length-prefixed Itanium identifier out — `_ZNSt3__112basic_string...C1...` -> `basic_string`. Readable,
// never worse than the raw mangled name. On macOS the real demangler succeeds, so this never fires.
static string demangleFallback(const string& s){
    for(size_t p=0;p<s.size();){
        if(isdigit((unsigned char)s[p])){ long len=0; size_t q=p; while(q<s.size()&&isdigit((unsigned char)s[q])){ len=len*10+(s[q]-'0'); q++; }
            if(len>0 && q+(size_t)len<=s.size()){ string id=s.substr(q,(size_t)len);
                if(!id.empty() && (isalpha((unsigned char)id[0])||id[0]=='_') && id.rfind("__",0)!=0) return id;   // first non-internal name wins
                p=q+(size_t)len; continue; }
            p=q; } else p++; }
    return s;
}
static string demangleX(const string& s){
    if(s.rfind("_Z",0)!=0 && s.rfind("__Z",0)!=0) return s;
    int st=0; char* d=abi::__cxa_demangle(s.c_str(),0,0,&st);
    if(st==0&&d){ string r=d; free(d); return r; } if(d) free(d); return demangleFallback(s); }

static const int ARGR[6] = {7,6,2,1,8,9};   // System V arg regs: rdi rsi rdx rcx r8 r9

struct Block {
    uint64_t addr=0; size_t bi=0, be=0;          // instruction index range [bi,be)
    vector<string> stmts;
    enum { FALL, GOTO, COND, RET } term=FALL;
    string cond, ret;                            // COND cond / RET expr
    uint64_t tgtT=0, tgtF=0;                      // COND: taken / fallthrough ; GOTO: tgtT
    bool isTarget=false, dead=false;              // isTarget: a jump lands here · dead: unreachable from entry
    string regdef[16];            // cross-block dataflow: exit value-expr of reg r, if (re)defined to a non-trivial expr
    uint32_t useBare=0, defSet=0; // useBare: reg read-before-local-def (cross-block input) · defSet: reg given a non-trivial value
};

struct Lifter {
    vector<Ins> ins; map<uint64_t,size_t> at;
    set<uint64_t> funcs, jtargets;
    std::unordered_map<uint64_t,string> syms;
    std::unordered_map<uint64_t,string> stubs;     // import stubs/GOT -> symbol name
    string reg[16]; set<int> argsSet; int tmp=0;
    string lastL, lastR; bool testMode=false;

    string name(uint64_t a){ auto st=stubs.find(a); if(st!=stubs.end()) return demangleX(st->second);
        auto it=syms.find(a); if(it!=syms.end()){ string dn=demangleX(it->second);   // demangle the DEFINITION name too (was raw -> _Z4add3iii)
            size_t par=dn.find('('); if(par!=string::npos)dn=dn.substr(0,par); size_t cc=dn.rfind("::"); if(cc!=string::npos)dn=dn.substr(cc+2);
            size_t sp=dn.rfind(' '); if(sp!=string::npos)dn=dn.substr(sp+1); return dn.empty()?it->second:dn; }
        char b[24]; snprintf(b,sizeof b,"sub_%llx",(unsigned long long)a); return b; }
    // Library/runtime symbol? Body is libc++/compiler boilerplate, not user code — keep callable, never decompile.
    bool isLibrary(uint64_t a){
        if(stubs.count(a)) return true;                              // import stub -> keep as named call, never lift
        auto it=syms.find(a); if(it==syms.end()) return false;
        const string& m=it->second; if(m=="main") return false;
        // judge by the fn's OWN mangled prefix (its namespace), not std:: anywhere in the signature
        string core=m; size_t u=core.find_first_not_of('_'); core = u==string::npos? "" : core.substr(u);
        static const char* RT[]={"clang_call_terminate","cxa_","Unwind_","gxx_personality","cxx_global",
                                 "GLOBAL__","mh_execute_header",
                                 "ZNSt","ZNKSt","ZNVSt","ZSt","ZNSa","ZNKSa",
                                 "ZN9__gnu_cxx","ZNK9__gnu_cxx","ZN10__cxxabiv",
                                 "ZGV","ZTV","ZTI","ZTS","ZTC","ZTT","Znw","Zna","Zdl","Zda"};
        for(auto r:RT) if(core.rfind(r,0)==0) return true;
        return false;
    }
    std::map<string, std::map<int,int>> fields;   // pointer base-expr -> {offset -> access width}  => recovered struct
    // ── cross-block dataflow instrumentation ──
    Block* curB=nullptr; uint32_t curLocalDef=0;
    static string seedName(int id){ int i=id&15; for(int k=0;k<6;k++) if(ARGR[k]==i) return "a"+std::to_string(k); return R64[i]; }   // per-block canonical name (args -> aN, else rax..r15)
    void noteRead(int id){ int i=id&15; if(!curB||(curLocalDef&(1u<<i)))return; if(reg[i]==seedName(i)) curB->useBare|=(1u<<i); }   // read while still at seed name = cross-block input
    string r(int id){ noteRead(id); return reg[id&15]; }
    void setReg(int id,const string& e){ reg[id&15]=e; for(int k=0;k<6;k++) if(ARGR[k]==(id&15)) argsSet.insert(id&15); curLocalDef|=(1u<<(id&15)); }
    static bool isName(const string& s){ if(s.empty()||!(isalpha((unsigned char)s[0])||s[0]=='_')) return false; for(char c:s) if(!(isalnum((unsigned char)c)||c=='_')) return false; return true; }
    static const char* tyOf(int w){ return w==1?"char":w==2?"short":w==4?"int":"long"; }
    // a memory operand -> text; if it's [namedPtr + off], record a struct field and render ptr->fOFF
    string memref(const Op& o, uint64_t nx, int width){
        if(o.rip) return "g_"+imm(nx+o.disp);
        if(o.base==5) return slot(o.disp);                       // rbp = stack local
        string be = (o.base>=0)? r(o.base) : "0";
        if(o.base>=0 && o.base!=4 && isName(be)){ fields[be][o.disp]=width; return be+"->"+fieldId(o.disp); }
        if(o.disp) return "*("+be+" + "+imm(o.disp)+")";
        return "*"+be;
    }
    string valOf(const Op& o, uint64_t nx, int width){
        if(o.t==Op::REG) return r(o.reg);
        if(o.t==Op::IMM) return imm(o.imm);
        if(o.t==Op::MEM) return memref(o,nx,width);
        return "?";
    }
    static string imm(int64_t v){ char b[32]; if(v>-1024&&v<1000000) snprintf(b,sizeof b,"%lld",(long long)v); else snprintf(b,sizeof b,"0x%llx",(unsigned long long)v); return b; }
    static string slot(int32_t d){ char b[24]; snprintf(b,sizeof b,"v%d", d<0?-d:d); return b; }
    // LEGAL C++ field id: negative offsets (vtable-relative access) become "fm24", not the illegal "f-24".
    static string fieldId(int off){ char b[16]; if(off<0) snprintf(b,sizeof b,"fm%d",-off); else snprintf(b,sizeof b,"f%d",off); return b; }
    string rhs(const Op& o, uint64_t nx){ return valOf(o, nx, o.w?8:4); }
    static const char* cop(const char* cc){ if(!strcmp(cc,"e"))return "=="; if(!strcmp(cc,"ne"))return "!=";
        if(!strcmp(cc,"l")||!strcmp(cc,"b"))return "<"; if(!strcmp(cc,"g")||!strcmp(cc,"a"))return ">";
        if(!strcmp(cc,"le")||!strcmp(cc,"be"))return "<="; if(!strcmp(cc,"ge")||!strcmp(cc,"ae"))return ">="; return "?"; }
    static string negate(const string& c){ // flip the comparison operator in "L <op> R"
        static const char* P[][2]={{"==","!="},{"!=","=="},{"<=",">"},{">=","<"},{"<",">="},{">","<="}};
        size_t sp=c.find(' '); if(sp==string::npos) return "!("+c+")";
        size_t sp2=c.find(' ', sp+1); if(sp2==string::npos) return "!("+c+")";
        string op=c.substr(sp+1, sp2-sp-1);
        for(auto&pr:P) if(op==pr[0]) return c.substr(0,sp+1)+pr[1]+c.substr(sp2);
        return "!("+c+")";
    }

    void scan(const uint8_t* code,size_t sz,uint64_t base){
        size_t o=0;
        while(o<sz){ Ins in=decode(code+o,sz-o,base+o); if(in.len<=0)in.len=1; at[in.addr]=ins.size(); ins.push_back(in); o+=in.len; }
        for(size_t k=0;k<ins.size();k++){ Ins& I=ins[k];
            if((I.mn=="jmp"||I.mn=="jcc")&&I.a.t==Op::REL) jtargets.insert(I.a.rel);
            else if(I.mn=="call"&&I.a.t==Op::REL) funcs.insert(I.a.rel);
            if(I.mn=="push"&&I.a.t==Op::REG&&I.a.reg==5 && k+1<ins.size()&&ins[k+1].mn=="mov"
               &&ins[k+1].a.t==Op::REG&&ins[k+1].a.reg==5&&ins[k+1].b.t==Op::REG&&ins[k+1].b.reg==4) funcs.insert(I.addr);
        }
        if(!ins.empty()) funcs.insert(ins.front().addr);
    }

    // lift instructions [bi,be) into a block, given whether it's the function entry
    Block liftBlock(size_t bi, size_t be, bool entry){
        Block B; B.bi=bi; B.be=be; B.addr=ins[bi].addr;
        curB=&B; curLocalDef=0;                                  // cross-block dataflow: track this block's reads/defs
        auto push=[&](const string& s){ B.stmts.push_back(s); };
        for(size_t k=bi;k<be;k++){ Ins& I=ins[k]; uint64_t nx=I.addr+I.len; const string& mn=I.mn;
            if(mn=="push"||mn=="pop"||mn=="leave"||mn=="nop"||mn=="cqo"||mn=="cdq"||mn=="cdqe"||mn=="clc"||mn=="stc"||mn=="cmc"||mn==".sse") continue;   // no value effect / elided vector op
            else if(mn=="mov"||mn=="lea"||mn=="movzxb"||mn=="movzxw"||mn=="movb"||mn=="movsxb"||mn=="movsxw"||mn=="movsxd"){
                int w = (mn=="movzxb"||mn=="movsxb")?1 : (mn=="movzxw"||mn=="movsxw")?2 : mn=="movsxd"?4 : (I.b.t==Op::REG? (I.b.w?8:4) : (I.a.w?8:4));
                if(I.a.t==Op::REG){
                    if(mn=="lea"&&I.b.t==Op::MEM){ if(I.b.rip) setReg(I.a.reg,"&g_"+imm(nx+I.b.disp)); else if(I.b.base==5) setReg(I.a.reg,"&"+slot(I.b.disp)); else setReg(I.a.reg,"("+r(I.b.base)+" + "+imm(I.b.disp)+")"); }
                    else setReg(I.a.reg, valOf(I.b,nx,w));
                } else if(I.a.t==Op::MEM){
                    if(I.a.base==5 && !I.a.rip) push(slot(I.a.disp)+" = "+valOf(I.b,nx,w)+";");
                    else push(memref(I.a,nx,w)+" = "+valOf(I.b,nx,w)+";");
                }
            }
            else if(mn=="add"||mn=="adc"||mn=="sub"||mn=="sbb"||mn=="and"||mn=="or"||mn=="xor"||mn=="imul"||mn=="shl"||mn=="shr"||mn=="sar"){
                const char* op=(mn=="add"||mn=="adc")?"+":(mn=="sub"||mn=="sbb")?"-":mn=="and"?"&":mn=="or"?"|":mn=="xor"?"^":mn=="imul"?"*":mn=="shl"?"<<":">>";
                if(I.a.t==Op::REG) setReg(I.a.reg,"("+r(I.a.reg)+" "+op+" "+rhs(I.b,nx)+")");
                else if(I.a.t==Op::MEM&&I.a.base==5) push(slot(I.a.disp)+" "+op+"= "+rhs(I.b,nx)+";");
            }
            else if(mn=="neg"){ if(I.a.t==Op::REG) setReg(I.a.reg,"(-"+r(I.a.reg)+")"); }
            else if(mn=="not"){ if(I.a.t==Op::REG) setReg(I.a.reg,"(~"+r(I.a.reg)+")"); }
            else if(mn=="inc"){ if(I.a.t==Op::REG) setReg(I.a.reg,"("+r(I.a.reg)+" + 1)"); else if(I.a.t==Op::MEM&&I.a.base==5) push(slot(I.a.disp)+" += 1;"); }
            else if(mn=="dec"){ if(I.a.t==Op::REG) setReg(I.a.reg,"("+r(I.a.reg)+" - 1)"); else if(I.a.t==Op::MEM&&I.a.base==5) push(slot(I.a.disp)+" -= 1;"); }
            else if(mn=="idiv"||mn=="div"){ string d=rhs(I.a,nx); setReg(0,"("+r(0)+" / "+d+")"); setReg(2,"("+r(0)+" % "+d+")"); }
            else if(mn=="mul"){ setReg(0,"("+r(0)+" * "+rhs(I.a,nx)+")"); }
            else if(mn=="imul3"){ if(I.a.t==Op::REG) setReg(I.a.reg,"("+valOf(I.b,nx,8)+" * "+std::to_string(I.cc)+")"); }   // imul r, r/m, imm
            else if(mn=="rol"||mn=="ror"){ if(I.a.t==Op::REG) setReg(I.a.reg, string(mn=="rol"?"std::rotl(":"std::rotr(")+r(I.a.reg)+", "+rhs(I.b,nx)+")"); }
            else if(mn=="rcl"||mn=="rcr"){ if(I.a.t==Op::REG) setReg(I.a.reg, r(I.a.reg)); }   // rotate-through-carry — rare; value approx
            else if(mn=="cmov"){ if(I.a.t==Op::REG){ string c = testMode? lastL+" "+cop(I.ccs)+" 0" : lastL+" "+cop(I.ccs)+" "+lastR;
                setReg(I.a.reg,"(("+c+") ? "+valOf(I.b,nx,8)+" : "+r(I.a.reg)+")"); } }   // conditional move -> ternary
            else if(mn=="cmp"){ lastL=rhs(I.a,nx); lastR=rhs(I.b,nx); testMode=false; }
            else if(mn=="test"){ lastL=rhs(I.a,nx); lastR=rhs(I.b,nx); testMode=true; }
            else if(mn=="call"||mn=="callr"){
                string args; for(int j=0;j<6;j++) if(argsSet.count(ARGR[j])){ if(!args.empty()) args+=", "; args+=r(ARGR[j]); }
                string callee = I.a.t==Op::REL ? name(I.a.rel) : "(*"+rhs(I.a,nx)+")";
                string t="t"+std::to_string(tmp++); push(t+" = "+callee+"("+args+");"); setReg(0,t); argsSet.clear();
            }
            else if(mn=="syscall") push("syscall("+r(0)+");");
            // terminators (last instr of the block)
            else if(mn=="jcc"){ B.term=Block::COND; B.cond = testMode? lastL+" "+cop(I.ccs)+" 0" : lastL+" "+cop(I.ccs)+" "+lastR; B.tgtT=I.a.rel; B.tgtF=nx; }
            else if(mn=="jmp"){ B.term=Block::GOTO; B.tgtT=I.a.rel; }
            else if(mn=="ret"){ B.term=Block::RET; B.ret=r(0); }
            else push("/* "+fmtIns(I)+" */");
        }
        // snapshot cross-block defs: registers given a non-trivial exit value (differs from their seed name)
        for(int rr=0;rr<16;rr++){ if(!(curLocalDef&(1u<<rr))||rr==4||rr==5)continue; const string& e=reg[rr];
            if(e==seedName(rr))continue; B.regdef[rr]=e; B.defSet|=(1u<<rr); }
        curB=nullptr;
        if(B.term==Block::FALL && be<ins.size()) B.tgtF=ins[be].addr;
        return B;
    }

    vector<Block> blocks; map<uint64_t,int> bidx;
    int BI(uint64_t a){ auto it=bidx.find(a); return it==bidx.end()? -1 : it->second; }

    // ── cross-block dataflow: materialize register values that are defined in one block and read in another
    // (otherwise they leak as bare rax/rbx/...). Liveness fixpoint over the CFG, then materialize at exits. ──
    void crossBlockDataflow(){
        int N=(int)blocks.size(); if(N==0) return;
        const uint32_t SPMASK = ~((1u<<4)|(1u<<5));                 // never treat rsp/rbp as a value var
        auto succs=[&](int b,int out[2])->int{ Block& B=blocks[b]; int n=0;
            if(B.term==Block::COND){ int t=BI(B.tgtT),f=BI(B.tgtF); if(t>=0)out[n++]=t; if(f>=0)out[n++]=f; }
            else if(B.term==Block::GOTO){ int t=BI(B.tgtT); if(t>=0)out[n++]=t; }
            else if(B.term==Block::FALL){ int f=BI(B.tgtF); if(f>=0)out[n++]=f; }
            return n; };
        vector<uint32_t> DEF(N,0), USE(N,0), liveIn(N,0), liveOut(N,0);
        for(int b=0;b<N;b++){ if(blocks[b].dead)continue; DEF[b]=blocks[b].defSet&SPMASK; USE[b]=blocks[b].useBare&SPMASK; }
        bool changed=true;
        while(changed){ changed=false;
            for(int b=N-1;b>=0;b--){ if(blocks[b].dead)continue;
                int s[2]; int n=succs(b,s); uint32_t lo=0; for(int i=0;i<n;i++) lo|=liveIn[s[i]];
                uint32_t li = USE[b] | (lo & ~DEF[b]);
                if(lo!=liveOut[b]||li!=liveIn[b]){ liveOut[b]=lo; liveIn[b]=li; changed=true; } } }
        for(int b=0;b<N;b++){ if(blocks[b].dead)continue; Block& B=blocks[b];
            uint32_t mat = DEF[b] & liveOut[b];
            for(int rr=0;rr<16;rr++){ if(!(mat&(1u<<rr))||rr==4||rr==5)continue; const string& e=B.regdef[rr]; if(e.empty())continue;
                B.stmts.push_back(seedName(rr)+" = "+e+";"); } }
    }

    void liftFn(size_t start,size_t end){
        // block leaders within [start,end)
        set<uint64_t> leaders; leaders.insert(ins[start].addr);
        for(size_t k=start;k<end;k++){ Ins& I=ins[k];
            if((I.mn=="jmp"||I.mn=="jcc")&&I.a.t==Op::REL){ if(I.a.rel>=ins[start].addr && (end>=ins.size()||I.a.rel<ins[end].addr)) leaders.insert(I.a.rel); if(k+1<end) leaders.insert(ins[k+1].addr); }
            else if(I.mn=="ret"&&k+1<end) leaders.insert(ins[k+1].addr);
        }
        for(int i=0;i<16;i++) reg[i]=R64[i];
        reg[7]="a0"; reg[6]="a1"; reg[2]="a2"; reg[1]="a3"; reg[8]="a4"; reg[9]="a5"; argsSet.clear();
        blocks.clear(); bidx.clear(); fields.clear();
        vector<uint64_t> L(leaders.begin(),leaders.end());
        for(size_t li=0;li<L.size();li++){
            size_t bi=at[L[li]]; size_t be = li+1<L.size()? at[L[li+1]] : end;
            // reset register state per block except keep entry's arg names
            if(li>0){ for(int i=0;i<16;i++) reg[i]=R64[i]; reg[7]="a0"; reg[6]="a1"; reg[2]="a2"; reg[1]="a3"; reg[8]="a4"; reg[9]="a5"; argsSet.clear(); }   // keep arg names consistent across blocks (so dataflow connects)
            Block B=liftBlock(bi,be, li==0); bidx[B.addr]=(int)blocks.size(); blocks.push_back(B);
        }
        // mark jump targets that need labels
        for(auto& B:blocks){ if(B.term==Block::COND){ int t=BI(B.tgtT); if(t>=0) blocks[t].isTarget=true; }
                              if(B.term==Block::GOTO){ int t=BI(B.tgtT); if(t>=0) blocks[t].isTarget=true; } }
        { set<int> reach; vector<int> stk{0};                                   // dead-block elimination: reachable from entry only
          while(!stk.empty()){ int b=stk.back(); stk.pop_back(); if(b<0||b>=(int)blocks.size()||reach.count(b))continue; reach.insert(b); Block& B=blocks[b];
            auto add=[&](uint64_t a){ int t=BI(a); if(t>=0)stk.push_back(t); };
            if(B.term==Block::COND){ add(B.tgtT); add(B.tgtF); } else if(B.term==Block::GOTO) add(B.tgtT); else if(B.term==Block::FALL) add(B.tgtF); }
          for(size_t i=0;i<blocks.size();i++) blocks[i].dead = !reach.count((int)i); }
        crossBlockDataflow();                              // materialize cross-block register values before structuring
        emit(0,(int)blocks.size(),1,0,0);                  // fills the output buffer (fields already collected in liftBlock)
        FnResult R; R.name=name(ins[start].addr); R.fields=fields; R.body=collect();
        // GOTO ELIMINATION: any function the structurer left with a goto is re-rendered goto-free as a
        // `while(1) switch(state)`. Clean functions keep their pretty while/if output.
        // structured-with-residual-gotos (IDA/Ghidra style) beats the while(1)switch flattening for
        // readability — flattening is OFF by default; EMBER_FLATTEN=1 opts back in.
        { static const bool FLAT = getenv("EMBER_FLATTEN") != nullptr;
          if(FLAT){ bool hasGoto=false; for(auto& l:R.body) if(l.find("goto ")!=string::npos){ hasGoto=true; break; }
            if(hasGoto){ emitStateMachine(); R.body=collect(); } } }
        for(auto& l:R.body){ for(int k=0;k<6;k++){ char a[8]; snprintf(a,8,"= a%d;",k); size_t p=l.find(a);
            if(p!=string::npos){ string lhs=trim(l.substr(0,p)); if(fields.count(lhs)) R.a0base=lhs; } } }   // local that aliases the this-pointer
        bool u6[6]={false}; for(auto& l:R.body) for(int k=0;k<6;k++){ char a[4]; snprintf(a,4,"a%d",k); if(l.find(a)!=string::npos) u6[k]=true; }
        for(int k=0;k<6;k++) if(u6[k]) R.maxArg=k;
        fns.push_back(R);
    }

    // buffered output so we can drop labels nothing jumps to after structuring
    struct Line{ string t; bool lab=false; uint64_t a=0; };
    vector<Line> out; set<uint64_t> used;
    static string IND(int n){ return string(n*4,' '); }
    static string hx(uint64_t a){ char b[24]; snprintf(b,sizeof b,"loc_%llx",(unsigned long long)a); return b; }
    void say(int i,const string& s){ out.push_back({IND(i)+s,false,0}); }
    void label(int i,uint64_t a){ out.push_back({IND(i)+hx(a)+":",true,a}); }
    void emitStmts(Block& B,int ind){ for(auto& s:B.stmts) say(ind,s); }
    vector<string> collect(){ vector<string> r; for(auto& L:out) if(!(L.lab && !used.count(L.a))) r.push_back(L.t); out.clear(); used.clear(); return r; }

    // Goto-free renderer: whole function as `int __s = 0; while (1) switch (__s) { case k: ...; __s = j; break; }`.
    // Block index = state id; entry = block 0. Every edge becomes a state assignment / conditional pick / return,
    // so no goto (and no labeled break/continue, which C lacks) survives. Also makes dangling impossible — every
    // resolvable target has a case by index.
    void emitStateMachine(){
        int N=(int)blocks.size();
        say(1,"int __s = 0;   // control-flow state (structured form of the original jumps)");
        say(1,"while (1) switch (__s) {");
        for(int k=0;k<N;k++){ Block& B=blocks[k];
            say(1,"case "+std::to_string(k)+":  // "+hx(B.addr));
            for(auto& s:B.stmts) say(2,s);
            if(B.term==Block::RET) say(2,"return "+B.ret+";");
            else if(B.term==Block::GOTO){ int t=BI(B.tgtT);
                if(t>=0) say(2,"__s = "+std::to_string(t)+"; break;");
                else say(2,"return 0;  // -> "+hx(B.tgtT)+" (tail-call / unrecovered)"); }
            else if(B.term==Block::COND){ int t=BI(B.tgtT), f=BI(B.tgtF);
                if(t>=0&&f>=0) say(2,"__s = ("+B.cond+") ? "+std::to_string(t)+" : "+std::to_string(f)+"; break;");
                else if(t>=0) say(2,"if ("+B.cond+") { __s = "+std::to_string(t)+"; break; } return 0;  // false -> "+hx(B.tgtF)+" (extern)");
                else if(f>=0) say(2,"if (!("+B.cond+")) { __s = "+std::to_string(f)+"; break; } return 0;  // true -> "+hx(B.tgtT)+" (extern)");
                else say(2,"return 0;  // both targets extern"); }
            else { int f=BI(B.tgtF); if(f<0&&k+1<N)f=k+1;
                if(f>=0) say(2,"__s = "+std::to_string(f)+"; break;");
                else say(2,"return 0;"); } }
        say(1,"}");
    }
    // a recovered function, captured (not printed) so we can synthesize classes across ALL of them
    struct FnResult { string name; std::map<string,std::map<int,int>> fields; string a0base; vector<string> body; int maxArg=-1; };
    vector<FnResult> fns;
    static string trim(const string& s){ size_t a=s.find_first_not_of(" "); if(a==string::npos) return ""; size_t b=s.find_last_not_of(" "); return s.substr(a,b-a+1); }
    static bool isIdent(char c){ return isalnum((unsigned char)c)||c=='_'; }
    static string replaceTok(const string& s,const string& from,const string& to){ if(from.empty())return s; string r; size_t i=0;
        while(i<s.size()){ if(s.compare(i,from.size(),from)==0 && (i==0||!isIdent(s[i-1])) && (i+from.size()>=s.size()||!isIdent(s[i+from.size()]))){ r+=to; i+=from.size(); } else r+=s[i++]; }
        return r; }
    void emitStructBody(std::map<int,int>& f, std::map<int,string>& nm, std::map<int,string>* ty=nullptr, const string& self=""){ int next=0; for(auto& fo:f){ if(fo.first>next&&fo.first>=0&&next>=0) printf("    char _pad%d[%d];\n",next,fo.first-next);
        string t = (ty&&ty->count(fo.first)) ? (*ty)[fo.first] : tyOf(fo.second); if(t=="@self") t = self.empty()?"void*":(self+"*");
        printf("    %s %s;\n",t.c_str(),nm[fo.first].c_str()); next=fo.first+fo.second; } }

    // ── humanize: infer readable names from behavior ──────────────────────────
    static void collectV(const string& l, set<string>& vs){
        for(size_t i=0;i<l.size();){ if(l[i]=='v'&&i+1<l.size()&&isdigit((unsigned char)l[i+1])&&(i==0||!isIdent(l[i-1]))){ size_t j=i+1; while(j<l.size()&&isdigit((unsigned char)l[j]))j++; if(j>=l.size()||!isIdent(l[j])) vs.insert(l.substr(i,j-i)); i=j; } else i++; } }
    // leaked cross-block x86-64 registers (rsp/rbp excluded — they're the frame). crossBlockDataflow
    // materializes them (assigned-before-use), so they read as raw machine registers; name them plain locals.
    static void collectRegs(const string& l, set<string>& vs){
        static const set<string> R={"rax","rcx","rdx","rbx","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
        for(size_t i=0;i<l.size();){ if((isalpha((unsigned char)l[i])||l[i]=='_')&&(i==0||!isIdent(l[i-1]))){ size_t j=i; while(j<l.size()&&isIdent(l[j]))j++; if(R.count(l.substr(i,j-i)))vs.insert(l.substr(i,j-i)); i=j; } else i++; } }
    // name vN locals by role: loop counter (cmp + ++) -> i/j/k; accumulator (=0 then += ) -> sum/total; params -> argN
    static bool isCKeyword(const string& s){ static const set<string> K={"int","long","char","short","double","float","void","unsigned","signed","bool","const","static","struct","class","auto","return","if","else","for","while","do","switch","new","delete","operator","this","true","false","to","of","the"}; return K.count(s)>0; }
    static string verbOf(const string& fn){
        static const std::map<string,string> M={{"strlen","len"},{"strnlen","len"},{"strcmp","cmp"},{"strncmp","cmp"},{"malloc","buf"},{"calloc","buf"},{"realloc","buf"},{"operator_new","obj"},{"fopen","fp"},{"sqrt","root"},{"fabs","mag"},{"atoi","n"},{"atol","n"},{"strtol","n"},{"strchr","p"},{"strrchr","p"},{"abs","val"},{"pow","p"},{"getchar","c"},
            {"toupper","upper"},{"tolower","lower"},{"strstr","pos"},{"strcpy","dst"},{"strncpy","dst"},{"strcat","dst"},{"strdup","dup"},{"memcpy","dst"},{"memmove","dst"},{"memset","dst"},{"memchr","p"},{"getenv","env"},{"fgets","line"},{"getline","line"},{"fread","n"},{"fwrite","n"},{"read","n"},{"write","n"},{"recv","n"},{"send","n"},{"socket","fd"},{"accept","fd"},{"open","fd"},{"floor","lo"},{"ceil","hi"},{"sin","val"},{"cos","val"},{"log","val"},{"exp","val"},{"rand","rnd"},{"strtok","tok"},{"isalpha","ok"},{"isdigit","ok"},{"isspace","ok"},{"isalnum","ok"}};
        auto it=M.find(fn); if(it!=M.end()) return it->second;
        string s=fn; for(const char* p:{"fx_","get_","compute_","calc_","make_","create_","find_","do_","read_","is_","has_"}){ size_t pl=strlen(p); if(s.size()>pl+1&&s.compare(0,pl,p)==0){ s=s.substr(pl); break; } }
        it=M.find(s); if(it!=M.end()) return it->second;
        if(s.empty()||s.rfind("sub_",0)==0||s.rfind("S_",0)==0||!isalpha((unsigned char)s[0])) return "";
        vector<string> segs; { string cur; for(char c:s){ if(c=='_'){ if(!cur.empty())segs.push_back(cur); cur.clear(); } else cur+=(char)tolower((unsigned char)c); } if(!cur.empty())segs.push_back(cur); }
        for(int i=(int)segs.size()-1;i>=0;i--){ const string& seg=segs[i]; if(seg.size()>=2&&isalpha((unsigned char)seg[0])&&!isCKeyword(seg)) return seg.size()>14?seg.substr(0,14):seg; }
        return ""; }
    string resolveDef(const string& v,const vector<string>& body){ int defs=0; string rhs;
        for(auto& l:body){ string t=trim(l); if(t.rfind(v+" = ",0)==0){ defs++; rhs=t.substr(v.size()+3); if(!rhs.empty()&&rhs.back()==';')rhs.pop_back(); } }
        if(defs!=1) return ""; while(!rhs.empty()&&rhs.front()=='(')rhs.erase(0,1);
        if(rhs.size()>=2&&rhs[0]=='t'&&isdigit((unsigned char)rhs[1])){ bool pure=true; for(char c:rhs)if(!isIdent(c))pure=false;
            if(pure){ for(auto& l:body){ string t=trim(l); if(t.rfind(rhs+" = ",0)==0){ string r2=t.substr(rhs.size()+3); if(!r2.empty()&&r2.back()==';')r2.pop_back(); while(!r2.empty()&&r2.front()=='(')r2.erase(0,1); return r2; } } } }
        return rhs; }
    std::map<string,string> nameVars(const vector<string>& body,int maxArg){
        std::map<string,string> m; set<string> vs; for(auto& l:body) collectV(l,vs); set<string> used;
        int ic=0;
        auto nextCounter=[&]()->string{ static const char* B[]={"i","j","k","l","m"}; string nm;
            do{ nm = (ic<5)? string(B[ic]) : (string(B[ic%5])+std::to_string(ic/5+1)); ic++; }while(used.count(nm)); used.insert(nm); return nm; };
        for(auto& v:vs){ bool cmp=false,inc=(false); for(auto& l:body){ string t=trim(l);
            if(t.rfind("while (",0)==0 && (t.find(v+" < ")!=string::npos||t.find(v+" != ")!=string::npos||t.find(v+" > ")!=string::npos||t.find(v+" <= ")!=string::npos)) cmp=true;
            if(t==v+" = ("+v+" + 1);") inc=true; }
            if(cmp&&inc){ m[v]=nextCounter(); } }
        // a `v=0; v=(v+1)` used as an array INDEX -> i/j/k (even without a `v<n` bound, e.g. strlen's counter)
        for(auto& v:vs){ if(m.count(v))continue; bool z=false,inc=false,idx=false; for(auto& l:body){ string t=trim(l);
            if(t==v+" = 0;")z=true; if(t==v+" = ("+v+" + 1);")inc=true;
            if(l.find(" + "+v+")")!=string::npos||l.find("("+v+" << ")!=string::npos||l.find("["+v+"]")!=string::npos)idx=true; }
            if(z&&inc&&idx){ m[v]=nextCounter(); } }
        const char* ACC[]={"sum","total","acc","result","prod"}; int ac=0;
        for(auto& v:vs){ if(m.count(v))continue; bool zero=false,accum=false; for(auto& l:body){ string t=trim(l);
            if(t==v+" = 0;") zero=true;
            if((t.rfind(v+" = ("+v+" + ",0)==0 && t!=v+" = ("+v+" + 1);")||t.rfind(v+" = ("+v+" - ",0)==0||t.rfind(v+" = ("+v+" * ",0)==0) accum=true; }
            if(zero&&accum&&ac<5){ m[v]=ACC[ac++]; used.insert(m[v]); } }
        set<string> called; for(auto& l:body){ size_t p=0; while((p=l.find('(',p))!=string::npos){ long b=(long)p-1; while(b>=0&&(isalnum((unsigned char)l[b])||l[b]=='_'))b--; if((long)p-1>b&&!isdigit((unsigned char)l[b+1]))called.insert(l.substr(b+1,p-1-b)); p++; } }
        for(auto& v:vs){ if(m.count(v))continue; string rhs=resolveDef(v,body); if(rhs.empty())continue;
            size_t e=0; while(e<rhs.size()&&(isalnum((unsigned char)rhs[e])||rhs[e]=='_'))e++; string callee=rhs.substr(0,e);
            if(e>=rhs.size()||rhs[e]!='('||callee.empty()||isdigit((unsigned char)callee[0])) continue;
            string nn=verbOf(callee); if(nn.size()<2||called.count(nn)) continue;
            string fin=nn; int s=2; while(used.count(fin)||called.count(fin))fin=nn+std::to_string(s++); m[v]=fin; used.insert(fin); }
        auto take=[&](const string& v,const char* base){ string fin=base; int s=2; while(used.count(fin)||called.count(fin))fin=base+std::to_string(s++); m[v]=fin; used.insert(fin); };
        for(auto& v:vs){ if(m.count(v))continue; bool idx=false,arrow=false,deref=false;   // POINTER/BUFFER: vN[i]/*( vN + i) -> buf, vN->f -> obj, *vN -> ptr
            for(auto& l:body){ if(l.find(v+"[")!=string::npos)idx=true; if(l.find(v+"->")!=string::npos)arrow=true;
                if(l.find("*("+v+" + ")!=string::npos)idx=true; if(l.find("*("+v+")")!=string::npos)deref=true;
                size_t p=0; while((p=l.find("*"+v,p))!=string::npos){ size_t af=p+1+v.size(); if((p==0||!isIdent(l[p-1]))&&(af>=l.size()||!isIdent(l[af])))deref=true; p=af; } }
            if(idx)take(v,"buf"); else if(arrow)take(v,"obj"); else if(deref)take(v,"ptr"); }
        for(auto& l:body){ string t=trim(l); if(t.rfind("return ",0)!=0)continue; string r=t.substr(7); if(!r.empty()&&r.back()==';')r.pop_back(); r=trim(r);   // returned value -> result
            if(r.size()>=2&&r[0]=='v'&&isdigit((unsigned char)r[1])){ bool pure=true; for(char c:r)if(!isIdent(c))pure=false; if(pure&&!m.count(r))take(r,"result"); } }
        for(auto& v:vs){ if(m.count(v))continue; bool bound=false;   // loop bound `i < vN` -> n
            for(auto& l:body){ size_t p=0; string pat="< "+v; while((p=l.find(pat,p))!=string::npos){ size_t af=p+pat.size(); if(af>=l.size()||!isIdent(l[af]))bound=true; p+=pat.size(); } }
            if(bound)take(v,"n"); }
        // ── PARAMETERS: name by ROLE (pointer/string/array/bound/scalar), not argN ──
        const char* SCAL[]={"a","b","c","d","e","f","g","h"}; int sc=0;
        for(int k=0;k<=maxArg;k++){ string ak="a"+std::to_string(k); if(m.count(ak))continue;
            set<string> names; names.insert(ak);                              // -O0 spills the param: `vJ = aK;` carries the real usage
            for(auto& l:body){ string t=trim(l); string suf=" = "+ak+";";
                if(t.size()>suf.size() && t.compare(t.size()-suf.size(),suf.size(),suf)==0){ string lhs=trim(t.substr(0,t.size()-suf.size())); if(isName(lhs)) names.insert(lhs); } }
            bool arrow=false,idxSh=false,idxByte=false,bound=false,self=false,list=false,nul=false;
            for(auto& nm:names){ for(auto& l:body){
                if(l.find(nm+"->")!=string::npos) arrow=true;
                if(l.find(nm+" = "+nm+"->")!=string::npos) list=true;                                   // h = h->next
                if(l.find("("+nm+" << ")!=string::npos||l.find(nm+"[")!=string::npos) idxSh=true;
                if(l.find("*("+nm+" + ")!=string::npos){ if(l.find("<< ")!=string::npos)idxSh=true; else idxByte=true; }
                { size_t p=0; string b="< "+nm; while((p=l.find(b,p))!=string::npos){ size_t af=p+b.size(); if(af>=l.size()||!isIdent(l[af]))bound=true; p+=b.size(); } }
                if(l.find("("+nm+" * "+nm+")")!=string::npos) self=true;
                if((l.find("*("+nm)!=string::npos||l.find(nm+"[")!=string::npos) && l.find("== 0")!=string::npos) nul=true; } }
            const char* base=nullptr;
            if(list) base="node"; else if(arrow) base="obj";
            else if(idxByte&&nul) base="s"; else if(idxByte) base="data";
            else if(idxSh) base="arr";
            else if(bound) base="n";
            else if(self) base="x";
            if(base) take(ak,base); else { take(ak, SCAL[sc<8?sc:7]); sc++; } }
        // LEAKED REGISTERS -> plain locals vN (generic, keyword-free, predictable; AI Clean-Up gives meaning).
        { set<string> regs; for(auto& l:body)collectRegs(l,regs);
          int next=0; auto bump=[&](const string& s){ if(s.size()>1&&s[0]=='v'){ bool num=true; for(size_t i=1;i<s.size();i++)if(!isdigit((unsigned char)s[i]))num=false; if(num){ int n=atoi(s.c_str()+1); if(n>=next)next=n+1; } } };
          for(auto& v:vs)bump(v); for(auto& kv:m)bump(kv.second);
          for(auto& r:regs){ if(m.count(r))continue; string fin; do{ fin="v"+std::to_string(next++); }while(used.count(fin)||called.count(fin)); m[r]=fin; used.insert(fin); } }
        return m;
    }
    // name struct fields by role: only-ever-+1 -> count; accumulated -> total; pointer-deref -> next; else value/data/...
    // best-guess NAME + C TYPE per field from how `base->fN` is used across (base,body) contexts; names deduped.
    void fieldRoles(std::map<int,int>& flds, const vector<std::pair<string,const vector<string>*>>& ctx,
                    std::map<int,string>& names, std::map<int,string>& types){
        const char* GEN[]={"value","data","item","member"}; int gc=0; set<string> used;
        auto uniq=[&](const string& b)->string{ if(!used.count(b)){used.insert(b);return b;} for(int s=2;;s++){ string c=b+std::to_string(s); if(!used.count(c)){used.insert(c);return c;} } };
        // libc++ std::string shape: width-1 flag byte (off>=8) tested `< 0`/`>= 0`, 8-byte size at +8, data ptr at 0.
        int strCap=-1; bool isStr=false;
        for(auto& bc:ctx){ const string& B=bc.first;
            for(auto& fo:flds){ if(fo.second!=1||fo.first<8)continue; string a=B+"->"+fieldId(fo.first);
                for(auto& l:*bc.second) if(l.find(a+" < 0")!=string::npos||l.find(a+" >= 0")!=string::npos) strCap=fo.first; } }
        if(strCap>=0 && flds.count(8) && flds[8]==8) isStr=true;
        for(auto& fo:flds){ int off=fo.first,width=fo.second; string F=fieldId(off);
            bool count=false,accum=false,next=false,bytePtr=false,sized=false,boolish=false;
            for(auto& bc:ctx){ const string& B=bc.first; string a=B+"->"+F;
                for(auto& l:*bc.second){ string t=trim(l); if(l.find(a)==string::npos)continue;
                    if(t==a+" = ("+a+" + 1);")count=true;
                    else if(t.rfind(a+" = (",0)==0 && (t.find(a+" + ")!=string::npos||t.find(a+" - ")!=string::npos||t.find(a+" * ")!=string::npos||t.find(" + "+a+")")!=string::npos||t.find(" * "+a+")")!=string::npos))accum=true;   // field on either side (commutative + and *)
                    if(l.find(a+"->")!=string::npos||t==B+" = "+a+";")next=true;
                    if(l.find("*("+a+")")!=string::npos||l.find(a+"[")!=string::npos||l.find("*("+a+" + ")!=string::npos)bytePtr=true;
                    for(const char* mf:{"memmove(","memcpy(","strcpy(","strncpy(","strlen(","strcmp(","strncmp(","strcat("}) if(l.find(mf)!=string::npos&&l.find(a)!=string::npos){bytePtr=true;break;}
                    { string pat="< "+a; size_t p=0; while((p=l.find(pat,p))!=string::npos){ size_t b=p+pat.size(); if(b>=l.size()||!isIdent(l[b]))sized=true; p+=pat.size(); } }
                    if(width==1&&(l.find(a+" == 0")!=string::npos||l.find("!("+a+")")!=string::npos||l.find(a+" & ")!=string::npos))boolish=true; } }
            string nm,ty=tyOf(width);
            if(isStr&&off==8){ nm="size"; }
            else if(isStr&&off==strCap){ nm="cap"; }
            else if(isStr&&off==0){ nm="data"; ty="char*"; }
            else if(next){ nm="next"; ty="@self"; }
            else if(bytePtr){ nm="data"; ty="char*"; }
            else if(sized){ nm="size"; }
            else if(count&&!accum){ nm="count"; }
            else if(accum){ nm="total"; }
            else if(boolish){ nm="flag"; }
            else { nm=GEN[gc%4]; if(gc>=4)nm+=std::to_string(off); gc++; }
            names[off]=uniq(nm); types[off]=ty; }
    }
    string guessTypeName(std::map<int,string>& names, const vector<string>& methodNames, const string& fallback){
        set<string> r; for(auto& kv:names){ string n=kv.second; while(!n.empty()&&isdigit((unsigned char)n.back()))n.pop_back(); r.insert(n); }
        auto has=[&](const string& s){ return r.count(s)>0; };
        for(auto& mn:methodNames){ string m=mn; for(auto& c:m)c=(char)tolower((unsigned char)c);
            if(m.find("push")!=string::npos||m.find("pop")!=string::npos) return "Stack";
            if(m.find("enqueue")!=string::npos||m.find("dequeue")!=string::npos) return "Queue";
            if(m.find("deposit")!=string::npos||m.find("withdraw")!=string::npos) return "Account"; }
        if(has("data")&&has("size")&&has("cap")) return "String";   // libc++ std::string shape
        if(has("next")) return names.size()<=2?"Node":"ListNode";
        if(has("data")&&has("size")) return "Buffer";
        if(has("count")&&has("total")) return "Counter";
        if(names.size()==2) return "Pair";
        return fallback;
    }
    std::map<int,string> nameFields(std::map<int,int>& flds, vector<int>& methods){
        std::map<int,string> m; const char* GEN[]={"value","data","item","member"}; int gc=0;
        for(auto& fo:flds){ int off=fo.first; string F=fieldId(off);
            bool count=false,total=false,ptr=false;
            for(int mi:methods){ string T=fns[mi].a0base; if(T.empty()) continue; for(auto& l:fns[mi].body){ string t=trim(l);   // receiver is still the alias local here, not yet "this"
                if(t==T+"->"+F+" = ("+T+"->"+F+" + 1);") count=true;
                else if(t.rfind(T+"->"+F+" = ("+T+"->"+F+" + ",0)==0||t.rfind(T+"->"+F+" = ("+T+"->"+F+" - ",0)==0) total=true;
                if(l.find(T+"->"+F+"->")!=string::npos) ptr=true; } }
            m[off] = count?"count" : ptr?"next" : total?"total" : (string(GEN[gc%4])+(gc>=4?std::to_string(off):"")); if(!count&&!ptr&&!total) gc++; }
        return m;
    }
    static string applyMap(string s, std::map<string,string>& m){ for(auto& kv:m) s=replaceTok(s,kv.first,kv.second); return s; }
    // ── string / data recovery from .rodata/.cstring ──────────────────────────
    const vector<uint8_t>* file=nullptr; vector<Section> secs;
    std::map<string,string> strRepl; vector<std::pair<string,string>> strDecls;
    string readCStr(uint64_t va){ for(auto& s:secs){ if(va>=s.vaddr && va<s.vaddr+s.size){ size_t off=s.fileoff+(va-s.vaddr); string r;
        for(size_t i=off;i<file->size()&&(*file)[i];i++){ unsigned char c=(*file)[i]; if(c<9||(c>13&&c<32)||c>126) return ""; r+=(char)c; if(r.size()>120)break; } return r; } } return ""; }
    static string strName(const string& s){ string r="s_"; for(char c:s){ if(isalnum((unsigned char)c)) r+=tolower(c); else if(!r.empty()&&r.back()!='_') r+='_'; if(r.size()>=22)break; } while(r.size()>2&&r.back()=='_')r.pop_back(); return r.size()>2?r:"s_str"; }
    static string replaceAll(string s,const string& a,const string& b){ if(a.empty())return s; size_t p=0; while((p=s.find(a,p))!=string::npos){ s.replace(p,a.size(),b); p+=b.size(); } return s; }
    static string esc(const string& s){ string r; for(char c:s){ if(c=='"'||c=='\\') r+='\\',r+=c; else if(c=='\n') r+="\\n"; else if(c=='\t') r+="\\t"; else r+=c; } return r; }
    void buildStrings(){
        std::map<string,uint64_t> toks;
        for(auto& F:fns) for(auto& l:F.body){ size_t i=0; while((i=l.find("g_",i))!=string::npos){ size_t j=i+2; while(j<l.size()&&(isxdigit((unsigned char)l[j])||l[j]=='x'))j++;
            string num=l.substr(i+2,j-(i+2)); if(!num.empty()) toks["g_"+num]=strtoull(num.c_str(),0,0); i=j; } }
        std::map<string,int> seen;
        for(auto& kv:toks){ string s=readCStr(kv.second); if(s.size()<2) continue; string nm=strName(s); if(seen[nm]++) nm+="_"+std::to_string(seen[nm]);
            strRepl["&"+kv.first]=nm; strRepl[kv.first]=nm; strDecls.push_back({nm, esc(s)}); }
    }
    // ── DATA TEMPLATES: materialize non-string data globals (.data/.rodata) into real C initializers,
    // so the offline decompile carries the program's ACTUAL data — no AI, no fake stubs. ──
    vector<std::pair<string,string>> dataDecls;
    std::map<string, vector<string>> dataHint;   // fnname -> ["dest=int[9]", ...] for ember-collapse to type the memcpy dest
    std::map<string,int> roleCount; std::map<string, std::map<string,string>> destRename;
    void mergeDataNames(const string& fn, std::map<string,string>& vmap){ auto it=destRename.find(fn); if(it!=destRename.end()) for(auto& kv:it->second) vmap[kv.first]=kv.second; }
    const Section* dataSecOf(uint64_t va){ for(auto& s:secs) if(va>=s.vaddr && va<s.vaddr+s.size){ return s.name==".text"||s.name=="__text"?nullptr:&s; } return nullptr; }
    vector<unsigned char> readBytes(uint64_t va,size_t n){ vector<unsigned char> b; for(auto& s:secs) if(va>=s.vaddr && va<s.vaddr+s.size){ size_t off=s.fileoff+(va-s.vaddr);
        for(size_t i=0;i<n && va+i<s.vaddr+s.size;i++) b.push_back(off+i<file->size()?(*file)[off+i]:0); break; } return b; }
    static string hexBytes(const vector<unsigned char>& b){ string r; for(size_t i=0;i<b.size();i++){ char x[8]; snprintf(x,sizeof x,"0x%02x",b[i]); r+=x; if(i+1<b.size())r+=","; r+=(i%16==15&&i+1<b.size())?"\n    ":" "; } return r; }
    static bool looksDouble(const vector<unsigned char>& b){ if(b.size()<8||b.size()%8)return false; int nz=0,ok=0;
        for(size_t i=0;i+8<=b.size();i+=8){ bool z=true; for(int k=0;k<8;k++)if(b[i+k])z=false; if(z)continue; nz++; unsigned char hi=b[i+7]; if(hi==0x3f||hi==0x40||hi==0xbf||hi==0xc0)ok++; }
        return nz>0 && ok==nz; }
    static string decVals(const vector<unsigned char>& b,int elsize,bool dbl){ string r; size_t n=b.size()/elsize;
        for(size_t i=0;i<n;i++){ const unsigned char* p=&b[i*elsize];
            if(dbl){ double d; memcpy(&d,p,8); char x[40]; snprintf(x,sizeof x,"%g",d); double rt=0; sscanf(x,"%lf",&rt); if(rt!=d) snprintf(x,sizeof x,"%.17g",d); r+=x; }
            else { uint64_t u=0; for(int k=elsize-1;k>=0;k--) u=(u<<8)|p[k]; int64_t v=elsize==4?(int32_t)u:elsize==2?(int16_t)u:elsize==1?(int8_t)u:(int64_t)u; r+=std::to_string((long long)v); }
            if(i+1<n)r+=","; r+=(i%16==15&&i+1<n)?"\n    ":" "; }
        return r; }
    static uint64_t le64(const unsigned char* p){ uint64_t v=0; for(int k=7;k>=0;k--)v=(v<<8)|p[k]; return v; }
    bool inText(uint64_t va){ for(auto& s:secs) if((s.name==".text"||s.name=="__text") && va>=s.vaddr && va<s.vaddr+s.size) return true; return false; }
    static string hx64(uint64_t v){ char b[24]; snprintf(b,sizeof b,"0x%llxL",(unsigned long long)v); return b; }
    string resolvePtr(uint64_t v){ if(!v) return "0"; string s=readCStr(v); if(s.size()>=1) return "\""+esc(s)+"\"";
        if(inText(v)){ auto it=syms.find(v); if(it!=syms.end()) return "(void*)&"+demangleX(it->second); } return "(void*)"+hx64(v); }
    bool ptrResolves(uint64_t v){ return v && (!readCStr(v).empty()||inText(v)||dataSecOf(v)); }
    bool isPtrTable(const vector<unsigned char>& b){ if(b.size()<8||b.size()%8)return false; int tot=0,ok=0;
        for(size_t i=0;i+8<=b.size();i+=8){ uint64_t v=le64(&b[i]); if(!v)continue; tot++; if(ptrResolves(v))ok++; } return tot>0&&ok*2>=tot; }
    string ptrVals(const vector<unsigned char>& b,bool& allStr){ allStr=true; string r; size_t n=b.size()/8;
        for(size_t i=0;i<n;i++){ string e=resolvePtr(le64(&b[i*8])); if(e.empty()||e[0]!='"')allStr=false; r+=(e.empty()?"(void*)0":e); if(i+1<n)r+=","; r+="\n    "; } return r; }
    static bool looksCode(const vector<unsigned char>& b){ return b.size()>=4 && b[0]==0x55 && b[1]==0x48 && b[2]==0x89 && b[3]==0xe5; }   // push rbp; mov rbp,rsp
    static bool looksStringPool(const vector<unsigned char>& b){ if(b.size()<4)return false; int pr=0,np=0,run=0,mxrun=0; bool nul=false;
        for(unsigned char c:b){ if(c==0){nul=true;run=0;continue;} np++; bool p=(c>=0x20&&c<0x7f)||c=='\n'||c=='\t'; if(p){pr++;run++;mxrun=std::max(mxrun,run);} else run=0; } return nul&&np>=3&&mxrun>=3&&pr*5>=np*4; }
    static bool looks8byte(const vector<unsigned char>& b){ if(b.size()<16||b.size()%8)return false; int tot=0,zhi=0,same=0; uint32_t f=0; bool fs=false;
        for(size_t i=0;i+8<=b.size();i+=8){ uint32_t hi=b[i+4]|(b[i+5]<<8)|(b[i+6]<<16)|((uint32_t)b[i+7]<<24); tot++; if(!hi)zhi++; if(!fs){f=hi;fs=true;} if(hi==f)same++; } return tot>=2&&(zhi*2>=tot||(f&&same==tot)); }
    static string poolStr(const vector<unsigned char>& b){ string r="\""; size_t last=b.size(); while(last>0&&b[last-1]==0)last--;
        for(size_t i=0;i<last;i++){ unsigned char c=b[i]; if(c=='"'||c=='\\'){r+='\\';r+=c;} else if(c=='\n')r+="\\n"; else if(c=='\t')r+="\\t"; else if(c>=0x20&&c<0x7f)r+=(char)c; else { char x[8]; snprintf(x,sizeof x,"\\%03o",c); r+=x; } } return r+"\""; }
    void buildDataTemplates(){
        std::map<string,uint64_t> toks;
        for(auto& F:fns) for(auto& l:F.body){ size_t i=0; while((i=l.find("g_",i))!=string::npos){ size_t j=i+2; while(j<l.size()&&(isxdigit((unsigned char)l[j])||l[j]=='x'))j++;
            string nm=l.substr(i,j-i); uint64_t va=strtoull(l.substr(i+2,j-(i+2)).c_str(),0,0);
            if(nm.size()>2 && !strRepl.count(nm) && !strRepl.count("&"+nm)) toks[nm]=va; i=j; } }
        for(auto& kv:toks){ const string& gx=kv.first; uint64_t va=kv.second; const Section* sec=dataSecOf(va); if(!sec)continue;
            long N=-1; string dest, destFn;
            for(auto& F:fns){ for(auto& l:F.body) for(const string& pre:{string(", "),string(", &")}){ size_t gp=l.find(pre+gx+", "); if(gp==string::npos)continue;
                size_t np=gp+pre.size()+gx.size()+2,ne=np; while(ne<l.size()&&isdigit((unsigned char)l[ne]))ne++; if(ne>np){ long n=atol(l.substr(np,ne-np).c_str()); if(n>0&&n<=65536){ N=n;
                    size_t mp=l.find("memcpy("); if(mp!=string::npos&&mp<gp){ size_t a=mp+7; while(a<l.size()&&(l[a]==' '||l[a]=='&'))a++; size_t e=a; while(e<l.size()&&(isalnum((unsigned char)l[e])||l[e]=='_'))e++; dest=l.substr(a,e-a); destFn=F.name; } } } } if(N>0)break; }
            if(N<=0) N=(long)std::min<uint64_t>(sec->vaddr+sec->size-va, 64);
            if(N<=0||N>65536)continue;
            vector<unsigned char> b=readBytes(va,(size_t)N); if(b.empty())continue;
            if(b.size()>=4 && b[0]==0xcf&&b[1]==0xfa&&b[2]==0xed&&b[3]==0xfe) continue;   // Mach-O header magic — not data
            if(looksCode(b)) continue;                                       // a function, not data
            if(sec->name=="__cstring"){ size_t e=0; while(e<b.size()&&b[e])e++; vector<unsigned char> sb(b.begin(),b.begin()+e);
                if(!sb.empty()){ int rn=++roleCount["str"]; string gn="str"+(rn>1?std::to_string(rn):""); strRepl[gx]=gn;
                    dataDecls.push_back({gn,"static const char "+gn+"[] = "+poolStr(sb)+";"}); continue; } }
            bool indexed=false, scalarDeref=false; int stride=0;
            for(auto& F:fns) for(auto& l:F.body){ size_t p=l.find("*("+gx+" + ");
                if(p!=string::npos){ indexed=true; size_t sh=l.find(" << ",p), cl=l.find(')',p); if(sh!=string::npos&&sh<cl){ int k=atoi(l.c_str()+sh+4); if(k>=0&&k<=4)stride=std::max(stride,1<<k); } else if(stride==0)stride=1; }
                if(l.find(gx+"[")!=string::npos)indexed=true;
                if(l.find("*"+gx)!=string::npos && l.find("*("+gx)==string::npos)scalarDeref=true; }
            if(N%8==0 && b.size()>=8 && isPtrTable(b)){ bool allStr; string pv=ptrVals(b,allStr); int cnt=(int)(b.size()/8);
                string ety=allStr?"const char*":"void*", role=allStr?"strings":"ptrs"; int rn=++roleCount[role]; string gn=role+(rn>1?std::to_string(rn):"");
                strRepl[gx]=gn; dataDecls.push_back({gn,"static "+ety+" "+gn+"["+std::to_string(cnt)+"] = {\n    "+pv+"\n};"}); continue; }
            if(dest.empty() && scalarDeref && !indexed){ vector<unsigned char> sb=readBytes(va,8); string ty,val;
                if(sb.size()==8 && looksDouble(sb)){ double d; memcpy(&d,sb.data(),8); char x[40]; snprintf(x,sizeof x,"%g",d); double rt=0; sscanf(x,"%lf",&rt); if(rt!=d)snprintf(x,sizeof x,"%.17g",d); ty="double"; val=x; }
                else if(sb.size()>=4){ ty="int"; val=std::to_string((int32_t)(sb[0]|(sb[1]<<8)|(sb[2]<<16)|((uint32_t)sb[3]<<24))); }
                if(!ty.empty()){ int rn=++roleCount["k"]; string gn="k"+(rn>1?std::to_string(rn):""); strRepl["*"+gx]=gn; strRepl[gx]="&"+gn;
                    dataDecls.push_back({gn,"static const "+ty+" "+gn+" = "+val+";"}); continue; } }
            if(dest.empty() && stride==0 && looksStringPool(b)){ int rn=++roleCount["chars"]; string gn="chars"+(rn>1?std::to_string(rn):"");
                strRepl[gx]=gn; dataDecls.push_back({gn,"static const char "+gn+"[] = "+poolStr(b)+";"}); continue; }
            string eltype="unsigned char"; int elsize=1; bool dbl=false;
            if(stride==8){ dbl=looksDouble(b); eltype=dbl?"double":"long"; elsize=8; }
            else if(stride==4){ eltype="int"; elsize=4; } else if(stride==2){ eltype="short"; elsize=2; } else if(stride==1){ eltype="unsigned char"; elsize=1; }
            else if(N%8==0 && looksDouble(b)){ eltype="double"; elsize=8; dbl=true; }
            else if(N%8==0 && looks8byte(b)){ eltype="long"; elsize=8; }
            else if(N%4==0){ eltype="int"; elsize=4; } else if(N%2==0){ eltype="short"; elsize=2; }
            if((long)b.size()%elsize!=0){ eltype="unsigned char"; elsize=1; dbl=false; }
            int count=(int)(b.size()/elsize); string vals=(elsize==1&&!dbl)?hexBytes(b):decVals(b,elsize,dbl);
            int sq=0; while(sq*sq<count)sq++;                          // readable names by role
            string role = (eltype=="int"&&count>1&&sq*sq==count)?"matrix":(eltype=="int"||eltype=="short")?"table":eltype=="double"?"coeffs":"data";
            int rn=++roleCount[role]; string sfx=rn>1?std::to_string(rn):"";
            string gname = dest.empty()? role+sfx : role+sfx+"_init", dname=role+sfx;
            strRepl[gx]=gname;
            dataDecls.push_back({gname,"static const "+eltype+" "+gname+"["+std::to_string(count)+"] = {\n    "+vals+"\n};"});
            if(!dest.empty()&&!destFn.empty()){ dataHint[destFn].push_back(dest+"="+eltype+"["+std::to_string(count)+"]"); destRename[destFn][dest]=dname; } }
    }
    void emitData(const string& fn, std::map<string,string>& vmap, const char* ind){ auto it=dataHint.find(fn); if(it==dataHint.end()||it->second.empty())return; string s=string(ind)+"// @data";
        for(auto& d:it->second){ size_t eq=d.find('='); string nm=d.substr(0,eq); string mapped=vmap.count(nm)?vmap[nm]:nm; s+=" "+mapped+"="+d.substr(eq+1); } printf("%s\n",s.c_str()); }
    string humanizeStr(string s){ for(auto& kv:strRepl) s=replaceAll(s,kv.first,kv.second); return s; }

    // Synthesize C++ from all lifted functions: merge compatible arg0-structs into classes
    // (a function whose `this` is struct T becomes a method of class T), rest stay free functions.
    // ── readability passes on a function's final statement lines ─────────────
    static bool isTmpName(const string& t){ if(t.size()<2||(t[0]!='v'&&t[0]!='t'))return false; for(size_t i=1;i<t.size();i++) if(!isdigit((unsigned char)t[i]))return false; return true; }
    static int countTok(const vector<string>& body, const string& name){ int c=0; for(auto& l:body){ size_t p=0; while((p=l.find(name,p))!=string::npos){ bool lb=(p==0||!isIdent(l[p-1])), rb=(p+name.size()>=l.size()||!isIdent(l[p+name.size()])); if(lb&&rb)c++; p+=name.size(); } } return c; }
    static string leadWS(const string& l){ size_t a=l.find_first_not_of(" "); return a==string::npos?"":l.substr(0,a); }
    void cleanupBody(vector<string>& body){    // dead-store/temp elimination + single-use temp inlining
        for(int round=0; round<8; round++){ bool changed=false;
            for(size_t i=0;i<body.size();i++){
                string ind=leadWS(body[i]); string s=trim(body[i]);
                size_t eq=s.find(" = "); if(eq==string::npos||s.empty()||s.back()!=';') continue;
                string lhs=trim(s.substr(0,eq)); string rhs=trim(s.substr(eq+3)); if(!rhs.empty()&&rhs.back()==';')rhs.pop_back(); rhs=trim(rhs);
                if(!isTmpName(lhs)) continue;
                bool isT=lhs[0]=='t'; int total=countTok(body,lhs), uses=total-1; bool hasCall=rhs.find('(')!=string::npos;
                if(uses==0){ if(hasCall){ body[i]=ind+rhs+";"; changed=true; } else if(isT||total==1){ body.erase(body.begin()+i); i--; changed=true; } continue; }
                if(isT && uses==1){ int u=-1; for(size_t j=i+1;j<body.size();j++){ if(countTok({body[j]},lhs)>0){ u=(int)j; break; } }
                    if(u<0) continue; bool safe=true; for(int j=(int)i+1;j<u;j++){ if(body[j].find('{')!=string::npos||body[j].find('}')!=string::npos){safe=false;break;} }
                    if(!safe) continue; string inl=replaceTok(body[u],lhs,"("+rhs+")"); if(trim(inl).size()>118) continue;
                    body[u]=inl; body.erase(body.begin()+i); i--; changed=true; continue; }
            }
            if(!changed) break;
        }
    }
    void printBody(vector<string> lines, const char* ind){ cleanupBody(lines); for(auto& l:lines) printf("%s%s\n", ind, l.c_str()); }

    void emitAll(){
        buildStrings();
        for(auto& d:strDecls) printf("const char* %s = \"%s\";\n", d.first.c_str(), d.second.c_str());
        if(!strDecls.empty()) printf("\n");
        buildDataTemplates();                                          // real data globals (arrays/blobs) -> initialized C
        for(auto& d:dataDecls) printf("%s\n", d.second.c_str());
        if(!dataDecls.empty()) printf("\n");
        struct Cls { string name; std::map<int,int> fields; vector<int> methods; };
        vector<Cls> classes; vector<int> clsOf(fns.size(),-1);
        auto compat=[](std::map<int,int>& a, std::map<int,int>& b){ for(auto& p:b){ auto it=a.find(p.first); if(it!=a.end()&&it->second!=p.second) return false; } return true; };
        for(size_t i=0;i<fns.size();i++){ FnResult& F=fns[i];
            if(F.a0base.empty()||!F.fields.count(F.a0base)) continue;
            auto& S=F.fields[F.a0base]; int ci=-1;
            for(size_t c=0;c<classes.size();c++) if(compat(classes[c].fields,S)){ ci=(int)c; break; }
            if(ci<0){ ci=(int)classes.size(); classes.push_back({"Cls"+std::to_string(ci),{},{}}); }
            for(auto& p:S) classes[ci].fields[p.first]=p.second;        // union the layout
            classes[ci].methods.push_back((int)i); clsOf[i]=ci;
        }
        int recN=0; set<string> usedCls;
        for(auto& C:classes){
            vector<std::pair<string,const vector<string>*>> ctx; vector<string> mnames;
            for(int mi:C.methods){ ctx.push_back({fns[mi].a0base,&fns[mi].body}); mnames.push_back(fns[mi].name); }
            std::map<int,string> fn, fty; fieldRoles(C.fields,ctx,fn,fty);                       // best-guess names + C types
            string fallback="Record"+std::to_string(recN); string guess=guessTypeName(fn,mnames,fallback); if(guess==fallback)recN++;
            string cn=guess; for(int s=2; usedCls.count(cn); s++) cn=guess+std::to_string(s); usedCls.insert(cn); C.name=cn;
            std::map<string,string> fmap; for(auto& fo:C.fields){ fmap[fieldId(fo.first)]=fn[fo.first]; }
            printf("class %s {\npublic:\n", C.name.c_str());
            emitStructBody(C.fields, fn, &fty, C.name);
            for(int mi:C.methods){ FnResult& F=fns[mi];
                auto vmap=nameVars(F.body,F.maxArg);
                string params; for(int k=1;k<=F.maxArg;k++){ if(k>1) params+=", "; string ab="a"+std::to_string(k); params+="long "+(vmap.count(ab)?vmap[ab]:("arg"+std::to_string(k))); }
                printf("    long %s(%s) {\n", F.name.c_str(), params.c_str());
                emitData(F.name,vmap,"    ");
                vector<string> lines; for(auto& l:F.body){ string t=replaceTok(l,F.a0base,"this"); if(trim(t)=="this = a0;") continue; lines.push_back(applyMap(applyMap(humanizeStr(t),fmap),vmap)); }
                printBody(lines,"    ");
                printf("    }\n");
            }
            printf("};\n\n");
        }
        // skip junk one-field structs synthesized from call temps (tN) or raw registers (rax…)
        auto isArtifactBase=[](const string& s){
            if(s.size()>=2 && s[0]=='t'){ bool d=true; for(size_t i=1;i<s.size();i++) if(!isdigit((unsigned char)s[i])){d=false;break;} if(d) return true; }
            static const char* R[]={"rax","rbx","rcx","rdx","rsi","rdi","rbp","rsp","r8","r9","r10","r11","r12","r13","r14","r15",0};
            for(int i=0;R[i];i++) if(s==R[i]) return true; return false; };
        auto isJunkBase=[](const string& b){ static const set<string> J={"cin","cout","cerr","clog","wcin","wcout","wcerr","wclog","stdscr","curscr","stdin","stdout","stderr"}; return J.count(b)>0||b.rfind("__",0)==0||b.rfind("operator",0)==0; };
        auto realBase=[&](const string& b,std::map<int,int>& f){ if(isJunkBase(b))return false; return !isArtifactBase(b) || f.size()>=2; };
        // ── CROSS-FUNCTION STRUCT UNIFICATION ── identical layouts share ONE name + field names, emitted once
        // (decl precedes every use since functions print in order). Shared name = behavioural guess or StructN.
        std::map<string,std::tuple<string,std::map<int,string>,std::map<int,string>>> byLayout; int structN=0;
        auto layoutKey=[](std::map<int,int>& f){ string k; for(auto& p:f) k+=std::to_string(p.first)+","+std::to_string(p.second)+";"; return k; };
        for(size_t i=0;i<fns.size();i++){ if(clsOf[i]>=0) continue; FnResult& F=fns[i];
            auto vmap=nameVars(F.body,F.maxArg); mergeDataNames(F.name,vmap);   // a0 now role-named by nameVars
            std::map<string,std::map<int,string>> bNames; std::map<string,string> baseStruct;
            for(auto& kv:F.fields){ if(!realBase(kv.first,kv.second))continue; string lk=layoutKey(kv.second);
                if(!byLayout.count(lk)){ std::map<int,string> nm,ty; vector<std::pair<string,const vector<string>*>> ctx{{kv.first,&F.body}}; fieldRoles(kv.second,ctx,nm,ty);
                    string fb="Struct"+std::to_string(structN); string g=guessTypeName(nm,{},fb); if(g==fb)structN++;
                    string cn=g; for(int s=2; usedCls.count(cn); s++)cn=g+std::to_string(s); usedCls.insert(cn);
                    printf("struct %s {\n",cn.c_str()); emitStructBody(kv.second,nm,&ty,cn); printf("};\n");
                    byLayout[lk]=std::make_tuple(cn,nm,ty); }
                bNames[kv.first]=std::get<1>(byLayout[lk]); baseStruct[kv.first]=std::get<0>(byLayout[lk]); }
            vector<string> lines; if(F.name=="main"){ vmap["a0"]="argc"; vmap["a1"]="argv"; }
            for(auto& l:F.body){ string t=humanizeStr(l);
                for(auto& bn:bNames){ const string& base=bn.first; auto& nm=bn.second;
                    for(auto& fo:F.fields[base]) t=replaceTok(t, base+"->"+fieldId(fo.first), base+"->"+nm[fo.first]); }
                lines.push_back(applyMap(t,vmap)); }
            auto emitTypes=[&](const char* ind){ string ts; for(auto& b:baseStruct){ string v=vmap.count(b.first)?vmap[b.first]:b.first; ts+=" "+v+"="+b.second; } if(!ts.empty())printf("%s// @types%s\n",ind,ts.c_str()); };   // var -> struct name (collapse declares it `struct X*`)
            if(F.name=="main"){ printf("int main(int argc, char** argv) {\n"); emitData(F.name,vmap,""); emitTypes(""); printBody(lines,""); printf("}\n\n"); continue; }
            // TYPE PROPAGATION: an arg with a synthesized struct (used as argK->fN) is that struct's pointer.
            string params; for(int k=0;k<=F.maxArg;k++){ if(k) params+=", "; string ab="a"+std::to_string(k);
                string pn = vmap.count(ab)? vmap[ab] : ("arg"+std::to_string(k));   // role-based param name
                if(baseStruct.count(ab)) params += "struct "+baseStruct[ab]+"* "+pn;
                else params += "long "+pn; }
            printf("long %s(%s) {\n", F.name.c_str(), applyMap(params,vmap).c_str());   // every function gets a return type
            emitData(F.name,vmap,""); emitTypes("");
            printBody(lines,"");
            printf("}\n\n");
        }
    }

    // emit blocks [lo,hi); recover while + if/else. `cont`=enclosing loop header,
    // `brk`=reconvergence/exit addr — terminal gotos to either are structural, so suppressed.
    void emit(int lo,int hi,int ind,uint64_t cont,uint64_t brk,uint64_t loopBrk=0){
        if(ind>200){                                                  // RECURSION-DEPTH GUARD: pathological/irreducible CFG -> flat goto emission (valid, just unstructured)
            for(int k=lo;k<hi;k++){ Block& B=blocks[k]; if(B.dead)continue;
                if(B.isTarget)label(ind,B.addr); emitStmts(B,ind);
                if(B.term==Block::RET)say(ind,"return "+B.ret+";");
                else if(B.term==Block::GOTO){ if(B.tgtT){ say(ind,"goto "+hx(B.tgtT)+";"); used.insert(B.tgtT); } }
                else if(B.term==Block::COND){ say(ind,"if ("+B.cond+") goto "+hx(B.tgtT)+";"); used.insert(B.tgtT); } }
            return; }
        int i=lo;
        while(i<hi){
            if(blocks[i].dead){ i++; continue; }
            Block& B=blocks[i];
            int latch=-1; for(int j=i;j<hi;j++){ Block& C=blocks[j];
                if(((C.term==Block::GOTO||C.term==Block::COND)&&BI(C.tgtT)==i)) latch=j; }
            if(latch>=0 && B.term==Block::COND){
                int tT=BI(B.tgtT), tF=BI(B.tgtF);
                int exitIdx = (tT>latch||tT<lo)? tT : (tF>latch||tF<lo)? tF : latch+1;
                bool bodyIsFall = (tF>=lo && tF<=latch);
                uint64_t exitAddr = (exitIdx>=0&&exitIdx<(int)blocks.size())? blocks[exitIdx].addr : brk;
                if(B.isTarget) label(ind,B.addr);
                emitStmts(B,ind);
                say(ind,"while ("+(bodyIsFall?negate(B.cond):B.cond)+") {");
                emit(i+1, latch+1, ind+1, B.addr, 0, exitAddr);   // body; cont=header, loopBrk=loop exit
                say(ind,"}");
                i = (exitIdx>i?exitIdx:latch+1); continue;   // forward-progress guard: a backward 'exit' would re-process the loop forever
            }
            if(latch>=0 && latch>i && B.term!=Block::COND && blocks[latch].term==Block::COND && BI(blocks[latch].tgtT)==i){   // do { body } while(cond) — test at the bottom
                Block& Lt=blocks[latch]; uint64_t exitAddr=Lt.tgtF; int exitIdx=BI(exitAddr);
                say(ind,"do {"); emit(i, latch, ind+1, B.addr, 0, exitAddr);
                emitStmts(Lt,ind+1); say(ind,"} while ("+Lt.cond+");");
                i = (exitIdx>i?exitIdx:latch+1); continue;
            }
            if(B.term==Block::COND){
                int tT=BI(B.tgtT), tF=BI(B.tgtF);
                if(tT>i && tF==i+1 && tT<=hi){
                    Block& last=blocks[tT-1]; int elseEnd=-1;
                    if(last.term==Block::GOTO){ int J=BI(last.tgtT); if(J>tT && J<=hi) elseEnd=J; }
                    int joinIdx = elseEnd>=0? elseEnd : tT;
                    uint64_t joinAddr = (joinIdx<(int)blocks.size())? blocks[joinIdx].addr : brk;
                    if(B.isTarget) label(ind,B.addr);
                    emitStmts(B,ind);
                    say(ind,"if ("+negate(B.cond)+") {");
                    emit(i+1, tT, ind+1, cont, joinAddr, loopBrk);
                    if(elseEnd>=0){ say(ind,"} else {"); emit(tT, elseEnd, ind+1, cont, joinAddr, loopBrk); say(ind,"}"); i=elseEnd; }
                    else { say(ind,"}"); i=tT; }
                    continue;
                }
            }
            if(B.isTarget) label(ind,B.addr);
            emitStmts(B,ind);
            if(B.term==Block::RET) say(ind,"return "+B.ret+";");
            else if(B.term==Block::GOTO){ uint64_t t=B.tgtT; int ti=BI(t);   // recover break/continue; only a SEQUENTIAL fall is silent
                if(!t||ti==i+1){}
                else if(loopBrk&&t==loopBrk) say(ind,"break;");
                else if(cont&&t==cont){ if(i+1<hi) say(ind,"continue;"); }
                else if(t==brk||(ti>=0&&ti==hi)){}
                else { say(ind,"goto "+hx(t)+";"); used.insert(t); } }
            else if(B.term==Block::COND){ uint64_t t=B.tgtT;
                if(loopBrk&&t==loopBrk) say(ind,"if ("+B.cond+") break;");
                else if(cont&&t==cont) say(ind,"if ("+B.cond+") continue;");
                else { say(ind,"if ("+B.cond+") goto "+hx(t)+";"); used.insert(t); } }
            i++;
        }
    }
};

// JSON-escape a string for the --records sidecar
static string jstr(const string& s){ string o="\""; for(char c:s){ unsigned char u=(unsigned char)c;
    if(c=='"'||c=='\\'){ o+='\\'; o+=c; } else if(c=='\n') o+="\\n"; else if(c=='\t') o+="\\t"; else if(c=='\r') o+="\\r";
    else if(u<0x20){ char b[8]; snprintf(b,sizeof b,"\\u%04x",u); o+=b; } else o+=c; } o+="\""; return o; }

int main(int argc,char** argv){
    const char* path=nullptr; uint64_t from=0,to=0; bool nosym=false, keepAll=false; const char* recPath=nullptr;
    for(int i=1;i<argc;i++){ string a=argv[i];
        if(a=="--from"&&i+1<argc) from=strtoull(argv[++i],0,0);
        else if(a=="--to"&&i+1<argc) to=strtoull(argv[++i],0,0);
        else if(a=="--nosym") nosym=true;     // ignore the symbol table -> sub_<addr> names (stripped-binary view; training input)
        else if(a=="--all") keepAll=true;     // decompile libc++/runtime bodies too (don't skip) — keeps names
        else if(a=="--records"&&i+1<argc) recPath=argv[++i];   // emit per-function JSONL corpus records (sidecar)
        else path=argv[i]; }
    if(!path){ fprintf(stderr,"usage: nxlift <mach-o> [--from 0x..] [--to 0x..]\n"); return 2; }
    vector<uint8_t> f; if(!readFile(path,f)){ perror("open"); return 1; }
    machoSelectSlice(f, 0x01000007);   // universal binary -> pick the x86_64 slice
    // arch guard — this decoder is x86-64 only. Decoding e.g. arm64 bytes as x86
    // produces pages of `.byte` garbage, so refuse clearly instead.
    if(f.size()>=8){
        uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24);
        uint32_t cpu  =f[4]|(f[5]<<8)|(f[6]<<16)|((uint32_t)f[7]<<24);
        if((magic==0xFEEDFACFu||magic==0xFEEDFACEu) && cpu!=0x01000007u){
            const char* a = cpu==0x0100000Cu?"arm64" : cpu==7?"i386" : cpu==12?"arm" : "non-x86-64";
            fprintf(stderr,
              "ember-lift: this is an %s Mach-O — the decoder currently supports x86-64 only.\n"
              "  Apple-silicon Macs build arm64 by default. For an x86-64 object to try it on:\n"
              "      clang -arch x86_64 -c file.c -o file.o\n", a);
            return 3;
        }
        if(magic==0xCAFEBABEu||magic==0xBEBAFECAu){
            fprintf(stderr,"ember-lift: this is a universal (fat) binary; extract a thin x86-64 slice first:\n"
                           "      lipo '%s' -thin x86_64 -output slice.o\n", path);
            return 3;
        }
    }
    size_t off,size; uint64_t base; bool isPE=false;
    if(!machoText(f.data(),f.size(),off,size,base)){                                  // Mach-O? else try a Windows PE (.exe) — decompile exes on any host
        if(peText(f.data(),f.size(),off,size,base)){ isPE=true; fprintf(stderr,"ember-lift: PE/.exe x86-64, .text @ 0x%llx\n",(unsigned long long)base); }
        else { fprintf(stderr,"ember-lift: need a Mach-O or PE (x86-64)\n"); return 1; } }
    Lifter L; if(!nosym && !isPE){ machoSymbols(f.data(),f.size(),L.syms); machoStubs(f.data(),f.size(),L.stubs); } L.file=&f; machoSections(f.data(),f.size(),L.secs); L.scan(f.data()+off,size,base);
    vector<uint64_t> starts(L.funcs.begin(),L.funcs.end());
    int kept=0,skipped=0;
    FILE* recFP = recPath? fopen(recPath,"w"):nullptr;       // per-function corpus records (sidecar to stdout)
    for(size_t s=0;s<starts.size();s++){
        uint64_t a=starts[s], b=s+1<starts.size()?starts[s+1]:base+size;
        if(from&&(a<from||a>=(to?to:base+size))) continue;
        if(!L.at.count(a)) continue;
        if(!nosym && !keepAll && L.isLibrary(a)){ skipped++; continue; }   // skip libc++/runtime bodies (--all keeps them)
        fprintf(stderr,"  decompiling %s\n", L.name(a).c_str()); kept++;
        L.liftFn(L.at[a], L.at.count(b)?L.at[b]:L.ins.size());
        if(recFP && !L.fns.empty()){ auto& F=L.fns.back();
            auto bytes=L.readBytes(a,(size_t)(b-a)); string hex; for(unsigned char c:bytes){ char t[3]; snprintf(t,sizeof t,"%02x",c); hex+=t; }
            size_t bi=L.at[a], ei=L.at.count(b)?L.at[b]:L.ins.size(); string dis="[";
            for(size_t k=bi;k<ei&&k<L.ins.size();k++){ Ins& in=L.ins[k]; char ah[24]; snprintf(ah,sizeof ah,"0x%llx: ",(unsigned long long)in.addr);
                dis += (k>bi?",":"") + jstr(string(ah)+fmtIns(in)); } dis+="]";
            string body; for(auto& l:F.body){ if(!body.empty())body+="\n"; body+=l; }
            string nm=F.name, lab;
            if(!nosym){ string dn=demangleX(F.name); size_t par=dn.find('('); if(par!=string::npos)dn=dn.substr(0,par);   // drop (args)
                size_t cc=dn.rfind("::"); if(cc!=string::npos)dn=dn.substr(cc+2); size_t sp=dn.rfind(' '); if(sp!=string::npos)dn=dn.substr(sp+1);  // drop ns + ret type
                nm=dn; size_t up=dn.find("__"); lab = up==string::npos? dn : dn.substr(0,up); }   // label = intent (before "__type_seed")
            fprintf(recFP,"{\"addr\":\"0x%llx\",\"name\":%s,\"label\":%s,\"size\":%llu,\"bytes_hex\":\"%s\",\"disasm\":%s,\"pseudocode\":%s,\"arch\":\"x86_64\"}\n",
                (unsigned long long)a, jstr(nm).c_str(), jstr(lab).c_str(), (unsigned long long)(b-a), hex.c_str(), dis.c_str(), jstr(body).c_str()); }
    }
    if(recFP) fclose(recFP);
    fprintf(stderr,"ember-lift: %d user function(s) decompiled, %d library function(s) skipped\n",kept,skipped);
    L.emitAll();
    return 0;
}
