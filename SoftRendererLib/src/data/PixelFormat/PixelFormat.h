#ifndef PIXELFORMAT_H
#define PIXELFORMAT_H

namespace Tergos2D
{

    enum class PixelFormat
    {
        RGB24 = 0, // 8 bits per channel: R, G, B or RGB888
        BGR24 = 1,
        ARGB8888 = 2,
        RGBA8888 = 3,
        ARGB1555 = 4, // 16 bits: 5 bits R, 5 bits G, 5 bits B, 1 bit A
        RGB565 = 5, // 16 bits: 5 bits R, 6 bits G, 5 bits B
        RGBA4444 = 6,
        GRAYSCALE8 = 7, // 8 bits grayscale
        COUNT  = 9
    };

}

#endif //  PIXELFORMAT_H
