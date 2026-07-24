// ember_ui.h — a tiny immediate-mode UI toolkit over legacy OpenGL: input state,
// draw primitives, clip stack, vector icons, and widgets (buttons / tabs /
// scrollbars / context menus). Event-driven: the platform sets ui::in for each
// event and calls the app's render; one-shot fields are cleared after the frame.
#ifndef EMBER_UI_H
#define EMBER_UI_H
#include "ember_theme.h"
#include "ember_font.h"
#include <string>
#include <vector>
#include <functional>
#include <cmath>

namespace ui {
using th::Col;
using std::string;

// ── input ────────────────────────────────────────────────────────────────────
enum Key { K_NONE, K_UP, K_DOWN, K_LEFT, K_RIGHT, K_PGUP, K_PGDN, K_HOME, K_END, K_ENTER, K_ESC, K_BACK, K_TAB, K_SLASH };
struct Input {
    float mx = -1, my = -1;             // mouse position (device px)
    bool  lDown = false, rDown = false; // buttons held
    bool  lPress = false, rPress = false, lRelease = false; // one-shot
    bool  dragging = false;             // left held and moved
    float pressX = 0, pressY = 0;       // left-press origin
    float wheel = 0;                    // one-shot vertical scroll (lines, +down)
    float wheelX = 0;                   // one-shot HORIZONTAL scroll (chars, +right) — trackpad swipe / shift+wheel
    int   key = K_NONE; bool shift = false, ctrl = false; // one-shot key
    unsigned ch = 0;                    // one-shot typed char
    float scale = 1.0f;                 // backing/DPI scale
    void clearOneShot() { lPress = rPress = lRelease = false; wheel = 0; wheelX = 0; key = K_NONE; ch = 0; }
};
inline Input in;
inline float winW = 1280, winH = 800;   // framebuffer size (device px), set per frame
inline bool menuActive = false;          // a context menu is open -> swallow clicks to widgets beneath it

inline bool inside(float x, float y, float w, float h) { return in.mx >= x && in.mx < x + w && in.my >= y && in.my < y + h; }
inline bool hovered(float x, float y, float w, float h) { return inside(x, y, w, h); }
inline bool clicked(float x, float y, float w, float h) { return in.lPress && !menuActive && inside(x, y, w, h); }
inline bool rclicked(float x, float y, float w, float h) { return in.rPress && !menuActive && inside(x, y, w, h); }

// ── primitives ───────────────────────────────────────────────────────────────
inline void rect(float x, float y, float w, float h, Col c) {
    glDisable(GL_TEXTURE_2D);
    if (c.a < 1) { glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
    glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_QUADS); glVertex2f(x, y); glVertex2f(x + w, y); glVertex2f(x + w, y + h); glVertex2f(x, y + h); glEnd();
    if (c.a < 1) glDisable(GL_BLEND);
}
inline void rectLine(float x, float y, float w, float h, Col c) {
    glDisable(GL_TEXTURE_2D); glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_LINE_LOOP); glVertex2f(x + .5f, y + .5f); glVertex2f(x + w - .5f, y + .5f); glVertex2f(x + w - .5f, y + h - .5f); glVertex2f(x + .5f, y + h - .5f); glEnd();
}
inline void hline(float x, float y, float w, Col c) { rect(x, y, w, in.scale, c); }
inline void vline(float x, float y, float h, Col c) { rect(x, y, in.scale, h, c); }
inline void tri(float ax, float ay, float bx, float by, float cx, float cy, Col c) {
    glDisable(GL_TEXTURE_2D); glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLES); glVertex2f(ax, ay); glVertex2f(bx, by); glVertex2f(cx, cy); glEnd();
}
inline void disc(float cx, float cy, float r, Col c) {
    glDisable(GL_TEXTURE_2D); glColor4f(c.r, c.g, c.b, c.a);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx, cy);
    for (int i = 0; i <= 24; i++) { float a = i * 6.2831853f / 24; glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r); } glEnd();
}
inline void ring(float cx, float cy, float r, float t, Col c) {
    glDisable(GL_TEXTURE_2D); glColor4f(c.r, c.g, c.b, c.a); glLineWidth(t);
    glBegin(GL_LINE_LOOP); for (int i = 0; i < 28; i++) { float a = i * 6.2831853f / 28; glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r); } glEnd();
    glLineWidth(1);
}
inline void line(float x0, float y0, float x1, float y1, float t, Col c) {
    glDisable(GL_TEXTURE_2D); glColor4f(c.r, c.g, c.b, c.a); glLineWidth(t);
    glBegin(GL_LINES); glVertex2f(x0, y0); glVertex2f(x1, y1); glEnd(); glLineWidth(1);
}

