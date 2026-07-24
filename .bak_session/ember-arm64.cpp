// ember-arm64 — lift decoded AArch64 (ARM64) -> readable C-ish pseudocode with
// control-flow structuring. The companion to ember-lift (x86-64). AArch64 is a
// fixed-width ISA (every instruction is 4 bytes), so decoding is bit-field
// pattern matching. We emit the SAME statement vocabulary as ember-lift
// (vN locals, ptr->fN fields, g_<addr> globals, tN call temps, a0.. args) so the
// shared structuring / struct+class / string-recovery passes work unchanged.
//
// build:  clang++ -std=c++17 -O2 ember-arm64.cpp -o ember-arm64
// use:    ember-arm64 <arm64 mach-o> [--from 0x.. --to 0x..] [--nosym]
#include "ember.h"
#include <map>
#include <set>
#include <algorithm>
#include <cxxabi.h>      // host tool (runs on macOS) -> demangle C++ symbols
using namespace nx;
using std::string; using std::vector; using std::map; using std::set;

// fallback when __cxa_demangle fails (mingw libc++ gap): first real length-prefixed Itanium identifier
static string demangleFallback(const string& s){
    for(size_t p=0;p<s.size();){
        if(isdigit((unsigned char)s[p])){ long len=0; size_t q=p; while(q<s.size()&&isdigit((unsigned char)s[q])){ len=len*10+(s[q]-'0'); q++; }
            if(len>0 && q+(size_t)len<=s.size()){ string id=s.substr(q,(size_t)len);
                if(!id.empty() && (isalpha((unsigned char)id[0])||id[0]=='_') && id.rfind("__",0)!=0) return id;
                p=q+(size_t)len; continue; }
            p=q; } else p++; }
    return s;
}
static string demangle(const string& s){
    if(s.rfind("_Z",0)!=0 && s.rfind("__Z",0)!=0) return s;
    int st=0; char* d=abi::__cxa_demangle(s.c_str(),0,0,&st);
    if(st==0&&d){ string r=d; free(d); return r; }
    if(d) free(d); return demangleFallback(s);
}
// AArch64 logical-immediate (bitmask) decode — for ORR/AND/EOR #imm and MOV bitmask.
static bool bitmask(int N,int imms,int immr,int datasize,uint64_t& out){
    int x=((N&1)<<6)|((~imms)&0x3f); if(x==0) return false;
    int len=31-__builtin_clz((unsigned)x); if(len<1) return false;
    int levels=(1<<len)-1; int S=imms&levels, R=immr&levels; int esize=1<<len;
    if(S==levels) return false;
    uint64_t welem = (S+1>=64)? ~0ull : ((1ull<<(S+1))-1);
    uint64_t rot = R? ((welem>>R)|(welem<<(esize-R))) : welem;
    if(esize<64) rot &= (esize==64?~0ull:((1ull<<esize)-1));
    uint64_t res=0; for(int i=0;i<datasize;i+=esize) res|=rot<<i;
    if(datasize<64) res&=((1ull<<datasize)-1);
    out=res; return true;
}
static int64_t sext(uint64_t v,int bits){ uint64_t m=1ull<<(bits-1); return (int64_t)((v^m)-m); }

struct Block {
    uint64_t addr=0; size_t bi=0, be=0;
    vector<string> stmts;
    enum { FALL, GOTO, COND, RET } term=FALL;
    string cond, ret; uint64_t tgtT=0, tgtF=0;
    bool isTarget=false, dead=false, inlineable=false;   // inlineable: small return-leaf reached only by jumps -> tail-dup into each jump site, drop the orphan
    string regdef[32];            // cross-block dataflow: exit value-expr of reg r, if (re)defined to a non-trivial expr
    int regdefSeq[32]={0};        // program order of each reg's last def -> materialize in THIS order (data deps like `hash += c; c = *s++`)
    uint32_t useBare=0, defSet=0; // useBare: reg read-before-local-def (a cross-block input) · defSet: reg given a non-trivial value
};

static string neonDisasm(uint32_t w);   // fwd: NEON mnemonic recovery (used by the lifter to show vectorized ops)

struct Lifter {
    struct AI { uint64_t addr; uint32_t w; };
    vector<AI> ins; map<uint64_t,size_t> at;
    set<uint64_t> funcs, jtargets;
    std::unordered_map<uint64_t,string> syms;
    std::unordered_map<uint64_t,string> stubs;     // import stubs + GOT slots -> symbol name (printf, ~basic_string, ...)
    const vector<uint8_t>* file=nullptr; vector<Section> secs;

    string reg[32]; uint64_t imm[32]; bool immok[32]; set<int> argsSet; int tmp=0;
    string neonKey, neonBuf;                 // a NEON constant-compare: bytes it reads from rodata + the stack buffer it reads from
    string neonStr[32];                      // FP/SIMD reg -> the rodata string constant last loaded into it (for NEON-assembled stack buffers)
    int64_t neonSrcVa[32]; int neonSrcLen[32];   // FP/SIMD reg -> rodata DATA constant (numeric array/struct init): source addr + byte len, materialized as g_<va> on store
    bool neonPend=false; int64_t neonPendStart=0; size_t neonPendIdx=0; string neonPendBuf;   // a contiguous run of NEON string stores -> one memcpy buffer
    bool spok[32]; int64_t spoff[32];        // reg = sp + spoff (stack-address alias) -> [reg+off] is a local
    bool fpSet=false; int64_t fpOff=0;       // x29 frame-pointer alias, established once in the prologue, kept across blocks
    string lastL,lastR; bool testMode=false;

    // demangled C++ signatures are huge; reduce to a readable call/decl token
    // e.g. "std::__1::basic_ostream<..>& std::__1::operator<<[abi:x]<..>(..)" -> "operator<<"
    static string shortName(const string& full){
        size_t op=full.find("operator");                                 // operator<< / == / new — keep the symbol, don't eat it as <template>
        if(op!=string::npos){ size_t i=op+8; string sym; static const string OPC="<>=!+-*/%&|^~";
            while(i<full.size()&&full[i]==' ')i++;
            if(i<full.size()&&isalpha((unsigned char)full[i])){ while(i<full.size()&&isalpha((unsigned char)full[i]))sym+=full[i++]; return "operator_"+sym; }
            if(i+1<full.size()&&full[i]=='['&&full[i+1]==']') return "operator_index";   // subscript -> LEGAL identifier
            if(i+1<full.size()&&full[i]=='('&&full[i+1]==')') return "operator_call";   // call -> LEGAL identifier (was "operator()", illegal as a fn/struct name)
            while(i<full.size()&&OPC.find(full[i])!=string::npos)sym+=full[i++];
            return "operator"+sym; }
        string s; int d=0; for(char c:full){ if(c=='<'){d++;continue;} if(c=='>'){if(d)d--;continue;} if(!d)s+=c; } // drop <templates>
        int depth=0,open=-1; for(int i=(int)s.size()-1;i>=0;i--){ char c=s[i]; if(c==')')depth++; else if(c=='('){ if(--depth==0){open=i;break;} } }
        if(open>=0) s=s.substr(0,open);                                  // drop the (arg list)
        while(!s.empty()&&s.back()==' ')s.pop_back();
        size_t p=s.rfind("::"); string nm = p==string::npos? s : s.substr(p+2);   // after last ::
        size_t sp=nm.rfind(' '); if(sp!=string::npos) nm=nm.substr(sp+1);          // drop return type
        size_t b=nm.find('['); if(b!=string::npos) nm=nm.substr(0,b);              // drop [abi:..] tag
        if(!nm.empty()&&nm[0]=='~') nm=nm.substr(1)+"_dtor";                        // destructor ~Foo -> Foo_dtor (legal C identifier, not folded)
        return nm.empty()? full : nm;
    }
    string fullName(uint64_t a){ auto st=stubs.find(a); if(st!=stubs.end()) return demangle(st->second); auto it=syms.find(a); if(it!=syms.end()) return demangle(it->second); char b[24]; snprintf(b,sizeof b,"sub_%llx",(unsigned long long)a); return b; }
    string name(uint64_t a){ return shortName(fullName(a)); }
    // Library/runtime symbol? Its BODY is libc++/compiler boilerplate the compiler inlined into the
    // binary, not the user's code — we keep it resolvable as a CALL target but never decompile it.
    // (A 13-line program pulls in ~47 std::__1 template instantiations; emitting them is the "slop".)
    bool isLibrary(uint64_t a){
        if(stubs.count(a)) return true;                               // an import stub -> never lift its body, keep as a named call
        auto it=syms.find(a); if(it==syms.end()) return false;        // unnamed (sub_..) -> treat as user
        const string& m=it->second;
        if(m=="main") return false;
        // Judge by the function's OWN mangled-name prefix, NOT by std:: appearing anywhere in the
        // signature — a user fn like xorString(std::string&, char) has std:: in its PARAMS but is
        // user code. Its mangled name is _Z9xorString..; a libc++ method is _ZNSt..  (St = std).
        string core=m; size_t u=core.find_first_not_of('_'); core = u==string::npos? "" : core.substr(u);  // reader strips one leading '_'; drop all
        static const char* RT[]={"clang_call_terminate","cxa_","Unwind_","gxx_personality","cxx_global",
                                 "GLOBAL__","mh_execute_header",
                                 "ZNSt","ZNKSt","ZNVSt","ZSt","ZNSa","ZNKSa",           // std:: namespace (St = std)
                                 "ZN9__gnu_cxx","ZNK9__gnu_cxx","ZN10__cxxabiv",         // libstdc++/abi internals
                                 "ZGV","ZTV","ZTI","ZTS","ZTC","ZTT",                    // guard vars, vtables, typeinfo
                                 "Znw","Zna","Zdl","Zda"};                               // operator new/new[]/delete/delete[]
        for(auto r:RT) if(core.rfind(r,0)==0) return true;
        return false;
    }
    static string imms(int64_t v){ char b[32]; if(v>-1024&&v<1000000) snprintf(b,sizeof b,"%lld",(long long)v); else snprintf(b,sizeof b,"0x%llx",(unsigned long long)v); return b; }
    static string slot(int64_t d){ char b[24]; snprintf(b,sizeof b,"v%lld",(long long)(d<0?-d:d)); return b; }
    // struct field id for offset `off` -> LEGAL C++ identifier ("f24"; negative offsets, from vtable-relative
    // access, become "fm24" not "f-24" which is a syntax error). Used by both the access (memref) and the decl.
    static string fieldId(int off){ char b[16]; if(off<0) snprintf(b,sizeof b,"fm%d",-off); else snprintf(b,sizeof b,"f%d",off); return b; }
    static bool isName(const string& s){ if(s.empty()||!(isalpha((unsigned char)s[0])||s[0]=='_'))return false; for(char c:s) if(!(isalnum((unsigned char)c)||c=='_'))return false; return true; }
    static const char* tyOf(int w){ return w==1?"char":w==2?"short":w==4?"int":"long"; }
    // printf/scanf-family: on Apple AArch64 the VARIADIC args are passed on the stack ([sp+0],[sp+8],…),
    // not in x1.., so the register-based arg collector misses them. We recover them from the arg-area stores.
    static bool isVariadic(const string& nm){ return nm.find("printf")!=string::npos||nm.find("scanf")!=string::npos; }
    // ── cross-block dataflow instrumentation ──
    Block* curB=nullptr; uint32_t curLocalDef=0;     // the block currently being lifted + which regs it has (re)defined
    int defSeq[32]={0}, seqCtr=0;                     // program order of each reg's last (re)definition in the current block
    void noteRead(int r){                            // a read of reg r while it's still its entry-seed name = a cross-block input
        if(!curB||r==31||(curLocalDef&(1u<<r)))return; const string& s=reg[r];
        if(s=="x"+std::to_string(r)||(r<8&&s=="a"+std::to_string(r))) curB->useBare|=(1u<<r); }
    string Xr(int r){ noteRead(r); return r==31? string("0") : reg[r]; }
    void setX(int d,const string& e){ if(d==31)return; reg[d]=e; immok[d]=false; spok[d]=false; regSx[d]=false; if(d<8) argsSet.insert(d); curLocalDef|=(1u<<d); defSeq[d]=++seqCtr; }
    // snapshot reg[r] into a temp when it's a non-trivial expr about to be used 2+ times in ONE instruction
    // (e.g. `add h, h, h, lsl #4` = h + (h<<4)). Without this, an unrolled hash loop inlines `h` twice each step
    // -> the expression DOUBLES every iteration (exponential blowup). Referencing it once by name keeps it linear.
    string snap(int r){ if(r==31) return Xr(r); const string& s=reg[r];
        bool trivial = true; for(char c:s) if(!isIdent(c)){ trivial=false; break; }   // a bare identifier/number needs no temp
        if(trivial || !curB) return Xr(r);
        noteRead(r); string t="t"+std::to_string(tmp++); curB->stmts.push_back(t+" = "+s+";"); reg[r]=t; return t; }
    void setImm(int d,uint64_t v){ if(d==31)return; imm[d]=v; immok[d]=true; spok[d]=false; reg[d]=imms((int64_t)v); if(d<8)argsSet.insert(d); curLocalDef|=(1u<<d); defSeq[d]=++seqCtr; }
    std::map<string,std::map<int,int>> fields;
    std::map<string,int> slotW;          // stack-local name -> access width (4=int, 8=long, 1=char, 2=short) for type recovery
    std::map<string,int> elemHint;       // pointer var -> ELEMENT size (from a scaled indexed access `[idx, lsl #k]`); lets collapse fold `p[i<<k]` -> `p[i]`
    bool regSx[32];                      // reg holds a SIGN-EXTENDED 32-bit value (from ldrsw/sxtw) -> a 64-bit mul of these must be `(long)a*(long)b`, not 32-bit int*int
    std::map<int,int> sfieldW;           // INDEXED computed-pointer deref: field offset -> access width (for opt-in struct-array field typing)

    // memory operand [base + off] -> text; record stack locals / struct fields / globals
    string memref(int base,int64_t off,int width){
        if(base==31||(base==29&&fpSet)||spok[base]){ string s=slot((spok[base]?spoff[base]:0)+off); if(width>slotW[s])slotW[s]=width; return s; }   // sp / established-frame / stack-alias -> local vN
        if(immok[base]){ uint64_t ga=imm[base]+off; auto st=stubs.find(ga); if(st!=stubs.end()) return shortName(demangle(st->second));   // *GOT -> the import (std::cout, …)
            char b[24]; snprintf(b,sizeof b,"g_%llx",(unsigned long long)ga); return string("*")+b; }
        noteRead(base);                                                  // a pointer base from a predecessor is a cross-block input too
        string be=reg[base];
        if(isName(be)){ fields[be][(int)off]=width; return be+"->"+fieldId((int)off); }
        // an INDEXED computed pointer (`buf + (i<<4)`, `p + i*24`) = a struct-ARRAY element; record the
        // per-field access WIDTH so the opt-in struct-width pass can type fields exactly (not gap-guessed).
        if((be.find("<<")!=string::npos||be.find(" * ")!=string::npos) && off>=0 && off<4096){ int o=(int)off; if(!sfieldW.count(o)||width<sfieldW[o]) sfieldW[o]=width; }
        if(off) return "*("+be+" + "+imms(off)+")";
        return "*"+be;
    }
    // declared arg count for common libc/libc++ functions, so a cross-block call still shows its args
    static int callArity(const string& fn){
        string base=fn; size_t p=base.find('('); if(p!=string::npos) base=base.substr(0,p);
        size_t sp=base.find_last_of(": "); if(sp!=string::npos) base=base.substr(sp+1);
        static const std::map<string,int> A={
            {"puts",1},{"strlen",1},{"printf",1},{"perror",1},{"free",1},{"fflush",1},{"atoi",1},{"atol",1},
            {"system",1},{"exit",1},{"putchar",1},{"abort",0},{"strdup",1},{"fputs",2},{"strcmp",2},{"strcpy",2},
            {"strcat",2},{"strchr",2},{"strstr",2},{"strcspn",2},{"strspn",2},{"strrchr",2},{"atof",1},
            {"strtoul",3},{"strtol",3},{"memcmp",3},{"memcpy",3},{"memmove",3},{"memset",3},{"strncmp",3},
            {"strncpy",3},{"fgets",3},{"fwrite",4},{"fread",4},{"snprintf",3},{"fprintf",2} };
        auto it=A.find(base); return it!=A.end()? it->second : 0;
    }
    static const char* condOp(int c){ switch(c){             // ARM cond -> C comparison (vs the cmp operands)
        case 0:return "=="; case 1:return "!=";               // eq ne
        case 10:return ">="; case 11:return "<";              // ge lt (signed)
        case 12:return ">"; case 13:return "<=";              // gt le (signed)
        case 2:case 8:return ">="; case 3:case 9:return "<";  // hs/cs/hi-ish, lo/cc/ls-ish (unsigned approx)
        case 4:return "<"; case 5:return ">=";                // mi pl (vs 0)
        default:return "?"; } }
    static string negate(const string& c){
        static const char* P[][2]={{"==","!="},{"!=","=="},{"<=",">"},{">=","<"},{"<",">="},{">","<="}};
        size_t sp=c.find(' '); if(sp==string::npos)return "!("+c+")"; size_t sp2=c.find(' ',sp+1); if(sp2==string::npos)return "!("+c+")";
        string op=c.substr(sp+1,sp2-sp-1); for(auto&pr:P) if(op==pr[0]) return c.substr(0,sp+1)+pr[1]+c.substr(sp2); return "!("+c+")"; }

    // ── branch classification (used for funcs/leaders/terminators) ──
    enum BK{ NONE, B_UNC, B_COND, B_RET, B_CALL, B_IND };
    BK branchOf(uint32_t w,uint64_t addr,uint64_t& tgt,int& cond){
        if((w&0xFC000000u)==0x14000000u){ tgt=addr+(sext(w&0x3ffffff,26)<<2); return B_UNC; }       // B
        if((w&0xFC000000u)==0x94000000u){ tgt=addr+(sext(w&0x3ffffff,26)<<2); return B_CALL; }       // BL
        if((w&0xFF000010u)==0x54000000u){ tgt=addr+(sext((w>>5)&0x7ffff,19)<<2); cond=w&0xf; return B_COND; } // B.cond
        if((w&0x7E000000u)==0x34000000u){ tgt=addr+(sext((w>>5)&0x7ffff,19)<<2); cond=100+((w>>24)&1); return B_COND; } // CBZ/CBNZ
        if((w&0x7E000000u)==0x36000000u){ tgt=addr+(sext((w>>5)&0x3fff,14)<<2); cond=200+((w>>24)&1); return B_COND; }  // TBZ/TBNZ
        if((w&0xFFFFFC1Fu)==0xD65F0000u){ return B_RET; }                                            // RET
        if((w&0xFFFFFBFFu)==0xD65F0BFFu){ return B_RET; }                                            // RETAA/RETAB (arm64e pointer-auth return)
        if((w&0xFFFFFC1Fu)==0xD63F0000u){ return B_IND; }                                            // BLR
        if((w&0xFEFFF800u)==0xD63F0800u){ return B_IND; }                                            // BLRAA/BLRAB/BLRAAZ/BLRABZ (auth indirect call)
        if((w&0xFFFFFC1Fu)==0xD61F0000u){ tgt=0; return B_IND; }                                     // BR
        if((w&0xFEFFF800u)==0xD61F0800u){ tgt=0; return B_IND; }                                     // BRAA/BRAB/BRAAZ/BRABZ (auth indirect branch)
        return NONE;
    }

    void scan(const uint8_t* code,size_t sz,uint64_t base){
        for(size_t o=0;o+4<=sz;o+=4){ uint32_t w=code[o]|(code[o+1]<<8)|(code[o+2]<<16)|((uint32_t)code[o+3]<<24);
            at[base+o]=ins.size(); ins.push_back({base+o,w}); }
        for(auto& I:ins){ uint64_t t=0; int c=0; BK k=branchOf(I.w,I.addr,t,c);
            if(k==B_UNC||k==B_COND) jtargets.insert(t);
            else if(k==B_CALL && t!=I.addr) funcs.insert(t); }   // skip self-target bl (unrelocated .o call)
        for(auto& kv:syms) if(kv.first>=base && kv.first<base+sz) funcs.insert(kv.first);   // every symtab fn
        if(!ins.empty()) funcs.insert(ins.front().addr);
        // PROLOGUE-BASED starts: a frame-setup (sub sp,#imm / stp x29,x30,[sp] / pacibsp) immediately AFTER a
        // function terminator (ret / tail-`b` / brk / nop / zero padding) is a reliable function start — recovers
        // functions reached only by INDIRECT calls (vtable methods, fn-pointers) that no BL targets.
        auto isProlog=[](uint32_t w){ return (w&0xFF0003FFu)==0xD10003FFu /*sub sp,sp,#imm*/ || w==0xD503237Fu /*pacibsp*/
            || ((w&0x3E400000u)==0x28000000u && (w&0x1f)==29 && ((w>>10)&0x1f)==30 && ((w>>5)&0x1f)==31) /*stp x29,x30,[sp,..]*/; };
        auto isTerm=[](uint32_t w){ return w==0xD65F03C0u /*ret*/ || (w&0xFC000000u)==0x14000000u /*b (tail)*/
            || (w&0xFFE0001Fu)==0xD4200000u /*brk*/ || w==0xD503201Fu /*nop*/ || w==0u /*padding*/; };
        for(size_t k=1;k<ins.size();k++) if(isProlog(ins[k].w) && isTerm(ins[k-1].w)) funcs.insert(ins[k].addr);
    }

