// ember_gui_win.cpp — EmberDragon GUI, Windows backend (Win32 + wgl).
// Thin: owns the window + GL context, feeds mouse/keyboard/scroll into ui::in,
// provides the open-file dialog + clipboard, and calls app::render() per event.
// Cross-compiled FROM macOS via pullio's mingw toolchain:
//   pullio --gcc cxx Win64 -i ember_gui_win.cpp -o ember-gui.exe --gui \
//          -- -lopengl32 -lgdi32 -luser32 -lcomdlg32
#include "ember_app.h"        // pulls <windows.h> + <GL/gl.h> under _WIN32
#include <commdlg.h>
#include <string>

static HDC gDC; static HGLRC gRC; static HWND gWnd; static std::string gDir;

static void loadFonts() {
    char win[MAX_PATH] = {0}; GetWindowsDirectoryA(win, MAX_PATH); std::string f = std::string(win) + "\\Fonts\\";
    ef::load({ f + "consola.ttf", f + "CascadiaMono.ttf", f + "lucon.ttf", f + "cour.ttf" });
}
static bool setupGL(HWND h) {
    gDC = GetDC(h); if (!gDC) return false;
    PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 24, 0,0,0,0,0,0,0,0,0,0,0,0,0, 24, 8, 0, PFD_MAIN_PLANE, 0,0,0,0 };
    int pf = ChoosePixelFormat(gDC, &pfd); if (!pf || !SetPixelFormat(gDC, pf, &pfd)) return false;
    gRC = wglCreateContext(gDC); if (!gRC || !wglMakeCurrent(gDC, gRC)) return false;
    int dpi = GetDeviceCaps(gDC, LOGPIXELSX); ui::in.scale = dpi > 0 ? dpi / 96.0f : 1.0f;
    loadFonts(); return true;
}
static void frame() {
    if (!gRC || !wglMakeCurrent(gDC, gRC)) return;   // skip the frame if the context is lost
    static float lastPx = 0; float px = 13 * ui::in.scale; if (px != lastPx) { ef::bake(px); lastPx = px; }
    glViewport(0, 0, (int)ui::winW, (int)ui::winH); glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, ui::winW, ui::winH, 0, -1, 1); glMatrixMode(GL_MODELVIEW); glLoadIdentity(); glDisable(GL_DEPTH_TEST);
    app::render(); ui::in.clearOneShot(); SwapBuffers(gDC);
}
static void winClipboard(const std::string& t) {
    if (!OpenClipboard(gWnd)) return; EmptyClipboard();
    HGLOBAL g = GlobalAlloc(GMEM_MOVEABLE, t.size() + 1);
    if (g) { memcpy(GlobalLock(g), t.c_str(), t.size() + 1); GlobalUnlock(g); if (!SetClipboardData(CF_TEXT, g)) GlobalFree(g); }  // free on failure (ownership only transfers on success)
    CloseClipboard();
}
static std::string winGetClipboard() {
    if (!OpenClipboard(gWnd)) return ""; std::string out;
    HANDLE h = GetClipboardData(CF_TEXT); if (h) { char* p = (char*)GlobalLock(h); if (p) { out = p; GlobalUnlock(h); } }
    CloseClipboard(); return out;
}
static std::string winPickFile() {
    char file[MAX_PATH] = {0}; OPENFILENAMEA o = {0}; o.lStructSize = sizeof(o); o.hwndOwner = gWnd;
    o.lpstrFilter = "Binaries\0*.exe;*.dll;*.o;*.obj;*.bin\0All Files\0*.*\0\0"; o.lpstrFile = file; o.nMaxFile = MAX_PATH;
    o.lpstrTitle = "Open a binary to decompile"; o.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameA(&o) ? std::string(file) : std::string();
}
static std::string winPickSavePath() {
    char file[MAX_PATH] = {0}; OPENFILENAMEA o = {0}; o.lStructSize = sizeof(o); o.hwndOwner = gWnd;
    o.lpstrFilter = "Binary\0*.*\0\0"; o.lpstrFile = file; o.nMaxFile = MAX_PATH;
    o.lpstrTitle = "Save the patched binary as a new file"; o.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    return GetSaveFileNameA(&o) ? std::string(file) : std::string();
}