// ── text (real AA font via ef, with a bitmap fallback inside ef) ─────────────
inline float advance() { return ef::ready() ? ef::advance() : 8 * in.scale; }
inline float lineH()   { return (ef::ready() ? ef::lineHeight() : 8 * in.scale + 4) * 1.18f; }
inline float textW(const string& s) { return ef::ready() ? ef::width(s) : s.size() * 8 * in.scale; }
inline float text(float x, float y, const string& s, Col c) { return ef::draw(x, y, s, c.r, c.g, c.b); }
inline void  textRight(float xr, float y, const string& s, Col c) { text(xr - textW(s), y, s, c); }
// batched text: bracket many emit() calls with beginText()/endText() to collapse them into one GL draw
inline void  beginText() { ef::begin(); }
inline void  endText()   { ef::end(); }
inline float emit(float x, float y, const string& s, Col c) { return ef::emit(x, y, s, c.r, c.g, c.b); }
inline void  emitRight(float xr, float y, const string& s, Col c) { ef::emit(xr - textW(s), y, s, c.r, c.g, c.b); }
inline void  textClip(float x, float y, float maxw, const string& s, Col c) {
    string t = s; float a = advance(); int max = a > 0 ? int(maxw / a) : (int)s.size();
    if ((int)t.size() > max && max > 1) { int keep = max > 2 ? max - 2 : 1; t = t.substr(0, keep) + ".."; }   // ".." reads as truncation; the font atlas has no "…" glyph
    text(x, y, t, c);
}

// ── clip stack (glScissor; convert our y-down coords to GL y-up) ─────────────
inline std::vector<float> clipStack;   // groups of 4: x,y,w,h
inline void pushClip(float x, float y, float w, float h) {
    if (w < 0) w = 0; if (h < 0) h = 0;                 // glScissor rejects negative dims (tiny windows)
    glEnable(GL_SCISSOR_TEST);
    glScissor((int)x, (int)(winH - (y + h)), (int)w, (int)h);
    clipStack.insert(clipStack.end(), { x, y, w, h });
}
inline void popClip() {
    clipStack.resize(clipStack.size() >= 4 ? clipStack.size() - 4 : 0);
    if (clipStack.size() >= 4) { size_t n = clipStack.size();
        glScissor((int)clipStack[n-4], (int)(winH - (clipStack[n-3] + clipStack[n-1])), (int)clipStack[n-2], (int)clipStack[n-1]);
    } else glDisable(GL_SCISSOR_TEST);
}

