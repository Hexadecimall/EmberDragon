// nxdis — x86-64 disassembler (listing). Uses the shared structured decoder.
// build:  clang++ -std=c++17 -O2 nxdis.cpp -o nxdis
// use:    nxdis [--raw] <mach-o|file> [baseaddr]
#include "ember.h"
using namespace nx;

int main(int argc, char** argv){
    bool raw=false; const char* path=nullptr; uint64_t base=0;
    for(int i=1;i<argc;i++){ std::string a=argv[i];
        if(a=="--raw") raw=true; else if(!path) path=argv[i]; else base=strtoull(argv[i],0,0); }
    if(!path){ fprintf(stderr,"usage: nxdis [--raw] <file> [baseaddr]\n"); return 2; }
    std::vector<uint8_t> f; if(!readFile(path,f)){ perror("open"); return 1; }
    size_t off=0,size=f.size(); uint64_t vaddr=base;
    if(!raw){
        if(machoText(f.data(),f.size(),off,size,vaddr)) printf("; Mach-O __TEXT,__text  vaddr=0x%llx  size=%zu\n",(unsigned long long)vaddr,size);
        else { fprintf(stderr,"nxdis: not a recognized Mach-O (try --raw)\n"); return 1; }
    }
    size_t o=0;
    while(o<size){
        Ins in=decode(f.data()+off+o, size-o, vaddr+o); if(in.len<=0) in.len=1;
        printf("%016llx:  ",(unsigned long long)in.addr);
        for(int k=0;k<in.len&&k<7;k++) printf("%02x ", f[off+o+k]);
        for(int k=in.len;k<7;k++) printf("   ");
        printf(" %s\n", fmtIns(in).c_str());
        o+=in.len;
    }
    return 0;
}
