// nxdecode.h — structured x86-64 decoder + Mach-O text locator, shared by
// nxdis (listing) and nxlift (pseudo-C). Inverse of ../backend/x64emit.h.
// Each instruction decodes to {mnemonic, operand a, operand b, cc, len} so the
// lifter can reason about it as data, not re-parse text.
#ifndef NXDECODE_H
#define NXDECODE_H
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <utility>

namespace nx {
static const char* R64[16]={"rax","rcx","rdx","rbx","rsp","rbp","rsi","rdi","r8","r9","r10","r11","r12","r13","r14","r15"};
static const char* R32[16]={"eax","ecx","edx","ebx","esp","ebp","esi","edi","r8d","r9d","r10d","r11d","r12d","r13d","r14d","r15d"};

struct Op {
    enum T { NONE, REG, MEM, IMM, REL } t = NONE;
    int  reg = 0; bool w = true;                 // REG: id + 64/32 width
    int  base = 0; int32_t disp = 0; bool rip=false; bool sib=false; int idx=4, scale=0;  // MEM
    int64_t imm = 0;                             // IMM
    uint64_t rel = 0;                            // REL: absolute target
};
struct Ins {
    uint64_t addr = 0; int len = 0;
    std::string mn;      // mnemonic: mov, add, imul, call, jcc, ret, ...
    Op a, b;             // a = first operand (usually dst), b = second
    int cc = 0;          // for jcc/setcc: 'b','a','e','n','l','g' encoded via ccName
    const char* ccs = "";
};

static int32_t rd32(const uint8_t* p, size_t& i){ int32_t v=p[i]|(p[i+1]<<8)|(p[i+2]<<16)|((uint32_t)p[i+3]<<24); i+=4; return v; }
static int64_t rd64(const uint8_t* p, size_t& i){ int64_t v=0; for(int k=0;k<8;k++) v|=(int64_t)p[i+k]<<(8*k); i+=8; return v; }
static const char* ccName(uint8_t op){ switch(op&0x0f){
    case 0x2:return "b"; case 0x3:return "ae"; case 0x4:return "e"; case 0x5:return "ne";
    case 0x6:return "be"; case 0x7:return "a"; case 0xc:return "l"; case 0xd:return "ge";
    case 0xe:return "le"; case 0xf:return "g"; } return "?"; }

// decode the ModRM rm operand into an Op; sets regField (ModRM.reg + REX.R).
static Op rmOperand(const uint8_t* p, size_t& i, bool W, bool R, bool X, bool B, int& regField){
    uint8_t m=p[i++]; int mod=m>>6, reg=(m>>3)&7, rm=m&7; regField = reg|(R?8:0);
    Op o;
    if(mod==3){ o.t=Op::REG; o.reg=rm|(B?8:0); o.w=W; return o; }
    o.t=Op::MEM; o.w=W;
    if(rm==4){                                    // SIB
        uint8_t s=p[i++]; o.sib=true; o.scale=s>>6; o.idx=((s>>3)&7)|(X?8:0); o.base=(s&7)|(B?8:0);
        if((s&7)==5 && mod==0){ o.base=-1; o.disp=rd32(p,i); return o; }
    } else if(mod==0 && rm==5){ o.rip=true; o.disp=rd32(p,i); return o; }
    else o.base = rm|(B?8:0);
    if(mod==1) o.disp=(int8_t)p[i++]; else if(mod==2) o.disp=rd32(p,i);
    return o;
}

static Ins decode(const uint8_t* p, size_t n, uint64_t va){
    Ins in; in.addr=va; size_t i=0; bool W=false,R=false,X=false,B=false;
    while(i<n){ uint8_t c=p[i];                                   // legacy prefixes (operand-size, segment, lock, addr-size)
        if(c==0x66||c==0x2e||c==0x3e||c==0x26||c==0x36||c==0x64||c==0x65||c==0xf0||c==0x67||c==0xf2||c==0xf3) i++; else break; }   // incl REP/REPNE + SSE mandatory prefixes
    while(i<n && (p[i]&0xf0)==0x40){ uint8_t r=p[i++]; W=r&8; R=r&4; X=r&2; B=r&1; }
    if(i>=n){ in.len=(int)i; in.mn=".trunc"; return in; }
    uint8_t op=p[i++];
    auto regOp=[&](int id){ Op o; o.t=Op::REG; o.reg=id; o.w=W; return o; };
    auto rr=[&](const char* mn, bool regDst){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.mn=mn;
        if(regDst){ in.a=regOp(rf); in.b=rm; } else { in.a=rm; in.b=regOp(rf); } };
    auto rr8=[&](const char* mn, bool regDst){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); in.mn=mn;   // 8-bit r/m8,r8 forms
        Op rg; rg.t=Op::REG; rg.reg=rf; rg.w=false; if(regDst){ in.a=rg; in.b=rm; } else { in.a=rm; in.b=rg; } };
    auto shN=[](int rf)->const char*{ return rf==0?"rol":rf==1?"ror":rf==2?"rcl":rf==3?"rcr":rf==4?"shl":rf==5?"shr":"sar"; };
    auto aluN=[](int idx)->const char*{ static const char* A[]={"add","or","adc","sbb","and","sub","xor","cmp"}; return A[idx&7]; };
    auto clOp=[&]()->Op{ Op o; o.t=Op::REG; o.reg=1; o.w=false; return o; };   // cl register (shift count)

