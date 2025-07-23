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
    PixelConverter::ConvertFunc convertToARGB8888Target = nullptr;

    PixelConverter::ConvertFunc convertFromARGB8888 = nullptr;

    convertToARGB8888 = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::ARGB8888);
    convertToARGB8888Target = PixelConverter::GetConversionFunction(targetInfo.format, PixelFormat::ARGB8888);
    convertFromARGB8888 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
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

        if (convertToARGB8888Target)
        {
            convertToARGB8888Target(dstPixel, dstARGB8888, 1);
        }
        uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;


        if(colorFactor){
            srcARGB8888[1] = (srcARGB8888[1] * coloring.color.data[1]) >> 8;
            srcARGB8888[2] = (srcARGB8888[2] * coloring.color.data[2]) >> 8;
            srcARGB8888[3] = (srcARGB8888[3] * coloring.color.data[3]) >> 8;
            srcAlpha = (srcAlpha * coloring.color.data[0]) >> 8;
        }

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
            srcFactorG = dstARGB8888[0];
            srcFactorB = dstARGB8888[0];
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 255 - dstARGB8888[0];
            srcFactorG = 255 - dstARGB8888[0];
            srcFactorB = 255 - dstARGB8888[0];
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
            dstFactorG = dstARGB8888[0];
            dstFactorB = dstARGB8888[0];
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 255 - dstARGB8888[0];
            dstFactorG = 255 - dstARGB8888[0];
            dstFactorB = 255 - dstARGB8888[0];
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
                dstARGB8888[1] = ((srcARGB8888[1] * srcFactorR - dstARGB8888[1] * dstFactorR) >> 8) < 0 ? 0 : (((srcARGB8888[1] * srcFactorR - dstARGB8888[1] * dstFactorR) >> 8) > 255 ? 255 : (srcARGB8888[1] * srcFactorR - dstARGB8888[1] * dstFactorR) >> 8);
                dstARGB8888[2] = ((srcARGB8888[2] * srcFactorG - dstARGB8888[2] * dstFactorG) >> 8) < 0 ? 0 : (((srcARGB8888[2] * srcFactorG - dstARGB8888[2] * dstFactorG) >> 8) > 255 ? 255 : (srcARGB8888[2] * srcFactorG - dstARGB8888[2] * dstFactorG) >> 8);
                dstARGB8888[3] = ((srcARGB8888[3] * srcFactorB - dstARGB8888[3] * dstFactorB) >> 8) < 0 ? 0 : (((srcARGB8888[3] * srcFactorB - dstARGB8888[3] * dstFactorB) >> 8) > 255 ? 255 : (srcARGB8888[3] * srcFactorB - dstARGB8888[3] * dstFactorB) >> 8);
                break;
            case BlendOperation::ReverseSubtract:
                dstARGB8888[1] = ((dstARGB8888[1] * dstFactorR - srcARGB8888[1] * srcFactorR) >> 8) < 0 ? 0 : (((dstARGB8888[1] * dstFactorR - srcARGB8888[1] * srcFactorR) >> 8) > 255 ? 255 : (dstARGB8888[1] * dstFactorR - srcARGB8888[1] * srcFactorR) >> 8);
                dstARGB8888[2] = ((dstARGB8888[2] * dstFactorG - srcARGB8888[2] * srcFactorG) >> 8) < 0 ? 0 : (((dstARGB8888[2] * dstFactorG - srcARGB8888[2] * srcFactorG) >> 8) > 255 ? 255 : (dstARGB8888[2] * dstFactorG - srcARGB8888[2] * srcFactorG) >> 8);
                dstARGB8888[3] = ((dstARGB8888[3] * dstFactorB - srcARGB8888[3] * srcFactorB) >> 8) < 0 ? 0 : (((dstARGB8888[3] * dstFactorB - srcARGB8888[3] * srcFactorB) >> 8) > 255 ? 255 : (dstARGB8888[3] * dstFactorB - srcARGB8888[3] * srcFactorB) >> 8);
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


