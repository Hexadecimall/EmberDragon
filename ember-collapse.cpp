// ember-collapse — EmberDragon's "bloat -> clean source" pass. Reads decompiled
// C/C++ on stdin, rewrites it toward something that reads like the original
// source, and writes it to stdout. 100% offline + deterministic (no network, no
// model) — this is what the GUI's "Clean Up" button runs. An optional AI naming
// pass (ember-ai) can run AFTER this when the user opts in.
//
// Transforms (all behavior-preserving rewrites of the decompiler's own idioms):
//   • operatorOP(a, b)            -> (a OP b)            (==, !=, <<, >>, <, >, +, ...)
//   • operator[](s, i)            -> (s[i])
//   • *g_<hex> as a << operand    -> std::cout
//   • basic_string(&dst, src)     -> std::string dst = src;
//   • ((expr) & (1<<0)) == 0      -> the bool idiom, ONLY where the mask is present
//   • const char* s_x = "lit";    -> inlined at its single use site, decl dropped
//   • vN = <atom>;                -> copy-propagated (only when provably safe) + removed
//   • vN = <expr>; (never read)   -> dead store removed (only when side-effect-free)
//   • ( expr );  /  ((x))         -> redundant outer / nested parens stripped
//
// Every textual transform is STRING-LITERAL AWARE (it never rewrites text inside
// "..." or '...'), so program data is preserved.
//
// build:  clang++ -std=c++17 -O2 ember-collapse.cpp -o ember-collapse
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <cctype>
#include <algorithm>
#include <functional>
using std::string; using std::vector; using std::map; using std::set;

static string trim(const string& s){ size_t a=s.find_first_not_of(" \t"); if(a==string::npos)return ""; size_t b=s.find_last_not_of(" \t"); return s.substr(a,b-a+1); }
static string indentOf(const string& s){ size_t a=s.find_first_not_of(" \t"); return a==string::npos? "" : s.substr(0,a); }
static bool isIdent(char c){ return isalnum((unsigned char)c)||c=='_'; }

// per-char mask: 1 = source code, 0 = inside a "..." / '...' literal (quotes + escapes included).
// Every scan below consults this so we never rewrite text that lives inside a string.
static vector<char> codeMask(const string& s){
    vector<char> m(s.size(), 1); char q = 0;
    for(size_t i=0;i<s.size();i++){ char c=s[i];
        if(q){ m[i]=0; if(c=='\\'&&i+1<s.size()){ m[i+1]=0; i++; continue; } if(c==q) q=0; }
        else if(c=='/'&&i+1<s.size()&&s[i+1]=='/'){ for(;i<s.size();i++) m[i]=0; break; }                                   // `//` line comment -> mask to EOL
        else if(c=='/'&&i+1<s.size()&&s[i+1]=='*'){ m[i]=m[i+1]=0; i+=2; for(;i<s.size();i++){ m[i]=0; if(s[i]=='*'&&i+1<s.size()&&s[i+1]=='/'){ m[i+1]=0; i++; break; } } }   // `/* … */` block comment
        else if(c=='"'||c=='\''){ q=c; m[i]=0; } }
    return m;
}

static const char* INFIX[]={"<<=",">>=","==","!=","<=",">=","&&","||","<<",">>","->","+","-","*","/","%","&","|","^","<",">","="};

// every `goto <ident>` target on a line (incl. inline `if (c) goto loc_X;`), literal-aware
static vector<char> codeMask(const string& s);
static int braceDelta(const string& l);
static void collectGotoTargets(const string& l, set<string>& tg){
    auto m=codeMask(l); size_t p=0;
    while((p=l.find("goto",p))!=string::npos){ bool lb=(p==0||!isIdent(l[p-1])); size_t a=p+4;
        if(p<m.size()&&m[p]&&lb&&a<l.size()&&(l[a]==' '||l[a]=='\t')){ while(a<l.size()&&(l[a]==' '||l[a]=='\t'))a++; size_t e=a; while(e<l.size()&&isIdent(l[e]))e++; if(e>a) tg.insert(l.substr(a,e-a)); }
        p+=4; }
}

// split top-level comma-separated args (commas/parens inside literals ignored)
static vector<string> splitArgs(const string& s){
    vector<string> out; auto m=codeMask(s); int d=0; size_t st=0;
    for(size_t i=0;i<s.size();i++){ if(!m[i])continue; char c=s[i];
        if(c=='('||c=='[')d++; else if(c==')'||c==']')d--;
        else if(c==','&&d==0){ out.push_back(trim(s.substr(st,i-st))); st=i+1; } }
    out.push_back(trim(s.substr(st))); return out;
}
static string deref(string a){ a=trim(a); if(!a.empty()&&a[0]=='&') a=trim(a.substr(1)); return a; }

// fold `<lit> +|-|* <lit>` -> the constant (e.g. the lifter's `cond ? 0 : 0 + 1` -> `... : 1`). Only when
// BOTH sides are integer literals AND neither neighbour is a higher/equal-precedence operator (so associativity
// can't change meaning) — and never on long literals (overflow). Literal/comment-aware.
static void foldConstArith(string& line){
    for(int g=0;g<100;g++){ auto m=codeMask(line); bool ch=false;
        for(size_t i=0;i<line.size();i++){
            if(i>=m.size()||!m[i]||!isdigit((unsigned char)line[i])) continue;
            if(i>0 && (isIdent(line[i-1])||line[i-1]=='.')) continue;                       // mid-number / identifier
            long lb=(long)i-1; while(lb>=0&&line[lb]==' ')lb--; char lbc=lb>=0?line[lb]:'(';
            if(strchr("*/%<>+-&|^~.",lbc)) continue;                                         // higher/equal-prec op on the left -> unsafe
            size_t a=i; while(a<line.size()&&(isxdigit((unsigned char)line[a])||line[a]=='x'||line[a]=='X'))a++;
            string L=line.substr(i,a-i); if(L.size()>10) continue;
            size_t o=a; while(o<line.size()&&line[o]==' ')o++;
            if(o>=line.size()||!(line[o]=='+'||line[o]=='-'||line[o]=='*')) continue; char op=line[o];
            if(o+1<line.size()&&(line[o+1]==op||line[o+1]=='=')) continue;                   // ++/--/+=/*=/<<
            size_t b=o+1; while(b<line.size()&&line[b]==' ')b++;
            if(b>=line.size()||!isdigit((unsigned char)line[b])) continue;
            size_t c=b; while(c<line.size()&&(isxdigit((unsigned char)line[c])||line[c]=='x'||line[c]=='X'))c++;
            string R=line.substr(b,c-b); if(R.size()>10) continue;
            size_t rb=c; while(rb<line.size()&&line[rb]==' ')rb++; char rbc=rb<line.size()?line[rb]:')';
            if(strchr("*/%<>+-&|^.",rbc)) continue;                                          // higher/equal-prec op on the right -> unsafe
            long long lv=strtoll(L.c_str(),0,0), rv=strtoll(R.c_str(),0,0);
            long long res = op=='+'?lv+rv : op=='-'?lv-rv : lv*rv;
            line = line.substr(0,i)+std::to_string(res)+line.substr(c); ch=true; break;
        }
        if(!ch)break;
    }
}

// `*(A + B)` -> `A[B]`, `*(A - B)` -> `A[-(B)]`. C DEFINES E1[E2] as *((E1)+(E2)), so this is a
// pure syntactic sugar (no type/scaling assumption) — exactly how Binary Ninja renders pointer loads.
// Iterates to a fixpoint so nested derefs (`*(*(p) + i)`) all collapse. Literal/comment-aware via codeMask.
static void arraySubscript(string& line){
    for(int guard=0; guard<200; guard++){
        auto m=codeMask(line); bool changed=false;
        for(size_t i=0;i+1<line.size();i++){
            if(i>=m.size()||!m[i]||line[i]!='*'||line[i+1]!='(') continue;
            long p=(long)i-1; while(p>=0&&line[p]==' ')p--;                                    // a `*` after an operand is MULTIPLY, not deref
            if(p>=0 && (isalnum((unsigned char)line[p])||line[p]=='_'||line[p]==')'||line[p]==']')) continue;
            size_t open=i+1; int d=0, splitPos=-1; char splitOp=0; bool prevOperand=false;     // find the FIRST top-level binary +/- inside the (...)
            size_t j=open;
            for(; j<line.size(); j++){ if(j<m.size()&&!m[j]){prevOperand=true;continue;} char c=line[j];
                if(c==' '||c=='\t')continue;                                                   // whitespace must NOT reset prevOperand (else `buf + i` reads `+` as unary)
                if(c=='('){d++;prevOperand=false;continue;} if(c=='['){d++;prevOperand=false;continue;}
                if(c==')'){ if(--d==0)break; prevOperand=true; continue; } if(c==']'){d--;prevOperand=true;continue;}
                if(d==1 && (c=='+'||c=='-') && prevOperand && splitPos<0){ splitPos=(int)j; splitOp=c; }
                if(isalnum((unsigned char)c)||c=='_'){ prevOperand=true; continue; }
                prevOperand=false; }
            if(j>=line.size()||splitPos<0) continue;
            string base=trim(line.substr(open+1,splitPos-(open+1)));
            string idx =trim(line.substr(splitPos+1,j-(splitPos+1)));
            if(base.empty()||idx.empty()) continue;
            bool simple=true; for(char c:base) if(!isIdent(c)) simple=false;                    // `&key`, `a + b` etc. must be parenthesized so `&key[i]` (== &(key[i])) can't happen
            string A = simple ? base : "("+base+")";
            string B = (splitOp=='-') ? "-("+idx+")" : idx;
            line = line.substr(0,i) + A + "[" + B + "]" + line.substr(j+1);
            changed=true; break;
        }
        if(!changed) break;
    }
}

// balanced (...) starting at '(' index p — parens inside literals don't count
static bool balanced(const string& s,size_t p,string& inside,size_t& end){
    if(p>=s.size()||s[p]!='(')return false; auto m=codeMask(s); int d=0;
    for(size_t i=p;i<s.size();i++){ if(!m[i])continue; if(s[i]=='(')d++; else if(s[i]==')'){ if(--d==0){ inside=s.substr(p+1,i-p-1); end=i; return true; } } }
    return false;
}

// ── Pass E: fold C++ ostream insertion chains back to `cout << a << b << endl;` ──
// The lifter splits every `<<` into its own statement (returning a stream temp tN)
// and inlines std::endl as a getloc / use_facet / put / flush block. This rejoins
// them. Recognition keys on VERY specific tokens (cout/cerr, __put_character_sequence,
// and a put()-arg containing use_facet) so it can never touch ordinary `a << b` shifts.
static bool isStreamName(const string& s){ return s=="cout"||s=="cerr"||s=="clog"||s=="wcout"||s=="wcerr"||s=="wclog"; }
static bool hasTok(const string& l,const string& tok){ size_t p=0; while((p=l.find(tok,p))!=string::npos){ bool lb=(p==0||!isIdent(l[p-1])),rb=(p+tok.size()>=l.size()||!isIdent(l[p+tok.size()])); if(lb&&rb)return true; p+=tok.size(); } return false; }
static string ioStripParens(string r){ r=trim(r); while(r.size()>=2&&r.front()=='('){ string in; size_t e; if(balanced(r,0,in,e)&&e==r.size()-1) r=trim(in); else break; } return r; }
static string stripTempAssign(const string& rin,string& temp){ temp.clear(); size_t p=rin.find(" = "); if(p==string::npos) return rin;
    string lhs=trim(rin.substr(0,p)); if(lhs.size()<2||(lhs[0]!='t'&&lhs[0]!='v')) return rin; for(size_t k=1;k<lhs.size();k++) if(!isdigit((unsigned char)lhs[k])) return rin; temp=lhs; return trim(rin.substr(p+3)); }
static void foldIostream(vector<string>& lines){
    // (1) a g_<hex> used ONLY as the sole operator<< arg is a manipulator (std::endl/flush) -> render "endl"
    std::map<string,int> gAll,gManip;
    for(auto& l:lines){ auto m=codeMask(l); for(size_t i=0;i<l.size();){ if(m[i]&&l[i]=='g'&&i+1<l.size()&&l[i+1]=='_'){ size_t j=i+2; while(j<l.size()&&isxdigit((unsigned char)l[j]))j++; if(j>i+2)gAll[l.substr(i,j-i)]++; i=j; } else i++; } }
    for(auto& l:lines){ string t=trim(l); size_t q=0; while((q=t.find("operator<<(",q))!=string::npos){ string in; size_t e; if(balanced(t,q+10,in,e)){ string a=trim(in); if(a.rfind("g_",0)==0){ bool pure=a.size()>2; for(size_t k=2;pure&&k<a.size();k++) if(!isxdigit((unsigned char)a[k]))pure=false; if(pure)gManip[a]++; } } q+=10; } }
    set<string> manip; for(auto&kv:gManip) if(gAll.count(kv.first)&&kv.second==gAll[kv.first]) manip.insert(kv.first);
    auto headOf=[&](const string& tin,string& head,string& temp)->bool{ string t=tin; if(!t.empty()&&t.back()==';')t.pop_back(); t=trim(t);
        string r=ioStripParens(stripTempAssign(t,temp));
        { size_t sp=r.find(' '); string first=sp==string::npos?r:r.substr(0,sp); if(isStreamName(first)&&sp!=string::npos&&r.size()>sp+3&&r.compare(sp+1,2,"<<")==0){ head=r; return true; } }
        if(r.rfind("__put_character_sequence(",0)==0){ string in; size_t e; if(balanced(r,24,in,e)&&e==r.size()-1){ auto a=splitArgs(in); if(a.size()==3&&isStreamName(trim(a[0]))){ head=trim(a[0])+" << "+trim(a[1]); return true; } } }
        return false; };
    auto contOf=[&](const string& tin,string& term,string& temp)->bool{ string t=tin; if(!t.empty()&&t.back()==';')t.pop_back(); t=trim(t);
        string r=ioStripParens(stripTempAssign(t,temp));
        if(r.rfind("operator<<(",0)==0){ string in; size_t e; if(balanced(r,10,in,e)&&e==r.size()-1){ auto a=splitArgs(in); if(a.size()==1&&!trim(a[0]).empty()){ term=trim(a[0]); return true; } } }   // operator<<() with a LOST operand must not fold to an empty `<< `
        if(r.rfind("__put_character_sequence(",0)==0){ string in; size_t e; if(balanced(r,24,in,e)&&e==r.size()-1){ auto a=splitArgs(in); if(a.size()==2&&!trim(a[0]).empty()&&!isStreamName(trim(a[0]))){ term=trim(a[0]); return true; } } }
        return false; };
    vector<string> out;
    for(size_t i=0;i<lines.size();){
        string head,temp; if(!headOf(trim(lines[i]),head,temp)){ out.push_back(lines[i]); i++; continue; }
        string ind=indentOf(lines[i]); string chain=head,cur=temp; size_t j=i+1;
        set<string> absorbed; if(!temp.empty())absorbed.insert(temp);   // every stream-temp whose binding we drop
        for(;;){ if(j>=lines.size())break; string g=trim(lines[j]);
            if(!cur.empty()&&g.rfind("getloc(",0)==0&&hasTok(g,cur)){          // inlined std::endl machinery on `cur`
                size_t putk=string::npos,flushk=string::npos; bool clean=true;
                for(size_t k=j+1;k<lines.size()&&k<=j+5;k++){ string lk=trim(lines[k]);
                    if(lk.rfind("put(",0)==0&&hasTok(lk,cur)&&lk.find("use_facet")!=string::npos){ putk=k; continue; }
                    if(lk.rfind("flush(",0)==0&&hasTok(lk,cur)){ flushk=k; break; }
                    if(lk.rfind("~locale",0)==0||lk.empty()) continue;
                    clean=false; break; }                                      // an UNRELATED statement inside the window -> NOT an endl block (no over-consume)
                if(clean&&putk!=string::npos&&flushk!=string::npos&&flushk>putk){ chain+=" << endl"; j=flushk+1; cur.clear(); continue; } }
            string term,ttemp; if(contOf(g,term,ttemp)){ if(manip.count(term))term="endl"; chain+=" << "+term; cur=ttemp; if(!ttemp.empty())absorbed.insert(ttemp); j++; continue; }
            break; }
        // safety: if any stream-temp whose binding we absorbed is still referenced by a NOT-consumed line of this
        // function, dropping its definition would dangle it (e.g. an endl block we couldn't match) -> abort, keep originals.
        bool leak=false;
        for(size_t k=j;k<lines.size()&&!leak;k++){ if(indentOf(lines[k]).empty()&&trim(lines[k]).rfind("}",0)==0) break;   // function boundary
            for(const auto& a:absorbed) if(hasTok(lines[k],a)){ leak=true; break; } }
        if(leak){ out.push_back(lines[i]); i++; continue; }
        out.push_back(ind+chain+";"); i=j;
    }
    lines.swap(out);
}

// operatorOP(a,b) -> (a OP b) ; operator[](s,i) -> (s[i]) ; basic_string(&d,s) -> std::string d = s
static bool foldCalls(string& line){
    auto m=codeMask(line);
    for(size_t i=0;i+8<line.size();i++){
        if(!m[i] || line.compare(i,8,"operator")!=0 || (i!=0&&isIdent(line[i-1]))) continue;
        size_t j=i+8; string op; while(j<line.size()&&line[j]!='(' ) op+=line[j++];
        if(j>=line.size()) continue;
        string inside; size_t end; if(!balanced(line,j,inside,end)) continue;
        auto args=splitArgs(inside);
        // DON'T fold a function DEFINITION/PROTOTYPE header — args there are param decls (`struct X* a`),
        // not expressions; folding `long operator=(struct X* a, struct X* b)` mangled it to `long (a = b)`.
        { bool isDecl=false; for(auto& a:args){ string t=trim(a);
            for(const char* ty:{"struct ","long ","int ","char","short","unsigned","double","float","void ","bool ","const "})
                if(t.rfind(ty,0)==0){ isDecl=true; break; } if(isDecl)break; }
          if(isDecl) continue; }
        if(op=="[]" && args.size()==2){ line=line.substr(0,i)+"("+deref(args[0])+"["+deref(args[1])+"])"+line.substr(end+1); return true; }
        bool known=false; for(auto o:INFIX) if(op==o){known=true;break;}
        if(!known||args.size()!=2) continue;
        line=line.substr(0,i)+"("+deref(args[0])+" "+op+" "+deref(args[1])+")"+line.substr(end+1); return true;
    }
    size_t bp=line.find("basic_string(");
    if(bp!=string::npos && bp<m.size() && m[bp]){ size_t paren=bp+12; string inside; size_t end;
        if(balanced(line,paren,inside,end)){ auto a=splitArgs(inside);
            if(a.size()>=2){ line=line.substr(0,bp)+"std::string "+deref(a[0])+" = "+a[1]+line.substr(end+1); return true; } } }
    return false;
}

// ((expr) & (1<<0)) == 0  bool idiom -> clean (expr). Only collapse "== 0)" on a line
// where the bit-mask was actually present — never touch ordinary `x == 0` comparisons.
static void cleanBoolBits(string& line){
    bool stripped=false;
    for(const char* pat:{" & (1<<0)"," & (1 << 0)","&(1<<0)"}){ size_t n=strlen(pat), p=0;
        while((p=line.find(pat,p))!=string::npos){ auto m=codeMask(line); if(m[p]){ line.erase(p,n); stripped=true; } else p+=n; } }
    if(!stripped) return;
    for(int guard=0; guard<20; guard++){ auto m=codeMask(line);
        size_t pos=string::npos; { size_t p=0; while((p=line.find("== 0)",p))!=string::npos){ if(p<m.size()&&m[p]){pos=p;break;} p+=5; } }
        if(pos==string::npos) break;
        size_t close=pos+4; int d=0; size_t open=string::npos;
        for(size_t i=close;;i--){ if(m[i]){ if(line[i]==')')d++; else if(line[i]=='('){ if(--d==0){open=i;break;} } } if(i==0)break; }
        if(open==string::npos||pos<=open+1) break;
        string expr=trim(line.substr(open+1,pos-(open+1)));
        bool neg = open>0 && line[open-1]=='!';
        size_t cut = neg? open-1 : open;
        // keep the replacement self-parenthesized: the (..==0) span we consume may BE the `if (...)`
        // wrapping parens, so a bare `!(x)` would leave `if !(x)` — re-wrap so it stays `if (!(x))`.
        line = line.substr(0,cut)+(neg? "("+expr+")" : "(!("+expr+"))")+line.substr(close+1);
    }
}

// the ostream global feeding a << is std::cout
static void nameStreams(string& line){
    auto m=codeMask(line); size_t p=0;
    while((p=line.find("(*g_",p))!=string::npos){ if(p>=m.size()||!m[p]){ p+=4; continue; }
        size_t q=p+4; while(q<line.size()&&isxdigit((unsigned char)line[q]))q++;
        if(q<line.size()&&line.compare(q,4," << ")==0){ line=line.substr(0,p)+"(std::cout"+line.substr(q); m=codeMask(line); p=0; } else p=q; }
}
// macOS libc stdio: `__stdinp`/`__stdoutp`/`__stderrp` ARE stdin/stdout/stderr (FILE*); the lifter tacks on a
// bogus `->fN` field access. Rewrite `__stdinp->f0` (and the bare symbol) -> `stdin`, etc.
static void mapStdio(string& line){
    static const char* SYM[3]={"__stdinp","__stdoutp","__stderrp"}; static const char* NM[3]={"stdin","stdout","stderr"};
    for(int k=0;k<3;k++){ string s=SYM[k], n=NM[k]; size_t p;
        while((p=line.find(s+"->f"))!=string::npos){ size_t e=p+s.size()+3; while(e<line.size()&&isdigit((unsigned char)line[e]))e++; line=line.substr(0,p)+n+line.substr(e); }   // sym->fN
        size_t q=0; while((q=line.find(s,q))!=string::npos){ bool lb=(q==0||!isIdent(line[q-1])), rb=(q+s.size()>=line.size()||!isIdent(line[q+s.size()])); if(lb&&rb){ line.replace(q,s.size(),n); q+=n.size(); } else q+=s.size(); } }   // bare sym
}

// FILE* handle alias: the GOT slot holding stdin is loaded as `vN = stdin;` and then dereffed as `(*vN)`
// (vN is really `&stdin`, a FILE**). When vN is assigned a stream once and otherwise appears ONLY inside
// `(*vN)`, fold every `(*vN)` -> the stream and drop the dead assignment: `fgets(buf,64,(*v138))` -> `(…,stdin)`.
static void foldStreamHandle(vector<string>& lines){
    static const char* STREAMS[3]={"stdin","stdout","stderr"};
    map<string,string> cand;                                   // vN -> stream (single assignment of a stream)
    map<string,int> assigns;                                   // count of `vN = …` assignments
    auto tokAt=[&](const string& L,size_t k,size_t e){ return (k==0||!isIdent(L[k-1]))&&(e>=L.size()||!isIdent(L[e])); };
    for(auto& L:lines){ string t=trim(L); size_t eq=t.find(" = "); if(eq==string::npos)continue; string lhs=t.substr(0,eq);
        bool name=!lhs.empty()&&(isalpha((unsigned char)lhs[0])||lhs[0]=='_'); for(char c:lhs) if(!isIdent(c)) name=false;
        if(!name)continue; assigns[lhs]++; string rhs=t.substr(eq+3); if(!rhs.empty()&&rhs.back()==';')rhs.pop_back();
        for(int k=0;k<3;k++) if(rhs==STREAMS[k]) cand[lhs]=STREAMS[k]; }
    for(auto it=cand.begin();it!=cand.end();){ const string& v=it->first; bool safe=(assigns[v]==1);
        for(size_t i=0;safe&&i<lines.size();i++){ const string& L=lines[i]; if(trim(L)==v+" = "+it->second+";")continue;
            if(trim(L).rfind("//",0)==0)continue;            // skip comment lines (e.g. the `// @types v138=long*` hint)
            size_t p=0; while((p=L.find(v,p))!=string::npos){ size_t e=p+v.size(); if(tokAt(L,p,e)){
                bool wrapped=(p>=2&&L[p-1]=='*'&&L[p-2]=='('&&e<L.size()&&L[e]==')'); if(!wrapped){safe=false;break;} } p=e; } }
        if(!safe){ it=cand.erase(it); continue; } ++it; }
    if(cand.empty())return;
    vector<string> out;
    for(auto& L:lines){ string t=trim(L); bool drop=false;
        for(auto& c:cand){ if(t==c.first+" = "+c.second+";"){drop=true;break;} }
        if(drop)continue; string s=L; for(auto& c:cand){ string from="(*"+c.first+")"; size_t p; while((p=s.find(from))!=string::npos) s.replace(p,from.size(),c.second); } out.push_back(s); }
    lines.swap(out);
}
// ── REDUNDANT-PAREN REMOVAL (precedence-aware) ── the lifter emits FULLY parenthesized expressions
// (`(((((i << 3) - i) ^ x) ^ y) ^ 56)`). Strip a `(...)` group only when C operator precedence proves it
// redundant — never when it could change meaning (correctness > prettiness). Conservative on ternary/comma/
// assignment/unary/member contexts (keeps the parens there).
static int binPrec(const string& op){
    if(op=="||")return 1; if(op=="&&")return 2; if(op=="|")return 3; if(op=="^")return 4; if(op=="&")return 5;
    if(op=="=="||op=="!=")return 6; if(op=="<"||op=="<="||op==">"||op==">=")return 7;
    if(op=="<<"||op==">>")return 8; if(op=="+"||op=="-")return 9; if(op=="*"||op=="/"||op=="%")return 10;
    return 0; }
// min precedence of a TOP-LEVEL binary operator in s; 100 if none (atom/call/unary-only); -1 if a top-level
// ?/ : / , / = makes unwrapping risky (caller keeps the parens).
static int exprMinPrec(const string& s){
    int depth=0, mn=100; bool prevOperand=false;
    for(size_t i=0;i<s.size();){
        char c=s[i];
        if(c==' '||c=='\t'){i++;continue;}
        if(c=='('){depth++;prevOperand=false;i++;continue;}
        if(c==')'){if(depth>0)depth--;prevOperand=true;i++;continue;}
        if(c=='['){ int d=1;i++; while(i<s.size()&&d){ if(s[i]=='[')d++; else if(s[i]==']')d--; i++; } prevOperand=true; continue; }
        if(c=='"'){ i++; while(i<s.size()&&s[i]!='"'){ if(s[i]=='\\')i++; i++; } if(i<s.size())i++; prevOperand=true; continue; }
        if(c=='\''){ i++; while(i<s.size()&&s[i]!='\''){ if(s[i]=='\\')i++; i++; } if(i<s.size())i++; prevOperand=true; continue; }
        if(isalnum((unsigned char)c)||c=='_'){ while(i<s.size()&&(isalnum((unsigned char)s[i])||s[i]=='_'))i++; prevOperand=true; continue; }
        string op; static const char* M[]={"<<",">>","<=",">=","==","!=","&&","||","->"};
        for(auto m:M){ size_t n=strlen(m); if(s.compare(i,n,m)==0){op=m;break;} }
        if(op.empty()) op=string(1,c);
        if(depth==0){
            if(op=="?"||op==":"||op==","||op=="=") return -1;
            if(op=="->"||op=="."){ i+=op.size(); prevOperand=false; continue; }   // member access -> next is a field operand
            if(!prevOperand){ i+=op.size(); prevOperand=false; continue; }         // unary prefix (-/*/&/!/~) -> not a binary op
            int p=binPrec(op); if(p>0 && p<mn) mn=p;
        }
        i+=op.size(); prevOperand=false;
    }
    return mn; }
static void reduceParens(string& line){
    for(int guard=0; guard<400; guard++){
        auto mask=codeMask(line); bool changed=false;
        for(size_t i=0;i<line.size();i++){
            if(i>=mask.size()||!mask[i]||line[i]!='(') continue;
            long p=(long)i-1; while(p>=0&&line[p]==' ')p--;                          // a CALL paren `f(` / `](` / `)(` is not a grouping paren
            if(p>=0 && (isalnum((unsigned char)line[p])||line[p]=='_'||line[p]==')'||line[p]==']')) continue;
            int d=0; size_t j=i; for(; j<line.size(); j++){ if(j<mask.size()&&!mask[j])continue; if(line[j]=='(')d++; else if(line[j]==')'){ if(--d==0)break; } }
            if(j>=line.size()) continue;
            { string body=trim(line.substr(i+1,j-i-1)); size_t tk=0; while(tk<body.size()&&(isalpha((unsigned char)body[tk])||body[tk]=='_'))tk++;
              static const std::set<string> TYKW={"unsigned","signed","const","int","char","short","long","void","float","double","struct","bool","size_t",
                  "uint8_t","uint16_t","uint32_t","uint64_t","int8_t","int16_t","int32_t","int64_t"};
              if(TYKW.count(body.substr(0,tk))) continue; }   // `(type)expr` is a CAST — its parens are load-bearing; stripping them yielded the invalid `(unsigned char*&v8)`
            int pin=exprMinPrec(line.substr(i+1,j-i-1)); if(pin<0) continue;
            // left context: is the group the RIGHT operand of a binary op / under a unary prefix?
            int lprec=0; bool leftUnary=false;
            { long q=(long)i-1; while(q>=0&&line[q]==' ')q--;
              if(q>=0){ char lc=line[q]; string two=q>=1?line.substr(q-1,2):"";
                if(two=="<<"||two==">>"||two=="<="||two==">="||two=="=="||two=="!="||two=="&&"||two=="||") lprec=binPrec(two);
                else if(strchr("+-*/%&|^<>",lc)){ long r=q-1; while(r>=0&&line[r]==' ')r--;
                    bool prevOp=r>=0&&(isalnum((unsigned char)line[r])||line[r]=='_'||line[r]==')'||line[r]==']');
                    if(prevOp) lprec=binPrec(string(1,lc)); else leftUnary=true; }
                else if(lc=='!'||lc=='~') leftUnary=true; } }   // else (/,/=/?/:/[/start = full operand
            // right context: is the group the LEFT operand of a binary op / a member/index base?
            int rprec=0; bool rightMember=false;
            { size_t q=j+1; while(q<line.size()&&line[q]==' ')q++;
              if(q<line.size()){ char rc=line[q]; string two=line.substr(q,std::min((size_t)2,line.size()-q));
                if(two=="<<"||two==">>"||two=="<="||two==">="||two=="=="||two=="!="||two=="&&"||two=="||") rprec=binPrec(two);
                else if(two=="->"||rc=='.'||rc=='[') rightMember=true;
                else if(strchr("+-*/%&|^<>",rc)) rprec=binPrec(string(1,rc)); } }   // else )/,/;/?/: = no right binding
            // a postfix `[`/`.`/`->` binds tighter than a leading unary prefix (`&x`, `*p`, `-n`), so
            // `(&key)[i]` must NOT lose its parens (that would mean `&(key[i])`). Detect a leading unary.
            bool innerUnaryPrefix=false; { string inr=trim(line.substr(i+1,j-i-1)); if(!inr.empty()&&strchr("&*-+!~",inr[0])) innerUnaryPrefix=true; }
            bool leftOK  = leftUnary ? (pin>=100) : (lprec==0 || pin>lprec);
            bool rightOK = rightMember ? (pin>=100 && !innerUnaryPrefix) : (rprec==0 || pin>=rprec);
            if(leftOK && rightOK){ line.erase(j,1); line.erase(i,1); changed=true; break; }
        }
        if(!changed) break;
    }
}
static string repTok(const string& s,const string& from,const string& to){ if(from.empty())return s; string r; size_t i=0;
    while(i<s.size()){ if(s.compare(i,from.size(),from)==0&&(i==0||!isIdent(s[i-1]))&&(i+from.size()>=s.size()||!isIdent(s[i+from.size()]))){ r+=to; i+=from.size(); } else r+=s[i++]; } return r; }

