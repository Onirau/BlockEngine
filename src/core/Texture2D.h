#include <bgfx/bgfx.h>

struct Texture2D {
    bgfx::TextureHandle handle;
    int width;
    int height;
};