    // ── lift one basic block [bi,be) ──
    Block liftBlock(size_t bi,size_t be){
        Block B; B.bi=bi; B.be=be; B.addr=ins[bi].addr;
        curB=&B; curLocalDef=0; seqCtr=0; for(int i=0;i<32;i++)defSeq[i]=0;     // cross-block dataflow: track this block's reads/defs (+ program order)
        auto push=[&](const string& s){ B.stmts.push_back(s); };
        // outgoing variadic stack-arg area: offset -> (value stored, index of that store stmt in B.stmts)
        std::map<int64_t,std::pair<string,size_t>> outArg;
        // Outgoing variadic args live in the low sp window — but so do LOCAL home slots (clang spills `k =
        // classify()` to e.g. [sp,#0x28]). The distinguisher: a real vararg slot is WRITE-ONLY (the callee
        // reads it), whereas a local home slot ROUND-TRIPS — it is stored AND later loaded back. So we exclude
        // any offset that is loaded anywhere in the block (loadedOffs, below). Frame-pointer (x29) slots are
        // always locals.
        std::set<int64_t> loadedOffs;
        auto argStoreOff=[&](int base,int64_t off)->int64_t{
            if(base==29 && fpSet) return (int64_t)0x7fffffff;
            int64_t so = base==31? off : (spok[base]? spoff[base]+off : (int64_t)0x7fffffff);
            return (so>=0 && so<64)? so : (int64_t)0x7fffffff; };
        for(size_t k=bi;k<be;k++){ uint32_t w=ins[k].w; uint64_t addr=ins[k].addr, nx=addr+4;
            int Rd=w&0x1f, Rn=(w>>5)&0x1f, Rm=(w>>16)&0x1f;
            uint64_t bt=0; int bc=0; BK bk=branchOf(w,addr,bt,bc);
            // terminators
            if(bk==B_RET){ B.term=Block::RET; B.ret=reg[0]; continue; }
            if(bk==B_UNC){ B.term=Block::GOTO; B.tgtT=bt; continue; }
            if(bk==B_CALL){ int arity=callArity(name(bt));   // known libc funcs: also pass their declared args even when set in a PRIOR block (cross-block) — argsSet only tracks THIS block, so a `puts(x0)` after a branch would otherwise drop its arg
                string args; for(int j=0;j<8;j++) if(argsSet.count(j) || j<arity){ if(!args.empty())args+=", "; noteRead(j); args+=reg[j]; }
                if(isVariadic(name(bt))){                                   // append the stack-passed variadic args, drop their setup stores
                    vector<size_t> rm; for(auto& kv:outArg){ if(loadedOffs.count(kv.first)) continue;   // a slot that is also LOADED is a local home, not a vararg — skip it (kills the phantom `k`)
                        if(!args.empty())args+=", "; args+=kv.second.first; rm.push_back(kv.second.second); }
                    std::sort(rm.begin(),rm.end()); for(size_t i=rm.size();i-->0;) if(rm[i]<B.stmts.size()) B.stmts.erase(B.stmts.begin()+rm[i]); }
                string t="t"+std::to_string(tmp++); push(t+" = "+name(bt)+"("+args+");"); for(int i=0;i<8;i++){reg[i]="x"+std::to_string(i);immok[i]=false;} setX(0,t); reg[0]=t; argsSet.clear(); outArg.clear(); loadedOffs.clear(); continue; }
            if(bk==B_IND){ if((w&0xFFFFFC1Fu)==0xD63F0000u||(w&0xFEFFF800u)==0xD63F0800u){ string t="t"+std::to_string(tmp++); push(t+" = (*"+Xr(Rn)+")();"); setX(0,t); reg[0]=t; } else { B.term=Block::GOTO; B.tgtT=0; push("/* br "+Xr(Rn)+" (indirect) */"); } continue; }
            if(bk==B_COND){ B.term=Block::COND; B.tgtT=bt; B.tgtF=nx;
                if(bc>=200){ int bit=((w>>31)&1)<<5 | ((w>>19)&0x1f); B.cond="("+Xr(Rd)+" & (1<<"+std::to_string(bit)+")) "+(bc==200?"==":"!=")+" 0"; }
                else if(bc>=100){ B.cond=Xr(Rd)+(bc==100?" == 0":" != 0"); }
                else B.cond = testMode? lastL+" "+condOp(bc)+" 0" : lastL+" "+condOp(bc)+" "+lastR;
                continue; }
            // skip prologue/epilogue & no-ops
            if(w==0xD503201Fu||w==0xD503233Fu||w==0xD50323BFu||w==0xD503237Fu||w==0xD50323FFu) continue;  // nop/paciasp/autiasp/pacibsp/autibsp
            { uint32_t t=w>>10; if(t>=0x36B040u&&t<=0x36B051u) continue; }   // PACIA/PACIB/AUTIA/…/XPACI/XPACD (data-proc-1-src) — pointer value unchanged, skip
            // STP/LDP (load/store pair) — used for frame save/restore; skip when base is sp/fp, else render
            if((w&0x3A000000u)==0x28000000u){ int base=Rn, Rt2=(w>>10)&0x1f; bool load=(w>>22)&1;
                if(base==31||base==29){ continue; }                       // frame save/restore -> hide
                int sz3=(w>>31)&1?8:4; int64_t off=sext((w>>15)&0x7f,7)*sz3;
                if(load){ setX(Rd,memref(base,off,sz3)); setX(Rt2,memref(base,off+sz3,sz3)); }
                else { push(memref(base,off,sz3)+" = "+Xr(Rd)+";"); push(memref(base,off+sz3,sz3)+" = "+Xr(Rt2)+";"); }
                continue; }
            // ADD/SUB (immediate)  family bits[28:23]=100010
            if(((w>>23)&0x3f)==0x22){ bool sub=(w>>30)&1, S=(w>>29)&1, sh=(w>>22)&1; uint64_t i12=(w>>10)&0xfff; if(sh)i12<<=12;
                if(Rd==31&&Rn==31&&!S) continue;                                                      // add/sub sp,sp -> frame
                if(!S&&(Rn==31||(Rn==29&&fpSet)||spok[Rn])&&Rd!=31){                                  // &local: add/sub from sp OR the established x29 frame pointer
                    int64_t base = spok[Rn]?spoff[Rn] : (Rn==29&&fpSet?fpOff:0);
                    spok[Rd]=true; spoff[Rd]=base+(sub?-(int64_t)i12:(int64_t)i12); reg[Rd]="&"+slot(spoff[Rd]); immok[Rd]=false; if(Rd<8)argsSet.insert(Rd);
                    if(Rd!=29) curLocalDef|=(1u<<Rd);                                                  // record the &local alias as a cross-block def so it's materialized (not leaked as a bare vNNN)
                    if(Rd==29){ fpSet=true; fpOff=spoff[Rd]; }                                         // record where the frame pointer points (mov/add x29,sp,#k)
                    continue; }
                if(immok[Rn] && !sub && !S){ setImm(Rd, imm[Rn]+i12); char b[24]; snprintf(b,sizeof b,"g_%llx",(unsigned long long)imm[Rd]); reg[Rd]=b; immok[Rd]=true; continue; } // adrp+add -> &global
                if(S){ lastL=Xr(Rn); lastR=imms((int64_t)i12); testMode=false; }                      // ADDS/SUBS set flags -> seed compare
                if(Rd!=31) setX(Rd, "("+Xr(Rn)+(sub?" - ":" + ")+imms((int64_t)i12)+")"); continue; }
            // MOV wide (MOVZ/MOVN/MOVK) bits[28:23]=100101
            if(((w>>23)&0x3f)==0x25){ int opc=(w>>29)&3, hw=(w>>21)&3; uint64_t i16=(w>>5)&0xffff; int sh=hw*16;
                if(opc==2){ setImm(Rd, i16<<sh); }                                                    // MOVZ
                else if(opc==0){ setImm(Rd, ~(i16<<sh)); }                                            // MOVN
                else if(opc==3){ uint64_t v=immok[Rd]?imm[Rd]:0; v=(v&~(0xffffull<<sh))|(i16<<sh); setImm(Rd,v); } // MOVK
                continue; }
            // ADR / ADRP  bits[28:24]=10000
            if(((w>>24)&0x1f)==0x10){ uint64_t immlo=(w>>29)&3, immhi=(w>>5)&0x7ffff; int64_t v=sext((immhi<<2)|immlo,21);
                if((w>>31)&1){ setImm(Rd, (addr&~0xfffull)+(v<<12)); }                                 // ADRP page
                else setImm(Rd, addr+v);                                                              // ADR
                char b[24]; snprintf(b,sizeof b,"g_%llx",(unsigned long long)imm[Rd]); reg[Rd]=b; immok[Rd]=true; continue; }
            // Logical (immediate) bits[28:23]=100100
            if(((w>>23)&0x3f)==0x24){ int opc=(w>>29)&3, N=(w>>22)&1, immr=(w>>16)&0x3f, imms6=(w>>10)&0x3f; uint64_t v;
                bool ok=bitmask(N,imms6,immr,(w>>31)&1?64:32,v);
                const char* op=opc==0?"&":opc==1?"|":opc==2?"^":"&";
                if(opc==1&&Rn==31&&ok){ setImm(Rd,v); continue; }                                     // ORR Xd,XZR,#imm = MOV
                if(opc==3&&Rd==31){ lastL=Xr(Rn); lastR=ok?imms((int64_t)v):"?"; testMode=true; continue; } // TST
                setX(Rd, "("+Xr(Rn)+" "+op+" "+(ok?imms((int64_t)v):"?")+")"); continue; }
            // UBFM/SBFM/BFM bits[28:23]=100110 -> LSL/LSR/ASR #imm, SXT*/UXT*, bitfield extract
            if(((w>>23)&0x3f)==0x26){ int opc=(w>>29)&3, size=(w>>31)&1?64:32, immr=(w>>16)&0x3f, imms6=(w>>10)&0x3f; string n=Xr(Rn);
                if(opc==1){                                                                            // BFM/BFI/BFXIL
                    if(immr==0 && spok[Rd] && imms6<size-1){                                            // BFI #0,#w into an aligned stack-address base = the masked-index idiom `&base[Rn & mask]` (clang's k[j & mask])
                        uint64_t mask=(imms6>=63)?~0ull:((1ull<<(imms6+1))-1);
                        string idx="("+n+" & "+imms((int64_t)mask)+")";
                        reg[Rd]="(&"+slot(spoff[Rd])+" + "+idx+")"; spok[Rd]=false; immok[Rd]=false; if(Rd<8)argsSet.insert(Rd); curLocalDef|=(1u<<Rd); defSeq[Rd]=++seqCtr; continue; }
                    setX(Rd,n); continue; }                                                            // other BFI forms ~ approximate (keep source)
                if(opc==0&&immr==0){ setX(Rd,n); regSx[Rd]=true; continue; }                            // SXTB/SXTH/SXTW -> identity (signed widen); mark for 64-bit-mul widening
                bool sx64=(size==64&&regSx[Rn]);                                                       // a 64-bit shift of a sign-extended int -> stays 64-bit
                if(imms6==size-1){ if(opc==2) setX(Rd,"(("+string(size==64?"unsigned long":"unsigned")+")("+n+") >> "+std::to_string(immr)+")");   // UBFM = LSR (logical/unsigned -> zero-extend, not sign-extend)
                    else { setX(Rd,"("+n+" >> "+std::to_string(immr)+")"); if(sx64)regSx[Rd]=true; } continue; }                          // SBFM = ASR (arithmetic/signed)
                if(imms6<immr){ int lsb=size-immr, fw=imms6+1;
                    if(imms6==immr-1){ setX(Rd, sx64? "((long)("+n+") << "+std::to_string(lsb)+")" : "("+n+" << "+std::to_string(lsb)+")"); if(sx64)regSx[Rd]=true; }   // plain LSL #lsb (64-bit sign-extended shift must not overflow at 32 bits)
                    else { uint64_t mask=fw>=64?~0ull:((1ull<<fw)-1); setX(Rd,"(("+n+" & "+imms((int64_t)mask)+") << "+std::to_string(lsb)+")"); }   // UBFIZ/SBFIZ: low `fw` bits inserted at `lsb` — the mask is NOT redundant here
                    continue; }
                int width=imms6-immr+1; uint64_t mask=width>=64?~0ull:((1ull<<width)-1);               // UBFX/UXTB/UXTH -> mask
                setX(Rd,"(("+n+" >> "+std::to_string(immr)+") & "+imms((int64_t)mask)+")"); continue; }
            // Logical (shifted register) bits[28:24]=01010 — Rm can carry a shift (asr/lsr/lsl/ror #imm) AND an N-bit invert
            if(((w>>24)&0x1f)==0x0a){ int opc=(w>>29)&3, sh=(w>>22)&3, imm6=(w>>10)&0x3f, N=(w>>21)&1;
                const char* op=opc==0?"&":opc==1?"|":opc==2?"^":"&";
                string rm=Xr(Rm);
                if(imm6){ int width=((w>>31)&1)?64:32;                                                 // apply the shifted-register modifier (the dropped `, asr #13` was turning `a ^ (a>>13)` into `a ^ a`)
                    if(sh==0) rm="("+rm+" << "+std::to_string(imm6)+")";
                    else if(sh==3) rm="(("+rm+" >> "+std::to_string(imm6)+") | ("+rm+" << "+std::to_string(width-imm6)+"))";   // ROR
                    else rm="("+rm+" >> "+std::to_string(imm6)+")"; }                                  // LSR/ASR (signedness recovered separately)
                if(opc==1&&Rn==31){ setX(Rd, N? "(~"+rm+")" : rm); continue; }                         // ORR Xd,XZR,Xm = MOV ; ORN Xd,XZR,Xm = MVN (~Rm)
                if(N) rm="(~"+rm+")";                                                                  // BIC/EON = op with ~Rm
                setX(Rd, "("+Xr(Rn)+" "+op+" "+rm+")"); continue; }
            // ADD/SUB (shifted register) bits[28:24]=01011, bit21=0
            if(((w>>24)&0x1f)==0x0b && !((w>>21)&1)){ bool sub=(w>>30)&1, S=(w>>29)&1; int sh=(w>>22)&3, imm6=(w>>10)&0x3f;
                string xn, rm; if(Rn==Rm && Rn!=31){ string t=snap(Rn); xn=t; rm=t; } else { xn=Xr(Rn); rm=Xr(Rm); }   // `h + (h<<k)`: snapshot h once so it isn't inlined twice (unrolled-hash blowup)
                if(imm6){ const char* s=sh==0?"<<":sh==1?">>":">>"; rm="("+rm+" "+s+" "+std::to_string(imm6)+")"; }
                if(S){ lastL=Xr(Rn); lastR=Xr(Rm); testMode=false; }                                  // (SUBS/ADDS reg) set flags -> seed compare
                if(Rd!=31) setX(Rd, "("+xn+(sub?" - ":" + ")+rm+")"); continue; }
            // ADD/SUB (extended register) bits[28:24]=01011, bit21=1 -> base + index*scale (pointer math)
            if(((w>>24)&0x1f)==0x0b && ((w>>21)&1)){ bool sub=(w>>30)&1, S=(w>>29)&1; int imm3=(w>>10)&7;
                string rm=Xr(Rm); if(imm3) rm="("+rm+" << "+std::to_string(imm3)+")";
                if(S){ lastL=Xr(Rn); lastR=Xr(Rm); testMode=false; }
                if(Rd!=31) setX(Rd, (Rn==31?string("(0"):"("+Xr(Rn))+(sub?" - ":" + ")+rm+")"); continue; }
            // MADD/MSUB/SMADDL/UMADDL (mul) bits[28:24]=11011, bit24=1
            if(((w>>24)&0x1f)==0x1b){ int Ra=(w>>10)&0x1f; bool msub=(w>>15)&1; int op31=(w>>21)&7; bool w64=(w>>31)&1;
                bool widen=(op31==1||op31==5);                                                          // SMADDL/UMADDL/SMULL/UMULL: 32x32 -> 64-bit product
                // a 64-bit multiply of sign-extended ints (or an explicit widening mul) is `(long)a * (long)b`,
                // NOT 32-bit `int * int` — without the cast `(int64_t)a*b >> 16` overflows at 32 bits (fixed-point).
                auto mop=[&](int r){ return (widen||(w64&&regSx[r]))? "(long)("+Xr(r)+")" : Xr(r); };
                if(op31==2||op31==6){ setX(Rd, "("+Xr(Rn)+" * "+Xr(Rm)+")"); continue; }                // SMULH/UMULH (high 64 of 128) — approximate
                if(Ra==31) setX(Rd, "("+mop(Rn)+" * "+mop(Rm)+")");
                else setX(Rd, "("+Xr(Ra)+(msub?" - ":" + ")+mop(Rn)+" * "+mop(Rm)+")");
                continue; }
            // data-proc 2-source bits[30:21]=0011010110 (matches 32- and 64-bit; ignore sf at bit31)
            if(((w>>21)&0x3ff)==0xd6){ int o2=(w>>10)&0x3f;
                if(o2==2||o2==3){ setX(Rd,"("+Xr(Rn)+" / "+Xr(Rm)+")"); continue; }                   // UDIV/SDIV
                const char* s=o2==8?"<<":o2==9?">>":o2==10?">>":o2==11?">>":0;                         // LSLV/LSRV/ASRV/RORV
                if(s){ bool w64=(w>>31)&1;
                    if(o2==9) setX(Rd,"(("+string(w64?"unsigned long":"unsigned")+")("+Xr(Rn)+") >> "+Xr(Rm)+")");   // LSRV = logical/unsigned shift
                    else setX(Rd,"("+Xr(Rn)+" "+s+" "+Xr(Rm)+")"); continue; } }
            // CSEL/CSINC/CSINV/CSNEG (conditional select; CSET/CINC are aliases) -> ternary
            if((w&0x1FE00000u)==0x1A800000u){ int op=(w>>30)&1, o2=(w>>10)&1, cc=(w>>12)&0xf;
                string ce = lastL.empty()? "cond" : (testMode? lastL+" "+condOp(cc)+" 0" : lastL+" "+condOp(cc)+" "+lastR);
                string el = op? (o2? "(-"+Xr(Rm)+")":"(~"+Xr(Rm)+")") : (o2? "("+Xr(Rm)+" + 1)":Xr(Rm));
                setX(Rd, "(("+ce+") ? "+Xr(Rn)+" : "+el+")"); continue; }
            // Load/store register (register offset) bits (w&0x3B200C00)==0x38200800 -> base[index]
            // opc (bits 23:22): 0=STR, 1=LDR(zero-ext), 2=LDRS*->64 (LDRSW etc.), 3=LDRS*->32 — opc!=0 is a LOAD.
            if((w&0x3B200C00u)==0x38200800u){ int size=(w>>30)&3, opc=(w>>22)&3; int S2=(w>>12)&1; int width=1<<size;
                if(opc==2&&size==3) continue;                                                          // PRFM (register) prefetch
                string base=Xr(Rn), idx=Xr(Rm); if(S2&&size) idx="("+idx+" << "+std::to_string(size)+")";
                if(S2&&size){ bool simple=!base.empty()&&(isalpha((unsigned char)base[0])||base[0]=='_'); for(char c:base)if(!isIdent(c))simple=false;   // a scaled access `base[idx, lsl #k]` => base points at `width`-byte elements
                    if(simple){ int& e=elemHint[base]; e=(e==0||e==width)?width:-1; } }
                string ref="*("+base+" + "+idx+")";
                if(opc!=0) setX(Rd,ref); else push(ref+" = "+Xr(Rd)+";"); continue; }
            // SIMD/FP load/store (V bit set). clang auto-vectorizes byte loops (e.g. a key compare) into
            // NEON; the GP decoders below would mis-read these and dead-eliminate them. Catch them here and,
            // for a LOAD from a known rodata address, surface the constant bytes (so a vectorized compare
            // against a hardcoded key still reveals the expected value), then skip GP decoding.
            if((w&0x3F000000u)==0x3D000000u || (w&0x3B000000u)==0x3C000000u){
                int size=(w>>30)&3, opc=(w>>22)&3; bool load=(opc&1); int Rn2=(w>>5)&0x1f, Rd2=w&0x1f;
                bool uns=((w&0x3F000000u)==0x3D000000u);
                uint64_t off = uns ? (((w>>10)&0xfffu) << ((opc>=2)?4:size)) : (uint64_t)sext((w>>12)&0x1ff,9);
                string mn = neonDisasm(w), note;
                if(load){
                    if(spok[Rn2] && neonBuf.empty()){ int64_t a=spoff[Rn2]+(int64_t)off; neonBuf="&"+slot(a); }   // a NEON load from the stack = the buffer being compared (lowest-offset = its start)
                    neonStr[Rd2].clear(); neonSrcVa[Rd2]=-1;
                    if(immok[Rn2]){ uint64_t va=imm[Rn2]+off;
                        if(dataSecOf(va)){ int nb=(opc>=2)?16:(1<<size); auto bs=readBytes(va,(size_t)nb);
                            string asc; bool any=false, bad=false; int printable=0;
                            for(unsigned char c:bs){ if(c==0) continue; if(c>=32&&c<127){ asc+=(char)c; any=true; printable++; } else bad=true; }
                            if(any && !bad && printable>=4){ note = "  // = \""+asc+"\""; neonKey += asc; neonStr[Rd2]=asc; }   // a REAL string (>=4 printable, no junk) -> NEON-assembled char buffer
                            else { string hex; for(unsigned char c:bs){ char t[3]; snprintf(t,sizeof t,"%02x",c); hex+=t; } note = "  // = 0x"+hex;
                                   neonSrcVa[Rd2]=(int64_t)va; neonSrcLen[Rd2]=nb; } } }   // numeric DATA constant ({100,0}, {3,1,4,1}) -> remember source so the store materializes it
                    push("// neon:  "+mn+note);
                }
                else {  // STORE — a NEON `str dN,[sp,#off]` of a known string constant materializes a real stack buffer (the password clang assembled via SIMD); coalesce contiguous stores into one memcpy
                    if(!neonStr[Rd2].empty() && spok[Rn2]){ int64_t a=spoff[Rn2]+(int64_t)off; string s=neonStr[Rd2];
                        auto esc=[&](const string& x){ string r; for(char c:x){ if(c=='"'||c=='\\')r+='\\'; r+=c; } return r; };
                        if(neonPend && a==neonPendStart+(int64_t)neonPendBuf.size() && neonPendIdx<B.stmts.size()){
                            neonPendBuf+=s; B.stmts[neonPendIdx]="memcpy(&"+slot(neonPendStart)+", \""+esc(neonPendBuf)+"\", "+std::to_string(neonPendBuf.size())+");  // string assembled on the stack via NEON";
                        } else {
                            neonPend=true; neonPendStart=a; neonPendBuf=s;
                            push("memcpy(&"+slot(a)+", \""+esc(s)+"\", "+std::to_string(s.size())+");  // string assembled on the stack via NEON"); neonPendIdx=B.stmts.size()-1;
                        }
                        neonStr[Rd2].clear();
                    } else if(neonSrcVa[Rd2]>=0 && spok[Rn2]){   // a NEON-assembled numeric DATA constant -> memcpy from g_<va>; buildDataTemplates makes it a typed `static const int g_va[]={...}`
                        int64_t a=spoff[Rn2]+(int64_t)off; char b[24]; snprintf(b,sizeof b,"g_%llx",(unsigned long long)neonSrcVa[Rd2]);
                        push("memcpy(&"+slot(a)+", "+b+", "+std::to_string(neonSrcLen[Rd2])+");  // data assembled on the stack via NEON");
                        neonSrcVa[Rd2]=-1; neonPend=false;
                    } else { neonPend=false; push("// neon:  "+mn); }
                }
                continue; }
            // Load/store register (unsigned immediate) bits: (w&0x3B000000)==0x39000000
            if((w&0x3B000000u)==0x39000000u){ int size=(w>>30)&3, opc=(w>>22)&3; uint64_t i12=(w>>10)&0xfff; int64_t off=i12<<size; int width=1<<size;
                if(opc==2&&size==3) continue;                                                          // PRFM prefetch — not a value load/store
                if(opc!=0){ setX(Rd, memref(Rn,off,width)); if(opc==2)regSx[Rd]=true; int64_t lo=argStoreOff(Rn,off); if(lo!=(int64_t)0x7fffffff) loadedOffs.insert(lo); }   // LDR / LDRSW / LDRSB / LDRSH (the bug: LDRSW has opc=2, bit22=0) — note the loaded slot so it's not mistaken for a vararg
                else { push(memref(Rn,off,width)+" = "+Xr(Rd)+";"); int64_t so=argStoreOff(Rn,off); if(so!=(int64_t)0x7fffffff) outArg[so]={Xr(Rd),B.stmts.size()-1}; }
                continue; }
            // Load/store register (immediate, pre/post-index & unscaled) (w&0x3B200400)==0x38000400/.c00 / unscaled .000
            if((w&0x3B000000u)==0x38000000u && !((w>>24)&1) && !((w>>21)&1)){ int size=(w>>30)&3, opc=(w>>22)&3; int64_t off=sext((w>>12)&0x1ff,9); int width=1<<size;
                if(opc==2&&size==3) continue;                                                          // PRFUM prefetch
                int idx=(w>>10)&3;                                                                     // 0=unscaled (LDUR/STUR) · 1=post-index · 3=pre-index (these last two WRITE BACK the base — the `s++` in a pointer walk)
                if((idx==1 || idx==3) && Rn!=31){
                    int64_t accOff = (idx==3)? off : 0;                                                // pre: access [Rn+off]; post: access [Rn], THEN advance
                    string base = reg[Rn];                                                             // base before the writeback (the load uses the old value)
                    if(opc!=0){ string t="t"+std::to_string(tmp++); push(t+" = "+memref(Rn,accOff,width)+";"); setX(Rd,t); }   // snapshot the loaded byte into a temp BEFORE the pointer moves -> the loop condition `while(c)` tests the value, not a re-deref of the advanced pointer
                    else push(memref(Rn,accOff,width)+" = "+Xr(Rd)+";");
                    setX(Rn, "("+base+(off<0?" - "+std::to_string((long long)-off):" + "+std::to_string((long long)off))+")");   // model the post/pre-increment so the loop's pointer actually advances
                    continue; }
                if(opc!=0){ setX(Rd, memref(Rn,off,width)); if(opc==2)regSx[Rd]=true; int64_t lo=argStoreOff(Rn,off); if(lo!=(int64_t)0x7fffffff) loadedOffs.insert(lo); } else { push(memref(Rn,off,width)+" = "+Xr(Rd)+";"); int64_t so=argStoreOff(Rn,off); if(so!=(int64_t)0x7fffffff) outArg[so]={Xr(Rd),B.stmts.size()-1}; }
                continue; }
            // LDR (literal, PC-relative)  (w&0x3B000000)==0x18000000
            if((w&0x3B000000u)==0x18000000u){ int64_t off=sext((w>>5)&0x7ffff,19)<<2; char b[24]; snprintf(b,sizeof b,"g_%llx",(unsigned long long)(addr+off)); setX(Rd, string("*")+b); continue; }
            // fmov Wd, Sn / Xd, Dn — the vectorized result leaving NEON for a general register (feeds the
            // flag test + csel that picks the success/fail string). If we accumulated the compared key,
            // synthesize a readable mismatch flag so the downstream branch/puts isn't left dangling.
            { uint32_t base=w&0xFFFFFC00u;
              if(base==0x1E260000u||base==0x9E660000u){ int rd=w&0x1f;
                if(!neonKey.empty() && !neonBuf.empty()){            // RECONSTRUCT the scalar loop clang vectorized away
                    string esc; for(char c:neonKey){ if(c=='"'||c=='\\')esc+='\\'; esc+=c; }
                    push("// (the clang-vectorized NEON above is the unrolled form of this byte loop:)");
                    push("int key_mismatch = 0;");
                    push("for (int i = 0; i < "+std::to_string((int)neonKey.size())+"; i++)");
                    push("    if (((unsigned char*)"+neonBuf+")[i] != (unsigned char)\""+esc+"\"[i]) key_mismatch = 1;");
                    setX(rd, "key_mismatch"); neonKey.clear(); neonBuf.clear(); }
                else if(!neonKey.empty()){ push(string("// >>> vectorized compare of the input against \"")+neonKey+"\"");
                    setX(rd, "key_mismatch /* nonzero if input != \""+neonKey+"\" */"); neonKey.clear(); }
                else setX(rd, "fmov_"+std::to_string(rd));
                continue; } }
            if(((w>>25)&7)==7){ push("// neon:  "+neonDisasm(w)); continue; }   // scalar-FP / NEON-SIMD data op — show the decoded vectorized op (no longer silently dropped)
            // conditional compare CCMP/CCMN — a flag-setting compare CHAINED off the previous one
            // (the length-then-content string dispatch in arg parsers). We don't model NZCV, so render it
            // as a readable compare instead of raw hex; the consuming b.<cond> still carries the branch.
            if((w&0x3FE00C10u)==0x3A400800u || (w&0x3FE00C10u)==0x3A400000u){
                bool isImm=(w>>11)&1; int op=(w>>30)&1, cnd=(w>>12)&0xf; (void)op;
                string lit = isImm? imms((int64_t)((w>>16)&0x1f)) : Xr(Rm);
                // CCMP chains a 2nd compare onto a 1st for short-circuit `A && B`: the 2nd compare runs only
                // when `cnd` holds (A's truth). Recover the compound so the consuming branch/csel gets BOTH
                // clauses AND the right polarity (the decisive operands are this instr's, not the prior cmp's).
                string guard; if(!lastL.empty()) guard = testMode ? (lastL+" "+condOp(cnd)+" 0") : (lastL+" "+condOp(cnd)+" "+lastR);
                if(!guard.empty()){ lastL = "(("+guard+") && ("+Xr(Rn); lastR = lit+"))"; }
                else { lastL = Xr(Rn); lastR = lit; }
                testMode = false;                                                // the final branch/csel applies its own condition to (Rn vs lit) -> correct direction
                continue; }
            // BRK — compiler trap (unreachable / bounds-check abort), e.g. brk #1
            if((w&0xFFE0001Fu)==0xD4200000u){ push("__builtin_trap();"); continue; }
            // LSE atomic memory ops (LDADD/LDCLR/LDEOR/LDSET/SWP...) — refcount/atomic helpers the compiler
            // inlined (e.g. shared_ptr's atomic refcount). Render as the matching __atomic_* builtin.
            if((w&0x3B200C00u)==0x38200000u){
                bool o3=(w>>15)&1; int opc=(w>>12)&7;
                static const char* AOP[]={"__atomic_fetch_add","__atomic_fetch_and","__atomic_fetch_xor","__atomic_fetch_or",
                                          "__atomic_fetch_max","__atomic_fetch_min","__atomic_fetch_umax","__atomic_fetch_umin"};
                string call=string(o3?"__atomic_exchange_n":AOP[opc])+"("+Xr(Rn)+", "+Xr(Rm)+")";
                if(Rd==31) push(call+";"); else setX(Rd,call);     // Rt==xzr -> result discarded (pure atomic store)
                continue; }
            // EXTR Xd,Xn,Xm,#imms — rotate/extract (Xn==Xm => ROR). The common 0x93c…/0x13… undecoded family.
            if((w&0x7FA00000u)==0x13800000u){ bool w64=(w>>31)&1; int W=w64?64:32; int s=(w>>10)&(w64?0x3f:0x1f);
                string xn=Xr(Rn), xm=Xr(Rm);
                if(Rn==Rm) setX(Rd, s==0? xn : "(("+xn+" >> "+std::to_string(s)+") | ("+xn+" << "+std::to_string(W-s)+"))");
                else setX(Rd, "(("+xn+" << "+std::to_string(W-s)+") | ("+xm+" >> "+std::to_string(s)+"))");
                continue; }
            // data-processing (1 source): RBIT / REV16 / REV / CLZ / CLS  (0x5ac0…/0xdac0…)
            if((w&0x7FFFFC00u)==0x5AC00000u){ int opc=(w>>10)&0x3f; string xn=Xr(Rn), e;
                switch(opc){ case 0: e="__rbit("+xn+")"; break; case 1: e="__rev16("+xn+")"; break;
                    case 2: e="__builtin_bswap32("+xn+")"; break; case 3: e="__builtin_bswap64("+xn+")"; break;
                    case 4: e=((w>>31)&1)?"__builtin_clzll("+xn+")":"__builtin_clz("+xn+")"; break; case 5: e="__cls("+xn+")"; break; }
                if(!e.empty()){ setX(Rd,e); continue; } }
            // load-acquire / store-release / exclusive (LDAR/STLR/LDXR/STXR, 0x08/48/88/c8…): model the memory access
            if((w&0x3F000000u)==0x08000000u){ int width=1<<((w>>30)&3); bool load=(w>>22)&1;
                if(load) setX(Rd, memref(Rn,0,width)); else push(memref(Rn,0,width)+" = "+Xr(Rd)+";");
                continue; }
            if((w&0xFFFFE01Fu)==0xD503201Fu) continue;                                   // system HINT (BTI/CSDB/…) — no-op
            // unknown -> one clean line (never .byte soup)
            push("/* .word "+imms((int64_t)(uint32_t)w)+" */");
        }
        // snapshot cross-block defs: registers given a non-trivial exit value (differs from their own seed name)
        for(int r=0;r<31;r++){ if(!(curLocalDef&(1u<<r)))continue; const string& e=reg[r];
            if(e=="x"+std::to_string(r) || (r<8 && e=="a"+std::to_string(r))) continue;     // defined back to its canonical name -> nothing to carry
            B.regdef[r]=e; B.regdefSeq[r]=defSeq[r]; B.defSet|=(1u<<r); }
        curB=nullptr;
        if(B.term==Block::FALL && be<ins.size()) B.tgtF=ins[be].addr;
        return B;
    }

