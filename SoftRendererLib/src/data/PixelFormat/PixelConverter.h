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
        using ConvertFunc = void (*)(const uint8_t *src, uint8_t *dst, size_t count);

        // Get the conversion function from one format to another
        static ConvertFunc GetConversionFunction(PixelFormat from, PixelFormat to);

        // Convert pixels using cached function pointer and batch processing
        void Convert(PixelFormat from, PixelFormat to, const uint8_t *src, uint8_t *dst, size_t count = 1) const;

    private:
        struct Conversion
        {
            PixelFormat from;
            PixelFormat to;
            ConvertFunc func;
        };

        // Optimized conversion functions
        static void RGB24ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGB565(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA1555ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGBA1555(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA4444ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGBA4444(const uint8_t *src, uint8_t *dst, size_t count);
        static void Grayscale8ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB24ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToGrayscale8(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA8888ToARGB8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void ARGB8888ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB24ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGB565ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA1555ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA4444ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void Grayscale8ToRGBA8888(const uint8_t *src, uint8_t *dst, size_t count);
        static void RGBA8888ToRGB24(const uint8_t *src, uint8_t *dst, size_t count);

        // Conversion mappings
        static constexpr Conversion defaultConversions[] = {
            {PixelFormat::RGB24, PixelFormat::ARGB8888, RGB24ToARGB8888},
            {PixelFormat::ARGB8888, PixelFormat::RGB24, ARGB8888ToRGB24},
            {PixelFormat::RGB565, PixelFormat::ARGB8888, RGB565ToARGB8888},
            {PixelFormat::ARGB8888, PixelFormat::RGB565, ARGB8888ToRGB565},
            {PixelFormat::RGBA1555, PixelFormat::ARGB8888, RGBA1555ToARGB8888},
            {PixelFormat::ARGB8888, PixelFormat::RGBA1555, ARGB8888ToRGBA1555},
            {PixelFormat::ARGB8888, PixelFormat::RGBA8888, ARGB8888ToRGBA8888},
            {PixelFormat::RGBA8888, PixelFormat::ARGB8888, RGBA8888ToARGB8888},
            {PixelFormat::RGBA4444, PixelFormat::ARGB8888, RGBA4444ToARGB8888},
            {PixelFormat::ARGB8888, PixelFormat::RGBA4444, ARGB8888ToRGBA4444},
            {PixelFormat::GRAYSCALE8, PixelFormat::ARGB8888, Grayscale8ToARGB8888},
            {PixelFormat::ARGB8888, PixelFormat::GRAYSCALE8, ARGB8888ToGrayscale8},
            {PixelFormat::RGB24, PixelFormat::GRAYSCALE8, RGB24ToGrayscale8},
            {PixelFormat::RGB565, PixelFormat::GRAYSCALE8, RGB565ToGrayscale8},
            {PixelFormat::RGB24, PixelFormat::RGBA8888, RGB24ToRGBA8888},
            {PixelFormat::RGB565, PixelFormat::RGB24, RGB565ToRGB24},
            {PixelFormat::RGB565, PixelFormat::RGBA8888, RGB565ToRGBA8888},
            {PixelFormat::RGBA1555, PixelFormat::RGBA8888, RGBA1555ToRGBA8888},
            {PixelFormat::RGBA4444, PixelFormat::RGBA8888, RGBA4444ToRGBA8888},
            {PixelFormat::GRAYSCALE8, PixelFormat::RGBA8888, Grayscale8ToRGBA8888},
            {PixelFormat::RGBA8888, PixelFormat::RGB24, RGBA8888ToRGB24}
            // Add more conversions as necessary
        };
    };
}

#endif // PIXELCONVERTER_H