// A `main(int argc, char** argv)` whose source was `main(void)` has clang reuse x0/x1 as scratch, so they
// surface as `argc = "..."` — clobbering a parameter, which Binary Ninja never does. When a param is
// WRITE-FIRST (its first body reference is an assignment, i.e. the incoming value is never read), it isn't
// really the parameter: rename it to a fresh typed local and declare it. Read-first params are left alone.
static void repurposeParams(vector<string>& lines){
    for(size_t h=0;h<lines.size();h++){
        string t=trim(lines[h]);
        if(t.rfind("int main(int argc",0)!=0 || t.empty() || t.back()!='{') continue;
        size_t e=h+1; int depth=1;
        for(;e<lines.size();e++){ auto m=codeMask(lines[e]); for(size_t k=0;k<lines[e].size();k++){ if(!m[k])continue; if(lines[e][k]=='{')depth++; else if(lines[e][k]=='}')depth--; } if(depth==0)break; }
        const char* params[2]={"argc","argv"};
        for(const char* pc:params){ string nm=pc;
            int state=0; string declTy="long";   // 0 unused, 1 write-first (scratch), 2 read-first (real param)
            for(size_t r=h+1;r<e && state==0;r++){ const string& s=lines[r]; auto m=codeMask(s); size_t p=0;
                while((p=s.find(nm,p))!=string::npos){ bool lb=p<m.size()&&m[p]&&(p==0||!isIdent(s[p-1])); size_t af=p+nm.size(); bool rb=(af>=s.size()||!isIdent(s[af]));
                    if(lb&&rb){ size_t q=af; while(q<s.size()&&s[q]==' ')q++;
                        bool wr=q<s.size()&&s[q]=='='&&(q+1>=s.size()||s[q+1]!='=');
                        state=wr?1:2; if(wr){ string rhs=trim(s.substr(q+1)); if(!rhs.empty()&&(rhs[0]=='"'||rhs.rfind("&s_",0)==0))declTy="const char*"; }   // string literal or string-global ptr (`&s_denied`, inlined to "..." later)
                        break; }
                    p=af; } }
            if(state!=1) continue;
            string repl = nm=="argc" ? "msg" : "arg";   // fresh, collision-free name
            { bool clash=true; for(int s2=0; clash; s2++){ string cand = s2? repl+std::to_string(s2+1) : repl; clash=false;
                for(size_t r=h+1;r<e;r++){ if(repTok(lines[r],cand,"\x01")!=lines[r]){ clash=true; break; } } if(!clash) repl=cand; } }
            for(size_t r=h+1;r<e;r++) lines[r]=repTok(lines[r],nm,repl);
            lines.insert(lines.begin()+h+1, "    "+declTy+" "+repl+";"); e++;
        }
    }
}

// iterate top-level function bodies: calls fn(header_index, body_start, body_end_exclusive).
static bool isFnHeader(const string& line){ string t=trim(line);
    if(t.empty()||t.back()!='{'||t.find('(')==string::npos||!indentOf(line).empty()) return false;
    for(const char* kw:{"if","for","while","else","switch","struct","class","do"}) if(t.rfind(kw,0)==0 && (t.size()==strlen(kw)||!isIdent(t[strlen(kw)]))) return false;
    return true; }
template<class F> static void eachFunction(vector<string>& lines, F fn){
    for(size_t i=0;i<lines.size();i++){ if(!isFnHeader(lines[i])) continue;
        size_t s=i+1, j=s; int depth=1;
        for(;j<lines.size();j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        fn(i,s,j); i=j; }
}

// A `char buf[N]` source local is decompiled as `long buf;` accessed via `&buf` (fgets(&buf,…), strcmp(&buf,…),
// buf[i]). Binary Ninja shows the real array. Promote any scalar local that is taken-address-of into a
// string/buffer call (or indexed) to `char buf[N]` (N from the fgets size) and rewrite every `&buf` -> `buf`.
static void promoteBuffers(vector<string>& lines){
    eachFunction(lines, [&](size_t h, size_t s, size_t e){
        std::map<string,size_t> declLine;
        for(size_t r=s;r<e;r++){ string d=trim(lines[r]);
            for(const char* ty:{"long ","int "}){ size_t tl=strlen(ty);
                if(d.rfind(ty,0)==0 && d.back()==';' && d.find('=')==string::npos && d.find('(')==string::npos){
                    string nm=trim(d.substr(tl,d.size()-tl-1));
                    if(!nm.empty()&&nm.find('*')==string::npos&&nm.find('[')==string::npos&&nm.find(',')==string::npos&&(isalpha((unsigned char)nm[0])||nm[0]=='_')) declLine[nm]=r; } } }
        static const char* SFN[]={"fgets(","gets(","scanf(","sscanf(","strcspn(","strlen(","strcmp(","strncmp(","strcpy(","strncpy(","strcat(","memcpy(","strtoul(","strtol(","puts(","strchr(","strstr("};
        for(auto& kv:declLine){ const string& nm=kv.first; string amp="&"+nm; bool buffery=false, bareUse=false; int sz=64; int eshift=0;
            for(size_t r=s;r<e;r++){ if(r==kv.second) continue; const string& L=lines[r]; auto m=codeMask(L); size_t p=0;
                while((p=L.find(nm,p))!=string::npos){ bool lb=p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1])); size_t af=p+nm.size(); bool rb=(af>=L.size()||!isIdent(L[af]));
                    if(!lb||!rb){ p=af; continue; }
                    long b=(long)p-1; while(b>=0&&L[b]==' ')b--; char bc=b>=0?L[b]:0;          // classify this occurrence
                    size_t a=af; while(a<L.size()&&L[a]==' ')a++; char ac=a<L.size()?L[a]:0;
                    if(bc=='&'){ buffery=true; for(const char* f:SFN) if(L.find(f)!=string::npos){ buffery=true; break; } }   // &nm — address use (safe)
                    else if(ac=='['){ buffery=true; }                                          // nm[i] — index use (safe)
                    else bareUse=true;                                                         // nm read/written as a SCALAR value -> retyping to char[] would corrupt it
                    p=af; }
                size_t fg=L.find("fgets("); if(fg!=string::npos && L.find(amp)!=string::npos){ vector<string> aa=splitArgs(L.substr(fg+6)); if(aa.size()>=2){ long n=strtol(trim(aa[1]).c_str(),0,0); if(n>0&&n<=4096)sz=(int)n; } }
                // a memset/memcpy/bzero into this buffer fixes its minimum size — `memset(&buf,0,101)` => buf must hold >=101 bytes (else a stack overflow miscompiles)
                for(const char* mf:{"memset(","memcpy(","memmove(","bzero("}){ size_t mp=L.find(mf); if(mp==string::npos)continue;
                    vector<string> aa=splitArgs(L.substr(mp+strlen(mf))); if(aa.empty())continue; string a0=trim(aa[0]);
                    if(a0!=amp && a0!=nm) continue;                                              // the dest must be THIS buffer
                    int si=(string(mf)=="bzero(")?1:2; if((int)aa.size()>si){ long n=strtol(trim(aa[si]).c_str(),0,0); if(n>sz&&n<=(1<<20))sz=(int)n; } }
                // a constant index `buf[K]` fixes the minimum size to K+1
                { size_t ip=0; while((ip=L.find(nm,ip))!=string::npos){ bool lb=ip==0||!isIdent(L[ip-1]); size_t af=ip+nm.size();
                    size_t q=string::npos; if(af<L.size()&&L[af]=='[') q=af+1; else if(af+1<L.size()&&L[af]==')'&&L[af+1]=='[') q=af+2;   // `nm[idx]` or `(&nm)[idx]`
                    if(lb && q!=string::npos){ int d2=0; size_t e2=q; for(;e2<L.size();e2++){ if(L[e2]=='[')d2++; else if(L[e2]==']'){ if(d2==0)break; d2--; } }   // bound the subscript
                        if(e2<L.size()){ string idx=trim(L.substr(q,e2-q));
                            bool allDig=!idx.empty(); for(char c:idx) if(!isdigit((unsigned char)c))allDig=false;
                            if(allDig){ long k=strtol(idx.c_str(),0,0); if(k+1>sz)sz=(int)k+1; }                                  // constant index nm[K] -> size >= K+1
                            size_t sh=idx.rfind(" << "); if(sh!=string::npos){ int k=atoi(idx.c_str()+sh+4); if(k>=1&&k<=3&&k>eshift)eshift=k; } } }   // nm[EXPR << k] => element stride 1<<k (int/short/long array, not char)
                    ip=af; } } }
            if(!buffery||bareUse) continue;   // only promote buffers used PURELY as &nm / nm[i] — never type-punned as a scalar
            // element type from the access stride: <<2 => int[], <<3 => long[], <<1 => short[], else char[]
            const char* ety = eshift==3?"long":eshift==2?"int":eshift==1?"short":"char"; int es=1<<eshift; int cnt=(sz+es-1)/es;
            lines[declLine[nm]] = indentOf(lines[declLine[nm]]) + ety + " " + nm + "[" + std::to_string(cnt) + "];";
            for(size_t r=s;r<e;r++){ if(r==declLine[nm])continue; lines[r]=repTok(lines[r],amp,nm); }
        }
    });
}

// fold a pointer-alias temp: `T* buf;` + a single `buf = ARR;` (ARR a plain local, e.g. a promoted array) ->
// replace `buf` with `ARR` everywhere and drop the decl + the assignment. Removes the `long* buf; buf = key;` slop.
static void foldArrayAlias(vector<string>& lines){
    eachFunction(lines, [&](size_t h, size_t s, size_t e){
        std::map<string,size_t> ptrDecl;
        for(size_t r=s;r<e;r++){ string d=trim(lines[r]);
            for(const char* ty:{"long* ","char* ","int* ","long *","char *","int *"}){ size_t tl=strlen(ty);
                if(d.rfind(ty,0)==0 && d.back()==';' && d.find('=')==string::npos && d.find('(')==string::npos){
                    string nm=trim(d.substr(tl,d.size()-tl-1)); if(!nm.empty()&&(isalpha((unsigned char)nm[0])||nm[0]=='_')&&nm.find(',')==string::npos) ptrDecl[nm]=r; } } }
        for(auto& kv:ptrDecl){ const string& nm=kv.first; int defs=0; size_t defLine=0; string tgt;
            for(size_t r=s;r<e;r++){ string t=trim(lines[r]); string pre=nm+" = ";
                if(t.rfind(pre,0)==0){ defs++; defLine=r; tgt=t.substr(pre.size()); if(!tgt.empty()&&tgt.back()==';')tgt.pop_back(); } }
            bool simpleTgt=!tgt.empty()&&(isalpha((unsigned char)tgt[0])||tgt[0]=='_'); for(char c:tgt) if(!isIdent(c)) simpleTgt=false;
            if(defs!=1||!simpleTgt) continue;
            // target must not be reassigned (arrays never are; guards against aliasing a mutated pointer)
            bool tgtMutated=false; for(size_t r=s;r<e;r++){ string t=trim(lines[r]); if(t.rfind(tgt+" = ",0)==0||t.rfind(tgt+"++",0)==0||t.rfind(tgt+" +=",0)==0) tgtMutated=true; }
            if(tgtMutated) continue;
            for(size_t r=s;r<e;r++){ if(r==kv.second||r==defLine)continue; lines[r]=repTok(lines[r],nm,tgt); }
            lines[kv.second]="\x01DROP"; lines[defLine]="\x01DROP";
        }
    });
    vector<string> keep; for(auto& l:lines) if(l!="\x01DROP") keep.push_back(l); lines.swap(keep);
}

// An array-of-struct walk `p[i].field` is lowered to byte math `*(p + i*STRIDE)` / `*((p + i*STRIDE)+OFF)`,
// which collapse renders as `p[i * STRIDE]` and `(p + i * STRIDE)[OFF]`. Recover the struct: when a base is
// indexed at a consistent STRIDE with TWO+ distinct field offsets (proving it's a struct, not a plain typed
// array), synthesize `struct StructN { … }` from the offsets, retype the base to `struct StructN*`, and rewrite
// every access to `base[idx].field_OFF`. (A single offset 0 is just a typed array — left alone.)
static bool isIdentName(const string& s);
namespace { struct SAcc{ size_t pos,len; string base,idx; long stride,off; }; }
static bool parseMul(string c,string& idx,long& K){                   // "IDX * <int>"  (last top-level ' * ')
    c=trim(c);
    for(;;){ if(c.size()<2||c.front()!='(')break; int dd=0; bool wrap=true;   // strip a fully-enclosing paren pair: `(i * 24)` -> `i * 24`
        for(size_t i=0;i<c.size();i++){ if(c[i]=='(')dd++; else if(c[i]==')'){ if(--dd==0){ if(i!=c.size()-1)wrap=false; break; } } }
        if(!wrap)break; c=trim(c.substr(1,c.size()-2)); }
    int d=0; long op=-1,opk=0; for(size_t i=1;i+1<c.size();i++){ char ch=c[i]; if(ch=='('||ch=='[')d++; else if(ch==')'||ch==']')d--;
        else if(d==0&&ch=='*'&&c[i-1]==' '&&c[i+1]==' '){op=(long)i;opk=1;}                              // IDX * K
        else if(d==0&&ch=='<'&&i+1<c.size()&&c[i+1]=='<'&&c[i-1]==' '){op=(long)i;opk=2;} }              // IDX << S  (stride = 1<<S)
    if(op<0)return false; string r=trim(c.substr(op+(opk==2?2:1))); if(r.empty()||!(isdigit((unsigned char)r[0])))return false;
    for(char ch:r) if(!isxdigit((unsigned char)ch)&&ch!='x')return false; long n=strtol(r.c_str(),0,0); K=opk==2?(1L<<n):n; if(K<2)return false; idx=trim(c.substr(0,op)); return !idx.empty(); }
static vector<SAcc> scanStructAccess(const string& L){
    vector<SAcc> out; auto m=codeMask(L);
    for(size_t i=0;i<L.size();i++){ if(i>=m.size()||!m[i])continue;
        if(L[i]=='('){ int d=0; size_t j=i; for(;j<L.size();j++){ if(j<m.size()&&!m[j])continue; if(L[j]=='(')d++; else if(L[j]==')'){ if(--d==0)break; } }   // form B: (BASE + IDX*K)[OFF]
            if(j+1>=L.size()||L[j+1]!='[')continue; size_t b=j+2,e=b; while(e<L.size()&&(isxdigit((unsigned char)L[e])||L[e]=='x'))e++;
            if(e==b||e>=L.size()||L[e]!=']')continue; long off=strtol(L.substr(b,e-b).c_str(),0,0);
            string in=L.substr(i+1,j-i-1); int dd=0; long plus=-1; for(size_t k=0;k+2<in.size();k++){ char ch=in[k]; if(ch=='('||ch=='[')dd++; else if(ch==')'||ch==']')dd--; else if(dd==0&&ch=='+'&&in[k-1]==' '&&in[k+1]==' '){plus=(long)k;break;} }
            if(plus<0)continue; string base=trim(in.substr(0,plus)); if(!isIdentName(base))continue; string idx; long K; if(!parseMul(trim(in.substr(plus+1)),idx,K))continue;
            out.push_back({i,e+1-i,base,idx,K,off}); i=e; continue; }
        if(isalpha((unsigned char)L[i])||L[i]=='_'){ size_t e=i; while(e<L.size()&&isIdent(L[e]))e++;       // form A: BASE[IDX*K]  (== field 0)
            if(e<L.size()&&L[e]=='['&&(i==0||!isIdent(L[i-1]))){ int d=0; size_t j=e; for(;j<L.size();j++){ if(L[j]=='[')d++; else if(L[j]==']'){ if(--d==0)break; } }
                if(j<L.size()){ string base=L.substr(i,e-i); string idx; long K; if(parseMul(L.substr(e+1,j-e-1),idx,K)){ out.push_back({i,j+1-i,base,idx,K,0}); i=j; continue; } } }
            i=e-1; } }
    return out;
}
static void recoverStructArray(vector<string>& lines){
    bool precise = getenv("EMBER_STRUCT_WIDTHS")!=nullptr;          // OPT-IN: use the lift's per-field access widths
    static bool warned=false;
    map<string,string> structName; vector<string> structDefs; set<string> usedNames;
    auto getStruct=[&](long stride,const set<long>& offs,const map<long,int>& fw)->string{
        string key=std::to_string(stride); for(long o:offs){ key+="|"+std::to_string(o); if(precise&&fw.count(o))key+=":"+std::to_string(fw.at(o)); }
        if(structName.count(key))return structName[key];
        string nm="Struct"+std::to_string(stride); { string c=nm; for(int s2=2;usedNames.count(c);s2++)c=nm+"_"+std::to_string(s2); nm=c; } usedNames.insert(nm);
        vector<long> ov(offs.begin(),offs.end()); string def="struct "+nm+" {\n";
        for(size_t i=0;i<ov.size();i++){ long o=ov[i],next=(i+1<ov.size())?ov[i+1]:stride,gap=next-o;   // observed offset = a SCALAR field
            long w; const char* ty;                                                                      // PRECISE: use the recorded access width; else type by the gap (default int) + pad
            if(precise && fw.count(o)){ w=fw.at(o); ty=w==1?"char":w==2?"short":w==4?"int":w==8?"long":"int"; }
            else { ty=gap==1?"char":gap==2?"short":gap==4?"int":gap==8?"long":"int"; w=gap==1?1:gap==2?2:gap==8?8:4; }
            if(w>gap)w=gap;
            def+="    "+string(ty)+" field_"+std::to_string(o)+";\n";
            if(gap>w) def+="    char pad_"+std::to_string(o+w)+"["+std::to_string(gap-w)+"];\n"; }
        def+="};"; structName[key]=nm; structDefs.push_back(def); return nm; };
    eachFunction(lines,[&](size_t h,size_t s,size_t e){
        map<long,int> fw;                                           // parse the lift's `// @sfields off=width` hint, then blank it
        for(size_t r=s;r<e;r++){ string t=trim(lines[r]); if(t.rfind("// @sfields",0)!=0)continue;
            size_t p=11; while(p<t.size()){ while(p<t.size()&&t[p]==' ')p++; size_t a=p; while(p<t.size()&&isdigit((unsigned char)t[p]))p++; if(p>a&&p<t.size()&&t[p]=='='){ long off=atol(t.substr(a,p-a).c_str()); int wv=atoi(t.c_str()+p+1); fw[off]=wv; } size_t sp=t.find(' ',p); p=(sp==string::npos)?t.size():sp+1; }
            lines[r]="\x01DROP"; }
        map<string,map<long,set<long>>> info;                       // base -> stride -> offsets seen
        for(size_t r=s;r<e;r++) for(auto& a:scanStructAccess(lines[r])) info[a.base][a.stride].insert(a.off);
        map<string,long> chosen; map<string,string> baseStruct;     // bases that are genuine struct arrays
        for(auto& kv:info){ for(auto& so:kv.second){ if(so.second.size()>=2){ chosen[kv.first]=so.first; baseStruct[kv.first]=getStruct(so.first,so.second,fw);
            if(precise&&!warned){ warned=true; fprintf(stderr,"ember: struct-field widths are inferred from observed accesses; unobserved fields are padding and field types are best-effort — verify before relying on the struct layout.\n"); } } } }
        if(chosen.empty())return;
        for(size_t r=s;r<e;r++){ auto accs=scanStructAccess(lines[r]); if(accs.empty())continue;
            for(size_t a=accs.size();a-->0;){ SAcc& A=accs[a]; if(!chosen.count(A.base)||chosen[A.base]!=A.stride)continue;
                lines[r]=lines[r].substr(0,A.pos)+A.base+"["+A.idx+"].field_"+std::to_string(A.off)+lines[r].substr(A.pos+A.len); } }
        for(auto& kv:baseStruct){ const string& base=kv.first, &sn=kv.second;   // retype the base (param in header, or a local decl)
            string ph="long "+base; size_t pp=lines[h].find(ph); if(pp!=string::npos && (pp+ph.size()>=lines[h].size()||!isIdent(lines[h][pp+ph.size()]))){ lines[h]=lines[h].substr(0,pp)+"struct "+sn+"* "+base+lines[h].substr(pp+ph.size()); continue; }
            for(size_t r=s;r<e;r++){ string t=trim(lines[r]); if(t=="long "+base+";"){ lines[r]=indentOf(lines[r])+"struct "+sn+"* "+base+";"; break; } } }
    });
    { vector<string> keep; for(auto& l:lines) if(l!="\x01DROP") keep.push_back(l); lines.swap(keep); }   // drop the consumed @sfields hint lines
    if(structDefs.empty())return;
    size_t ins=0; for(;ins<lines.size();ins++) if(isFnHeader(lines[ins]))break;   // emit struct defs just before the first function
    vector<string> blk; for(auto& d:structDefs){ size_t p=0,q; while((q=d.find('\n',p))!=string::npos){ blk.push_back(d.substr(p,q-p)); p=q+1; } blk.push_back(d.substr(p)); blk.push_back(""); }
    lines.insert(lines.begin()+ins, blk.begin(), blk.end());
}
// ── CONTEXT NAMING ── an input buffer read by fgets/scanf right after a prompt `printf("… key: ")` is named
// from the prompt keyword: `fgets(&v56, …)` after `printf("crackme :: key (8 chars): ")` -> the local becomes `key`.
// This is the "name it by what the program does" win for the generic vN that no behavioral heuristic could catch.
static const std::set<string> g_promptStop={"crackme","please","enter","the","your","this","that","input","and","for","chars","char"};
static const std::set<string> g_promptRes={"int","long","char","short","void","unsigned","return","if","else","while","switch","struct","const","static","main","argc","argv","stdin","stdout","stderr","buf","len"};
// the keyword from the most recent prompt printf/puts above line `before` within [fs,fe) — "key"/"name"/"password"
static string promptKeyword(const vector<string>& lines,size_t fs,size_t before){
    string prompt;
    for(size_t q=before; q-->fs+1; ){ string tq=trim(lines[q]); size_t s=string::npos;
        for(const char* fn:{"printf(\"","puts(\"","fputs(\""}){ size_t x=tq.find(fn); if(x!=string::npos){ s=tq.find('"',x); break; } }
        if(s!=string::npos){ size_t e=tq.find('"',s+1); if(e!=string::npos){ prompt=tq.substr(s+1,e-s-1); break; } }
        if(tq.empty())continue; if(tq=="{"||tq.back()=='}')break; }
    if(prompt.empty())return "";
    size_t cc=prompt.find("::"); string rest=cc!=string::npos?prompt.substr(cc+2):prompt;
    vector<string> words; { string cur; for(char c:rest){ if(isalpha((unsigned char)c))cur+=(char)tolower(c); else { if(cur.size()>=2)words.push_back(cur); cur.clear(); } } if(cur.size()>=2)words.push_back(cur); }
    for(auto& w:words) if(w.size()>=3 && !g_promptStop.count(w) && !g_promptRes.count(w)) return w;
    return "";
}
// INPUT ARRAY RECOVERY: a char buffer read by fgets whose bytes are accessed at CONSTANT offsets (v8=key[0],
// v9=key[1], …, since slot(off)=v<off>) — recover the array. `fgets(&v8,…); v13 + v8 …` -> `char key[N]; key[5]+key[0]…`.
static void recoverInputArray(vector<string>& lines){
    size_t i=0;
    while(i<lines.size()){
        string t=trim(lines[i]);
        bool hdr = indentOf(lines[i]).empty() && !t.empty() && t.back()=='{' && t.find('(')!=string::npos
                   && t.rfind("if",0)!=0 && t.rfind("for",0)!=0 && t.rfind("while",0)!=0 && t.rfind("switch",0)!=0 && t.rfind("struct",0)!=0 && t.rfind("class",0)!=0;
        if(!hdr){ i++; continue; }
        size_t fs=i,fe=i; int d=0; for(size_t r=i;r<lines.size();r++){ d+=braceDelta(lines[r]); if(d==0){ fe=r; break; } }
        if(fe<=fs){ i++; continue; }
        long base=-1,sz=0; size_t readLine=0;
        for(size_t r=fs+1;r<fe;r++){ string tr=trim(lines[r]); size_t p=tr.find("fgets(&v"); if(p==string::npos)p=tr.find("gets(&v");
            if(p==string::npos)continue; size_t a=tr.find("&v",p)+2,e=a; while(e<tr.size()&&isdigit((unsigned char)tr[e]))e++; if(e==a)continue;
            base=atol(tr.substr(a,e-a).c_str()); long defSz=64; size_t cm=tr.find(',',e);
            if(cm!=string::npos){ size_t b=cm+1; while(b<tr.size()&&tr[b]==' ')b++; size_t b2=b; while(b2<tr.size()&&isdigit((unsigned char)tr[b2]))b2++; if(b2>b)defSz=atol(tr.substr(b,b2-b).c_str()); }
            sz=defSz; readLine=r; break; }
        if(base<0||sz<2||sz>4096){ i=fe+1; continue; }
        // byte widths from the lifter's `// @widths vN=W` hint — only width-1 slots are buffer bytes
        std::map<long,int> wid; for(size_t r=fs+1;r<fe;r++){ string ht=trim(lines[r]); if(ht.rfind("// @widths",0)!=0)continue;
            size_t p=10; while(p<ht.size()){ while(p<ht.size()&&ht[p]==' ')p++; if(p>=ht.size()||ht[p]!='v')break; size_t a=p+1,e=a; while(e<ht.size()&&isdigit((unsigned char)ht[e]))e++;
                if(e<ht.size()&&ht[e]=='='){ long n=atol(ht.substr(a,e-a).c_str()); int w=atoi(ht.c_str()+e+1); wid[n]=w; } size_t sp=ht.find(' ',p); p=(sp==string::npos)?ht.size():sp+1; } }
        auto tokIn=[&](const string& nm){ for(size_t r=fs;r<=fe;r++){ const string& L=lines[r]; auto m=codeMask(L); size_t p=0; while((p=L.find(nm,p))!=string::npos){ if(p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1]))&&(p+nm.size()>=L.size()||!isIdent(L[p+nm.size()])))return true; p+=nm.size(); } } return false; };
        std::vector<long> idxs; for(long k=0;k<sz&&k<512;k++){ long off=base+k; if(wid.count(off)&&wid[off]!=1)continue;   // skip non-byte slots
            if(tokIn("v"+std::to_string(off))) idxs.push_back(k); }
        bool split=false; for(long k:idxs) if(k>=1){ split=true; break; }      // need >=1 byte beyond the base = a real byte-split array
        if(!split){ i=fe+1; continue; }
        string name=promptKeyword(lines,fs,readLine); if(name.empty())name="inp";
        { string b=name; int s=2; while(tokIn(b))b=name+std::to_string(s++); name=b; }
        for(size_t r=fs;r<=fe;r++) for(long k:idxs) lines[r]=repTok(lines[r],"v"+std::to_string(base+k),name+"["+std::to_string(k)+"]");
        for(size_t r=fs;r<=fe;r++){ string& L=lines[r]; string from="&"+name+"[0]"; size_t p; while((p=L.find(from))!=string::npos) L.replace(p,from.size(),name); }   // &key[0] -> key (array decay)
        lines.insert(lines.begin()+fs+1, "    char "+name+"["+std::to_string(sz)+"];");
        fe++; i=fe+1;
    }
}
static void nameInputBuffers(vector<string>& lines){
    const std::set<string>& STOP=g_promptStop;
    const std::set<string>& RES=g_promptRes;
    size_t i=0;
    while(i<lines.size()){
        string t=trim(lines[i]);
        bool hdr = indentOf(lines[i]).empty() && !t.empty() && t.back()=='{' && t.find('(')!=string::npos
                   && t.rfind("if",0)!=0 && t.rfind("for",0)!=0 && t.rfind("while",0)!=0 && t.rfind("switch",0)!=0 && t.rfind("struct",0)!=0 && t.rfind("class",0)!=0;
        if(!hdr){ i++; continue; }
        size_t fs=i,fe=i; int d=0; for(size_t r=i;r<lines.size();r++){ d+=braceDelta(lines[r]); if(d==0){ fe=r; break; } }
        if(fe<=fs){ i++; continue; }
        auto tokIn=[&](const string& nm){ for(size_t r=fs;r<=fe;r++){ const string& L=lines[r]; auto m=codeMask(L); size_t p=0; while((p=L.find(nm,p))!=string::npos){ if(p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1]))&&(p+nm.size()>=L.size()||!isIdent(L[p+nm.size()])))return true; p+=nm.size(); } } return false; };   // skip matches inside string literals (the prompt itself contains "key")
        std::map<string,string> ren; std::set<string> taken;
        for(size_t r=fs+1;r<fe;r++){ string tr=trim(lines[r]); string v;
            for(const char* fn:{"fgets(&","gets(&"}){ size_t p=tr.find(fn); if(p!=string::npos){ size_t a=tr.find('&',p)+1,e=a; while(e<tr.size()&&isIdent(tr[e]))e++; v=tr.substr(a,e-a); break; } }
            if(v.empty()){ size_t p=tr.find("scanf("); if(p!=string::npos){ size_t a=tr.rfind('&'); if(a!=string::npos){ size_t e=a+1; while(e<tr.size()&&isIdent(tr[e]))e++; v=tr.substr(a+1,e-a-1); } } }
            if(v.size()<2||v[0]!='v'||!isdigit((unsigned char)v[1])||ren.count(v))continue;
            string prompt;
            for(size_t q=r; q-->fs+1; ){ string tq=trim(lines[q]); size_t s=string::npos;
                for(const char* fn:{"printf(\"","puts(\"","fputs(\""}){ size_t x=tq.find(fn); if(x!=string::npos){ s=tq.find('"',x); break; } }
                if(s!=string::npos){ size_t e=tq.find('"',s+1); if(e!=string::npos){ prompt=tq.substr(s+1,e-s-1); break; } }
                if(tq.empty()) continue; if(tq=="{"||tq.back()=='}') break; }
            if(prompt.empty())continue;
            size_t cc=prompt.find("::"); string rest=cc!=string::npos?prompt.substr(cc+2):prompt;
            vector<string> words; { string cur; for(char c:rest){ if(isalpha((unsigned char)c))cur+=(char)tolower(c); else { if(cur.size()>=2)words.push_back(cur); cur.clear(); } } if(cur.size()>=2)words.push_back(cur); }
            string kw; for(auto& w:words){ if(w.size()>=3 && !STOP.count(w) && !RES.count(w)){ kw=w; break; } }
            if(kw.empty())continue;
            string fin=kw; int sfx=2; while(tokIn(fin)||taken.count(fin)) fin=kw+std::to_string(sfx++);
            ren[v]=fin; taken.insert(fin); }
        for(auto& kv:ren) for(size_t r=fs;r<=fe;r++) lines[r]=repTok(lines[r],kv.first,kv.second);
        i=fe+1;
    }
}
// ((X)) -> (X) when the outer pair wraps exactly the inner pair (literal-aware)
static void dedupeParens(string& line){
    for(int guard=0; guard<40; guard++){ bool done=true; auto m=codeMask(line);
        for(size_t i=0;i+1<line.size();i++){ if(i+1>=m.size()||!m[i]||!m[i+1]||line[i]!='('||line[i+1]!='(') continue;
            int d=0; size_t innerClose=string::npos;
            for(size_t k=i+1;k<line.size();k++){ if(!m[k])continue; if(line[k]=='(')d++; else if(line[k]==')'){ if(--d==0){innerClose=k;break;} } }
            if(innerClose==string::npos||innerClose+1>=line.size()) continue;
            if(m[innerClose+1]&&line[innerClose+1]==')'){ line.erase(innerClose+1,1); line.erase(i,1); done=false; break; } }
        if(done) break; }
}