    if(op>=0x50&&op<=0x57){ in.mn="push"; in.a=regOp((op-0x50)|(B?8:0)); in.a.w=true; }
    else if(op>=0x58&&op<=0x5f){ in.mn="pop"; in.a=regOp((op-0x58)|(B?8:0)); in.a.w=true; }
    else if(op>=0xb8&&op<=0xbf){ in.mn="mov"; in.a=regOp((op-0xb8)|(B?8:0)); in.b.t=Op::IMM; in.b.imm=W?rd64(p,i):rd32(p,i); }
    else if(op==0x89) rr("mov",false);
    else if(op==0x8b) rr("mov",true);
    else if(op==0x8d) rr("lea",true);
    else if(op==0xc7){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); int32_t v=rd32(p,i); in.mn="mov"; in.a=rm; in.b.t=Op::IMM; in.b.imm=v; }  // mov r/m, imm32
    else if(op==0x01) rr("add",false);
    else if(op==0x03) rr("add",true);                            // reg-dst ALU forms (clang loads then ops)
    else if(op==0x29) rr("sub",false);
    else if(op==0x2b) rr("sub",true);
    else if(op==0x21) rr("and",false);
    else if(op==0x23) rr("and",true);
    else if(op==0x09) rr("or", false);
    else if(op==0x0b) rr("or", true);
    else if(op==0x31) rr("xor",false);
    else if(op==0x33) rr("xor",true);
    else if(op==0x39) rr("cmp",false);
    else if(op==0x3b) rr("cmp",true);
    else if(op==0x85) rr("test",false);
    else if(op>=0x70&&op<=0x7f){ int8_t r=(int8_t)p[i++]; in.mn="jcc"; in.ccs=ccName(op); in.a.t=Op::REL; in.a.rel=va+i+r; }  // jcc rel8 (short)
    else if(op==0x88){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); in.mn="movb"; in.a=rm; in.b=regOp(rf); in.b.w=false; }
    else if(op==0x81||op==0x83){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); int32_t v = op==0x83 ? (int8_t)p[i++] : rd32(p,i); in.a=rm; in.b.t=Op::IMM; in.b.imm=v;
        in.mn = rf==0?"add":rf==5?"sub":rf==7?"cmp":rf==4?"and":rf==1?"or":rf==6?"xor":"grp"; }
    else if(op==0xc0){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); uint8_t v=p[i++]; in.a=rm; in.b.t=Op::IMM; in.b.imm=v; in.mn=shN(rf); }   // shift r/m8, imm8
    else if(op==0xc1){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); uint8_t v=p[i++]; in.a=rm; in.b.t=Op::IMM; in.b.imm=v; in.mn=shN(rf); }       // shift r/m, imm8
    else if(op==0xd0){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); in.a=rm; in.b.t=Op::IMM; in.b.imm=1; in.mn=shN(rf); }                     // shift r/m8, 1
    else if(op==0xd1){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.a=rm; in.b.t=Op::IMM; in.b.imm=1; in.mn=shN(rf); }                         // shift r/m, 1
    else if(op==0xd2){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); in.a=rm; in.b=clOp(); in.mn=shN(rf); }                                    // shift r/m8, cl
    else if(op==0xd3){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.a=rm; in.b=clOp(); in.mn=shN(rf); }                                        // shift r/m, cl
    else if(op==0xf7){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.a=rm; if(rf<=1){ in.b.t=Op::IMM; in.b.imm=rd32(p,i); }   // test r/m,imm32 reads the imm (was a desync bug)
        in.mn=rf==2?"not":rf==3?"neg":rf==7?"idiv":rf==6?"div":rf==5?"imul":rf==4?"mul":rf<=1?"test":"grpF7"; }
    else if(op==0xff){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.a=rm; in.mn=rf==2?"callr":rf==4?"jmpr":rf==6?"push":rf==0?"inc":rf==1?"dec":"grpFF"; }
    else if(op==0x84) rr8("test",false);                                                                       // test r/m8, r8
    else if(op==0xa8){ in.mn="test"; in.a=regOp(0); in.a.w=false; in.b.t=Op::IMM; in.b.imm=(int8_t)p[i++]; }    // test al, imm8
    else if(op==0xa9){ in.mn="test"; in.a=regOp(0); in.b.t=Op::IMM; in.b.imm=rd32(p,i); }                       // test eax/rax, imm32
    else if(op==0x11) rr("adc",false); else if(op==0x13) rr("adc",true);                                        // adc/sbb (carry arith)
    else if(op==0x19) rr("sbb",false); else if(op==0x1b) rr("sbb",true);
    else if(op==0x00) rr8("add",false); else if(op==0x02) rr8("add",true);                                      // 8-bit ALU r/m8,r8 + r8,r/m8
    else if(op==0x10) rr8("adc",false); else if(op==0x12) rr8("adc",true);
    else if(op==0x18) rr8("sbb",false); else if(op==0x1a) rr8("sbb",true);
    else if(op==0x08) rr8("or", false); else if(op==0x0a) rr8("or", true);
    else if(op==0x20) rr8("and",false); else if(op==0x22) rr8("and",true);
    else if(op==0x28) rr8("sub",false); else if(op==0x2a) rr8("sub",true);
    else if(op==0x30) rr8("xor",false); else if(op==0x32) rr8("xor",true);
    else if(op==0x38) rr8("cmp",false); else if(op==0x3a) rr8("cmp",true);
    else if(op==0x80){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); int8_t v=(int8_t)p[i++]; in.a=rm; in.b.t=Op::IMM; in.b.imm=v;
        in.mn=rf==0?"add":rf==5?"sub":rf==7?"cmp":rf==4?"and":rf==1?"or":rf==6?"xor":"grp80"; }                 // ALU r/m8, imm8
    else if(op==0x63){ rr("movsxd",true); }                                                                    // movsxd r64, r/m32 (sign-extend)
    else if(op==0xf6){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); in.a=rm; if(rf<=1){ in.b.t=Op::IMM; in.b.imm=(int8_t)p[i++]; }
        in.mn=rf==2?"not":rf==3?"neg":rf<=1?"test":rf==4?"mul":rf==5?"imul":rf==6?"div":"idiv"; }               // 8-bit grp3 (r/m8)
    else if(op==0x69){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); int32_t v=rd32(p,i); in.mn="imul3"; in.a=regOp(rf); in.b=rm; in.cc=v; }   // imul r, r/m, imm32
    else if(op==0x6b){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); int8_t v=(int8_t)p[i++]; in.mn="imul3"; in.a=regOp(rf); in.b=rm; in.cc=v; } // imul r, r/m, imm8
    else if(op==0xc6){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); int8_t v=(int8_t)p[i++]; in.mn="movb"; in.a=rm; in.b.t=Op::IMM; in.b.imm=v; }   // mov r/m8, imm8
    else if(op>=0xb0&&op<=0xb7){ in.mn="mov"; in.a=regOp((op-0xb0)|(B?8:0)); in.a.w=false; in.b.t=Op::IMM; in.b.imm=(int8_t)p[i++]; }              // mov r8, imm8
    else if((op&0xC7)==0x04){ in.mn=aluN((op>>3)&7); in.a=regOp(0); in.a.w=false; in.b.t=Op::IMM; in.b.imm=(int8_t)p[i++]; }                       // ALU al, imm8
    else if((op&0xC7)==0x05){ in.mn=aluN((op>>3)&7); in.a=regOp(0); in.b.t=Op::IMM; in.b.imm=rd32(p,i); }                                         // ALU eax/rax, imm32
    else if(op==0x98){ in.mn="cdqe"; in.a=regOp(0); }                                                           // cdqe/cwde (sign-extend accumulator)
    else if(op==0xf8){ in.mn="clc"; } else if(op==0xf9){ in.mn="stc"; } else if(op==0xf5){ in.mn="cmc"; }       // flag ops (no value effect)
    else if(op==0x99){ in.mn=W?"cqo":"cdq"; }
    else if(op==0xc3){ in.mn="ret"; }
    else if(op==0xc9){ in.mn="leave"; }
    else if(op==0x90){ in.mn="nop"; }
    else if(op==0xe8){ int32_t r=rd32(p,i); in.mn="call"; in.a.t=Op::REL; in.a.rel=va+i+r; }
    else if(op==0xe9){ int32_t r=rd32(p,i); in.mn="jmp";  in.a.t=Op::REL; in.a.rel=va+i+r; }
    else if(op==0xeb){ int8_t r=(int8_t)p[i++]; in.mn="jmp"; in.a.t=Op::REL; in.a.rel=va+i+r; }
    else if(op==0x0f){ uint8_t o2=p[i++];
        if(o2==0x05) in.mn="syscall";
        else if(o2==0xaf) rr("imul",true);
        else if(o2==0xb6){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.mn="movzxb"; in.a=regOp(rf); in.b=rm; }
        else if(o2==0xb7){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.mn="movzxw"; in.a=regOp(rf); in.b=rm; }
        else if(o2>=0x80&&o2<=0x8f){ int32_t r=rd32(p,i); in.mn="jcc"; in.ccs=ccName(o2); in.a.t=Op::REL; in.a.rel=va+i+r; }
        else if(o2>=0x90&&o2<=0x9f){ int rf; Op rm=rmOperand(p,i,false,R,X,B,rf); in.mn="setcc"; in.ccs=ccName(o2); in.a=rm; }
        else if(o2==0x1f){ int rf; rmOperand(p,i,W,R,X,B,rf); in.mn="nop"; }   // multi-byte NOP (alignment padding)
        else if(o2>=0x40&&o2<=0x4f){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.mn="cmov"; in.ccs=ccName(o2); in.a=regOp(rf); in.b=rm; }   // cmovcc r, r/m
        else if(o2==0xbe){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.mn="movsxb"; in.a=regOp(rf); in.b=rm; in.b.w=false; }   // movsx r, r/m8
        else if(o2==0xbf){ int rf; Op rm=rmOperand(p,i,W,R,X,B,rf); in.mn="movsxw"; in.a=regOp(rf); in.b=rm; in.b.w=false; }   // movsx r, r/m16
        // SSE/MMX (float/vector) — consume the ModRM so the integer stream stays in sync; elided in the lift
        else if((o2>=0x10&&o2<=0x6f)||(o2>=0x74&&o2<=0x76)||(o2>=0xc2&&o2<=0xc6)||(o2>=0xd0)){ int rf; rmOperand(p,i,W,R,X,B,rf); in.mn=".sse"; }
        else { in.mn=".0f"; in.b.t=Op::IMM; in.b.imm=o2; }   // remaining no-modrm 0f ops (cpuid/rdtsc/...)
    }
    else { in.mn=".byte"; in.a.t=Op::IMM; in.a.imm=op; }
    in.len=(int)i; return in;
}

