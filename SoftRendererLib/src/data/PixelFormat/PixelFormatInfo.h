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
     PixelFormat format;

        uint16_t redMask;
        uint16_t greenMask;
        uint16_t blueMask;
        uint16_t alphaMask;

        uint8_t bytesPerPixel;
        uint8_t bitsPerPixel;
        uint8_t numChannels;
        uint8_t redShift;

        uint8_t greenShift;
        uint8_t blueShift;
        uint8_t alphaShift;

        bool isBitFormat;
        bool hasAlpha;

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
        static const PixelFormatInfo GetInfo(PixelFormat format);
    private:
    };

}

#endif // PIXELFORMATINFO_H
