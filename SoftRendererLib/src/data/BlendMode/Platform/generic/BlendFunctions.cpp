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
                                BlendMode selectedBlendMode)
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
                                BlendMode selectedBlendMode)
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
        uint8_t alpha = 255; // Default alpha value

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