// ── vector icons (id-switched bold shapes) ───────────────────────────────────
enum Icon { IC_OPEN, IC_RELOAD, IC_EXPORT, IC_SEARCH, IC_FUNC, IC_HEX, IC_ASM, IC_CODE, IC_AI, IC_LOG, IC_SYM, IC_STR, IC_XREF, IC_CLOSE, IC_DRAGON, IC_FILE, IC_PLUS, IC_HOME, IC_SAVE, IC_TERM, IC_COLLAPSE, IC_BUILD, IC_GRAPH, IC_BUG, IC_SETTINGS, IC_MENU, IC_RUN };
inline void icon(float x, float y, float s, Icon id, Col c) {
    float u = s / 16.0f;   // unit; icons designed on a 16-grid
    auto X = [&](float v) { return x + v * u; }; auto Y = [&](float v) { return y + v * u; };
    switch (id) {
        case IC_OPEN: rect(X(2),Y(4),6*u,2.4f*u,c); rect(X(2),Y(5.6f),12*u,7.4f*u,c); rect(X(3),Y(7),10*u,5.4f*u,th::PANEL); break;   // folder (body + tab + inner cut)
        case IC_FILE: rect(X(3),Y(2),Y(13)-Y(2),Y(14)-Y(2),c); rect(X(4),Y(3),Y(12)-Y(4),Y(13)-Y(3),th::PANEL); break;
        case IC_RELOAD: ring(X(8),Y(8),5*u,2*u,c); tri(X(11),Y(2),X(14),Y(5),X(11),Y(7),c); break;
        case IC_EXPORT: line(X(8),Y(2),X(8),Y(9),2*u,c); tri(X(5),Y(7),X(11),Y(7),X(8),Y(11),c); rect(X(3),Y(12),Y(13)-Y(3),2*u,c); break;
        case IC_SEARCH: ring(X(7),Y(7),4*u,2*u,c); line(X(10),Y(10),X(14),Y(14),2*u,c); break;
        case IC_FUNC: line(X(6),Y(4),X(6),Y(13),2*u,c); line(X(5),Y(8),X(9),Y(8),2*u,c); tri(X(6),Y(3),X(9),Y(3),X(9),Y(5),c); break;
        case IC_HEX: for(int r=0;r<2;r++)for(int col=0;col<3;col++) rectLine(X(2+col*4),Y(3+r*5),3.2f*u,4*u,c); break;
        case IC_ASM: rectLine(X(2),Y(3),12*u,10*u,c); for(int i=0;i<3;i++) line(X(4),Y(5+i*3),X(11),Y(5+i*3),1.4f*u,c); break;
        case IC_CODE: tri(X(5),Y(4),X(2),Y(8),X(5),Y(12),c); tri(X(11),Y(4),X(14),Y(8),X(11),Y(12),c); break;
        case IC_AI: { float cx=X(8),cy=Y(8); tri(cx,Y(1.5f),X(6.4f),cy,X(9.6f),cy,c); tri(cx,Y(14.5f),X(6.4f),cy,X(9.6f),cy,c);   // 4-point sparkle
            tri(X(1.5f),cy,cx,Y(6.4f),cx,Y(9.6f),c); tri(X(14.5f),cy,cx,Y(6.4f),cx,Y(9.6f),c);
            disc(X(13),Y(3.5f),1.1f*u,c); } break;
        case IC_LOG: for(int i=0;i<4;i++){ disc(X(3),Y(3.5f+i*3),0.9f*u,c); line(X(5),Y(3.5f+i*3),X(13),Y(3.5f+i*3),1.4f*u,c);} break;
        case IC_SYM: disc(X(8),Y(8),3.2f*u,c); ring(X(8),Y(8),5*u,1.4f*u,c); break;
        case IC_STR: text(X(2),Y(2),"\"\"",c); break;
        case IC_XREF: line(X(3),Y(8),X(13),Y(8),2*u,c); tri(X(10),Y(4),X(14),Y(8),X(10),Y(12),c); break;
        case IC_CLOSE: line(X(4),Y(4),X(12),Y(12),2*u,c); line(X(12),Y(4),X(4),Y(12),2*u,c); break;
        case IC_PLUS: line(X(8),Y(3),X(8),Y(13),2*u,c); line(X(3),Y(8),X(13),Y(8),2*u,c); break;
        case IC_DRAGON: disc(X(8),Y(8),5*u,c); tri(X(8),Y(1),X(5),Y(5),X(11),Y(5),th::ACCENT2); break;
        case IC_HOME: tri(X(8),Y(2),X(2),Y(8),X(14),Y(8),c); rect(X(4),Y(8),8*u,6*u,c); rect(X(7),Y(10),3*u,4*u,th::PANEL); break;
        case IC_SAVE: rectLine(X(3),Y(3),10*u,10*u,c); rect(X(5),Y(3),5*u,4*u,c); rect(X(5),Y(9),6*u,3*u,c); break;
        case IC_TERM: rectLine(X(2),Y(3),12*u,10*u,c); tri(X(4),Y(6),X(7),Y(8),X(4),Y(10),c); line(X(8),Y(10),X(11),Y(10),1.4f*u,c); break;
        case IC_COLLAPSE: tri(X(2),Y(3),X(2),Y(13),X(6.5f),Y(8),c); tri(X(14),Y(3),X(14),Y(13),X(9.5f),Y(8),c); line(X(8),Y(2),X(8),Y(14),1.6f*u,c); break;   // > | <  collapse inward
        case IC_BUILD: line(X(4),Y(12.5f),X(9.5f),Y(7),2.4f*u,c); line(X(7.5f),Y(3),X(13.5f),Y(7.5f),3.4f*u,c); break;                          // hammer: handle + head
        case IC_GRAPH: rectLine(X(5.5f),Y(2),5*u,3.4f*u,c); rectLine(X(2),Y(10.5f),4.6f*u,3.4f*u,c); rectLine(X(9.4f),Y(10.5f),4.6f*u,3.4f*u,c);  // CFG: a node with two children
            line(X(8),Y(5.4f),X(4.3f),Y(10.5f),1.3f*u,c); line(X(8),Y(5.4f),X(11.7f),Y(10.5f),1.3f*u,c); break;
        case IC_BUG: disc(X(8),Y(8.5f),3*u,c); line(X(3),Y(6),X(5.5f),Y(7.5f),1.3f*u,c); line(X(13),Y(6),X(10.5f),Y(7.5f),1.3f*u,c);   // a bug (debugger)
            line(X(3),Y(11),X(5.5f),Y(9.5f),1.3f*u,c); line(X(13),Y(11),X(10.5f),Y(9.5f),1.3f*u,c); line(X(6.5f),Y(3),X(7.2f),Y(5.5f),1.1f*u,c); line(X(9.5f),Y(3),X(8.8f),Y(5.5f),1.1f*u,c); break;
        case IC_SETTINGS: { float cx=X(8),cy=Y(8),r=4.2f*u;                                          // gear: 8 teeth + ring + hub
            for(int i=0;i<8;i++){ float a=i*0.7853982f; line(cx+std::cos(a)*r,cy+std::sin(a)*r,cx+std::cos(a)*(r+2.4f*u),cy+std::sin(a)*(r+2.4f*u),2.4f*u,c); }
            ring(cx,cy,r,2*u,c); disc(cx,cy,1.5f*u,c); } break;
        case IC_MENU: for(int i=0;i<3;i++) line(X(3),Y(4.5f+i*3.5f),X(13),Y(4.5f+i*3.5f),1.6f*u,c); break;   // hamburger (command palette)
        case IC_RUN: tri(X(4),Y(3),X(4),Y(13),X(13),Y(8),c); break;   // play triangle (EmberRun)
    }
}

