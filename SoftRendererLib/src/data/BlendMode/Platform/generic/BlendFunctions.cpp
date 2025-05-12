#include "../../BlendMode.h"
#include "../../BlendFunctions.h"
#include "../../../PixelFormat/PixelConverter.h"
#include "../../../PixelFormat/PixelFormatInfo.h"
#include "esp_cpu.h"
#include "esp_attr.h"
#include "esp_heap_caps.h"

using namespace Tergos2D;


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
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;

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
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }

        uint8_t invAlpha = 255 - alpha;

        // Blend the source and destination pixels
        dstPixel[0] = (srcColor[0] * alpha + dstPixel[0] * invAlpha) >> 8;
        dstPixel[1] = (srcColor[1] * alpha + dstPixel[1] * invAlpha) >> 8;
        dstPixel[2] = (srcColor[2] * alpha + dstPixel[2] * invAlpha) >> 8;
    }
}


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
    size_t i = 0;
    const size_t batchSize = 8;
    const size_t batchCount = rowLength / batchSize;
    const size_t remainingPixels = rowLength % batchSize;

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


void BlendFunctions::BlendSolidRowRGB24(uint8_t *dstRow,
                                        const uint8_t *srcRow,
                                        size_t rowLength,
                                        const PixelFormatInfo &targetInfo,
                                        const PixelFormatInfo &sourceInfo,
                                        Coloring coloring,
                                        bool useSolidColor,
                                        BlendContext& context)
{
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, 1);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    uint8_t alpha = 255; // Default alpha
    if (context.mode != BlendMode::COLORINGONLY)
    {
        alpha = (*reinterpret_cast<const uint32_t *>(srcRow) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
    }

    if (alpha == 0)
    {
        return;
    }

    if (colorFactor != 0)
    {
        srcRGB24[0] = (srcRGB24[0] * colorDataAsRGB[0]) >> 8;
        srcRGB24[1] = (srcRGB24[1] * colorDataAsRGB[1]) >> 8;
        srcRGB24[2] = (srcRGB24[2] * colorDataAsRGB[2]) >> 8;
        alpha = (alpha * colorDataAsRGB[2]) >> 8;
    }

    uint8_t invAlpha = 255 - alpha;

    for (size_t i = 0; i < rowLength * 3; i += 3)
    {
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
        case BlendFactor::DestAlpha:
            srcFactorR = 255;
            srcFactorG = 255;
            srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 0;
            srcFactorG = 0;
            srcFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcRGB24[0];
            srcFactorG = srcRGB24[1];
            srcFactorB = srcRGB24[2];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstRow[i];
            srcFactorG = dstRow[i + 1];
            srcFactorB = dstRow[i + 2];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcRGB24[0];
            srcFactorG = 255 - srcRGB24[1];
            srcFactorB = 255 - srcRGB24[2];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstRow[i];
            srcFactorG = 255 - dstRow[i + 1];
            srcFactorB = 255 - dstRow[i + 2];
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
        case BlendFactor::DestAlpha:
            dstFactorR = 255;
            dstFactorG = 255;
            dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 0;
            dstFactorG = 0;
            dstFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcRGB24[0];
            dstFactorG = srcRGB24[1];
            dstFactorB = srcRGB24[2];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstRow[i];
            dstFactorG = dstRow[i + 1];
            dstFactorB = dstRow[i + 2];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcRGB24[0];
            dstFactorG = 255 - srcRGB24[1];
            dstFactorB = 255 - srcRGB24[2];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstRow[i];
            dstFactorG = 255 - dstRow[i + 1];
            dstFactorB = 255 - dstRow[i + 2];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstRow[i] = (srcRGB24[0] * srcFactorR + dstRow[i] * dstFactorR) >> 8;
                dstRow[i + 1] = (srcRGB24[1] * srcFactorG + dstRow[i + 1] * dstFactorG) >> 8;
                dstRow[i + 2] = (srcRGB24[2] * srcFactorB + dstRow[i + 2] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
            {
                    int tempR = (srcRGB24[0] * srcFactorR - dstRow[i] * dstFactorR) >> 8;
                    int tempG = (srcRGB24[1] * srcFactorG - dstRow[i + 1] * dstFactorG) >> 8;
                    int tempB = (srcRGB24[2] * srcFactorB - dstRow[i + 2] * dstFactorB) >> 8;

                    dstRow[i] = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
                    dstRow[i + 1] = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
                    dstRow[i + 2] = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);
                    break;
            }
            case BlendOperation::ReverseSubtract:
            {
                int tempR = (dstRow[i] * dstFactorR - srcRGB24[0] * srcFactorR) >> 8;
                int tempG = (dstRow[i + 1] * dstFactorG - srcRGB24[1] * srcFactorG) >> 8;
                int tempB = (dstRow[i + 2] * dstFactorB - srcRGB24[2] * srcFactorB) >> 8;

                dstRow[i] = tempR < 0 ? 0 : (tempR > 255 ? 255 : tempR);
                dstRow[i + 1] = tempG < 0 ? 0 : (tempG > 255 ? 255 : tempG);
                dstRow[i + 2] = tempB < 0 ? 0 : (tempB > 255 ? 255 : tempB);
                break;
            }

            default:
                break;
        }
    }
}

