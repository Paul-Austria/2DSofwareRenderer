#ifndef PIXELFORMATHANDLER_H
#define PIXELFORMATHANDLER_H

#include "PixelFormat.h"
#include <cstring>
#include <stdexcept>

namespace Renderer2D
{

class PixelConverter
{
public:
    using ConvertFunc = void (*)(const uint8_t *src, uint8_t *dst);

    // Get the conversion function from one format to another
    ConvertFunc GetConversionFunction(PixelFormat from, PixelFormat to) const {
        // Search through static conversions
        for (const auto& conversion : defaultConversions) {
            if (conversion.from == from && conversion.to == to) {
                return conversion.func;
            }
        }
        // If no conversion is found, throw an error
        throw std::runtime_error("Conversion function not found between these formats.");
    }

    // Convert pixels using the pre-registered static conversions
    void Convert(PixelFormat from, PixelFormat to, const uint8_t* src, uint8_t* dst) const {
        ConvertFunc func = GetConversionFunction(from, to);
        func(src, dst);  // Call the conversion function directly
    }

private:
    struct Conversion {
        PixelFormat from;
        PixelFormat to;
        ConvertFunc func;
    };

    // Conversion helper functions (static)
    static void RGB24ToRGBA32(const uint8_t* src, uint8_t* dst) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = 255; // Default alpha
    }

    static void RGBA32ToRGB24(const uint8_t* src, uint8_t* dst) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }

    static void BGR24ToRGBA32(const uint8_t* src, uint8_t* dst) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        dst[3] = 255; // Default alpha
    }

    static void BGRA32ToRGB24(const uint8_t* src, uint8_t* dst) {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
    }

    static void Grayscale8ToRGBA32(const uint8_t* src, uint8_t* dst) {
        dst[0] = dst[1] = dst[2] = src[0];
        dst[3] = 255; // Default alpha
    }

    // Static array of conversions
    static constexpr Conversion defaultConversions[] = {
        {PixelFormat::RGB24, PixelFormat::RGBA32, RGB24ToRGBA32},
        {PixelFormat::RGBA32, PixelFormat::RGB24, RGBA32ToRGB24},
        {PixelFormat::BGR24, PixelFormat::RGBA32, BGR24ToRGBA32},
        {PixelFormat::BGRA32, PixelFormat::RGB24, BGRA32ToRGB24},
        {PixelFormat::GRAYSCALE8, PixelFormat::RGBA32, Grayscale8ToRGBA32}
        // Add more static conversions here if needed
    };
};

}

#endif // !PIXELFORMATHANDLER_H
