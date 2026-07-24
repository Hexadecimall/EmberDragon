// ember_term.h — built-in terminals backed by real PTY-spawned processes (POSIX).
// Each Pty now drives a full GRID VT/xterm emulator (rows×cols of cells with
// per-cell color/attrs, a cursor at (row,col), an alternate screen buffer, scroll
// regions, erase/insert/delete-line/char, and SGR colors). That makes real
// full-screen TUIs work — lldb's prompt AND curses apps (vim, htop, lldb gui).
// For backward-compat the emulator is flattened to `lines` (vector<string>) +
// `col` after every poll, so the existing renderer / line-selection keep working;
// `caretLine()` and `cell()` expose the real cursor row + colored cells.
// TWO sessions: `main_` (user Terminal) and `dbg` (the Debugger's lldb). The legacy
// free functions operate on main_. On Windows the whole thing is a stub.
#ifndef EMBER_TERM_H
#define EMBER_TERM_H
#include <string>
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <cctype>
#include <algorithm>
#ifndef _WIN32
  #include <util.h>
  #include <unistd.h>
  #include <fcntl.h>
  #include <termios.h>
  #include <sys/ioctl.h>
  #include <sys/wait.h>
  #include <cerrno>
  #include <csignal>
#endif

namespace term {
using std::string; using std::vector;
inline size_t maxScroll = 4000;

// ── a screen cell: one display glyph (UTF-8) + palette colors + attributes ──
struct Cell { string ch = " "; int16_t fg = -1, bg = -1; uint8_t attr = 0; };   // attr: 1 bold · 2 underline · 4 reverse · fg/bg: -1 default, 0-255 palette

// ── the VT/xterm grid emulator (no PTY — pure state machine, unit-testable) ──
struct VT {
    int rows = 40, cols = 140, cx = 0, cy = 0, top = 0, bot = 39;
    vector<vector<Cell>> grid, alt, *scr = &grid, scroll;   // scroll = primary-screen scrollback
    bool altActive = false, wrapnext = false, appCursor = false, cursorVis = true;
    Cell pen; int scx = 0, scy = 0; bool swn = false;       // current SGR pen + saved cursor (incl. pending-wrap state)
    int st = 0; string par; bool priv = false;              // esc parser: 0 normal,1 esc,2 csi,3 osc,4 charset
    string u8; int pendNeed = 0, pendHave = 0;              // UTF-8 accumulator

