// ember_image.h — load a PNG/JPG into a GL texture (stb_image) and draw it as a
// quad. Used for the real EmberDragon logo on the home screen + toolbar.
#ifndef EMBER_IMAGE_H
#define EMBER_IMAGE_H
#if defined(_WIN32)
  #include <windows.h>
  #include <GL/gl.h>
#elif defined(__APPLE__)
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#define STB_IMAGE_IMPLEMENTATION
#include "corpus/code_web/stb_image.h"
#include <string>

namespace img {
struct Tex { GLuint id = 0; int w = 0, h = 0; };
inline Tex load(const std::string& path) {
    Tex t; int n = 0; unsigned char* d = stbi_load(path.c_str(), &t.w, &t.h, &n, 4);
    if (!d) { t.id = 0; return t; }
    glGenTextures(1, &t.id); glBindTexture(GL_TEXTURE_2D, t.id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t.w, t.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, d);
    stbi_image_free(d); return t;
}
inline void draw(const Tex& t, float x, float y, float w, float h) {     // y-down ortho; image drawn upright
    if (!t.id) return;
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, t.id);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glColor4f(1, 1, 1, 1);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(x, y);       glTexCoord2f(1, 0); glVertex2f(x + w, y);
    glTexCoord2f(1, 1); glVertex2f(x + w, y + h); glTexCoord2f(0, 1); glVertex2f(x, y + h);
    glEnd();
    glDisable(GL_TEXTURE_2D); glDisable(GL_BLEND);
}
} // namespace img
#endif
