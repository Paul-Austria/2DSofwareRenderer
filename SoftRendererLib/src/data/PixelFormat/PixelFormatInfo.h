#ifndef PIXELFORMATINFO_H
#define PIXELFORMATINFO_H

#include "PixelFormat.h"
#include <unordered_map>
#include <cstdint>
#include "../Color.h"
#include <cstdint>
namespace Renderer2D
{

    struct PixelFormatInfo
    {
        PixelFormat format;    // The pixel format
        uint8_t bytesPerPixel; // Number of bytes per pixel
        uint8_t bitsPerPixel;  // how many bits per pixel
        bool isBitFormat;       // uses less then one byte per pixel (grayscale 4 or grayscale 1)
        uint8_t numChannels;   // Number of color channels
        bool hasAlpha;         // Whether the format includes an alpha channel
        const char *name;      // A human-readable name for the format

        // Bit masks and shifts for each channel
        uint16_t redMask, greenMask, blueMask, alphaMask;
        uint8_t redShift, greenShift, blueShift, alphaShift;

        PixelFormatInfo() = default;

        PixelFormatInfo(PixelFormat format, uint8_t bpp,uint8_t bitspp, bool isBitFormat, uint8_t channels, bool alpha, const char *name,
                        uint16_t redMask, uint8_t redShift,
                        uint16_t greenMask, uint8_t greenShift,
                        uint16_t blueMask, uint8_t blueShift,
                        uint16_t alphaMask = 0, uint8_t alphaShift = 0)
            : format(format), bytesPerPixel(bpp), bitsPerPixel(bitspp),isBitFormat(isBitFormat) ,numChannels(channels), hasAlpha(alpha), name(name),
              redMask(redMask), redShift(redShift),
              greenMask(greenMask), greenShift(greenShift),
              blueMask(blueMask), blueShift(blueShift),
              alphaMask(alphaMask), alphaShift(alphaShift) {
                if(isBitFormat)
                {
                    bytesPerPixel = bitsPerPixel % 8;
                }else
                {
                    bitsPerPixel = bytesPerPixel*8;
                }
              }


    private:

    };

    class PixelFormatRegistry
    {
    public:
        static const PixelFormatInfo &GetInfo(PixelFormat format)
        {
            static std::unordered_map<PixelFormat, PixelFormatInfo> formatInfoMap = {
                {PixelFormat::RGB24, {PixelFormat::RGB24, 3,0,false, 3, false, "RGB24", 0xFF, 0, 0xFF, 8, 0xFF, 16}},
                {PixelFormat::BGR24, {PixelFormat::BGR24, 3,0,false, 3, false, "BGR24", 0xFF, 16, 0xFF, 8, 0xFF, 0}},

                {PixelFormat::ARGB8888, {PixelFormat::ARGB8888, 4,0,false, 4, true, "ARGB8888", 0xFF, 16, 0xFF, 8, 0xFF, 0, 0xFF, 0}},
                {PixelFormat::RGBA8888, {PixelFormat::RGBA8888, 4,0,false, 4, true, "RGBA8888", 0xFF, 24, 0xFF, 16, 0xFF, 8, 0xFF, 24}},
                {PixelFormat::ARGB1555, {PixelFormat::ARGB1555, 2,0,false, 4, true, "RGBA1555", 0x7C00, 10, 0x03E0, 5, 0x001F, 0, 0x8000, 15}},
                {PixelFormat::RGB565, {PixelFormat::RGB565, 2,0,false, 3, false, "RGB565", 0xF800, 11, 0x07E0, 5, 0x001F, 0}},
                {PixelFormat::RGBA4444, {PixelFormat::RGBA4444, 2,0,false, 4, true, "RGBA4444", 0xF000, 12, 0x0F00, 8, 0x00F0, 4, 0x000F, 0}},
                {PixelFormat::GRAYSCALE8, {PixelFormat::GRAYSCALE8, 1,0,false, 1, true, "Grayscale8", 0xFF, 0, 0x00, 0, 0x00, 0}},
                {PixelFormat::GRAYSCALE4, {PixelFormat::GRAYSCALE4, 0, 4,true, 1, false, "Grayscale4", 0xF, 0, 0, 0, 0, 0}},  
                {PixelFormat::GRAYSCALE1, {PixelFormat::GRAYSCALE1, 0, 1,true, 1, false, "Grayscale1", 0x1, 0, 0, 0, 0, 0}},  
               
            };

            return formatInfoMap.at(format);
        }
    };

}

#endif // PIXELFORMATINFO_H
