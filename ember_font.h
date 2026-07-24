// ember_font.h — crisp anti-aliased monospace text for the EmberDragon GUI.
// Bakes a real system code font (SF Mono / Consolas / Menlo) into a GL texture
// atlas via stb_truetype (oversampled → sharp at any size), draws textured quads.
// Falls back to the 8x8 bitmap font only if no TTF can be loaded, so it never
// breaks. Single-header; each platform backend is its own translation unit, so
// defining STB_TRUETYPE_IMPLEMENTATION here is safe (one impl per executable).
#ifndef EMBER_FONT_H
#define EMBER_FONT_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#if defined(_WIN32)
  #include <windows.h>
  #include <GL/gl.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F          // GL 1.2 token; mingw's gl.h is 1.1
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include "corpus/code_web/stb_truetype.h"

namespace ef {
static std::vector<unsigned char> g_ttf;        // the loaded font file bytes
static stbtt_fontinfo g_info;                   // parsed font
static bool   g_loaded=false, g_baked=false;
static GLuint g_tex=0;
static int    g_aw=1024, g_ah=1024;
static stbtt_packedchar g_pc[96];               // ASCII 32..127
static float  g_px=0, g_adv=0, g_asc=0, g_lh=0;

// Load the first readable TTF from a candidate list (no GL needed yet).
inline bool load(const std::vector<std::string>& paths){
    for(const auto& p : paths){
        FILE* f=fopen(p.c_str(),"rb"); if(!f) continue;
        fseek(f,0,SEEK_END); long n=ftell(f); fseek(f,0,SEEK_SET);
        if(n<=0){ fclose(f); continue; }
        std::vector<unsigned char> buf(n);
        size_t rd=fread(buf.data(),1,n,f); fclose(f);
        if((long)rd!=n) continue;
        int off=stbtt_GetFontOffsetForIndex(buf.data(),0);   // works for .ttf and .ttc face 0
        if(off<0) continue;
        if(!stbtt_InitFont(&g_info, buf.data(), off)) continue;
        g_ttf.swap(buf); g_loaded=true; return true;
    }
    return false;
}

// (Re)bake the atlas at a given pixel height. Needs a current GL context.
inline bool bake(float pixelHeight){
    if(!g_loaded) return false;
    if(pixelHeight<8) pixelHeight=8; if(pixelHeight>96) pixelHeight=96;
    std::vector<unsigned char> a(g_aw*g_ah, 0);
    stbtt_pack_context pc;
    if(!stbtt_PackBegin(&pc, a.data(), g_aw, g_ah, 0, 1, nullptr)) return false;
    stbtt_PackSetOversampling(&pc, 2, 2);                    // sharp small text
    int ok = stbtt_PackFontRange(&pc, g_ttf.data(), 0, pixelHeight, 32, 95, g_pc);
    stbtt_PackEnd(&pc);
    if(!ok){ /* some glyphs may not have fit, but keep what we have */ }
    // expand single-channel coverage → RGBA (white, alpha=coverage) for reliable
    // GL_MODULATE coloring with glColor3f.
    std::vector<unsigned char> rgba(g_aw*g_ah*4);
    for(int i=0;i<g_aw*g_ah;i++){ rgba[i*4+0]=255; rgba[i*4+1]=255; rgba[i*4+2]=255; rgba[i*4+3]=a[i]; }
    if(!g_tex) glGenTextures(1,&g_tex);
    glBindTexture(GL_TEXTURE_2D, g_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_aw, g_ah, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
    // metrics
    int asc,desc,gap; stbtt_GetFontVMetrics(&g_info,&asc,&desc,&gap);
    float sc=stbtt_ScaleForPixelHeight(&g_info, pixelHeight);
    g_asc=asc*sc; g_lh=(asc-desc+gap)*sc;
    g_adv=g_pc['0'-32].xadvance;                            // monospace advance
    g_px=pixelHeight; g_baked=true;
    return true;
}

inline bool  ready(){ return g_baked; }
inline float advance(){ return g_adv; }
inline float lineHeight(){ return g_lh; }
inline float ascent(){ return g_asc; }
inline float width(const std::string& s){ return s.size()*g_adv; }

// Draw text with top-left at (x,y); returns the pen x after the string.
// ── batched drawing (one GL begin/end + state setup for many strings) ────────
// Hot panels call begin(), emit() many times, end() — collapsing hundreds of
// per-token draw calls into a single batch (the big win for editor/scroll perf).
inline void begin() { if(!g_baked) return; glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, g_tex); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glBegin(GL_QUADS); }
inline void end()   { if(!g_baked) return; glEnd(); glDisable(GL_TEXTURE_2D); glDisable(GL_BLEND); }
inline float emit(float x, float y, const std::string& s, float r, float g, float b) {   // assumes a batch is open
    if(!g_baked) return x;
    glColor3f(r,g,b); float xpos=x, ypos=y+g_asc;
    for(unsigned char ch : s){ if(ch<32||ch>126){ xpos+=g_adv; continue; }
        stbtt_aligned_quad q; stbtt_GetPackedQuad(g_pc, g_aw, g_ah, ch-32, &xpos, &ypos, &q, 1);
        glTexCoord2f(q.s0,q.t0); glVertex2f(q.x0,q.y0); glTexCoord2f(q.s1,q.t0); glVertex2f(q.x1,q.y0);
        glTexCoord2f(q.s1,q.t1); glVertex2f(q.x1,q.y1); glTexCoord2f(q.s0,q.t1); glVertex2f(q.x0,q.y1); }
    return xpos;
}
inline float draw(float x,float y,const std::string& s,float r,float g,float b){ if(!g_baked) return x; begin(); float e=emit(x,y,s,r,g,b); end(); return e; }
} // namespace ef
#endif