    // ── everything below is the shared structuring / synthesis machinery ──
    vector<Block> blocks; map<uint64_t,int> bidx;
    int BI(uint64_t a){ auto it=bidx.find(a); return it==bidx.end()?-1:it->second; }

    // ── cross-block dataflow: register values defined in one block and read in another are otherwise lost
    // (each block reset its register state) and leak as bare `xN`. Compute liveness over the CFG, then at
    // each producing block's exit materialize `xR = <expr>;` for every register that is live across an
    // out-edge — so the consumer reads a real, assigned variable (this is the Binary-Ninja-grade win). ──
    void crossBlockDataflow(){
        int N=(int)blocks.size(); if(N==0) return;
        const uint32_t SPMASK = ~((1u<<31) | (fpSet?(1u<<29):0u));   // never treat xzr/sp or the frame ptr as a value var
        auto succs=[&](int b,int out[2])->int{ Block& B=blocks[b]; int n=0;
            if(B.term==Block::COND){ int t=BI(B.tgtT),f=BI(B.tgtF); if(t>=0)out[n++]=t; if(f>=0)out[n++]=f; }
            else if(B.term==Block::GOTO){ int t=BI(B.tgtT); if(t>=0)out[n++]=t; }
            else if(B.term==Block::FALL){ int f=BI(B.tgtF); if(f>=0)out[n++]=f; }
            return n; };
        vector<uint32_t> DEF(N,0), USE(N,0), liveIn(N,0), liveOut(N,0);
        for(int b=0;b<N;b++){ if(blocks[b].dead)continue; DEF[b]=blocks[b].defSet&SPMASK; USE[b]=blocks[b].useBare&SPMASK; }
        bool changed=true;                                          // backward liveness fixpoint (handles loops/back-edges)
        while(changed){ changed=false;
            for(int b=N-1;b>=0;b--){ if(blocks[b].dead)continue;
                int s[2]; int n=succs(b,s); uint32_t lo=0; for(int i=0;i<n;i++) lo|=liveIn[s[i]];
                uint32_t li = USE[b] | (lo & ~DEF[b]);
                if(lo!=liveOut[b]||li!=liveIn[b]){ liveOut[b]=lo; liveIn[b]=li; changed=true; } } }
        // ── SSA LIVE-RANGE SPLITTING (Chaitin webs): one machine register reused for unrelated
        // values (x8 = stdin … x8 = hash) is ONE name today, which reads like nonsense. Split each
        // register into independent webs (connected def/use components) and give each web its own
        // token (`x8`, `x8s1`, …) so the renamer makes them distinct locals. Scoped to the non-arg,
        // non-frame x-registers (r in [8,30]\{29}) used purely 64-bit, so arg/param dataflow and the
        // 32/64-bit-view aliasing are untouched — a pure, correctness-preserving readability win. ──
        auto tokVer=[&](int r,int v){ return "x"+std::to_string(r)+(v?("s"+std::to_string(v)):string()); };
        // forward reaching-def fixpoint helper (per register), defs identified by their block index
        for(int r=8;r<31;r++){ if(r==29) continue; uint32_t bit=1u<<r; if(!(SPMASK&bit)) continue;
            // skip if the 32-bit view (wR) appears anywhere — splitting would break the x/w alias
            bool hasW=false; { string wtok="w"+std::to_string(r);
              for(int b=0;b<N&&!hasW;b++){ Block& B=blocks[b]; if(B.dead)continue;
                auto has=[&](const string& s){ return replaceTok(s,wtok,"\1")!=s; };
                for(auto& l:B.stmts) if(has(l)){hasW=true;break;} if(!hasW&&(has(B.cond)||has(B.ret)))hasW=true; } }
            if(hasW) continue;
            // mat-defs (block emits xR = expr) and use-blocks (read incoming xR)
            vector<char> isDef(N,0),isMat(N,0),isUse(N,0);
            for(int b=0;b<N;b++){ if(blocks[b].dead)continue;
                isDef[b]=(DEF[b]&bit)!=0; isMat[b]=(DEF[b]&bit)&&(liveOut[b]&bit); isUse[b]=(USE[b]&bit)!=0; }
            vector<int> mats; for(int b=0;b<N;b++) if(isMat[b]) mats.push_back(b);
            if(mats.size()<2) continue;                            // 0/1 def -> nothing to split
            // reaching defs: reachIn[b] = OR over preds of reachOut[p]; reachOut = mat? {b} : (def? {} : reachIn)
            vector<set<int>> reachIn(N), reachOut(N);
            for(bool ch=true; ch; ){ ch=false;
              for(int b=0;b<N;b++){ if(blocks[b].dead)continue;
                set<int> ro = isDef[b] ? (isMat[b]? set<int>{b} : set<int>{}) : reachIn[b];
                if(ro!=reachOut[b]){ reachOut[b]=ro; ch=true; }
                int s[2]; int ns=succs(b,s); for(int i=0;i<ns;i++){ size_t before=reachIn[s[i]].size();
                  reachIn[s[i]].insert(ro.begin(),ro.end()); if(reachIn[s[i]].size()!=before) ch=true; } } }
            // union-find over mat-def block indices
            map<int,int> uf; for(int d:mats) uf[d]=d;
            auto find=[&](int x){ while(uf[x]!=x){ uf[x]=uf[uf[x]]; x=uf[x]; } return x; };
            auto uni=[&](int a,int b){ a=find(a); b=find(b); if(a!=b) uf[a]=b; };
            for(int b=0;b<N;b++){ if(blocks[b].dead||!isUse[b])continue;
                set<int> S=reachIn[b]; if(isMat[b]) S.insert(b);   // accumulation: read-then-write joins its inputs
                int prev=-1; for(int d:S){ if(prev>=0) uni(prev,d); prev=d; } }
            // assign version numbers per representative, ordered by first def block
            map<int,int> ver; int nv=0; for(int d:mats){ int rep=find(d); if(!ver.count(rep)) ver[rep]=nv++; }
            if(nv<2) continue;                                     // all one web -> no actual split
            // rewrite incoming reads in each use-block to its web token
            for(int b=0;b<N;b++){ if(blocks[b].dead||!isUse[b])continue;
                set<int>& S=reachIn[b]; if(S.empty())continue; int v=ver[find(*S.begin())]; if(!v)continue;
                string from="x"+std::to_string(r), to=tokVer(r,v);
                for(auto& l:blocks[b].stmts) l=replaceTok(l,from,to);
                blocks[b].cond=replaceTok(blocks[b].cond,from,to); blocks[b].ret=replaceTok(blocks[b].ret,from,to); }
            // stash the per-block def token so the materialize loop below emits the versioned name
            for(int d:mats) blocks[d].regdef[r] = "\x01"+std::to_string(ver[find(d)])+"\x01"+blocks[d].regdef[r];
        }
        for(int b=0;b<N;b++){ if(blocks[b].dead)continue; Block& B=blocks[b];
            uint32_t mat = DEF[b] & liveOut[b];                     // materialize only the values actually read downstream
            vector<int> order; for(int r=0;r<31;r++) if((mat&(1u<<r)) && !(r==29&&fpSet) && !B.regdef[r].empty()) order.push_back(r);
            std::sort(order.begin(),order.end(),[&](int a,int c){ return B.regdefSeq[a] < B.regdefSeq[c]; });   // PROGRAM ORDER, not register order: `hash += c` must be emitted before `c = *s++` reloads c
            for(int r : order){
                string e=B.regdef[r];
                string tok=(r<8?"a":"x")+std::to_string(r);          // arg regs use the a-token (cross-block consumer resets to aN -> same name)
                if(e.size()>1 && e[0]=='\x01'){ size_t z=e.find('\x01',1); int v=atoi(e.c_str()+1); e=e.substr(z+1); tok="x"+std::to_string(r)+(v?("s"+std::to_string(v)):string()); }
                B.stmts.push_back(tok+" = "+e+";"); }
        }
    }

    void liftFn(size_t start,size_t end){
        set<uint64_t> leaders; leaders.insert(ins[start].addr);
        for(size_t k=start;k<end;k++){ uint64_t t=0;int c=0; BK b=branchOf(ins[k].w,ins[k].addr,t,c);
            if(b==B_UNC||b==B_COND){ if(t>=ins[start].addr&&(end>=ins.size()||t<ins[end].addr)) leaders.insert(t); if(k+1<end) leaders.insert(ins[k+1].addr); }
            else if(b==B_RET&&k+1<end) leaders.insert(ins[k+1].addr); }
        for(int i=0;i<32;i++){ reg[i]="x"+std::to_string(i); immok[i]=false; spok[i]=false; }
        for(int i=0;i<8;i++) reg[i]="a"+std::to_string(i);
        reg[31]="0"; spok[31]=true; spoff[31]=0; fpSet=false; fpOff=0; argsSet.clear(); blocks.clear(); bidx.clear(); fields.clear(); slotW.clear(); elemHint.clear(); sfieldW.clear(); neonKey.clear(); neonBuf.clear(); neonPend=false; neonPendBuf.clear(); for(int i=0;i<32;i++){ neonStr[i].clear(); neonSrcVa[i]=-1; neonSrcLen[i]=0; regSx[i]=false; }
        vector<uint64_t> L(leaders.begin(),leaders.end());
        for(size_t li=0;li<L.size();li++){ size_t bi=at[L[li]], be=li+1<L.size()?at[L[li+1]]:end;
            if(li>0){ for(int i=0;i<32;i++){reg[i]="x"+std::to_string(i);immok[i]=false;spok[i]=false;regSx[i]=false;} for(int i=0;i<8;i++)reg[i]="a"+std::to_string(i); reg[31]="0"; spok[31]=true; spoff[31]=0; argsSet.clear();
                      if(fpSet){ spok[29]=true; spoff[29]=fpOff; } }   // keep the frame-pointer alias alive across basic blocks
            Block B=liftBlock(bi,be); bidx[B.addr]=(int)blocks.size(); blocks.push_back(B); }
        for(auto& B:blocks){ if(B.term==Block::COND){int t=BI(B.tgtT); if(t>=0)blocks[t].isTarget=true;} if(B.term==Block::GOTO){int t=BI(B.tgtT); if(t>=0)blocks[t].isTarget=true;} }
        { set<int> reach; vector<int> stk{0};                                   // dead-block elimination: reachable from entry only
          while(!stk.empty()){ int b=stk.back(); stk.pop_back(); if(b<0||b>=(int)blocks.size()||reach.count(b))continue; reach.insert(b); Block& B=blocks[b];
            auto add=[&](uint64_t a){ int t=BI(a); if(t>=0)stk.push_back(t); };
            if(B.term==Block::COND){ add(B.tgtT); add(B.tgtF); } else if(B.term==Block::GOTO) add(B.tgtT); else if(B.term==Block::FALL) add(B.tgtF); }
          for(size_t i=0;i<blocks.size();i++) blocks[i].dead = !reach.count((int)i); }
        // tail-duplication of small return-leaves: a RET block reached ONLY by jumps (no fall-through
        // predecessor) can be copied into each goto/cond site and the orphan dropped -> kills the goto
        // with zero duplicate/dead code in the output (switch-dispatch case bodies become `return f();`).
        for(size_t ti=0; ti<blocks.size(); ti++){ Block& L=blocks[ti];
            if(L.dead||L.term!=Block::RET||!L.isTarget||L.stmts.size()>3) continue;
            bool fallIn = (ti>0 && !blocks[ti-1].dead && (blocks[ti-1].term==Block::FALL||blocks[ti-1].term==Block::COND));  // prev block falls through into L?
            if(fallIn) continue;                 // keep it (a sequential path needs it) — don't risk a duplicate
            L.inlineable=true; L.dead=true; }    // only-jumped-to -> every entry is a site emit() will inline
        // every live block is a label candidate: recovered/flat code can jump to a block that the normal
        // structured flow reaches by fall-through (so it was never a tgtT target). collect() prunes any
        // label no goto references, so this only ever *resolves* would-be-dangling gotos — never adds noise.
        for(auto& B:blocks) if(!B.dead&&!B.inlineable) B.isTarget=true;
        crossBlockDataflow();                                   // materialize cross-block register values before structuring
        emit(0,(int)blocks.size(),1,0,0);
        // completeness safety net: the structurer's forward-progress jumps (i=exitIdx / i=elseEnd) can skip
        // over reachable blocks, dropping their code AND leaving gotos to them dangling. Emit any reachable
        // block we never wrote out, flat + labeled, the way IDA/Ghidra dump un-structurable tails. Loop to
        // a fixpoint (a skipped block's successors may themselves be skipped).
        for(bool more=true; more; ){ more=false;
          for(int k=0;k<(int)blocks.size();k++){ Block& B=blocks[k];
            if(B.inlineable||emittedBI.count(k)) continue;
            if(B.dead && !used.count(B.addr)) continue;          // emit a dead block only if a goto actually targets it (e.g. an EH landing pad)
            more=true; label(1,B.addr); emitStmts(B,1);
            if(B.term==Block::RET) say(1,"return "+B.ret+";");
            else if(B.term==Block::GOTO){ if(B.tgtT){ say(1,"goto "+hx(B.tgtT)+";"); used.insert(B.tgtT); } }
            else if(B.term==Block::COND){ say(1,"if ("+B.cond+") goto "+hx(B.tgtT)+";"); used.insert(B.tgtT);
                int kf=k+1; if(kf<(int)blocks.size()){ say(1,"goto "+hx(blocks[kf].addr)+";"); used.insert(blocks[kf].addr); } }
            else if(B.term==Block::FALL){ int kf=k+1; if(kf<(int)blocks.size()){ say(1,"goto "+hx(blocks[kf].addr)+";"); used.insert(blocks[kf].addr); } } } }
        // any goto target that has no block of its own (tail-call / inter-function back-edge the lifter
        // didn't split a leader for) gets a labeled stub so the goto resolves instead of dangling.
        { vector<uint64_t> ext(used.begin(),used.end());
          for(uint64_t a:ext) if(BI(a)<0){ label(1,a); say(1,"; // -> "+hx(a)+" (tail-call / unrecovered target)"); } }
        FnResult R; R.name=name(ins[start].addr); string fn=fullName(ins[start].addr);
        // isMethod = a class-qualifier "::" in the NAME, before the param list — NOT a "::" inside a
        // parameter type like `bruteForce(std::string)` (that std:: is not a receiver → no `this`).
        { size_t lp=fn.find('('); string qual = lp==string::npos ? fn : fn.substr(0,lp); R.isMethod = qual.find("::")!=string::npos; }
        if(fn.find('(')!=string::npos && fn.rfind("sub_",0)!=0) R.sig=tidyCxxSig(fn);   // a real demangled C++ signature -> keep it for a doc comment
        R.fields=fields; R.slotW=slotW; R.elemHint=elemHint; R.sfieldW=sfieldW; R.body=collect();
        // GOTO ELIMINATION: if the structurer left any goto (irreducible flow, switch-dispatch, or a
        // back-edge C can't express with break/continue), re-render the whole function as a goto-free
        // `while(1) switch(state)` — every block a case, every edge a state assignment. Clean functions
        // (already goto-free) keep their pretty while/if output; only the gnarly ones convert.
        // GOTO HANDLING: structured-with-residual-gotos (IDA/Ghidra style) is FAR more readable than the
        // while(1)switch state-machine flattening — even on irreducible real-world C++ (vector/string dtor
        // chains, EH cleanup). So flattening is now OFF by default; set EMBER_FLATTEN=1 to opt back in.
        { static const bool FLAT = getenv("EMBER_FLATTEN") != nullptr;
          if(FLAT){ bool hasGoto=false; for(auto& l:R.body) if(l.find("goto ")!=string::npos){ hasGoto=true; break; }
            if(hasGoto){ emitStateMachine(); R.body=collect(); } } }
        for(auto& l:R.body){ for(int k=0;k<8;k++){ char a[8]; snprintf(a,8,"= a%d;",k); size_t p=l.find(a);
            if(p!=string::npos){ string lhs=trim(l.substr(0,p)); if(fields.count(lhs)) R.a0base=lhs; } } }
        bool u8[8]={false}; for(auto& l:R.body) for(int k=0;k<8;k++){ char a[4]; snprintf(a,4,"a%d",k); if(l.find(a)!=string::npos)u8[k]=true; }
        for(int k=0;k<8;k++) if(u8[k]) R.maxArg=k;
        fns.push_back(R);
    }

