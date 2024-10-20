#ifndef PIXELFORMATINFO_H
#define PIXELFORMATINFO_H

#include "PixelFormat.h"
#include <unordered_map>
#include <cstdint>

namespace Renderer2D
{

    struct PixelFormatInfo
    {
        PixelFormat format;    // The pixel format
        uint8_t bytesPerPixel; // Number of bytes per pixel
        uint8_t numChannels;   // Number of color channels
        bool hasAlpha;         // Whether the format includes an alpha channel
        const char *name;      // A human-readable name for the format

        PixelFormatInfo(PixelFormat format, int bpp, int channels, bool alpha, const char *name)
            : format(format), bytesPerPixel(bpp), numChannels(channels), hasAlpha(alpha), name(name) {}
    };

    class PixelFormatRegistry
    {
    public:
        static const PixelFormatInfo &GetInfo(PixelFormat format)
        {
            static std::unordered_map<PixelFormat, PixelFormatInfo> formatInfoMap = {
                {PixelFormat::RGB24, {PixelFormat::RGB24, 3, 3, false, "RGB24"}},
                {PixelFormat::RGBA32, {PixelFormat::RGBA32, 4, 4, true, "RGBA32"}},
                {PixelFormat::BGR24, {PixelFormat::BGR24, 3, 3, false, "BGR24"}},
                {PixelFormat::BGRA32, {PixelFormat::BGRA32, 4, 4, true, "BGRA32"}},
                {PixelFormat::GRAYSCALE8, {PixelFormat::GRAYSCALE8, 1, 1, false, "Grayscale8"}}};

            return formatInfoMap.at(format);
        }
    };

}

#endif // PIXELFORMATINFO_H
