#include "BlendMode.h"
#include "BlendFunctions.h"
#include "../PixelFormat/PixelConverter.h"
#include "../PixelFormat/PixelFormatInfo.h"

#include <algorithm>
#include <cmath>

using namespace Tergos2D;

void BlendFunctions::BlendRow(uint8_t *dstRow,
                              const uint8_t *srcRow,
                              size_t rowLength,
                              const PixelFormatInfo &targetInfo,
                              const PixelFormatInfo &sourceInfo,
                              Coloring coloring,
                              bool useSolidColor,
                              BlendContext& context)
{
    auto blendFunc = GetBlendFunc(targetInfo.format, useSolidColor);

    if (blendFunc != nullptr)
    {
        blendFunc(dstRow, srcRow, rowLength, targetInfo, sourceInfo, coloring, useSolidColor, context);
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

        uint8_t srcFactorR, dstFactorR;
        uint8_t srcFactorG, dstFactorG;
        uint8_t srcFactorB, dstFactorB;

        switch (context.colorBlendFactorSrc)
        {
        case BlendFactor::Zero:
            srcFactorR = srcFactorG = srcFactorB = 0;
            break;
        case BlendFactor::One:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = srcAlpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            srcFactorR = srcFactorG = srcFactorB = 255 - srcAlpha;
            break;
        case BlendFactor::DestAlpha:
            srcFactorR = dstARGB8888[0];
            srcFactorG = dstARGB8888[1];
            srcFactorB = dstARGB8888[2];
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 255 - dstARGB8888[0];
            srcFactorG = 255 - dstARGB8888[1];
            srcFactorB = 255 - dstARGB8888[2];
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcARGB8888[1];
            srcFactorG = srcARGB8888[2];
            srcFactorB = srcARGB8888[3];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstARGB8888[1];
            srcFactorG = dstARGB8888[2];
            srcFactorB = dstARGB8888[3];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcARGB8888[1];
            srcFactorG = 255 - srcARGB8888[2];
            srcFactorB = 255 - srcARGB8888[3];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstARGB8888[1];
            srcFactorG = 255 - dstARGB8888[2];
            srcFactorB = 255 - dstARGB8888[3];
            break;
        default:
            srcFactorR = srcFactorG = srcFactorB = 255;
            break;
        }

        switch (context.colorBlendFactorDst)
        {
        case BlendFactor::Zero:
            dstFactorR = dstFactorG = dstFactorB = 0;
            break;
        case BlendFactor::One:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        case BlendFactor::SourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = srcAlpha;
            break;
        case BlendFactor::InverseSourceAlpha:
            dstFactorR = dstFactorG = dstFactorB = 255 - srcAlpha;
            break;
        case BlendFactor::DestAlpha:
            dstFactorR = dstARGB8888[0];
            dstFactorG = dstARGB8888[1];
            dstFactorB = dstARGB8888[2];
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 255 - dstARGB8888[0];
            dstFactorG = 255 - dstARGB8888[1];
            dstFactorB = 255 - dstARGB8888[2];
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcARGB8888[1];
            dstFactorG = srcARGB8888[2];
            dstFactorB = srcARGB8888[3];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstARGB8888[1];
            dstFactorG = dstARGB8888[2];
            dstFactorB = dstARGB8888[3];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcARGB8888[1];
            dstFactorG = 255 - srcARGB8888[2];
            dstFactorB = 255 - srcARGB8888[3];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstARGB8888[1];
            dstFactorG = 255 - dstARGB8888[2];
            dstFactorB = 255 - dstARGB8888[3];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstARGB8888[1] = (srcARGB8888[1] * srcFactorR + dstARGB8888[1] * dstFactorR) >> 8;
                dstARGB8888[2] = (srcARGB8888[2] * srcFactorG + dstARGB8888[2] * dstFactorG) >> 8;
                dstARGB8888[3] = (srcARGB8888[3] * srcFactorB + dstARGB8888[3] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
                dstARGB8888[1] = (srcARGB8888[1] * srcFactorR - dstARGB8888[1] * dstFactorR) >> 8;
                dstARGB8888[2] = (srcARGB8888[2] * srcFactorG - dstARGB8888[2] * dstFactorG) >> 8;
                dstARGB8888[3] = (srcARGB8888[3] * srcFactorB - dstARGB8888[3] * dstFactorB) >> 8;
                break;
            case BlendOperation::ReverseSubtract:
                dstARGB8888[1] = (dstARGB8888[1] * dstFactorR - srcARGB8888[1] * srcFactorR) >> 8;
                dstARGB8888[2] = (dstARGB8888[2] * dstFactorG - srcARGB8888[2] * srcFactorG) >> 8;
                dstARGB8888[3] = (dstARGB8888[3] * dstFactorB - srcARGB8888[3] * srcFactorB) >> 8;
                break;
            case BlendOperation::BitwiseAnd:
                dstARGB8888[1] = (srcARGB8888[1] * srcFactorR & dstARGB8888[1] * dstFactorR) >> 8;
                dstARGB8888[2] = (srcARGB8888[2] * srcFactorG & dstARGB8888[2] * dstFactorG) >> 8;
                dstARGB8888[3] = (srcARGB8888[3] * srcFactorB & dstARGB8888[3] * dstFactorB) >> 8;
                break;
            default:
                break;
        }

        // Use the maximum alpha
        dstARGB8888[0] = std::max(srcAlpha, dstARGB8888[0]);

        // Convert blended pixel back to target format
        convertFromARGB8888(dstARGB8888, dstPixel, 1);
    }
}



// ONLY SOURCEALPHA, INVERSESOURCEALPHA, ADD ONE ZERO ADD
void BlendFunctions::BlendToRGB24Simple(uint8_t *dstRow,
    const uint8_t *srcRow,
    size_t rowLength,
    const PixelFormatInfo &targetInfo,
    const PixelFormatInfo &sourceInfo,
    Coloring coloring,
    bool useSolidColor,
    BlendContext& context)
{
    // Conversion function for the source format
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];

    convertToRGB24(srcRow, srcRGB24, rowLength);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;
    uint8_t colorR = coloring.color.data[1];
    uint8_t colorG = coloring.color.data[2];
    uint8_t colorB = coloring.color.data[3];

    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t grayValue = srcPixel[0];
        uint8_t alpha = (grayValue == 0) ? 0 : 255;

        if (alpha == 0)
        {
        continue;
        }

        uint8_t *srcColor = &srcRGB24[i * 3];

        // Apply coloring if needed
        if (coloring.colorEnabled)
        {
            srcColor[0] = ((srcColor[0] * inverseColorFactor) + (colorR * colorFactor)) >> 8;
            srcColor[1] = ((srcColor[1] * inverseColorFactor) + (colorG * colorFactor)) >> 8;
            srcColor[2] = ((srcColor[2] * inverseColorFactor) + (colorB * colorFactor)) >> 8;
        }

        uint8_t invAlpha = 255 - alpha;

        // Blend the source and destination pixels
        dstPixel[0] = (srcColor[0] * alpha + dstPixel[0] * invAlpha) >> 8;
        dstPixel[1] = (srcColor[1] * alpha + dstPixel[1] * invAlpha) >> 8;
        dstPixel[2] = (srcColor[2] * alpha + dstPixel[2] * invAlpha) >> 8;
    }
}