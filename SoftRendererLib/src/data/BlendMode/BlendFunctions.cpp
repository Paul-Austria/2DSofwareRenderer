#include "BlendMode.h"
#include "BlendFunctions.h"
#include "../PixelFormat/PixelConverter.h"
#include "../PixelFormat/PixelFormatInfo.h"

#include <algorithm>
#include <cmath>

using namespace Renderer2D;

void BlendFunctions::BlendRow(uint8_t *dstRow,
                              const uint8_t *srcRow,
                              size_t rowLength,
                              const PixelFormatInfo &targetInfo,
                              const PixelFormatInfo &sourceInfo,
                              SelectedBlendMode selectedBlendMode)
{

    bool registryRet = false;
    auto blendFunc = GetBlendFunc(targetInfo.format, registryRet);
    if (registryRet)
    {
        blendFunc(dstRow,
                  srcRow,
                  rowLength,
                  targetInfo,
                  sourceInfo,
                  selectedBlendMode);
        return;
    }
    // Get conversion functions once
    auto convertToARGB8888 = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::ARGB8888);
    auto convertFromARGB8888 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for conversion
    alignas(16) uint8_t srcARGB8888[4];
    alignas(16) uint8_t dstARGB8888[4];

    for (size_t i = 0; i < rowLength; ++i)
    {
        const uint8_t *srcPixel = srcRow + i * sourceInfo.bytesPerPixel;
        uint8_t *dstPixel = dstRow + i * targetInfo.bytesPerPixel;

        // Convert source to ARGB8888
        convertToARGB8888(srcPixel, srcARGB8888, 1);
        uint8_t srcAlpha = srcARGB8888[0];

        if (srcAlpha == 0)
        {
            continue;
        }

        convertToARGB8888(dstPixel, dstARGB8888, 1);
        uint8_t invAlpha = 255 - srcAlpha;

        for (int c = 1; c <= 3; ++c) // Red, Green, Blue channels
        {
            dstARGB8888[c] = (srcARGB8888[c] * srcAlpha + dstARGB8888[c] * invAlpha) >> 8;
        }

        // Use the maximum alpha
        dstARGB8888[0] = std::max(srcAlpha, dstARGB8888[0]);

        // Convert blended pixel back to target format
        convertFromARGB8888(dstARGB8888, dstPixel, 1);
    }
}




void BlendFunctions::BlendRGB24(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                SelectedBlendMode selectedBlendMode)
{
    // Conversion function for the source format
    auto convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024*3];

   convertToRGB24(srcRow, srcRGB24, rowLength);

    for (size_t i = 0; i < rowLength; ++i)
    {
        const uint8_t *srcPixel = srcRow + i * sourceInfo.bytesPerPixel;
        uint8_t *dstPixel = dstRow + i * targetInfo.bytesPerPixel;
        uint8_t alpha = (*((uint32_t*)srcPixel) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
        if (alpha == 0)
        {
            continue;
        }

        uint8_t invAlpha = 255 - alpha;

        for (int c = 0; c < 3; ++c)
        {
            dstPixel[c] = (srcRGB24[i * 3 + c] * alpha + dstPixel[c] * invAlpha) >> 8;
        }
    }
}