// format a decoded operand to text
static std::string fmtOp(const Op& o){
    char b[96];
    switch(o.t){
        case Op::REG: return (o.w?R64:R32)[o.reg&15];
        case Op::IMM: snprintf(b,sizeof b,"0x%llx",(unsigned long long)o.imm); return b;
        case Op::REL: snprintf(b,sizeof b,"0x%llx",(unsigned long long)o.rel); return b;
        case Op::MEM: {
            std::string s="[";
            if(o.rip) s+="rip";
            else if(o.base<0) s+="";
            else s+=R64[o.base&15];
            if(o.sib && o.idx!=4){ s+="+"; s+=R64[o.idx&15]; s+="*"; s+=std::to_string(1<<o.scale); }
            if(o.disp||o.rip||o.base<0){ char d[24]; snprintf(d,sizeof d,"%+d",o.disp); s+=d; }
            s+="]"; return s;
        }
        default: return "";
    }
}
static std::string fmtIns(const Ins& in){
    std::string s = (in.mn=="jcc"||in.mn=="setcc"||in.mn=="cmov") ? (in.mn=="jcc"?"j":in.mn=="cmov"?"cmov":"set")+std::string(in.ccs) : in.mn;
    if(in.a.t!=Op::NONE){ s+=" "; s+=fmtOp(in.a); }
    if(in.b.t!=Op::NONE){ s+=", "; s+=fmtOp(in.b); }
    return s;
}