void BlendFunctions::BlendRGB24(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                Coloring coloring,
                                bool useSolidColor,
                                BlendContext& context)
{
    // Conversion function for the source format could be either rgb24 or bgr24
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;
    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t alpha = 255;

        uint8_t *srcColor = &srcRGB24[i * 3];
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

        if (colorFactor != 0)
        {
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }

        if (alpha == 255)
        {
            dstPixel[0] = srcColor[0];
            dstPixel[1] = srcColor[1];
            dstPixel[2] = srcColor[2];
            continue;
        }

        uint8_t invAlpha = 255 - alpha;

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
        case BlendFactor::DestAlpha:
            srcFactorR = 255;
            srcFactorG = 255;
            srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 0;
            srcFactorG = 0;
            srcFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcColor[0];
            srcFactorG = srcColor[1];
            srcFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstPixel[0];
            srcFactorG = dstPixel[1];
            srcFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcColor[0];
            srcFactorG = 255 - srcColor[1];
            srcFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstPixel[0];
            srcFactorG = 255 - dstPixel[1];
            srcFactorB = 255 - dstPixel[2];
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
        case BlendFactor::DestAlpha:
            dstFactorR = 255;
            dstFactorG = 255;
            dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 0;
            dstFactorG = 0;
            dstFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcColor[0];
            dstFactorG = srcColor[1];
            dstFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstPixel[0];
            dstFactorG = dstPixel[1];
            dstFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcColor[0];
            dstFactorG = 255 - srcColor[1];
            dstFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstPixel[0];
            dstFactorG = 255 - dstPixel[1];
            dstFactorB = 255 - dstPixel[2];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstPixel[0] = (srcColor[0] * srcFactorR + dstPixel[0] * dstFactorR) >> 8;
                dstPixel[1] = (srcColor[1] * srcFactorG + dstPixel[1] * dstFactorG) >> 8;
                dstPixel[2] = (srcColor[2] * srcFactorB + dstPixel[2] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
                dstPixel[0] = ((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) < 0 ? 0 : (((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) > 255 ? 255 : (srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8);
                dstPixel[1] = ((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) < 0 ? 0 : (((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) > 255 ? 255 : (srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8);
                dstPixel[2] = ((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) < 0 ? 0 : (((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) > 255 ? 255 : (srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8);
                break;
            case BlendOperation::ReverseSubtract:
                dstPixel[0] = ((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) < 0 ? 0 : (((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) > 255 ? 255 : (dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8);
                dstPixel[1] = ((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) < 0 ? 0 : (((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) > 255 ? 255 : (dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8);
                dstPixel[2] = ((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) < 0 ? 0 : (((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) > 255 ? 255 : (dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8);
                break;
            default:
                break;
        }
    }
}

void BlendFunctions::BlendRGBA32ToRGB24(uint8_t *dstRow,
                                        const uint8_t *srcRow,
                                        size_t rowLength,
                                        const PixelFormatInfo &targetInfo,
                                        const PixelFormatInfo &sourceInfo,
                                        Coloring coloring,
                                        bool useSolidColor,
                                        BlendContext& context)
{
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    PixelConverter::ConvertFunc convertColorToRGB24 = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];
    alignas(16) uint8_t colorDataAsRGB[3];

    convertToRGB24(srcRow, srcRGB24, rowLength);
    convertColorToRGB24(coloring.color.data, colorDataAsRGB, 1);


    convertToRGB24(srcRow, srcRGB24, rowLength);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled * coloring.color.data[0];
    uint8_t inverseColorFactor = 255 - colorFactor;

    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t alpha = (context.mode == BlendMode::COLORINGONLY) * 255 + (context.mode != BlendMode::COLORINGONLY) * srcPixel[3];

        uint8_t mask = -(alpha != 0);
        alpha &= mask;

        uint8_t *srcColor = &srcRGB24[i * 3];

        if(colorFactor)
        {
            srcColor[0] = (srcColor[0] * colorDataAsRGB[0]) >> 8;
            srcColor[1] = (srcColor[1] * colorDataAsRGB[1]) >> 8;
            srcColor[2] = (srcColor[2] * colorDataAsRGB[2]) >> 8;
            alpha = (alpha * coloring.color.data[0]) >> 8;
        }
        uint8_t invAlpha = 255 - alpha;

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
        case BlendFactor::DestAlpha:
            srcFactorR = 255;
            srcFactorG = 255;
            srcFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            srcFactorR = 0;
            srcFactorG = 0;
            srcFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            srcFactorR = srcColor[0];
            srcFactorG = srcColor[1];
            srcFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            srcFactorR = dstPixel[0];
            srcFactorG = dstPixel[1];
            srcFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            srcFactorR = 255 - srcColor[0];
            srcFactorG = 255 - srcColor[1];
            srcFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            srcFactorR = 255 - dstPixel[0];
            srcFactorG = 255 - dstPixel[1];
            srcFactorB = 255 - dstPixel[2];
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
        case BlendFactor::DestAlpha:
            dstFactorR = 255;
            dstFactorG = 255;
            dstFactorB = 255;
            break;
        case BlendFactor::InverseDestAlpha:
            dstFactorR = 0;
            dstFactorG = 0;
            dstFactorB = 0;
            break;
        case BlendFactor::SourceColor:
            dstFactorR = srcColor[0];
            dstFactorG = srcColor[1];
            dstFactorB = srcColor[2];
            break;
        case BlendFactor::DestColor:
            dstFactorR = dstPixel[0];
            dstFactorG = dstPixel[1];
            dstFactorB = dstPixel[2];
            break;
        case BlendFactor::InverseSourceColor:
            dstFactorR = 255 - srcColor[0];
            dstFactorG = 255 - srcColor[1];
            dstFactorB = 255 - srcColor[2];
            break;
        case BlendFactor::InverseDestColor:
            dstFactorR = 255 - dstPixel[0];
            dstFactorG = 255 - dstPixel[1];
            dstFactorB = 255 - dstPixel[2];
            break;
        default:
            dstFactorR = dstFactorG = dstFactorB = 255;
            break;
        }

        switch (context.colorBlendOperation)
        {
            case BlendOperation::Add:
                dstPixel[0] = (srcColor[0] * srcFactorR + dstPixel[0] * dstFactorR) >> 8;
                dstPixel[1] = (srcColor[1] * srcFactorG + dstPixel[1] * dstFactorG) >> 8;
                dstPixel[2] = (srcColor[2] * srcFactorB + dstPixel[2] * dstFactorB) >> 8;
                break;
            case BlendOperation::Subtract:
                dstPixel[0] = ((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) < 0 ? 0 : (((srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8) > 255 ? 255 : (srcColor[0] * srcFactorR - dstPixel[0] * dstFactorR) >> 8);
                dstPixel[1] = ((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) < 0 ? 0 : (((srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8) > 255 ? 255 : (srcColor[1] * srcFactorG - dstPixel[1] * dstFactorG) >> 8);
                dstPixel[2] = ((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) < 0 ? 0 : (((srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8) > 255 ? 255 : (srcColor[2] * srcFactorB - dstPixel[2] * dstFactorB) >> 8);
            break;
            case BlendOperation::ReverseSubtract:
                dstPixel[0] = ((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) < 0 ? 0 : (((dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8) > 255 ? 255 : (dstPixel[0] * dstFactorR - srcColor[0] * srcFactorR) >> 8);
                dstPixel[1] = ((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) < 0 ? 0 : (((dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8) > 255 ? 255 : (dstPixel[1] * dstFactorG - srcColor[1] * srcFactorG) >> 8);
                dstPixel[2] = ((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) < 0 ? 0 : (((dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8) > 255 ? 255 : (dstPixel[2] * dstFactorB - srcColor[2] * srcFactorB) >> 8);
                break;
            case BlendOperation::BitwiseAnd:
                dstPixel[0] = (srcColor[0] * srcFactorR & dstPixel[0] * dstFactorR) >> 8;
                dstPixel[1] = (srcColor[1] * srcFactorG & dstPixel[1] * dstFactorG) >> 8;
                dstPixel[2] = (srcColor[2] * srcFactorB & dstPixel[2] * dstFactorB) >> 8;
                break;
            default:
                break;
        }
    }
}