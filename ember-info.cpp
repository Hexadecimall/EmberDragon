// ember-info — static binary intel every RE tool has: Mach-O header, sections (with
// per-section Shannon entropy → packed/encrypted detection), entry point, debug-info
// kind, imports (stubs), defined symbols/functions, and overall entropy. From scratch
// on ember.h's Mach-O readers, no deps.
//   build:  clang++ -std=c++17 -O2 ember-info.cpp -o ember-info
//   use:    ember-info <binary> [--json]
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <unistd.h>
#include "ember.h"
using std::string; using std::vector;
using namespace nx;

// ANSI (auto-off when piped)
static bool TTY=true;
#define C(x) (TTY?x:"")
#define ACC  "\033[38;2;255;138;42m"
#define PINK "\033[38;2;255;92;205m"
#define TEAL "\033[38;2;78;201;176m"
#define DIM  "\033[38;2;120;120;132m"
#define NUM  "\033[38;2;181;206;168m"
#define WARN "\033[38;2;230;120;80m"
#define RST  "\033[0m"
#define B    "\033[1m"

static bool readFileV(const char* p, vector<uint8_t>& v){ FILE* f=fopen(p,"rb"); if(!f)return false; fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET); v.resize(n>0?n:0); if(n>0 && fread(v.data(),1,n,f)!=(size_t)n){ fclose(f); return false; } fclose(f); return true; }

static double entropy(const uint8_t* d, size_t n){ if(!n) return 0; size_t c[256]={0}; for(size_t i=0;i<n;i++) c[d[i]]++;
    double h=0; for(int i=0;i<256;i++) if(c[i]){ double p=(double)c[i]/n; h-=p*std::log2(p); } return h; }

static const char* cpuName(uint32_t c){ switch(c){ case 0x0100000c: return "arm64"; case 0x0100000C|0x80000000: return "arm64e"; case 0x01000007: return "x86-64"; case 7: return "i386"; case 12: return "arm"; default: return "?"; } }
static const char* fileType(uint32_t t){ switch(t){ case 1:return "OBJECT"; case 2:return "EXECUTE"; case 6:return "DYLIB"; case 7:return "DYLINKER"; case 8:return "BUNDLE"; case 5:return "CORE"; case 10:return "DSYM"; default:return "?"; } }

