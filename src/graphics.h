#pragma once

#include <vector>
#include <cstdint>

#include <SDL2/SDL.h>
#include "include/core/SkCanvas.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/encode/SkPngEncoder.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFont.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkFontMgr.h"
#include "include/ports/SkFontMgr_fontconfig.h"
#include "include/ports/SkFontMgr_empty.h"
#include "include/ports/SkFontScanner_FreeType.h"

#include "vec.h"
#include "common.h"

namespace graphics {

enum ColorPalette {
    RED    = 0,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE
};

struct Color {
    uint8_t r8, g8, b8, a8; // canonical storage

    // Constructors
    Color(uint8_t r=255, uint8_t g=255, uint8_t b=255, uint8_t a=255)
        : r8(r), g8(g), b8(b), a8(a) {}

    Color(float r, float g, float b, float a=1.0f) {
        r8 = static_cast<uint8_t>(r * 255.0f);
        g8 = static_cast<uint8_t>(g * 255.0f);
        b8 = static_cast<uint8_t>(b * 255.0f);
        a8 = static_cast<uint8_t>(a * 255.0f);
    }

    Color(uint32_t packed) {
        r8 = (packed >> 24) & 0xFF;
        g8 = (packed >> 16) & 0xFF;
        b8 = (packed >> 8)  & 0xFF;
        a8 = (packed)       & 0xFF;
    }

    // Accessors
    float r() const { return r8 / 255.0f; }
    float g() const { return g8 / 255.0f; }
    float b() const { return b8 / 255.0f; }
    float a() const { return a8 / 255.0f; }

    uint8_t& r() { return r8; }
    uint8_t& g() { return g8; }
    uint8_t& b() { return b8; }
    uint8_t& a() { return a8; }

    uint32_t rgba() const {
        return (uint32_t(r8) << 24) |
               (uint32_t(g8) << 16) |
               (uint32_t(b8) << 8)  |
               (uint32_t(a8));
    }

    SkColor sk_color() const {
        return (uint32_t(a8) << 24) |
               (uint32_t(r8) << 16) |
               (uint32_t(g8) << 8)  |
               (uint32_t(b8));
    }
};
extern std::vector<Color> palette;

struct Camera {
    // TODO: prob more
    vec2  pos   = vec2(0, 0);
    float scale = 1.0f;
};

struct WindowStructs {
    int width, height;

    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  fb_texture;

    size_t               rowBytes;
    std::vector<uint8_t> pixels;

    Camera camera;

    sk_sp<SkSurface> surface;
    SkFont           default_font;
    SkFont           dbg_font;

    ~WindowStructs() {
        SDL_DestroyTexture(fb_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }
};
static std::shared_ptr<WindowStructs> default_window;

typedef struct NodeGraph {
    NodeGraph*              parent = nullptr;
    std::vector<NodeGraph*> children;

    Color color    = Color(1.0f, 1.0f, 1.0f);
    vec2  rel_pos  = vec2(0, 0);
    vec2  rec_size = vec2(0, 0);

    ~NodeGraph() {
        for (auto child : children) {
            delete child;
        }
    }

    AABB GetAABB() const {
        AABB bb(vec2(0, 0), this->rec_size);
        for (auto& child : children) {
            AABB child_bb = child->GetAABB();
            child_bb.ul += child->rel_pos;
            child_bb.br += child->rel_pos;

            bb.ul.x = std::min(bb.ul.x, child_bb.ul.x);
            bb.ul.y = std::min(bb.ul.y, child_bb.ul.y);
            bb.br.x = std::max(bb.br.x, child_bb.br.x);
            bb.br.y = std::max(bb.br.y, child_bb.br.y);
        }

        return bb;
    }
} NodeGraph;

/**
 * @brief Initialize the window system and skia
 */
void initWindow(int width, int height, bool debug_counters = false);

/**
 * @brief Redraw the screen
 */
bool updateWindow(NodeGraph* graph);

/**
 * @brief Create a new font
 */
SkFont createNewFont(std::string font_name, int font_size);

/**
 * @brief Draw a string onto the canvas
 */
void drawString(SkCanvas* canvas, const char* text, vec2& pos, SkFont& font, Color color);

/**
 * @brief Draw a box onto the canvas
 */
void drawBox(SkCanvas* canvas, vec2& pos, vec2& size, Color color);

/**
 * @brief Draw a node graph
 */
void drawNodeGraph(SkCanvas* canvas, NodeGraph* root, vec2 root_position);

}
