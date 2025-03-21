#include "../../BlendMode.h"
#include "../../BlendFunctions.h"
#include "../../../PixelFormat/PixelConverter.h"
#include "../../../PixelFormat/PixelFormatInfo.h"

using namespace Tergos2D;

void BlendFunctions::BlendSolidRowRGB24(uint8_t *dstRow,
                                        const uint8_t *srcRow,
                                        size_t rowLength,
                                        const PixelFormatInfo &targetInfo,
                                        const PixelFormatInfo &sourceInfo,
                                        Coloring coloring,
                                        bool useSolidColor,
                                        BlendMode selectedBlendMode,
                                        BlendContext context)
{
    // Conversion function for the source format
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[3]; // Only store one color if it's uniform

    convertToRGB24(srcRow, srcRGB24, 1); // Convert only one pixel if all are the same

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    uint8_t alpha = 255; // Default alpha
    if (sourceInfo.format == PixelFormat::GRAYSCALE8)
    {
        uint8_t grayValue = srcRow[0];
        alpha = (grayValue == 0) ? 0 : 255;
    }
    else if (selectedBlendMode != BlendMode::COLORINGONLY)
    {
        alpha = (*reinterpret_cast<const uint32_t *>(srcRow) >> sourceInfo.alphaShift) & sourceInfo.alphaMask;
    }

    if (alpha == 0)
    {
        return;
    }

    if (colorFactor != 0)
    {
        srcRGB24[0] = (srcRGB24[0] * inverseColorFactor + coloring.color.data[1] * colorFactor) >> 8;
        srcRGB24[1] = (srcRGB24[1] * inverseColorFactor + coloring.color.data[2] * colorFactor) >> 8;
        srcRGB24[2] = (srcRGB24[2] * inverseColorFactor + coloring.color.data[3] * colorFactor) >> 8;
    }

    uint8_t invAlpha = 255 - alpha;
    for (size_t i = 0; i < rowLength * 3; i += 3)
    {
        dstRow[i] = (srcRGB24[0] * alpha + dstRow[i] * invAlpha) >> 8;
        dstRow[i + 1] = (srcRGB24[1] * alpha + dstRow[i + 1] * invAlpha) >> 8;
        dstRow[i + 2] = (srcRGB24[2] * alpha + dstRow[i + 2] * invAlpha) >> 8;
    }
}





void BlendFunctions::BlendRGB24(uint8_t *dstRow,
                                const uint8_t *srcRow,
                                size_t rowLength,
                                const PixelFormatInfo &targetInfo,
                                const PixelFormatInfo &sourceInfo,
                                Coloring coloring,
                                bool useSolidColor,
                                BlendMode selectedBlendMode,
                                BlendContext context)
{

    // Conversion function for the source format
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);

    // Temporary storage for source pixel in RGB24
    alignas(16) uint8_t srcRGB24[1024 * 3];

    convertToRGB24(srcRow, srcRGB24, rowLength);

    const uint8_t *srcPixel = srcRow;
    uint8_t *dstPixel = dstRow;

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    for (size_t i = 0; i < rowLength; ++i, srcPixel += sourceInfo.bytesPerPixel, dstPixel += targetInfo.bytesPerPixel)
    {
        uint8_t alpha = 255;

        if (sourceInfo.format == PixelFormat::GRAYSCALE8)
        {
            uint8_t grayValue = srcPixel[0];
            alpha = (grayValue == 0) ? 0 : 255;
        }
        else if (selectedBlendMode == BlendMode::COLORINGONLY)
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

        uint8_t *srcColor = &srcRGB24[i * 3];

        if (colorFactor != 0)
        {
            srcColor[0] = (srcColor[0] * inverseColorFactor + coloring.color.data[1] * colorFactor) >> 8;
            srcColor[1] = (srcColor[1] * inverseColorFactor + coloring.color.data[2] * colorFactor) >> 8;
            srcColor[2] = (srcColor[2] * inverseColorFactor + coloring.color.data[3] * colorFactor) >> 8;
        }

        if (alpha == 255)
        {
            dstPixel[0] = srcColor[0];
            dstPixel[1] = srcColor[1];
            dstPixel[2] = srcColor[2];
            continue;
        }

        uint8_t invAlpha = 255 - alpha;
        dstPixel[0] = (srcColor[0] * alpha + dstPixel[0] * invAlpha) >> 8;
        dstPixel[1] = (srcColor[1] * alpha + dstPixel[1] * invAlpha) >> 8;
        dstPixel[2] = (srcColor[2] * alpha + dstPixel[2] * invAlpha) >> 8;
    }
}


