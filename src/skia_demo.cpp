#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <chrono>

#include "common.h"

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

static int frame = 0;

SkFont createNewFont() {
    // Use default FontConfig + scanner
    auto mgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());

    sk_sp<SkTypeface> tf = mgr->matchFamilyStyle(
        "DejaVu Sans",
        SkFontStyle(SkFontStyle::kNormal_Weight,
                    SkFontStyle::kNormal_Width,
                    SkFontStyle::kUpright_Slant));

    if (!tf) {
        throw std::runtime_error("No typeface found");
    }

    return SkFont(tf, 32);
}

void drawString(SkCanvas* canvas, SkFont& font) {
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(true);

    frame++;
    canvas->drawSimpleText("Hello, Skia!", strlen("Hello, Skia!"), SkTextEncoding::kUTF8, 50, 100, font, paint);
}

int main() {
    const int W = 800, H = 600;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* win = SDL_CreateWindow("Skia Live",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_SHOWN);
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* tex = SDL_CreateTexture(ren,
        SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);

    // BGRA matches SDL_PIXELFORMAT_ARGB8888 bytes on little-endian
    SkImageInfo info = SkImageInfo::Make(W, H,
        kBGRA_8888_SkColorType, kPremul_SkAlphaType);

    const size_t rowBytes = info.minRowBytes(); // == W * 4 for BGRA8888
    std::vector<uint8_t> pixels(rowBytes * H, 0);

    // NEW API: wrap your pixels
    sk_sp<SkSurface> surface = SkSurfaces::WrapPixels(info, pixels.data(), rowBytes);
    if (!surface) return 1;

    // Create font
    SkFont font = createNewFont();

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;

        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(SK_ColorBLACK);
        drawString(canvas, font);

        // upload pixels to the window
        SDL_UpdateTexture(tex, nullptr, pixels.data(), (int)rowBytes);
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);

        // TODO: Query for display refresh rate
        SDL_Delay(6); // ~165 fps
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