// minimal Mach-O 64 reader: locate (__TEXT,__text)
static bool machoText(const uint8_t* f, size_t fsz, size_t& off, size_t& size, uint64_t& vaddr){
    if(fsz<32) return false;
    uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24);
    if(magic!=0xFEEDFACFu) return false;
    uint32_t ncmds=f[16]|(f[17]<<8)|(f[18]<<16)|((uint32_t)f[19]<<24);
    size_t p=32;
    for(uint32_t c=0;c<ncmds && p+8<=fsz;c++){
        uint32_t cmd=f[p]|(f[p+1]<<8)|(f[p+2]<<16)|((uint32_t)f[p+3]<<24);
        uint32_t csz=f[p+4]|(f[p+5]<<8)|(f[p+6]<<16)|((uint32_t)f[p+7]<<24);
        if(cmd==0x19 && p+72<=fsz){
            uint32_t nsects=f[p+64]|(f[p+65]<<8)|(f[p+66]<<16)|((uint32_t)f[p+67]<<24);
            size_t sp=p+72;
            for(uint32_t s=0;s<nsects && s<100000;s++, sp+=80){ if(sp+80>fsz) break;
                char sect[17]={0}; memcpy(sect,&f[sp],16);
                if(!strcmp(sect,"__text")){
                    uint64_t addr=0,sz=0; uint32_t fo;
                    for(int k=0;k<8;k++){ addr|=(uint64_t)f[sp+32+k]<<(8*k); sz|=(uint64_t)f[sp+40+k]<<(8*k); }
                    fo=f[sp+48]|(f[sp+49]<<8)|(f[sp+50]<<16)|((uint32_t)f[sp+51]<<24);
                    off=fo; size=(size_t)sz; vaddr=addr;
                    if(off>fsz) return false; if(off+size>fsz) size=fsz-off; return true;   // clamp to the file
                }
            }
        }
        p+=csz;
    }
    return false;
}
// minimal PE32/PE32+ reader: locate the .text section (so Windows .exe decompile on any host)
static bool peText(const uint8_t* f, size_t fsz, size_t& off, size_t& size, uint64_t& vaddr){
    if(fsz<0x40 || f[0]!='M' || f[1]!='Z') return false;
    uint32_t pe = f[0x3c]|(f[0x3d]<<8)|(f[0x3e]<<16)|((uint32_t)f[0x3f]<<24);
    if((size_t)pe+24>fsz || f[pe]!='P'||f[pe+1]!='E'||f[pe+2]||f[pe+3]) return false;
    uint16_t nsec = f[pe+6]|(f[pe+7]<<8);
    uint16_t optsz = f[pe+20]|(f[pe+21]<<8);
    size_t opt = (size_t)pe+24; if(opt+2>fsz) return false;
    uint16_t magic = f[opt]|(f[opt+1]<<8);          // 0x10b PE32, 0x20b PE32+
    uint64_t imageBase=0;
    if(magic==0x20b){ if(opt+32>fsz) return false; for(int k=0;k<8;k++) imageBase|=(uint64_t)f[opt+24+k]<<(8*k); }
    else { if(opt+32>fsz) return false; imageBase=f[opt+28]|(f[opt+29]<<8)|(f[opt+30]<<16)|((uint32_t)f[opt+31]<<24); }
    size_t sec = opt + optsz;
    for(uint16_t s=0;s<nsec && s<10000;s++){ size_t sp=sec+(size_t)s*40; if(sp+40>fsz) break;
        char nm[9]={0}; memcpy(nm,&f[sp],8);
        uint32_t vsize=f[sp+8]|(f[sp+9]<<8)|(f[sp+10]<<16)|((uint32_t)f[sp+11]<<24);
        uint32_t va   =f[sp+12]|(f[sp+13]<<8)|(f[sp+14]<<16)|((uint32_t)f[sp+15]<<24);
        uint32_t rawsz=f[sp+16]|(f[sp+17]<<8)|(f[sp+18]<<16)|((uint32_t)f[sp+19]<<24);
        uint32_t raw  =f[sp+20]|(f[sp+21]<<8)|(f[sp+22]<<16)|((uint32_t)f[sp+23]<<24);
        if(!strcmp(nm,".text")){ off=raw; size=rawsz?rawsz:vsize; vaddr=imageBase+va;
            if(off>fsz) return false; if(off+size>fsz) size=fsz-off; return true; }   // clamp attacker-controlled raw/size to the file
    }
    return false;
}
// Mach-O symbol table (LC_SYMTAB): map symbol vaddr -> name (strip leading '_').
static void machoSymbols(const uint8_t* f, size_t fsz, std::unordered_map<uint64_t,std::string>& syms){
    if(fsz<32) return; uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24);
    if(magic!=0xFEEDFACFu) return;
    uint32_t ncmds=f[16]|(f[17]<<8)|(f[18]<<16)|((uint32_t)f[19]<<24); size_t p=32;
    for(uint32_t c=0;c<ncmds && p+8<=fsz;c++){
        uint32_t cmd=f[p]|(f[p+1]<<8)|(f[p+2]<<16)|((uint32_t)f[p+3]<<24);
        uint32_t csz=f[p+4]|(f[p+5]<<8)|(f[p+6]<<16)|((uint32_t)f[p+7]<<24);
        if(cmd==0x02){                                  // LC_SYMTAB
            uint32_t symoff=f[p+8]|(f[p+9]<<8)|(f[p+10]<<16)|((uint32_t)f[p+11]<<24);
            uint32_t nsyms =f[p+12]|(f[p+13]<<8)|(f[p+14]<<16)|((uint32_t)f[p+15]<<24);
            uint32_t stroff=f[p+16]|(f[p+17]<<8)|(f[p+18]<<16)|((uint32_t)f[p+19]<<24);
            for(uint32_t s=0;s<nsyms;s++){ size_t e=symoff+(size_t)s*16; if(e+16>fsz) break;
                uint32_t strx=f[e]|(f[e+1]<<8)|(f[e+2]<<16)|((uint32_t)f[e+3]<<24);
                uint64_t val=0; for(int k=0;k<8;k++) val|=(uint64_t)f[e+8+k]<<(8*k);
                if(!val) continue; size_t so=stroff+strx; if(so>=fsz) continue;
                std::string nm((const char*)&f[so], strnlen((const char*)&f[so], fsz-so)); if(!nm.empty()&&nm[0]=='_') nm=nm.substr(1);   // bound the read to EOF
                if(!nm.empty()) syms.emplace(val, nm);
            }
        }
        p+=csz;
    }
}
// Does this Mach-O carry debug info? STABS debug-map entries (N_STAB bit 0xe0 set: N_SO/N_OSO/N_FUN, left by
// `clang -g`) or an embedded __DWARF segment. NB: the LINKED executable keeps the debug MAP + function names, but
// local-variable DWARF lives in the referenced .o/.dSYM — so we can confirm "-g" and use function names, while
// local names still need the .dSYM (a future reader). Returns: 0=none, 1=STABS debug-map, 2=embedded __DWARF.
static int machoDebugKind(const uint8_t* f, size_t fsz){
    if(fsz<32) return 0; uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24); if(magic!=0xFEEDFACFu) return 0;
    uint32_t ncmds=f[16]|(f[17]<<8)|(f[18]<<16)|((uint32_t)f[19]<<24); size_t p=32; int kind=0;
    for(uint32_t c=0;c<ncmds && p+8<=fsz;c++){
        uint32_t cmd=f[p]|(f[p+1]<<8)|(f[p+2]<<16)|((uint32_t)f[p+3]<<24);
        uint32_t csz=f[p+4]|(f[p+5]<<8)|(f[p+6]<<16)|((uint32_t)f[p+7]<<24);
        if(cmd==0x19 && p+24<=fsz){ char sn[17]={0}; for(int k=0;k<16;k++) sn[k]=(char)f[p+8+k]; if(std::string(sn)=="__DWARF") return 2; }   // LC_SEGMENT_64 named __DWARF
        if(cmd==0x02){ uint32_t symoff=f[p+8]|(f[p+9]<<8)|(f[p+10]<<16)|((uint32_t)f[p+11]<<24); uint32_t nsyms=f[p+12]|(f[p+13]<<8)|(f[p+14]<<16)|((uint32_t)f[p+15]<<24);
            for(uint32_t s=0;s<nsyms;s++){ size_t e=(size_t)symoff+(size_t)s*16; if(e+16>fsz) break; uint8_t ntype=f[e+4]; if(ntype&0xe0){ kind=1; break; } } }   // any N_STAB entry = a -g debug map
        p+=csz;
    }
    return kind;
}
// LC_MAIN entry-point: returns the raw `entryoff` (a FILE offset from the image start), or 0 if none.
// The caller maps it to a vaddr via the __text section's (fileoff -> vaddr) base. Lets us recover `main`
// even when the binary is stripped of its `_main` symbol.
static uint64_t machoEntryOff(const uint8_t* f, size_t fsz){
    if(fsz<32) return 0; uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24); if(magic!=0xFEEDFACFu) return 0;
    uint32_t ncmds=f[16]|(f[17]<<8)|(f[18]<<16)|((uint32_t)f[19]<<24); size_t p=32;
    for(uint32_t c=0;c<ncmds && p+8<=fsz;c++){
        uint32_t cmd=f[p]|(f[p+1]<<8)|(f[p+2]<<16)|((uint32_t)f[p+3]<<24);
        uint32_t csz=f[p+4]|(f[p+5]<<8)|(f[p+6]<<16)|((uint32_t)f[p+7]<<24);
        if(cmd==0x80000028u && p+16<=fsz){ uint64_t eo=0; for(int k=0;k<8;k++) eo|=(uint64_t)f[p+8+k]<<(8*k); return eo; }   // LC_MAIN entryoff
        p+=csz;
    }
    return 0;
}
// ── FLIRT signature matching (same masking/hash as ember-sigs, so the lifter can NAME functions by body) ──
static inline int64_t flSext(uint64_t v,int bits){ uint64_t m=1ull<<(bits-1); return (int64_t)((v^m)-m); }
static inline uint32_t flMask(uint32_t w,uint64_t addr,uint64_t fs,uint64_t fe){
    if((w&0xFC000000u)==0x94000000u){ uint64_t t=addr+((uint64_t)(flSext(w&0x3ffffff,26)<<2)); if(t<fs||t>=fe)return w&0xFC000000u; return w; }   // BL
    if((w&0xFC000000u)==0x14000000u){ uint64_t t=addr+((uint64_t)(flSext(w&0x3ffffff,26)<<2)); if(t<fs||t>=fe)return w&0xFC000000u; return w; }   // B
    if((w&0x1F000000u)==0x10000000u) return w&0x9F00001Fu;                     // ADR/ADRP
    if((w&0x3B000000u)==0x18000000u) return w&0xFF0000FFu;                     // LDR(literal)
    return w; }
