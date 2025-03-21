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

        // Compute the source and destination color factors based on the BlendContext
        uint8_t srcFactor[3], dstFactor[3];  // For R, G, B
        uint8_t srcAlphaFactor, dstAlphaFactor;




        // Helper function to calculate the blend factor for R, G, B
        auto getFactor = [](BlendFactor factor, uint8_t src, uint8_t dst, uint8_t alpha) -> uint8_t {
            switch (factor)
            {
                case BlendFactor::Zero: return 0;
                case BlendFactor::One: return 255;
                case BlendFactor::SourceColor: return src;
                case BlendFactor::InverseSourceColor: return 255 - src;
                case BlendFactor::SourceAlpha: return alpha;
                case BlendFactor::InverseSourceAlpha: return 255 - alpha;
                case BlendFactor::DestColor: return dst;
                case BlendFactor::InverseDestColor: return 255 - dst;
                case BlendFactor::DestAlpha: return dst;
                case BlendFactor::InverseDestAlpha: return 255 - dst;
                default: return 255;  // default to One
            }
        };

        // Calculate color factors for R, G, B
        srcFactor[0] = getFactor(BlendFactor::SourceAlpha, srcPixel[0], dstPixel[0], alpha);
        dstFactor[0] = getFactor(BlendFactor::InverseSourceAlpha, srcPixel[0], dstPixel[0], alpha);

        srcFactor[1] = getFactor(BlendFactor::SourceAlpha, srcPixel[1], dstPixel[1], alpha);
        dstFactor[1] = getFactor(BlendFactor::InverseSourceAlpha, srcPixel[1], dstPixel[1], alpha);

        srcFactor[2] = getFactor(BlendFactor::SourceAlpha, srcPixel[2], dstPixel[2], alpha);
        dstFactor[2] = getFactor(BlendFactor::InverseSourceAlpha, srcPixel[2], dstPixel[2], alpha);

        // Apply color blend operation (simplified, add mode here)
        uint8_t srcColor[3] = { srcRGB24[i * 3], srcRGB24[i * 3 + 1], srcRGB24[i * 3 + 2] };
        uint8_t blendedColor[3];
        for (int c = 0; c < 3; ++c)
        {
            blendedColor[c] = (srcColor[c] * srcFactor[c] + dstPixel[c] * dstFactor[c]) / 255;
        }

        // Calculate alpha factors (same approach as for color channels)
        srcAlphaFactor = getFactor(BlendFactor::One, alpha, dstPixel[3], alpha);
        dstAlphaFactor = getFactor(BlendFactor::One, alpha, dstPixel[3], alpha);

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
