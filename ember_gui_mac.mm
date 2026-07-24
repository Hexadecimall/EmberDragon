// ember_gui_mac.mm — EmberDragon GUI, macOS backend (Cocoa + NSOpenGLView).
// Thin: owns the window + GL context, feeds mouse/keyboard/scroll into ui::in,
// provides the open-file dialog + clipboard, and calls app::render() per event.
//   build:  clang++ -std=c++17 -O2 -DGL_SILENCE_DEPRECATION -x objective-c++ \
//             ember_gui_mac.mm -framework Cocoa -framework OpenGL -o ember-gui
#import <Cocoa/Cocoa.h>
#include <mach-o/dyld.h>     // _NSGetExecutablePath -> resolve real exe dir for global installs
#include "ember_app.h"

static std::string gDir;

static void loadFonts() {
    std::vector<std::string> paths;
    if (!gDir.empty()) paths.push_back(gDir + "../Resources/Menlo.ttc");   // bundled inside the .app -> self-contained text
    paths.push_back("/usr/local/share/emberdragon/Menlo.ttc");            // /usr/local install -> shared resources
    paths.insert(paths.end(), { "/System/Library/Fonts/SFNSMono.ttf", "/System/Library/Fonts/Menlo.ttc",
               "/System/Library/Fonts/Monaco.ttf", "/System/Library/Fonts/Courier.ttc" });
    ef::load(paths);
}

@interface EmberView : NSOpenGLView
@end
@implementation EmberView
- (instancetype)initWithFrame:(NSRect)f pixelFormat:(NSOpenGLPixelFormat*)pf {
    if ((self = [super initWithFrame:f pixelFormat:pf])) [self setWantsBestResolutionOpenGLSurface:YES];
    return self;
}
- (BOOL)isFlipped { return YES; }
- (BOOL)acceptsFirstResponder { return YES; }
- (void)prepareOpenGL { [super prepareOpenGL]; GLint v = 1; [[self openGLContext] setValues:&v forParameter:NSOpenGLContextParameterSwapInterval];
    [[self openGLContext] makeCurrentContext]; loadFonts(); }
- (void)updateTrackingAreas { [super updateTrackingAreas];
    for (NSTrackingArea* a in [self trackingAreas]) [self removeTrackingArea:a];
    [self addTrackingArea:[[NSTrackingArea alloc] initWithRect:[self bounds]
        options:(NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect) owner:self userInfo:nil]]; }
- (float)scale { NSRect b = [self convertRectToBacking:[self bounds]]; return [self bounds].size.width > 0 ? b.size.width / [self bounds].size.width : 1; }
- (void)setMouse:(NSEvent*)e { NSPoint p = [self convertPoint:[e locationInWindow] fromView:nil]; float s = [self scale]; ui::in.mx = p.x * s; ui::in.my = p.y * s;
    NSEventModifierFlags mf = [e modifierFlags]; ui::in.shift = (mf & NSEventModifierFlagShift) != 0; ui::in.ctrl = (mf & NSEventModifierFlagCommand) != 0; }   // carry LIVE modifiers on mouse events so shift+click extends selection (was stale — only set on keyDown)
- (void)reshape { [super reshape]; NSRect bk = [self convertRectToBacking:[self bounds]]; ui::winW = bk.size.width; ui::winH = bk.size.height; [self setNeedsDisplay:YES]; }
- (void)drawRect:(NSRect)d { (void)d; [[self openGLContext] makeCurrentContext];
    float s = [self scale]; ui::in.scale = s;
    static float lastPx = 0; float px = 13 * s; if (px != lastPx) { ef::bake(px); lastPx = px; }
    glViewport(0, 0, (int)ui::winW, (int)ui::winH); glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(0, ui::winW, ui::winH, 0, -1, 1); glMatrixMode(GL_MODELVIEW); glLoadIdentity(); glDisable(GL_DEPTH_TEST);
    app::render(); ui::in.clearOneShot();
    [[self openGLContext] flushBuffer];
    if (app::g.pending != app::P_NONE) dispatch_async(dispatch_get_main_queue(), ^{ app::processPending(); [self setNeedsDisplay:YES]; }); }  // modal/heavy work OFF the draw
