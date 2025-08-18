#include "graphics.h"

namespace graphics {

static bool DEBUG_COUNTERS = false;

void initWindow(int width, int height, bool debug_counters) {
    DEBUG_COUNTERS = debug_counters;

    // Create default_window struct
    default_window = std::make_shared<WindowStructs>();
    default_window->width  = width;
    default_window->height = height;

    // Initialize SDL and Skia
    SDL_Init(SDL_INIT_VIDEO);
    default_window->window = SDL_CreateWindow("System Verilog Project Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    default_window->renderer = SDL_CreateRenderer(default_window->window, -1, SDL_RENDERER_ACCELERATED);
    default_window->fb_texture = SDL_CreateTexture(default_window->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    // BGRA matches SDL_PIXELFORMAT_ARGB8888 bytes on little-endian
    SkImageInfo info = SkImageInfo::Make(width, height, kBGRA_8888_SkColorType, kPremul_SkAlphaType);

    default_window->rowBytes = info.minRowBytes(); // == W * 4 for BGRA8888
    default_window->pixels   = std::vector<uint8_t>(default_window->rowBytes * height, 0);

    // NEW API: wrap your pixels
    default_window->surface = SkSurfaces::WrapPixels(info, default_window->pixels.data(), default_window->rowBytes);
    if (!default_window->surface) {
        throw std::runtime_error("Could not create surface");
    }

    // Create default font
    default_window->default_font = createNewFont("DejaVu Sans", 32);

    // Debug font
    default_window->dbg_font = createNewFont("DejaVu Sans", 20);
}

bool updateWindow() {
    static uint32_t startTime   = SDL_GetTicks();
    static float    fps         = 0.0f;
    static int      frame_count = 0;
    static std::string fps_string = "FPS: --";
    if (DEBUG_COUNTERS)
        frame_count++;

    static bool running = true;
    SDL_Event e;

    while (SDL_PollEvent(&e)) if (e.type == SDL_QUIT) running = false;

    SkCanvas* canvas = default_window->surface->getCanvas();
    canvas->clear(0xFF1B1C1D);
    
    vec2 rec_pos(150, 150);
    vec2 rec_size(400, 200);
    drawBox(canvas, rec_pos, rec_size, 0xff4285F4);

    vec2 text_pos(250, 250);
    drawString(canvas, "Hello World!", text_pos, default_window->default_font, SK_ColorWHITE);

    if (DEBUG_COUNTERS) {
        // FPS Counter
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - startTime >= 1000) {
            fps = frame_count * 1000.0f / (currentTime - startTime);
            frame_count = 0;
            startTime = currentTime;
            fps_string = "FPS: " + std::to_string(int(fps));        
        }
        // FPS Counter Debug Counter
        vec2 fps_counter_pos(10, 20);
        drawString(canvas, fps_string.c_str(), fps_counter_pos, default_window->dbg_font, SK_ColorWHITE);
    }
    
    // upload pixels to the window
    SDL_UpdateTexture(default_window->fb_texture, nullptr, default_window->pixels.data(), (int)default_window->rowBytes);
    SDL_RenderClear(default_window->renderer);
    SDL_RenderCopy(default_window->renderer, default_window->fb_texture, nullptr, nullptr);
    SDL_RenderPresent(default_window->renderer);
    
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

void drawBox(SkCanvas* canvas, vec2& pos, vec2& size, SkColor color) {
    SkPaint paint;
    paint.setColor(color);
    paint.setAntiAlias(true);
    canvas->drawRoundRect(SkRect::MakeXYWH(pos.x, pos.y, size.x, size.y), 10, 10, paint);
}

}