    VT() { resizeGrid(40, 140); }
    void resizeGrid(int r, int c) {
        r = std::max(1, r); c = std::max(1, c);
        vector<vector<Cell>> ng(r, vector<Cell>(c)), na(r, vector<Cell>(c));     // preserve the top-left overlap so a resize doesn't blank a running program
        for (int y = 0; y < r && y < (int)grid.size(); y++) for (int x = 0; x < c && x < (int)grid[y].size(); x++) ng[y][x] = grid[y][x];
        for (int y = 0; y < r && y < (int)alt.size();  y++) for (int x = 0; x < c && x < (int)alt[y].size();  x++) na[y][x] = alt[y][x];
        grid.swap(ng); alt.swap(na); rows = r; cols = c;
        scr = altActive ? &alt : &grid; top = 0; bot = rows - 1;
        if (cx >= cols) cx = cols - 1; if (cy >= rows) cy = rows - 1; wrapnext = false;
    }
    void clearGrids() { for (auto& row : grid) row.assign(cols, Cell{}); for (auto& row : alt) row.assign(cols, Cell{}); }
    vector<int> params() { vector<int> v; if (par.empty()) return v; size_t p = 0;
        while (p <= par.size()) { size_t q = par.find(';', p); string t = par.substr(p, q == string::npos ? string::npos : q - p);
            v.push_back(t.empty() ? 0 : atoi(t.c_str())); if (q == string::npos) break; p = q + 1; } return v; }
    int p0(int def) { auto v = params(); return (v.empty() || v[0] == 0) ? def : v[0]; }
    Cell& at(int y, int x) { static Cell junk; if (y < 0 || y >= rows || x < 0 || x >= cols) return junk; return (*scr)[y][x]; }
    void scrollUp(int n) { for (int i = 0; i < n; i++) { if (!altActive && top == 0) { scroll.push_back((*scr)[0]); if (scroll.size() > maxScroll) scroll.erase(scroll.begin()); }
        for (int y = top; y < bot; y++) (*scr)[y] = (*scr)[y + 1]; (*scr)[bot] = vector<Cell>(cols); } }
    void scrollDown(int n) { for (int i = 0; i < n; i++) { for (int y = bot; y > top; y--) (*scr)[y] = (*scr)[y - 1]; (*scr)[top] = vector<Cell>(cols); } }
    void lf() { if (cy >= bot) scrollUp(1); else cy++; }
    void putG(const string& g) { if (wrapnext) { cx = 0; lf(); wrapnext = false; }
        Cell& c = at(cy, cx); c.ch = g; c.fg = pen.fg; c.bg = pen.bg; c.attr = pen.attr;
        if (cx >= cols - 1) wrapnext = true; else cx++; }
    void eraseLine(int m) { if (m == 0) for (int x = cx; x < cols; x++) at(cy, x) = Cell{};
        else if (m == 1) for (int x = 0; x <= cx && x < cols; x++) at(cy, x) = Cell{};
        else for (int x = 0; x < cols; x++) at(cy, x) = Cell{}; }
    void eraseDisp(int m) { if (m == 0) { eraseLine(0); for (int y = cy + 1; y < rows; y++) for (int x = 0; x < cols; x++) at(y, x) = Cell{}; }
        else if (m == 1) { eraseLine(1); for (int y = 0; y < cy; y++) for (int x = 0; x < cols; x++) at(y, x) = Cell{}; }
        else { for (int y = 0; y < rows; y++) for (int x = 0; x < cols; x++) at(y, x) = Cell{}; if (m == 3) scroll.clear(); } }
    void sgr() { auto v = params(); if (v.empty()) { pen = Cell{}; return; }
        for (size_t i = 0; i < v.size(); i++) { int n = v[i];
            if (n == 0) pen = Cell{}; else if (n == 1) pen.attr |= 1; else if (n == 4) pen.attr |= 2; else if (n == 7) pen.attr |= 4;
            else if (n == 22) pen.attr &= ~1; else if (n == 24) pen.attr &= ~2; else if (n == 27) pen.attr &= ~4;
            else if (n >= 30 && n <= 37) pen.fg = n - 30; else if (n == 39) pen.fg = -1;
            else if (n >= 40 && n <= 47) pen.bg = n - 40; else if (n == 49) pen.bg = -1;
            else if (n >= 90 && n <= 97) pen.fg = n - 90 + 8; else if (n >= 100 && n <= 107) pen.bg = n - 100 + 8;
            else if ((n == 38 || n == 48) && i + 1 < v.size()) { int mode = v[i + 1];
                if (mode == 5 && i + 2 < v.size()) { (n == 38 ? pen.fg : pen.bg) = v[i + 2]; i += 2; }
                else if (mode == 2 && i + 4 < v.size()) { int r = v[i + 2], g = v[i + 3], b = v[i + 4];
                    (n == 38 ? pen.fg : pen.bg) = 16 + 36 * (r * 5 / 255) + 6 * (g * 5 / 255) + (b * 5 / 255); i += 4; } } } }
    void setMode(bool on) { if (!priv) return; int m = atoi(par.c_str());
        if (m == 25) cursorVis = on; else if (m == 1) appCursor = on;
        else if (m == 1049 || m == 47 || m == 1047) { if (on && !altActive) { altActive = true; scr = &alt; for (auto& r : alt) r = vector<Cell>(cols); cx = cy = 0; wrapnext = false; }
            else if (!on && altActive) { altActive = false; scr = &grid; wrapnext = false; } } }
    void csi(char c) { switch (c) {
        case 'A': cy = std::max(top, cy - p0(1)); break;                         case 'B': cy = std::min(bot, cy + p0(1)); break;
        case 'C': cx = std::min(cols - 1, cx + p0(1)); wrapnext = false; break;  case 'D': cx = std::max(0, cx - p0(1)); wrapnext = false; break;
        case 'E': cx = 0; cy = std::min(bot, cy + p0(1)); break;                 case 'F': cx = 0; cy = std::max(top, cy - p0(1)); break;
        case 'G': cx = std::min(cols - 1, std::max(0, p0(1) - 1)); wrapnext = false; break;
        case 'd': cy = std::min(rows - 1, std::max(0, p0(1) - 1)); break;
        case 'H': case 'f': { auto v = params(); int r = v.size() > 0 && v[0] ? v[0] : 1, cc = v.size() > 1 && v[1] ? v[1] : 1;
            cy = std::min(rows - 1, r - 1); cx = std::min(cols - 1, cc - 1); wrapnext = false; } break;
        case 'J': eraseDisp(par.empty() ? 0 : atoi(par.c_str())); break;         case 'K': eraseLine(par.empty() ? 0 : atoi(par.c_str())); break;
        case 'L': { int k = p0(1); for (int i = 0; i < k; i++) { if (cy < top || cy > bot) break; for (int y = bot; y > cy; y--) (*scr)[y] = (*scr)[y - 1]; (*scr)[cy] = vector<Cell>(cols); } } break;
        case 'M': { int k = p0(1); for (int i = 0; i < k; i++) { if (cy < top || cy > bot) break; for (int y = cy; y < bot; y++) (*scr)[y] = (*scr)[y + 1]; (*scr)[bot] = vector<Cell>(cols); } } break;
        case '@': { int k = p0(1); auto& row = (*scr)[cy]; for (int i = 0; i < k && cx < cols; i++) row.insert(row.begin() + cx, Cell{}); row.resize(cols); } break;
        case 'P': { int k = p0(1); auto& row = (*scr)[cy]; for (int i = 0; i < k && cx < (int)row.size(); i++) row.erase(row.begin() + cx); row.resize(cols); } break;
        case 'X': { int k = p0(1); for (int i = 0; i < k && cx + i < cols; i++) at(cy, cx + i) = Cell{}; } break;
        case 'S': scrollUp(p0(1)); break;                                        case 'T': scrollDown(p0(1)); break;
        case 'r': { auto v = params(); int t = v.size() > 0 && v[0] ? v[0] : 1, b = v.size() > 1 && v[1] ? v[1] : rows;
            top = std::max(0, t - 1); bot = std::min(rows - 1, b - 1); if (bot < top) bot = top; cx = cy = 0; } break;
        case 'm': sgr(); break;   case 'h': setMode(true); break;   case 'l': setMode(false); break;
        case 's': scx = cx; scy = cy; swn = wrapnext; break;   case 'u': cx = scx; cy = scy; wrapnext = swn; break;   default: break; } }
    void esc(char c) { switch (c) { case 'D': lf(); break; case 'M': if (cy <= top) scrollDown(1); else cy--; break;
        case 'E': cx = 0; lf(); break; case '7': scx = cx; scy = cy; swn = wrapnext; break; case '8': cx = scx; cy = scy; wrapnext = swn; break;
        case 'c': clearGrids(); scroll.clear(); cx = cy = 0; pen = Cell{}; break; default: break; } }
    void byte(unsigned char c) {
        if (st == 1) { if (c == '[') { st = 2; par.clear(); priv = false; } else if (c == ']') { st = 3; } else if (c == '(' || c == ')') { st = 4; } else { esc(c); st = 0; } return; }
        if (st == 2) { if (c == '?') { priv = true; return; } if ((c >= '0' && c <= '9') || c == ';') { par += c; if (par.size() > 80) st = 0; return; } csi(c); st = 0; return; }
        if (st == 3) { if (c == 7) st = 0; else if (c == 27) st = 1; return; }   // OSC ... BEL/ESC
        if (st == 4) { st = 0; return; }
        if (c == 27) { st = 1; return; }
        if (c == '\n') { lf(); return; }   if (c == '\r') { cx = 0; wrapnext = false; return; }
        if (c == '\b') { if (cx > 0) cx--; wrapnext = false; return; }   if (c == '\t') { cx = std::min(cols - 1, ((cx / 8) + 1) * 8); return; }
        if (c == 7) return;                                              // bell
        if (c < 0x80) { putG(string(1, (char)c)); return; }
        if ((c & 0xC0) == 0xC0) { u8 = string(1, (char)c); pendNeed = (c >= 0xF0) ? 3 : (c >= 0xE0) ? 2 : 1; pendHave = 0; return; }
        if ((c & 0xC0) == 0x80) { if (pendNeed > 0) { u8 += string(1, (char)c); if (++pendHave >= pendNeed) { putG(u8); pendNeed = 0; } } return; } }
    void feed(const string& s) { for (unsigned char c : s) byte(c); }
    int caretLine() { return altActive ? cy : (int)scroll.size() + cy; }       // cursor row in the flattened view
    vector<string> textLines() { vector<string> o; auto rowtxt = [&](vector<Cell>& r) { string s; for (auto& c : r) s += c.ch; while (!s.empty() && s.back() == ' ') s.pop_back(); return s; };
        if (!altActive) for (auto& r : scroll) o.push_back(rowtxt(r));          // scrollback hidden while the alt-screen (a TUI) owns the display
        for (auto& r : *scr) o.push_back(rowtxt(r)); return o; }
    void reset() { altActive = false; scr = &grid; scroll.clear(); resizeGrid(rows, cols); clearGrids(); cx = cy = 0; top = 0; bot = rows - 1; pen = Cell{}; st = 0; par.clear(); pendNeed = 0; wrapnext = false; }
};

struct Pty {
    VT vt;
    vector<string> lines = { "" };       // = vt.textLines(), rebuilt after poll (renderer + selection read this)
    bool started = false, alive = false;
    int col = 0;                         // = vt.cx (compat caret column)
    std::string termEnv = "xterm-256color";   // both sessions now use a real grid, so lldb/curses get full cursor-addressing
#ifndef _WIN32
    int masterFd = -1; pid_t child = -1;
    int rows_ = 40, cols_ = 140;