void BlendFunctions::BlendRGBA32ToRGB24(uint8_t *dstRow,
    const uint8_t *srcRow,
    size_t rowLength,
    const PixelFormatInfo &targetInfo,
    const PixelFormatInfo &sourceInfo,
    Coloring coloring,
    bool useSolidColor,
    BlendMode selectedBlendMode,
    BlendContext blendContext)
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
        uint8_t alpha = srcPixel[3];

        // Apply coloring if enabled
        uint8_t srcColor[3] = { srcRGB24[i * 3], srcRGB24[i * 3 + 1], srcRGB24[i * 3 + 2] };
        if (coloring.colorEnabled) {
            srcColor[0] = (srcColor[0] * inverseColorFactor + colorR * colorFactor) / 255;
            srcColor[1] = (srcColor[1] * inverseColorFactor + colorG * colorFactor) / 255;
            srcColor[2] = (srcColor[2] * inverseColorFactor + colorB * colorFactor) / 255;
        }

        // Calculate color factors for R, G, B
        uint8_t srcFactor[3], dstFactor[3];
        for (int c = 0; c < 3; ++c)
        {
            switch (blendContext.colorBlendFactorSrc)
            {
                case BlendFactor::Zero: srcFactor[c] = 0; break;
                case BlendFactor::One: srcFactor[c] = 255; break;
                case BlendFactor::SourceColor: srcFactor[c] = srcColor[c]; break;
                case BlendFactor::InverseSourceColor: srcFactor[c] = 255 - srcColor[c]; break;
                case BlendFactor::SourceAlpha: srcFactor[c] = alpha; break;
                case BlendFactor::InverseSourceAlpha: srcFactor[c] = 255 - alpha; break;
                case BlendFactor::DestColor: srcFactor[c] = dstPixel[c]; break;
                case BlendFactor::InverseDestColor: srcFactor[c] = 255 - dstPixel[c]; break;
                case BlendFactor::DestAlpha: srcFactor[c] = dstPixel[3]; break;
                case BlendFactor::InverseDestAlpha: srcFactor[c] = 255 - dstPixel[3]; break;
                default: srcFactor[c] = 255; break;
            }

            switch (blendContext.colorBlendFactorDst)
            {
                case BlendFactor::Zero: dstFactor[c] = 0; break;
                case BlendFactor::One: dstFactor[c] = 255; break;
                case BlendFactor::SourceColor: dstFactor[c] = srcColor[c]; break;
                case BlendFactor::InverseSourceColor: dstFactor[c] = 255 - srcColor[c]; break;
                case BlendFactor::SourceAlpha: dstFactor[c] = alpha; break;
                case BlendFactor::InverseSourceAlpha: dstFactor[c] = 255 - alpha; break;
                case BlendFactor::DestColor: dstFactor[c] = dstPixel[c]; break;
                case BlendFactor::InverseDestColor: dstFactor[c] = 255 - dstPixel[c]; break;
                case BlendFactor::DestAlpha: dstFactor[c] = dstPixel[3]; break;
                case BlendFactor::InverseDestAlpha: dstFactor[c] = 255 - dstPixel[3]; break;
                default: dstFactor[c] = 255; break;
            }
        }

        // Apply color blend operation (simplified, add mode here)
        uint8_t blendedColor[3];
        for (int c = 0; c < 3; ++c)
        {
            blendedColor[c] = (srcColor[c] * srcFactor[c] + dstPixel[c] * dstFactor[c]) / 255;
        }

        // Calculate alpha factors (same approach as for color channels)
        uint8_t srcAlphaFactor, dstAlphaFactor;

        switch (blendContext.alphaBlendFactorSrc)
        {
            case BlendFactor::Zero: srcAlphaFactor = 0; break;
            case BlendFactor::One: srcAlphaFactor = 255; break;
            case BlendFactor::SourceColor: srcAlphaFactor = alpha; break;
            case BlendFactor::InverseSourceColor: srcAlphaFactor = 255 - alpha; break;
            case BlendFactor::SourceAlpha: srcAlphaFactor = alpha; break;
            case BlendFactor::InverseSourceAlpha: srcAlphaFactor = 255 - alpha; break;
            case BlendFactor::DestColor: srcAlphaFactor = dstPixel[3]; break;
            case BlendFactor::InverseDestColor: srcAlphaFactor = 255 - dstPixel[3]; break;
            case BlendFactor::DestAlpha: srcAlphaFactor = dstPixel[3]; break;
            case BlendFactor::InverseDestAlpha: srcAlphaFactor = 255 - dstPixel[3]; break;
            default: srcAlphaFactor = 255; break;
        }

        switch (blendContext.alphaBlendFactorDst)
        {
            case BlendFactor::Zero: dstAlphaFactor = 0; break;
            case BlendFactor::One: dstAlphaFactor = 255; break;
            case BlendFactor::SourceColor: dstAlphaFactor = alpha; break;
            case BlendFactor::InverseSourceColor: dstAlphaFactor = 255 - alpha; break;
            case BlendFactor::SourceAlpha: dstAlphaFactor = alpha; break;
            case BlendFactor::InverseSourceAlpha: dstAlphaFactor = 255 - alpha; break;
            case BlendFactor::DestColor: dstAlphaFactor = dstPixel[3]; break;
            case BlendFactor::InverseDestColor: dstAlphaFactor = 255 - dstPixel[3]; break;
            case BlendFactor::DestAlpha: dstAlphaFactor = dstPixel[3]; break;
            case BlendFactor::InverseDestAlpha: dstAlphaFactor = 255 - dstPixel[3]; break;
            default: dstAlphaFactor = 255; break;
        }

        uint8_t blendedAlpha = (alpha * srcAlphaFactor + dstPixel[3] * dstAlphaFactor) / 255;

        // Combine the final color and alpha
        uint8_t invAlpha = 255 - blendedAlpha;

        for (int c = 0; c < 3; ++c)
        {
            dstPixel[c] = (blendedColor[c] * blendedAlpha + dstPixel[c] * invAlpha) >> 8;
        }
        dstPixel[3] = blendedAlpha;  // Update the alpha channel
    }
}