#include "graphics.h"

namespace graphics {

void initWindow() {
    // Create default_window struct
    default_window = std::make_shared<WindowStructs>();

    // Initialize SDL and Skia
    const int W = 800, H = 600;

    SDL_Init(SDL_INIT_VIDEO);
    default_window->window = SDL_CreateWindow("Skia Live", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W, H, SDL_WINDOW_SHOWN);
    default_window->renderer = SDL_CreateRenderer(default_window->window, -1, SDL_RENDERER_ACCELERATED);
    default_window->fb_texture = SDL_CreateTexture(default_window->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, W, H);

    // BGRA matches SDL_PIXELFORMAT_ARGB8888 bytes on little-endian
    SkImageInfo info = SkImageInfo::Make(W, H, kBGRA_8888_SkColorType, kPremul_SkAlphaType);

    default_window->rowBytes = info.minRowBytes(); // == W * 4 for BGRA8888
    default_window->pixels   = std::vector<uint8_t>(default_window->rowBytes * H, 0);

    // NEW API: wrap your pixels
    default_window->surface = SkSurfaces::WrapPixels(info, default_window->pixels.data(), default_window->rowBytes);
    if (!default_window->surface) {
        throw std::runtime_error("Could not create surface");
    }

    // Create default font
    default_window->default_font = createNewFont("DejaVu Sans", 32);
}

bool updateWindow() {
    static bool running = true;
    SDL_Event e;

    while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;

    SkCanvas* canvas = default_window->surface->getCanvas();
    canvas->clear(0xFF1B1C1D);

    vec2 text_pos(50, 100);
    drawString(canvas, "Hello World!", text_pos, default_window->default_font, SK_ColorWHITE);
    
    // upload pixels to the window
    SDL_UpdateTexture(default_window->fb_texture, nullptr, default_window->pixels.data(), (int)default_window->rowBytes);
    SDL_RenderClear(default_window->renderer);
    SDL_RenderCopy(default_window->renderer, default_window->fb_texture, nullptr, nullptr);
    SDL_RenderPresent(default_window->renderer);
    
    SDL_Delay(6); // ~165 FPS
    
    if (running == false)
        SDL_Quit();

    return running;
}

SkFont createNewFont(std::string font_name, int font_size) {
    // Use default FontConfig + scanner
    auto mgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());

    sk_sp<SkTypeface> tf = mgr->matchFamilyStyle(
        font_name.c_str(),
        SkFontStyle(SkFontStyle::kNormal_Weight,
                    SkFontStyle::kNormal_Width,
                    SkFontStyle::kUpright_Slant));

    if (!tf) {
        throw std::runtime_error("No typeface found");
    }

    return SkFont(tf, font_size);

}

void drawString(SkCanvas* canvas, const char* text, vec2& pos, SkFont& font, SkColor color) {
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, pos.x, pos.y, font, paint);
}

}