static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
    case WM_SIZE: ui::winW = LOWORD(l); ui::winH = HIWORD(l); frame(); return 0;
    case WM_PAINT: { PAINTSTRUCT ps; BeginPaint(h, &ps); frame(); EndPaint(h, &ps); return 0; }
    case WM_ERASEBKGND: return 1;
    case WM_MOUSEMOVE: ui::in.mx = (float)(short)LOWORD(l); ui::in.my = (float)(short)HIWORD(l); if (w & MK_LBUTTON) ui::in.dragging = true; frame(); return 0;
    case WM_LBUTTONDOWN: SetCapture(h); ui::in.mx = (float)(short)LOWORD(l); ui::in.my = (float)(short)HIWORD(l); ui::in.lDown = ui::in.lPress = true; ui::in.pressX = ui::in.mx; ui::in.pressY = ui::in.my; frame(); return 0;
    case WM_LBUTTONUP: ReleaseCapture(); ui::in.lDown = false; ui::in.lRelease = true; ui::in.dragging = false; frame(); return 0;
    case WM_RBUTTONDOWN: ui::in.mx = (float)(short)LOWORD(l); ui::in.my = (float)(short)HIWORD(l); ui::in.rDown = ui::in.rPress = true; frame(); return 0;
    case WM_RBUTTONUP: ui::in.rDown = false; frame(); return 0;
    case WM_TIMER: if (app::optimizing.load() || app::optJustFinished.load() || app::analyzing.load() || app::anaJustFinished.load() || app::aiBusy.load() || app::aiDone.load() || app::recompiling.load() || app::understanding.load() || app::understandDone.load()) frame(); return 0;
    case WM_MOUSEWHEEL: { float d = -GET_WHEEL_DELTA_WPARAM(w) / 120.0f * 3;
        if (GET_KEYSTATE_WPARAM(w) & MK_SHIFT) ui::in.wheelX = d; else ui::in.wheel = d;   // shift+wheel = horizontal
        frame(); return 0; }
    case WM_MOUSEHWHEEL: ui::in.wheelX = GET_WHEEL_DELTA_WPARAM(w) / 120.0f * 3; frame(); return 0;   // trackpad / tilt-wheel horizontal
    case WM_CHAR: if (w >= 32 && w < 127) { ui::in.ch = (unsigned)w; frame(); } else if (w == 8) { ui::in.ch = 8; frame(); } return 0;
    case WM_KEYDOWN: { bool ctrl = GetKeyState(VK_CONTROL) & 0x8000, shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0; ui::in.shift = shift; ui::in.ctrl = ctrl;
        if (ctrl) { if (w == 'P' && shift) app::togglePalette(); else if (w == 'S') app::saveFile();
            else if (w == 'O') app::g.pending = app::P_OPEN; else if (w == 'E') app::g.pending = app::P_EXPORT; else if (w == 'R' && app::g.bin.loaded) { app::g.pending = app::P_OPENPATH; app::g.pendingArg = app::g.bin.path; }
            else if (w == 'C') app::copySelection(); else if (w == 'V') app::pasteActive(); else if (w == 'X') app::cutActive();
            else if (w == 'Z') { if (shift) app::redoActive(); else app::undoActive(); } else if (w == 'A') app::selectAllActive();
            else if (w == 'F') app::toggleFind(); else if (w == 'G') app::findNext(); else if (w == 'B' && app::g.bin.loaded) app::runRecompile(); }
        else { switch (w) { case VK_UP: ui::in.key = ui::K_UP; break; case VK_DOWN: ui::in.key = ui::K_DOWN; break; case VK_LEFT: ui::in.key = ui::K_LEFT; break; case VK_RIGHT: ui::in.key = ui::K_RIGHT; break; case VK_PRIOR: ui::in.key = ui::K_PGUP; break; case VK_NEXT: ui::in.key = ui::K_PGDN; break; case VK_RETURN: ui::in.key = ui::K_ENTER; break; case VK_ESCAPE: ui::in.key = ui::K_ESC; break; case VK_F2: app::startRenameActive(); break; } }
        frame(); return 0; }
    case WM_DESTROY: app::saveConfig(); PostQuitMessage(0); return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hp, LPSTR cmd, int show) {
    (void)hp; SetProcessDPIAware();
    char exe[MAX_PATH] = {0}; GetModuleFileNameA(0, exe, MAX_PATH); { std::string e = exe; gDir = e.substr(0, e.find_last_of("\\/") + 1); }
    app::init(gDir); app::pickFile = winPickFile; app::pickSavePath = winPickSavePath; app::setClipboard = winClipboard; app::getClipboard = winGetClipboard;
    std::string bin = cmd && *cmd ? std::string(cmd) : ""; if (!bin.empty() && bin.front() == '"') { bin.erase(0, 1); if (!bin.empty() && bin.back() == '"') bin.pop_back(); }
    if (!bin.empty()) app::openBinary(bin);

    WNDCLASSA wc = {}; wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = "EmberDragon"; wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW; wc.hCursor = LoadCursor(0, IDC_ARROW); wc.hbrBackground = 0;
    if (!RegisterClassA(&wc)) { MessageBoxA(0, "Failed to register window class", "EmberDragon", MB_ICONERROR); return 1; }
    gWnd = CreateWindowA("EmberDragon", "EmberDragon", WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 820, 0, 0, hInst, 0);
    if (!gWnd) { MessageBoxA(0, "Failed to create window", "EmberDragon", MB_ICONERROR); return 1; }
    if (!setupGL(gWnd)) { MessageBoxA(0, "Failed to initialize OpenGL", "EmberDragon", MB_ICONERROR); return 1; }
    ShowWindow(gWnd, show); UpdateWindow(gWnd); SetTimer(gWnd, 1, 60, 0);   // poll background optimize
    MSG msg; while (GetMessage(&msg, 0, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessage(&msg);
        if (app::g.pending != app::P_NONE) { app::processPending(); InvalidateRect(gWnd, 0, FALSE); } }   // deferred open/export
    wglMakeCurrent(0, 0); wglDeleteContext(gRC); ReleaseDC(gWnd, gDC);
    return 0;
}
