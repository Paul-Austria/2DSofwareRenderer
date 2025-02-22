#include "BlendMode.h"
#include "BlendFunctions.h"
#include "../PixelFormat/PixelConverter.h"
#include "../PixelFormat/PixelFormatInfo.h"

#include <algorithm>
#include <cmath>

using namespace Renderer2D;

void BlendFunctions::BlendPixel(uint8_t *dstPixel, const uint8_t *srcPixel, uint8_t alpha, uint8_t invAlpha, BlendMode blendMode)
{
    switch (blendMode)
    {
    case BlendMode::SIMPLE:
        dstPixel[0] = (srcPixel[0] * alpha + dstPixel[0] * invAlpha) >> 8;
        break;
    case BlendMode::MULTIPLY:
        dstPixel[0] = ((srcPixel[0] * dstPixel[0]) * alpha + dstPixel[0] * invAlpha) >> 8;
        break;
    default:
        break;
    }
}

void BlendFunctions::BlendRow(uint8_t *dstRow,
                              const uint8_t *srcRow,
                              size_t rowLength,
                              const PixelFormatInfo &targetInfo,
                              const PixelFormatInfo &sourceInfo,
                              Coloring coloring,
                              bool useSolidColor,
                              BlendMode selectedBlendMode)
{

    auto blendFunc = GetBlendFunc(targetInfo.format,useSolidColor);

    if (blendFunc != nullptr)
    {
        blendFunc(dstRow, srcRow, rowLength, targetInfo, sourceInfo, coloring, selectedBlendMode);
        return;
    }
    
    // Get conversion functions once
    PixelConverter::ConvertFunc convertToARGB8888 = nullptr;
    PixelConverter::ConvertFunc convertFromARGB8888 = nullptr;

    convertToARGB8888 = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::ARGB8888);
    convertFromARGB8888 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for conversion
    alignas(16) uint8_t srcARGB8888[4];
    alignas(16) uint8_t dstARGB8888[4];

    for (size_t i = 0; i < rowLength; ++i)
    {
        const uint8_t *srcPixel = srcRow + i * sourceInfo.bytesPerPixel;
        uint8_t *dstPixel = dstRow + i * targetInfo.bytesPerPixel;

        // Convert source to ARGB8888
        uint8_t srcAlpha;

        convertToARGB8888(srcPixel, srcARGB8888, 1);
        srcAlpha = srcARGB8888[0];

        if (srcAlpha == 0)
        {
            continue;
        }

        if (convertToARGB8888)
        {
            convertToARGB8888(dstPixel, dstARGB8888, 1);
        }
        uint8_t invAlpha = 255 - srcAlpha;
        uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
        uint8_t inverseColorFactor = 255 - colorFactor;
        for (int c = 1; c <= 3; ++c) // Red, Green, Blue channels
        {
            srcARGB8888[c] = (srcARGB8888[c] * srcAlpha + coloring.color.data[c] * invAlpha) >> 8;
            if (selectedBlendMode != BlendMode::COLORINGONLY)
            {
                dstARGB8888[c] = (srcARGB8888[c] * srcAlpha + dstARGB8888[c] * invAlpha) >> 8;
            }
        }

        // Use the maximum alpha
        dstARGB8888[0] = std::max(srcAlpha, dstARGB8888[0]);

        // Convert blended pixel back to target format
        convertFromARGB8888(dstARGB8888, dstPixel, 1);
    }
}