static string stripOuterParens(const string& line){
    string ind=indentOf(line), body=trim(line);
    if(body.size()>3 && body.front()=='(' && body.substr(body.size()-2)==");"){
        auto m=codeMask(body); int d=0; bool wraps=true;
        for(size_t i=0;i<body.size()-1;i++){ if(!m[i])continue; if(body[i]=='(')d++; else if(body[i]==')'){ if(--d==0 && i!=body.size()-2){ wraps=false; break; } } }
        if(wraps) return ind+trim(body.substr(1,body.size()-3))+";"; }
    return line;
}

// fold `x = (x OP y);` / `x = x OP y;` -> `x OP= y;`  (and `x += 1;` -> `x++;`). Folds ONLY when the
// operand is a single top-level term, so grouping/precedence can never change (`x = x*y + z;` is left alone).
static string foldCompound(const string& line){
    string ind=indentOf(line), t=trim(line);
    if(t.size()<5 || t.back()!=';') return line; string body=t.substr(0,t.size()-1);
    size_t eq=body.find(" = "); if(eq==string::npos) return line;
    string lhs=body.substr(0,eq), rhs=body.substr(eq+3);
    if(lhs.empty()||!(isalpha((unsigned char)lhs[0])||lhs[0]=='_')) return line;
    for(char c:lhs){ if(!(isalnum((unsigned char)c)||c=='_'||c=='.'||c=='['||c==']'||c=='-'||c=='>')) return line; }   // simple lvalue: ident, ->f, .f, [i]
    if(rhs.size()>=2 && rhs.front()=='(' && rhs.back()==')'){ int d=0; bool wrap=true;                                // strip one fully-wrapping paren layer
        for(size_t i=0;i<rhs.size();i++){ if(rhs[i]=='(')d++; else if(rhs[i]==')'){ if(--d==0 && i!=rhs.size()-1){ wrap=false; break; } } }
        if(wrap) rhs=rhs.substr(1,rhs.size()-2); }
    if(rhs.compare(0,lhs.size(),lhs)!=0) return line; size_t p=lhs.size();
    if(p>=rhs.size()||rhs[p]!=' ') return line; p++;
    static const char* OPS[]={"<<",">>","+","-","*","/","%","&","|","^"}; string op;
    for(auto o:OPS){ size_t ol=strlen(o); if(rhs.compare(p,ol,o)==0){ op=o; p+=ol; break; } }
    if(op.empty()||p>=rhs.size()||rhs[p]!=' ') return line; p++;
    string operand=rhs.substr(p); if(operand.empty()) return line;
    { int d=0; for(size_t i=0;i<operand.size();i++){ char c=operand[i]; if(c=='('||c=='[')d++; else if(c==')'||c==']')d--;   // operand must have NO top-level binary operator
        else if(d==0 && i>0 && i+1<operand.size() && operand[i-1]==' ' && operand[i+1]==' ' && (c=='+'||c=='-'||c=='*'||c=='/'||c=='%'||c=='&'||c=='|'||c=='^'||c=='<'||c=='>')) return line; } }
    if((op=="+"||op=="-") && operand=="1") return ind+lhs+(op=="+"?"++":"--")+";";
    return ind+lhs+" "+op+"= "+operand+";";
}

// --- pass-3 helpers -----------------------------------------------------------
// if trimmed line is "<temp> <op>= <rhs>;" return the temp; set compound + rhs(no ';')
static string assignLHS(const string& line, bool& compound, string& rhs){
    compound=false; rhs=""; string t=trim(line);
    if(t.empty()||!(t[0]=='v'||t[0]=='t')) return ""; size_t i=1; while(i<t.size()&&isdigit((unsigned char)t[i]))i++;
    if(i==1 || (i<t.size()&&isIdent(t[i]))) return "";                 // need digits + clean boundary
    string lhs=t.substr(0,i); while(i<t.size()&&t[i]==' ')i++;
    static const char* C[]={"<<=",">>=","+=","-=","*=","/=","%=","&=","|=","^="};
    for(auto c:C){ size_t n=strlen(c); if(t.compare(i,n,c)==0){ compound=true; string r=trim(t.substr(i+n)); if(!r.empty()&&r.back()==';')r.pop_back(); rhs=trim(r); return lhs; } }
    if(i<t.size()&&t[i]=='='&&(i+1>=t.size()||t[i+1]!='=')){ string r=trim(t.substr(i+1)); if(!r.empty()&&r.back()==';')r.pop_back(); rhs=trim(r); return lhs; }
    return "";
}
// names written anywhere in a line (assignment LHS base, compound-assign, ++/--) — for anti-dependence
static void collectMutated(const string& s, set<string>& out){
    auto m=codeMask(s);
    for(size_t i=0;i+1<s.size();i++){ if(!m[i])continue;
        if((s[i]=='+'&&s[i+1]=='+')||(s[i]=='-'&&s[i+1]=='-')){
            size_t b=i; while(b>0&&isIdent(s[b-1]))b--; if(b<i) out.insert(s.substr(b,i-b));
            size_t a=i+2, e=a; while(e<s.size()&&isIdent(s[e]))e++; if(e>a) out.insert(s.substr(a,e-a)); } }
    for(size_t i=0;i<s.size();i++){ if(!m[i]||s[i]!='=')continue;
        bool cmp=(i+1<s.size()&&s[i+1]=='=')||(i>0&&(s[i-1]=='='||s[i-1]=='!'||s[i-1]=='<'||s[i-1]=='>'));
        if(cmp) continue; int j=(int)i-1; while(j>=0&&s[j]==' ')j--; int e=j; while(j>=0&&isIdent(s[j]))j--;
        if(e>j) out.insert(s.substr(j+1,e-j)); }
}
// like collectMutated but ONLY `++`/`--` (NOT plain `=` or compound-assign). copy-prop uses this for its
// "don't propagate into a var that's also incremented" guard — a single-def var (def==1) is otherwise
// safe to inline, and using the full collectMutated (which marks every `=` LHS) here wrongly blocked
// essentially ALL copy-propagation. (Compound-assign `v+=x` already bumps def past 1, so it's excluded.)
static void collectIncDec(const string& s, set<string>& out){
    auto m=codeMask(s);
    for(size_t i=0;i+1<s.size();i++){ if(!m[i])continue;
        if((s[i]=='+'&&s[i+1]=='+')||(s[i]=='-'&&s[i+1]=='-')){
            size_t b=i; while(b>0&&isIdent(s[b-1]))b--; if(b<i) out.insert(s.substr(b,i-b));
            size_t a=i+2, e=a; while(e<s.size()&&isIdent(s[e]))e++; if(e>a) out.insert(s.substr(a,e-a)); } }
}
static bool hasSideEffect(const string& r){
    if(r.find('(')!=string::npos||r.find("++")!=string::npos||r.find("--")!=string::npos) return true;
    if(hasTok(r,"cout")||hasTok(r,"cerr")||hasTok(r,"clog")||hasTok(r,"wcout")||hasTok(r,"wcerr")||hasTok(r,"wclog")) return true;   // `cout << x` PRINTS — never a dead store, even without parens
    auto m=codeMask(r); for(size_t i=0;i<r.size();i++){ if(!m[i]||r[i]!='=')continue;
        bool cmp=(i+1<r.size()&&r[i+1]=='=')||(i>0&&(r[i-1]=='='||r[i-1]=='!'||r[i-1]=='<'||r[i-1]=='>')); if(!cmp) return true; }
    return false;
}

// ── stack-canary strip (G6): -O0 clang inserts a stack-smashing guard whose decompiled
// form (`vN = __stack_chk_guard->f0;` … `if (__stack_chk_guard->f0 != vN) __stack_chk_fail();`)
// plus a bogus `struct S_*___stack_chk_guard` leaks ~5 undeclared identifiers per function.
// It is behavior-irrelevant — strip it entirely. Runs first so the canary temp is never declared. ──
static void stripCanary(vector<string>& lines){
    vector<string> keep;
    for(size_t i=0;i<lines.size();){
        string t=trim(lines[i]);
        auto skipBlock=[&](){ int d=0; bool started=false;
            while(i<lines.size()){ auto m=codeMask(lines[i]); for(size_t k=0;k<lines[i].size();k++){ if(!m[k])continue; if(lines[i][k]=='{'){d++;started=true;} else if(lines[i][k]=='}')d--; } i++; if(started&&d<=0)break; } };
        if(t.rfind("struct ",0)==0 && t.find("stack_chk_guard")!=string::npos && t.back()=='{'){ skipBlock(); continue; }
        if(t.rfind("if",0)==0 && t.find("__stack_chk_guard")!=string::npos && t.back()=='{'){ skipBlock(); continue; }
        if(t.find("__stack_chk_guard")!=string::npos || t.find("__stack_chk_fail")!=string::npos){ i++; continue; }
        keep.push_back(lines[i]); i++;
    }
    lines.swap(keep);
}