    struct Line{ string t; bool lab=false; uint64_t a=0; };
    vector<Line> out; set<uint64_t> used; set<int> emittedBI;   // emittedBI: block indices actually written out, for the completeness safety net
    static string IND(int n){ return string(n*4,' '); }
    static string hx(uint64_t a){ char b[24]; snprintf(b,sizeof b,"loc_%llx",(unsigned long long)a); return b; }
    void say(int i,const string& s){ out.push_back({IND(i)+s,false,0}); }
    void label(int i,uint64_t a){ out.push_back({IND(i)+hx(a)+":",true,a}); }
    void emitStmts(Block& B,int ind){ int k=BI(B.addr); if(k>=0)emittedBI.insert(k); for(auto& s:B.stmts) say(ind,s); }
    vector<string> collect(){ vector<string> r; for(auto& L:out) if(!(L.lab&&!used.count(L.a))) r.push_back(L.t); out.clear(); used.clear(); emittedBI.clear(); return r; }

    // Goto-free renderer: the whole function as `int __s = 0; while (1) switch (__s) { case k: ...; __s = j; break; }`.
    // Block index = state id; entry = block 0 (lowest addr). Every terminator becomes a state assignment, a
    // conditional state pick, or a return — so no goto (and no labeled break/continue, which C lacks) is needed.
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
            else { int f=BI(B.tgtF); if(f<0&&k+1<N)f=k+1;            // FALL -> next block
                if(f>=0) say(2,"__s = "+std::to_string(f)+"; break;");
                else say(2,"return 0;"); } }
        say(1,"}");
    }
    struct FnResult{ string name; string sig; std::map<string,std::map<int,int>> fields; std::map<string,int> slotW; std::map<string,int> elemHint; std::map<int,int> sfieldW; string a0base; vector<string> body; int maxArg=-1; bool isMethod=false; };
    // tidy a demangled C++ signature for a doc comment: std::__1:: -> std::, basic_string<char,..> -> string
    static string tidyCxxSig(string s){
        for(size_t p; (p=s.find("std::__1::"))!=string::npos;) s.replace(p,10,"std::");
        // collapse `basic_string<char, ...>` (any args) -> `string`
        for(;;){ size_t p=s.find("basic_string<"); if(p==string::npos)break; int d=0; size_t e=p+12; for(;e<s.size();e++){ if(s[e]=='<')d++; else if(s[e]=='>'){ if(--d==0){e++;break;} } } s.replace(p,e-p,s.compare(p+13,5,"wchar")==0?"wstring":"string"); }
        for(size_t p; (p=s.find(", std::allocator<"))!=string::npos;){ int d=0; size_t e=p+2; for(;e<s.size();e++){ if(s[e]=='<')d++; else if(s[e]=='>'){ if(--d==0){e++;break;} } } s.erase(p,e-p); }   // drop the default allocator arg
        return s;
    }
    vector<FnResult> fns;
    static string trim(const string& s){ size_t a=s.find_first_not_of(" "); if(a==string::npos)return ""; size_t b=s.find_last_not_of(" "); return s.substr(a,b-a+1); }
    static bool isIdent(char c){ return isalnum((unsigned char)c)||c=='_'; }
    static string replaceTok(const string& s,const string& from,const string& to){ if(from.empty())return s; string r; size_t i=0;
        while(i<s.size()){ if(s.compare(i,from.size(),from)==0&&(i==0||!isIdent(s[i-1]))&&(i+from.size()>=s.size()||!isIdent(s[i+from.size()]))){ r+=to; i+=from.size(); } else r+=s[i++]; } return r; }
    void emitStructBody(std::map<int,int>& f,std::map<int,string>& nm,std::map<int,string>* ty=nullptr,const string& self=""){ int next=0; for(auto& fo:f){ if(fo.first>next&&fo.first>=0&&next>=0)printf("    char _pad%d[%d];\n",next,fo.first-next);
        string t = (ty&&ty->count(fo.first)) ? (*ty)[fo.first] : tyOf(fo.second); if(t=="@self") t = self.empty()?"void*":(self+"*");   // self-referential pointer (linked list)
        printf("    %s %s;\n",t.c_str(),nm[fo.first].c_str()); next=fo.first+fo.second; } }

    static void collectV(const string& l,set<string>& vs){ for(size_t i=0;i<l.size();){ if(l[i]=='v'&&i+1<l.size()&&isdigit((unsigned char)l[i+1])&&(i==0||!isIdent(l[i-1]))){ size_t j=i+1; while(j<l.size()&&isdigit((unsigned char)l[j]))j++; if(j>=l.size()||!isIdent(l[j]))vs.insert(l.substr(i,j-i)); i=j; } else i++; } }
    // leaked cross-block registers (xN/wN, N=0..30; not sp/xzr=31, not the frame ptr x29) read as raw
    // machine registers. They're materialized (assigned-before-use) by crossBlockDataflow, so naming
    // them as ordinary locals is a pure readability win — feed them through the same heuristics as vN.
    static void collectRegs(const string& l,set<string>& vs){ for(size_t i=0;i<l.size();){ char c=l[i];
        if((c=='x'||c=='w')&&(i==0||!isIdent(l[i-1]))){ size_t j=i+1; while(j<l.size()&&isdigit((unsigned char)l[j]))j++;
            if(j>i+1){ int n=atoi(l.c_str()+i+1); size_t k=j;                                  // optional SSA web suffix `sV` -> a distinct local
                if(k<l.size()&&l[k]=='s'&&k+1<l.size()&&isdigit((unsigned char)l[k+1])){ k++; while(k<l.size()&&isdigit((unsigned char)l[k]))k++; }
                if(k>=l.size()||!isIdent(l[k])){ if(n>=0&&n<=30&&n!=29) vs.insert(l.substr(i,k-i)); i=k; continue; } } }
        i++; } }
    // derive a readable variable name from the function whose result it holds: strip a verb prefix,
    // take the distinguishing trailing segment, map common libc — `fx_sqrt`->sqrt, `strlen`->len,
    // `popcount_kernighan`->kernighan, `compute_area`->area. "" if no good name (sub_/numeric).
    static bool isCKeyword(const string& s){ static const set<string> K={"int","long","char","short","double","float","void","unsigned","signed","bool","const","static","struct","class","auto","return","if","else","for","while","do","switch","new","delete","operator","this","true","false","to","of","the"}; return K.count(s)>0; }
    static string verbOf(const string& fn){
        static const std::map<string,string> M={{"strlen","len"},{"strnlen","len"},{"strcmp","cmp"},{"strncmp","cmp"},
            {"malloc","buf"},{"calloc","buf"},{"realloc","buf"},{"operator_new","obj"},{"fopen","fp"},{"sqrt","root"},{"fabs","mag"},
            {"atoi","n"},{"atol","n"},{"strtol","n"},{"strchr","p"},{"strrchr","p"},{"abs","val"},{"pow","p"},{"getchar","c"},
            {"toupper","upper"},{"tolower","lower"},{"strstr","pos"},{"strcpy","dst"},{"strncpy","dst"},{"strcat","dst"},
            {"strdup","dup"},{"memcpy","dst"},{"memmove","dst"},{"memset","dst"},{"memchr","p"},{"getenv","env"},
            {"fgets","line"},{"getline","line"},{"fread","n"},{"fwrite","n"},{"read","n"},{"write","n"},{"recv","n"},{"send","n"},
            {"socket","fd"},{"accept","fd"},{"open","fd"},{"floor","lo"},{"ceil","hi"},{"sin","val"},{"cos","val"},{"log","val"},
            {"exp","val"},{"rand","rnd"},{"strtok","tok"},{"isalpha","ok"},{"isdigit","ok"},{"isspace","ok"},{"isalnum","ok"}};
        auto it=M.find(fn); if(it!=M.end()) return it->second;
        string s=fn; for(const char* p:{"fx_","get_","compute_","calc_","make_","create_","find_","do_","read_","is_","has_"}){ size_t pl=strlen(p); if(s.size()>pl+1&&s.compare(0,pl,p)==0){ s=s.substr(pl); break; } }
        it=M.find(s); if(it!=M.end()) return it->second;
        if(s.empty()||s.rfind("sub_",0)==0||s.rfind("S_",0)==0||!isalpha((unsigned char)s[0])) return "";
        vector<string> segs; { string cur; for(char c:s){ if(c=='_'){ if(!cur.empty())segs.push_back(cur); cur.clear(); } else cur+=(char)tolower((unsigned char)c); } if(!cur.empty())segs.push_back(cur); }
        for(int i=(int)segs.size()-1;i>=0;i--){ const string& seg=segs[i]; if(seg.size()>=2&&isalpha((unsigned char)seg[0])&&!isCKeyword(seg)) return seg.size()>14?seg.substr(0,14):seg; }   // last meaningful, non-keyword segment
        return "";
    }
    // unwrap parens / resolve a one-step temp alias `vN = tM` then `tM = rhs` -> the underlying rhs
    string resolveDef(const string& v,const vector<string>& body){ int defs=0; string rhs;
        for(auto& l:body){ string t=trim(l); if(t.rfind(v+" = ",0)==0){ defs++; rhs=t.substr(v.size()+3); if(!rhs.empty()&&rhs.back()==';')rhs.pop_back(); } }
        if(defs!=1) return ""; while(!rhs.empty()&&rhs.front()=='(')rhs.erase(0,1);
        if(rhs.size()>=2&&rhs[0]=='t'&&isdigit((unsigned char)rhs[1])){ bool pure=true; for(char c:rhs)if(!isIdent(c))pure=false;
            if(pure){ for(auto& l:body){ string t=trim(l); if(t.rfind(rhs+" = ",0)==0){ string r2=t.substr(rhs.size()+3); if(!r2.empty()&&r2.back()==';')r2.pop_back(); while(!r2.empty()&&r2.front()=='(')r2.erase(0,1); return r2; } } } }
        return rhs; }
    // Recover each parameter's WIDTH (int vs long vs char) the same way retW recovers return widths:
    // at -O0 a param is spilled `vJ = aK;`, and the spill slot vJ's max access width is in F.slotW.
    // param index -> width (1/2/4/8); absent => unknown => caller keeps `long`.
    std::map<int,int> paramWidths(const FnResult& F){
        std::map<int,int> pw;
        for(auto& l:F.body){ string t=trim(l); if(t.empty()||t.back()!=';') continue;
            size_t eq=t.find(" = a"); if(eq==string::npos) continue;
            string lhs=trim(t.substr(0,eq));                                   // the spill slot
            size_t s=eq+4, e=s; while(e<t.size()&&isdigit((unsigned char)t[e]))e++;
            if(e==s || e!=t.size()-1) continue;                                // RHS must be exactly `aK;`
            int k=atoi(t.substr(s,e-s).c_str());
            auto it=F.slotW.find(lhs); if(it==F.slotW.end()) continue;
            auto pit=pw.find(k); if(pit==pw.end()) pw[k]=it->second; else if(pit->second!=it->second) pw[k]=8; }   // conflicting widths -> widest (long)
        return pw;
    }
    std::map<string,string> nameVars(const vector<string>& body,int maxArg){
        std::map<string,string> m; set<string> vs; for(auto& l:body)collectV(l,vs);
        for(auto& l:body)collectRegs(l,vs);    // leaked registers (xN/wN) go through the SAME behavioral heuristics as vN — so the djb2 accumulator becomes `hash`, not `v137`
        set<string> used;
        int ic=0;
        // -O0 gives each loop its own stack slot, so a program can have far more live counters than i/j/k/l/m.
        // Hand out i,j,k,l,m, then i2,j2,k2,… (collision-safe) instead of falling back to vN.
        auto nextCounter=[&]()->string{ static const char* B[]={"i","j","k","l","m"}; string nm;
            do{ nm = (ic<5)? string(B[ic]) : (string(B[ic%5])+std::to_string(ic/5+1)); ic++; }while(used.count(nm)); used.insert(nm); return nm; };
        for(auto& v:vs){ bool cmp=false,inc=false; for(auto& l:body){ string t=trim(l);
            if(t.rfind("while (",0)==0&&(t.find(v+" < ")!=string::npos||t.find(v+" != ")!=string::npos||t.find(v+" > ")!=string::npos||t.find(v+" <= ")!=string::npos))cmp=true;
            if(t==v+" = ("+v+" + 1);")inc=true; } if(cmp&&inc){ m[v]=nextCounter(); } }
        // a `v=0; v=(v+1)` used as an array INDEX -> i/j/k (even with no `v<n` bound, e.g. strlen's counter)
        for(auto& v:vs){ if(m.count(v))continue; bool z=false,inc=false,idx=false; for(auto& l:body){ string t=trim(l);
            if(t==v+" = 0;")z=true; if(t==v+" = ("+v+" + 1);")inc=true;
            if(l.find(" + "+v+")")!=string::npos||l.find("("+v+" << ")!=string::npos||l.find("["+v+"]")!=string::npos)idx=true; }
            if(z&&inc&&idx){ m[v]=nextCounter(); } }
        // HASH accumulator: a var seeded with a constant and mixed via a SELF-shift (`v = (… (v << k) …)`) — the
        // djb2 / FNV / rolling-hash signature. Name it `hash` (and a `hash & MASK` derivative `digest`). MUST run
        // before the plain accumulator so `hash = hash*33 + c` isn't mislabeled `sum`.
        int hc=0; for(auto& v:vs){ if(m.count(v))continue; bool seed=false,mix=false;
            for(auto& l:body){ string t=trim(l); if(t.rfind(v+" = ",0)!=0)continue; string r=t.substr(v.size()+3); if(!r.empty()&&r.back()==';')r.pop_back();
                // a self-referential SHIFT, multiply-by-constant, or XOR in an update line = the hash-mix signature
                if(r.find("("+v+" << ")!=string::npos||r.find(" "+v+" <<")!=string::npos) mix=true;
                { size_t q=r.find("("+v+" * "); if(q!=string::npos){ size_t d=q+v.size()+4; if(d<r.size()&&isdigit((unsigned char)r[d])) mix=true; } }   // v * <const>
                if(r.find(v+" ^ ")!=string::npos||r.find("^ "+v)!=string::npos) mix=true;
                if(mix) continue;
                bool num=!r.empty(); size_t s=(!r.empty()&&r[0]=='-')?1:0; for(size_t i=s;i<r.size();i++) if(!isdigit((unsigned char)r[i])&&!isxdigit((unsigned char)r[i])&&r[i]!='x')num=false;
                if(num && r!="0" && r!="1" && r!="-1") seed=true; }     // a NON-trivial constant seed (5381/FNV/…); seed 0 = a numeric Horner accumulator (atoi), not a hash
            if(seed&&mix){ m[v]= hc++==0?"hash":("hash"+std::to_string(hc)); used.insert(m[v]); } }
        for(auto& v:vs){ if(m.count(v))continue; string rhs=resolveDef(v,body);   // `digest = (hash & 0xffffff)` — the masked hash result
            if(rhs.empty())continue; for(auto& kv:m){ if(kv.second.rfind("hash",0)!=0)continue; if(rhs.find("("+kv.first+" & ")!=string::npos||rhs.find(kv.first+" & ")==0){ string fin="digest"; int s=2; while(used.count(fin))fin="digest"+std::to_string(s++); m[v]=fin; used.insert(fin); break; } } }
        const char* ACC[]={"sum","total","acc","result","prod"}; int ac=0;
        for(auto& v:vs){ if(m.count(v))continue; bool z=false,a=false; for(auto& l:body){ string t=trim(l);
            if(t==v+" = 0;")z=true; if((t.rfind(v+" = ("+v+" + ",0)==0 && t!=v+" = ("+v+" + 1);")||t.rfind(v+" = ("+v+" - ",0)==0||t.rfind(v+" = ("+v+" * ",0)==0)a=true; } if(z&&a&&ac<5){ m[v]=ACC[ac++]; used.insert(m[v]); } }
        // names CALLED in the body — a var named after one would shadow it (`begin = begin(...)` won't compile)
        set<string> called; for(auto& l:body){ size_t p=0; while((p=l.find('(',p))!=string::npos){ long b=(long)p-1; while(b>=0&&(isalnum((unsigned char)l[b])||l[b]=='_'))b--; if((long)p-1>b&&!isdigit((unsigned char)l[b+1]))called.insert(l.substr(b+1,p-1-b)); p++; } }
        // call-result naming: a vN defined exactly once by a call (possibly via a temp) -> name from the callee
        for(auto& v:vs){ if(m.count(v))continue; string rhs=resolveDef(v,body); if(rhs.empty())continue;
            size_t e=0; while(e<rhs.size()&&(isalnum((unsigned char)rhs[e])||rhs[e]=='_'))e++; string callee=rhs.substr(0,e);
            if(e>=rhs.size()||rhs[e]!='('||callee.empty()||isdigit((unsigned char)callee[0])) continue;
            string nn=verbOf(callee); if(nn.size()<2||called.count(nn)) continue;                  // skip a name that shadows a called function
            string fin=nn; int s=2; while(used.count(fin)||called.count(fin))fin=nn+std::to_string(s++); m[v]=fin; used.insert(fin); }
        auto take=[&](const string& v,const char* base){ string fin=base; int s=2; while(used.count(fin)||called.count(fin))fin=base+std::to_string(s++); m[v]=fin; used.insert(fin); };
        // POINTER / BUFFER naming by how vN is dereferenced: vN[i] -> buf, vN->f -> obj, *vN -> ptr
        for(auto& v:vs){ if(m.count(v))continue; bool idx=false,arrow=false,deref=false;
            for(auto& l:body){ if(l.find(v+"[")!=string::npos)idx=true; if(l.find(v+"->")!=string::npos)arrow=true;
                if(l.find("*("+v+" + ")!=string::npos)idx=true;                       // *(vN + (i<<k)) = an indexed buffer
                if(l.find("*("+v+")")!=string::npos)deref=true;                        // *(vN) = a plain deref
                size_t p=0; while((p=l.find("*"+v,p))!=string::npos){ size_t af=p+1+v.size(); if((p==0||!isIdent(l[p-1]))&&(af>=l.size()||!isIdent(l[af])))deref=true; p=af; } }
            if(idx)take(v,"buf"); else if(arrow)take(v,"obj"); else if(deref)take(v,"ptr"); }
        // the RETURNED value -> result
        for(auto& l:body){ string t=trim(l); if(t.rfind("return ",0)!=0)continue; string r=t.substr(7); if(!r.empty()&&r.back()==';')r.pop_back(); r=trim(r);
            if(r.size()>=2&&r[0]=='v'&&isdigit((unsigned char)r[1])){ bool pure=true; for(char c:r)if(!isIdent(c))pure=false; if(pure&&!m.count(r))take(r,"result"); } }
        // a var used as a loop BOUND (`i < vN`) is a count/size -> n
        for(auto& v:vs){ if(m.count(v))continue; bool bound=false;
            for(auto& l:body){ size_t p=0; string pat="< "+v; while((p=l.find(pat,p))!=string::npos){ size_t af=p+pat.size(); if(af>=l.size()||!isIdent(l[af]))bound=true; p+=pat.size(); } }
            if(bound)take(v,"n"); }
        // ── PARAMETERS: name by ROLE (pointer/string/array/bound/scalar), not argN — the readability fix ──
        const char* SCAL[]={"a","b","c","d","e","f","g","h"}; int sc=0;
        for(int k=0;k<=maxArg;k++){ string ak="a"+std::to_string(k); if(m.count(ak))continue;
            set<string> names; names.insert(ak);                              // -O0 spills the param: `vJ = aK;` carries the real usage
            for(auto& l:body){ string t=trim(l); string suf=" = "+ak+";";
                if(t.size()>suf.size() && t.compare(t.size()-suf.size(),suf.size(),suf)==0){ string lhs=trim(t.substr(0,t.size()-suf.size())); if(isName(lhs)) names.insert(lhs); } }
            bool arrow=false,idxSh=false,idxByte=false,bound=false,self=false,list=false,nul=false;
            for(auto& nm:names){ for(auto& l:body){
                if(l.find(nm+"->")!=string::npos) arrow=true;
                if(l.find(nm+" = "+nm+"->")!=string::npos) list=true;                                   // h = h->next  (linked list)
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
            if(base) take(ak,base); else { take(ak, SCAL[sc<8?sc:7]); sc++; } }   // generic scalar -> a,b,c (reads like math, not argN)
        // SOURCE-EXPRESSION naming: a vN with a single plain def gets a name from WHAT IT READS.
        //   vN = obj->field   -> field        (the value pulled out of a struct: `op`, `value`, `data`)
        //   vN = *ptr / X[i]   -> ch           (a single element load — usually a byte/char in these programs)
        // High precision because resolveDef only fires on a UNIQUE plain definition.
        for(auto& v:vs){ if(m.count(v))continue; string rhs=resolveDef(v,body); if(rhs.empty())continue;
            size_t ar=rhs.rfind("->");
            if(ar!=string::npos){ size_t s=ar+2,e=s; while(e<rhs.size()&&(isalnum((unsigned char)rhs[e])||rhs[e]=='_'))e++;
                string fld=rhs.substr(s,e-s);
                bool rawFieldId = fld.size()>=2 && (fld[0]=='f') && (isdigit((unsigned char)fld[1]) || (fld[1]=='m'&&fld.size()>=3&&isdigit((unsigned char)fld[2])));  // f0 / fm1 — the lift's placeholder ids, not real names
                if(e==rhs.size() && fld.size()>=2 && !isdigit((unsigned char)fld[0]) && !rawFieldId && !called.count(fld)){ take(v,fld.c_str()); continue; } }
        }
        for(auto& v:vs){ if(m.count(v))continue; string rhs=resolveDef(v,body); if(rhs.empty())continue;
            bool elem = (rhs[0]=='*' && rhs.size()>1 && (isalpha((unsigned char)rhs[1])||rhs[1]=='_'));     // *p — a single element load (usually a byte/char here)
            if(elem) take(v,"ch"); }
        // COUNTDOWN: seeded with a constant (>1) and repeatedly DECREMENTED -> a remaining-count / loop ticker.
        for(auto& v:vs){ if(m.count(v))continue; bool seed=false,dec=false;
            for(auto& l:body){ string t=trim(l); if(t.rfind(v+" = ",0)==0){ string r=t.substr(v.size()+3); if(!r.empty()&&r.back()==';')r.pop_back();
                    bool num=!r.empty()&&(isdigit((unsigned char)r[0])); if(num){ for(char c:r) if(!isxdigit((unsigned char)c)&&c!='x')num=false; } if(num&&r!="0"&&r!="1")seed=true; }
                if(t==v+" = ("+v+" - 1);"||t.rfind(v+" = ("+v+" - ",0)==0)dec=true; }
            if(seed&&dec) take(v,"remaining"); }
        // LEAKED REGISTERS -> plain locals vN. crossBlockDataflow materializes them (assigned-before-use), so
        // they're real values that just read as raw machine registers. Generic vN (not the semantic heuristics,
        // which can produce keywords like `operator` or shadow a callee) is predictable + always compiles; the
        // AI Clean-Up pass gives them meaning. Numbered above existing vN so no collision.
        { set<string> regs; for(auto& l:body)collectRegs(l,regs);
          int next=0; auto bump=[&](const string& s){ if(s.size()>1&&s[0]=='v'){ bool num=true; for(size_t i=1;i<s.size();i++)if(!isdigit((unsigned char)s[i]))num=false; if(num){ int n=atoi(s.c_str()+1); if(n>=next)next=n+1; } } };
          for(auto& v:vs)bump(v); for(auto& kv:m)bump(kv.second);
          for(auto& r:regs){ if(m.count(r))continue; string fin; do{ fin="v"+std::to_string(next++); }while(used.count(fin)||called.count(fin)); m[r]=fin; used.insert(fin); } }
        return m;
    }
    // ── FIELD ROLES: best-guess NAME + C TYPE for each field, from how `base->fN` is used across the given
    // (base, body) contexts. Names are deduped. "@self" type = pointer to the containing struct (the
    // linked-list `next`); emitStructBody swaps in the real struct name. This is the offline best-guess. ──
    void fieldRoles(std::map<int,int>& flds, const vector<std::pair<string,const vector<string>*>>& ctx,
                    std::map<int,string>& names, std::map<int,string>& types){
        const char* GEN[]={"value","data","item","member"}; int gc=0; set<string> used;
        auto uniq=[&](const string& b)->string{ if(!used.count(b)){used.insert(b);return b;} for(int s=2;;s++){ string c=b+std::to_string(s); if(!used.count(c)){used.insert(c);return c;} } };
        // libc++ std::string shape: a width-1 long/short FLAG byte (offset >=8) tested `< 0`/`>= 0`, with an
        // 8-byte size at +8 and (usually) a data ptr at 0. Recognizing it -> fields read data/size/cap, type -> String.
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
                    if(l.find(a+"->")!=string::npos||t==B+" = "+a+";")next=true;                                  // base->F->… or base=base->F -> pointer
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
    // best-guess TYPE NAME from field roles (+ method names): Stack/Queue/Account (by method), Node (next),
    // Buffer (data+size), Counter (count+total), Pair (2 scalars) … else the caller's generic fallback.
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
    // the REAL class name from a demangled method signature: `PongGame::run()` -> "PongGame" (the segment
    // before the last `::` ahead of the param list). Far more reliable than guessing — use it when present.
    static string enclosingClass(const string& sig){
        if(sig.empty()) return ""; size_t lp=sig.find('('); string q = lp==string::npos?sig:sig.substr(0,lp);
        size_t c2=q.rfind("::"); if(c2==string::npos) return ""; string cls=q.substr(0,c2);
        size_t c1=cls.rfind("::"); if(c1!=string::npos) cls=cls.substr(c1+2);                 // keep last namespace component
        if(cls.empty()) return ""; for(char ch:cls) if(!(isalnum((unsigned char)ch)||ch=='_')) return "";   // plain identifier only (no templates)
        return cls;
    }
    std::map<int,string> nameFields(std::map<int,int>& flds,vector<int>& methods){
        std::map<int,string> m; const char* GEN[]={"value","data","item","member"}; int gc=0;
        for(auto& fo:flds){ int off=fo.first; string F=fieldId(off); bool count=false,total=false,ptr=false;
            for(int mi:methods){ string T=fns[mi].a0base; if(T.empty())continue; for(auto& l:fns[mi].body){ string t=trim(l);
                if(t==T+"->"+F+" = ("+T+"->"+F+" + 1);")count=true;
                else if(t.rfind(T+"->"+F+" = ("+T+"->"+F+" + ",0)==0||t.rfind(T+"->"+F+" = ("+T+"->"+F+" - ",0)==0)total=true;
                if(l.find(T+"->"+F+"->")!=string::npos)ptr=true; } }
            m[off]=count?"count":ptr?"next":total?"total":(string(GEN[gc%4])+(gc>=4?std::to_string(off):"")); if(!count&&!ptr&&!total)gc++; }
        return m;
    }
    static string applyMap(string s,std::map<string,string>& m){ for(auto& kv:m) s=replaceTok(s,kv.first,kv.second); return s; }
    std::map<string,string> strRepl; vector<std::pair<string,string>> strDecls;
    string readCStr(uint64_t va){ for(auto& s:secs){ if(va>=s.vaddr&&va<s.vaddr+s.size){ size_t off=s.fileoff+(va-s.vaddr); string r;
        for(size_t i=off;i<file->size()&&(*file)[i];i++){ unsigned char c=(*file)[i]; if(c<9||(c>13&&c<32)||c>126)return ""; r+=(char)c; if(r.size()>120)break; } return r; } } return ""; }
    static string strName(const string& s){ string r="s_"; for(char c:s){ if(isalnum((unsigned char)c))r+=tolower(c); else if(!r.empty()&&r.back()!='_')r+='_'; if(r.size()>=22)break; } while(r.size()>2&&r.back()=='_')r.pop_back(); return r.size()>2?r:"s_str"; }
    static string replaceAll(string s,const string& a,const string& b){ if(a.empty())return s; size_t p=0; while((p=s.find(a,p))!=string::npos){ s.replace(p,a.size(),b); p+=b.size(); } return s; }
    static string esc(const string& s){ string r; for(char c:s){ if(c=='"'||c=='\\')r+='\\',r+=c; else if(c=='\n')r+="\\n"; else if(c=='\t')r+="\\t"; else r+=c; } return r; }
    void buildStrings(){ std::map<string,uint64_t> toks;
        for(auto& F:fns) for(auto& l:F.body){ size_t i=0; while((i=l.find("g_",i))!=string::npos){ size_t j=i+2; while(j<l.size()&&isxdigit((unsigned char)l[j]))j++;
            string num=l.substr(i+2,j-(i+2)); if(!num.empty())toks["g_"+num]=strtoull(num.c_str(),0,16); i=j; } }
        std::map<string,int> seen;
        for(auto& kv:toks){ string s=readCStr(kv.second); if(s.size()<2)continue; string nm=strName(s); if(seen[nm]++)nm+="_"+std::to_string(seen[nm]);
            strRepl["*"+kv.first]=nm; strRepl[kv.first]="&"+nm; strDecls.push_back({nm,esc(s)}); } }
    // ── DATA TEMPLATES: materialize non-string data globals (.data/.rodata) into READABLE typed C —
    // `static const int g_X[9] = { 1, 2, 3, ... };` (element type from access/size, decimal values),
    // and a `// @data dest=int[9]` hint so the memcpy destination local is typed as a real array
    // (which also fixes the size: `long v` can't hold a 36-byte matrix). No AI, no fake stubs. ──
    vector<std::pair<string,string>> dataDecls;
    std::map<string, vector<string>> dataHint;   // fnname -> ["dest=int[9]", ...] consumed by ember-collapse
    std::map<string,int> roleCount;              // role -> running count, for readable global/local names
    std::map<string, std::map<string,string>> destRename;   // fnname -> origDest -> readable name
    void mergeDataNames(const string& fn, std::map<string,string>& vmap){ auto it=destRename.find(fn); if(it!=destRename.end()) for(auto& kv:it->second) vmap[kv.first]=kv.second; }
    const Section* dataSecOf(uint64_t va){ for(auto& s:secs) if(va>=s.vaddr && va<s.vaddr+s.size){ return s.name=="__text"?nullptr:&s; } return nullptr; }
    vector<unsigned char> readBytes(uint64_t va,size_t n){ vector<unsigned char> b; for(auto& s:secs) if(va>=s.vaddr && va<s.vaddr+s.size){ size_t off=s.fileoff+(va-s.vaddr);
        for(size_t i=0;i<n && va+i<s.vaddr+s.size;i++) b.push_back(off+i<file->size()?(*file)[off+i]:0); break; } return b; }
    static string hexBytes(const vector<unsigned char>& b){ string r; for(size_t i=0;i<b.size();i++){ char x[8]; snprintf(x,sizeof x,"0x%02x",b[i]); r+=x; if(i+1<b.size())r+=","; r+=(i%16==15&&i+1<b.size())?"\n    ":" "; } return r; }
    // every 8-byte group is a "sane" IEEE-754 double (exponent byte ~±1..±1000, or 0.0)
    static bool looksDouble(const vector<unsigned char>& b){ if(b.size()<8||b.size()%8)return false; int nz=0,ok=0;
        for(size_t i=0;i+8<=b.size();i+=8){ bool z=true; for(int k=0;k<8;k++)if(b[i+k])z=false; if(z)continue; nz++;
            unsigned char hi=b[i+7]; if(hi==0x3f||hi==0x40||hi==0xbf||hi==0xc0)ok++; }
        return nz>0 && ok==nz; }
    // decode bytes as `count` little-endian elements of `elsize` (or double if dbl) -> decimal initializer list
    static string decVals(const vector<unsigned char>& b,int elsize,bool dbl){ string r; size_t n=b.size()/elsize;
        for(size_t i=0;i<n;i++){ const unsigned char* p=&b[i*elsize];
            if(dbl){ double d; memcpy(&d,p,8); char x[40]; snprintf(x,sizeof x,"%g",d); double rt=0; sscanf(x,"%lf",&rt); if(rt!=d) snprintf(x,sizeof x,"%.17g",d); r+=x; }
            else { uint64_t u=0; for(int k=elsize-1;k>=0;k--) u=(u<<8)|p[k]; int64_t v=elsize==4?(int32_t)u:elsize==2?(int16_t)u:elsize==1?(int8_t)u:(int64_t)u; r+=std::to_string((long long)v); }
            if(i+1<n)r+=","; r+=(i%16==15&&i+1<n)?"\n    ":" "; }
        return r; }
    static uint64_t le64(const unsigned char* p){ uint64_t v=0; for(int k=7;k>=0;k--)v=(v<<8)|p[k]; return v; }
    bool inText(uint64_t va){ for(auto& s:secs) if(s.name=="__text" && va>=s.vaddr && va<s.vaddr+s.size) return true; return false; }
    static string hx64(uint64_t v){ char b[24]; snprintf(b,sizeof b,"0x%llxL",(unsigned long long)v); return b; }
    // a pointer value -> a readable element: a "string" it points to, a &function, or (void*)0x..
    string resolvePtr(uint64_t v){ if(!v) return "0"; string s=readCStr(v); if(s.size()>=1) return "\""+esc(s)+"\"";
        if(inText(v)){ auto it=syms.find(v); if(it!=syms.end()) return "(void*)&"+shortName(demangle(it->second)); }
        return "(void*)"+hx64(v); }
    bool ptrResolves(uint64_t v){ return v && (!readCStr(v).empty()||inText(v)||dataSecOf(v)); }
    // >= half of the 8-byte LE values resolve to a string/function/data address -> it's a pointer table
    bool isPtrTable(const vector<unsigned char>& b){ if(b.size()<8||b.size()%8)return false; int tot=0,ok=0;
        for(size_t i=0;i+8<=b.size();i+=8){ uint64_t v=le64(&b[i]); if(!v)continue; tot++; if(ptrResolves(v))ok++; } return tot>0&&ok*2>=tot; }
    string ptrVals(const vector<unsigned char>& b,bool& allStr){ allStr=true; string r; size_t n=b.size()/8;
        for(size_t i=0;i<n;i++){ string e=resolvePtr(le64(&b[i*8])); if(e.empty()||e[0]!='"')allStr=false; r+=(e.empty()?"(void*)0":e); if(i+1<n)r+=","; r+="\n    "; } return r; }
    // a blob that starts like an ARM64 prologue (stp ..,[sp..] / sub sp,sp) is code, not data
    static bool looksCode(const vector<unsigned char>& b){ if(b.size()<4)return false; uint32_t w=b[0]|(b[1]<<8)|(b[2]<<16)|((uint32_t)b[3]<<24);
        return (w&0xFFC003E0u)==0xA9000000u || (w&0xFF8003FFu)==0xD10003FFu; }
    // 8-byte elements (long/pointer): high dwords mostly 0 (small longs) or all the same (relocated pointers) -> not int[2N]
    static bool looks8byte(const vector<unsigned char>& b){ if(b.size()<16||b.size()%8)return false; int tot=0,zhi=0,same=0; uint32_t f=0; bool fs=false;
        for(size_t i=0;i+8<=b.size();i+=8){ uint32_t hi=b[i+4]|(b[i+5]<<8)|(b[i+6]<<16)|((uint32_t)b[i+7]<<24); tot++; if(!hi)zhi++; if(!fs){f=hi;fs=true;} if(hi==f)same++; }
        return tot>=2 && (zhi*2>=tot || (f&&same==tot)); }
    static bool looksStringPool(const vector<unsigned char>& b){ if(b.size()<4)return false; int pr=0,np=0,run=0,mxrun=0; bool nul=false;
        for(unsigned char c:b){ if(c==0){nul=true;run=0;continue;} np++; bool p=(c>=0x20&&c<0x7f)||c=='\n'||c=='\t'; if(p){pr++;run++;mxrun=std::max(mxrun,run);} else run=0; }
        return nul && np>=3 && mxrun>=3 && pr*5>=np*4; }   // a real string run, not a double's 2 stray printable bytes
    // emit a byte buffer as a C string literal. Non-printables use 3-digit OCTAL (\NNN) — never \xNN (greedy)
    // and never bare \0 (greedy when a digit follows: `\0`+'2' parses as \02). NUL = \000.
    static string poolStr(const vector<unsigned char>& b){ string r="\""; size_t last=b.size(); while(last>0&&b[last-1]==0)last--;   // drop trailing NUL padding
        for(size_t i=0;i<last;i++){ unsigned char c=b[i]; if(c=='"'||c=='\\'){r+='\\';r+=c;} else if(c=='\n')r+="\\n"; else if(c=='\t')r+="\\t"; else if(c>=0x20&&c<0x7f)r+=(char)c; else { char x[8]; snprintf(x,sizeof x,"\\%03o",c); r+=x; } } return r+"\""; }
    void buildDataTemplates(){
        std::map<string,uint64_t> toks;
        for(auto& F:fns) for(auto& l:F.body){ size_t i=0; while((i=l.find("g_",i))!=string::npos){ size_t j=i+2; while(j<l.size()&&isxdigit((unsigned char)l[j]))j++;
            string nm=l.substr(i,j-i); uint64_t va=strtoull(l.substr(i+2,j-(i+2)).c_str(),0,16);
            if(nm.size()>2 && !strRepl.count(nm) && !strRepl.count("*"+nm)) toks[nm]=va; i=j; } }
        for(auto& kv:toks){ const string& gx=kv.first; uint64_t va=kv.second; const Section* sec=dataSecOf(va); if(!sec)continue;   // skip code / external
            long N=-1; string dest, destFn;                                  // exact extent + memcpy destination local
            for(auto& F:fns){ for(auto& l:F.body) for(const string& pre:{string(", "),string(", &")}){ size_t gp=l.find(pre+gx+", "); if(gp==string::npos)continue;
                size_t np=gp+pre.size()+gx.size()+2,ne=np; while(ne<l.size()&&isdigit((unsigned char)l[ne]))ne++; if(ne>np){ long n=atol(l.substr(np,ne-np).c_str()); if(n>0&&n<=65536){ N=n;
                    size_t mp=l.find("memcpy("); if(mp!=string::npos&&mp<gp){ size_t a=mp+7; while(a<l.size()&&(l[a]==' '||l[a]=='&'))a++; size_t e=a; while(e<l.size()&&(isalnum((unsigned char)l[e])||l[e]=='_'))e++; dest=l.substr(a,e-a); destFn=F.name; } } } } if(N>0)break; }
            if(N<=0){ uint64_t bound=sec->vaddr+sec->size;            // no memcpy size hint: bound the extent by the NEXT data global in this section (packed) — not an arbitrary 64
                for(auto& kv2:toks){ if(kv2.second>va && kv2.second<bound && dataSecOf(kv2.second)==sec) bound=kv2.second; }
                N=(long)std::min<uint64_t>(bound-va, 4096); }
            if(N<=0||N>65536)continue;
            vector<unsigned char> b=readBytes(va,(size_t)N); if(b.empty())continue;
            if(b.size()>=4 && b[0]==0xcf&&b[1]==0xfa&&b[2]==0xed&&b[3]==0xfe) continue;   // Mach-O header magic at file start — not data
            if(looksCode(b)) continue;                                       // a function, not data -> leave as a symbol (stubGlobals floors it)
            // a short C-string buildStrings missed (it requires >=2 chars) -> emit a real char[] string, not a numeric array
            if(sec->name=="__cstring"){ size_t e=0; while(e<b.size()&&b[e])e++; vector<unsigned char> sb(b.begin(),b.begin()+e);
                if(!sb.empty()){ int rn=++roleCount["str"]; string gname="str"+(rn>1?std::to_string(rn):""); strRepl[gx]=gname;
                    dataDecls.push_back({gname,"static const char "+gname+"[] = "+poolStr(sb)+";"}); continue; } }
            bool indexed=false, scalarDeref=false, byteAlias=false; int stride=0;   // how is the global used? (stride from `*(gx + (i << k))`)
            for(auto& F:fns) for(auto& l:F.body){ size_t p=l.find("*("+gx+" + ");
                if(p!=string::npos){ indexed=true; size_t sh=l.find(" << ",p), cl=l.find(')',p); if(sh!=string::npos&&sh<cl){ int k=atoi(l.c_str()+sh+4); if(k>=0&&k<=4)stride=std::max(stride,1<<k); } else if(stride==0)stride=1; }
                if(l.find(gx+"[")!=string::npos)indexed=true;
                if(l.find("*"+gx)!=string::npos && l.find("*("+gx)==string::npos)scalarDeref=true; }
            // POINTER-ALIAS access: `obj = gx;` then `obj->fN` / `*(obj + i)` — a struct/byte RECORD array the stride
            // detector misses (it walks via the alias, not `gx[i]`). Field-deref through the alias = a byte/record array.
            for(auto& F:fns){ set<string> alias; for(auto& l:F.body){ string t=trim(l); size_t eq=t.find(" = ");
                if(eq!=string::npos && t.back()==';'){ string rhs=t.substr(eq+3); rhs.pop_back(); string lhs=t.substr(0,eq); if(rhs==gx && isName(lhs)) alias.insert(lhs); } }
                for(auto& a:alias) for(auto& l:F.body){ if(l.find(a+"->")!=string::npos || (l.find("*("+a+" + ")!=string::npos && l.find("<< ")==string::npos)){ byteAlias=true; indexed=true; } } }
            if(byteAlias && stride==0) stride=1;   // record/byte array accessed through a pointer alias -> unsigned char[]
            // POINTER TABLE -> const char*[] (string table, the biggest readability win) / void*[]
            if(N%8==0 && b.size()>=8 && isPtrTable(b)){ bool allStr; string pv=ptrVals(b,allStr); int cnt=(int)(b.size()/8);
                string ety=allStr?"const char*":"void*", role=allStr?"strings":"ptrs"; int rn=++roleCount[role]; string gname=role+(rn>1?std::to_string(rn):"");
                strRepl[gx]=gname; dataDecls.push_back({gname,"static "+ety+" "+gname+"["+std::to_string(cnt)+"] = {\n    "+pv+"\n};"}); continue; }
            // SCALAR (before string-pool: a double like 100.0 is mostly NULs + 2 printable bytes — not a string)
            if(dest.empty() && scalarDeref && !indexed){ vector<unsigned char> sb=readBytes(va,8); string ty,val;
                if(sb.size()==8 && looksDouble(sb)){ double d; memcpy(&d,sb.data(),8); char x[40]; snprintf(x,sizeof x,"%g",d); double rt=0; sscanf(x,"%lf",&rt); if(rt!=d)snprintf(x,sizeof x,"%.17g",d); ty="double"; val=x; }
                else if(sb.size()>=4){ ty="int"; val=std::to_string((int32_t)(sb[0]|(sb[1]<<8)|(sb[2]<<16)|((uint32_t)sb[3]<<24))); }
                if(!ty.empty()){ int rn=++roleCount["k"]; string gname="k"+(rn>1?std::to_string(rn):""); strRepl["*"+gx]=gname; strRepl[gx]="&"+gname;
                    dataDecls.push_back({gname,"static const "+ty+" "+gname+" = "+val+";"}); continue; } }
            // STRING POOL (NUL-separated printable, not memcpy'd, not a stride-typed numeric) -> char[] (no explicit size)
            if(dest.empty() && stride==0 && looksStringPool(b)){ int rn=++roleCount["chars"]; string gname="chars"+(rn>1?std::to_string(rn):"");
                strRepl[gx]=gname; dataDecls.push_back({gname,"static const char "+gname+"[] = "+poolStr(b)+";"}); continue; }
            // NUMERIC ARRAY — element size from the access STRIDE when indexed (reliable), else size divisibility
            string eltype="unsigned char"; int elsize=1; bool dbl=false;
            if(stride==8){ dbl=looksDouble(b); eltype=dbl?"double":"long"; elsize=8; }
            else if(stride==4){ eltype="int"; elsize=4; }
            else if(stride==2){ eltype="short"; elsize=2; }
            else if(stride==1){ eltype="unsigned char"; elsize=1; }
            else if(N%8==0 && looksDouble(b)){ eltype="double"; elsize=8; dbl=true; }
            else if(N%8==0 && looks8byte(b)){ eltype="long"; elsize=8; }     // 8-byte elements -> long[], not int[2N]
            else if(N%4==0){ eltype="int"; elsize=4; }
            else if(N%2==0){ eltype="short"; elsize=2; }
            if((long)b.size()%elsize!=0){ eltype="unsigned char"; elsize=1; dbl=false; }   // never mis-size the copy
            int count=(int)(b.size()/elsize);
            string vals = (elsize==1&&!dbl)? hexBytes(b) : decVals(b,elsize,dbl);
            int sq=0; while(sq*sq<count)sq++;                          // readable names by role
            string role = (eltype=="int"&&count>1&&sq*sq==count)?"matrix":(eltype=="int"||eltype=="short")?"table":eltype=="double"?"coeffs":"data";
            int rn=++roleCount[role]; string sfx=rn>1?std::to_string(rn):"";
            string gname = dest.empty()? role+sfx : role+sfx+"_init", dname=role+sfx;   // memcpy'd: global=const source, local=the role
            strRepl[gx]=gname;                                          // rename the global wherever it's referenced
            dataDecls.push_back({gname,"static const "+eltype+" "+gname+"["+std::to_string(count)+"] = {\n    "+vals+"\n};"});
            if(!dest.empty()&&!destFn.empty()){ dataHint[destFn].push_back(dest+"="+eltype+"["+std::to_string(count)+"]"); destRename[destFn][dest]=dname; } }
    }
    void emitData(const string& fn, std::map<string,string>& vmap, const char* ind){   // @data hint -> collapse types the dest local
        auto it=dataHint.find(fn); if(it==dataHint.end()||it->second.empty()) return; string s=string(ind)+"// @data";
        for(auto& d:it->second){ size_t eq=d.find('='); string nm=d.substr(0,eq); string mapped=vmap.count(nm)?vmap[nm]:nm; s+=" "+mapped+"="+d.substr(eq+1); }
        printf("%s\n",s.c_str()); }
    string humanizeStr(string s){ for(auto& kv:strRepl)s=replaceAll(s,kv.first,kv.second); return s; }

    // ── readability passes on a function's final statement lines ─────────────
    static bool isTmpName(const string& t){ if(t.size()<2||(t[0]!='v'&&t[0]!='t'))return false; for(size_t i=1;i<t.size();i++) if(!isdigit((unsigned char)t[i]))return false; return true; }
    static int countTok(const vector<string>& body, const string& name){ int c=0; for(auto& l:body){ size_t p=0; while((p=l.find(name,p))!=string::npos){ bool lb=(p==0||!isIdent(l[p-1])), rb=(p+name.size()>=l.size()||!isIdent(l[p+name.size()])); if(lb&&rb)c++; p+=name.size(); } } return c; }
    static string leadWS(const string& l){ size_t a=l.find_first_not_of(" "); return a==string::npos?"":l.substr(0,a); }
    // dead-store + dead-temp elimination + single-use temp inlining (copy propagation) -> human-readable C
    void cleanupBody(vector<string>& body){
        for(int round=0; round<8; round++){ bool changed=false;
            for(size_t i=0;i<body.size();i++){
                string ind=leadWS(body[i]); string s=trim(body[i]);
                size_t eq=s.find(" = "); if(eq==string::npos||s.empty()||s.back()!=';') continue;
                string lhs=trim(s.substr(0,eq)); string rhs=trim(s.substr(eq+3)); if(!rhs.empty()&&rhs.back()==';')rhs.pop_back(); rhs=trim(rhs);
                if(!isTmpName(lhs)||lhs[0]=='\0') continue;
                bool isT=lhs[0]=='t'; int total=countTok(body,lhs), uses=total-1; bool hasCall=rhs.find('(')!=string::npos;
                if(uses==0){                                           // dead assignment
                    if(hasCall){ body[i]=ind+rhs+";"; changed=true; }   //  keep the call (side effects), drop the dead temp
                    else if(isT||total==1){ body.erase(body.begin()+i); i--; changed=true; }  // pure & unused -> drop
                    continue;
                }
                if(isT && uses==1){                                    // single-use SSA temp -> inline it
                    int u=-1; for(size_t j=i+1;j<body.size();j++){ if(countTok({body[j]},lhs)>0){ u=(int)j; break; } }
                    if(u<0) continue;
                    bool safe=true; for(int j=(int)i+1;j<u;j++){ if(body[j].find('{')!=string::npos||body[j].find('}')!=string::npos){safe=false;break;} }
                    if(!safe) continue;
                    string inl=replaceTok(body[u],lhs,"("+rhs+")"); if(trim(inl).size()>118) continue;   // don't over-nest
                    body[u]=inl; body.erase(body.begin()+i); i--; changed=true; continue;
                }
            }
            if(!changed) break;
        }
    }
    void printBody(vector<string> lines, const char* ind){ cleanupBody(lines); for(auto& l:lines) printf("%s%s\n", ind, l.c_str()); }

    // emit a width hint (consumed + stripped by ember-collapse's declare-locals) so recovered
    // scalar locals get their REAL C type (int vs long vs char/short) instead of a blanket long.
    void emitWidths(FnResult& F, std::map<string,string>& vmap, const char* ind){
        if(!F.sfieldW.empty()){ string s2=string(ind)+"// @sfields";   // struct-array field offset=width (opt-in precise typing; collapse strips this comment)
            for(auto& kv:F.sfieldW) s2+=" "+std::to_string(kv.first)+"="+std::to_string(kv.second); printf("%s\n", s2.c_str()); }
        if(!F.elemHint.empty()){ string s3=string(ind)+"// @elem"; bool any=false;   // pointer var -> element size, so collapse folds p[i<<k] -> p[i]
            for(auto& kv:F.elemHint){ if(kv.second<=0)continue; string n=vmap.count(kv.first)?vmap[kv.first]:kv.first; s3+=" "+n+"="+std::to_string(kv.second); any=true; }
            if(any) printf("%s\n", s3.c_str()); }
        if(F.slotW.empty()) return; string s=string(ind)+"// @widths";
        for(auto& kv:F.slotW){ string n=vmap.count(kv.first)?vmap[kv.first]:kv.first; s+=" "+n+"="+std::to_string(kv.second); }
        printf("%s\n", s.c_str());
    }
    void emitAll(){
        buildStrings();
        for(auto& d:strDecls) printf("const char* %s = \"%s\";\n",d.first.c_str(),d.second.c_str());
        if(!strDecls.empty())printf("\n");
        buildDataTemplates();                                          // real data globals (arrays/blobs) -> initialized C
        for(auto& d:dataDecls) printf("%s\n",d.second.c_str());
        if(!dataDecls.empty())printf("\n");
        // ── FUNCTION-BEHAVIOR NAMING ── a stripped `sub_<addr>` whose dominant action is printing or returning a
        // string literal is named for what it DOES: puts("hello world") -> print_hello_world, return "online" ->
        // get_online. Only small wrapper functions (so an incidental printf in a big function doesn't mislabel it).
        { auto strSymIn=[&](const string& l)->string{ size_t i=0; while((i=l.find("g_",i))!=string::npos){ if(i==0||!isIdent(l[i-1])){ size_t j=i+2; while(j<l.size()&&isxdigit((unsigned char)l[j]))j++; auto it=strRepl.find(l.substr(i,j-i)); if(it!=strRepl.end()){ string s=it->second; if(!s.empty()&&s[0]=='&')s=s.substr(1); return s; } i=j; } else i++; } return ""; };   // a g_<addr> string ref -> its s_name (strRepl is built, body not yet rewritten)
          std::map<string,string> fnRename; set<string> usedFn; for(auto& F:fns) if(F.name.rfind("sub_",0)!=0) usedFn.insert(F.name);
          for(auto& F:fns){ if(F.name.rfind("sub_",0)!=0||F.body.size()>12) continue;
              string printSym, retSym;
              for(auto& l:F.body){ string t=trim(l); string sym=strSymIn(t); if(sym.empty())continue;
                  bool isPrint=(t.find("puts(")!=string::npos||t.find("printf(")!=string::npos||t.find("fputs(")!=string::npos||t.find("fprintf(")!=string::npos);
                  if(isPrint&&printSym.empty())printSym=sym; if(t.rfind("return",0)==0&&retSym.empty())retSym=sym; }
              string base; if(!printSym.empty()) base="print_"+(printSym.rfind("s_",0)==0?printSym.substr(2):printSym);
              else if(!retSym.empty()) base="get_"+(retSym.rfind("s_",0)==0?retSym.substr(2):retSym); else continue;
              if(base.size()>26)base=base.substr(0,26); while(!base.empty()&&base.back()=='_')base.pop_back(); if(base.size()<3)continue;
              string fin=base; int s=2; while(usedFn.count(fin))fin=base+std::to_string(s++); usedFn.insert(fin); fnRename[F.name]=fin; }
          if(!fnRename.empty()) for(auto& F:fns){ for(auto& kv:fnRename) for(auto& l:F.body) l=replaceTok(l,kv.first,kv.second);
              auto it=fnRename.find(F.name); if(it!=fnRename.end()) F.name=it->second; } }
        // RETURN-TYPE WIDTH: a callee whose result every caller stores into a 32-bit slot returns `int`,
        // not `long` — so intermediate arithmetic on the result keeps the right width (fixed-point etc.).
        std::map<string,int> retW;
        for(auto& F:fns){ for(size_t i=0;i<F.body.size();i++){ const string& l=F.body[i]; size_t eq=l.find(" = "); if(eq==string::npos)continue;
            string lhs=trim(l.substr(0,eq)); if(lhs.size()<2||lhs[0]!='t'||!isdigit((unsigned char)lhs[1]))continue;       // tN = callee(...)
            string rhs=l.substr(eq+3); size_t par=rhs.find('('); if(par==string::npos)continue; string callee=trim(rhs.substr(0,par));
            if(callee.empty()||callee.find(' ')!=string::npos||!isName(callee))continue;
            for(size_t j=i+1;j<F.body.size();j++){ string m=trim(F.body[j]); size_t e2=m.find(" = "); if(e2==string::npos)continue;   // find the spill `vM = tN;`
                string r2=m.substr(e2+3); if(!r2.empty()&&r2.back()==';')r2.pop_back();
                if(trim(r2)==lhs){ string dst=trim(m.substr(0,e2)); int w=F.slotW.count(dst)?F.slotW[dst]:8;
                    auto it=retW.find(callee); if(it==retW.end())retW[callee]=w; else if(it->second!=w)retW[callee]=8; break; } } } }
        // VOID detection: a function that is CALLED but whose result is never captured by any caller (every
        // call is a bare `f(...)` statement, never `x = f(...)` or `g(f(...))`) returns void. clang leaves the
        // incoming param in x0 at `ret`, which we'd otherwise print as a bogus `return data;`.
        std::set<string> calledAny, resultUsed;
        auto tokIn=[&](const string& s,const string& tk){ size_t p=0; while((p=s.find(tk,p))!=string::npos){ bool lb=(p==0||!(isalnum((unsigned char)s[p-1])||s[p-1]=='_')); size_t af=p+tk.size(); bool rb=(af>=s.size()||!(isalnum((unsigned char)s[af])||s[af]=='_')); if(lb&&rb)return true; p=af; } return false; };
        for(auto& F:fns){ for(size_t i=0;i<F.body.size();i++){ string t=trim(F.body[i]);
            size_t eq=t.find(" = "); string lhs, rhs=(eq==string::npos)?t:t.substr(eq+3); if(eq!=string::npos)lhs=trim(t.substr(0,eq));
            size_t par=rhs.find('('); if(par==string::npos) continue; string callee=trim(rhs.substr(0,par)); if(callee.empty()||!isName(callee)) continue;
            calledAny.insert(callee);
            // The lift parks EVERY call's result in a temp (`tN = f()`), even when discarded — so "result used"
            // means: that temp/slot is actually READ on another line. A dead result temp => the call is void-use.
            if(eq==string::npos){ resultUsed.insert(callee); continue; }                        // a bare call expression (rare) — treat as used to be safe
            if(lhs.size()>=2 && (lhs[0]=='t'||lhs[0]=='v') && isdigit((unsigned char)lhs[1])){
                bool used=false; for(size_t j=0;j<F.body.size();j++){ if(j==i)continue; if(tokIn(F.body[j],lhs)){used=true;break;} }
                if(used) resultUsed.insert(callee);
            } else resultUsed.insert(callee); }                                                  // captured into a named local -> used
        }
        std::set<string> voidFn; for(auto& F:fns){ if(F.name=="main")continue; if(calledAny.count(F.name)&&!resultUsed.count(F.name)) voidFn.insert(F.name); }
        auto retTy=[&](const string& fn)->string{ if(voidFn.count(fn))return "void"; auto it=retW.find(fn); return it!=retW.end()?string(tyOf(it->second)):string("long"); };
        struct Cls{ string name; std::map<int,int> fields; vector<int> methods; };
        vector<Cls> classes; vector<int> clsOf(fns.size(),-1);
        auto compat=[](std::map<int,int>& a,std::map<int,int>& b){ for(auto& p:b){ auto it=a.find(p.first); if(it!=a.end()&&it->second!=p.second)return false; } return true; };
        for(size_t i=0;i<fns.size();i++){ FnResult& F=fns[i]; if(!F.isMethod||F.a0base.empty()||!F.fields.count(F.a0base))continue;   // only real members (name has ::) become methods
            auto& S=F.fields[F.a0base]; int ci=-1; for(size_t c=0;c<classes.size();c++) if(compat(classes[c].fields,S)){ci=(int)c;break;}
            if(ci<0){ ci=(int)classes.size(); classes.push_back({"Cls"+std::to_string(ci),{},{}}); }
            for(auto& p:S) classes[ci].fields[p.first]=p.second; classes[ci].methods.push_back((int)i); clsOf[i]=ci; }
        // ── method-ification: rewrite a free-function-style method call `tick(&v68)` -> `v68.tick()`
        // (or `tick(v68)` -> `v68->tick()`). Gated on an UNAMBIGUOUS method name (a member, not also a free
        // fn, not a ctor/dtor) and a SIMPLE lvalue receiver, so it never touches an ordinary call. ──
        std::set<string> methodNames, freeNames, classDemNames;
        for(size_t i=0;i<fns.size();i++){ if(clsOf[i]>=0) methodNames.insert(fns[i].name); else freeNames.insert(fns[i].name);
            string c=enclosingClass(fns[i].sig); if(!c.empty()){ size_t q=c.rfind("::"); classDemNames.insert(q==string::npos?c:c.substr(q+2)); } }
        auto splitTop=[](const string& s)->vector<string>{ vector<string> out; int d=0; size_t st=0; for(size_t i=0;i<s.size();i++){ char c=s[i]; if(c=='('||c=='['||c=='{')d++; else if(c==')'||c==']'||c=='}')d--; else if(c==','&&d==0){ out.push_back(s.substr(st,i-st)); st=i+1; } } out.push_back(s.substr(st)); return out; };
        auto methodify=[&](const string& s)->string{ string out; size_t i=0;
            while(i<s.size()){
                if((isalpha((unsigned char)s[i])||s[i]=='_') && (i==0||!isIdent(s[i-1]))){ size_t e=i; while(e<s.size()&&isIdent(s[e]))e++; string nm=s.substr(i,e-i);
                    if(e<s.size()&&s[e]=='(' && methodNames.count(nm) && !freeNames.count(nm) && !classDemNames.count(nm) && nm[0]!='~'){
                        int d=0; size_t q=e; for(;q<s.size();q++){ if(s[q]=='(')d++; else if(s[q]==')'){ if(--d==0)break; } }
                        if(q<s.size()){ vector<string> args=splitTop(s.substr(e+1,q-e-1));
                            if(!args.empty()){ string recv=trim(args[0]); string op;
                                while(recv.size()>2&&recv.front()=='('&&recv.back()==')'){ int d=0; bool wrap=true; for(size_t k=0;k<recv.size();k++){ if(recv[k]=='(')d++; else if(recv[k]==')'){ if(--d==0&&k!=recv.size()-1){wrap=false;break;} } } if(!wrap)break; recv=trim(recv.substr(1,recv.size()-2)); }   // (&v60) -> &v60
                                if(recv.size()>1&&recv[0]=='&'){ string r=trim(recv.substr(1)); if(isName(r)){ op="."; recv=r; } }
                                else if(isName(recv)){ op="->"; }
                                if(!op.empty()){ string rest; for(size_t k=1;k<args.size();k++){ rest+=(k>1?", ":"")+trim(args[k]); }
                                    out += recv+op+nm+"("+rest+")"; i=q+1; continue; } } } }
                    out += nm; i=e; continue; }
                out += s[i]; i++; }
            return out; };
        int recN=0; set<string> usedCls;
        for(auto& C:classes){
            vector<std::pair<string,const vector<string>*>> ctx; vector<string> mnames;   // (receiver, body) per method + method names
            for(int mi:C.methods){ ctx.push_back({fns[mi].a0base,&fns[mi].body}); mnames.push_back(fns[mi].name); }
            std::map<int,string> fn, fty; fieldRoles(C.fields,ctx,fn,fty);                 // best-guess names + C types
            string cls; for(int mi:C.methods){ cls=enclosingClass(fns[mi].sig); if(!cls.empty())break; }   // REAL class name from `Class::method` if demangled
            string fallback="Record"+std::to_string(recN); string guess = !cls.empty()?cls:guessTypeName(fn,mnames,fallback); if(guess==fallback)recN++;
            string cn=guess; for(int s=2; usedCls.count(cn); s++) cn=guess+std::to_string(s); usedCls.insert(cn); C.name=cn;   // unique class name
            std::map<string,string> fmap; for(auto& fo:C.fields){ fmap[fieldId(fo.first)]=fn[fo.first]; }
            printf("class %s {\npublic:\n",C.name.c_str()); emitStructBody(C.fields,fn,&fty,C.name);
            for(int mi:C.methods){ FnResult& F=fns[mi]; auto vmap=nameVars(F.body,F.maxArg); mergeDataNames(F.name,vmap); auto mpw=paramWidths(F);
                string params; for(int k=1;k<=F.maxArg;k++){ if(k>1)params+=", "; string ab="a"+std::to_string(k); params+=string(tyOf(mpw.count(k)?mpw[k]:8))+" "+(vmap.count(ab)?vmap[ab]:("arg"+std::to_string(k))); }
                if(!F.sig.empty()) printf("    // %s\n",F.sig.c_str());   // real demangled C++ signature (doc comment; keeps the compilable body intact)
                // INSIDE the class body the method name must be UNQUALIFIED — `tick`, not `Counter::tick`
                // (extra-qualification is a hard compile error). Recover ctor (name==class) / dtor (~) too.
                string mbase=F.name; { size_t cc=mbase.rfind("::"); if(cc!=string::npos) mbase=mbase.substr(cc+2); }
                string clsBase=cls; { size_t q=clsBase.rfind("::"); if(q!=string::npos) clsBase=clsBase.substr(q+2); }
                bool isDtor = !mbase.empty()&&mbase[0]=='~';
                bool isCtor = !isDtor && !clsBase.empty() && mbase==clsBase;
                string rty=retTy(F.name);
                if(isCtor)      printf("    %s(%s) {\n", C.name.c_str(), params.c_str());            // ctor: no return type, class-renamed name
                else if(isDtor) printf("    ~%s(%s) {\n", C.name.c_str(), params.c_str());           // dtor: ~Class
                else            printf("    %s %s(%s) {\n", rty.c_str(), mbase.c_str(), params.c_str());
                emitWidths(F,vmap,"    "); emitData(F.name,vmap,"    ");
                vector<string> lines; for(auto& l:F.body){ string t=replaceTok(l,F.a0base,"this"); if(trim(t)=="this = a0;")continue; lines.push_back(methodify(applyMap(applyMap(humanizeStr(t),fmap),vmap))); }
                // ctor/dtor return nothing; a void method's `return <garbage>;` must become a bare `return;`
                bool noVal = isCtor||isDtor||rty=="void";
                if(noVal) for(auto& l:lines){ string t=trim(l);
                    if(t.rfind("return ",0)==0 && t.back()==';'){ string ind=l.substr(0,l.find_first_not_of(" \t")==string::npos?0:l.find_first_not_of(" \t")); l=ind+"return;"; } }
                printBody(lines,"    ");
                printf("    }\n"); }
            printf("};\n\n"); }
        // a synthesized struct is only meaningful for a real aggregate base (an arg, a
        // stack local, or a named pointer). Bases that are call temps (tN) or raw
        // callee-saved registers (xN/wN) are decompiler artifacts — don't emit junk
        // one-field "struct S_main_t1 { long f0; };" decls for them.
        auto isArtifactBase=[](const string& s){ if(s.size()<2) return false; char c=s[0]; if(c!='t'&&c!='x'&&c!='w') return false; for(size_t i=1;i<s.size();i++) if(!isdigit((unsigned char)s[i])) return false; return true; };
        auto isJunkBase=[](const string& b){ static const set<string> J={"cin","cout","cerr","clog","wcin","wcout","wcerr","wclog","stdscr","curscr","stdin","stdout","stderr"}; return J.count(b)>0||b.rfind("__",0)==0||b.rfind("operator",0)==0; };   // iostream/ncurses globals + compiler internals -> not user structs
        // a base is REAL if it's a named local/arg, OR a register/temp with >=2 fields (a reg holding a struct ptr).
        auto realBase=[&](const string& b,std::map<int,int>& f){ if(isJunkBase(b))return false; return !isArtifactBase(b) || f.size()>=2; };
        // ── CROSS-FUNCTION STRUCT UNIFICATION ── structs with an IDENTICAL layout (offset:width set) share ONE name
        // + ONE set of field names, emitted once on first encounter (functions are printed in order, so the decl
        // always precedes every use). The shared name is a behavioural guess (Node/Buffer/…) or StructN. ──
        std::map<string,std::tuple<string,std::map<int,string>,std::map<int,string>>> byLayout; int structN=0;
        auto layoutKey=[](std::map<int,int>& f){ string k; for(auto& p:f) k+=std::to_string(p.first)+","+std::to_string(p.second)+";"; return k; };
        for(size_t i=0;i<fns.size();i++){ if(clsOf[i]>=0)continue; FnResult& F=fns[i];
            auto vmap=nameVars(F.body,F.maxArg); mergeDataNames(F.name,vmap);   // a0 now role-named by nameVars (no arg0 override)
            std::map<string,std::map<int,string>> bNames; std::map<string,string> baseStruct;   // per-base field names (body rename) + shared struct name
            // DEREF TYPING: a base with exactly ONE field at offset 0 is a SCALAR POINTER (char*/short*/int*/long*),
            // not a struct — `p->f0` is really `*p`. Demote it so strcpy/strlen/array walks read like real code
            // instead of inventing `struct Struct0 { char value; }`. (Real aggregates have >=2 fields or a non-0 field.)
            std::map<string,string> scalarPtr;   // base -> "char*"/"int*"/...
            for(auto& kv:F.fields){ if(isJunkBase(kv.first))continue; auto& f=kv.second;
                if(f.size()==1 && f.begin()->first==0){ int w=f.begin()->second; if(w==1||w==2||w==4||w==8){
                    scalarPtr[kv.first]=(string)(w==1?"char":w==2?"short":w==4?"int":"long")+"*"; } } }
            for(auto& kv:F.fields){ if(scalarPtr.count(kv.first)) continue;   // demoted scalar pointers don't get a struct
                if(!realBase(kv.first,kv.second))continue; string lk=layoutKey(kv.second);
                if(!byLayout.count(lk)){ std::map<int,string> nm,ty; vector<std::pair<string,const vector<string>*>> ctx{{kv.first,&F.body}}; fieldRoles(kv.second,ctx,nm,ty);
                    string fb="Struct"+std::to_string(structN); string g=guessTypeName(nm,{},fb); if(g==fb)structN++;
                    string cn=g; for(int s=2; usedCls.count(cn); s++)cn=g+std::to_string(s); usedCls.insert(cn);
                    printf("struct %s {\n",cn.c_str()); emitStructBody(kv.second,nm,&ty,cn); printf("};\n");
                    byLayout[lk]=std::make_tuple(cn,nm,ty); }
                bNames[kv.first]=std::get<1>(byLayout[lk]); baseStruct[kv.first]=std::get<0>(byLayout[lk]); }
            vector<string> lines; if(F.name=="main"){ vmap["a0"]="argc"; vmap["a1"]="argv"; }
            for(auto& l:F.body){ string t=humanizeStr(l);
                for(auto& bn:bNames){ const string& base=bn.first; auto& nm=bn.second;
                    for(auto& fo:F.fields[base]) t=replaceTok(t, base+"->"+fieldId(fo.first), base+"->"+nm[fo.first]); }   // base->fN -> base->name (boundary-safe)
                for(auto& sp:scalarPtr) t=replaceTok(t, sp.first+"->"+fieldId(0), "(*"+sp.first+")");   // scalar pointer: p->f0 -> (*p)
                lines.push_back(methodify(applyMap(t,vmap))); }
            auto emitTypes=[&](const char* ind){ string ts;
                for(auto& b:baseStruct){ string v=vmap.count(b.first)?vmap[b.first]:b.first; ts+=" "+v+"="+b.second; }
                for(auto& b:scalarPtr){ string v=vmap.count(b.first)?vmap[b.first]:b.first; ts+=" "+v+"="+b.second; }
                if(!ts.empty())printf("%s// @types%s\n",ind,ts.c_str()); };   // var -> type (collapse: struct name -> `struct X*`, `char*` verbatim)
            if(F.name=="main"){ printf("int main(int argc, char** argv) {\n"); emitWidths(F,vmap,""); emitData(F.name,vmap,""); emitTypes(""); printBody(lines,""); printf("}\n\n"); continue; }
            // TYPE PROPAGATION (G2): an arg used as `argK->fN`, OR copied to a stack local that carries
            // the `->fN` accesses (`vJ = aK;` at -O0), is that struct's pointer — not a bare long.
            std::map<string,string> argStruct, argScalar;            // "aK" -> struct name / scalar-ptr type (via a copy `base = aK`)
            for(auto& kv:baseStruct){ for(auto& l:F.body){ string tl=trim(l); for(int k=0;k<=F.maxArg;k++){ char a[6]; snprintf(a,6,"a%d",k);
                    if(tl==kv.first+" = "+a+";") argStruct[a]=kv.second; } } }
            for(auto& kv:scalarPtr){ for(auto& l:F.body){ string tl=trim(l); for(int k=0;k<=F.maxArg;k++){ char a[6]; snprintf(a,6,"a%d",k);
                    if(tl==kv.first+" = "+a+";") argScalar[a]=kv.second; } } }
            auto pw=paramWidths(F);
            string params; for(int k=0;k<=F.maxArg;k++){ if(k)params+=", "; string ab="a"+std::to_string(k);
                string pn = vmap.count(ab)? vmap[ab] : ("arg"+std::to_string(k));   // role-based param name
                if(baseStruct.count(ab)) params+="struct "+baseStruct[ab]+"* "+pn;
                else if(scalarPtr.count(ab)) params+=scalarPtr[ab]+" "+pn;
                else if(argStruct.count(ab)) params+="struct "+argStruct[ab]+"* "+pn;
                else if(argScalar.count(ab)) params+=argScalar[ab]+" "+pn;
                else params+=string(tyOf(pw.count(k)?pw[k]:8))+" "+pn; }   // scalar param: width-typed (int/char/short) via its spill slot, else long
            if(!F.sig.empty()) printf("// %s\n",F.sig.c_str());   // real demangled C++ signature (doc comment)
            printf("%s %s(%s) {\n",retTy(F.name).c_str(),F.name.c_str(),applyMap(params,vmap).c_str());   // return type inferred from caller usage (int vs long)
            if(voidFn.count(F.name)) for(auto& l:lines){ string t=trim(l);   // void: `return <expr>;` -> `return;` (drop the bogus x0-passthrough value)
                if(t.rfind("return ",0)==0 && t.back()==';'){ size_t a=l.find("return"); l=l.substr(0,a)+"return;"; } }
            emitWidths(F,vmap,""); emitData(F.name,vmap,""); emitTypes("");
            printBody(lines,"");
            printf("}\n\n"); }
    }

    void emit(int lo,int hi,int ind,uint64_t cont,uint64_t brk,uint64_t loopBrk=0){
        if(ind>200){                                                  // RECURSION-DEPTH GUARD: pathologically-nested control flow (e.g. Dtools' brute-forcer)
            for(int k=lo;k<hi;k++){ Block& B=blocks[k]; if(B.dead)continue;   // would overflow the stack — fall back to FLAT goto emission for this region (valid, just less structured)
                if(B.isTarget)label(ind,B.addr); emitStmts(B,ind);
                if(B.term==Block::RET)say(ind,"return "+B.ret+";");
                else if(B.term==Block::GOTO){ int ti=BI(B.tgtT);
                    if(ti>=0&&blocks[ti].inlineable){ emitStmts(blocks[ti],ind); say(ind,"return "+blocks[ti].ret+";"); }
                    else if(B.tgtT){ say(ind,"goto "+hx(B.tgtT)+";"); used.insert(B.tgtT); } }
                else if(B.term==Block::COND){ int ti=BI(B.tgtT);
                    if(ti>=0&&blocks[ti].inlineable&&blocks[ti].stmts.empty()) say(ind,"if ("+B.cond+") return "+blocks[ti].ret+";");
                    else if(ti>=0&&blocks[ti].inlineable){ say(ind,"if ("+B.cond+") {"); emitStmts(blocks[ti],ind+1); say(ind+1,"return "+blocks[ti].ret+";"); say(ind,"}"); }
                    else { say(ind,"if ("+B.cond+") goto "+hx(B.tgtT)+";"); used.insert(B.tgtT); } } }
            return; }
        int i=lo;
        while(i<hi){ if(blocks[i].dead){ Block& D=blocks[i];
                // a loop-exit / forward-jump (i=exitIdx/elseEnd) can land on a tail-dup'd return-leaf that
                // was marked dead. Nothing falls into it normally, so reaching it here IS a real exit path
                // that needs its return — emit it instead of silently skipping (the missing `return sum;`).
                if(D.inlineable&&D.term==Block::RET){ emitStmts(D,ind); say(ind,"return "+D.ret+";"); }   // tail-dup copies are per-path: don't gate on emittedBI (a prior jump-site copy must not suppress this exit's copy)
                i++; continue; } Block& B=blocks[i];
            int latch=-1; for(int j=i;j<hi;j++){ Block& C=blocks[j]; if(((C.term==Block::GOTO||C.term==Block::COND)&&BI(C.tgtT)==i))latch=j; }
            if(B.term==Block::COND && BI(B.tgtT)==i){   // SINGLE-BLOCK SELF-LOOP: back-edge to itself, test at the BOTTOM -> do { body } while(cond)
                uint64_t exitAddr=B.tgtF; int exitIdx=BI(exitAddr);              // the body IS this block's stmts; emitting it as a while-header left the loop empty
                if(B.isTarget)label(ind,B.addr);
                say(ind,"do {"); emitStmts(B,ind+1); say(ind,"} while ("+B.cond+");");
                i=(exitIdx>i?exitIdx:i+1); continue; }
            if(latch>=0&&B.term==Block::COND){ int tT=BI(B.tgtT),tF=BI(B.tgtF);   // while (test at top) — header is the conditional
                int exitIdx=(tT>latch||tT<lo)?tT:(tF>latch||tF<lo)?tF:latch+1; bool bodyIsFall=(tF>=lo&&tF<=latch);
                uint64_t exitAddr=(exitIdx>=0&&exitIdx<(int)blocks.size())?blocks[exitIdx].addr:brk;
                // does the body range [i+1,latch] carry any real statements? If not, the HEADER block holds the
                // loop body and the conditional exit is mid/bottom (clang -O0 `while(1){ body; if(c)break; }`):
                // emitting the header's stmts as a pre-header preamble would leave the loop empty. Render do/while.
                bool bodyHas=false; for(int j=i+1;j<=latch;j++){ if(j<0||j>=(int)blocks.size())continue; if(!blocks[j].dead&&!blocks[j].stmts.empty()){bodyHas=true;break;} }
                if(!bodyHas && !B.stmts.empty()){
                    if(B.isTarget)label(ind,B.addr);
                    say(ind,"do {"); emitStmts(B,ind+1); say(ind,"} while ("+(bodyIsFall?negate(B.cond):B.cond)+");");
                    for(int j=i+1;j<=latch;j++) if(j>=0&&j<(int)blocks.size()) emittedBI.insert(j);   // the back-edge plumbing is captured by the loop — keep the safety net from re-emitting it flat
                    i=(exitIdx>i?exitIdx:latch+1); continue; }
                if(B.isTarget)label(ind,B.addr); emitStmts(B,ind);
                say(ind,"while ("+(bodyIsFall?negate(B.cond):B.cond)+") {");
                emit(i+1,latch+1,ind+1,B.addr,0,exitAddr); say(ind,"}");                 // loop body: cont=header, loopBrk=exit
                i=(exitIdx>i?exitIdx:latch+1); continue; }   // forward-progress guard: a backward 'exit' would re-process the loop forever
            if(latch>=0&&latch>i&&B.term!=Block::COND&&blocks[latch].term==Block::COND&&BI(blocks[latch].tgtT)==i){   // do { body } while(cond) — test at the BOTTOM
                Block& Lt=blocks[latch]; uint64_t exitAddr=Lt.tgtF; int exitIdx=BI(exitAddr);
                say(ind,"do {"); emit(i,latch,ind+1,B.addr,0,exitAddr);
                if(Lt.isTarget)label(ind+1,Lt.addr);                                      // the latch can itself be a goto target — without its label that goto dangles
                emitStmts(Lt,ind+1); say(ind,"} while ("+Lt.cond+");");                  // latch's own stmts close the body; its cond is the test
                i=(exitIdx>i?exitIdx:latch+1); continue; }
            if(B.term==Block::COND){ int tT=BI(B.tgtT),tF=BI(B.tgtF);
                if(tT>i&&tF==i+1&&tT<=hi){ Block& last=blocks[tT-1]; int elseEnd=-1;
                    if(last.term==Block::GOTO){ int J=BI(last.tgtT); if(J>tT&&J<=hi)elseEnd=J; }
                    int joinIdx=elseEnd>=0?elseEnd:tT; uint64_t joinAddr=(joinIdx<(int)blocks.size())?blocks[joinIdx].addr:brk;
                    if(B.isTarget)label(ind,B.addr); emitStmts(B,ind);
                    say(ind,"if ("+negate(B.cond)+") {"); emit(i+1,tT,ind+1,cont,joinAddr,loopBrk);
                    if(elseEnd>=0){ say(ind,"} else {"); emit(tT,elseEnd,ind+1,cont,joinAddr,loopBrk); say(ind,"}"); i=elseEnd; }
                    else { say(ind,"}"); i=tT; } continue; } }
            if(B.isTarget)label(ind,B.addr); emitStmts(B,ind);
            if(B.term==Block::RET)say(ind,"return "+B.ret+";");
            else if(B.term==Block::GOTO){ uint64_t t=B.tgtT; int ti=BI(t);   // recover break/continue; only a SEQUENTIAL fall is silent
                if(ti>=0&&blocks[ti].inlineable){ emitStmts(blocks[ti],ind); say(ind,"return "+blocks[ti].ret+";"); }   // tail-dup return-leaf MUST win over break/continue/region-exit, else a leaf that coincides with the loop-break or if-join is silently dropped (the missing `return n;`)
                else if(!t||ti==i+1){}
                else if(loopBrk&&t==loopBrk)say(ind,"break;");
                else if(cont&&t==cont){ if(i+1<hi)say(ind,"continue;"); }                // a back-edge at the loop's end loops anyway -> no redundant trailing continue
                else if(t==brk||(ti>=0&&ti==hi)){}                                       // if-join / region-exit fall
                else{ say(ind,"goto "+hx(t)+";"); used.insert(t); } }
            else if(B.term==Block::COND){ uint64_t t=B.tgtT; int ti=BI(t);
                if(ti>=0&&blocks[ti].inlineable&&blocks[ti].stmts.empty()) say(ind,"if ("+B.cond+") return "+blocks[ti].ret+";");   // `if (c) return x;` — inlineable return-leaf wins over break/continue
                else if(ti>=0&&blocks[ti].inlineable){ say(ind,"if ("+B.cond+") {"); emitStmts(blocks[ti],ind+1); say(ind+1,"return "+blocks[ti].ret+";"); say(ind,"}"); }   // tail-dup the return-leaf into the taken side
                else if(loopBrk&&t==loopBrk)say(ind,"if ("+B.cond+") break;");
                else if(cont&&t==cont)say(ind,"if ("+B.cond+") continue;");
                else{ say(ind,"if ("+B.cond+") goto "+hx(t)+";"); used.insert(t); } }
            i++;
        }
    }
};

// ── NEON / Advanced-SIMD disassembly (the subset clang emits for auto-vectorized loops) ──
static string neonDisasm(uint32_t w){
    char b[96]; int Rd=w&0x1f, Rn=(w>>5)&0x1f, Rm=(w>>16)&0x1f;
    auto arr=[&](int size,int q){ static const char* T[4]={"b","h","s","d"}; int lanes=(64<<q)/(8<<size); char r[10]; snprintf(r,sizeof r,"v%d.%d%s",0,lanes,T[size]); return string(r); };
    auto V=[&](int r,int size,int q){ static const char* T[4]={"b","h","s","d"}; int lanes=(64<<q)/(8<<size); char s[12]; snprintf(s,sizeof s,"v%d.%d%s",r,lanes,T[size]); return string(s); };
    auto Sc=[&](int r,int size){ static const char* T[4]={"b","h","s","d"}; char s[8]; snprintf(s,sizeof s,"%s%d",T[size],r); return string(s); };
    auto G=[&](int r,bool x){ if(r==31)return string(x?"xzr":"wzr"); return (x?"x":"w")+std::to_string(r); };
    (void)arr;
    // scalar SIMD/FP load/store (unsigned offset) — ldr/str s/d/q/b/h
    if((w&0x3F000000u)==0x3D000000u){ int size=(w>>30)&3,opc=(w>>22)&3; bool load=opc&1; int realsz=(opc>=2)?4:size; uint64_t off=((w>>10)&0xfffu)<<realsz;
        string bn = (Rn==31)?string("sp"):("x"+std::to_string(Rn));
        snprintf(b,sizeof b,"%-6s %s, [%s, #0x%llx]",load?"ldr":"str",Sc(Rd,realsz).c_str(),bn.c_str(),(unsigned long long)off); return b; }
    // AdvSIMD modified immediate (movi/mvni) — distinguished from shift-by-imm by immh(bits22:19)==0
    if((w&0x9F800400u)==0x0F000400u && ((w>>19)&0xf)==0){ int q=(w>>30)&1,op=(w>>29)&1,cmode=(w>>12)&0xf; uint64_t imm=((w>>16)&7); imm=(imm<<5)|((w>>5)&0x1f);
        snprintf(b,sizeof b,"%-6s %s, #0x%llx",op?"mvni":"movi",V(Rd,(cmode>>1)==0?2:(cmode>>1)==2?1:0,q).c_str(),(unsigned long long)imm); return b; }
    // AdvSIMD across lanes: addv / uaddlv·saddlv / umaxv·smaxv / uminv·sminv
    if((w&0x9F3E0C00u)==0x0E300800u){ int q=(w>>30)&1,u=(w>>29)&1,size=(w>>22)&3,op=(w>>12)&0x1f;
        const char* m = op==0x1b?"addv" : op==0x03?(u?"uaddlv":"saddlv") : op==0x0a?(u?"umaxv":"smaxv") : op==0x1a?(u?"uminv":"sminv") : 0;
        if(m){ int dsz=(op==0x03)?size+1:size; snprintf(b,sizeof b,"%-6s %s, %s",m,Sc(Rd,dsz).c_str(),V(Rn,size,q).c_str()); return b; } }
    // AdvSIMD two-register misc: not/mvn, cmeq/cmlt/cmgt-vs-zero, xtn, neg, abs, rev, cnt, cmge0,cmle0
    if((w&0x9F3E0C00u)==0x0E200800u){ int q=(w>>30)&1,u=(w>>29)&1,size=(w>>22)&3,op=(w>>12)&0x1f;
        const char* m=0; bool vsZero=false;
        if(op==0x05&&u){ snprintf(b,sizeof b,"%-6s %s, %s","mvn",V(Rd,0,q).c_str(),V(Rn,0,q).c_str()); return b; }       // NOT/MVN (size=00)
        else if(op==0x08){ m=u?"cmge":"cmgt"; vsZero=true; } else if(op==0x09){ m=u?"cmle":"cmeq"; vsZero=true; }
        else if(op==0x0a&&!u){ m="cmlt"; vsZero=true; } else if(op==0x0b){ m=u?"neg":"abs"; }
        else if(op==0x12){ m=u?"sqxtun":"xtn"; } else if(op==0x04&&!u){ m="cls"; } else if(op==0x05&&!u){ m="cnt"; }
        else if(op==0x00){ m=u?"rev32":"rev64"; } else if(op==0x01&&!u){ m="rev16"; }
        if(m){ if(vsZero) snprintf(b,sizeof b,"%-6s %s, %s, #0",m,V(Rd,size,q).c_str(),V(Rn,size,q).c_str());
               else snprintf(b,sizeof b,"%-6s %s, %s",m,V(Rd,size,q).c_str(),V(Rn,size,q).c_str()); return b; } }
    // AdvSIMD permute: uzp1/uzp2/zip1/zip2/trn1/trn2
    if((w&0x9F200C00u)==0x0E000800u){ int q=(w>>30)&1,size=(w>>22)&3,op=(w>>12)&7;
        const char* m = op==1?"uzp1":op==5?"uzp2":op==2?"trn1":op==6?"trn2":op==3?"zip1":op==7?"zip2":0;
        if(m){ snprintf(b,sizeof b,"%-6s %s, %s, %s",m,V(Rd,size,q).c_str(),V(Rn,size,q).c_str(),V(Rm,size,q).c_str()); return b; } }
    // AdvSIMD copy: dup(element/gp), ins, umov, smov, mov(element)
    if((w&0x9FE08400u)==0x0E000400u){ int q=(w>>30)&1,imm5=(w>>16)&0x1f,imm4=(w>>11)&0xf,op=(w>>29)&1; int size=0; while(size<4 && !((imm5>>size)&1))size++;
        if(imm4==0&&!op){ snprintf(b,sizeof b,"dup    %s, %s[%d]",V(Rd,size,q).c_str(),Sc(Rn,size).c_str(),imm5>>(size+1)); return b; }
        if(imm4==1&&!op){ snprintf(b,sizeof b,"dup    %s, %s",V(Rd,size,q).c_str(),G(Rn,size==3).c_str()); return b; }
        if((imm4==5||imm4==7)&&!op){ snprintf(b,sizeof b,"%-6s %s, %s[%d]",imm4==5?"smov":"umov",G(Rd,size==3||(imm4==7&&size==2)).c_str(),Sc(Rn,size).c_str(),imm5>>(size+1)); return b; }
        if(op){ snprintf(b,sizeof b,"mov    %s[%d], %s[%d]",Sc(Rd,size).c_str(),imm5>>(size+1),Sc(Rn,size).c_str(),imm4>>size); return b; } }
    // AdvSIMD shift by immediate (immh!=0): ushll/sshll/shl/ushr/sshr
    if((w&0x9F800400u)==0x0F000400u && ((w>>19)&0xf)!=0){ int q=(w>>30)&1,u=(w>>29)&1,immh=(w>>19)&0xf,immb=(w>>16)&7,op=(w>>11)&0x1f; int size=immh>=8?3:immh>=4?2:immh>=2?1:0; int esz=8<<size; int sh=(immh<<3|immb);
        const char* m=0; bool widen=false; int shimm=0;
        if(op==0x0a){ m="shl"; shimm=sh-esz; } else if(op==0x00){ m=u?"ushr":"sshr"; shimm=2*esz-sh; }
        else if(op==0x14){ m=u?"ushll":"sshll"; widen=true; shimm=sh-esz; }
        if(m){ if(widen) snprintf(b,sizeof b,"%-6s %s, %s, #%d",m,V(Rd,size+1,1).c_str(),V(Rn,size,q).c_str(),shimm);
               else snprintf(b,sizeof b,"%-6s %s, %s, #%d",m,V(Rd,size,q).c_str(),V(Rn,size,q).c_str(),shimm); return b; } }
    // AdvSIMD three same: add/sub/cmeq/cmgt/cmge/cmhi/cmhs/cmtst/mul/and/orr/eor/bic/orn/bsl
    if((w&0x9E200400u)==0x0E200400u){ int q=(w>>30)&1,u=(w>>29)&1,size=(w>>22)&3,op=(w>>11)&0x1f;
        const char* m=0;
        switch(op){ case 0x10: m=u?"sub":"add"; break; case 0x06: m=u?"cmhi":"cmgt"; break; case 0x07: m=u?"cmhs":"cmge"; break;
            case 0x11: m=u?"cmeq":"cmtst"; break; case 0x13: m=u?"pmul":"mul"; break; case 0x01: m=u?"uqadd":"sqadd"; break;
            case 0x0c: m=u?"umax":"smax"; break; case 0x0d: m=u?"umin":"smin"; break; case 0x12: m=u?"mls":"mla"; break;
            case 0x03: { static const char* L0[4]={"and","bic","orr","orn"}; static const char* L1[4]={"eor","bsl","bit","bif"}; m=(u?L1:L0)[size];
                snprintf(b,sizeof b,"%-6s %s, %s, %s",m,V(Rd,0,q).c_str(),V(Rn,0,q).c_str(),V(Rm,0,q).c_str()); return b; } }
        if(m){ snprintf(b,sizeof b,"%-6s %s, %s, %s",m,V(Rd,size,q).c_str(),V(Rn,size,q).c_str(),V(Rm,size,q).c_str()); return b; } }
    // FP<->GP fmov (scalar lane 0 <-> general register)
    { uint32_t base=w&0xFFFFFC00u;
      if(base==0x1E260000u){ snprintf(b,sizeof b,"fmov   %s, %s",G(Rd,0).c_str(),Sc(Rn,2).c_str()); return b; }   // fmov Wd, Sn
      if(base==0x1E270000u){ snprintf(b,sizeof b,"fmov   %s, %s",Sc(Rd,2).c_str(),G(Rn,0).c_str()); return b; }   // fmov Sd, Wn
      if(base==0x9E660000u){ snprintf(b,sizeof b,"fmov   %s, %s",G(Rd,1).c_str(),Sc(Rn,3).c_str()); return b; }   // fmov Xd, Dn
      if(base==0x9E670000u){ snprintf(b,sizeof b,"fmov   %s, %s",Sc(Rd,3).c_str(),G(Rn,1).c_str()); return b; } } // fmov Dd, Xn
    // LD1/ST1 (one/multiple structures)
    if((w&0xBFBF0000u)==0x0C402000u||(w&0xBFBF0000u)==0x0C002000u){ int q=(w>>30)&1,size=(w>>10)&3; bool load=(w>>22)&1;
        string bn = (Rn==31)?string("sp"):("x"+std::to_string(Rn));
        snprintf(b,sizeof b,"%-6s {%s}, [%s]",load?"ld1":"st1",V(Rd,size,q).c_str(),bn.c_str()); return b; }
    char hb[24]; snprintf(hb,sizeof hb,"neon.0x%08x",w); return hb;
}

// textual AArch64 disassembly of one 32-bit word (for the `--disasm` listing / GUI Disassembly view)
static string a64disasm(uint32_t w, uint64_t addr, std::unordered_map<uint64_t,string>& syms){
    char b[160]; int Rd=w&0x1f, Rn=(w>>5)&0x1f, Rm=(w>>16)&0x1f; bool sf=(w>>31)&1;
    auto R=[&](int r,bool s)->string{ if(r==31)return s?"xzr":"wzr"; return (s?"x":"w")+std::to_string(r); };
    auto Rs=[&](int r,bool s)->string{ if(r==31)return "sp"; return (s?"x":"w")+std::to_string(r); };
    auto hx=[&](int64_t v)->string{ char t[24]; if(v<0)snprintf(t,sizeof t,"-0x%llx",(unsigned long long)(-v)); else snprintf(t,sizeof t,"0x%llx",(unsigned long long)v); return t; };
    auto sym=[&](uint64_t a)->string{ auto it=syms.find(a); if(it!=syms.end())return it->second; char t[20]; snprintf(t,sizeof t,"0x%llx",(unsigned long long)a); return t; };
    static const char* CC[]={"eq","ne","cs","cc","mi","pl","vs","vc","hi","ls","ge","lt","gt","le","al","nv"};
    if((w&0xFC000000u)==0x14000000u) return "b      "+sym(addr+(sext(w&0x3ffffff,26)<<2));
    if((w&0xFC000000u)==0x94000000u) return "bl     "+sym(addr+(sext(w&0x3ffffff,26)<<2));
    if((w&0xFF000010u)==0x54000000u){ snprintf(b,sizeof b,"b.%-4s %s",CC[w&0xf],sym(addr+(sext((w>>5)&0x7ffff,19)<<2)).c_str()); return b; }
    if((w&0x7E000000u)==0x34000000u){ snprintf(b,sizeof b,"%-6s %s, %s",((w>>24)&1)?"cbnz":"cbz",R(Rd,sf).c_str(),sym(addr+(sext((w>>5)&0x7ffff,19)<<2)).c_str()); return b; }
    if((w&0x7E000000u)==0x36000000u){ int bit=((w>>31)&1)<<5|((w>>19)&0x1f); snprintf(b,sizeof b,"%-6s %s, #%d, %s",((w>>24)&1)?"tbnz":"tbz",R(Rd,1).c_str(),bit,sym(addr+(sext((w>>5)&0x3fff,14)<<2)).c_str()); return b; }
    if((w&0xFFFFFC1Fu)==0xD65F0000u) return "ret";
    if((w&0xFFFFFC1Fu)==0xD63F0000u) return "blr    "+R(Rn,1);
    if((w&0xFFFFFC1Fu)==0xD61F0000u) return "br     "+R(Rn,1);
    if((w&0xFFFFFBFFu)==0xD65F0BFFu) return ((w>>10)&1)?"retab":"retaa";                                         // arm64e pointer-auth return (bit10 = A/B key)
    if((w&0xFEFFF800u)==0xD63F0800u){ snprintf(b,sizeof b,"%-6s %s",((w>>10)&1)?"blrab":"blraa",R(Rn,1).c_str()); return b; }   // BLRAA/BLRAB(Z) auth indirect call
    if((w&0xFEFFF800u)==0xD61F0800u){ snprintf(b,sizeof b,"%-6s %s",((w>>10)&1)?"brab":"braa",R(Rn,1).c_str()); return b; }     // BRAA/BRAB(Z) auth indirect branch
    if(w==0xD503201Fu) return "nop"; if(w==0xD503233Fu) return "paciasp"; if(w==0xD50323BFu) return "autiasp"; if(w==0xD503237Fu) return "pacibsp"; if(w==0xD50323FFu) return "autibsp";
    if((w&0xFFE0FC00u)==0xDAC10000u){ int o=(w>>10)&0x1f; const char* m=o==0?"pacia":o==1?"pacib":o==2?"pacda":o==3?"pacdb":o==8?"autia":o==9?"autib":o==10?"autda":o==11?"autdb":o==16?"xpaci":o==17?"xpacd":0;   // 1-source data-proc PAC sign/auth/strip
        if(m){ snprintf(b,sizeof b,"%-6s %s, %s",m,R(Rd,1).c_str(),Rs(Rn,1).c_str()); return b; } }
    if(((w>>23)&0x3f)==0x22){ bool sub=(w>>30)&1,S=(w>>29)&1,sh=(w>>22)&1; uint64_t i=(w>>10)&0xfff; if(sh)i<<=12;
        if(S&&Rd==31){ snprintf(b,sizeof b,"%-6s %s, #%s",sub?"cmp":"cmn",Rs(Rn,sf).c_str(),hx(i).c_str()); return b; }
        snprintf(b,sizeof b,"%-6s %s, %s, #%s",sub?(S?"subs":"sub"):(S?"adds":"add"),Rs(Rd,sf).c_str(),Rs(Rn,sf).c_str(),hx(i).c_str()); return b; }
    if(((w>>23)&0x3f)==0x25){ int opc=(w>>29)&3,hw=(w>>21)&3; uint64_t i=(w>>5)&0xffff; const char* m=opc==0?"movn":opc==2?"movz":"movk";
        if(hw)snprintf(b,sizeof b,"%-6s %s, #%s, lsl #%d",m,R(Rd,sf).c_str(),hx(i).c_str(),hw*16); else snprintf(b,sizeof b,"%-6s %s, #%s",m,R(Rd,sf).c_str(),hx(i).c_str()); return b; }
    if(((w>>24)&0x1f)==0x10){ uint64_t lo=(w>>29)&3,hi=(w>>5)&0x7ffff; int64_t v=sext((hi<<2)|lo,21); uint64_t t=(w>>31)&1?(addr&~0xfffull)+(v<<12):addr+v; snprintf(b,sizeof b,"%-6s %s, %s",(w>>31)&1?"adrp":"adr",R(Rd,1).c_str(),sym(t).c_str()); return b; }
    if(((w>>23)&0x3f)==0x24){ int opc=(w>>29)&3,N=(w>>22)&1,immr=(w>>16)&0x3f,is=(w>>10)&0x3f; uint64_t v; bool ok=bitmask(N,is,immr,sf?64:32,v);
        if(opc==1&&Rn==31&&ok){ snprintf(b,sizeof b,"mov    %s, #%s",R(Rd,sf).c_str(),hx((int64_t)v).c_str()); return b; }
        snprintf(b,sizeof b,"%-6s %s, %s, #%s",opc==0?"and":opc==1?"orr":opc==2?"eor":"ands",R(Rd,sf).c_str(),R(Rn,sf).c_str(),ok?hx((int64_t)v).c_str():"?"); return b; }
    if(((w>>23)&0x3f)==0x26){ snprintf(b,sizeof b,"%-6s %s, %s, #%d, #%d",((w>>29)&3)==0?"sbfm":((w>>29)&3)==1?"bfm":"ubfm",R(Rd,sf).c_str(),R(Rn,sf).c_str(),(w>>16)&0x3f,(w>>10)&0x3f); return b; }
    if(((w>>24)&0x1f)==0x0a){ int opc=(w>>29)&3,sh=(w>>22)&3,imm6=(w>>10)&0x3f,N=(w>>21)&1; if(opc==1&&Rn==31&&!N&&!imm6){ snprintf(b,sizeof b,"mov    %s, %s",R(Rd,sf).c_str(),R(Rm,sf).c_str()); return b; }
        static const char* MN[2][4]={{"and","orr","eor","ands"},{"bic","orn","eon","bics"}}; static const char* SH[]={"lsl","lsr","asr","ror"}; char sfx[24]=""; if(imm6)snprintf(sfx,sizeof sfx,", %s #%d",SH[sh],imm6);
        snprintf(b,sizeof b,"%-6s %s, %s, %s%s",MN[N][opc],R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str(),sfx); return b; }
    if(((w>>24)&0x1f)==0x0b){ bool sub=(w>>30)&1,S=(w>>29)&1; if(S&&Rd==31){ snprintf(b,sizeof b,"%-6s %s, %s",sub?"cmp":"cmn",R(Rn,sf).c_str(),R(Rm,sf).c_str()); return b; } snprintf(b,sizeof b,"%-6s %s, %s, %s",sub?(S?"subs":"sub"):(S?"adds":"add"),R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str()); return b; }
    if(((w>>24)&0x1f)==0x1b){ int Ra=(w>>10)&0x1f; if(Ra==31){ snprintf(b,sizeof b,"mul    %s, %s, %s",R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str()); return b; } snprintf(b,sizeof b,"madd   %s, %s, %s, %s",R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str(),R(Ra,sf).c_str()); return b; }
    if(((w>>21)&0x3ff)==0xd6){ int o2=(w>>10)&0x3f; const char* m=o2==2?"udiv":o2==3?"sdiv":o2==8?"lslv":o2==9?"lsrv":o2==10?"asrv":o2==11?"rorv":0; if(m){ snprintf(b,sizeof b,"%-6s %s, %s, %s",m,R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str()); return b; } }
    if((w&0x1FE00000u)==0x1A800000u){ int op=(w>>30)&1,o2=(w>>10)&1,cc=(w>>12)&0xf; snprintf(b,sizeof b,"%-6s %s, %s, %s, %s",op?(o2?"csneg":"csinv"):(o2?"csinc":"csel"),R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str(),CC[cc]); return b; }
    if((w&0x3A000000u)==0x28000000u){ bool load=(w>>22)&1; int Rt2=(w>>10)&0x1f,sz=(w>>31)&1?8:4; int64_t off=sext((w>>15)&0x7f,7)*sz; snprintf(b,sizeof b,"%-6s %s, %s, [%s, #%s]",load?"ldp":"stp",R(Rd,(w>>31)&1).c_str(),R(Rt2,(w>>31)&1).c_str(),Rs(Rn,1).c_str(),hx(off).c_str()); return b; }
    if((w&0x3F000000u)==0x3D000000u || (w&0x3B200C00u)==0x3C000000u) return neonDisasm(w);   // SIMD/FP load-store -> real NEON mnemonic (must precede the GP load decoders, whose mask ignores the V bit)
    if((w&0x3B200C00u)==0x38200800u){ int size=(w>>30)&3,opc=(w>>22)&3; snprintf(b,sizeof b,"%-6s %s, [%s, %s]",opc!=0?"ldr":"str",R(Rd,size==3||opc==2).c_str(),Rs(Rn,1).c_str(),R(Rm,1).c_str()); return b; }
    if((w&0x3B000000u)==0x39000000u){ int size=(w>>30)&3,opc=(w>>22)&3; uint64_t i=((w>>10)&0xfff)<<size;   // opc 0=store,1=load,2=load-signed-to-64 (LDRSW),3=load-signed-to-32
        static const char* M[4][4]={{"strb","strh","str","str"},{"ldrb","ldrh","ldr","ldr"},{"ldrsb","ldrsh","ldrsw","prfm"},{"ldrsb","ldrsh","ldrsw","?"}};
        snprintf(b,sizeof b,"%-6s %s, [%s, #%s]",M[opc][size],R(Rd,size==3||opc==2).c_str(),Rs(Rn,1).c_str(),hx(i).c_str()); return b; }
    if((w&0x3B000000u)==0x38000000u){ int size=(w>>30)&3,opc=(w>>22)&3; int64_t off=sext((w>>12)&0x1ff,9); snprintf(b,sizeof b,"%-6s %s, [%s, #%s]",opc!=0?"ldur":"stur",R(Rd,size==3||opc==2).c_str(),Rs(Rn,1).c_str(),hx(off).c_str()); return b; }
    if((w&0x3B000000u)==0x18000000u){ snprintf(b,sizeof b,"ldr    %s, %s",R(Rd,sf).c_str(),sym(addr+(sext((w>>5)&0x7ffff,19)<<2)).c_str()); return b; }
    if(((w>>25)&7)==7) return neonDisasm(w);
    if((w&0x3FE00C10u)==0x3A400800u || (w&0x3FE00C10u)==0x3A400000u){   // CCMP/CCMN (conditional compare)
        static const char* CC[]={"eq","ne","cs","cc","mi","pl","vs","vc","hi","ls","ge","lt","gt","le","al","nv"};
        bool isImm=(w>>11)&1; const char* mn=((w>>30)&1)?"ccmp":"ccmn"; int cnd=(w>>12)&0xf, nzcv=w&0xf;
        if(isImm) snprintf(b,sizeof b,"%-6s %s, #%d, #%d, %s",mn,R(Rn,sf).c_str(),(int)((w>>16)&0x1f),nzcv,CC[cnd]);
        else      snprintf(b,sizeof b,"%-6s %s, %s, #%d, %s",mn,R(Rn,sf).c_str(),R(Rm,sf).c_str(),nzcv,CC[cnd]);
        return b; }
    if((w&0xFFE0001Fu)==0xD4200000u){ snprintf(b,sizeof b,"brk    #%d",(int)((w>>5)&0xffff)); return b; }   // BRK trap
    if((w&0x3B200C00u)==0x38200000u){   // LSE atomic memory ops
        static const char* LD[]={"ldadd","ldclr","ldeor","ldset","ldsmax","ldsmin","ldumax","ldumin"};
        bool o3=(w>>15)&1; int opc=(w>>12)&7, A=(w>>23)&1, Rr=(w>>22)&1; bool x=((w>>30)&3)==3;
        string mn=string(o3?"swp":LD[opc])+(A&&Rr?"al":A?"a":Rr?"l":"");
        snprintf(b,sizeof b,"%-6s %s, %s, [%s]",mn.c_str(),R(Rm,x).c_str(),R(Rd,x).c_str(),Rs(Rn,1).c_str()); return b; }
    if((w&0x7FA00000u)==0x13800000u){   // EXTR (rotate-extract); Rn==Rm is the ROR alias
        int imms=(w>>10)&0x3f; if(Rn==Rm) snprintf(b,sizeof b,"ror    %s, %s, #%d",R(Rd,sf).c_str(),R(Rn,sf).c_str(),imms);
        else snprintf(b,sizeof b,"extr   %s, %s, %s, #%d",R(Rd,sf).c_str(),R(Rn,sf).c_str(),R(Rm,sf).c_str(),imms); return b; }
    if((w&0x5FE00000u)==0x5AC00000u){   // data-processing (1 source): RBIT/REV/CLZ/CLS
        static const char* DP1[]={"rbit","rev16","rev32","rev","clz","cls"}; int oc=(w>>10)&0x3f;
        snprintf(b,sizeof b,"%-6s %s, %s",oc<6?DP1[oc]:"dp1",R(Rd,sf).c_str(),R(Rn,sf).c_str()); return b; }
    snprintf(b,sizeof b,".word  0x%08x",w); return b;
}

// JSON-escape for the --records sidecar
static string jstr(const string& s){ string o="\""; for(char c:s){ unsigned char u=(unsigned char)c;
    if(c=='"'||c=='\\'){ o+='\\'; o+=c; } else if(c=='\n') o+="\\n"; else if(c=='\t') o+="\\t"; else if(c=='\r') o+="\\r";
    else if(u<0x20){ char b[8]; snprintf(b,sizeof b,"\\u%04x",u); o+=b; } else o+=c; } o+="\""; return o; }

int main(int argc,char** argv){
    const char* path=nullptr; uint64_t from=0,to=0; bool nosym=false, dis=false, keepAll=false; const char* recPath=nullptr;
    for(int i=1;i<argc;i++){ string a=argv[i];
        if(a=="--from"&&i+1<argc)from=strtoull(argv[++i],0,0);
        else if(a=="--to"&&i+1<argc)to=strtoull(argv[++i],0,0);
        else if(a=="--disasm")dis=true;
        else if(a=="--records"&&i+1<argc) recPath=argv[++i];   // per-function JSONL corpus records
        else if(a=="--all")keepAll=true;                       // decompile libc++/runtime bodies too (don't skip) — keeps names
        else if(a=="--nosym")nosym=true; else path=argv[i]; }
    if(!path){ fprintf(stderr,"usage: ember-arm64 <arm64 mach-o> [--from 0x.. --to 0x..] [--nosym]\n"); return 2; }
    vector<uint8_t> f; if(!readFile(path,f)){ perror("open"); return 1; }
    machoSelectSlice(f, 0x0100000c);   // universal binary -> pick the arm64/arm64e slice (most macOS system binaries are fat)
    if(f.size()>=8){ uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24); uint32_t cpu=f[4]|(f[5]<<8)|(f[6]<<16)|((uint32_t)f[7]<<24);
        if(magic==0xFEEDFACFu && cpu!=0x0100000Cu){ fprintf(stderr,"ember-arm64: not an arm64 Mach-O (cputype 0x%x). Use ember-lift for x86-64.\n",cpu); return 3; } }
    size_t off,size; uint64_t base;
    if(!machoText(f.data(),f.size(),off,size,base)){ fprintf(stderr,"ember-arm64: need a Mach-O\n"); return 1; }
    Lifter L; if(!nosym){ machoSymbols(f.data(),f.size(),L.syms); machoStubs(f.data(),f.size(),L.stubs); } L.file=&f; machoSections(f.data(),f.size(),L.secs);
    if(!nosym){ uint64_t eoff=machoEntryOff(f.data(),f.size()); if(eoff){ uint64_t entry=base+eoff-off; if(entry>=base && !L.syms.count(entry)) L.syms.emplace(entry,"main"); } }   // stripped binary: recover `main` from the LC_MAIN entry (entryoff is a file offset -> vaddr via __text base)
    if(!dis){ int dk = nosym?0:machoDebugKind(f.data(),f.size());        // report whether we can lean on real debug symbols or must infer names by behavior
        if(dk==2)      fprintf(stderr,"  debug info: embedded DWARF — using symbol names\n");
        else if(dk==1) fprintf(stderr,"  debug info: -g debug map present (%zu symbols) — using function names; locals inferred by behavior\n", L.syms.size());
        else           fprintf(stderr,"  debug info: none — inferring all names by behavior + FLIRT\n"); }
    L.scan(f.data()+off,size,base);
    // ── FLIRT: name unknown (sub_) functions by matching their masked-body hash against the harvested sig DB.
    // KEY: we don't trust our own end-detection — for each function START we try every LENGTH the DB knows,
    // so a match is decoupled from where WE think the function ends (the boundary-fragility fix). A hit means
    // the function's first L masked bytes EXACTLY equal a known function of length L -> it IS that function. ──
    if(!nosym && !dis){ std::map<std::pair<uint64_t,uint32_t>,std::string> fldb;
        std::string home = getenv("HOME")?getenv("HOME"):"";
        for(std::string p : { std::string("/usr/local/share/emberdragon/sigs.db"), home+"/Library/Application Support/EmberDragon/sigs.db" }) flLoad(p.c_str(), fldb);
        if(!fldb.empty()){
            std::vector<uint32_t> lens; { std::set<uint32_t> s; for(auto& kv:fldb) s.insert(kv.first.second); lens.assign(s.rbegin(),s.rend()); }   // distinct lengths, DESCENDING (prefer the longest = most confident match)
            vector<uint64_t> fs(L.funcs.begin(),L.funcs.end()); std::sort(fs.begin(),fs.end()); int hit=0;
            for(uint64_t a : fs){ if(L.syms.count(a))continue;                                     // already named -> keep it
                size_t foff=off+(size_t)(a-base); uint64_t avail=base+size-a;
                for(uint32_t Ln : lens){ if(Ln<8 || Ln>avail) continue; if(foff+Ln>f.size())continue;
                    uint64_t h=flHash(f.data()+foff, a, Ln); auto it=fldb.find({h,Ln});
                    if(it!=fldb.end() && !it->second.empty()){ L.syms[a]=it->second; hit++; break; } } }
            if(hit) fprintf(stderr,"  FLIRT: identified %d function(s) by signature\n",hit); } }
    if(dis){                                                  // linear disassembly listing with function labels
        for(auto& I:L.ins){ if(L.funcs.count(I.addr)){ auto it=L.syms.find(I.addr); printf("\n%s:\n", it!=L.syms.end()?it->second.c_str():("sub_"+std::to_string(I.addr)).c_str()); }
            printf("%016llx:  %s\n",(unsigned long long)I.addr, a64disasm(I.w,I.addr,L.syms).c_str()); }
        return 0; }
    vector<uint64_t> starts(L.funcs.begin(),L.funcs.end());
    int kept=0,skipped=0;
    FILE* recFP = recPath? fopen(recPath,"w"):nullptr;
    for(size_t s=0;s<starts.size();s++){ uint64_t a=starts[s], b=s+1<starts.size()?starts[s+1]:base+size;
        if(from&&(a<from||a>=(to?to:base+size)))continue; if(!L.at.count(a))continue;
        if(!nosym && !keepAll && L.isLibrary(a)){ skipped++; continue; }   // skip libc++/runtime bodies (--all keeps them)
        fprintf(stderr,"  decompiling %s\n", L.name(a).c_str());   // real per-function progress (GUI tails this)
        kept++;
        L.liftFn(L.at[a], L.at.count(b)?L.at[b]:L.ins.size());
        if(recFP && !L.fns.empty()){ auto& F=L.fns.back();
            auto bytes=L.readBytes(a,(size_t)(b-a)); string hex; for(unsigned char c:bytes){ char t[3]; snprintf(t,sizeof t,"%02x",c); hex+=t; }
            size_t bi=L.at[a], ei=L.at.count(b)?L.at[b]:L.ins.size(); string ds="[";
            for(size_t k=bi;k<ei&&k<L.ins.size();k++){ char ah[24]; snprintf(ah,sizeof ah,"0x%llx: ",(unsigned long long)L.ins[k].addr);
                ds += (k>bi?",":"") + jstr(string(ah)+a64disasm(L.ins[k].w,L.ins[k].addr,L.syms)); } ds+="]";
            string body; for(auto& l:F.body){ if(!body.empty())body+="\n"; body+=l; }
            string lab; if(!nosym){ size_t up=F.name.find("__"); lab = up==string::npos? F.name : F.name.substr(0,up); }   // name() already demangled
            fprintf(recFP,"{\"addr\":\"0x%llx\",\"name\":%s,\"label\":%s,\"size\":%llu,\"bytes_hex\":\"%s\",\"disasm\":%s,\"pseudocode\":%s,\"arch\":\"arm64\"}\n",
                (unsigned long long)a, jstr(F.name).c_str(), jstr(lab).c_str(), (unsigned long long)(b-a), hex.c_str(), ds.c_str(), jstr(body).c_str()); } }
    if(recFP) fclose(recFP);
    fprintf(stderr,"ember-arm64: %d user function(s) decompiled, %d library function(s) skipped\n",kept,skipped);
    L.emitAll();
    return 0;
}