int main(int argc,char** argv){
    TTY=isatty(1);
    string path; bool json=false;
    for(int i=1;i<argc;i++){ string a=argv[i]; if(a=="--json")json=true; else path=a; }
    if(path.empty()){ fprintf(stderr,"usage: ember-info <binary> [--json]\n"); return 2; }
    vector<uint8_t> f; if(!readFileV(path.c_str(),f)){ fprintf(stderr,"ember-info: cannot read %s\n",path.c_str()); return 1; }

    bool wasFat = f.size()>=4 && ((f[0]==0xca&&f[1]==0xfe&&f[2]==0xba&&f[3]==0xbe)||(f[0]==0xbe&&f[1]==0xba&&f[2]==0xfe&&f[3]==0xca));
    if(wasFat){ std::vector<uint8_t> t=f; machoSelectSlice(t,0x0100000c); if(t.size()>=4 && t[0]==0xcf&&t[1]==0xfa){ f=t; } else { t=f; machoSelectSlice(t,0x01000007); f=t; } }
    if(f.size()<32 || !(f[0]==0xcf&&f[1]==0xfa&&f[2]==0xed&&f[3]==0xfe)){ fprintf(stderr,"ember-info: not a 64-bit Mach-O (fat=%d)\n",wasFat); return 1; }

    uint32_t cpu=f[4]|(f[5]<<8)|(f[6]<<16)|((uint32_t)f[7]<<24);
    uint32_t ftype=f[12]|(f[13]<<8)|(f[14]<<16)|((uint32_t)f[15]<<24);
    uint32_t ncmds=f[16]|(f[17]<<8)|(f[18]<<16)|((uint32_t)f[19]<<24);
    uint32_t flags=f[24]|(f[25]<<8)|(f[26]<<16)|((uint32_t)f[27]<<24);
    // UUID + linked-dylib names from the load commands
    string uuid; vector<string> dylibs;
    { size_t p=32; for(uint32_t c=0;c<ncmds && p+8<=f.size();c++){ uint32_t cmd=f[p]|(f[p+1]<<8)|(f[p+2]<<16)|((uint32_t)f[p+3]<<24); uint32_t csz=f[p+4]|(f[p+5]<<8)|(f[p+6]<<16)|((uint32_t)f[p+7]<<24); if(csz<8)break;
        if(cmd==0x1b && p+24<=f.size()){ char b[48]; snprintf(b,sizeof b,"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",f[p+8],f[p+9],f[p+10],f[p+11],f[p+12],f[p+13],f[p+14],f[p+15],f[p+16],f[p+17],f[p+18],f[p+19],f[p+20],f[p+21],f[p+22],f[p+23]); uuid=b; }
        else if((cmd==0xc||cmd==0xd||cmd==0x8000001f) && p+12<=f.size()){ uint32_t noff=f[p+8]|(f[p+9]<<8)|(f[p+10]<<16)|((uint32_t)f[p+11]<<24); if(noff<csz && p+noff<f.size()){ string d((const char*)&f[p+noff], strnlen((const char*)&f[p+noff], csz-noff)); size_t s=d.find_last_of('/'); dylibs.push_back(s==string::npos?d:d.substr(s+1)); } }
        p+=csz; } }

    vector<Section> secs; machoSections(f.data(),f.size(),secs);
    std::unordered_map<uint64_t,string> syms; machoSymbols(f.data(),f.size(),syms);
    std::unordered_map<uint64_t,string> imports; machoStubs(f.data(),f.size(),imports);
    size_t toff,tsz; uint64_t tva=0; bool hasText=machoText(f.data(),f.size(),toff,tsz,tva);
    uint64_t eoff=machoEntryOff(f.data(),f.size()); uint64_t entryVa = (eoff&&hasText)? tva+eoff-toff : 0;
    int dk=machoDebugKind(f.data(),f.size());
    double fileEnt=entropy(f.data(),f.size());

    if(json){
        printf("{\n  \"file\": \"%s\",\n  \"arch\": \"%s\",\n  \"type\": \"%s\",\n  \"bytes\": %zu,\n  \"entropy\": %.3f,\n  \"uuid\": \"%s\",\n  \"entry\": \"0x%llx\",\n  \"symbols\": %zu,\n  \"imports\": %zu,\n  \"sections\": [\n",
            path.c_str(),cpuName(cpu),fileType(ftype),f.size(),fileEnt,uuid.c_str(),(unsigned long long)entryVa,syms.size(),imports.size());
        for(size_t i=0;i<secs.size();i++){ auto& s=secs[i]; double e=(s.fileoff+s.size<=f.size())?entropy(f.data()+s.fileoff,s.size):0;
            printf("    {\"seg\":\"%s\",\"name\":\"%s\",\"vaddr\":\"0x%llx\",\"size\":%zu,\"entropy\":%.3f}%s\n",s.seg.c_str(),s.name.c_str(),(unsigned long long)s.vaddr,s.size,e,i+1<secs.size()?",":""); }
        printf("  ]\n}\n"); return 0;
    }

    printf("%s%s🔥🐉 ember-info%s  %s%s%s\n",C(ACC),C(B),C(RST),C(B),path.c_str(),C(RST));
    printf("%s──────────────────────────────────────────────────────────────%s\n",C(DIM),C(RST));
    printf("  %sarch%s      %s%s%s   %stype%s %s   %sload cmds%s %u   %ssize%s %s%zu%s bytes\n",
        C(DIM),C(RST),C(TEAL),cpuName(cpu),C(RST),C(DIM),C(RST),fileType(ftype),C(DIM),C(RST),ncmds,C(DIM),C(RST),C(NUM),f.size(),C(RST));
    if(!uuid.empty()) printf("  %suuid%s      %s\n",C(DIM),C(RST),uuid.c_str());
    printf("  %sflags%s     0x%x%s%s%s\n",C(DIM),C(RST),flags,(flags&0x200000)?" PIE":"",(flags&0x80)?" TWOLEVEL":"",(flags&0x1)?" NOUNDEFS":"");
    if(entryVa) printf("  %sentry%s     %s0x%llx%s\n",C(DIM),C(RST),C(ACC),(unsigned long long)entryVa,C(RST));
    printf("  %sdebug%s     %s\n",C(DIM),C(RST), dk==2?"embedded DWARF":dk==1?"-g debug map":"none (names by behavior + FLIRT)");
    printf("  %sentropy%s   %s%.3f%s / 8.0  %s%s\n",C(DIM),C(RST),fileEnt>7.2?C(WARN):C(NUM),fileEnt,C(RST),C(DIM),fileEnt>7.2?"⚠ likely packed/encrypted":fileEnt>6.0?"(compressed-ish)":"(normal code/data)");
    printf("  %ssymbols%s   %s%zu%s defined   %simports%s %s%zu%s\n",C(DIM),C(RST),C(NUM),syms.size(),C(RST),C(DIM),C(RST),C(NUM),imports.size(),C(RST));

    // sections + per-section entropy
    printf("\n%s%sSECTIONS%s  %s(entropy flags packed regions)%s\n",C(ACC),C(B),C(RST),C(DIM),C(RST));
    printf("  %s%-10s %-16s %-12s %10s  %s%s\n",C(DIM),"SEGMENT","SECTION","VADDR","SIZE","ENTROPY",C(RST));
    for(auto& s:secs){ if(s.size==0) continue; double e=(s.fileoff+s.size<=f.size())?entropy(f.data()+s.fileoff,s.size):0;
        const char* ec = e>7.2?WARN:e>6.0?NUM:DIM;
        printf("  %-10s %-16s %s0x%-10llx%s %10zu  %s%.3f%s%s\n",s.seg.c_str(),s.name.c_str(),C(DIM),(unsigned long long)s.vaddr,C(RST),s.size,C(ec),e,C(RST),(e>7.2&&s.fileoff+s.size<=f.size())?"  ⚠":""); }

    // dylib dependencies
    if(!dylibs.empty()){ printf("\n%s%sLINKED LIBRARIES%s (%zu)\n",C(TEAL),C(B),C(RST),dylibs.size());
        for(auto& d:dylibs) printf("  %s%s%s\n",C(DIM),d.c_str(),C(RST)); }

    // imports (stubs)
    if(!imports.empty()){ vector<string> names; for(auto& kv:imports) names.push_back(kv.second); std::sort(names.begin(),names.end()); names.erase(std::unique(names.begin(),names.end()),names.end());
        printf("\n%s%sIMPORTS%s (%zu)\n",C(PINK),C(B),C(RST),names.size());
        int col=0; printf("  "); for(auto& n:names){ printf("%s%-22s%s",C(DIM),n.c_str(),C(RST)); if(++col%3==0){printf("\n  ");} } if(col%3)printf("\n"); }

    // defined functions/symbols (sorted by addr)
    if(!syms.empty()){ vector<std::pair<uint64_t,string>> fns(syms.begin(),syms.end()); std::sort(fns.begin(),fns.end());
        printf("\n%s%sDEFINED SYMBOLS%s (%zu, sorted by address)\n",C(TEAL),C(B),C(RST),fns.size());
        int shown=0; for(auto& kv:fns){ if(shown++>=40){ printf("  %s… +%zu more%s\n",C(DIM),fns.size()-40,C(RST)); break; }
            printf("  %s0x%-10llx%s %s\n",C(DIM),(unsigned long long)kv.first,C(RST),kv.second.c_str()); } }
    return 0;
}
