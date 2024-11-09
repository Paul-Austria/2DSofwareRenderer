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

        // Channel offsets
        uint8_t redOffset;   // Offset for the red channel in bytes
        uint8_t greenOffset; // Offset for the green channel in bytes
        uint8_t blueOffset;  // Offset for the blue channel in bytes
        uint8_t alphaOffset; // Offset for the alpha channel (if present)

        PixelFormatInfo() = default;
        PixelFormatInfo(PixelFormat format, int bpp, int channels, bool alpha, const char *name,
                        uint8_t redOffset, uint8_t greenOffset, uint8_t blueOffset, uint8_t alphaOffset = 0)
            : format(format), bytesPerPixel(bpp), numChannels(channels), hasAlpha(alpha), name(name),
              redOffset(redOffset), greenOffset(greenOffset), blueOffset(blueOffset), alphaOffset(alphaOffset) {}
    };

    class PixelFormatRegistry
    {
    public:
        static const PixelFormatInfo &GetInfo(PixelFormat format)
        {
            static std::unordered_map<PixelFormat, PixelFormatInfo> formatInfoMap = {
                {PixelFormat::RGB24, {PixelFormat::RGB24, 3, 3, false, "RGB24", 0, 1, 2}},
                {PixelFormat::ARGB8888, {PixelFormat::ARGB8888, 4, 4, true, "ARGB8888", 1, 2, 3, 0}},
                {PixelFormat::RGBA8888, {PixelFormat::RGBA8888, 4, 4, true, "RGBA8888", 0, 1, 2, 3}},
                {PixelFormat::RGBA1555, {PixelFormat::RGBA1555, 2, 4, true, "RGBA1555", 0, 1, 2, 3}},
                {PixelFormat::GRAYSCALE8, {PixelFormat::GRAYSCALE8, 1, 1, false, "Grayscale8", 0, 0, 0}},
                {PixelFormat::RGB565, {PixelFormat::RGB565, 2, 3, false, "RGB565", 0, 1, 2}},
                {PixelFormat::RGBA4444, {PixelFormat::RGBA4444, 2, 4, true, "RGBA4444", 0, 1, 2, 3}}};

            return formatInfoMap.at(format);
        }
    };

}

#endif // PIXELFORMATINFO_H
