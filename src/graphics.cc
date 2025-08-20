#include "graphics.h"

namespace graphics {

static bool DEBUG_COUNTERS = false;

const float scroll_sensitivity = 20.0f;

std::vector<Color> palette = {
    Color(0xFF595EFFu),
    Color(0xFFCA3AFFu),
    Color(0x8AC926FFu),
    Color(0x1982C4FFu),
    Color(0x6A4C93FFu)
};

void initWindow(int width, int height, bool debug_counters) {
    DEBUG_COUNTERS = debug_counters;

    // Create default_window struct
    default_window = std::make_shared<WindowStructs>();
    default_window->width  = width;
    default_window->height = height;

    // Initialize SDL and Skia
    SDL_Init(SDL_INIT_VIDEO);
    default_window->window = SDL_CreateWindow("System Verilog Project Viewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    default_window->renderer = SDL_CreateRenderer(default_window->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); // Enabled VSync
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

    // Init camera
    default_window->camera.pos   = vec2(0.f, 0.f);
    default_window->camera.scale = 1.f;

    // Create default font
    default_window->default_font = createNewFont("DejaVu Sans", 32);

    // Debug font
    default_window->dbg_font = createNewFont("DejaVu Sans", 20);
}

inline vec2 worldToScreen(const vec2& w, const Camera& cam) {
    // screen = (world - cam.pos) * cam.scale
    return (w - cam.pos) * cam.scale;
}
inline vec2 screenToWorld(const vec2& s, const Camera& cam) {
    // world = s / cam.scale + cam.pos
    return s / cam.scale + cam.pos;
}

inline vec2 getMouse() {
    int mx, my; SDL_GetMouseState(&mx, &my); return {float(mx), float(my)};
}

bool updateWindow(NodeGraph* graph) {
    static uint32_t startTime   = SDL_GetTicks();
    static float    fps         = 0.0f;
    static int      frame_count = 0;
    static std::string fps_string = "FPS: --";
    if (DEBUG_COUNTERS)
        frame_count++;

    static bool running = true;

    SDL_Event e;
    static bool dragging = false;
    static vec2 lastMouse(0, 0);
    const float minScale = 0.05f;
    const float maxScale = 50.0f;
    const float zoomStep = 1.1f; // 10% per wheel notch

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = false;

        // TODO: Modify the position of the camera based on either shift + mouse click and drag, or 
        // scrolling/double finger touchpad movement. Zoom camera with ctrl + scolling
        // Start/stop LMB drag panningc
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            dragging = true;
            lastMouse = {float(e.button.x), float(e.button.y)};
        }
        if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
            dragging = false;
        }

        // Drag to pan (scale-aware)
        if (e.type == SDL_MOUSEMOTION && dragging) {
            vec2 now{float(e.motion.x), float(e.motion.y)};
            vec2 d = now - lastMouse;
            default_window->camera.pos -= vec2(d) / default_window->camera.scale; // minus => screen drag matches world move
            lastMouse = now;
        }

        // Wheel / touchpad: pan vs zoom (Ctrl = zoom, otherwise pan)
        if (e.type == SDL_MOUSEWHEEL) {
            // SDL on some platforms flips sign; account for that
            const int flip = (e.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1;

            // Prefer precise deltas when available (trackpads), else integer steps
            const float wx = (e.wheel.preciseX != 0.0f) ? e.wheel.preciseX : (float)e.wheel.x;
            const float wy = (e.wheel.preciseY != 0.0f) ? e.wheel.preciseY : (float)e.wheel.y;

            const bool ctrl  = (SDL_GetModState() & KMOD_CTRL) != 0;
            const bool shift = (SDL_GetModState() & KMOD_SHIFT) != 0;


            if (ctrl) {
                // --- Zoom about mouse cursor ---
                vec2 mouse = getMouse();                                // screen coords
                vec2 anchorWorld = screenToWorld(mouse, default_window->camera);

                float factor   = std::pow(zoomStep, flip * wy);
                float newScale = std::clamp(default_window->camera.scale * factor, minScale, maxScale);

                // Keep the world point under the cursor fixed:
                // anchorWorld = mouse / newScale + newPos  =>  newPos = anchorWorld - mouse / newScale
                default_window->camera.scale = newScale;
                default_window->camera.pos   = anchorWorld - mouse / newScale;
            } else {
                // --- Pan with wheel/trackpad ---
                const bool shift = (SDL_GetModState() & KMOD_SHIFT) != 0;

                vec2 deltaScreen{wx * flip, -wy * flip}; // natural feel
                vec2 deltaWorld = (shift
                        ? vec2(-wy * flip, 0.f)   // Shift+wheel = horizontal pan with vertical wheel
                        : deltaScreen) / default_window->camera.scale * scroll_sensitivity;

                // Move camera opposite of finger/scroll to make content follow the gesture
                default_window->camera.pos += deltaWorld;
            }
        }
    }

    // Clear screen
    SkCanvas* canvas = default_window->surface->getCanvas();
    canvas->clear(0xFF1B1C1D);
    
    // Draw the graphc
    drawNodeGraph(canvas, graph, vec2(0, 0));

    // Draw FPS counter if enabled
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

void drawString(SkCanvas* canvas, const char* text, vec2& pos, SkFont& font, Color color) {
    SkPaint paint;
    paint.setColor(color.sk_color());
    paint.setAntiAlias(true);
    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, pos.x, pos.y, font, paint);
}

void drawBox(SkCanvas* canvas, vec2& pos, vec2& size, Color color) {
    SkPaint paint;
    paint.setColor(color.sk_color());
    paint.setAntiAlias(true);

    vec2 sp = worldToScreen(pos, default_window->camera);
    vec2 ss = size * default_window->camera.scale;

    auto rect = SkRect::MakeXYWH(sp.x, sp.y, ss.x, ss.y);
    canvas->drawRoundRect(rect, 10, 10, paint);
}

void drawNodeGraph(SkCanvas* canvas, NodeGraph* root, vec2 root_position) {
    vec2 pos = root_position + root->rel_pos;
    if (root->rec_size != vec2(0, 0) && root->color.a() != 0) {
        drawBox(canvas, pos, root->rec_size, root->color);
    }

    for (size_t i = 0; i < root->children.size(); i++) {
        drawNodeGraph(canvas, root->children[i], pos);
    }
}

}