// ── input ──
- (void)mouseMoved:(NSEvent*)e { [self setMouse:e]; [self setNeedsDisplay:YES]; }
- (void)mouseDown:(NSEvent*)e { [self setMouse:e]; ui::in.lDown = true; ui::in.lPress = true; ui::in.pressX = ui::in.mx; ui::in.pressY = ui::in.my; [self setNeedsDisplay:YES]; }
- (void)mouseUp:(NSEvent*)e { [self setMouse:e]; ui::in.lDown = false; ui::in.lRelease = true; [self setNeedsDisplay:YES]; }
- (void)mouseDragged:(NSEvent*)e { [self setMouse:e]; ui::in.dragging = true; [self setNeedsDisplay:YES]; }
- (void)rightMouseDown:(NSEvent*)e { [self setMouse:e]; ui::in.rDown = true; ui::in.rPress = true; [self setNeedsDisplay:YES]; }
- (void)rightMouseUp:(NSEvent*)e { [self setMouse:e]; ui::in.rDown = false; [self setNeedsDisplay:YES]; }
- (void)scrollWheel:(NSEvent*)e {
    bool prec = [e hasPreciseScrollingDeltas];
    static double acc = 0;                                   // accumulate fractional trackpad deltas -> integer line steps
    acc += -[e scrollingDeltaY] * (prec ? 0.45 : 3.0);
    int lines = (int)acc; if (lines > 60) lines = 60; if (lines < -60) lines = -60;   // cap a fast flick
    acc -= lines;
    // HORIZONTAL: trackpad two-finger swipe (scrollingDeltaX), or shift+wheel (macOS routes that to deltaX). +right.
    static double accx = 0;
    accx += -[e scrollingDeltaX] * (prec ? 0.45 : 3.0);
    int cols = (int)accx; if (cols > 80) cols = 80; if (cols < -80) cols = -80;
    accx -= cols;
    if (lines != 0 || cols != 0) { ui::in.wheel = (float)lines; ui::in.wheelX = (float)cols; [self setNeedsDisplay:YES]; } }
- (void)keyDown:(NSEvent*)e {
    unsigned short kc = [e keyCode]; NSString* c = [e charactersIgnoringModifiers]; ui::in.shift = ([e modifierFlags] & NSEventModifierFlagShift) != 0; ui::in.ctrl = ([e modifierFlags] & NSEventModifierFlagCommand) != 0;
    bool cmd = ([e modifierFlags] & NSEventModifierFlagCommand) != 0, shift = ([e modifierFlags] & NSEventModifierFlagShift) != 0;
    if (([e modifierFlags] & NSEventModifierFlagControl) && (app::g.focus == app::F_TERM || app::g.focus == app::F_DEBUG) && [c length]) {   // Ctrl+key -> control char to the shell/lldb (Ctrl-C/D/Z/L…)
        bool toDbg = app::g.focus == app::F_DEBUG; unichar u = tolower([c characterAtIndex:0]);
        if (u >= 'a' && u <= 'z') { std::string cc(1, (char)(u - 'a' + 1)); if (toDbg) term::dbg.input(cc); else term::input(cc); [self setNeedsDisplay:YES]; return; }
        if (u == '[') { if (toDbg) term::dbg.input("\x1b"); else term::input("\x1b"); [self setNeedsDisplay:YES]; return; } }
    switch (kc) { case 126: ui::in.key = ui::K_UP; break; case 125: ui::in.key = ui::K_DOWN; break; case 123: ui::in.key = ui::K_LEFT; break; case 124: ui::in.key = ui::K_RIGHT; break;
        case 116: ui::in.key = ui::K_PGUP; break; case 121: ui::in.key = ui::K_PGDN; break;
        case 36: ui::in.key = ui::K_ENTER; break; case 53: ui::in.key = ui::K_ESC; break; case 51: ui::in.ch = 8; break; case 48: ui::in.key = ui::K_TAB; break;
        case 120: app::startRenameActive(); break;   // F2 = rename symbol
        default: if ([c length] && !cmd) { unichar u = [c characterAtIndex:0]; if (u >= 32 && u < 127) ui::in.ch = u; } }
    if (cmd && [c length]) { unichar u = tolower([c characterAtIndex:0]);
        if (u == 'p' && shift) app::togglePalette();
        else if (u == 's') app::saveFile();
        else if (u == 'c') app::copySelection();
        else if (u == 'v') app::pasteActive();
        else if (u == 'x') app::cutActive();
        else if (u == 'z') { if (shift) app::redoActive(); else app::undoActive(); }
        else if (u == 'a') app::selectAllActive();
        else if (u == 'f') app::toggleFind();
        else if (u == 'g') app::findNext();
        else if (u == 'b') { if (app::g.bin.loaded) app::runRecompile(); }
        else if (u == '.') app::applyAvailableFix();                              // ⌘. -> apply the clangd quick-fix
        else if (u == 'o') app::g.pending = app::P_OPEN;
        else if (u == 'e') app::g.pending = app::P_EXPORT;
        else if (u == 'r' && app::g.bin.loaded) { app::g.pending = app::P_OPENPATH; app::g.pendingArg = app::g.bin.path; } }
    [self setNeedsDisplay:YES];
}
@end