//RGB565 functions are not optimized
void BlendFunctions::BlendGrayscale8ToRGB565(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                Coloring coloring,
                                bool useSolidColor,
                                BlendContext& context)
{
    uint16_t colorTint = 0;
    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    if (colorFactor > 0) {
        uint8_t r = (coloring.color.data[1] * colorFactor) >> 8;
        uint8_t g = (coloring.color.data[2] * colorFactor) >> 8;
        uint8_t b = (coloring.color.data[3] * colorFactor) >> 8;
        colorTint = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    uint16_t *dstPixel = reinterpret_cast<uint16_t*>(dstRow);
    const uint8_t *srcPixel = srcRow;

    // Process pixels in batches of 8 for better cache utilization
    const size_t batchSize = 8;
    const size_t batchCount = rowLength / batchSize;

    for (size_t batch = 0; batch < batchCount; ++batch) {
        for (size_t j = 0; j < batchSize; ++j, ++srcPixel, ++dstPixel) {
            uint8_t gray = *srcPixel;

            // Skip fully transparent pixels
            if (gray == 0) {
                continue;
            }

            // Convert grayscale to RGB565 directly
            // For grayscale, R=G=B=gray, so we can optimize the conversion
            uint16_t grayRGB565 = ((gray & 0xF8) << 8) | ((gray & 0xFC) << 3) | (gray >> 3);

            // If color tinting is enabled
            if (colorFactor > 0) {
                // Blend between grayscale and color tint
                uint16_t r = ((grayRGB565 >> 11) * (255 - colorFactor) + ((colorTint >> 11) & 0x1F) * colorFactor) >> 8;
                uint16_t g = (((grayRGB565 >> 5) & 0x3F) * (255 - colorFactor) + ((colorTint >> 5) & 0x3F) * colorFactor) >> 8;
                uint16_t b = ((grayRGB565 & 0x1F) * (255 - colorFactor) + (colorTint & 0x1F) * colorFactor) >> 8;
                grayRGB565 = (r << 11) | (g << 5) | b;
            }

            // Fast path for fully opaque pixels
            if (gray == 255) {
                *dstPixel = grayRGB565;
                continue;
            }

            // Handle blending based on blend factors
            uint16_t dstColor = *dstPixel;
            uint8_t invAlpha = 255 - gray;

            uint8_t srcR = (grayRGB565 >> 11) & 0x1F;
            uint8_t srcG = (grayRGB565 >> 5) & 0x3F;
            uint8_t srcB = grayRGB565 & 0x1F;

            uint8_t dstR = (dstColor >> 11) & 0x1F;
            uint8_t dstG = (dstColor >> 5) & 0x3F;
            uint8_t dstB = dstColor & 0x1F;

            uint8_t srcFactor, dstFactor;

            // Fast path for common blend mode
            if (context.colorBlendFactorSrc == BlendFactor::SourceAlpha &&
                context.colorBlendFactorDst == BlendFactor::InverseSourceAlpha) {
                srcFactor = gray;
                dstFactor = invAlpha;
            } else {
                // Handle other blend modes
                switch (context.colorBlendFactorSrc) {
                    case BlendFactor::Zero: srcFactor = 0; break;
                    case BlendFactor::One: srcFactor = 255; break;
                    case BlendFactor::SourceAlpha: srcFactor = gray; break;
                    case BlendFactor::InverseSourceAlpha: srcFactor = invAlpha; break;
                    default: srcFactor = 255; break;
                }

                switch (context.colorBlendFactorDst) {
                    case BlendFactor::Zero: dstFactor = 0; break;
                    case BlendFactor::One: dstFactor = 255; break;
                    case BlendFactor::SourceAlpha: dstFactor = gray; break;
                    case BlendFactor::InverseSourceAlpha: dstFactor = invAlpha; break;
                    default: dstFactor = 255; break;
                }
            }

            // Blend the colors
            uint8_t blendedR = (srcR * srcFactor + dstR * dstFactor) >> 8;
            uint8_t blendedG = (srcG * srcFactor + dstG * dstFactor) >> 8;
            uint8_t blendedB = (srcB * srcFactor + dstB * dstFactor) >> 8;

            *dstPixel = (blendedR << 11) | (blendedG << 5) | blendedB;
        }
    }

    // Handle remaining pixels
    for (size_t i = batchCount * batchSize; i < rowLength; ++i, ++srcPixel, ++dstPixel) {
        uint8_t gray = *srcPixel;
        if (gray == 0) continue;

        uint16_t grayRGB565 = ((gray & 0xF8) << 8) | ((gray & 0xFC) << 3) | (gray >> 3);

        if (colorFactor > 0) {
            uint16_t r = ((grayRGB565 >> 11) * (255 - colorFactor) + ((colorTint >> 11) & 0x1F) * colorFactor) >> 8;
            uint16_t g = (((grayRGB565 >> 5) & 0x3F) * (255 - colorFactor) + ((colorTint >> 5) & 0x3F) * colorFactor) >> 8;
            uint16_t b = ((grayRGB565 & 0x1F) * (255 - colorFactor) + (colorTint & 0x1F) * colorFactor) >> 8;
            grayRGB565 = (r << 11) | (g << 5) | b;
        }

        if (gray == 255) {
            *dstPixel = grayRGB565;
            continue;
        }

        uint16_t dstColor = *dstPixel;
        uint8_t srcR = (grayRGB565 >> 11) & 0x1F;
        uint8_t srcG = (grayRGB565 >> 5) & 0x3F;
        uint8_t srcB = grayRGB565 & 0x1F;

        uint8_t dstR = (dstColor >> 11) & 0x1F;
        uint8_t dstG = (dstColor >> 5) & 0x3F;
        uint8_t dstB = dstColor & 0x1F;

        uint8_t alpha = gray;
        uint8_t invAlpha = 255 - alpha;

        uint8_t blendedR = (srcR * alpha + dstR * invAlpha) >> 8;
        uint8_t blendedG = (srcG * alpha + dstG * invAlpha) >> 8;
        uint8_t blendedB = (srcB * alpha + dstB * invAlpha) >> 8;

        *dstPixel = (blendedR << 11) | (blendedG << 5) | blendedB;
    }
}

void BlendFunctions::BlendRGB565(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                Coloring coloring,
                                bool useSolidColor,
                                BlendContext& context)
{
    PixelConverter::ConvertFunc convertToRGB565 = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::RGB565);
    PixelConverter::ConvertFunc convertColorToRGB565 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, PixelFormat::RGB565);

    // Temporary storage for source pixel in RGB565
    alignas(16) uint16_t srcRGB565[1024];
    alignas(16) uint16_t colorDataAsRGB565;

    convertToRGB565(srcRow, reinterpret_cast<uint8_t*>(srcRGB565), rowLength);
    convertColorToRGB565(coloring.color.data, reinterpret_cast<uint8_t*>(&colorDataAsRGB565), 1);

    const uint8_t *srcPixel = srcRow;
    uint16_t *dstPixel = (uint16_t*) dstRow;

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel++)
    {
        uint8_t alpha = 255;

        uint16_t srcColor = srcRGB565[i];
        if (sourceInfo.format == PixelFormat::GRAYSCALE8)
        {
            uint8_t grayValue = srcPixel[0];
            alpha = (grayValue == 0) ? 0 : 255;
        }
        else if (context.mode == BlendMode::COLORINGONLY)
        {
            alpha = 255;
        }
        else
        {
            alpha = (*reinterpret_cast<const uint32_t *>(srcPixel) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
        }

        if (alpha == 0)
        {
            continue;
        }

        uint8_t original = *reinterpret_cast<const uint8_t *>(srcPixel);
        uint8_t r = original & 0xFF;
        uint8_t srcR = (srcColor >> 11) & 0x1F;
        uint8_t srcG = (srcColor >> 5) & 0x3F;
        uint8_t srcB = srcColor & 0x1F;

        if (colorFactor != 0)
        {
            uint8_t colorR = (colorDataAsRGB565 >> 11) & 0x1F;
            uint8_t colorG = (colorDataAsRGB565 >> 5) & 0x3F;
            uint8_t colorB = colorDataAsRGB565 & 0x1F;

            srcR = (srcR * colorR) >> 5;
            srcG = (srcG * colorG) >> 6;
            srcB = (srcB * colorB) >> 5;

            alpha = (alpha * coloring.color.data[0]) >> 8;
        }

        if (alpha == 255)
        {
            *dstPixel = srcColor;
            continue;
        }

        uint8_t dstR = (*dstPixel >> 11) & 0x1F;
        uint8_t dstG = (*dstPixel >> 5) & 0x3F;
        uint8_t dstB = *dstPixel & 0x1F;

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
                srcFactorR = srcFactorG = srcFactorB = alpha;
                break;
            case BlendFactor::InverseSourceAlpha:
                srcFactorR = srcFactorG = srcFactorB = 255 - alpha;
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
                dstFactorR = dstFactorG = dstFactorB = alpha;
                break;
            case BlendFactor::InverseSourceAlpha:
                dstFactorR = dstFactorG = dstFactorB = 255 - alpha;
                break;
            default:
                dstFactorR = dstFactorG = dstFactorB = 255;
                break;
        }

        int blendedR, blendedG, blendedB;

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                blendedR = ((srcR * srcFactorR + dstR * dstFactorR) >> 8) & 0x1F;
                blendedG = ((srcG * srcFactorG + dstG * dstFactorG) >> 8) & 0x3F;
                blendedB = ((srcB * srcFactorB + dstB * dstFactorB) >> 8) & 0x1F;
                break;

            case BlendOperation::Subtract:
            {
                blendedR = ((srcR * srcFactorR - dstR * dstFactorR) >> 8);
                blendedG = ((srcG * srcFactorG - dstG * dstFactorG) >> 8);
                blendedB = ((srcB * srcFactorB - dstB * dstFactorB) >> 8);

                // Simple clamping
                blendedR = blendedR < 0 ? 0 : (blendedR > 0x1F ? 0x1F : blendedR);
                blendedG = blendedG < 0 ? 0 : (blendedG > 0x3F ? 0x3F : blendedG);
                blendedB = blendedB < 0 ? 0 : (blendedB > 0x1F ? 0x1F : blendedB);
                break;
            }

            case BlendOperation::ReverseSubtract:
            {
                blendedR = ((dstR * dstFactorR - srcR * srcFactorR) >> 8);
                blendedG = ((dstG * dstFactorG - srcG * srcFactorG) >> 8);
                blendedB = ((dstB * dstFactorB - srcB * srcFactorB) >> 8);

                // Simple clamping
                blendedR = blendedR < 0 ? 0 : (blendedR > 0x1F ? 0x1F : blendedR);
                blendedG = blendedG < 0 ? 0 : (blendedG > 0x3F ? 0x3F : blendedG);
                blendedB = blendedB < 0 ? 0 : (blendedB > 0x1F ? 0x1F : blendedB);
                break;
            }

            default:
                blendedR = ((srcR * srcFactorR + dstR * dstFactorR) >> 8) & 0x1F;
                blendedG = ((srcG * srcFactorG + dstG * dstFactorG) >> 8) & 0x3F;
                blendedB = ((srcB * srcFactorB + dstB * dstFactorB) >> 8) & 0x1F;
                break;
        }

        *dstPixel = (blendedR << 11) | (blendedG << 5) | blendedB;
    }
}