static inline uint64_t flHash(const uint8_t* code,uint64_t fstart,size_t len){
    uint64_t h=1469598103934665603ull;
    for(size_t o=0;o+4<=len;o+=4){ uint32_t w=code[o]|(code[o+1]<<8)|(code[o+2]<<16)|((uint32_t)code[o+3]<<24);
        w=flMask(w,fstart+o,fstart,fstart+len); for(int b=0;b<4;b++){ h^=(w>>(b*8))&0xff; h*=1099511628211ull; } }
    return h; }
static inline void flLoad(const char* path, std::map<std::pair<uint64_t,uint32_t>,std::string>& m){
    FILE* in=fopen(path,"r"); if(!in)return; char line[4096];
    // LINE-BY-LINE + tolerant: a single malformed line (e.g. a demangled name with a
    // space) must NOT halt the load — the old `while(fscanf(...)==3)` stopped at the
    // first bad line and silently dropped HALF the DB. Read the name as the rest of the
    // line so spaced C++ names survive.
    while(fgets(line,sizeof line,in)){ unsigned long long c; unsigned l; char nm[4000];
        if(sscanf(line,"%llx %u %3999[^\n]",&c,&l,nm)!=3) continue;                         // skip bad line, keep going
        size_t e=strlen(nm); while(e>0 && (nm[e-1]==' '||nm[e-1]=='\r'||nm[e-1]=='\t')) nm[--e]=0;   // trim trailing ws/CR
        if(!e) continue; auto k=std::make_pair((uint64_t)c,(uint32_t)l); auto it=m.find(k);
        if(it==m.end())m[k]=nm; else if(it->second!=nm)it->second="";/*ambiguous*/ }
    fclose(in); }