// ── tooltips (deferred: a hovered widget records text; drawTooltip() paints it last) ──
inline string g_tip; inline float g_tipX = 0, g_tipY = 0;
inline void setTip(float x, float y, const string& t) { g_tip = t; g_tipX = x; g_tipY = y; }
inline void drawTooltip() {
    if (g_tip.empty()) return; float s = in.scale, pad = 6 * s, tw = textW(g_tip) + pad * 2, th0 = lineH() + pad;
    float x = g_tipX, y = g_tipY; if (x + tw > winW) x = winW - tw - 2 * s; if (y + th0 > winH) y = g_tipY - th0 - 8 * s;
    rect(x, y, tw, th0, th::Col{0.12f, 0.12f, 0.15f, 0.97f}); rectLine(x, y, tw, th0, th::ACCENT);
    text(x + pad, y + pad / 2, g_tip, th::TEXT);
    g_tip.clear();
}

// ── widgets ──────────────────────────────────────────────────────────────────
// toolbar icon button; returns true on click. Pass a tip for a hover tooltip.
inline bool iconButton(float x, float y, float s, Icon id, bool active = false, const char* tip = nullptr) {
    bool hov = hovered(x, y, s, s);
    if (active) rect(x, y, s, s, th::SEL_DIM);
    else if (hov) rect(x, y, s, s, th::HOVER);
    icon(x + s * 0.18f, y + s * 0.18f, s * 0.64f, id, active ? th::ACCENT : hov ? th::TEXT : th::TEXT_DIM);
    if (hov && tip) setTip(x, y + s + 4 * in.scale, tip);
    return clicked(x, y, s, s);
}
// text button; primary = accent fill
inline bool button(float x, float y, float w, float h, const string& label, bool primary = false, bool enabled = true) {
    bool hov = enabled && hovered(x, y, w, h);
    Col bg = !enabled ? th::Col{0.16f,0.16f,0.18f} : primary ? th::ACCENT : (hov ? th::HOVER : th::PANEL_HI);
    rect(x, y, w, h, bg); rectLine(x, y, w, h, !enabled ? th::Col{0.22f,0.22f,0.25f} : th::BORDER);
    Col tc = !enabled ? th::Col{0.42f,0.43f,0.47f} : primary ? th::Col{0.07f, 0.06f, 0.04f} : (hov ? th::TEXT : th::TEXT_DIM);
    text(x + (w - textW(label)) / 2, y + (h - lineH()) / 2 + lineH() * 0.08f, label, tc);
    return enabled && clicked(x, y, w, h);
}
// tab in a strip; returns true on click
inline bool tab(float x, float y, float w, float h, Icon id, const string& label, bool active) {
    bool hov = hovered(x, y, w, h);
    rect(x, y, w, h, active ? th::PANEL : hov ? th::HOVER : th::TOOLBAR);
    if (active) rect(x, y, w, 2 * in.scale, th::ACCENT);
    float ix = x + th::PAD * in.scale, iy = y + (h - h * 0.5f) / 2;
    icon(ix, iy, h * 0.5f, id, active ? th::ACCENT : th::TEXT_DIM);
    text(ix + h * 0.5f + 4 * in.scale, y + (h - lineH()) / 2, label, active ? th::TEXT : th::TEXT_DIM);
    return clicked(x, y, w, h);
}
// closable tab: draws an × on the right. Sets *closed=true when the × is clicked
// (and swallows that click); returns true on a body click.
inline bool tabX(float x, float y, float w, float h, Icon id, const string& label, bool active, bool* closed) {
    bool hov = hovered(x, y, w, h); float s = in.scale;
    rect(x, y, w, h, active ? th::PANEL : hov ? th::HOVER : th::TOOLBAR);
    if (active) rect(x, y, w, 2 * s, th::ACCENT);
    float ix = x + th::PAD * s, iy = y + (h - h * 0.5f) / 2;
    icon(ix, iy, h * 0.5f, id, active ? th::ACCENT : th::TEXT_DIM);
    float xb = h * 0.4f, xx = x + w - xb - 6 * s, xy = y + (h - xb) / 2;
    text(ix + h * 0.5f + 4 * s, y + (h - lineH()) / 2, label, active ? th::TEXT : th::TEXT_DIM);
    bool xhov = hovered(xx, xy, xb, xb);
    if (xhov) rect(xx - 2 * s, xy - 2 * s, xb + 4 * s, xb + 4 * s, th::PANEL_HI);
    icon(xx, xy, xb, IC_CLOSE, xhov ? th::TEXT : (active || hov) ? th::TEXT_DIM : th::TEXT_MUT);
    if (closed && clicked(xx - 2 * s, xy - 2 * s, xb + 4 * s, xb + 4 * s)) { *closed = true; return false; }   // × consumes the click
    return clicked(x, y, w, h);
}

