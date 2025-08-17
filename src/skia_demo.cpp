#include <cstdint>
#include <iostream>

#include "common.h"

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

void drawNewString(SkCanvas* canvas) {
    // Use default FontConfig + scanner
    auto mgr = SkFontMgr_New_FontConfig(nullptr, SkFontScanner_Make_FreeType());

    sk_sp<SkTypeface> tf = mgr->matchFamilyStyle(
        "DejaVu Sans",
        SkFontStyle(SkFontStyle::kNormal_Weight,
                    SkFontStyle::kNormal_Width,
                    SkFontStyle::kUpright_Slant));

    if (!tf) {
        printf("No typeface found\n");
        return;
    }

    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    paint.setAntiAlias(true);

    SkFont font(tf, 32);
    canvas->drawSimpleText("Hello, Skia!", strlen("Hello, Skia!"),
                           SkTextEncoding::kUTF8,
                           50, 100, font, paint);
}

int main()
{

    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(300, 300)));
    SkCanvas* canvas = surface->getCanvas();

    // Clear
    canvas->clear(SK_ColorBLACK);

    // Draw string
    drawNewString(canvas);
    sk_sp<SkImage> img = canvas->getSurface()->makeImageSnapshot();

    if (!img) { return -1; }

    sk_sp<SkData> png = SkPngEncoder::Encode(nullptr, img.get(), SkPngEncoder::Options());
    if (!png) { return -1; }

    SkFILEWStream out(ResolveUserPath("output.png").c_str());
    out.write(png->data(), png->size());

    return 0;
}
