#ifndef PIXELFORMAT_H
#define PIXELFORMAT_H

namespace Renderer2D{

enum class PixelFormat
{
    RGB24,         // 8 bits per channel: R, G, B or RGB888
    ARGB8888,
    RGBA1555,      // 16 bits: 5 bits R, 5 bits G, 5 bits B, 1 bit A
    GRAYSCALE8,    // 8 bits grayscale

    RGB565,        // 16 bits: 5 bits R, 6 bits G, 5 bits B
    RGBA4444,
};

}

#endif //  PIXELFORMAT_H
