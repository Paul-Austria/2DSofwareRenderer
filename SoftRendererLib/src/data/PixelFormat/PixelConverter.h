#ifndef PIXELFORMATHANDLER
#define PIXELFORMATHANDLER

#include "PixelFormat.h"

#include <unordered_map>
#include <cstring>

class PixelConverter
{
public:
    using ConvertFunc = void (*)(const uint8_t *src, uint8_t *dst);

    struct Conversion
    {
        PixelFormat from;
        PixelFormat to;
        ConvertFunc func;
    };

    PixelConverter() : staticConversions(defaultConversions) {}
    ~PixelConverter();

    ConvertFunc *GetConvertFunction()
    {
        for (const auto &conversion : staticConversions)
        {
            if (conversion.from == from && conversion.to == to)
            {
                conversion.func(src, dst);
                return true;
            }
        }
    }

private:
    const Conversion *staticConversions;        // Pointer to static conversions
    std::vector<Conversion> dynamicConversions; // Vector for dynamically registered conversions

    // Conversion helper functions
    static void RGB24ToRGBA32(const uint8_t *src, uint8_t *dst)
    {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = 255; // Default alpha
    }

    static void RGBA32ToRGB24(const uint8_t *src, uint8_t *dst)
    {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
    }

    static void BGR24ToRGBA32(const uint8_t *src, uint8_t *dst)
    {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
        dst[3] = 255; // Default alpha
    }

    static void BGRA32ToRGB24(const uint8_t *src, uint8_t *dst)
    {
        dst[0] = src[2];
        dst[1] = src[1];
        dst[2] = src[0];
    }

    static void Grayscale8ToRGBA32(const uint8_t *src, uint8_t *dst)
    {
        dst[0] = dst[1] = dst[2] = src[0];
        dst[3] = 255; // Default alpha
    }

    static constexpr Conversion defaultConversions[] = {
        {PixelFormat::RGB24, PixelFormat::RGBA32, RGB24ToRGBA32},
        {PixelFormat::RGBA32, PixelFormat::RGB24, RGBA32ToRGB24},
        {PixelFormat::BGR24, PixelFormat::RGBA32, BGR24ToRGBA32},
        {PixelFormat::BGRA32, PixelFormat::RGB24, BGRA32ToRGB24},
        {PixelFormat::GRAYSCALE8, PixelFormat::RGBA32, Grayscale8ToRGBA32}
        // Additional static conversions can be added here
    };
};

#endif // !PIXELFORMATHANDLER