// ── declare-locals pass (G1+G4): the decompiler emits stack slots (vN), call temps
// (tN), recovered names (i/sum/...) and leaked registers (a0/x9/...) WITHOUT ever
// declaring them, so the output never compiles. This pass walks each function, finds
// every identifier that is assigned / address-taken / dereferenced but is neither a
// param nor already declared, infers a type from its usage, and emits declarations at
// the top of the function body. 100% text + literal-aware. ──
static bool startsWithType(const string& t){
    static const char* T[]={"std::string","std::vector","unsigned","struct","static","const","int64_t","uint64_t",
        "int32_t","uint32_t","int16_t","int8_t","uint8_t","size_t","long","int","char","double","float","bool",
        "short","void","auto",0};
    for(int i=0;T[i];i++){ size_t n=strlen(T[i]); if(t.compare(0,n,T[i])==0 && (t.size()==n||!isIdent(t[n]))) return true; }
    return false;
}
// if `line` declares a variable (TYPE ... NAME [= ...];), record NAME as declared
static void collectDeclared(const string& line, set<string>& decl){
    string t=trim(line);
    if(!startsWithType(t)) return;
    if(t.rfind("struct ",0)==0 && t.back()=='{') return;          // struct DEFINITION, not a var decl
    auto m=codeMask(t); size_t stop=t.size();
    for(size_t i=0;i<t.size();i++){ if(!m[i])continue; if(t[i]=='('){ return; }   // a function decl/call, not a var
        if(t[i]=='='||t[i]==';'){ stop=i; break; } }
    long e=(long)stop-1; while(e>=0&&(t[e]==' '||t[e]=='\t'))e--;
    while(e>=0&&t[e]==']'){ int d=0; while(e>=0){ if(t[e]==']')d++; else if(t[e]=='['){ if(--d==0){e--;break;} } e--; } while(e>=0&&t[e]==' ')e--; }
    long ne=e; while(e>=0&&isIdent(t[e]))e--;
    if(ne>e){ string nm=t.substr(e+1,ne-e); if(!nm.empty()&&!isdigit((unsigned char)nm[0])) decl.insert(nm); }
}
static bool isBuiltinName(const string& s){
    static const set<string> B={"this","cout","cerr","clog","cin","wcout","wcerr","wclog","endl","ends","flush",
        "stdin","stdout","stderr","NULL","nullptr","true","false","string","std","return","if","else","for",
        "while","do","switch","case","default","break","continue","goto","sizeof","new","delete","int","long",
        "char","double","float","bool","void","unsigned","short","const","static","struct","class","auto",
        "operator","template","typename","namespace","using","public","private","protected","virtual",
        "throw","try","catch","mutable","explicit","friend","union","enum","typedef","register","volatile",
        "size_t","int64_t","uint64_t","int32_t","uint32_t","int8_t","uint8_t","printf","fprintf","sprintf",
        "snprintf","scanf","sscanf","malloc","free","calloc","realloc","memcpy","memset","memmove","memcmp",
        "strlen","strcmp","strncmp","strcpy","strncpy","strcat","strchr","strncat","puts","putchar","getchar",
        "fopen","fclose","fwrite","fread","fputs","fgets","exit","abort","assert","sqrt","pow","fabs","abs",
        "floor","ceil","sin","cos","tan","log","exp","atoi","atol","atof","strtol","rand","srand","qsort",
        "strtoul","strtoull","strtod","strstr","strrchr","strspn","strcspn","strtok","getenv","setenv",
        "system","popen","pclose","fgets","fputc","fgetc","fseek","ftell","rewind","perror","tmpfile",
        "isdigit","isalpha","isalnum","isspace","isupper","islower","ispunct","toupper","tolower",
        "access","chmod","rename","remove","unlink","getpid","sleep","usleep","time","clock"};
    return B.count(s)>0;
}
// is this identifier one the decompiler leaves undeclared (vN / tN / register / recovered name)?
// We declare anything ASSIGNED/address-taken/dereferenced that isn't a param, builtin, struct, global
// or function — so no explicit name whitelist is needed; this just blocks obvious non-locals.
static bool isDeclarable(const string& nm){
    if(nm.empty()||isdigit((unsigned char)nm[0])) return false;
    if(nm.rfind("g_",0)==0||nm.rfind("loc_",0)==0||nm.rfind("sub_",0)==0||nm.rfind("S_",0)==0||nm.rfind("s_",0)==0) return false;
    if(nm.rfind("__",0)==0) return false;                          // runtime internals (__stack_chk_guard, ...)
    return true;
}
// a leaked machine register (arm64 xN/wN/aN) the lifter couldn't resolve to a value. Declared as an
// uninitialized long so the output COMPILES; semantics may be off (real fix = lifter dataflow, G4).
static bool isRegister(const string& s){
    if(s.size()<2||s.size()>3) return false; char c=s[0];
    if(c=='x'||c=='w'){ for(size_t i=1;i<s.size();i++) if(!isdigit((unsigned char)s[i])) return false; int n=atoi(s.c_str()+1); return n>=0&&n<=30; }
    if(c=='a'&&s.size()==2&&isdigit((unsigned char)s[1])) return (s[1]-'0')<=7;
    static const set<string> X86={"rax","rcx","rdx","rbx","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};   // x86-64 leaked regs (rsp/rbp excluded)
    return X86.count(s)>0;
}
// ── stub undefined `g_<hex>` data globals (G11 bridge): the lifter references rodata/data globals it
// hasn't materialized; declare each as a buffer at file scope so the program compiles + links. The
// real fix is the lifter emitting the actual bytes; until then these are clearly-marked placeholders. ──
// drop dead bare-load statements: a line that is exactly `*(<expr>);` with NO call and NO assignment — a memory
// load whose result is discarded (the lifter often emits it AND inlines the same load into the next `if`). Pure,
// side-effect-free, redundant -> remove. Conservative: only `*(...)` loads, never calls/stores/decls.
static void dropDeadExprs(vector<string>& lines){
    vector<string> keep; keep.reserve(lines.size());
    for(auto& l:lines){ string t=trim(l); bool drop=false;
        if(t.size()>=4 && t[0]=='*' && t[1]=='(' && t.compare(t.size()-2,2,");")==0 && t.find('"')==string::npos){
            bool call=false,assign=false;
            for(size_t k=1;k<t.size();k++){ if(t[k]=='(' && (isalnum((unsigned char)t[k-1])||t[k-1]=='_'))call=true;
                if(t[k]=='=' && t[k-1]!='!' && t[k-1]!='<' && t[k-1]!='>' && t[k-1]!='=' && (k+1>=t.size()||t[k+1]!='='))assign=true; }
            if(!call && !assign) drop=true; }
        if(!drop) keep.push_back(l); }
    lines.swap(keep);
}
static void stubGlobals(vector<string>& lines){
    set<string> used, defined;
    for(auto& l:lines){ auto m=codeMask(l);
        for(size_t k=0;k<l.size();){ if(m[k]&&l[k]=='g'&&k+1<l.size()&&l[k+1]=='_'&&(k==0||!isIdent(l[k-1]))){ size_t e=k+2; while(e<l.size()&&isxdigit((unsigned char)l[e]))e++; if(e>k+2) used.insert(l.substr(k,e-k)); k=e; } else k++; } }
    for(auto& l:lines){ string t=trim(l); if(startsWithType(t)) for(const auto& g:used) if(t.find(g+"[")!=string::npos||t.find(" "+g+" ")!=string::npos||t.find("*"+g+" ")!=string::npos) defined.insert(g); }
    vector<string> stubs;
    for(const auto& g:used) if(!defined.count(g)) stubs.push_back("static long "+g+"[8192];   // unresolved data global — real bytes pending (lifter)");
    if(!stubs.empty()){ stubs.push_back(""); lines.insert(lines.begin(), stubs.begin(), stubs.end()); }
}
// ── index-fold: a typed file-scope array `static int T[N]` accessed as `*(T + (i << 2))` must read
// as `T[i]` (element-indexed), not byte-offset pointer math — both correctness AND readability. ──
static void foldDataIndex(vector<string>& lines){
    map<string,int> esz;                                            // typed array name -> element size in bytes
    for(auto& l:lines){ string t=trim(l); size_t br=t.find('['); if(br==string::npos)continue;
        if(t.rfind("static",0)!=0 && !startsWithType(t)) continue;
        long e=(long)br-1; while(e>=0&&t[e]==' ')e--; long b=e; while(b>=0&&isIdent(t[b]))b--; if(e<=b)continue; string nm=t.substr(b+1,e-b);
        string pre=t.substr(0,b+1);
        int sz = (pre.find('*')!=string::npos||pre.find("long")!=string::npos||pre.find("double")!=string::npos) ? 8
               : pre.find("int")!=string::npos ? 4 : pre.find("short")!=string::npos ? 2 : 1;
        esz[nm]=sz; }
    if(esz.empty()) return;
    for(auto& l:lines){
        for(auto& kv:esz){ const string& nm=kv.first; int sz=kv.second;
            string pat="*("+nm+" + (";                              // *(nm + (EXPR << k))  -> nm[EXPR]
            size_t p=0; auto m=codeMask(l);
            while((p=l.find(pat,p))!=string::npos){ if(p>=m.size()||!m[p]){ p+=pat.size(); continue; }
                size_t inner=p+pat.size()-1; string ie; size_t iend;
                if(!balanced(l,inner,ie,iend) || iend+1>=l.size() || l[iend+1]!=')'){ p+=pat.size(); continue; }
                size_t sh=ie.rfind(" << "); bool ok=false; string expr;
                if(sh!=string::npos){ int k=atoi(ie.c_str()+sh+4); if((1<<k)==sz){ expr=trim(ie.substr(0,sh)); ok=true; } }
                else if(sz==1){ expr=trim(ie); ok=true; }
                if(ok){ l=l.substr(0,p)+nm+"["+expr+"]"+l.substr(iend+2); m=codeMask(l); p=0; } else p+=pat.size(); }
            if(sz==1){ string pat2="*("+nm+" + "; size_t q=0; m=codeMask(l);   // stride-1 char: *(nm + EXPR) -> nm[EXPR]
                while((q=l.find(pat2,q))!=string::npos){ if(q>=m.size()||!m[q]||(q+pat2.size()<l.size()&&l[q+pat2.size()]=='(')){ q+=pat2.size(); continue; }
                    size_t s=q+pat2.size(),e2=s; int d=0; while(e2<l.size()){ char c=l[e2]; if(c=='(')d++; else if(c==')'){ if(d==0)break; d--; } e2++; }
                    if(e2<l.size()&&l[e2]==')'){ l=l.substr(0,q)+nm+"["+trim(l.substr(s,e2-s))+"]"+l.substr(e2+1); m=codeMask(l); q=0; } else q+=pat2.size(); } }
        } }
}
// ── typed-pointer index fold: once a pointer PARAMETER/LOCAL is typed (`int const* arr`), the lifter's
// byte-addressed subscript `arr[i << 2]` (or `arr[i * 4]`) is element `i*4`, not `arr[i]`. Recover the
// real element index by dividing out the element size — both correctness AND readability. Mirrors
// foldDataIndex (which only covers `static` file-scope arrays) for function params/locals. Runs AFTER
// cppifyHeaders has typed the params. Single-`*` scalar pointers only; struct/char*/unknown left alone.
static int ptrElemSize(const string& type){
    if(std::count(type.begin(),type.end(),'*')!=1) return 0;          // exactly one level of indirection
    string b; for(size_t i=0;i<type.size();i++){ char c=type[i]; if(c=='*'||c=='&')continue; b+=c; }
    // strip qualifiers
    for(const char* q:{"const","volatile","unsigned","signed","static","struct"}){ size_t p; while((p=b.find(q))!=string::npos) b.erase(p,strlen(q)); }
    b=trim(b);
    if(b=="char"||b=="int8_t"||b=="uint8_t"||b=="bool") return 1;
    if(b=="short"||b=="int16_t"||b=="uint16_t") return 2;
    if(b=="int"||b=="int32_t"||b=="uint32_t"||b=="float") return 4;
    if(b=="long"||b=="double"||b=="int64_t"||b=="uint64_t"||b=="size_t"||b=="long long") return 8;
    return 0;
}
// if subscript `idx` is top-level `EXPR << k` with (1<<k)==elemSize, or `EXPR * elemSize`, return EXPR; else ""
static string divideIndex(const string& idx, int elemSize){
    int depth=0;
    for(size_t i=0;i+1<idx.size();i++){ char c=idx[i];
        if(c=='('||c=='[')depth++; else if(c==')'||c==']'){ if(depth)depth--; }
        else if(depth==0 && c=='<' && idx[i+1]=='<'){
            string lhs=trim(idx.substr(0,i)), rhs=trim(idx.substr(i+2));
            if(!lhs.empty() && !rhs.empty() && rhs.find_first_not_of("0123456789")==string::npos){
                int k=atoi(rhs.c_str()); if(k>=0&&k<31&&(1<<k)==elemSize) return stripOuterParens(lhs); }
            return ""; }
        else if(depth==0 && c=='*' && i>0 && idx[i-1]!='*' && idx[i+1]!='*'){
            string lhs=trim(idx.substr(0,i)), rhs=trim(idx.substr(i+1));
            if(!lhs.empty() && !rhs.empty() && rhs.find_first_not_of("0123456789")==string::npos
               && atoi(rhs.c_str())==elemSize) return stripOuterParens(lhs);
            return ""; } }
    return "";
}
// element byte-size of a plain (non-pointer) scalar type, else 0
static int scalarSize(const string& type){
    if(type.find('*')!=string::npos) return 0;
    string b=type; for(const char* q:{"const","volatile","unsigned","signed","static"}){ size_t p; while((p=b.find(q))!=string::npos) b.erase(p,strlen(q)); }
    b=trim(b);
    if(b=="char"||b=="int8_t"||b=="uint8_t"||b=="bool") return 1;
    if(b=="short"||b=="int16_t"||b=="uint16_t") return 2;
    if(b=="int"||b=="int32_t"||b=="uint32_t"||b=="float") return 4;
    if(b=="long"||b=="double"||b=="int64_t"||b=="uint64_t"||b=="size_t"||b=="long long") return 8;
    return 0;
}
// `(&x)->member` -> `x.member` (address-of then arrow = direct member access; a safe C identity).
// copy-prop inlines `p = &x; p->f` into `(&x)->f`; this cleans it up — esp. method calls `(&v60)->tick()`.
static void foldAddrArrow(string& l){
    auto m=codeMask(l); size_t p=0;
    while((p=l.find("(&",p))!=string::npos){ if(p>=m.size()||!m[p]){ p+=2; continue; }
        size_t s=p+2, e=s; while(e<l.size()&&isIdent(l[e]))e++;
        if(e>s && e+2<l.size() && l[e]==')' && l[e+1]=='-' && l[e+2]=='>'){
            string id=l.substr(s,e-s); l=l.substr(0,p)+id+"."+l.substr(e+3); m=codeMask(l); p+=id.size()+1; }
        else p+=2; }
}
static void foldTypedIndex(vector<string>& lines){
    // GLOBAL element sizes: array declarations anywhere (`[static] [const] T NAME[N]`) — file-scope data
    // templates (`static const int matrix[9]`) and local arrays alike — so `matrix[i<<2]` -> `matrix[i]`.
    map<string,int> gElem;
    for(auto& l:lines){ string t=trim(l); size_t br=t.find('[');
        if(br==string::npos) continue; if(t.rfind("static",0)!=0 && !startsWithType(t)) continue;
        long e=(long)br-1; while(e>=0&&t[e]==' ')e--; long b=e; while(b>=0&&isIdent(t[b]))b--; if(e<=b)continue;
        string nm=t.substr(b+1,e-b); int s=scalarSize(t.substr(0,b+1)); if(s>=2) gElem[nm]=s; }
    size_t i=0;
    while(i<lines.size()){ string t=trim(lines[i]);
        bool hdr = !t.empty() && t.back()=='{' && t.find('(')!=string::npos && indentOf(lines[i]).empty()
                   && t.rfind("if",0)!=0 && t.rfind("for",0)!=0 && t.rfind("while",0)!=0 && t.rfind("else",0)!=0
                   && t.rfind("switch",0)!=0 && t.rfind("struct",0)!=0 && t.rfind("class",0)!=0;
        if(!hdr){ i++; continue; }
        map<string,int> elem=gElem;                                    // var name -> element size (>=2); seeded with global arrays
        // params from the header signature
        size_t lp=t.find('('); string params; size_t pe;
        if(balanced(t,lp,params,pe)) for(auto& p:splitArgs(params)){ string pp=trim(p);
            long e=(long)pp.size()-1; while(e>=0&&!isIdent(pp[e]))e--; long b=e; while(b>=0&&isIdent(pp[b]))b--;
            if(e>b){ string nm=pp.substr(b+1,e-b), ty=trim(pp.substr(0,b+1)); int s=ptrElemSize(ty); if(s>=2) elem[nm]=s; } }
        size_t start=i+1, j=start; int depth=1;
        for(; j<lines.size(); j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        size_t end=j;
        // pointer LOCAL declarations inside the body
        for(size_t r=start;r<end;r++){ string d=trim(lines[r]);
            if(d.empty()||d.back()!=';'||d.find('(')!=string::npos||d.find('=')!=string::npos||d.find('*')==string::npos) continue;
            string body=d.substr(0,d.size()-1); long e=(long)body.size()-1; while(e>=0&&isIdent(body[e]))e--;
            if(e<0) continue; string nm=body.substr(e+1), ty=trim(body.substr(0,e+1)); if(!nm.empty()&&isIdent(nm[0])){ int s=ptrElemSize(ty); if(s>=2) elem[nm]=s; } }
        // `// @elem name=size` hints from the lifter (untyped pointers with a known scaled-access element size)
        for(size_t r=start;r<end;r++){ string h=trim(lines[r]); if(h.rfind("// @elem",0)!=0) continue;
            size_t p=8; while(p<h.size()){ while(p<h.size()&&h[p]==' ')p++; size_t eq=h.find('=',p); if(eq==string::npos)break;
                string nm=trim(h.substr(p,eq-p)); size_t q=eq+1,qe=q; while(qe<h.size()&&isdigit((unsigned char)h[qe]))qe++;
                if(qe>q&&!nm.empty()){ int s=atoi(h.substr(q,qe-q).c_str()); if(s>=2) elem[nm]=s; } p=qe; } }
        if(!elem.empty()) for(size_t r=start;r<end;r++){ string& L=lines[r];
            for(auto& kv:elem){ const string& nm=kv.first; int sz=kv.second; string pat=nm+"[";
                size_t p=0; auto m=codeMask(L);
                while((p=L.find(pat,p))!=string::npos){ bool ok=p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1]));
                    if(!ok){ p+=pat.size(); continue; }
                    size_t bs=p+pat.size()-1; int d2=0; size_t q=bs;                  // find matching ]
                    for(; q<L.size(); q++){ char c=L[q]; if(c=='[')d2++; else if(c==']'){ if(--d2==0)break; } }
                    if(q>=L.size()){ p+=pat.size(); continue; }
                    string idx=L.substr(bs+1,q-bs-1); string ne=divideIndex(idx,sz);
                    if(!ne.empty()){ L=L.substr(0,p)+nm+"["+ne+"]"+L.substr(q+1); m=codeMask(L); p+=nm.size()+2+ne.size(); }
                    else p=q+1; } } }
        i=end;
    }
    // drop the consumed `// @elem` hint lines (declareLocals already ran, so we strip them here)
    { vector<string> keep; keep.reserve(lines.size()); for(auto& l:lines){ if(trim(l).rfind("// @elem",0)==0) continue; keep.push_back(l); } lines.swap(keep); }
}
// ── address-of-literal repair (G-fix): collapse's copy-prop/string-inline can fold an rvalue atom
// into an `&<slot>` position, producing `&0`, `&123`, `&0x6e616c63`, `&"str"` — all of which are
// "cannot take the address of an rvalue" compile errors. Repair each before locals are declared.
// reconstruct a packed-ASCII immediate (0x6e616c63 -> "clan") if every LE byte is printable.
static bool packedAsciiToStr(const string& numTok, string& out){
    uint64_t v = strtoull(numTok.c_str(), nullptr, 0);
    if(v==0) return false;
    char buf[9]; int n=0;
    for(int i=0;i<8;i++){ unsigned char c=(v>>(8*i))&0xff; if(c==0) break; if(c<0x20||c>0x7e) return false; buf[n++]=(char)c; }
    if(n<2) return false;                       // 1 printable byte isn't worth a string -> leave as int
    buf[n]=0; out = string("\"")+buf+"\""; return true;
}
// is the `&` at index q a UNARY address-of (not bitwise-AND, not `&&`)?
static bool isUnaryAmp(const string& L, size_t q){
    if(q>=L.size()||L[q]!='&') return false;
    if(q+1<L.size() && L[q+1]=='&') return false;       // `&&`
    if(q>0 && L[q-1]=='&') return false;                // tail of `&&`
    long p=(long)q-1; while(p>=0 && (L[p]==' '||L[p]=='\t')) p--;
    if(p<0) return true;                                // expr/line start -> unary
    char c=L[p];
    if(isIdent(c)||c==')'||c==']') return false;        // `name & lit`, `expr) & lit` -> BINARY and
    return true;                                        // after ( , = ? : { << return etc. -> UNARY
}
static set<long> g_addrSlots;       // ints that became &__addrN, so declareLocals can declare `long __addrN;`
static void fixAddrOfLiteral(vector<string>& lines){
    for(auto& L : lines){
        auto m = codeMask(L);
        for(size_t i=0;i<L.size();){
            if(!(i<m.size() && m[i]) || L[i]!='&' || !isUnaryAmp(L,i)){ i++; continue; }
            size_t a=i+1; while(a<L.size() && (L[a]==' '||L[a]=='\t')) a++;
            if(a>=L.size()){ i++; continue; }
            if(a<m.size() && !m[a] && L[a]=='"'){ L.erase(i, a-i); m=codeMask(L); continue; }   // &"str" -> "str"
            if(a<m.size() && m[a] && isdigit((unsigned char)L[a])){                              // &<num>
                size_t e=a;
                if(L.compare(a,2,"0x")==0||L.compare(a,2,"0X")==0){ e=a+2; while(e<L.size()&&isxdigit((unsigned char)L[e]))e++; }
                else { while(e<L.size()&&isdigit((unsigned char)L[e]))e++; }
                if(e<L.size() && isIdent(L[e])){ i=e; continue; }     // not a bare number -> skip
                string numTok=L.substr(a,e-a), asStr;
                if(packedAsciiToStr(numTok, asStr)){                  // &0xPACKED -> string literal
                    L = L.substr(0,i)+asStr+L.substr(e); m=codeMask(L); continue;
                }
                long iv = atol(numTok.c_str()); g_addrSlots.insert(iv);   // &<int> -> addressable placeholder
                string rep = "&__addr"+std::to_string(iv);
                L = L.substr(0,i)+rep+L.substr(e); m=codeMask(L); i+=rep.size(); continue;
            }
            i++;
        }
    }
}
static void declareLocals(vector<string>& lines){
    set<string> structNames, knownNames;        // knownNames = struct/class/union types + function names (never locals)
    for(auto& l:lines){ string t=trim(l);
        for(const char* kw:{"struct ","class ","union "}) if(t.rfind(kw,0)==0 && t.back()=='{'){ size_t a=strlen(kw),e=a; while(e<t.size()&&isIdent(t[e]))e++; if(e>a){ string nm=t.substr(a,e-a); knownNames.insert(nm); if(string(kw)=="struct ")structNames.insert(nm); } }
        bool hdr=!t.empty()&&t.back()=='{'&&t.find('(')!=string::npos&&indentOf(l).empty()
                 &&t.rfind("if",0)!=0&&t.rfind("for",0)!=0&&t.rfind("while",0)!=0&&t.rfind("else",0)!=0&&t.rfind("switch",0)!=0&&t.rfind("struct",0)!=0&&t.rfind("class",0)!=0;
        if(hdr){ size_t lp=t.find('('); long ne=(long)lp-1; while(ne>=0&&t[ne]==' ')ne--; long b=ne; while(b>=0&&isIdent(t[b]))b--; if(ne>b)knownNames.insert(t.substr(b+1,ne-b)); }
        if(indentOf(l).empty() && startsWithType(t) && t.find('(')==string::npos) collectDeclared(l, knownNames); }   // file-scope globals (data templates, string consts) — never re-declare as a local
    size_t i=0;
    while(i<lines.size()){
        string t=trim(lines[i]);
        // function OR method header (methods are indented inside a class) — process both so method
        // locals get declared and their @widths hint is consumed
        bool hdr = !t.empty()&&t.back()=='{'&&t.find('(')!=string::npos
                   && t.rfind("if",0)!=0&&t.rfind("for",0)!=0&&t.rfind("while",0)!=0&&t.rfind("else",0)!=0
                   && t.rfind("switch",0)!=0&&t.rfind("struct",0)!=0&&t.rfind("class",0)!=0&&t.rfind("do",0)!=0;
        if(!hdr){ i++; continue; }
        size_t lp=t.find('('); long ne0=(long)lp-1; while(ne0>=0&&t[ne0]==' ')ne0--; long nb=ne0; while(nb>=0&&isIdent(t[nb]))nb--;
        string fnname=(ne0>nb)?t.substr(nb+1,ne0-nb):"";
        string paramStr; size_t pe; balanced(t,lp,paramStr,pe);
        set<string> declared; declared.insert("this");
        for(auto& p:splitArgs(paramStr)){ string pp=trim(p); if(pp.empty()||pp=="void")continue;
            long e=(long)pp.size()-1; while(e>=0&&!isIdent(pp[e]))e--; long b=e; while(b>=0&&isIdent(pp[b]))b--; if(e>b)declared.insert(pp.substr(b+1,e-b)); }
        size_t start=i+1, j=start; int depth=1;
        for(; j<lines.size(); j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        size_t end=j;
        for(size_t r=start;r<end;r++){ string b=trim(lines[r]); if(b.size()>1 && b.back()==':' && (isalpha((unsigned char)b[0])||b[0]=='_')){ size_t e=0; while(e<b.size()&&isIdent(b[e]))e++; if(e==b.size()-1) declared.insert(b.substr(0,e)); } }   // a `X:` label is NOT a variable -> never declare it (e.g. the renamed L1/L2 goto targets)
        // consume the lifter's `// @widths name=W ...` hint -> real scalar types (int/long/char/short)
        map<string,int> widthHint;
        for(size_t r=start;r<end;r++){ string ht=trim(lines[r]); if(ht.rfind("// @widths",0)==0){
            size_t p=10; while(p<ht.size()){ while(p<ht.size()&&ht[p]==' ')p++; size_t eq=ht.find('=',p); if(eq==string::npos)break; string nm=ht.substr(p,eq-p);
                size_t v=eq+1,ve=v; while(ve<ht.size()&&isdigit((unsigned char)ht[ve]))ve++; if(ve>v)widthHint[nm]=atoi(ht.substr(v,ve-v).c_str()); p=ve; }
            lines.erase(lines.begin()+r); end--; r--; } }
        // consume `// @data name=TYPE[COUNT] ...` -> declare the memcpy destination as a real typed array
        map<string,std::pair<string,int>> dataArr;
        for(size_t r=start;r<end;r++){ string ht=trim(lines[r]); if(ht.rfind("// @data",0)==0){
            size_t p=8; while(p<ht.size()){ while(p<ht.size()&&ht[p]==' ')p++; size_t eq=ht.find('=',p); if(eq==string::npos)break; string nm=ht.substr(p,eq-p);
                size_t b=ht.find('[',eq), e=ht.find(']',eq); if(b==string::npos||e==string::npos||e<=b)break; string ety=trim(ht.substr(eq+1,b-eq-1)); int cnt=atoi(ht.substr(b+1,e-b-1).c_str());
                if(!nm.empty()&&cnt>0)dataArr[nm]={ety,cnt}; p=e+1; }
            lines.erase(lines.begin()+r); end--; r--; } }
        // consume `// @types name=StructName ...` -> declare that local as `struct StructName*` (the lifter knows
        // the struct of each register/local base; unification renamed structs off the S_fn_base scheme so the
        // name-based fallback below can't find them — this hint is authoritative).
        map<string,string> typeHint;
        for(size_t r=start;r<end;r++){ string ht=trim(lines[r]); if(ht.rfind("// @types",0)==0){
            size_t p=9; while(p<ht.size()){ while(p<ht.size()&&ht[p]==' ')p++; size_t eq=ht.find('=',p); if(eq==string::npos)break; string nm=ht.substr(p,eq-p);
                size_t v=eq+1,ve=v; while(ve<ht.size()&&ht[ve]!=' ')ve++; if(ve>v)typeHint[nm]=ht.substr(v,ve-v); p=ve; }
            lines.erase(lines.begin()+r); end--; r--; } }
        // a typed-array local decays to a pointer: `&v116` -> `v116` (memcpy/calls take the array directly)
        for(auto& kv:dataArr){ string amp="&"+kv.first; for(size_t r=start;r<end;r++){ string& L=lines[r]; auto m=codeMask(L); size_t p=0;
            while((p=L.find(amp,p))!=string::npos){ size_t af=p+amp.size(); bool ok=p<m.size()&&m[p]&&(af>=L.size()||!isIdent(L[af])); if(ok){ L.erase(p,1); m=codeMask(L); } else p+=amp.size(); } } }
        for(size_t r=start;r<end;r++) collectDeclared(lines[r], declared);
        vector<string> order; set<string> seen; map<string,int> needPtr, needArr;
        auto consider=[&](const string& nm,bool arrow,bool index){ if(declared.count(nm)||isBuiltinName(nm)||knownNames.count(nm)||!isDeclarable(nm))return;
            if(!seen.count(nm)){ seen.insert(nm); order.push_back(nm); } if(arrow)needPtr[nm]=1; if(index)needArr[nm]=1; };
        for(size_t r=start;r<end;r++){ const string& L=lines[r]; auto m=codeMask(L);
            for(size_t k=0;k<L.size();k++){ if(!m[k])continue;
                if(!isIdent(L[k])||(k>0&&isIdent(L[k-1]))||isdigit((unsigned char)L[k])) continue;
                // skip member accesses (X.f / X->f) — f is a field, not a variable
                if(k>=1&&L[k-1]=='.'){ continue; }
                if(k>=2&&L[k-1]=='>'&&L[k-2]=='-'){ continue; }
                size_t e=k; while(e<L.size()&&isIdent(L[e]))e++; string nm=L.substr(k,e-k);
                size_t nn=e; while(nn<L.size()&&L[nn]==' ')nn++;
                bool isCall = nn<L.size()&&L[nn]=='(';
                bool assigned=false,arrow=false,index=false;
                if(nn<L.size()&&L[nn]=='='&&(nn+1>=L.size()||L[nn+1]!='=')) assigned=true;
                if(nn+1<L.size()&&strchr("+-*/%&|^",L[nn])&&L[nn+1]=='=') assigned=true;
                if(nn+2<L.size()&&((L[nn]=='<'&&L[nn+1]=='<')||(L[nn]=='>'&&L[nn+1]=='>'))&&L[nn+2]=='=') assigned=true;
                if(nn+1<L.size()&&L[nn]=='-'&&L[nn+1]=='>') arrow=true;
                if(nn<L.size()&&L[nn]=='[') index=true;
                bool addr=false; { long q=(long)k-1; while(q>=0&&L[q]==' ')q--; if(q>=0&&L[q]=='&'&&(q==0||L[q-1]!='&'))addr=true; }
                bool incdec=(nn+1<L.size()&&((L[nn]=='+'&&L[nn+1]=='+')||(L[nn]=='-'&&L[nn+1]=='-'))) ||
                            (k>=2&&((L[k-1]=='+'&&L[k-2]=='+')||(L[k-1]=='-'&&L[k-2]=='-')));
                (void)assigned;(void)addr;(void)incdec;(void)isRegister;
                if(isCall&&!arrow&&!index){ k=e-1; continue; }                    // a call -> callee, excluded anyway
                consider(nm,arrow,index);                                          // ANY use; the exclusion set filters non-locals
                k=e-1;
            } }
        string bi="    "; for(size_t r=start;r<end;r++){ string id=indentOf(lines[r]); if(!trim(lines[r]).empty()){ bi=id; break; } }
        auto cTyOf=[](int w)->string{ return w==1?"char":w==2?"short":w==4?"int":"long"; };
        vector<string> decls;
        for(auto& nm:order){ string ty="long", suf="";
            if(typeHint.count(nm)){ string th=typeHint[nm]; ty = (!th.empty()&&th.back()=='*') ? th : ("struct "+th+"*"); }   // authoritative type from the lifter: `char*`/`int*` used verbatim, a struct name gets `struct …*`
            else if(dataArr.count(nm)){ ty=dataArr[nm].first; suf="["+std::to_string(dataArr[nm].second)+"]"; }   // typed data-array destination
            else if(needPtr.count(nm)){ string found, s1="S_"+fnname+"_"+nm;
                if(structNames.count(s1)) found=s1;
                else for(auto& sn:structNames){ if(sn.size()>nm.size()+1 && sn.compare(sn.size()-nm.size()-1,nm.size()+1,"_"+nm)==0){ if(found.empty())found=sn; else {found="";break;} } }
                ty = !found.empty()? ("struct "+found+"*") : "long*"; }
            else if(needArr.count(nm)) ty="long*";
            else if(widthHint.count(nm)) ty=cTyOf(widthHint[nm]);    // recovered scalar width
            string note = isRegister(nm) ? "   // unresolved register (cross-block dataflow)" : "";
            decls.push_back(bi+ty+" "+nm+suf+";"+note); }
        for(long s : g_addrSlots){ string nm="__addr"+std::to_string(s); bool used=false;   // declare the &__addrN placeholders fixAddrOfLiteral created
            for(size_t r=start;r<end && !used;r++) if(hasTok(lines[r],nm)) used=true;
            if(used && !declared.count(nm)){ declared.insert(nm); decls.push_back(bi+"long "+nm+";"); } }
        if(!decls.empty()){ lines.insert(lines.begin()+start, decls.begin(), decls.end()); end+=decls.size(); }
        i=end+1;
    }
}

// ── forward-declaration pass (G): the decompiler emits functions in address order with
// no prototypes, so any function that calls a later-defined one fails ("use of undeclared
// identifier"). Emit a prototype for every function at the top. Pointers to structs in a
// prototype only need an incomplete type, so this is safe above the struct definitions. ──
// index of the top-level simple-assignment '=' (not ==, <=, +=, &&, etc.), or npos
static size_t dvFindAssign(const string& s){
    int d=0;
    for(size_t i=0;i<s.size();i++){ char c=s[i];
        if(c=='('||c=='['||c=='{')d++; else if(c==')'||c==']'||c=='}'){ if(d)d--; }
        else if(c=='='&&d==0){ char p=i?s[i-1]:0, n=i+1<s.size()?s[i+1]:0;
            if(n=='=') { i++; continue; }
            if(strchr("=!<>+-*/%&|^~",p)) continue;
            return i; } }
    return string::npos;
}
// true if rhs has no function call (no side effects) — a '(' right after an identifier is a call
static bool dvSideEffectFree(const string& rhs){
    for(size_t i=0;i<rhs.size();i++) if(rhs[i]=='('){ size_t j=i; while(j>0&&isspace((unsigned char)rhs[j-1]))j--; if(j>0&&(isalnum((unsigned char)rhs[j-1])||rhs[j-1]=='_')) return false; }
    return true;
}
static bool dvIsSlop(const string& id){ return id.size()>1 && id[0]=='v' && isdigit((unsigned char)id[1]); }   // a `vNNN` stack-slot temp
// a statement that LOOKS like `type ident;` but is really a keyword stmt (`return v148;`, `goto v9;` are NOT decls)
static bool dvKeywordStmt(const string& t){ size_t p=0; while(p<t.size()&&(isalnum((unsigned char)t[p])||t[p]=='_'))p++; string w=t.substr(0,p);
    return w=="return"||w=="goto"||w=="break"||w=="continue"||w=="throw"||w=="delete"||w=="case"||w=="default"||w=="new"||w=="else"||w=="do"||w=="co_return"; }
// whole-word replace of identifier `from` -> `to` in `s`
static void dvWordReplace(string& s,const string& from,const string& to){
    for(size_t p=0;(p=s.find(from,p))!=string::npos;){
        bool lb = p==0 || !(isalnum((unsigned char)s[p-1])||s[p-1]=='_');
        size_t e=p+from.size(); bool rb = e>=s.size() || !(isalnum((unsigned char)s[e])||s[e]=='_');
        if(lb&&rb){ s.replace(p,from.size(),to); p+=to.size(); } else p+=from.size();
    }
}
// net brace delta of a line, IGNORING braces inside "..." / '...' literals and after // comments
static int dvNetBraces(const string& s){
    int d=0; bool inS=false,inC=false;
    for(size_t i=0;i<s.size();i++){ char c=s[i];
        if(inS){ if(c=='\\'){i++;continue;} if(c=='"')inS=false; continue; }
        if(inC){ if(c=='\\'){i++;continue;} if(c=='\'')inC=false; continue; }
        if(c=='"'){inS=true;continue;} if(c=='\''){inC=true;continue;}
        if(c=='/'&&i+1<s.size()&&s[i+1]=='/') break;
        if(c=='{')d++; else if(c=='}')d--; }
    return d;
}
// for each function, find the body range [s+1, e). Returns e (index of the closing `}` at col 0).
static size_t dvFnBody(const vector<string>& L,size_t s){
    int depth=0; size_t e=s;
    for(; e<L.size(); e++){ depth+=dvNetBraces(L[e]); if(depth<=0&&e>s) break; }
    return e;
}
static bool dvIsFnHeader(const string& raw){
    string h=trim(raw); if(h.empty()||h.back()!='{'||h.find('(')==string::npos) return false;
    if(!(isalpha((unsigned char)h[0])||h[0]=='_')) return false;  // a real header starts with a type/name (excludes `}`-leading, `(`-grouping); indented = a class method
    for(const char* kw:{"if","for","while","else","switch","struct","class","do","#","/","enum","union","namespace","extern"}) if(h.rfind(kw,0)==0 && (h.size()==strlen(kw)||!isIdent(h[strlen(kw)]))) return false;
    return true;
}
// X = E;  immediately followed by  return X;   ->   return E;   (the -O0 return-laundering idiom)
static void dvFoldReturnTemp(vector<string>& lines){
    for(size_t i=0;i+1<lines.size();i++){
        string a=trim(lines[i]); size_t eq=dvFindAssign(a);
        if(eq==string::npos||a.empty()||a.back()!=';') continue;
        string lhs=trim(a.substr(0,eq)), rhs=trim(a.substr(eq+1)); rhs.pop_back();   // drop ';'
        if(lhs.find_first_of("*[]().")!=string::npos) continue;                       // must be a bare var
        size_t j=i+1; while(j<lines.size()&&trim(lines[j]).empty())j++;
        if(j>=lines.size()) continue;
        string r=trim(lines[j]);
        if(r=="return "+lhs+";"){ lines[j]=indentOf(lines[j])+"return "+rhs+";"; lines.erase(lines.begin()+i); }
    }
}
// rename each function's returned `vNNN` slot to `ret` — the single most-read value deserves a real name
static void dvNameReturnVars(vector<string>& lines){
    for(size_t s=0;s<lines.size();s++){
        if(!dvIsFnHeader(lines[s])) continue; size_t e=dvFnBody(lines,s);
        string rv; bool clash=false;
        for(size_t i=s+1;i<e;i++){ string t=trim(lines[i]);
            if(t.rfind("return ",0)==0 && t.back()==';'){ string v=trim(t.substr(7)); v.pop_back(); if(dvIsSlop(v)) rv=v; }
            if(t.find("ret")!=string::npos){ for(size_t p=0;(p=lines[i].find("ret",p))!=string::npos;p+=3){ bool lb=p==0||!(isalnum((unsigned char)lines[i][p-1])||lines[i][p-1]=='_'); bool rb=p+3>=lines[i].size()||!(isalnum((unsigned char)lines[i][p+3])||lines[i][p+3]=='_'); if(lb&&rb)clash=true; } }
        }
        if(!rv.empty()&&!clash) for(size_t i=s;i<e;i++) dvWordReplace(lines[i],rv,"ret");
        s=e;
    }
}

// ── offline (NO-AI) behavior-based naming — ported from the GUI's autoNameOffline /
//    autoNameVarsOffline so the CORE decompiler (CLI + TUI) names sub_ functions and
//    placeholder locals from their behavior, not just the GUI. Deterministic. AI stays
//    in the separate -ai build. Disable with EMBER_NONAME=1. ─────────────────────────
static string ccVerbOf(const string& c){
    static const std::map<string,string> V = {
        {"open","openFile"},{"openat","openFile"},{"fopen","openFile"},{"creat","createFile"},{"close","closeFile"},{"fclose","closeFile"},
        {"read","readBytes"},{"fread","readBytes"},{"pread","readBytes"},{"write","writeBytes"},{"fwrite","writeBytes"},{"pwrite","writeBytes"},
        {"malloc","allocate"},{"calloc","allocate"},{"realloc","reallocate"},{"free","release"},
        {"memcpy","copyMemory"},{"memmove","moveMemory"},{"memset","fillMemory"},{"bzero","zeroMemory"},
        {"strlen","stringLength"},{"strcmp","compareStrings"},{"strncmp","compareStrings"},{"strcasecmp","compareStrings"},
        {"strcpy","copyString"},{"strncpy","copyString"},{"strcat","concatStrings"},{"strdup","dupString"},{"strchr","findChar"},{"strstr","findSubstring"},
        {"printf","printText"},{"fprintf","printText"},{"snprintf","formatString"},{"sprintf","formatString"},{"puts","printLine"},{"fputs","printText"},{"perror","printError"},
        {"socket","createSocket"},{"connect","connectSocket"},{"bind","bindSocket"},{"listen","listenSocket"},{"accept","acceptConnection"},{"send","sendData"},{"recv","recvData"},
        {"opendir","openDir"},{"readdir","readDir"},{"closedir","closeDir"},{"stat","statPath"},{"lstat","statPath"},{"mkdir","makeDir"},{"unlink","removeFile"},{"rmdir","removeDir"},{"rename","renamePath"},
        {"getenv","getEnvVar"},{"setenv","setEnvVar"},{"exit","exitProcess"},{"abort","abortProcess"},{"fork","forkProcess"},{"waitpid","waitForChild"},
        {"pthread_create","startThread"},{"pthread_join","joinThread"},{"pthread_mutex_lock","lock"},{"pthread_mutex_unlock","unlock"},
        {"sqrt","computeSqrt"},{"pow","computePow"},{"atoi","parseInt"},{"atol","parseLong"},{"strtol","parseLong"},{"atof","parseDouble"},{"strtod","parseDouble"},
        {"fgets","readLine"},{"getline","readLine"},{"fgetc","readChar"},{"getchar","readChar"},{"fputc","writeChar"},{"putchar","writeChar"},{"fseek","seekFile"},{"ftell","tellFile"},{"rewind","rewindFile"},{"feof","atEndOfFile"},
        {"qsort","sortArray"},{"bsearch","binarySearch"},{"system","runCommand"},{"popen","runCommand"},{"getpid","getProcessId"},{"sleep","sleepSeconds"},{"usleep","sleepMicros"},{"nanosleep","sleepNanos"},
        {"time","getTime"},{"clock","getClock"},{"gettimeofday","getTime"},{"rand","randomValue"},{"srand","seedRandom"},{"random","randomValue"},
        {"tolower","toLower"},{"toupper","toUpper"},{"isdigit","isDigit"},{"isalpha","isAlpha"},{"isspace","isSpace"},
        {"sigaction","installSignalHandler"},{"signal","installSignalHandler"},{"mmap","mapMemory"},{"munmap","unmapMemory"},{"dlopen","loadLibrary"},{"dlsym","resolveSymbol"},
        {"htons","hostToNet"},{"ntohs","netToHost"},{"inet_addr","parseAddress"},{"setsockopt","configureSocket"},{"getaddrinfo","resolveHost"},
    };
    auto it=V.find(c); return it==V.end()?string():it->second;
}
// identifier immediately before the '(' in a function header
static string ccFnHeaderName(const string& header){
    string h=trim(header); size_t par=h.find('('); if(par==string::npos) return "";
    size_t e=par; while(e>0 && h[e-1]==' ') e--; size_t b=e; while(b>0 && (isalnum((unsigned char)h[b-1])||h[b-1]=='_')) b--;
    return h.substr(b,e-b);
}
static string ccGuessName(const vector<string>& body,const string& self){
    std::map<string,int> calls;
    for(auto& l:body) for(size_t i=0;i<l.size();){ if(isalpha((unsigned char)l[i])||l[i]=='_'){ size_t j=i; while(j<l.size()&&(isalnum((unsigned char)l[j])||l[j]=='_'))j++;
        if(j<l.size()&&l[j]=='('){ string w=l.substr(i,j-i); if(w!="if"&&w!="while"&&w!="for"&&w!="switch"&&w!="return"&&w!="sizeof"&&w!=self)calls[w]++; } i=j; } else i++; }
    auto has=[&](const char* n){ return calls.count(n)>0; };
    if((has("open")||has("openat")||has("fopen"))&&(has("read")||has("fread"))) return "readFile";
    if((has("open")||has("openat")||has("fopen"))&&(has("write")||has("fwrite"))) return "writeFile";
    if(has("opendir")&&has("readdir")) return "listDirectory";
    if(has("socket")&&has("connect")) return "openConnection";
    if(has("socket")&&(has("bind")||has("listen"))) return "startServer";
    if(has("malloc")&&has("memcpy")) return "cloneBuffer";
    if(has("qsort")) return "sortArray";
    if(has("bsearch")) return "binarySearch";
    if((has("fopen")||has("open"))&&(has("fgets")||has("getline"))) return "readFileLines";
    if(has("malloc")&&(has("free")||has("realloc"))) return "manageBuffer";
    if(has("system")||has("popen")) return "runCommand";
    if(has("connect")&&(has("send")||has("write"))) return "sendRequest";
    if(has("accept")&&(has("recv")||has("read"))) return "handleClient";
    if(has("pthread_create")) return "startThread";
    { bool cmp=has("strcmp")||has("strncmp")||has("strcasecmp")||has("memcmp");
      for(auto& l:body){ size_t q=l.find('"'); while(q!=string::npos){ size_t e=l.find('"',q+1); if(e==string::npos)break;
        string s; for(size_t k=q+1;k<e;k++) s+=(char)tolower((unsigned char)l[k]);
        if(s.find("usage:")!=string::npos||s.find("usage ")!=string::npos) return "printUsage";
        if(s.find("version")!=string::npos&&s.size()<24) return "printVersion";
        if(cmp&&(s.find("password")!=string::npos||s.find("incorrect")!=string::npos||s.find("correct")!=string::npos)) return "verifyPassword";
        q=l.find('"',e+1); } } }
    if(has("getenv")) return "readEnv";
    if(has("fork")||has("execve")||has("execvp")||has("posix_spawn")) return "spawnProcess";
    if(has("mmap")) return "mapMemory";
    if(has("regcomp")||has("regexec")) return "matchRegex";
    if(has("inet_pton")||has("getaddrinfo")||has("gethostbyname")) return "resolveHost";
    if(has("localtime")||has("strftime")||has("gmtime")) return "formatTime";
    if(has("ioctl")) return "deviceControl";
    if(has("dlopen")||has("dlsym")) return "loadModule";
    if(has("snprintf")||has("sprintf")||has("vsnprintf")) return "formatString";
    if(has("strcmp")||has("strncmp")||has("strcasecmp")) return "compareStrings";
    string best; int bestc=0; for(auto& kv:calls){ string v=ccVerbOf(kv.first); if(!v.empty()&&kv.second>bestc){ best=v; bestc=kv.second; } }
    if(!best.empty()) return best;
    if(calls.size()==1) return calls.begin()->first+"Wrapper";
    return "";
}
static void autoNameFns(vector<string>& lines){ if(getenv("EMBER_NONAME"))return;
    std::set<string> used;
    for(auto& l:lines) for(size_t i=0;i<l.size();){ if(isalpha((unsigned char)l[i])||l[i]=='_'){ size_t j=i; while(j<l.size()&&(isalnum((unsigned char)l[j])||l[j]=='_'))j++; used.insert(l.substr(i,j-i)); i=j; } else i++; }
    std::map<string,string> renames;
    for(size_t s=0;s<lines.size();s++){ if(!dvIsFnHeader(lines[s])){ continue; } size_t e=dvFnBody(lines,s);
        string nm=ccFnHeaderName(lines[s]);
        if(nm.rfind("sub_",0)==0){ vector<string> body(lines.begin()+s,lines.begin()+std::min(e+1,lines.size()));
            string g=ccGuessName(body,nm);
            if(!g.empty()&&g!=nm){ string base=g; int k=2; while(used.count(g))g=base+std::to_string(k++); used.insert(g); renames[nm]=g; } }
        s=e; }
    if(renames.empty())return;
    for(auto& l:lines) for(auto& kv:renames) dvWordReplace(l,kv.first,kv.second);
}
static bool ccVarPlaceholder(const string& t){ size_t i=0; if(t.size()<2)return false;
    if(t[0]=='v'||t[0]=='t')i=1; else if(t.rfind("arg",0)==0)i=3; else return false;
    if(i>=t.size())return false; for(size_t k=i;k<t.size();k++) if(!isdigit((unsigned char)t[k]))return false; return true; }
static string ccGuessVar(const vector<string>& body,const string& v){
    auto idc=[](char c){ return isalnum((unsigned char)c)||c=='_'; };
    bool returned=false,arrow=false,indexed=false,deref=false,setZero=false,inc=false,accum=false;
    for(auto& L:body){ for(size_t p=0;(p=L.find(v,p))!=string::npos;){ size_t e=p+v.size();
            if(!((p==0||!idc(L[p-1]))&&(e>=L.size()||!idc(L[e])))){ p=e; continue; }
            size_t a=p; while(a>0&&L[a-1]==' ')a--; char pre=a>0?L[a-1]:0;
            size_t b=e; while(b<L.size()&&L[b]==' ')b++; char post=b<L.size()?L[b]:0, post2=(b+1<L.size())?L[b+1]:0;
            if(pre=='*')deref=true; if(post=='[')indexed=true; if(post=='-'&&post2=='>')arrow=true;
            if(p>=7&&L.compare(p-7,7,"return ")==0)returned=true;
            if(post=='='&&post2!='='){ size_t r=b+1; while(r<L.size()&&L[r]==' ')r++; string rhs=L.substr(r);
                if(!rhs.empty()&&rhs[0]=='0'&&(rhs.size()==1||!idc(rhs[1])))setZero=true;
                else if(rhs.find(v)!=string::npos){ if(rhs.find(v+" + 1")!=string::npos)inc=true; else accum=true; } }
            p=e; }
        if(L.find(v+"++")!=string::npos)inc=true; }
    if(arrow)return "node"; if(deref)return "ptr"; if(indexed)return "buf";
    if(setZero&&inc)return "i"; if(setZero&&accum)return "sum"; if(returned)return "result";
    return ""; }
static void autoNameVars(vector<string>& lines){ if(getenv("EMBER_NONAME"))return;
    for(size_t s=0;s<lines.size();s++){ if(!dvIsFnHeader(lines[s]))continue; size_t e=dvFnBody(lines,s);
        std::set<string> vars,used;
        for(size_t i=s;i<=e&&i<lines.size();i++) for(size_t p=0;p<lines[i].size();){ if(isalpha((unsigned char)lines[i][p])||lines[i][p]=='_'){ size_t j=p; while(j<lines[i].size()&&(isalnum((unsigned char)lines[i][j])||lines[i][j]=='_'))j++; string t=lines[i].substr(p,j-p); used.insert(t); if(ccVarPlaceholder(t))vars.insert(t); p=j; } else p++; }
        if(vars.empty()){ s=e; continue; }
        vector<string> body(lines.begin()+s,lines.begin()+std::min(e+1,lines.size()));
        std::map<string,string> rn;
        for(auto& v:vars){ string nm=ccGuessVar(body,v); if(nm.empty()||nm==v)continue; string base=nm; int k=2; while(used.count(nm))nm=base+std::to_string(k++); used.insert(nm); rn[v]=nm; }
        if(!rn.empty()) for(size_t i=s;i<=e&&i<lines.size();i++) for(auto& kv:rn) dvWordReplace(lines[i],kv.first,kv.second);
        s=e; }
}
// DEAD-VARIABLE ELIMINATION: a `vNNN` that is only ever ASSIGNED (never read, never address-taken) and whose
// every store RHS is side-effect-free is dead -> drop all its stores + its declaration. Kills the -O0
// pointer-juggling slop (`v104 = &v152; v104 = &v176; ...`) that no later statement ever consumes.
// a TRUE bare declaration `type vN;` -> returns the declared identifier, else "".
// the part before the final identifier must look like a type (only type chars, no operators /
// member-access). this rejects expression statements that `dvFindAssign` can't see — e.g.
// `obj->total += v4;` (a compound store through a pointer) which must NEVER be treated as a
// dead-`v4` declaration and dropped (that silently deletes a real computation).
static string dvBareDeclId(const string& t){
    if(dvKeywordStmt(t) || t.empty() || t.back()!=';' || t.find('(')!=string::npos) return "";
    string d=t.substr(0,t.size()-1); size_t ne=d.size(); while(ne>0&&(isalnum((unsigned char)d[ne-1])||d[ne-1]=='_'))--ne;
    if(ne==0) return "";                                          // no type prefix => not a decl
    string pre=d.substr(0,ne);                                    // the would-be type
    for(char c:pre) if(!(isalnum((unsigned char)c)||strchr("_*&<>:, \t",c))) return "";   // any operator / `->` / `.` / `[` => an expression, not a decl
    if(trim(pre).empty()) return "";
    return d.substr(ne);
}
static void dvDropDeadVars(vector<string>& lines){
    set<size_t> kill;
    for(size_t s=0;s<lines.size();s++){
        if(!dvIsFnHeader(lines[s])) continue; size_t e=dvFnBody(lines,s);
        map<string,int> uses; set<string> addr, defVars;
        for(size_t i=s+1;i<e;i++){ string t=trim(lines[i]); if(t.empty())continue;
            size_t eq=dvFindAssign(t); string dv;
            if(eq==string::npos){                                                     // bare decl `type vN;` -> count nothing
                string bid=dvBareDeclId(t); if(!bid.empty()&&dvIsSlop(bid)) continue;   // a real `type vN;` decl: skip use-counting. an expr stmt (`p->f += vN;`) falls through to the token scan so vN's use IS counted.
            } else { string lhs=trim(t.substr(0,eq));
                if(lhs.find_first_of("*[].")==string::npos&&lhs.find("->")==string::npos){ size_t ne=lhs.size(); while(ne>0&&(isalnum((unsigned char)lhs[ne-1])||lhs[ne-1]=='_'))--ne; string id=lhs.substr(ne); if(dvIsSlop(id))dv=id; } }
            bool skipped=false;
            for(size_t p=0;p<lines[i].size();){ char c=lines[i][p]; if(isalpha((unsigned char)c)||c=='_'){ size_t b=p; while(p<lines[i].size()&&(isalnum((unsigned char)lines[i][p])||lines[i][p]=='_'))p++; string id=lines[i].substr(b,p-b);
                if(dvIsSlop(id)){ size_t q=b; while(q>0&&isspace((unsigned char)lines[i][q-1]))q--; if(q>0&&lines[i][q-1]=='&') addr.insert(id);
                    if(id==dv&&!skipped){ skipped=true; defVars.insert(id); } else uses[id]++; } } else p++; }
        }
        for(size_t i=s+1;i<e;i++){ string t=trim(lines[i]); if(t.empty())continue; size_t eq=dvFindAssign(t);
            if(eq==string::npos){ string id=dvBareDeclId(t); if(!id.empty()&&dvIsSlop(id)&&defVars.count(id)&&uses[id]==0&&!addr.count(id)) kill.insert(i); }   // kill ONLY a genuine dead `type vN;` decl, never an expression statement
            else { string lhs=trim(t.substr(0,eq)),rhs=trim(t.substr(eq+1)); if(lhs.find_first_of("*[].")==string::npos&&lhs.find("->")==string::npos){ size_t ne=lhs.size(); while(ne>0&&(isalnum((unsigned char)lhs[ne-1])||lhs[ne-1]=='_'))--ne; string id=lhs.substr(ne); if(dvIsSlop(id)&&uses[id]==0&&!addr.count(id)&&dvSideEffectFree(rhs)) kill.insert(i); } }
        }
        s=e;
    }
    if(kill.empty()) return;
    vector<string> nb; for(size_t i=0;i<lines.size();i++) if(!kill.count(i)) nb.push_back(lines[i]); lines.swap(nb);
}
// BRACE REPAIR (all-arch, runs last): the lifters' structuring of C++ exception landing pads can emit
// orphan `}` / drop closes, leaving a function brace-unbalanced. Boundaries come from col-0 headers (NOT
// brace depth, which the orphans corrupt). Within each function: drop lone `}` that go below depth 0,
// and append any missing closes before the next function. Makes the output parseable on big EH-heavy files.
static void braceRepair(vector<string>& lines){
    // global pass: drop lone-`}` orphans (closes with no open in scope), append missing closes at EOF.
    set<size_t> drop; long depth=0;
    for(size_t i=0;i<lines.size();i++){ int n=dvNetBraces(lines[i]);
        if(depth+n<0){ if(trim(lines[i])=="}"){ drop.insert(i); continue; } depth=0; continue; }       // orphan close
        depth+=n; }
    long need=depth;                                                                                    // unclosed at EOF
    vector<string> nb; for(size_t i=0;i<lines.size();i++) if(!drop.count(i)) nb.push_back(lines[i]);
    for(long k=0;k<need;k++) nb.push_back("}");
    // HARD GUARANTEE: reconcile any residual count gap left by non-lone orphans (`} while(...)`, `} else {`)
    long o=0,c=0; for(auto& l:nb){ bool s=false,ch=false; for(size_t i=0;i<l.size();i++){ char x=l[i];
        if(s){ if(x=='\\')i++; else if(x=='"')s=false; continue; } if(ch){ if(x=='\\')i++; else if(x=='\'')ch=false; continue; }
        if(x=='"'){s=true;continue;} if(x=='\''){ch=true;continue;} if(x=='/'&&i+1<l.size()&&l[i+1]=='/')break;
        if(x=='{')o++; else if(x=='}')c++; } }
    if(c>o){ long k=c-o; for(size_t i=nb.size(); i-->0 && k>0;) if(trim(nb[i])=="}"){ nb.erase(nb.begin()+i); --k; } }
    else if(o>c) for(long k=0;k<o-c;k++) nb.push_back("}");
    lines.swap(nb);
}
// drop any `type name;` declaration whose name then occurs nowhere else in the function (left dead by the
// dead-store / return-fold passes — e.g. the `int result;` after `result=v148; return result;` collapses)
static void dvDropUnusedDecls(vector<string>& lines){
    set<size_t> kill;
    for(size_t s=0;s<lines.size();s++){ if(!dvIsFnHeader(lines[s]))continue; size_t e=dvFnBody(lines,s);
        for(size_t i=s+1;i<e;i++){ string t=trim(lines[i]);
            if(t.empty()||t.back()!=';'||dvKeywordStmt(t)||dvFindAssign(t)!=string::npos||t.find('(')!=string::npos||t.find(',')!=string::npos) continue;
            string d=t.substr(0,t.size()-1); size_t ne=d.size(); while(ne>0&&(isalnum((unsigned char)d[ne-1])||d[ne-1]=='_'))--ne; string id=d.substr(ne);
            if(id.empty()||ne==0||!(isalpha((unsigned char)id[0])||id[0]=='_')) continue;     // needs a type before the name
            { bool typeish=true; for(size_t k=0;k<ne;k++){ char c=d[k]; if(!(isalnum((unsigned char)c)||c=='_'||c=='*'||c=='&'||c==' '||c==':'||c=='<'||c=='>')){ typeish=false; break; } } if(!typeish) continue; }   // a real decl's prefix is type tokens only; `sum += arr[i].field_0` is a compound-assign, NOT a decl
            int cnt=0; for(size_t k=s+1;k<e;k++){ const string& ln=lines[k]; for(size_t p=0;(p=ln.find(id,p))!=string::npos;){ bool lb=p==0||!(isalnum((unsigned char)ln[p-1])||ln[p-1]=='_'); size_t en=p+id.size(); bool rb=en>=ln.size()||!(isalnum((unsigned char)ln[en])||ln[en]=='_'); if(lb&&rb)cnt++; p=en; } }
            if(cnt<=1) kill.insert(i);
        }
        s=e;
    }
    if(kill.empty())return; vector<string> nb; for(size_t i=0;i<lines.size();i++) if(!kill.count(i))nb.push_back(lines[i]); lines.swap(nb);
}
// drop the empty-body `while (...) {}` loops left behind after destructor stripping. These are libc++
// RAII cleanup iterations (`while (it != &local) {}`) whose bodies were the `~T()` calls pass-0b removed.
// GATE: only loops whose condition takes the address of a stack local (`&v`) — a real spin-wait never does.
static void dropRaiiLoops(vector<string>& lines){
    for(size_t i=0;i+1<lines.size();i++){
        string t=trim(lines[i]);
        if(t.rfind("while (",0)!=0 || t.empty() || t.back()!='{') continue;
        if(t.find("&v")==string::npos && t.find("& v")==string::npos) continue;        // RAII compares vs &<local>
        size_t j=i+1; while(j<lines.size()&&trim(lines[j]).empty())j++;                 // body must be empty
        if(j<lines.size() && trim(lines[j])=="}"){ lines.erase(lines.begin()+i, lines.begin()+j+1); i--; }
    }
}
// split `s` by `delim` at the TOP nesting level (respect <> () [] so template/fn-ptr commas don't split)
static vector<string> splitTopLevel(const string& s, char delim){
    vector<string> out; int d=0; size_t st=0;
    for(size_t i=0;i<s.size();i++){ char c=s[i];
        if(c=='<'||c=='('||c=='[') d++; else if(c=='>'||c==')'||c==']'){ if(d)d--; }
        else if(c==delim && d==0){ out.push_back(s.substr(st,i-st)); st=i+1; } }
    out.push_back(s.substr(st)); return out;
}
// C++ OUTPUT (default): turn `// <demangled C++ sig>` + the following C header into a real C++ header —
// the demangled signature is the source of truth for the qualified name + the actual param TYPES. The
// recovered C-header param names ride along. Drops the comment (kills the "mountain of comments").
// Set EMBER_C=1 to keep the plain-C style (comment + `long name(...)`).
static void cppifyHeaders(vector<string>& lines){
    for(size_t i=0;i+1<lines.size();i++){
        string t=trim(lines[i]);
        if(t.rfind("// ",0)!=0) continue;
        string sig=trim(t.substr(3));
        if(sig.empty()||sig.find('(')==string::npos) continue;
        if(sig.find("'lambda'")!=string::npos||sig.find("$_")!=string::npos||sig.find("__invoke")!=string::npos) continue;  // lambdas: leave alone
        size_t j=i+1; while(j<lines.size()&&trim(lines[j]).empty())j++;
        if(j>=lines.size()) continue;
        string ht=trim(lines[j]);
        if(ht.empty()||ht.back()!='{'||ht.find('(')==string::npos) continue;                         // need a definition header
        for(const char* kw:{"if","for","while","else","switch","struct","class","do"}) if(ht.rfind(kw,0)==0){ ht.clear(); break; }
        if(ht.empty()) continue;
        bool isConst=false; { size_t cp=sig.rfind(" const"); if(cp!=string::npos&&cp+6==sig.size()){ isConst=true; sig=sig.substr(0,cp); } }
        size_t cold=sig.find(" (.cold"); if(cold!=string::npos) sig=sig.substr(0,cold);
        int d=0; size_t lp=string::npos;                                                             // last top-level '(' = param list
        for(size_t k=sig.size();k-->0;){ char c=sig[k]; if(c==')')d++; else if(c=='('){ if(--d==0){ lp=k; break; } } }
        if(lp==string::npos) continue;
        size_t rp=lp; d=0; for(size_t k=lp;k<sig.size();k++){ char c=sig[k]; if(c=='(')d++; else if(c==')'){ if(--d==0){ rp=k; break; } } }
        string qual=trim(sig.substr(0,lp));
        if(qual.empty()||qual.find(' ')!=string::npos) continue;                                      // ret-type leaked / malformed -> skip
        vector<string> sigTypes; for(auto& p:splitTopLevel(sig.substr(lp+1,rp-lp-1),',')){ string tp=trim(p); if(!tp.empty()&&tp!="void") sigTypes.push_back(tp); }
        string ind=indentOf(lines[j]);
        size_t hlp=ht.find('('); string before=trim(ht.substr(0,hlp));                                // "<ret> <name>"
        size_t ne=before.size(); while(ne>0&&isIdent(before[ne-1]))ne--; string ret=trim(before.substr(0,ne)); if(ret.empty())ret="long";
        string hin; size_t he; balanced(ht,hlp,hin,he);
        vector<string> cnames; for(auto& p:splitTopLevel(hin,',')){ string tp=trim(p); if(tp.empty()||tp=="void")continue;
            size_t e=tp.size(); while(e>0&&!isIdent(tp[e-1]))e--; size_t b=e; while(b>0&&isIdent(tp[b-1]))b--; cnames.push_back(tp.substr(b,e-b)); }
        int off = ((int)cnames.size()==(int)sigTypes.size()+1) ? 1 : 0;                               // method receiver as 1st C param -> skip
        bool ctor=false,dtor=false;                                                                   // ctors/dtors take no return type
        { size_t cc=qual.rfind("::"); string last = cc==string::npos? qual : qual.substr(cc+2);
          if(!last.empty()&&last[0]=='~') dtor=true;
          else if(cc!=string::npos){ string rest=qual.substr(0,cc); size_t c2=rest.rfind("::"); string cls=c2==string::npos?rest:rest.substr(c2+2); if(last==cls&&!cls.empty()) ctor=true; } }
        string params; for(size_t k=0;k<sigTypes.size();k++){ if(k)params+=", ";
            string nm = (off+(int)k < (int)cnames.size())? cnames[off+k] : ("p"+std::to_string(k));
            params += sigTypes[k]+" "+nm; }
        // INSIDE a class body (indented header) the name must be UNQUALIFIED — `tick`, not `Counter::tick`
        // (extra-qualification is a hard error). Out-of-class definitions (col 0) keep the `Class::` qualifier.
        string emitName=qual; if(!ind.empty()){ size_t cc=qual.rfind("::"); if(cc!=string::npos) emitName=qual.substr(cc+2); }
        lines[j]=ind + ((ctor||dtor)? "" : ret+" ") + emitName + "(" + params + ")" + (isConst?" const":"") + " {";
        lines.erase(lines.begin()+i);                                                                 // drop the now-redundant comment
    }
}
// legalize residual C++ operator names -> legal C identifiers (runs AFTER folding, so most operator
// CALLS are already infix; what's left is mainly the operator's own DEFINITION/prototype header).
static void legalizeOps(vector<string>& lines){
    static const std::pair<const char*,const char*> M[]={      // LONGEST first so `operator<<` isn't eaten as `operator<`
        {"operator<<=","operator_lshe"},{"operator>>=","operator_rshe"},
        {"operator==","operator_eq"},{"operator!=","operator_ne"},{"operator<=","operator_le"},{"operator>=","operator_ge"},
        {"operator<<","operator_lsh"},{"operator>>","operator_rsh"},{"operator&&","operator_land"},{"operator||","operator_lor"},
        {"operator+=","operator_adde"},{"operator-=","operator_sube"},{"operator*=","operator_mule"},{"operator/=","operator_dive"},
        {"operator%=","operator_mode"},{"operator&=","operator_ande"},{"operator|=","operator_ore"},{"operator^=","operator_xore"},
        {"operator++","operator_inc"},{"operator--","operator_dec"},{"operator->","operator_arrow"},{"operator[]","operator_index"},{"operator()","operator_call"},
        {"operator=","operator_assign"},{"operator<","operator_lt"},{"operator>","operator_gt"},
        {"operator+","operator_add"},{"operator-","operator_sub"},{"operator*","operator_mul"},{"operator/","operator_div"},
        {"operator%","operator_mod"},{"operator&","operator_band"},{"operator|","operator_bor"},{"operator^","operator_xor"},
        {"operator~","operator_bnot"},{"operator!","operator_lnot"}};
    for(auto& l:lines){ if(l.find("operator")==string::npos) continue;
        for(auto& pr:M){ size_t n=strlen(pr.first), p=0; auto m=codeMask(l);
            while((p=l.find(pr.first,p))!=string::npos){ if(p<m.size()&&m[p]&&(p==0||!isIdent(l[p-1]))){ l=l.substr(0,p)+pr.second+l.substr(p+n); m=codeMask(l); p+=strlen(pr.second); } else p+=n; } } }
    // destructor ~Foo used as a function NAME (followed by '(') -> Foo_dtor  (covers x86; arm64 shortName already does it)
    for(auto& l:lines){ if(l.find('~')==string::npos) continue; auto m=codeMask(l);
        for(size_t i=0;i<l.size();){ if(i<m.size()&&m[i]&&l[i]=='~'&&i+1<l.size()&&(isalpha((unsigned char)l[i+1])||l[i+1]=='_')){
            size_t e=i+1; while(e<l.size()&&isIdent(l[e]))e++; size_t j=e; while(j<l.size()&&l[j]==' ')j++;
            if(j<l.size()&&l[j]=='('){ string id=l.substr(i+1,e-i-1); l=l.substr(0,i)+id+"_dtor"+l.substr(e); m=codeMask(l); i+=id.size()+5; continue; } }
            i++; } }
}
static void forwardDecls(vector<string>& lines){
    vector<string> protos; set<string> seen;
    for(auto& l:lines){ string t=trim(l);
        bool hdr = !t.empty()&&t.back()=='{'&&t.find('(')!=string::npos&&indentOf(l).empty()
                   && t.rfind("if",0)!=0&&t.rfind("for",0)!=0&&t.rfind("while",0)!=0&&t.rfind("else",0)!=0
                   && t.rfind("switch",0)!=0&&t.rfind("struct",0)!=0&&t.rfind("class",0)!=0&&t.rfind("union",0)!=0;
        if(!hdr) continue;
        size_t lp=t.find('('); long ne=(long)lp-1; while(ne>=0&&t[ne]==' ')ne--; long nb=ne; while(nb>=0&&isIdent(t[nb]))nb--;
        string nm=(ne>nb)?t.substr(nb+1,ne-nb):"";
        if(nm.empty()||nm=="main"||seen.count(nm)) continue; seen.insert(nm);
        string sig=t; if(!sig.empty()&&sig.back()=='{')sig.pop_back(); protos.push_back(trim(sig)+";");
    }
    // stub called-but-undefined `sub_<addr>` functions (unrelocated .o `bl`/`call` targets resolve to
    // bogus in-function addresses) so the output passes a syntax check instead of "undeclared identifier".
    set<string> calledUndef;
    for(auto& l:lines){ auto m=codeMask(l);
        for(size_t i=0;i<l.size();){ if(i<m.size()&&m[i]&&l[i]=='s'&&l.compare(i,4,"sub_")==0&&(i==0||!isIdent(l[i-1]))){
            size_t e=i+4; while(e<l.size()&&isIdent(l[e]))e++; size_t j=e; while(j<l.size()&&l[j]==' ')j++;
            if(e>i+4 && j<l.size() && l[j]=='('){ string nm=l.substr(i,e-i); if(!seen.count(nm)) calledUndef.insert(nm); }
            i=e; } else i++; } }
    for(auto& nm:calledUndef) protos.push_back("long "+nm+"(...);");   // (...) accepts any arity (unknown signature)
    if(!protos.empty()){ protos.push_back(""); lines.insert(lines.begin(), protos.begin(), protos.end()); }
}

// ── idiomatic STL: the recovered libc++ std::string comes back as `struct String { char* data; long size; char cap; }`
// with raw SSO accessors. Turn it into real `std::string`: the size accessor `(X->cap<0)?X->size:X->cap` -> `X->size()`,
// data -> X->c_str(), retype `struct String*` -> `std::string*`, drop the struct decl. TYPE-GATED to String vars only
// (other structs with a size/data field are untouched), so it never breaks non-string code. Also `size(x)`->`x.size()`. ──
static void foldStdString(vector<string>& lines){
    std::set<string> sTypes;
    for(auto& l:lines){ string t=trim(l); if(t.rfind("struct String",0)==0 && t.find('{')!=string::npos){ size_t a=7,b=a; while(b<t.size()&&isIdent(t[b]))b++; if(b>a)sTypes.insert(t.substr(a,b-a)); } }
    // collect std::string-typed vars: `struct String[*] V`, and real `std::string[ const&*] V`
    std::set<string> svars;
    auto grabAfter=[&](const string& l,size_t q){ while(q<l.size()&&(l[q]=='*'||l[q]=='&'||l[q]==' '))q++; size_t e=q; while(e<l.size()&&isIdent(l[e]))e++; if(e>q&&!isdigit((unsigned char)l[q])){ string v=l.substr(q,e-q); if(v!="const")svars.insert(v); } };
    for(auto& l:lines){ for(auto& T:sTypes){ string pat="struct "+T; size_t p=0; while((p=l.find(pat,p))!=string::npos){ grabAfter(l,p+pat.size()); p+=pat.size(); } }
        size_t p=0; while((p=l.find("std::string",p))!=string::npos){ size_t q=p+11; if(q<l.size()&&isIdent(l[q])){p=q;continue;} // skip std::string_view etc
            while(q<l.size()){ if(l[q]==' '||l[q]=='&'||l[q]=='*'){q++;continue;} if(l.compare(q,5,"const")==0&&(q+5>=l.size()||!isIdent(l[q+5]))){q+=5;continue;} break; }   // skip qualifiers properly
            size_t e=q; while(e<l.size()&&isIdent(l[e]))e++; if(e>q&&!isdigit((unsigned char)l[q]))svars.insert(l.substr(q,e-q)); p=q+1; } }
    if(sTypes.empty()&&svars.empty()) return;
    auto repl=[&](string& s,const string& from,const string& to){ size_t p=0; while((p=s.find(from,p))!=string::npos){ s=s.substr(0,p)+to+s.substr(p+from.size()); p+=to.size(); } };
    auto fieldMethod=[&](string& s,const string& V,const string& f,const string& m){ string from=V+"->"+f; size_t p=0;
        while((p=s.find(from,p))!=string::npos){ size_t af=p+from.size(); bool boundOk=(p==0||!isIdent(s[p-1])); if(!boundOk||(af<s.size()&&(isIdent(s[af])||s[af]=='('))){ p=af; continue; } s.replace(p,from.size(),V+"->"+m); p+=V.size()+2+m.size(); } };
    for(auto& l:lines){
        for(auto& V:svars){
            repl(l,"("+V+"->cap < 0) ? "+V+"->size : "+V+"->cap",V+"->size()");
            repl(l,"("+V+"->cap >= 0) ? "+V+"->size : "+V+"->cap",V+"->size()");
            fieldMethod(l,V,"size","size()"); fieldMethod(l,V,"cap","size()"); fieldMethod(l,V,"data","c_str()");
            // std::string method called free-function style: size(x)/length(x)/c_str(x)/empty(x) -> x.method()
            for(const char* mth:{"size","length","c_str","empty","clear"}){ repl(l,string(mth)+"("+V+")",V+"."+mth+"()"); repl(l,string(mth)+"(&"+V+")",V+"."+mth+"()"); } }
        for(auto& T:sTypes){ repl(l,"struct "+T+"*","std::string*"); repl(l,"struct "+T+" ","std::string "); }
    }
    // drop the `struct String { … };` decl blocks (now std::string)
    vector<string> keep; for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]); bool drop=false;
        for(auto& T:sTypes) if(t=="struct "+T+" {"){ int d=0; size_t j=i; for(;j<lines.size();j++){ for(char c:lines[j]){ if(c=='{')d++; else if(c=='}')d--; } if(d<=0)break; } i=j; drop=true; break; }
        if(!drop) keep.push_back(lines[i]); }
    lines.swap(keep);
}

