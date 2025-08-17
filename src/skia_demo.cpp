#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"

int main() {
    // Create raster surface (800x600)
    SkImageInfo info = SkImageInfo::MakeN32Premul(800, 600);
    auto surface = SkSurfaces::Raster(info);
    if (!surface) return 1;

    SkCanvas* canvas = surface->getCanvas();

    // Clear background
    canvas->clear(SK_ColorWHITE);

    // Draw a rectangle
    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(3);
    SkRect rect = SkRect::MakeXYWH(100, 100, 200, 100);
    canvas->drawRect(rect, paint);

    // Draw a line
    paint.setColor(SK_ColorBLACK);
    paint.setStrokeWidth(2);
    canvas->drawLine(200, 200, 300, 300, paint);

    // Snapshot image
    sk_sp<SkImage> image(surface->makeImageSnapshot());

    // Encode as PNG
    SkPngEncoder::Options options;
    options.fZLibLevel = 9; // max compression
    sk_sp<SkData> png = SkPngEncoder::Encode(nullptr, image.get(), options);

    if (png) {
        SkFILEWStream out("output.png");
        out.write(png->data(), png->size());
    }

    return 0;
}
