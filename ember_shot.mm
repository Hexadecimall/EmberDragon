// ember_shot.mm — headless render of the EmberDragon workspace to a PNG, for
// verifying the GUI without a window. Usage: ember-shot <binary> [view] [out.png]
#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#include "ember_app.h"
#include <unistd.h>
#include <vector>

static void writePNG(const char* path, int W, int H, unsigned char* rgba) {
    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:W pixelsHigh:H
        bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:W*4 bitsPerPixel:32];
    unsigned char* d = [rep bitmapData];
    for (int y = 0; y < H; y++) memcpy(d + y*W*4, rgba + (H-1-y)*W*4, W*4);   // GL is bottom-up
    NSData* png = [rep representationUsingType:NSBitmapImageFileTypePNG properties:@{}];
    [png writeToFile:[NSString stringWithUTF8String:path] atomically:YES];
}

int main(int argc, const char** argv) {
    @autoreleasepool {
        int W = 1280, H = 820;
        CGLPixelFormatAttribute attrs[] = { kCGLPFAAccelerated, kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_Legacy, (CGLPixelFormatAttribute)0 };
        CGLPixelFormatObj pf; GLint np; CGLChoosePixelFormat(attrs, &pf, &np);
        CGLContextObj ctx; CGLCreateContext(pf, NULL, &ctx); CGLSetCurrentContext(ctx);
        GLuint fbo, rb; glGenFramebuffersEXT(1, &fbo); glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
        glGenRenderbuffersEXT(1, &rb); glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, W, H);
        glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, rb);

        ef::load({ "/System/Library/Fonts/SFNSMono.ttf", "/System/Library/Fonts/Menlo.ttc" }); ef::bake(13);
        ui::winW = W; ui::winH = H; ui::in.scale = 1;
        std::string dir = "/Users/gideoncox/pullio/decompile/";
        app::init(dir); app::setClipboard = [](const std::string&) {};
        if (argc > 1) { app::openBinary(argv[1]); for (int t = 0; t < 600 && app::analyzing.load(); t++) { usleep(20000); app::pumpAnalysis(); } }   // pump each tick so the worker's finalize runs
        if (argc > 2) { std::string v = argv[2]; if (v == "disasm") app::g.mainTab = app::MAIN_DISASM; else if (v == "hex") app::g.mainTab = app::MAIN_HEX; else if (v == "graph") app::g.mainTab = app::MAIN_GRAPH; else if (v == "settings") app::g.settingsOpen = true;
            else if (v == "files") { app::g.sideTab = app::SIDE_FILES; app::g.aiOptIn = false; } }
        for (int i = 0; i < (int)app::g.bin.funcs.size(); i++) if (app::g.bin.funcs[i].name == "main") { app::g.selFn = i; break; }   // show main
        for (int i = 0; i < 2; i++) {   // immediate-mode: a second frame settles layout
            glViewport(0, 0, W, H); glMatrixMode(GL_PROJECTION); glLoadIdentity(); glOrtho(0, W, H, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW); glLoadIdentity(); glDisable(GL_DEPTH_TEST);
            app::render(); ui::in.clearOneShot(); glFlush();
        }
        std::vector<unsigned char> px(W*H*4); glReadPixels(0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, px.data());
        writePNG(argc > 3 ? argv[3] : "/tmp/ember_shot.png", W, H, px.data());
        fprintf(stderr, "wrote %s  (%d functions)\n", argc > 3 ? argv[3] : "/tmp/ember_shot.png", (int)app::g.bin.funcs.size());
    }
    return 0;
}