// ── C++ RAII / EH CLEANUP ELISION (Hex-Rays/Ghidra style) ────────────────────
// In C++ output, destructors + operator_delete + _Unwind_Resume landing pads are IMPLICIT — the
// compiler's per-scope-exit cleanup chains and exception unwinding are pure boilerplate the reader
// never wants. ~3,700 of vortex's gotos jump into this machinery. We find labels whose block is
// cleanup-ONLY and TERMINAL (never falls into user code), drop those blocks, and remove every goto
// to them. Strictly bounded — a block with ANY user statement (cout, a real call, a field store,
// a valued return) is never classified cleanup, so program logic is untouched.
static bool isLabelLine(const string& t, string& lbl);   // (defined in SMART LOOP RECOVERY below)
static bool isCleanupStmt(const string& t){
    if(t.empty()||t=="}"||(t.size()>1&&t.back()==':')) return true;                 // blank / close / label = neutral
    static const char* K[]={"operator_delete","_Unwind_Resume","__clang_call_terminate","__cxa_","std::terminate"};
    for(auto k:K) if(t.find(k)!=string::npos) return true;
    if(t.find("_dtor(")!=string::npos||t.find("::~")!=string::npos) return true;     // explicit destructor call
    return false;
}
// cleanup-neutral: a cleanup stmt, OR EH register-juggling — a plain temp assignment `vN = <no-call expr>;`
// (e.g. `v8 = a;`, `v104 = (v64 + 24);`). NOT a memory/field store and NOT `vN = call()` (those carry user
// effects), so a block of only-these + a real cleanup keyword is safe to drop.
static bool cleanupNeutral(const string& t){
    if(isCleanupStmt(t)) return true;
    size_t eq=t.find(" = "); if(eq==string::npos||t.empty()||t[0]!='v') return false;
    for(size_t k=1;k<eq;k++) if(!isdigit((unsigned char)t[k])) return false;        // LHS must be a bare vNNN temp (no ->, *, [, .)
    string rhs=t.substr(eq+3);
    for(size_t k=0;k<rhs.size();k++) if(rhs[k]=='('&&k>0&&isIdent(rhs[k-1])) return false;   // no function call on the RHS
    return true;
}
static bool isIdentName(const string& s){ if(s.empty()||isdigit((unsigned char)s[0]))return false; for(char c:s) if(!isIdent(c))return false; return true; }
// the goto target on a line (if it's a pure `goto L;` or `if (c) goto L;`), else ""
static string soleGotoTarget(const string& t){
    if(t.rfind("goto ",0)==0 && t.back()==';'){ string l=t.substr(5); l.pop_back(); return isIdentName(l)?l:""; }
    size_t gp=t.rfind(") goto ");
    if(t.rfind("if (",0)==0 && gp!=string::npos && t.back()==';'){ string l=t.substr(gp+7); l.pop_back(); return isIdentName(l)?l:""; }
    return "";
}
// ── SWITCH RECOVERY ── clang -O0 lowers `switch(V){ case C0:…; case Ck:…; default:…; }` to a comparison
// chain nested in an if/else:  `if (!(V == C0)) { if (V==C1) goto L1; … } else { <body C0> goto J;  L1: <body C1> goto J; … }  <default> J:`
// Recognize that exact shape (pure `== const` dispatch only) and rebuild a real switch. Labels are resolved
// LOCALLY inside the else block, so reuse of L1/L2 across functions is irrelevant. Strict: bail on anything odd.
static int braceDelta(const string& l){ auto m=codeMask(l); int d=0; for(size_t i=0;i<l.size();i++){ if(i<m.size()&&m[i]){ if(l[i]=='{')d++; else if(l[i]=='}')d--; } } return d; }
static bool parseEqGoto(const string& t,string& V,string& C,string& lab){   // "if (V == C) goto LAB;"  (C an integer literal)
    if(t.rfind("if (",0)!=0||t.empty()||t.back()!=';') return false; size_t gp=t.find(") goto "); if(gp==string::npos) return false;
    string cond=t.substr(4,gp-4); lab=t.substr(gp+7); if(lab.empty()||lab.back()!=';')return false; lab.pop_back(); if(!isIdentName(lab))return false;
    size_t eq=cond.find(" == "); if(eq==string::npos)return false; V=trim(cond.substr(0,eq)); C=trim(cond.substr(eq+4));
    if(V.empty()||C.empty())return false; bool num=true; size_t s=(C[0]=='-')?1:0; if(s>=C.size())return false;
    if(C.compare(0,2,"0x")==0||C.compare(0,3,"-0x")==0){ for(size_t i=(C[0]=='-')?3:2;i<C.size();i++) if(!isxdigit((unsigned char)C[i]))num=false; }
    else for(size_t i=s;i<C.size();i++) if(!isdigit((unsigned char)C[i]))num=false;
    for(char c:V) if(!isIdent(c)) num=false;   // V must be a bare identifier (no side effects)
    return num;
}
static bool isIntLit(const string& C){ if(C.empty())return false; size_t s=(C[0]=='-')?1:0; if(s>=C.size())return false;
    if(C.compare(s,2,"0x")==0){ for(size_t i=s+2;i<C.size();i++) if(!isxdigit((unsigned char)C[i]))return false; return C.size()>s+2; }
    for(size_t i=s;i<C.size();i++) if(!isdigit((unsigned char)C[i]))return false; return true; }