@interface AppDelegate : NSObject<NSApplicationDelegate> @property (strong) NSWindow* win; @property (strong) EmberView* view; @end
@implementation AppDelegate
- (void)refresh { [self.view setNeedsDisplay:YES]; }
- (void)applicationWillTerminate:(NSNotification*)n { (void)n; app::saveConfig(); term::shutdown(); }
- (void)menuOpen:(id)s { (void)s; if (app::pickFile) app::openBinary(app::pickFile()); [self refresh]; }
- (void)menuExport:(id)s { (void)s; app::exportBinary(); [self refresh]; }
- (void)menuReload:(id)s { (void)s; if (app::g.bin.loaded) app::openBinary(app::g.bin.path); [self refresh]; }
- (void)menuApplyFix:(id)s { (void)s; app::applyAvailableFix(); [self refresh]; }   // ⌘. — captured here so the system doesn't treat it as Cancel (exit fullscreen)
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)a { (void)a; return YES; }
@end

static void buildMenu(AppDelegate* d) {
    NSMenu* bar = [NSMenu new]; [NSApp setMainMenu:bar];
    NSMenuItem* appI = [NSMenuItem new]; [bar addItem:appI]; NSMenu* app = [NSMenu new]; [appI setSubmenu:app];
    [app addItemWithTitle:@"About EmberDragon" action:@selector(orderFrontStandardAboutPanel:) keyEquivalent:@""];
    [app addItem:[NSMenuItem separatorItem]]; [app addItemWithTitle:@"Quit EmberDragon" action:@selector(terminate:) keyEquivalent:@"q"];
    NSMenuItem* fI = [NSMenuItem new]; [bar addItem:fI]; NSMenu* file = [[NSMenu alloc] initWithTitle:@"File"]; [fI setSubmenu:file];
    [[file addItemWithTitle:@"Open…" action:@selector(menuOpen:) keyEquivalent:@"o"] setTarget:d];
    [[file addItemWithTitle:@"Export decomp/" action:@selector(menuExport:) keyEquivalent:@"e"] setTarget:d];
    [[file addItemWithTitle:@"Reload" action:@selector(menuReload:) keyEquivalent:@"r"] setTarget:d];
    [file addItem:[NSMenuItem separatorItem]]; [file addItemWithTitle:@"Close Window" action:@selector(performClose:) keyEquivalent:@"w"];
    NSMenuItem* eI = [NSMenuItem new]; [bar addItem:eI]; NSMenu* edit = [[NSMenu alloc] initWithTitle:@"Edit"]; [eI setSubmenu:edit];
    [[edit addItemWithTitle:@"Apply Fix" action:@selector(menuApplyFix:) keyEquivalent:@"."] setTarget:d];   // ⌘. quick-fix (menu shortcut = intercepted before the system Cancel)
}

