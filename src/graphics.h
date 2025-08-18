#pragma once

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

#include "common.h"

namespace graphics {

struct WindowStructs {
    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  fb_texture;

    size_t               rowBytes;
    std::vector<uint8_t> pixels;

    sk_sp<SkSurface> surface;
    SkFont           default_font;

    ~WindowStructs() {
        SDL_DestroyTexture(fb_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }
};
static std::shared_ptr<WindowStructs> default_window;

/**
 * @brief Initialize the window system and skia
 */
void initWindow();

/**
 * @brief Get default canvas
 */
SkCanvas* getCanvas();

/**
 * @brief Redraw the screen
 */
bool updateWindow();

/**
 * @brief Create a new font
 */
SkFont createNewFont(std::string font_name, int font_size);

/**
 * @brief Draw a string onto the canvas
 */
void drawString(SkCanvas* canvas, const char* text, vec2& pos, SkFont& font, SkColor color);

}
