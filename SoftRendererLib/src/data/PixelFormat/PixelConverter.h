#ifndef PIXELCONVERTER_H
#define PIXELCONVERTER_H

#include "PixelFormat.h"
#include <cstring>
#include <stdexcept>

namespace Renderer2D
{
    class PixelConverter
    {
    public:
        using ConvertFunc = void (*)(const uint8_t* src, uint8_t* dst, size_t count);

        // Get the conversion function from one format to another
        ConvertFunc GetConversionFunction(PixelFormat from, PixelFormat to) const;

        // Convert pixels using cached function pointer and batch processing
        void Convert(PixelFormat from, PixelFormat to, const uint8_t* src, uint8_t* dst, size_t count = 1) const;

    private:
        struct Conversion {
            PixelFormat from;
            PixelFormat to;
            ConvertFunc func;
        };

        // Optimized conversion functions
        static void RGB24ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count);
        static void RGBA32ToRGB24(const uint8_t* src, uint8_t* dst, size_t count);
        static void BGR24ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count);
        static void BGRA32ToRGB24(const uint8_t* src, uint8_t* dst, size_t count);
        static void Grayscale8ToRGBA32(const uint8_t* src, uint8_t* dst, size_t count);
        static void Grayscale8ToRGB24(const uint8_t* src, uint8_t* dst, size_t count);
        static void RGBA32ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count);
        static void RGB24ToGrayscale8(const uint8_t* src, uint8_t* dst, size_t count);

        // Conversion mappings
        static constexpr Conversion defaultConversions[] = {
            {PixelFormat::RGB24, PixelFormat::RGBA32, RGB24ToRGBA32},
            {PixelFormat::RGBA32, PixelFormat::RGB24, RGBA32ToRGB24},
            {PixelFormat::BGR24, PixelFormat::RGBA32, BGR24ToRGBA32},
            {PixelFormat::BGRA32, PixelFormat::RGB24, BGRA32ToRGB24},
            {PixelFormat::GRAYSCALE8, PixelFormat::RGBA32, Grayscale8ToRGBA32},
            {PixelFormat::GRAYSCALE8, PixelFormat::RGB24, Grayscale8ToRGB24},
            {PixelFormat::RGBA32, PixelFormat::GRAYSCALE8, RGBA32ToGrayscale8},
            {PixelFormat::RGB24, PixelFormat::GRAYSCALE8, RGB24ToGrayscale8}
            // Add more conversions as necessary
        };
    };
}

#endif // PIXELCONVERTERNEW_H