// all Mach-O sections (for reading .data/.rodata/.cstring -> recovered data/strings)
struct Section { std::string seg, name; uint64_t vaddr=0; size_t fileoff=0, size=0; };
static void machoSections(const uint8_t* f, size_t fsz, std::vector<Section>& out){
    if(fsz<32) return; uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24); if(magic!=0xFEEDFACFu) return;
    uint32_t ncmds=f[16]|(f[17]<<8)|(f[18]<<16)|((uint32_t)f[19]<<24); size_t p=32;
    for(uint32_t c=0;c<ncmds && p+8<=fsz;c++){
        uint32_t cmd=f[p]|(f[p+1]<<8)|(f[p+2]<<16)|((uint32_t)f[p+3]<<24);
        uint32_t csz=f[p+4]|(f[p+5]<<8)|(f[p+6]<<16)|((uint32_t)f[p+7]<<24);
        if(cmd==0x19 && p+72<=fsz){ uint32_t nsects=f[p+64]|(f[p+65]<<8)|(f[p+66]<<16)|((uint32_t)f[p+67]<<24); size_t sp=p+72;
            for(uint32_t s=0;s<nsects && s<100000;s++, sp+=80){ if(sp+80>fsz) break; Section sec; char nm[17]={0},sg[17]={0}; memcpy(nm,&f[sp],16); memcpy(sg,&f[sp+16],16);
                sec.name=nm; sec.seg=sg; for(int k=0;k<8;k++){ sec.vaddr|=(uint64_t)f[sp+32+k]<<(8*k); sec.size|=(uint64_t)f[sp+40+k]<<(8*k); }
                sec.fileoff=f[sp+48]|(f[sp+49]<<8)|(f[sp+50]<<16)|((uint32_t)f[sp+51]<<24); out.push_back(sec); } }
        p+=csz;
    }
}
// Resolve imported-function stubs + GOT/lazy pointers to their symbol names, so a
// call through __stubs (which the lifter sees as sub_<addr>) becomes printf /
// ~basic_string / operator<< etc. Walks LC_SYMTAB (the indexed symbol table,
// incl. undefined imports), LC_DYSYMTAB (the indirect symbol table), and the
// S_SYMBOL_STUBS (0x08) / S_*_SYMBOL_POINTERS (0x06/0x07) sections. Fills
// imports[address] = name for every stub entry and pointer slot.
static void machoStubs(const uint8_t* f, size_t fsz, std::unordered_map<uint64_t,std::string>& imports){
    if(fsz<32) return; uint32_t magic=f[0]|(f[1]<<8)|(f[2]<<16)|((uint32_t)f[3]<<24); if(magic!=0xFEEDFACFu) return;
    auto u32=[&](size_t o)->uint32_t{ return o+4<=fsz ? (f[o]|(f[o+1]<<8)|(f[o+2]<<16)|((uint32_t)f[o+3]<<24)) : 0; };
    auto u64=[&](size_t o)->uint64_t{ uint64_t v=0; for(int k=0;k<8&&o+k<fsz;k++) v|=(uint64_t)f[o+k]<<(8*k); return v; };
    uint32_t ncmds=u32(16); size_t p=32;
    uint32_t symoff=0,stroff=0, indoff=0;
    struct SSec{ uint64_t addr; uint64_t size; uint32_t type, res1, res2; };
    std::vector<SSec> secs;
    for(uint32_t c=0;c<ncmds && p+8<=fsz;c++){
        uint32_t cmd=u32(p), csz=u32(p+4); if(csz==0) break;
        if(cmd==0x02){ symoff=u32(p+8); stroff=u32(p+16); }                    // LC_SYMTAB
        else if(cmd==0x0b){ indoff=u32(p+56); }                                // LC_DYSYMTAB.indirectsymoff
        else if(cmd==0x19){ uint32_t nsects=u32(p+64); size_t sp=p+72;          // LC_SEGMENT_64 sections
            for(uint32_t s=0;s<nsects;s++, sp+=80){ uint32_t type=u32(sp+64)&0xff;
                if(type==0x06||type==0x07||type==0x08){ SSec sc; sc.addr=u64(sp+32); sc.size=u64(sp+40); sc.type=type; sc.res1=u32(sp+68); sc.res2=u32(sp+72); secs.push_back(sc); } } }
        p+=csz;
    }
    if(!symoff||!indoff) return;
    auto symName=[&](uint32_t idx)->std::string{ size_t e=(size_t)symoff+(size_t)idx*16; if(e+4>fsz)return ""; uint32_t strx=u32(e); size_t so=(size_t)stroff+strx; if(so>=fsz)return "";
        std::string nm((const char*)&f[so], strnlen((const char*)&f[so], fsz-so)); if(!nm.empty()&&nm[0]=='_') nm=nm.substr(1); return nm; };   // bound the read to EOF
    auto indSym=[&](uint32_t k)->uint32_t{ return u32((size_t)indoff+(size_t)k*4); };
    for(auto& sc:secs){ uint32_t esz = sc.type==0x08 ? sc.res2 : 8; if(esz==0) continue;     // stub size, else pointer size
        uint32_t count=(uint32_t)(sc.size/esz);
        for(uint32_t i=0;i<count && i<100000;i++){ uint32_t symidx=indSym(sc.res1+i);
            if(symidx & 0xc0000000u) continue;     // INDIRECT_SYMBOL_LOCAL(0x80000000)|ABS(0x40000000) placeholders, possibly OR'd
            std::string nm=symName(symidx); if(!nm.empty()) imports.emplace(sc.addr+(uint64_t)i*esz, nm); } }
}
static bool readFile(const char* path, std::vector<uint8_t>& f){
    FILE* fp=fopen(path,"rb"); if(!fp) return false;
    fseek(fp,0,SEEK_END); long n=ftell(fp); fseek(fp,0,SEEK_SET);
    f.resize(n); bool ok = fread(f.data(),1,n,fp)==(size_t)n; fclose(fp); return ok;
}
// If `f` is a fat/universal Mach-O (most macOS system binaries are x86_64 + arm64e), replace it in place with
// the THIN slice matching wantCpu (CPU_TYPE_ARM64=0x0100000c / CPU_TYPE_X86_64=0x01000007), else the first slice.
// The fat header + fat_arch entries are BIG-ENDIAN on disk. Slice offsets are file-relative -> extract cleanly.
static void machoSelectSlice(std::vector<uint8_t>& f, uint32_t wantCpu){
    if(f.size()<8) return;
    uint32_t magic=((uint32_t)f[0]<<24)|((uint32_t)f[1]<<16)|((uint32_t)f[2]<<8)|f[3];
    bool fat64=(magic==0xcafebabfu), fat32=(magic==0xcafebabeu); if(!fat32&&!fat64) return;
    uint32_t n=((uint32_t)f[4]<<24)|((uint32_t)f[5]<<16)|((uint32_t)f[6]<<8)|f[7];
    auto be32=[&](size_t o){ return o+4<=f.size()?(((uint32_t)f[o]<<24)|((uint32_t)f[o+1]<<16)|((uint32_t)f[o+2]<<8)|f[o+3]):0u; };
    auto be64=[&](size_t o){ uint64_t v=0; for(int k=0;k<8&&o+k<f.size();k++)v=(v<<8)|f[o+k]; return v; };
    size_t entSz=fat64?32:20, p=8; uint64_t bestOff=0,bestSize=0; bool found=false;
    for(uint32_t i=0;i<n && p+entSz<=f.size(); i++,p+=entSz){
        uint32_t cpu=be32(p); uint64_t off= fat64?be64(p+8):be32(p+8), sz= fat64?be64(p+16):be32(p+12);
        if(off==0||sz==0||off+sz>f.size()) continue;
        if(cpu==wantCpu){ bestOff=off; bestSize=sz; found=true; break; }   // exact arch match wins
        if(!found){ bestOff=off; bestSize=sz; found=true; }                // else keep the first valid slice
    }
    if(found&&bestSize>0){ std::vector<uint8_t> slice(f.begin()+bestOff, f.begin()+bestOff+bestSize); f.swap(slice); }
}
} // namespace nx
#endif
