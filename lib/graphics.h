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
#include "include/effects/SkImageFilters.h"

#include "vec.h"
#include "common.h"
#include "sv_colorizer.h"
#include "sv.h"

namespace graphics {

enum ColorPalette {
    WHITE  = 0,
    RED,
    YELLOW,
    GREEN,
    BLUE,
    PURPLE
};
extern std::vector<Color> palette;

SkColor color_to_sk(Color color);

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

struct CodePanel {
    vec2  pos;
    vec2  size;
    float scrollY;
    bool  visible;
};

/**
 * @brief Initialize the window system and skia
 */
void initWindow(int width, int height, bool debug_counters = false);

/**
 * @brief Redraw the screen
 */
bool updateWindow(SV::Module* root, sv::ColorizedDoc& g_doc);

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
void drawNodeGraph(SkCanvas* canvas, SV::Module* root, std::string instance_name, vec2& root_position);

/**
 * @brief Render some code
 */
void renderSourceFile(SkCanvas* canvas, vec2 pos, const char* source_code, size_t scroll_line_number);


// replace your renderSourceFile with this variant:
void renderCodePanel(SkCanvas* canvas, const CodePanel& panel, const sv::ColorizedDoc& doc, SkFont& code_font);

}
