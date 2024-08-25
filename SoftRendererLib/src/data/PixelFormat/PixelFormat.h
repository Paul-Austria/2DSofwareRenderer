#ifndef PIXELFORMAT_H
#define PIXELFORMAT_H

enum class PixelFormat
{
    RGB24,   // 8 bits per channel: R, G, B
    RGBA32,  // 8 bits per channel: R, G, B, A
    BGR24,   // 8 bits per channel: B, G, R
    BGRA32,  // 8 bits per channel: B, G, R, A
    GRAYSCALE8, // 8 bits grayscale
};



#endif //  PIXELFORMAT_H