    void sync() { lines = vt.textLines(); if (lines.empty()) lines.push_back(""); col = vt.cx; }
    int caretLine() { return vt.caretLine(); }
    Cell* cell(int line, int col) {                                            // colored cell at a flattened-view line (or null)
        if (vt.altActive) { if (line < 0 || line >= vt.rows || col < 0 || col >= vt.cols) return nullptr; return &vt.alt[line][col]; }
        int sb = (int)vt.scroll.size();
        if (line < sb) { if (col < 0 || col >= (int)vt.scroll[line].size()) return nullptr; return &vt.scroll[line][col]; }
        int gy = line - sb; if (gy < 0 || gy >= vt.rows || col < 0 || col >= vt.cols) return nullptr; return &vt.grid[gy][col]; }

    bool start(const char* runCmd = nullptr) {
        if (started && alive) return true;
        struct winsize ws = { (unsigned short)rows_, (unsigned short)cols_, 0, 0 };
        child = forkpty(&masterFd, nullptr, nullptr, &ws);
        if (child < 0) { vt.reset(); vt.feed("(failed to start)"); sync(); started = false; return false; }
        if (child == 0) {
            setenv("TERM", termEnv.c_str(), 1);
            const char* home = getenv("HOME"); if (home) { if (chdir(home)) {} }
            if (runCmd && *runCmd) { execl("/bin/sh", "sh", "-c", runCmd, (char*)nullptr); _exit(127); }
            const char* sh = getenv("SHELL"); if (!sh) sh = "/bin/zsh";
            execlp(sh, sh, "-i", (char*)nullptr); _exit(127);
        }
        fcntl(masterFd, F_SETFL, O_NONBLOCK);
        started = true; alive = true; vt.reset(); vt.resizeGrid(rows_, cols_); sync();
        return true;
    }
    bool poll() {
        if (!started || !alive || masterFd < 0) return false;
        char buf[8192]; ssize_t n; bool any = false;
        while ((n = read(masterFd, buf, sizeof buf)) > 0) { for (ssize_t i = 0; i < n; i++) vt.byte((unsigned char)buf[i]); any = true; }
        if (n == 0 || (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {     // EOF: the child exited
            alive = false; int st;
            for (int k = 0; k < 50 && child > 0; k++) { pid_t r = waitpid(child, &st, WNOHANG); if (r == child || (r < 0 && errno == ECHILD)) break; usleep(1000); }
            close(masterFd); masterFd = -1; child = -1;
            if (vt.altActive) { vt.altActive = false; vt.scr = &vt.grid; }       // a curses app that died without restoring -> drop back to primary
            vt.feed("\r\n[process exited — press Enter or click to restart]\r\n");
            any = true;
        }
        if (any) sync();
        return any;
    }
    void input(const string& s) { if (started && alive && masterFd >= 0) { ssize_t w = write(masterFd, s.data(), s.size()); (void)w; } }
    void resize(int rows, int cols) { rows = std::max(1, rows); cols = std::max(20, cols);
        if (rows == rows_ && cols == cols_) return;
        rows_ = rows; cols_ = cols; vt.resizeGrid(rows, cols);
        if (started && alive && masterFd >= 0) { struct winsize ws = { (unsigned short)rows, (unsigned short)cols, 0, 0 }; ioctl(masterFd, TIOCSWINSZ, &ws); }
        sync(); }
    void shutdown() { if (child > 0) kill(child, SIGTERM); }
    // arrow / nav keys, encoded for the child's current mode (DECCKM app-cursor vs normal)
    string arrow(char d) { return vt.appCursor ? (string("\x1bO") + d) : (string("\x1b[") + d); }
#else
    void sync() {} int caretLine() { return (int)lines.size() - 1; } Cell* cell(int, int) { return nullptr; }
    bool start(const char* = nullptr) { lines = { "the built-in terminal is macOS-only in this build" }; started = false; return false; }
    bool poll() { return false; }
    void input(const string&) {}
    void resize(int, int) {}
    void shutdown() {}
    string arrow(char d) { return string("\x1b[") + d; }
#endif
};

inline Pty main_;   // the user Terminal (bottom dock)
inline Pty dbg;     // the Debugger's lldb session (its own dock tab)

// ── legacy free API: operates on main_ so existing call sites keep working ─────
inline std::vector<std::string>& lines = main_.lines;
inline bool& started = main_.started;
inline bool& alive   = main_.alive;
inline int&  col     = main_.col;
inline bool start() { return main_.start(); }
inline bool poll() { bool a = main_.poll(); bool b = dbg.poll(); return a || b; }
inline void input(const string& s) { main_.input(s); }
inline void resize(int rows, int cols) { main_.resize(rows, cols); }
inline void shutdown() { main_.shutdown(); dbg.shutdown(); }
} // namespace term
#endif
