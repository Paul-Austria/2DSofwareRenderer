#ifndef PIXELFORMATINFO_H
#define PIXELFORMATINFO_H

#include "PixelFormat.h"
#include <unordered_map>
#include "../Color.h"
#include <stdint.h>
namespace Tergos2D
{

    struct alignas(4) PixelFormatInfo
    {
        PixelFormat format;    // The pixel format
        uint8_t bytesPerPixel; // Number of bytes per pixel
        uint8_t bitsPerPixel;  // how many bits per pixel
        bool isBitFormat;      // uses less then one byte per pixel (grayscale 4 or grayscale 1)
        uint8_t numChannels;   // Number of color channels
        bool hasAlpha;         // Whether the format includes an alpha channel
        const char *name;      // A human-readable name for the format

        // Bit masks and shifts for each channel
        uint16_t redMask, greenMask, blueMask, alphaMask;
        uint8_t redShift, greenShift, blueShift, alphaShift;

        PixelFormatInfo() = default;

        PixelFormatInfo(PixelFormat fmt)
            : format(fmt), bytesPerPixel(0), bitsPerPixel(0), isBitFormat(false),
            numChannels(0), hasAlpha(false),
            redMask(0), greenMask(0), blueMask(0), alphaMask(0),
            redShift(0), greenShift(0), blueShift(0), alphaShift(0)
        {}

        PixelFormatInfo(PixelFormat fmt, uint8_t bpp, uint8_t bitspp, bool isBitFmt, uint8_t channels, bool alpha,
                uint16_t redMaskParam, uint8_t redShiftParam,
                uint16_t grnMaskParam, uint8_t greenShiftParam,
                uint16_t blueMaskParam, uint8_t blueShiftParam,
                uint16_t alphaMaskParam = 0, uint8_t alphaShiftParam = 0)
        : format(fmt),
        bytesPerPixel(bpp),
        bitsPerPixel(bitspp),
        isBitFormat(isBitFmt),
        numChannels(channels),
        hasAlpha(alpha),
        redMask(redMaskParam),
        greenMask(grnMaskParam),
        blueMask(blueMaskParam),
        alphaMask(alphaMaskParam),
        redShift(redShiftParam),
        greenShift(greenShiftParam),
        blueShift(blueShiftParam),
        alphaShift(alphaShiftParam)
        {
        }

    private:
    };

    class PixelFormatRegistry
    {
    public:
        static const PixelFormatInfo &GetInfo(PixelFormat format);

    private:
        static const PixelFormatInfo formatInfoArray[];
    };

}

#endif // PIXELFORMATINFO_H