static void recoverSwitch(vector<string>& lines){
  for(size_t h=0; h+2<lines.size(); h++){
    string t=trim(lines[h]); string ind=indentOf(lines[h]);
    if(t.size()<8 || t.rfind("if (",0)!=0 || t.back()!='{') continue;
    // switch head: `if (!(Vh == C0)) {`  OR  `if (Vh != C0) {`  (C0 = the case whose body is the else block)
    string Vh,C0;
    if(t.rfind("if (!(",0)==0){ size_t cl=t.rfind(")) {"); if(cl==string::npos||cl<6) continue; string in=t.substr(6,cl-6); size_t eq=in.find(" == "); if(eq==string::npos) continue; Vh=trim(in.substr(0,eq)); C0=trim(in.substr(eq+4)); }
    else { size_t cl=t.rfind(") {"); if(cl==string::npos||cl<4) continue; string in=t.substr(4,cl-4); size_t ne=in.find(" != "); if(ne==string::npos) continue; Vh=trim(in.substr(0,ne)); C0=trim(in.substr(ne+4)); }
    if(!isIntLit(C0)){ continue; } { bool ok=!Vh.empty(); for(char c:Vh) if(!isIdent(c)) ok=false; if(!ok) continue; }
    // walk to the `} else {` that closes the if at the head's level
    int depth=1; size_t ifEnd=0;
    for(size_t j=h+1;j<lines.size();j++){ string tj=trim(lines[j]);
        if(depth==1 && tj=="} else {"){ ifEnd=j; break; } depth+=braceDelta(lines[j]); if(depth<=0) break; }
    if(!ifEnd) continue;
    // the if-block [h+1, ifEnd) must be PURELY `if (Vd == Ck) goto Lk;` (one dispatch var Vd)
    vector<std::pair<string,string>> disp;  // (const, label)  for the non-C0 cases
    string V; bool clean=true; for(size_t j=h+1;j<ifEnd;j++){ string V2,C,L; if(!parseEqGoto(trim(lines[j]),V2,C,L)){clean=false;break;} if(V.empty())V=V2; else if(V2!=V){clean=false;break;} disp.push_back({C,L}); }
    if(!clean||disp.size()<2||V.empty()) continue;
    // Vd (dispatch) must equal Vh, OR be a copy of it (`Vd = Vh;` / `Vh = Vd;` shortly before the head)
    if(V!=Vh){ bool copy=false; for(size_t j=(h>6?h-6:0);j<h;j++){ string tj=trim(lines[j]); if(tj==V+" = "+Vh+";"||tj==Vh+" = "+V+";"){copy=true;break;} } if(!copy) continue; }
    // else block [ifEnd+1, elseEnd)
    depth=1; size_t elseEnd=0; for(size_t j=ifEnd+1;j<lines.size();j++){ depth+=braceDelta(lines[j]); if(depth<=0){ elseEnd=j; break; } }
    if(!elseEnd) continue;
    // segment the else block by depth-0 dispatch labels Lk. Each segment is one case body. Cases may
    // terminate with `goto J;` (break to a shared join) OR with `return …;` (early-return cases — no join).
    string J; std::map<string,string> labConst; for(auto& d:disp) labConst[d.second]=d.first;
    vector<std::pair<size_t,string>> seg; seg.push_back({ifEnd+1,""});               // (startLine, const)  first = C0
    { int d2=0; for(size_t j=ifEnd+1;j<elseEnd;j++){ string lb; if(d2==0 && isLabelLine(trim(lines[j]),lb) && labConst.count(lb)) seg.push_back({j,labConst[lb]}); d2+=braceDelta(lines[j]); } }
    if(seg.size()<2) continue;
    vector<std::pair<string,vector<string>>> cases; bool fail=false;
    for(size_t s=0;s<seg.size();s++){ size_t a=seg[s].first, b=(s+1<seg.size())?seg[s+1].first:elseEnd;
        vector<string> body; int d2=0;
        for(size_t j=a;j<b;j++){ string tj=trim(lines[j]); string lb;
            if(j==a && isLabelLine(tj,lb)){ continue; }                              // drop the case's own label line
            if(d2==0 && tj.rfind("goto ",0)==0 && tj.back()==';'){ string g=soleGotoTarget(tj); if(!g.empty()){ if(J.empty())J=g; else if(g!=J)fail=true; d2+=braceDelta(lines[j]); continue; } }   // a break-to-join goto
            d2+=braceDelta(lines[j]); body.push_back(lines[j]); }
        cases.push_back({seg[s].second, body}); }
    if(fail) continue;
    cases[0].first=C0; for(size_t k=1;k<cases.size();k++) if(cases[k].first.empty()){fail=true;break;}
    if(fail||cases.size()<2) continue;
    // default body: with a join J -> [elseEnd+1, `J:`); without (return-cases) -> code right after the else
    // block until a label or scope close. joinLine marks the end of the region the switch replaces.
    size_t joinLine; vector<string> defBody;
    if(!J.empty()){ joinLine=0; for(size_t j=elseEnd+1;j<lines.size()&&j<elseEnd+200;j++){ string lb; if(isLabelLine(trim(lines[j]),lb)&&lb==J){joinLine=j;break;} if(braceDelta(lines[j])<0)break; }
        if(!joinLine) continue; for(size_t j=elseEnd+1;j<joinLine;j++) defBody.push_back(lines[j]); }
    else { size_t j=elseEnd+1; for(; j<lines.size(); j++){ string lb; if(isLabelLine(trim(lines[j]),lb))break; if(braceDelta(lines[j])<0)break; defBody.push_back(lines[j]); } joinLine=j; }
    // ── build the switch ──
    auto reindent=[&](vector<string>& body,const string& base)->vector<string>{
        int mn=1<<30; for(auto& l:body){ if(trim(l).empty())continue; int n=(int)indentOf(l).size(); if(n<mn)mn=n; } if(mn==(1<<30))mn=0;
        vector<string> o; for(auto& l:body){ if(trim(l).empty()){o.push_back("");continue;} string s=l.substr(std::min((size_t)mn,indentOf(l).size())); o.push_back(base+s); } return o; };
    auto endsControl=[&](vector<string>& body){ for(size_t i=body.size();i-->0;){ string t=trim(body[i]); if(t.empty()||t=="}")continue; return t.rfind("return",0)==0||t.rfind("break",0)==0||t.rfind("goto ",0)==0||t.rfind("continue",0)==0; } return false; };  // no break needed if the case already returns/breaks
    vector<string> sw; sw.push_back(ind+"switch ("+V+") {");
    for(auto& c:cases){ sw.push_back(ind+"case "+c.first+": {");
        for(auto& l:reindent(const_cast<vector<string>&>(c.second), ind+"    ")) sw.push_back(l);
        if(!endsControl(c.second)) sw.push_back(ind+"    break;"); sw.push_back(ind+"}"); }
    sw.push_back(ind+"default: {");
    for(auto& l:reindent(defBody, ind+"    ")) sw.push_back(l);
    if(!endsControl(defBody)) sw.push_back(ind+"    break;"); sw.push_back(ind+"}");
    sw.push_back(ind+"}");
    // splice: replace [h, joinLine) with sw (keep J: line at joinLine — dropOrphanLabels removes it if now unused)
    vector<string> out; out.reserve(lines.size());
    for(size_t j=0;j<h;j++) out.push_back(lines[j]);
    for(auto& l:sw) out.push_back(l);
    for(size_t j=joinLine;j<lines.size();j++) out.push_back(lines[j]);
    lines.swap(out);
    h += sw.size()-1;   // -1: the for-loop's ++ lands us on the first line after the spliced switch
  }
}
static void elideCleanup(vector<string>& lines){
    // index labels -> line; and the set of label names
    std::map<string,size_t> labAt;
    for(size_t i=0;i<lines.size();i++){ string lb; if(isLabelLine(trim(lines[i]),lb)) labAt[lb]=i; }
    if(labAt.empty()) return;
    // block of a label = [labelLine+1, next label line). Classify clean = every stmt is cleanup OR a
    // goto to a clean label, AND the block is TERMINAL (ends in return/unwind/`goto clean`, never falls
    // into user code). Fixpoint because cleanup blocks chain into each other.
    auto nextLabelLine=[&](size_t from)->size_t{ for(size_t r=from;r<lines.size();r++){ string lb; if(isLabelLine(trim(lines[r]),lb)) return r; } return lines.size(); };
    std::map<string,size_t> blockEnd; for(auto& kv:labAt) blockEnd[kv.first]=nextLabelLine(kv.second+1);
    std::map<string,bool> clean; for(auto& kv:labAt) clean[kv.first]=true;
    for(bool more=true; more; ){ more=false;
      for(auto& kv:labAt){ if(!clean[kv.first]) continue;
        size_t s=kv.second+1, e=blockEnd[kv.first]; bool terminal=false, ok=true;
        for(size_t r=s;r<e;r++){ string t=trim(lines[r]); if(t.empty()||t=="}") continue;
            if(t.rfind("return",0)==0){ ok=false; break; }       // a RETURN (esp. valued) is NEVER cleanup — dropping it would lose the function's result
            string g=soleGotoTarget(t);
            if(!g.empty()){ if(!clean.count(g)||!clean[g]){ ok=false; break; } if(t.rfind("goto ",0)==0) terminal=true; continue; }   // goto to clean = ok; unconditional goto-clean terminates
            if(t.find("_Unwind_Resume")!=string::npos||t.find("__clang_call_terminate")!=string::npos||t.find("std::terminate")!=string::npos) terminal=true;   // ONLY exception-unwind/terminate ends a clean chain
            if(!cleanupNeutral(t)){ ok=false; break; } }
        // also require the block is not entered by fall-through from USER code: the line before its label
        // must be a terminator (goto/return/unwind/`}`), so dropping the block + its gotos removes every path in.
        bool gotoEntered=true; { size_t p=kv.second; while(p>0){ string pt=trim(lines[p-1]); if(pt.empty()){p--;continue;}
            gotoEntered = pt.back()=='}' || pt.rfind("goto ",0)==0 || pt.rfind("return",0)==0 || pt.find("_Unwind_Resume")!=string::npos
                       || soleGotoTarget(pt).size() || cleanupNeutral(pt); break; } }   // terminator OR a clean predecessor (fall-through within a cleanup chain)
        if(ok&&terminal&&gotoEntered){} else if(clean[kv.first]){ clean[kv.first]=false; more=true; } }
    }
    // collect clean labels that are genuinely cleanup (and at least one cleanup stmt -> not an empty user label)
    set<string> drop; for(auto& kv:labAt){ if(!clean[kv.first]) continue;
        bool hasCleanup=false; for(size_t r=kv.second+1;r<blockEnd[kv.first];r++){ string t=trim(lines[r]);
            if(t.find("operator_delete")!=string::npos||t.find("_Unwind_Resume")!=string::npos||t.find("_dtor(")!=string::npos||t.find("::~")!=string::npos||t.find("terminate")!=string::npos){ hasCleanup=true; break; } }
        if(hasCleanup) drop.insert(kv.first); }
    if(drop.empty()) return;
    // remove the clean blocks (label..end) and every goto targeting them
    set<size_t> kill;
    for(const auto& lb:drop){ for(size_t r=labAt[lb]; r<blockEnd[lb]; r++) kill.insert(r); }
    vector<string> out; out.reserve(lines.size());
    for(size_t i=0;i<lines.size();i++){ if(kill.count(i)) continue; string t=trim(lines[i]);
        string g=soleGotoTarget(t); if(!g.empty()&&drop.count(g)) continue;        // goto/if-goto into dropped cleanup -> remove
        out.push_back(lines[i]); }
    lines.swap(out);
}

// LATCH INCREMENT double-count: `do { …; i++; } while ((i + 1) OP C);` — clang -O0 compares the POST-increment
// value (`add i,1; cmp i,C`), but the lifter materialized `i++` AND kept the pre-increment symbolic `(i+1)` in the
// condition, so the recovered loop runs one short. When the body increments `i`, rewrite `(i + 1)` -> `i` in the
// while-condition. Conservative: only the var actually incremented in the body.
static void fixLatchIncrement(vector<string>& lines){
    for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]);
        if(t.rfind("} while (",0)!=0 || t.size()<12 || t.back()!=';') continue;
        size_t e=t.rfind(");"); if(e==string::npos||e<=9) continue; string cond=t.substr(9,e-9);
        if(cond.find("+ 1)")==string::npos) continue;
        int d=0; size_t doLine=string::npos;                                  // find the matching `do {` by brace balance
        for(size_t r=i+1; r-->0; ){ d += -braceDelta(lines[r]); if(d==0){ doLine=r; break; } }
        if(doLine==string::npos || trim(lines[doLine])!="do {") continue;
        bool changed=false;
        for(size_t r=doLine+1;r<i;r++){ string b=trim(lines[r]); string v;
            if(b.size()>3 && b.compare(b.size()-3,3,"++;")==0) v=b.substr(0,b.size()-3);
            else { size_t eq=b.find(" = ("); if(eq!=string::npos){ string lhs=b.substr(0,eq); if(b==lhs+" = ("+lhs+" + 1);") v=lhs; } }
            if(v.empty()||!isIdentName(v)) continue;
            string pat="("+v+" + 1)"; size_t p=0; while((p=cond.find(pat,p))!=string::npos){ cond.replace(p,pat.size(),v); changed=true; p+=v.size(); } }
        if(changed) lines[i]=indentOf(lines[i])+"} while ("+cond+");";
    }
}
// ── SMART LOOP RECOVERY ──────────────────────────────────────────────────────
// fan-in (how many gotos target each label) over the whole buffer, literal-aware.
static std::map<string,int> labelFanIn(const vector<string>& lines){
    std::map<string,int> f; for(auto& l:lines){ set<string> tg; collectGotoTargets(l,tg); for(auto& t:tg) f[t]++; }
    return f;
}
static bool isLabelLine(const string& t, string& lbl){
    if(t.size()<2||t.back()!=':') return false; size_t e=0; while(e<t.size()&&isIdent(t[e]))e++;
    if(e!=t.size()-1) return false; lbl=t.substr(0,e); return lbl.rfind("loc_",0)==0 || (lbl.size()>1&&lbl[0]=='L'&&isdigit((unsigned char)lbl[1]));
}
// GUARD-CLAUSE FLATTENING: clang -O0 lowers `if (!(A && B && C)) <body>` to a nested single-if chain that
// jumps PAST the body:  `if (A) { if (B) { if (C) goto L; } } <body> L:`  -> `if (!((A) && (B) && (C))) { <body> }`.
// Only fires on a strictly-nested chain (each if's block holds exactly the next if, nothing else), the terminal
// `if (Z) goto L;`, then exactly N closing braces, then a body, then `L:` reached ONLY by that goto (fan-in 1).
static void foldGuardChain(vector<string>& lines){
    auto condOfOpen=[](const string& t,string& c)->bool{ if(t.rfind("if (",0)!=0||t.empty()||t.back()!='{')return false; size_t cl=t.rfind(") {"); if(cl==string::npos||cl<4)return false; c=t.substr(4,cl-4); return !c.empty(); };
    for(size_t h=0; h<lines.size(); h++){
        string ind=indentOf(lines[h]); string c0; if(!condOfOpen(trim(lines[h]),c0)) continue;
        vector<string> conds; size_t j=h; string termCond, lab;
        bool ok=true; while(j<lines.size()){ string t=trim(lines[j]); string c;
            if(condOfOpen(t,c)){ conds.push_back(c); j++; continue; }
            string V,C,L; if(parseEqGoto(t,V,C,L) || (t.rfind("if (",0)==0 && t.find(") goto ")!=string::npos && t.back()==';')){
                size_t gp=t.find(") goto "); termCond=t.substr(4,gp-4); lab=t.substr(gp+7); if(!lab.empty()&&lab.back()==';')lab.pop_back(); if(!isIdentName(lab)){ok=false;} j++; break; }
            ok=false; break; }
        if(!ok||conds.empty()||termCond.empty()||lab.empty()) continue;     // need >=1 wrapping if (>=2 conditions total)
        size_t need=conds.size(), k=j; while(need>0 && k<lines.size() && trim(lines[k])=="}"){ k++; need--; }
        if(need>0) continue;                                                // not a clean nest of closing braces
        // body = [k, Lline) where Lline is `lab:` at the head indent; must be the ONLY jump to lab
        size_t Lline=0; for(size_t r=k;r<lines.size();r++){ string lb; if(isLabelLine(trim(lines[r]),lb)&&lb==lab){ Lline=r; break; } if(braceDelta(lines[r])<0&&indentOf(lines[r]).size()<ind.size()) break; }
        if(!Lline||Lline<=k) continue;
        // fan-in must be 1 WITHIN this function (labels like L1 are reused across functions, so a global count lies)
        size_t fs=h; while(fs>0){ const string& p=lines[fs]; if(indentOf(p).empty()&&!trim(p).empty()&&p.back()=='{')break; fs--; }
        size_t fe=Lline; for(size_t r=Lline;r<lines.size();r++){ if(indentOf(lines[r]).empty()&&trim(lines[r])=="}"){ fe=r; break; } }
        int loc=0; for(size_t r=fs;r<fe;r++){ set<string> tg; collectGotoTargets(lines[r],tg); if(tg.count(lab))loc++; } if(loc!=1) continue;
        bool bodyBad=false; for(size_t r=k;r<Lline;r++){ set<string> tg; collectGotoTargets(lines[r],tg); if(tg.count(lab)){bodyBad=true;break;} } if(bodyBad) continue;
        // build  if (!((A) && (B) && (Z))) { <body> }
        string conj; for(auto& c:conds){ if(!conj.empty())conj+=" && "; conj+="("+c+")"; } conj+=" && ("+termCond+")";
        vector<string> nb; nb.push_back(ind+"if (!("+conj+")) {");
        for(size_t r=k;r<Lline;r++){ if(trim(lines[r]).empty()){nb.push_back("");continue;} nb.push_back(ind+"    "+trim(lines[r])); }   // simple bodies (guard returns); flat reindent
        nb.push_back(ind+"}");
        vector<string> out; for(size_t r=0;r<h;r++) out.push_back(lines[r]);
        for(auto& l:nb) out.push_back(l);
        for(size_t r=Lline+1;r<lines.size();r++) out.push_back(lines[r]);   // drop the now-orphan `lab:`
        lines.swap(out); h += nb.size()-1;   // -1: the for-loop's ++ lands us on the first line AFTER the spliced block
    }
}
// re-indent lines [a,b) by +4 spaces (wrapping a body one level deeper).
static void indentRange(vector<string>& lines,size_t a,size_t b){ for(size_t r=a;r<b&&r<lines.size();r++) lines[r]="    "+lines[r]; }
// SINGLE-ENTRY guard: a region [start,end) is only safe to wrap in a loop/block if every label
// DEFINED inside it is reached ONLY from inside it. If an outside goto jumps into the region (an
// exception-landing pad, a shared cleanup), wrapping it would absorb unrelated code into a fake
// loop — semantically legal C but grossly misleading. This is the crux of real structuring.
static bool singleEntry(const vector<string>& lines,size_t start,size_t end){
    set<string> inner; for(size_t r=start;r<end&&r<lines.size();r++){ string lbl; if(isLabelLine(trim(lines[r]),lbl)) inner.insert(lbl); }
    if(inner.empty()) return true;
    for(size_t r=0;r<lines.size();r++){ if(r>=start&&r<end) continue; set<string> tg; collectGotoTargets(lines[r],tg);
        for(auto& t:tg) if(inner.count(t)) return false; }   // an outside goto targets an inner label -> multi-entry, refuse
    return true;
}

// do-while: `L:  <body, brace-balanced>  if (c) goto L;`  (L reached ONLY by this back-edge)
//   -> `do { <body> } while (c);`   (and `goto L;` -> `} while (1);` = infinite loop with inner breaks)
static void foldDoWhile(vector<string>& lines){
    auto fan=labelFanIn(lines);
    for(bool more=true; more; ){ more=false;
      for(size_t i=0;i<lines.size();i++){ string lbl,t=trim(lines[i]);
        if(!isLabelLine(t,lbl)||fan[lbl]!=1) continue;
        int depth=0; size_t back=string::npos; string cond; bool ok=true;
        for(size_t j=i+1;j<lines.size()&&ok;j++){ string tj=trim(lines[j]);
            if(depth==0){                                                                  // at the loop's own level -> is this the back-edge?
                if(tj=="goto "+lbl+";"){ back=j; cond="1"; break; }
                size_t gp=tj.rfind(") goto "+lbl+";");
                if(tj.rfind("if (",0)==0 && gp!=string::npos){ back=j; cond=tj.substr(4,gp-4); break; } }
            for(char c:lines[j]){ if(c=='{')depth++; else if(c=='}'){ if(--depth<0){ok=false;break;} } }
            if(j-i>1500) ok=false; }
        if(back==string::npos||!ok||back<=i) continue;
        if(!singleEntry(lines,i,back+1)) continue;                 // don't absorb EH pads / shared cleanup into a fake loop
        string ind=indentOf(lines[i]);
        indentRange(lines,i+1,back);
        lines[i]=ind+"do {";
        lines[back]=ind+"} while ("+cond+");";
        fan[lbl]=0; more=true; }
    }
}
// while (test at top): `L:  if (c) goto E;  <body>  goto L;  E:`  (L,E each fan-in 1)
//   -> `while (!(c)) { <body> }`   (loop continues while the exit test is false)
static void foldWhile(vector<string>& lines){
    auto fan=labelFanIn(lines);
    for(bool more=true; more; ){ more=false;
      for(size_t i=0;i+1<lines.size();i++){ string lbl,t=trim(lines[i]);
        if(!isLabelLine(t,lbl)||fan[lbl]!=1) continue;
        // first real line after the header label must be the exit test `if (c) goto E;`
        size_t h=i+1; while(h<lines.size()&&trim(lines[h]).empty())h++;
        string th=h<lines.size()?trim(lines[h]):"";
        size_t gp=th.rfind(") goto "); if(th.rfind("if (",0)!=0||gp==string::npos) continue;
        string exitLbl=trim(th.substr(gp+7)); if(!exitLbl.empty()&&exitLbl.back()==';')exitLbl.pop_back();
        string cond=th.substr(4,gp-4);
        // walk the body to the unconditional back-edge `goto L;` at depth 0, then `E:` must immediately follow
        int depth=0; size_t back=string::npos; bool ok=true;
        for(size_t j=h+1;j<lines.size()&&ok;j++){ string tj=trim(lines[j]);
            if(depth==0 && tj=="goto "+lbl+";"){ back=j; break; }
            for(char c:lines[j]){ if(c=='{')depth++; else if(c=='}'){ if(--depth<0){ok=false;break;} } }
            if(j-h>1500) ok=false; }
        if(back==string::npos||!ok) continue;
        size_t en=back+1; while(en<lines.size()&&trim(lines[en]).empty())en++;
        string lblE; if(en>=lines.size()||!isLabelLine(trim(lines[en]),lblE)||lblE!=exitLbl) continue;   // exit label must follow the back-edge
        if(!singleEntry(lines,i,back+1)) continue;                 // body must not be jumped INTO from outside (EH pad / shared cleanup)
        string ind=indentOf(lines[i]);
        // emit: `while (!(c)) {` ... body ... `}`  (drop header label, exit test, back-edge; keep E only if others target it)
        indentRange(lines,h+1,back);
        lines[i]=ind+"while (!("+cond+")) {";
        lines[h]="";                                   // drop the exit test (now the while condition)
        lines[back]=ind+"}";                           // back-edge -> close brace
        if(fan[exitLbl]<=1) lines[en]="";              // drop the now-unused exit label
        fan[lbl]=0; if(fan.count(exitLbl)) fan[exitLbl]--; more=true; }
    }
}
// THE dominant RAII idiom: `if (sso-check) goto L; operator_delete(...); [more cleanup] L:` — the compiler's
// "free the buffer if it was heap-allocated" sequence. In C++ this destructor is IMPLICIT, so the whole
// idiom (the SSO-check goto + the cleanup body + the now-orphan label) is excised. Bounded hard: L must be
// fan-in 1 (only this goto), the body must be SHORT + brace-flat + cleanup-ONLY with a real delete/dtor, and
// there must be no return inside — so user logic is never touched.
static void elideRaiiSkip(vector<string>& lines){
    auto fan=labelFanIn(lines);
    for(bool more=true;more;){ more=false; vector<string> out; out.reserve(lines.size());
      for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]); string g=soleGotoTarget(t);
        if(!g.empty() && t.rfind("if (",0)==0 && fan.count(g) && fan[g]==1){
            int depth=0; size_t L=string::npos; bool ok=true,hasCleanup=false;
            for(size_t j=i+1;j<lines.size();j++){ string tj=trim(lines[j]),lb;
                if(depth==0 && isLabelLine(tj,lb) && lb==g){ L=j; break; }
                if(tj.rfind("return",0)==0 || !cleanupNeutral(tj)){ ok=false; break; }
                if(tj.find("operator_delete")!=string::npos||tj.find("_dtor(")!=string::npos||tj.find("::~")!=string::npos||tj.find("_Unwind_Resume")!=string::npos) hasCleanup=true;
                for(char c:lines[j]){ if(c=='{')depth++; else if(c=='}')depth--; }
                if(depth!=0||j-i>40){ ok=false; break; } }
            if(L!=string::npos && ok && hasCleanup){ i=L; more=true; continue; } }   // excise [if-goto .. label] entirely
        out.push_back(lines[i]); }
      lines.swap(out); }
}

// THE destructor LADDER: a maximal RUN of consecutive cleanup-only lines — `operator_delete(a); if(sso) goto L;
// L: operator_delete(b); if(sso) goto M; ...` — the compiler's multi-object RAII teardown. The whole run is
// implicit in C++. Excise it wholesale, but ONLY when it's provably self-contained: brace-balanced, every
// goto inside targets a label inside the run (no jump out to user code), and no goto from OUTSIDE jumps into
// it (singleEntry) — so removing it can't strand any path. Never spans a function header or a return.
static void elideCleanupRun(vector<string>& lines){
    auto cleanupish=[&](const string& ln)->bool{ string x=trim(ln),lb; if(x.empty()||x=="}")return true; if(isLabelLine(x,lb))return true;
        if(x.rfind("return",0)==0) return false; if(!soleGotoTarget(x).empty())return true; return cleanupNeutral(x); };
    auto isReal=[&](const string& ln){ string x=trim(ln); return x.find("operator_delete")!=string::npos||x.find("_dtor(")!=string::npos||x.find("::~")!=string::npos||x.find("_Unwind_Resume")!=string::npos; };
    static int uid=0;
    for(bool more=true;more;){ more=false;
      for(size_t i=0;i<lines.size();i++){ if(!isReal(lines[i])||!cleanupish(lines[i])) continue;
        size_t s=i; while(s>0 && cleanupish(lines[s-1])) s--;
        size_t e=i+1; while(e<lines.size() && cleanupish(lines[e])) e++;
        int br=0,realN=0; bool hasGoto=false,hasUnwind=false; set<string> inner;
        for(size_t r=s;r<e;r++){ string x=trim(lines[r]),lb; for(char c:lines[r]){ if(c=='{')br++; else if(c=='}')br--; }
            if(isReal(lines[r])) realN++; if(x.find("_Unwind_Resume")!=string::npos||x.find("terminate")!=string::npos) hasUnwind=true;
            if(!soleGotoTarget(x).empty()) hasGoto=true; if(isLabelLine(x,lb)) inner.insert(lb); }
        // ONLY a genuine decompiler RAII ladder (goto-connected / multi-destructor / EH-unwind), NEVER a lone
        // `operator_delete(x);` in user/edited code — that must be preserved (it was getting silently dropped).
        if(br!=0 || !(hasGoto || hasUnwind || realN>=2)){ i=e; continue; }
        bool ok=true;                                                               // every goto INSIDE must stay inside (no jump out to user code)
        for(size_t r=s;r<e&&ok;r++){ string g=soleGotoTarget(trim(lines[r])); if(!g.empty()&&!inner.count(g)) ok=false; }
        if(!ok){ i=e; continue; }
        // The whole RAII ladder is implicit in C++. Every path through it converges on the exit (line e),
        // so redirect every goto into the ladder -> the exit label, then delete the ladder. Multi-entry safe:
        // all entries reach the same exit; only the (implicit) destructors are skipped.
        string exitLbl,xl; bool synth=false;
        if(e<lines.size() && isLabelLine(trim(lines[e]),xl)) exitLbl=xl;            // exit already labelled
        else { exitLbl="cl_exit_"+std::to_string(uid++); synth=true; }
        bool anyIn=false;
        for(size_t r=0;r<lines.size();r++){ if(r>=s&&r<e)continue; for(const auto& lb:inner){ string from="goto "+lb+";";
            size_t p=lines[r].find(from); if(p!=string::npos){ lines[r].replace(p,from.size(),"goto "+exitLbl+";"); anyIn=true; } } }
        if(synth && anyIn) lines.insert(lines.begin()+e, indentOf(lines[e<lines.size()?e:s])+exitLbl+":");   // materialize the exit label only if needed
        lines.erase(lines.begin()+s,lines.begin()+e); more=true; break; }
    }
}