int main(int argc, const char** argv) {
    @autoreleasepool {
        std::string self = argv[0];
        { char buf[4096]; uint32_t sz = sizeof buf;                  // real exe path -> works when launched as a global command (argv[0] has no slash)
          if (_NSGetExecutablePath(buf, &sz) == 0) { char rp[4096]; self = realpath(buf, rp) ? rp : buf; } }
        gDir = self.substr(0, self.find_last_of('/') + 1);
        app::init(gDir);
        app::pickFile = []() -> std::string { NSOpenPanel* o = [NSOpenPanel openPanel]; [o setCanChooseFiles:YES]; [o setAllowsMultipleSelection:NO]; [o setMessage:@"Choose a Mach-O binary or object to decompile"];
            return ([o runModal] == NSModalResponseOK) ? std::string([[[o URL] path] UTF8String]) : std::string(); };
        app::pickSavePath = []() -> std::string { NSSavePanel* sp = [NSSavePanel savePanel]; [sp setMessage:@"Save the patched binary as a new file"]; [sp setCanCreateDirectories:YES];
            return ([sp runModal] == NSModalResponseOK) ? std::string([[[sp URL] path] UTF8String]) : std::string(); };
        app::setClipboard = [](const std::string& t) { NSPasteboard* pb = [NSPasteboard generalPasteboard]; [pb clearContents]; [pb setString:[NSString stringWithUTF8String:t.c_str()] forType:NSPasteboardTypeString]; };
        app::getClipboard = []() -> std::string { NSString* sv = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString]; return sv ? std::string([sv UTF8String]) : std::string(); };
        if (argc > 1) app::openBinary(argv[1]);

        [NSApplication sharedApplication]; [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        AppDelegate* d = [AppDelegate new]; [NSApp setDelegate:d]; buildMenu(d);
        NSRect frame = NSMakeRect(0, 0, 1280, 820);
        NSWindow* w = [[NSWindow alloc] initWithContentRect:frame
            styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
            backing:NSBackingStoreBuffered defer:NO];
        [w setTitle:@"EmberDragon"]; [w center]; [w setMinSize:NSMakeSize(820, 520)]; [w setAcceptsMouseMovedEvents:YES];
        NSOpenGLPixelFormatAttribute attrs[] = { NSOpenGLPFADoubleBuffer, NSOpenGLPFAColorSize, 24, NSOpenGLPFAAccelerated, 0 };
        NSOpenGLPixelFormat* pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
        EmberView* v = [[EmberView alloc] initWithFrame:frame pixelFormat:pf];
        [w setContentView:v]; [w makeFirstResponder:v]; d.win = w; d.view = v;
        // poll the terminal PTY + background optimize ~25x/s; redraw when there's new output
        [NSTimer scheduledTimerWithTimeInterval:0.04 repeats:YES block:^(NSTimer* t) { (void)t;
            static int lspE = 0, lspC = 0, lspD = 0, lspR = 0; int e = lsp::client.epoch.load(), ec = lsp::client.complEpoch.load(), ed = lsp::client.defEpoch.load(), er = lsp::client.renameEpoch.load(); bool lspChanged = (e != lspE || ec != lspC || ed != lspD || er != lspR); lspE = e; lspC = ec; lspD = ed; lspR = er;   // repaint when clangd diagnostics / completions / definition / rename land
            if (lspChanged || term::poll() || app::optimizing.load() || app::optJustFinished.load() || app::analyzing.load() || app::anaJustFinished.load() || app::aiBusy.load() || app::aiDone.load() || app::recompiling.load() || app::understanding.load() || app::understandDone.load()) [v setNeedsDisplay:YES]; }];
        [w makeKeyAndOrderFront:nil]; [NSApp activateIgnoringOtherApps:YES]; [NSApp run];
    }
    return 0;
}