// vertical scrollbar; updates *scroll (in rows). Returns true if it handled a drag.
inline void scrollbar(float x, float y, float w, float h, int total, int view, int* scroll) {
    if (total <= view) return;
    rect(x, y, w, h, th::SUNKEN);
    float th_h = h * (float)view / total; if (th_h < 24 * in.scale) th_h = 24 * in.scale; if (th_h > h) th_h = h;
    float maxScroll = (float)(total - view), denom = h - th_h; if (denom < 1) denom = 1;   // guard tiny panels
    float ty = y + denom * (*scroll / maxScroll);
    bool hov = hovered(x, y, w, h);
    rect(x + w * 0.2f, ty, w * 0.6f, th_h, hov ? th::TEXT_DIM : th::SCROLL);
    if (in.lDown && inside(x, y, w, h)) {                       // click/drag the track -> jump
        float rel = (in.my - y - th_h / 2) / denom; if (rel < 0) rel = 0; if (rel > 1) rel = 1;
        *scroll = (int)(rel * maxScroll + 0.5f);
    }
}

// ── context menu (one global, drawn on top, dispatched by id) ─────────────────
struct MenuItem { string label; int id; bool sep = false; bool disabled = false; };
struct ContextMenu { bool open = false; float x = 0, y = 0; std::vector<MenuItem> items; int picked = -1; float scroll = 0; };
inline ContextMenu menu;
inline bool menuJustOpened = false;       // don't let the opening right-click also close it this frame
inline void openMenu(float x, float y, std::vector<MenuItem> items) { menu = {}; menu.open = true; menuActive = true; menuJustOpened = true; menu.x = x; menu.y = y; menu.items = std::move(items); }
// draw + hit-test; sets menu.picked to the chosen id (or -1). Call LAST each frame.
inline void drawMenu() {
    menu.picked = -1; if (!menu.open) return;
    float rh = lineH() + 6 * in.scale, pad = 8 * in.scale, sepH = 7 * in.scale, margin = 6 * in.scale;
    float w = 0; for (auto& it : menu.items) { float iw = textW(it.label) + pad * 3; if (iw > w) w = iw; }
    if (w < 140 * in.scale) w = 140 * in.scale; if (menu.x + w > winW) menu.x = winW - w - 4; if (menu.x < 2) menu.x = 2;
    float H = 0; for (auto& it : menu.items) H += it.sep ? sepH : rh;
    float avail = winH - margin * 2, boxH = H < avail ? H : avail; bool scroll = H > avail + 0.5f;   // taller than the screen -> scroll
    if (menu.y + boxH > winH) menu.y = winH - boxH - margin; if (menu.y < margin) menu.y = margin;
    if (scroll && hovered(menu.x, menu.y, w, boxH)) menu.scroll -= in.wheel * rh;
    float maxS = H - boxH; if (menu.scroll > maxS) menu.scroll = maxS; if (menu.scroll < 0) menu.scroll = 0;
    rect(menu.x - 1, menu.y - 1, w + 2, boxH + 2, th::BORDER); rect(menu.x, menu.y, w, boxH, th::PANEL_HI);
    pushClip(menu.x, menu.y, w, boxH);
    float cy = menu.y - (scroll ? menu.scroll : 0);
    for (auto& it : menu.items) { float ih = it.sep ? sepH : rh;
        if (cy + ih > menu.y && cy < menu.y + boxH) {                                  // only visible rows
            if (it.sep) hline(menu.x + pad, cy + 3 * in.scale, w - pad * 2, th::BORDER);
            else { bool hov = hovered(menu.x, cy, w, rh) && cy >= menu.y - 0.5f && cy + rh <= menu.y + boxH + 0.5f && !it.disabled;
                if (hov) rect(menu.x, cy, w, rh, th::SEL);
                text(menu.x + pad, cy + (rh - lineH()) / 2, it.label, it.disabled ? th::TEXT_MUT : hov ? th::TEXT : th::TEXT_DIM);
                if (in.lPress && hov) menu.picked = it.id; } }
        cy += ih;
    }
    popClip();
    if (scroll) { float thh = boxH * boxH / H, ty = menu.y + (maxS > 0 ? (boxH - thh) * (menu.scroll / maxS) : 0); rect(menu.x + w - 3 * in.scale, ty, 3 * in.scale, thh, th::SCROLL); }
    bool justOpened = menuJustOpened; menuJustOpened = false;
    if (!justOpened && (in.lPress || in.rPress)) { menu.open = false; menuActive = false; menu.scroll = 0; }   // a later click (pick captured above) dismisses
}
} // namespace ui
#endif