// de-slop the remaining gotos: a forward conditional skip `if (c) goto L; <body> L:` is just `if (!(c)) { <body> }`.
// CONSERVATIVE so it never changes semantics: fold ONLY when L is the target of EXACTLY ONE goto (this one), the
// body is brace-balanced, and there is NO other label inside the body (so nothing jumps INTO the new if-block).
// clang reorders `if (C) {A} else {B}` so one arm is hoisted into the sibling block, reached by a cross-block
// goto — the ubiquitous validate-then-branch shape:
//     if (C) goto L;  <thenTail>  } else {  goto E;  L: <Lbody>  }   E:
// Rebuild it as a real if/else:  if (C) { <Lbody> } else { <thenTail> }   (both arms straight-line, single ref to L).
static void foldReorderedIfElse(vector<string>& lines){
    auto refCount=[&](const string& L){ int c=0; for(auto& l:lines){ set<string> tg; collectGotoTargets(l,tg); if(tg.count(L))c++; } return c; };
    auto str3=[&](size_t a,size_t b){ for(size_t r=a;r<b;r++){ string t=trim(lines[r]); if(t.empty())continue;            // straight-line? no label/goto, brace-balanced
            if(t.back()==':'&&isIdentName(t.substr(0,t.size()-1)))return false; if(t.find("goto ")!=string::npos)return false; }
        int d=0; for(size_t r=a;r<b;r++){ auto m=codeMask(lines[r]); for(size_t k=0;k<lines[r].size();k++){ if(!m[k])continue; if(lines[r][k]=='{')d++; else if(lines[r][k]=='}')d--; } } return d==0; };
    for(size_t ei=0; ei<lines.size(); ei++){
        if(trim(lines[ei])!="} else {") continue;
        string ind=indentOf(lines[ei]);
        size_t ee=ei+1; int depth=1;                                                                   // matching close of the else block
        for(;ee<lines.size();ee++){ auto m=codeMask(lines[ee]); for(size_t k=0;k<lines[ee].size();k++){ if(!m[k])continue; if(lines[ee][k]=='{')depth++; else if(lines[ee][k]=='}')depth--; } if(depth==0)break; }
        if(ee>=lines.size()) continue;
        size_t b=ei+1; while(b<ee && trim(lines[b]).empty())b++;                                        // 1st meaningful = `goto E;`
        if(b>=ee) continue; string g=trim(lines[b]); if(g.rfind("goto ",0)!=0||g.back()!=';') continue;
        string E=g.substr(5); E.pop_back(); if(!isIdentName(E)) continue;
        size_t lp=b+1; while(lp<ee && trim(lines[lp]).empty())lp++;                                     // 2nd meaningful = `L:`
        if(lp>=ee) continue; string ll=trim(lines[lp]); if(ll.size()<2||ll.back()!=':') continue;
        string L=ll.substr(0,ll.size()-1); if(!isIdentName(L)||refCount(L)!=1) continue;
        if(!str3(lp+1,ee)) continue;                                                                    // Lbody straight-line
        size_t gpos=string::npos;                                                                       // find `if (C) goto L;` in the then-block
        for(long r=(long)ei-1;r>=0;r--){ string t=trim(lines[r]); size_t gp=t.find(") goto ");
            if(t.rfind("if (",0)==0&&gp!=string::npos&&t.back()==';'){ string lab=t.substr(gp+7); if(!lab.empty()&&lab.back()==';')lab.pop_back(); if(lab==L){ gpos=(size_t)r; break; } }
            if(t=="} else {"||trim(lines[r])=="}"&&indentOf(lines[r])<=ind) break; }                    // don't cross out of the then-block
        if(gpos==string::npos||indentOf(lines[gpos])!=ind+"    ") continue;
        if(!str3(gpos+1,ei)) continue;                                                                  // thenTail straight-line
        string gt=trim(lines[gpos]); size_t gp=gt.find(") goto "); string C=gt.substr(4,gp-4);
        vector<string> repl; string gind=ind+"    ";
        repl.push_back(gind+"if ("+C+") {");
        for(size_t r=lp+1;r<ee;r++) repl.push_back("    "+lines[r]);
        if(gpos+1<ei){ repl.push_back(gind+"} else {"); for(size_t r=gpos+1;r<ei;r++) repl.push_back("    "+lines[r]); }
        repl.push_back(gind+"}");
        repl.push_back(ind+"}");                                                                         // re-close the outer then-block
        lines.erase(lines.begin()+gpos, lines.begin()+ee+1);
        lines.insert(lines.begin()+gpos, repl.begin(), repl.end());
        ei=gpos;
    }
}
static void foldIfGoto(vector<string>& lines){
    map<string,int> gt;
    for(auto& l:lines){ auto m=codeMask(l); size_t p=0; while((p=l.find("goto ",p))!=string::npos){ if(p<m.size()&&m[p]){ size_t s=p+5,e=s; while(e<l.size()&&isIdent(l[e]))e++; if(e>s)gt[l.substr(s,e-s)]++; } p+=5; } }
    for(size_t i=0;i+1<lines.size();i++){ string t=trim(lines[i]);
        if(t.rfind("if (",0)!=0||t.empty()||t.back()!=';') continue;
        size_t gp=t.rfind(") goto "); if(gp==string::npos) continue;
        string lbl=trim(t.substr(gp+7)); if(!lbl.empty()&&lbl.back()==';')lbl.pop_back();
        if(lbl.empty()||!gt.count(lbl)) continue;
        string cond=t.substr(4,gp-4), ind=indentOf(lines[i]); int depth=0; size_t L=string::npos; bool ok=true;
        for(size_t j=i+1;j<lines.size();j++){ string tj=trim(lines[j]);
            if(depth==0 && tj==lbl+":"){ L=j; break; }                                                 // label at the if's level = fold point
            for(char c:lines[j]){ if(c=='{')depth++; else if(c=='}'){ if(--depth<0){ok=false;break;} } }
            if(!ok||j-i>400){ok=false;break;} }                                                        // body must be brace-balanced; inner labels stay valid (goto into/out of a block is legal)
        if(L==string::npos||!ok||L<=i+1) continue;
        lines[i]=ind+"if (!("+cond+")) {";
        for(size_t r=i+1;r<L;r++) lines[r]="    "+lines[r];                                            // indent the body one level
        if(gt[lbl]==1) lines[L]=ind+"}";                                                               // L reached ONLY by this goto -> close + drop the now-orphan label
        else lines.insert(lines.begin()+L, ind+"}");                                                   // others still goto L -> keep the label, just close the block right before it
    }
}
// pass F2a-5 — drop no-op `if (C) { }` shells. libc++ SSO/capacity checks decompile to
// `if ((v & (1<<31)) == 0) { operator_delete(...); }`; after dead-store elimination empties the
// body, the `if (pure-cond) { }` left behind is pure noise. No else -> remove the whole if.
// `if (C) { } else { BODY }` -> `if (!(C)) { BODY }` (invert, drop the empty then-branch).
// safe to DISCARD: no function call (ident directly before `(`) and no assignment. Grouping parens are fine —
// so `(x & (1<<31)) == 0` is pure, but `getenv(&x)` and `a = b` are not. (hasSideEffect is too blunt: it
// rejects every condition with parens, which is nearly all of them.)
static bool condPure(const string& s){ auto m=codeMask(s);
    for(size_t i=0;i<s.size();i++){ if(!m[i])continue;
        if(s[i]=='(' && i>0 && isIdent(s[i-1])) return false;                                          // call: name(
        if(s[i]=='='){ bool cmp=(i+1<s.size()&&s[i+1]=='=')||(i>0&&(s[i-1]=='='||s[i-1]=='!'||s[i-1]=='<'||s[i-1]=='>')); if(!cmp) return false; } }
    return true;
}
static void dropEmptyIf(vector<string>& lines){
    for(bool more=true; more; ){ more=false; vector<string> out; out.reserve(lines.size());
        for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]); bool handled=false;
            if(t.rfind("if (",0)==0 && t.size()>3 && t.back()=='{'){ size_t ce=t.rfind(") {");
                if(ce!=string::npos && ce>=4){ string cond=t.substr(4,ce-4);
                    size_t j=i+1; while(j<lines.size()&&trim(lines[j]).empty())j++;
                    if(j<lines.size()){ string nx=trim(lines[j]);
                        if(nx=="}" && condPure(cond)){ i=j; handled=true; more=true; }                  // empty body, no else, pure cond -> remove the whole if
                        else if(nx=="} else {"){ string nc="!("+cond+")";                               // empty then -> invert + drop the else wrapper (cond still runs, so safe even if impure)
                            if(cond.rfind("!(",0)==0){ string inr; size_t e; if(balanced(cond,1,inr,e)&&e==cond.size()-1) nc=inr; }   // !(!(X)) -> X
                            out.push_back(indentOf(lines[i])+"if ("+nc+") {"); i=j; handled=true; more=true; } } } }
            if(!handled) out.push_back(lines[i]); }
        lines.swap(out); }
}
// pass F2a-6 — drop `loc_<hex>:` (and renamed L#) labels that NO goto targets. The structurer + foldIfGoto
// leave dead jump anchors behind; a label nothing jumps to is pure noise. literal/comment-aware via collectGotoTargets.
// `goto L;` / `if(c) goto L;` immediately followed by `L:` is a no-op jump to the next line -> drop the goto.
// (The big source: cleanup-ladder gotos redirected to an exit label that's now right after them.)
static void dropJumpToNext(vector<string>& lines){
    for(bool more=true;more;){ more=false; vector<string> out; out.reserve(lines.size());
        for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]); string g=soleGotoTarget(t);
            if(!g.empty()){ size_t j=i+1; while(j<lines.size()&&trim(lines[j]).empty())j++; string lb;
                if(j<lines.size() && isLabelLine(trim(lines[j]),lb) && lb==g){               // jumps to the immediately-following label
                    bool cond=t.rfind("if (",0)==0; string c; if(cond){ size_t gp=t.rfind(") goto "); c=t.substr(4,gp-4); }
                    if(!cond || condPure(c)){ more=true; continue; } } }                      // unconditional, or pure test -> drop the goto
            out.push_back(lines[i]); }
        lines.swap(out); }
}
static void dropOrphanLabels(vector<string>& lines){
    set<string> tg; for(auto& l:lines) collectGotoTargets(l, tg);
    vector<string> out; out.reserve(lines.size());
    for(auto& l:lines){ string t=trim(l);
        if(t.size()>1 && t.back()==':' && (isalpha((unsigned char)t[0])||t[0]=='_')){
            size_t e=0; while(e<t.size()&&isIdent(t[e]))e++;
            if(e==t.size()-1 && !tg.count(t.substr(0,e))) continue; }   // orphan label -> drop
        out.push_back(l); }
    lines.swap(out);
}
// after cppify, PARAMS are finally typed (their real `std::string [const&|*]` type was merged in from the // sig
// comment). Now finish the STL idioms: free-function `size(x)`->`x.size()`, and fix the access operator so a
// value/ref uses `.` and a pointer uses `->` (cppify can overwrite a `std::string*` param with `std::string const&`,
// leaving the body's `obj->size()` mismatched). TYPE-GATED to std::string vars -> safe.
static void methodifyStrings(vector<string>& lines){
    std::map<string,string> sop;   // std:: container var -> access operator ("." value/ref, "->" pointer)
    static const char* C[]={"string","vector","map","set","unordered_map","unordered_set","deque","list","array","pair"};
    for(auto& l:lines){ for(const char* cont:C){ string tk="std::"+string(cont); size_t p=0;
        while((p=l.find(tk,p))!=string::npos){ size_t q=p+tk.size();
            if(q<l.size()&&isIdent(l[q])){p=q;continue;}                          // std::string_view etc -> skip
            if(q<l.size()&&l[q]=='<'){ int d=0; while(q<l.size()){ if(l[q]=='<')d++; else if(l[q]=='>'){if(--d==0){q++;break;}} q++; } }   // skip <template args>
            bool ptr=false;
            while(q<l.size()){ if(l[q]==' '||l[q]=='&'){q++;continue;} if(l[q]=='*'){ptr=true;q++;continue;} if(l.compare(q,5,"const")==0&&(q+5>=l.size()||!isIdent(l[q+5]))){q+=5;continue;} break; }
            size_t e=q; while(e<l.size()&&isIdent(l[e]))e++; if(e>q&&!isdigit((unsigned char)l[q])) sop[l.substr(q,e-q)]=ptr?"->":"."; p=q+1; } } }
    if(sop.empty()) return;
    auto repl=[&](string& s,const string& from,const string& to){ if(from==to)return; size_t p=0; while((p=s.find(from,p))!=string::npos){ s.replace(p,from.size(),to); p+=to.size(); } };
    static const char* M[]={"size","length","c_str","empty","clear","data","begin","end","front","back","push_back","pop_back","count"};
    for(auto& l:lines){ for(auto& kv:sop){ const string& V=kv.first; const string& op=kv.second;
        // operator_index(V, EXPR) / operator_index(&V, EXPR) -> V[EXPR]  (the [] operator)
        for(string pre:{"operator_index("+V+", ","operator_index(&"+V+", "}){ size_t p=0;
            while((p=l.find(pre,p))!=string::npos){ size_t as=p+pre.size(); int d=1; size_t q=as;
                while(q<l.size()){ if(l[q]=='(')d++; else if(l[q]==')'){if(--d==0)break;} q++; }
                if(d!=0||q>=l.size())break; string idx=l.substr(as,q-as); l.replace(p,q-p+1,V+"["+idx+"]"); p+=V.size()+2+idx.size(); } }
        for(const char* m:M){
            repl(l, string(m)+"(&"+V+")", V+op+m+"()");          // method(&x) -> x.method()
            repl(l, string(m)+"("+V+")",  V+op+m+"()");          // method(x)  -> x.method()
            if(op==".") repl(l, V+"->"+m+"()", V+"."+m+"()");    // ref/value written as ptr -> fix to '.'
            else        repl(l, V+"."+m+"()", V+"->"+m+"()"); } } }
}
// ── SEMANTIC NAMING FROM FORMAT STRINGS / OUTPUT LABELS ──────────────────────────────────────────
// The single richest source of a variable's MEANING in a stripped binary is the text printed around it:
//   printf("time=%d", v68)         -> v68 is `time`
//   cout << "balance = " << v40    -> v40 is `balance`
//   scanf("%d", &v8) after a prompt … (the prompt naming is handled elsewhere)
// This pass reads printf/scanf/fprintf format strings and cout/cerr `"label" << var` chains, extracts the
// label word sitting against each conversion / inserted variable, and renames the corresponding temp.
// Renaming is pure cosmetics (no semantic change) and is collision-guarded against every identifier already
// in the function plus C/C++ keywords (isBuiltinName), so it can only ever improve readability.
static string fmtLabel(const string& fmt, size_t pct){            // the identifier word just before fmt[pct]=='%'
    long i=(long)pct-1; while(i>=0 && !(isalnum((unsigned char)fmt[i])||fmt[i]=='_')) i--;
    long e=i; while(i>=0 && (isalnum((unsigned char)fmt[i])||fmt[i]=='_')) i--;
    if(e>i){ string w=fmt.substr(i+1,e-i); if(!w.empty()&&(isalpha((unsigned char)w[0])||w[0]=='_')) return w; }
    return "";
}
static string lastWord(const string& s){                          // last identifier word in a string (for cout labels)
    long i=(long)s.size()-1; while(i>=0 && !(isalnum((unsigned char)s[i])||s[i]=='_')) i--;
    long e=i; while(i>=0 && (isalnum((unsigned char)s[i])||s[i]=='_')) i--;
    if(e>i){ string w=s.substr(i+1,e-i); if(!w.empty()&&(isalpha((unsigned char)w[0])||w[0]=='_')) return w; }
    return "";
}
static void nameByFormat(vector<string>& lines){
    static const set<string> F0={"printf","sprintf","snprintf","scanf","sscanf"};   // format = 1st arg
    static const set<string> F1={"fprintf","fscanf","dprintf"};                     // format = 2nd arg
    size_t fi=0;
    while(fi<lines.size()){
        if(!dvIsFnHeader(lines[fi])){ fi++; continue; }
        size_t start=fi+1, end=dvFnBody(lines,fi);
        set<string> idents;                                       // every identifier already in the function (collision guard)
        for(size_t r=start;r<end;r++){ const string& L=lines[r]; auto m=codeMask(L);
            for(size_t k=0;k<L.size();){ if(k<m.size()&&m[k]&&(isalpha((unsigned char)L[k])||L[k]=='_')){ size_t e=k; while(e<L.size()&&isIdent(L[e]))e++; idents.insert(L.substr(k,e-k)); k=e; } else k++; } }
        map<string,string> ren; set<string> usedLabels;
        auto isTemp=[](const string& v){ if(!(v.size()>1&&(v[0]=='v'||v[0]=='t')&&isdigit((unsigned char)v[1])))return false; for(size_t k=2;k<v.size();k++)if(!isdigit((unsigned char)v[k]))return false; return true; };
        auto propose=[&](const string& a,const string& label){ string v=deref(a); if(!isTemp(v)||ren.count(v))return;
            string lb; for(char c:label){ if(isalnum((unsigned char)c)||c=='_')lb+=(char)tolower((unsigned char)c); }
            if(lb.size()<2||lb.size()>24||isdigit((unsigned char)lb[0])||isBuiltinName(lb))return;
            string fin=lb; int s=2; while(idents.count(fin)||usedLabels.count(fin))fin=lb+std::to_string(s++);
            ren[v]=fin; usedLabels.insert(fin); };
        for(size_t r=start;r<end;r++){ const string& L=lines[r]; auto m=codeMask(L);
            // printf-family
            for(int pass=0;pass<2;pass++){ const set<string>& S=pass?F1:F0; int fmtIdx=pass?1:0;
                for(const string& fn:S){ size_t p=0;
                    while((p=L.find(fn+"(",p))!=string::npos){ bool lb=(p==0||!isIdent(L[p-1]));
                        if(!lb||p>=m.size()||!m[p]){ p+=fn.size(); continue; }
                        size_t op=p+fn.size(); int d=0; size_t q=op; for(;q<L.size();q++){ if(q<m.size()&&!m[q])continue; if(L[q]=='(')d++; else if(L[q]==')'){ if(--d==0)break; } }
                        if(q>=L.size()){ p=op; continue; }
                        vector<string> args=splitArgs(L.substr(op+1,q-op-1));
                        if((int)args.size()>fmtIdx){ string f=trim(args[fmtIdx]);
                            if(f.size()>=2&&f.front()=='"'&&f.back()=='"'){ string fmt=f.substr(1,f.size()-2);
                                if(fmt.find("%*")==string::npos){ int conv=0;
                                    for(size_t j=0;j<fmt.size();j++){ if(fmt[j]!='%')continue; if(j+1<fmt.size()&&fmt[j+1]=='%'){ j++; continue; }
                                        string label=fmtLabel(fmt,j); int ai=fmtIdx+1+conv; if(ai<(int)args.size()&&!label.empty()) propose(args[ai],label); conv++; } } } }
                        p=q+1; } } }
            // cout/cerr style: a string literal directly followed by `<< <temp>`
            { const string sep="\" << "; size_t p=0;
                while((p=L.find(sep,p))!=string::npos){ size_t vs=p+sep.size(); size_t ve=vs; while(ve<L.size()&&isIdent(L[ve]))ve++;
                    if(ve>vs){ string var=L.substr(vs,ve-vs);
                        long oq=(long)p-1; while(oq>=0 && !(L[oq]=='"'&&(oq==0||L[oq-1]!='\\'))) oq--;
                        if(oq>=0 && (size_t)oq<p){ string label=lastWord(L.substr(oq+1,p-oq-1)); if(!label.empty()) propose(var,label); } }
                    p=vs; } }
            // STRUCT FIELD assignment: `obj->balance = v68;` / `v68 = obj->balance;` -> name v68 after the field.
            { string lt=trim(L); if(!lt.empty()&&lt.back()==';'){ string body=lt.substr(0,lt.size()-1);
                auto mm=codeMask(body); int d=0; size_t eq=string::npos;
                for(size_t k=0;k<body.size();k++){ if(k<mm.size()&&!mm[k])continue; char c=body[k]; if(c=='('||c=='[')d++; else if(c==')'||c==']'){if(d)d--;}
                    else if(c=='='&&d==0){ char pr=k?body[k-1]:0,nx=k+1<body.size()?body[k+1]:0; if(nx!='='&&pr!='='&&pr!='!'&&pr!='<'&&pr!='>'&&pr!='+'&&pr!='-'&&pr!='*'&&pr!='/'&&pr!='%'&&pr!='&'&&pr!='|'&&pr!='^'){ eq=k; break; } } }
                if(eq!=string::npos){ string lhs=trim(body.substr(0,eq)), rhs=trim(body.substr(eq+1));
                    auto fieldOf=[](const string& e)->string{ size_t a=e.rfind("->"); size_t b=e.rfind('.'); size_t s=string::npos; int adv=0;
                        if(a!=string::npos){ s=a; adv=2; } if(b!=string::npos&&(s==string::npos||b>s)){ s=b; adv=1; }
                        if(s==string::npos) return ""; string f=e.substr(s+adv); for(char c:f) if(!isIdent(c)) return "";
                        if(f.size()>=2 && !(f[0]=='f'&&(isdigit((unsigned char)f[1])||(f[1]=='m'&&f.size()>2)))) return f; return ""; };
                    string lf=fieldOf(lhs), rf=fieldOf(rhs);
                    if(!lf.empty()) propose(rhs, lf);
                    if(!rf.empty()) propose(lhs, rf); } } }
            // SCANF PROMPT pairing: `printf("Enter age: "); scanf("%d", &v8);` -> v8 named `age` (the prompt's last word).
            { static const char* RD[]={"scanf(","fscanf(","sscanf(","fgets(","getline("}; bool isRead=false;
                for(const char* rd:RD) if(L.find(rd)!=string::npos){ isRead=true; break; }
                if(isRead){ string prompt;                                  // the nearest preceding output string (1 line back)
                    for(long pr=(long)r-1; pr>=(long)start; pr--){ string pt=trim(lines[pr]); if(pt.empty())continue;
                        if(pt.find('"')!=string::npos && (pt.find("printf")!=string::npos||pt.find("fputs")!=string::npos||pt.find("puts")!=string::npos||pt.find("fprintf")!=string::npos||pt.find("cout")!=string::npos)){
                            size_t q1=pt.find('"'),q2=(q1==string::npos)?string::npos:pt.find('"',q1+1); if(q2!=string::npos) prompt=lastWord(pt.substr(q1+1,q2-q1-1)); }
                        break; }
                    if(!prompt.empty()){ size_t p=0;                        // name the (single) &TEMP read target
                        while((p=L.find("&",p))!=string::npos){ size_t s=p+1,e=s; while(e<L.size()&&isIdent(L[e]))e++; if(e>s){ propose(L.substr(p,e-p), prompt); break; } p++; } } } }
        }
        if(!ren.empty()) for(size_t r=start;r<end;r++){ string& L=lines[r]; auto m=codeMask(L);
            for(auto& kv:ren){ size_t p=0; while((p=L.find(kv.first,p))!=string::npos){ bool ok=p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1])); size_t af=p+kv.first.size(); ok=ok&&(af>=L.size()||!isIdent(L[af]));
                if(ok){ L=L.substr(0,p)+kv.second+L.substr(af); m=codeMask(L); p+=kv.second.size(); } else p+=kv.first.size(); } } }
        fi=end;
    }
}
int main(int argc, char** argv){
    vector<string> lines;
    // read from a FILE arg if given (shell-free -> works under NXRT, where stdin isn't forwarded), else stdin
    FILE* in = (argc>1 && argv[1][0]!='-') ? fopen(argv[1],"rb") : stdin; if(!in) in=stdin;
    { int c; string cur; while((c=getc(in))!=EOF){ if(c=='\n'){ lines.push_back(cur); cur.clear(); } else cur+=(char)c; } if(!cur.empty())lines.push_back(cur); }
    if(in!=stdin) fclose(in);
    stripCanary(lines);    // pass 0 (G6) — remove the -O0 stack-smashing-guard boilerplate
    // pass 0b — drop explicit C++ destructor calls (`~basic_string(&x);`, `~Vec(&x);`): RAII makes them
    // implicit at scope end, so they're pure decompiler noise. Unused cleanup labels get removed downstream.
    { vector<string> keep; for(auto& l:lines){ string t=trim(l);
        if(t.size()>3 && t[0]=='~' && t.back()==';' && t.find('(')!=string::npos){ size_t p=1; while(p<t.size()&&(isalnum((unsigned char)t[p])||t[p]=='_'))p++; if(p>1 && p<t.size() && t[p]=='(') continue; }   // matches `~Name(...);`
        // also `Name_dtor(...);` — the lifter already renamed `~Name` to `Name_dtor` (shortName), so the tilde is gone
        if(t.back()==';' && t.find('=')==string::npos){ size_t dp=t.find("_dtor("); if(dp>0 && dp!=string::npos){
            bool ok=true; for(size_t k=0;k<dp;k++){ char c=t[k]; if(!(isalnum((unsigned char)c)||c=='_'||c==':')){ ok=false; break; } } if(ok) continue; } }
        keep.push_back(l); } lines.swap(keep); }
    dropRaiiLoops(lines);  // pass 0c — remove the now-empty `while (it != &local) {}` RAII cleanup loops
    repurposeParams(lines);// pass 0d — rename clobbered main() params (`argc = "..."`) to fresh typed locals

    // pass 1 — per-line idiom folding (all literal-aware)
    for(auto& l:lines){
        for(int k=0;k<40 && foldCalls(l);k++){}
        arraySubscript(l);     // *(p + i) -> p[i]  (Binary-Ninja-style array indexing; C-guaranteed identity)
        foldConstArith(l);     // 0 + 1 -> 1  (literal arithmetic the lifter leaves behind)
        cleanBoolBits(l);
        nameStreams(l); mapStdio(l);
        dedupeParens(l);
        l=stripOuterParens(l);
        l=foldCompound(l);     // x = (x OP y); -> x OP= y;   and  x = x + 1; -> x++;
    }
    foldStreamHandle(lines);   // (*v138) where v138=stdin -> stdin (FILE** GOT-slot alias)

    // pass 2 — inline single-use string-literal decls (compiler-introduced s_ globals)
    { struct Decl{ int idx; string nm, val; }; vector<Decl> decls; std::map<int,int> declIdx;
      for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]);
        if(t.rfind("const char* ",0)==0){ size_t eq=t.find(" = \""); if(eq!=string::npos && t.back()==';'){
            string nm=trim(t.substr(12,eq-12)); string val=t.substr(eq+3); val=val.substr(0,val.size()-1);
            declIdx[(int)i]=(int)decls.size(); decls.push_back({(int)i,nm,val}); } } }
      map<string,int> uses;
      for(size_t i=0;i<lines.size();i++){ if(declIdx.count((int)i)) continue; auto m=codeMask(lines[i]);
        for(auto& d:decls){ size_t p=0; while((p=lines[i].find(d.nm,p))!=string::npos){
            bool ok = p<m.size()&&m[p] && (p==0||!isIdent(lines[i][p-1])) && (p+d.nm.size()>=lines[i].size()||!isIdent(lines[i][p+d.nm.size()]));
            if(ok) uses[d.nm]++; p+=d.nm.size(); } } }
      std::map<int,bool> drop;
      for(auto& d:decls){ if(uses[d.nm]!=1) continue; drop[d.idx]=true;
        for(size_t i=0;i<lines.size();i++){ if(declIdx.count((int)i)) continue; string& l=lines[i]; auto m=codeMask(l);
          for(const string& form : { "&"+d.nm, d.nm }){ size_t p=0; while((p=l.find(form,p))!=string::npos){
              bool ok = p<m.size()&&m[p] && (p==0||!isIdent(l[p-1])); size_t af=p+form.size(); ok = ok && (af>=l.size()||!isIdent(l[af]));
              if(ok){ l=l.substr(0,p)+d.val+l.substr(af); m=codeMask(l); p+=d.val.size(); } else p+=form.size(); } } } }
      vector<string> keep; for(size_t i=0;i<lines.size();i++) if(!drop.count((int)i)) keep.push_back(lines[i]);
      lines.swap(keep); }

    // pass 3 — per-function copy-propagation + dead-store elimination on vN/tN temps
    {
      size_t i=0;
      while(i<lines.size()){
        bool hdr = dvIsFnHeader(lines[i]);   // col-0 functions AND indented class methods -> copy-prop their bodies too
        if(!hdr){ i++; continue; }
        size_t start=i+1, j=start; int depth=1;
        for(; j<lines.size(); j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        size_t end=j;
        bool again=true; int guard=0;
        while(again && guard++<50){ again=false;
          map<string,int> def, use; set<string> mutated, incdec;
          auto eachTemp=[&](const string& s, auto fn){ auto m=codeMask(s); for(size_t k=0;k<s.size();){ if(m[k]&&(s[k]=='v'||s[k]=='t')&&k+1<s.size()&&isdigit((unsigned char)s[k+1])&&(k==0||!isIdent(s[k-1]))){ size_t e=k+1; while(e<s.size()&&isdigit((unsigned char)s[e]))e++; if(e>=s.size()||!isIdent(s[e])) fn(s.substr(k,e-k)); k=e; } else k++; } };
          for(size_t r=start;r<end;r++){ bool comp; string rhsv; string lhs=assignLHS(lines[r],comp,rhsv);
            if(!lhs.empty()){ def[lhs]++; if(comp) use[lhs]++; eachTemp(rhsv,[&](const string& v){ use[v]++; }); }
            else eachTemp(lines[r],[&](const string& v){ use[v]++; });
            collectMutated(lines[r], mutated); collectIncDec(lines[r], incdec); }
          // copy-propagate: vN PLAINLY defined once, RHS a safe atom -> inline, drop the def
          for(size_t r=start;r<end && !again;r++){ bool comp; string rhs; string lhs=assignLHS(lines[r],comp,rhs);
            if(lhs.empty()||comp) continue; if(def[lhs]!=1) continue;
            if(incdec.count(lhs)) continue;   // never propagate a var that's also ++/--’d (a loop counter `v8++` would become `0++`); plain-`=` self-def is fine since def==1
            bool atom = !rhs.empty() && (rhs[0]=='"' || isdigit((unsigned char)rhs[0]) || (rhs[0]=='-'&&rhs.size()>1&&isdigit((unsigned char)rhs[1])));
            string idRhs; if(!atom){ bool ident=!rhs.empty()&&!isdigit((unsigned char)rhs[0]); for(char c:rhs) if(!isIdent(c)){ident=false;break;} if(ident){ atom=true; idRhs=rhs; } }
            // &X : address-of a simple local — the ADDRESS is stable even when X's CONTENTS are later mutated,
            // so inlining `vN = &X` is always safe (leave idRhs empty so the mutated-source check is skipped).
            if(!atom && rhs.size()>1 && rhs[0]=='&'){ string inr=rhs.substr(1); bool id=!inr.empty()&&(isalpha((unsigned char)inr[0])||inr[0]=='_'); for(char c:inr) if(!isIdent(c)){id=false;break;} if(id){ atom=true; rhs="("+rhs+")"; } }   // parenthesize so `vN->f0` -> `(&X)->f0`, never `&X->f0`
            if(!atom) continue;
            if(!idRhs.empty() && mutated.count(idRhs)) continue;       // RHS source mutated between def/use -> unsafe
            int declIdx=-1;
            for(size_t r2=start;r2<end;r2++){ if(r2==r) continue; string& L=lines[r2];
              { string dt=trim(L); if(declIdx<0 && !dt.empty() && dt.back()==';' && dt.find('=')==string::npos && dt.find('(')==string::npos){ string bd=dt.substr(0,dt.size()-1);   // a PURE declaration of lhs (`int v92;`) -> drop it; never substitute the value INTO the declaration
                  if(bd.size()>lhs.size() && bd.compare(bd.size()-lhs.size(),lhs.size(),lhs)==0){ char bc=bd[bd.size()-lhs.size()-1]; if((bc==' '||bc=='*'||bc=='&'||bc=='\t') && !trim(bd.substr(0,bd.size()-lhs.size())).empty()){ declIdx=(int)r2; continue; } } } }
              auto m=codeMask(L); size_t p=0;
              while((p=L.find(lhs,p))!=string::npos){ bool ok=p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1])); size_t af=p+lhs.size(); ok=ok&&(af>=L.size()||!isIdent(L[af]));
                if(ok){ L=L.substr(0,p)+rhs+L.substr(af); m=codeMask(L); p+=rhs.size(); } else p+=lhs.size(); } }
            if(declIdx>=0){ size_t a=r,b=(size_t)declIdx; if(b>a){ lines.erase(lines.begin()+b); lines.erase(lines.begin()+a); } else { lines.erase(lines.begin()+a); lines.erase(lines.begin()+b); } end-=2; }
            else { lines.erase(lines.begin()+r); end--; } again=true;
          }
          if(again) continue;
          // dead-store: vN never read, RHS side-effect-free -> drop
          for(size_t r=start;r<end && !again;r++){ bool comp; string rhs; string lhs=assignLHS(lines[r],comp,rhs);
            if(lhs.empty()||comp) continue; if(use[lhs]!=0) continue; if(hasSideEffect(rhs)) continue;
            lines.erase(lines.begin()+r); end--; again=true;
          }
        }
        i=end;
      }
    }

    // pass 3.5 — fold the return-value temp + cleanup gotos into direct returns:
    //   vN = K; goto loc_X;   ->   return K;       (vN = the function's return variable)
    // then drop labels nothing jumps to. RAII-cleanup destructors at the old labels are
    // left as trailing dead code (the structured lifter emitted them per scope-exit).
    {
      auto isTmp=[](const string& v){ return v.size()>1&&(v[0]=='v'||v[0]=='t')&&isdigit((unsigned char)v[1]); };
      // cleanup = blank / label / ~dtor / a sub_<x>(&...) destructor call (NOT a plain side-effecting sub_x() call)
      auto isCleanup=[&](const string& c){ if(c.empty()||c[0]=='~'||(c.size()>1&&c.back()==':'&&c.rfind("loc_",0)==0)) return true;
          if(c.rfind("sub_",0)==0) return c.find("(&")!=string::npos; return false; };
      size_t i=0;
      while(i<lines.size()){
        string t=trim(lines[i]);
        bool hdr = !t.empty()&&t.back()=='{'&&t.find('(')!=string::npos&&indentOf(lines[i]).empty()
                   && t.rfind("if",0)!=0&&t.rfind("for",0)!=0&&t.rfind("while",0)!=0&&t.rfind("else",0)!=0&&t.rfind("switch",0)!=0&&t.rfind("struct",0)!=0&&t.rfind("class",0)!=0;
        if(!hdr){ i++; continue; }
        size_t start=i+1, j=start; int depth=1;
        for(; j<lines.size(); j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        size_t end=j;
        std::map<string,size_t> labelAt; for(size_t r=start;r<end;r++){ string b=trim(lines[r]); if(b.size()>1&&b.back()==':'&&b.rfind("loc_",0)==0) labelAt[b.substr(0,b.size()-1)]=r; }
        string rv; for(size_t r=start;r<end;r++){ string b=trim(lines[r]); if(b.rfind("return ",0)==0&&b.back()==';'){ string v=b.substr(7,b.size()-8); bool ok=isTmp(v); for(char c:v) if(!isIdent(c))ok=false; if(ok) rv=v; } }
        if(!rv.empty()){
          // CORRECT fold: scanning forward from a line (skipping cleanup only), does the path provably
          // reach `return rv;`/`return <const>;`? — directly, or via ONE FORWARD goto (never a back-edge
          // loop goto, never past a `}` or side-effecting code). Only then is `rv = K;` a branch return.
          std::function<bool(size_t,int)> reaches=[&](size_t from,int hops)->bool{ if(hops<0) return false;
            for(size_t r2=from;r2<end;r2++){ string c=trim(lines[r2]); if(c.empty()||isCleanup(c)) continue;
              if(c=="return "+rv+";") return true;
              if(c.rfind("return ",0)==0&&c.back()==';'){ string v=c.substr(7,c.size()-8); return !v.empty()&&(isdigit((unsigned char)v[0])||v[0]=='-'); }
              if(c.rfind("goto ",0)==0){ string L=c.substr(5); if(!L.empty()&&L.back()==';')L.pop_back(); L=trim(L); auto it=labelAt.find(L); return it!=labelAt.end()&&it->second>r2 && reaches(it->second+1,hops-1); }
              return false; }
            return false; };
          for(size_t r=start;r<end;r++){ string b=trim(lines[r]);
            if(b.rfind(rv+" = ",0)==0 && b.back()==';' && reaches(r+1,3)){ string X=b.substr(rv.size()+3); X=X.substr(0,X.size()-1); lines[r]=indentOf(lines[r])+"return "+X+";"; } }
          bool stillAssigned=false; for(size_t r=start;r<end;r++) if(trim(lines[r]).rfind(rv+" = ",0)==0) stillAssigned=true;
          if(!stillAssigned) for(size_t r=start;r<end;r++) if(trim(lines[r])=="return "+rv+";") lines[r]=indentOf(lines[r])+"return 0;";
        }
        i=end;
      }
      // A) drop dead line-leading gotos (previous real statement is a return)
      { vector<string> keep; for(auto& l:lines){ string t=trim(l);
          if(t.rfind("goto ",0)==0){ int k=(int)keep.size()-1; while(k>=0){ string p=trim(keep[k]); if(p.empty()||isCleanup(p)){ k--; continue; } break; }
            if(k>=0 && trim(keep[k]).rfind("return",0)==0) continue; }
          keep.push_back(l); } lines.swap(keep); }
      // B) drop labels nothing jumps to — recognising INLINE `if(...) goto loc_X;` too
      { set<string> tg; for(auto& l:lines) collectGotoTargets(l,tg);
        vector<string> keep; for(auto& l:lines){ string t=trim(l); if(t.size()>1&&t.back()==':'&&t.rfind("loc_",0)==0 && !tg.count(t.substr(0,t.size()-1))) continue; keep.push_back(l); } lines.swap(keep); }
      // C) drop unreachable code after a return/goto WITHIN its own block (until a leading } closes
      //    that block, or a live label re-activates). A return in an if-branch does NOT kill the else.
      { vector<string> keep; long long depth=0, deadDepth=1LL<<40;   // long long: sentinel must survive LLP64 (Windows `long`=32-bit) or it truncates to 0 and the pass deletes everything
        for(auto& l:lines){ string t=trim(l); int o=0,c=0; { auto m=codeMask(l); for(size_t k=0;k<l.size();k++){ if(!m[k])continue; if(l[k]=='{')o++; else if(l[k]=='}')c++; } }
          int leadClose=0; for(char ch:t){ if(ch=='}')leadClose++; else if(ch==' '||ch=='\t')continue; else break; }
          long long openDepth=depth-leadClose, after=depth+o-c; bool isLabel=t.size()>1&&t.back()==':'&&t.rfind("loc_",0)==0;
          if(depth>=deadDepth){                                  // inside a dead region
            if(isLabel || t=="do {" || openDepth<deadDepth){ deadDepth=1LL<<40; keep.push_back(l); if(t.rfind("return",0)==0||t.rfind("goto ",0)==0) deadDepth=after; depth=after; continue; }   // a } closed the dead block, a live label, OR a `do {` loop header (its body is reached via the back-edge, not dead) -> reactivate
            depth=after; continue;                               // else skip the dead line
          }
          keep.push_back(l); if(t.rfind("return",0)==0||t.rfind("goto ",0)==0) deadDepth=after; depth=after; }
        lines.swap(keep); }
      // D) remove empty if/else shells — ONLY when the condition is side-effect-free.
      //    A `cond` containing a call (write(), calloc(), …) is NEVER dropped, and loops
      //    (while/for, whose condition may do the work) are never touched.
      {
        // A `(` indicates a CALL (side effect) when its previous non-space char is `)` or `]`
        // (indirect/funcptr/array call like (*fp)(), (**g)(), tbl[i]()) OR a non-keyword
        // identifier (write(), calloc()). A `(` after an operator/`(`/start is pure grouping.
        auto hasCall=[](const string& s){
          for(size_t i=0;i<s.size();i++){ if(s[i]!='(') continue;
            size_t j=i; char pc=0; while(j>0){ j--; char c=s[j]; if(c!=' '&&c!='\t'){ pc=c; break; } }
            if(pc==0) continue;                              // leading '(' -> grouping
            if(pc==')'||pc==']') return true;                // call on an expression result
            if(isalnum((unsigned char)pc)||pc=='_'){ size_t b=j+1; while(b>0&&isIdent(s[b-1]))b--; string w=s.substr(b,j+1-b);
              if(w!="if"&&w!="while"&&w!="for"&&w!="switch") return true; } }
          return false; };
        for(int rounds=0; rounds<8; rounds++){ bool changed=false; vector<string> keep; keep.reserve(lines.size());
          for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]);
            bool isIf = t.rfind("if ",0)==0 && !t.empty() && t.back()=='{';
            // both branches empty: `if (C) {` + `} else {` + `}`  -> drop all three (C pure)
            if(isIf && i+2<lines.size() && trim(lines[i+1])=="} else {" && trim(lines[i+2])=="}" && !hasCall(t)){ i+=2; changed=true; continue; }
            // empty then, no else: `if (C) {` + `}` -> drop both (C pure)
            if(isIf && i+1<lines.size() && trim(lines[i+1])=="}" && !hasCall(t)){ i+=1; changed=true; continue; }
            // empty else after a real then: `} else {` + `}` -> `}` (always safe)
            if(t=="} else {" && i+1<lines.size() && trim(lines[i+1])=="}"){ keep.push_back(indentOf(lines[i])+"}"); i+=1; changed=true; continue; }
            keep.push_back(lines[i]); }
          lines.swap(keep); if(!changed) break; }
      }
    }

    // pass 3.7 — label hygiene: (1) drop a `goto X;` whose target X: is the very next meaningful line
    // (a no-op fall-through), then (2) re-drop any now-unused loc_ labels, then (3) rename the surviving
    // `loc_<hex>` labels to clean per-function L1, L2, … — a PRO decompiler, not loc_<address> slop.
    {
      // (0) IF-INVERSION: `if (C) goto L;`  <straight-line block>  `L:`  ->  `if (!(C)) { block }`
      //     fires ONLY when L is the target of this one goto, the block has no labels/gotos, and is
      //     brace-balanced — a behavior-preserving fold that kills a lot of the goto-soup. (1)/(2) below
      //     then drop the now-dead label. Conservative on purpose: skips anything it can't prove safe.
      { bool ch=true; int gd=0; while(ch && gd++<12){ ch=false;
          for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]);
            if(t.rfind("if (",0)!=0 && t.rfind("if(",0)!=0) continue;
            size_t op=lines[i].find('('); if(op==string::npos) continue;
            string cond; size_t ce; if(!balanced(lines[i],op,cond,ce)) continue;
            string rest=trim(lines[i].substr(ce+1));
            if(rest.rfind("goto ",0)!=0 || rest.empty() || rest.back()!=';') continue;          // must be exactly `if (C) goto L;`
            string L=trim(rest.substr(5)); if(!L.empty()&&L.back()==';')L.pop_back(); L=trim(L);
            if(L.empty()){ continue; } { bool idok=true; for(char c:L) if(!isIdent(c)){idok=false;break;} if(!idok) continue; }
            int m=-1; bool ok=true; int depth=0;                                                 // find L: forward at the if's OWN depth; bail on any goto/scope-exit inside the block
            for(size_t j=i+1;j<lines.size();j++){ string tj=trim(lines[j]); if(tj.empty()) continue;
                if(depth==0 && tj.size()>1 && tj.back()==':' && tj.rfind("loc_",0)==0){ if(tj==L+":") m=(int)j; break; }   // only a label at depth 0 is a valid fold point — NOT one inside a sibling `} else {` block (that fold scrambles switch cases)
                if(tj.rfind("goto ",0)==0){ ok=false; break; }
                auto msk=codeMask(lines[j]); for(size_t k=0;k<lines[j].size()&&ok;k++){ if(!msk[k])continue; char c=lines[j][k]; if(c=='{')depth++; else if(c=='}'){ if(--depth<0){ok=false;break;} } }   // depth<0 => we exited the if's enclosing scope; the label lives elsewhere -> don't fold
                if(!ok) break; }
            if(!ok || m<=(int)i+1) continue;                                                     // m<=i+1 => empty block, skip
            { int refs=0; for(auto& l:lines){ set<string> tg; collectGotoTargets(l,tg); if(tg.count(L)) refs++; } if(refs!=1) continue; }   // L used by exactly this goto
            string ind=indentOf(lines[i]);
            lines[i]=ind+"if (!("+trim(cond)+")) {";
            lines[m]=ind+"}";                                                                    // the label becomes the closing brace; the goto is gone
            ch=true;
          } } }
      // (1) remove redundant forward gotos: `goto L;` immediately followed by `L:`
      { vector<string> keep; for(size_t i=0;i<lines.size();i++){ string t=trim(lines[i]);
          if(t.rfind("goto ",0)==0 && t.back()==';'){ string L=trim(t.substr(5)); if(!L.empty()&&L.back()==';')L.pop_back(); L=trim(L);
            size_t j=i+1; while(j<lines.size()&&trim(lines[j]).empty())j++;                       // next non-blank line
            if(j<lines.size() && trim(lines[j])==L+":") continue; }                                // it's the target label -> the goto is a fall-through, drop it
          keep.push_back(lines[i]); } lines.swap(keep); }
      // (2) drop labels nothing jumps to anymore (some became unused after step 1)
      { set<string> tg; for(auto& l:lines) collectGotoTargets(l,tg);
        vector<string> keep; for(auto& l:lines){ string t=trim(l); if(t.size()>1&&t.back()==':'&&t.rfind("loc_",0)==0 && !tg.count(t.substr(0,t.size()-1))) continue; keep.push_back(l); } lines.swap(keep); }
      // (3) rename surviving loc_<hex> -> L1, L2, … per function (function-scoped, so each fn resets)
      size_t i=0;
      while(i<lines.size()){ string t=trim(lines[i]);
        bool hdr = !t.empty() && t.back()=='{' && t.find('(')!=string::npos && indentOf(lines[i]).empty()
                   && t.rfind("if",0)!=0 && t.rfind("for",0)!=0 && t.rfind("while",0)!=0 && t.rfind("else",0)!=0 && t.rfind("switch",0)!=0 && t.rfind("struct",0)!=0 && t.rfind("class",0)!=0;
        if(!hdr){ i++; continue; }
        size_t start=i+1, j=start; int depth=1;
        for(; j<lines.size(); j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        size_t end=j; map<string,string> ren; int n=0;
        for(size_t r=start;r<end;r++){ string b=trim(lines[r]); if(b.size()>1&&b.back()==':'&&b.rfind("loc_",0)==0){ string nm=b.substr(0,b.size()-1); if(!ren.count(nm)) ren[nm]="L"+std::to_string(++n); } }
        if(!ren.empty()) for(size_t r=start;r<end;r++){ string& L=lines[r]; auto m=codeMask(L);
          for(auto& kv:ren){ size_t p=0; while((p=L.find(kv.first,p))!=string::npos){ bool ok=p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1])); size_t af=p+kv.first.size(); ok=ok&&(af>=L.size()||!isIdent(L[af]));
            if(ok){ L=L.substr(0,p)+kv.second+L.substr(af); m=codeMask(L); p+=kv.second.size(); } else p+=kv.first.size(); } } }
        i=end;
      }
    }

    // pass 3.8 — array-indexing recovery + dead synthesized-struct removal. A `T** P` parameter accessed
    // as `P->f<N>` is really P[N/8] (each element is an 8-byte pointer): argv->f8 -> argv[1]. Then any
    // synthesized `struct S_..` that nothing references anymore is dropped (e.g. S_main_v16 after recovery).
    {
      size_t i=0;
      while(i<lines.size()){ string t=trim(lines[i]);
        bool hdr = !t.empty() && t.back()=='{' && t.find('(')!=string::npos && indentOf(lines[i]).empty()
                   && t.rfind("if",0)!=0 && t.rfind("for",0)!=0 && t.rfind("while",0)!=0 && t.rfind("else",0)!=0 && t.rfind("switch",0)!=0 && t.rfind("struct",0)!=0 && t.rfind("class",0)!=0;
        if(!hdr){ i++; continue; }
        size_t lp=t.find('('); string params; size_t pe; set<string> dbl;
        if(balanced(t,lp,params,pe)) for(auto& p:splitArgs(params)){ string pp=trim(p); if(pp.find("**")!=string::npos){ long e=(long)pp.size()-1; while(e>=0&&!isIdent(pp[e]))e--; long b=e; while(b>=0&&isIdent(pp[b]))b--; if(e>b) dbl.insert(pp.substr(b+1,e-b)); } }   // double-pointer params
        size_t start=i+1, j=start; int depth=1;
        for(; j<lines.size(); j++){ auto m=codeMask(lines[j]); for(size_t k=0;k<lines[j].size();k++){ if(!m[k])continue; if(lines[j][k]=='{')depth++; else if(lines[j][k]=='}')depth--; } if(depth==0)break; }
        size_t end=j;
        if(!dbl.empty()) for(size_t r=start;r<end;r++){ string& L=lines[r];
          for(const string& P:dbl){ string pat=P+"->f"; size_t p=0;
            while((p=L.find(pat,p))!=string::npos){ auto m=codeMask(L); bool ok=p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1]));
              size_t ds=p+pat.size(), de=ds; while(de<L.size()&&isdigit((unsigned char)L[de]))de++;
              if(ok && de>ds && (de>=L.size()||!isIdent(L[de]))){ long off=atol(L.substr(ds,de-ds).c_str()); if(off%8==0){ string rep=P+"["+std::to_string(off/8)+"]"; L=L.substr(0,p)+rep+L.substr(de); p+=rep.size(); continue; } }
              p=ds; } } }
        i=end;
      }
      // drop dead synthesized `struct S_..` (nothing references it). O(n) PER PASS via a global
      // S_-name count + per-struct self-count — was O(structs*lines) with a full re-scan after every
      // removal (tron's 67k lines took ~90s here; this is the "won't decompile" freeze).
      for(int pass=0; pass<12; pass++){
        std::map<string,int> cnt;                                                   // total token occurrences of each S_<name>
        for(auto& L:lines){ auto m=codeMask(L); for(size_t k=0;k<L.size();){
          if(k<m.size()&&m[k]&&L[k]=='S'&&k+1<L.size()&&L[k+1]=='_'&&(k==0||!isIdent(L[k-1]))){
            size_t e=k; while(e<L.size()&&isIdent(L[e]))e++; cnt[L.substr(k,e-k)]++; k=e; } else k++; } }
        vector<std::pair<size_t,size_t>> dead;                                      // [a,b] struct ranges with zero EXTERNAL refs
        for(size_t a=0;a<lines.size();a++){ string t=trim(lines[a]);
          if(t.rfind("struct S_",0)!=0 || t.empty() || t.back()!='{') continue;
          size_t ns=7, ne=ns; while(ne<t.size()&&isIdent(t[ne]))ne++; string nm=t.substr(ns,ne-ns);   // full "S_<name>" (ns=7 = the 'S')
          size_t b=a; int d=0; for(; b<lines.size(); b++){ auto m=codeMask(lines[b]); for(size_t k=0;k<lines[b].size();k++){ if(!m[k])continue; if(lines[b][k]=='{')d++; else if(lines[b][k]=='}')d--; } if(d==0)break; }
          if(b>=lines.size()){ a=lines.size(); continue; }
          int self=0; for(size_t r=a;r<=b;r++){ const string& L=lines[r]; auto m=codeMask(L); size_t p=0;       // refs INSIDE its own def
            while((p=L.find(nm,p))!=string::npos){ if(p<m.size()&&m[p]&&(p==0||!isIdent(L[p-1]))&&(p+nm.size()>=L.size()||!isIdent(L[p+nm.size()])))self++; p+=nm.size(); } }
          if(cnt[nm]-self<=0) dead.push_back({a,b});
          a=b;                                                                       // skip past this struct's body
        }
        if(dead.empty()) break;
        for(size_t i=dead.size();i-->0;) lines.erase(lines.begin()+dead[i].first, lines.begin()+dead[i].second+1);  // erase back-to-front
      }
    }

    foldIostream(lines);   // pass E — rejoin C++ ostream chains: `cout << a << b << endl;`

    fixAddrOfLiteral(lines);  // pass E.5 — repair &<int>/&"str"/&0xPACKED before locals are declared
    dropDeadExprs(lines);  // pass E.6 — drop redundant dead `*(...)` load statements
    recoverInputArray(lines); // pass F0.3 — fgets buffer with constant-offset byte slots -> `char key[N]; key[0]…key[k]`
    declareLocals(lines);  // pass F (G1+G4) — declare every used-but-undeclared local/temp/register
    nameInputBuffers(lines); // pass F0.4 — name fgets/scanf buffers from the preceding prompt string (v56 -> key)
    promoteBuffers(lines); // pass F0.45 — `long buf;` used purely as `&buf`/`buf[i]` -> real `char buf[N]` array, `&buf`->`buf`
    foldArrayAlias(lines); // pass F0.46 — fold `T* p; p = arr;` pointer-alias slop into direct `arr[...]`
    recoverStructArray(lines); // pass F0.47 — `p[i*STRIDE]`/`(p+i*STRIDE)[OFF]` -> `struct StructN* p; p[i].field_OFF`
    foldStdString(lines);  // pass F0.5 — recovered struct String -> real std::string (.size()/.c_str(), type-gated)
    if(!getenv("EMBER_C")){ elideCleanup(lines); elideRaiiSkip(lines); elideCleanupRun(lines); }   // pass F1.2 — drop C++ RAII/EH cleanup (dtor/operator_delete/_Unwind_Resume, the `if(sso)goto L; delete; L:` idiom, and whole destructor LADDERS); implicit in C++, ~3.7k of vortex's gotos
   
    foldDoWhile(lines);    // pass F1.3b — `L: <body> if(c) goto L;`           -> `do { <body> } while (c);`
    fixLatchIncrement(lines); // pass F1.3c — `do { i++; } while ((i+1) != C)` -> `… while (i != C)` (latch compares post-increment)
    foldGuardChain(lines); // pass F1.37 — `if(A){if(B){if(C) goto L;}} <body> L:` -> `if (!((A)&&(B)&&(C))) { <body> }`
    recoverSwitch(lines);  // pass F1.38 — rebuild `switch(V){case…}` from the clang -O0 if(!(V==C0)){chain}else{cases} lowering
    foldReorderedIfElse(lines); // pass F1.39 — clang's hoisted-arm `if(C)goto L; tail; }else{ goto E; L: body }` -> real if/else
    foldIfGoto(lines);     // pass F1.4 — `if (c) goto L; <body> L:` -> `if (!(c)) { <body> }` (de-slop forward-skip gotos)
    if(!getenv("EMBER_C")) dropJumpToNext(lines);   // pass F1.45 — drop `goto L; L:` no-ops (esp. cleanup-ladder gotos redirected to their exit)
    foldDataIndex(lines);  // pass F1.5 — `*(T + (i<<2))` -> `T[i]` for typed data arrays
    stubGlobals(lines);    // pass F2 — stub undefined g_<hex> data globals so it links
    if(!getenv("EMBER_NODV")){
    dvFoldReturnTemp(lines);// pass F2a-1 — `X=E; return X;` -> `return E;` (kills the -O0 return-laundering temp)
    dvDropDeadVars(lines);  // pass F2a-2 — drop write-only `vNNN` slots (the `v104=&vN` pointer-juggling slop)
    dvNameReturnVars(lines);// pass F2a-3 — rename each function's returned `vNNN` -> `ret`
    dvDropUnusedDecls(lines);// pass F2a-4 — drop now-unused local declarations (e.g. the dead `int result;`)
    }
    dropRaiiLoops(lines);  // pass F2b — re-run: catch RAII loops emptied by intermediate folding (not just pass-0b)
    dropEmptyIf(lines);    // pass F2c — drop no-op `if (pure) { }` shells (SSO checks emptied by dead-store elim) + invert empty-then/else
    dropOrphanLabels(lines);// pass F2d — drop `loc_X:`/`L#:` labels nothing jumps to (dead anchors left by folding)
    for(auto& l:lines){ foldAddrArrow(l); reduceParens(l); }   // pass F2e — `(&x)->f` -> `x.f`, then strip precedence-redundant parens (AFTER the structural folds that match parenthesized patterns)
    legalizeOps(lines);    // pass F3 — operator<sym> / residual illegal fn names -> legal C identifiers
    forwardDecls(lines);   // pass G — prototype every function so callers can precede callees
    if(!getenv("EMBER_C")) cppifyHeaders(lines);   // pass H — C++ OUTPUT (default): merge `// <sig>` into a real C++ header. EMBER_C=1 = plain C.
    if(!getenv("EMBER_C")) methodifyStrings(lines);   // pass H.5 — finish STL idioms now params are typed: size(x)->x.size(), fix ./-> for refs vs ptrs
    foldTypedIndex(lines);   // pass H.6 — now params are typed, recover element indexing: `arr[i << 2]` on an `int* arr` -> `arr[i]`
    nameByFormat(lines);     // pass H.7 — name temps from printf/cout labels: `printf("time=%d", v68)` -> v68 becomes `time`
    autoNameFns(lines);      // pass H.8 — OFFLINE behavior-based function naming (sub_XXXX -> readFile/verifyPassword/…), NO AI
    autoNameVars(lines);     // pass H.9 — OFFLINE behavior-based local naming (leftover vN/tN/argN -> buf/i/sum/ptr/node/result), NO AI

    // pass 4 — infer #includes from the library symbols used, so it compiles
    { string all; for(auto& l:lines) all+="\n"+l; all+="\n";
      auto tok=[&](const char* w){ size_t n=strlen(w),p=0; while((p=all.find(w,p))!=string::npos){ bool lb=!isIdent(all[p-1]); bool rb=p+n>=all.size()||!isIdent(all[p+n]); if(lb&&rb)return true; p+=n; } return false; };
      auto has=[&](const char* w){ return all.find(w)!=string::npos; };
      vector<string> inc; auto need=[&](const string& h){ for(auto& x:inc) if(x==h) return; inc.push_back(h); };
      if(tok("printf")||tok("fprintf")||tok("sprintf")||tok("snprintf")||tok("puts")||tok("putchar")||tok("fputs")||tok("fopen")||tok("fwrite")||tok("fread")) need("<cstdio>");
      if(tok("malloc")||tok("free")||tok("calloc")||tok("realloc")||tok("atoi")||tok("atol")||tok("strtol")||tok("exit")||tok("qsort")||tok("abs")||tok("rand")) need("<cstdlib>");
      if(tok("strlen")||tok("strcmp")||tok("strncmp")||tok("strcpy")||tok("strncpy")||tok("strcat")||tok("memcpy")||tok("memset")||tok("memmove")||tok("memcmp")||tok("strchr")) need("<cstring>");
      if(tok("sqrt")||tok("pow")||tok("sin")||tok("cos")||tok("tan")||tok("fabs")||tok("floor")||tok("ceil")||tok("log")||tok("exp")) need("<cmath>");
      if(has("int8_t")||has("int16_t")||has("int32_t")||has("int64_t")||has("uint8_t")||has("uint32_t")||has("uint64_t")||has("size_t")) need("<cstdint>");
      bool cppio = tok("cout")||tok("cerr")||tok("cin")||tok("endl")||has("operator<<")||has("basic_ostream"); if(cppio) need("<iostream>");
      if(has("std::string")||has("basic_string")) need("<string>");
      if(has("std::vector")||has("vector<")) need("<vector>");
      if(has("std::map")||has("std::unordered_map")) need("<map>");
      bool usingStd = tok("cout")||tok("cerr")||tok("cin")||tok("endl")||has("string ")||has("vector");   // bare std names -> `using namespace std;` (always safe; never breaks std:: uses)
      // IDEMPOTENT: drop any preamble the input already has, so re-running Clean Up doesn't stack a 2nd copy.
      { vector<string> keep; for(auto& l:lines){ size_t a=l.find_first_not_of(" \t"); string t=(a==string::npos)?string():l.substr(a);
          if(t.rfind("#include",0)==0 || t=="using namespace std;") continue; keep.push_back(l); } lines.swap(keep);
        while(!lines.empty() && lines.front().find_first_not_of(" \t")==string::npos) lines.erase(lines.begin()); }   // and the blank lines it left
      if(!inc.empty()||usingStd){ vector<string> hdr; for(auto& h:inc) hdr.push_back("#include "+h);
        if(usingStd){ hdr.push_back(""); hdr.push_back("using namespace std;"); } hdr.push_back("");
        lines.insert(lines.begin(), hdr.begin(), hdr.end()); } }

    if(!getenv("EMBER_NOREPAIR")) braceRepair(lines);   // pass I (last) — balance EH-mangled braces so the output parses
    for(auto& l:lines) printf("%s\n", l.c_str());
    return 0;
}
