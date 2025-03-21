#include "../../BlendMode.h"
#include "../../BlendFunctions.h"
#include "../../../PixelFormat/PixelConverter.h"
#include "../../../PixelFormat/PixelFormatInfo.h"

using namespace Tergos2D;

#include <arm_neon.h>
void BlendFunctions::BlendSolidRowRGB24(uint8_t *dstRow,
                                        const uint8_t *srcRow,
                                        size_t rowLength,
                                        const PixelFormatInfo &targetInfo,
                                        const PixelFormatInfo &sourceInfo,
                                        Coloring coloring,
                                        bool useSolidColor,
                                        BlendMode selectedBlendMode)
{
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    if(convertToRGB24 == nullptr) return;

    alignas(16) uint8_t srcRGB24[3];
    convertToRGB24(srcRow, srcRGB24, 1);

    uint8_t colorFactor = coloring.colorEnabled ? coloring.color.data[0] : 0;
    uint8_t inverseColorFactor = 255 - colorFactor;

    uint8_t alpha = 255;
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

    uint8x8_t vSrcColor = vdup_n_u8(srcRGB24[0]);  // Red
    uint8x8_t vSrcColorG = vdup_n_u8(srcRGB24[1]); // Green
    uint8x8_t vSrcColorB = vdup_n_u8(srcRGB24[2]); // Blue
    uint8x8_t vAlpha = vdup_n_u8(alpha);
    uint8x8_t vInvAlpha = vdup_n_u8(invAlpha);

    size_t i = 0;
    for (; i + 8 * 3 <= rowLength * 3; i += 8 * 3)
    {
        uint8x8x3_t dstPixel = vld3_u8(dstRow + i);

        // Perform alpha blending: dst = (src * alpha + dst * invAlpha) >> 8
        dstPixel.val[0] = vshrn_n_u16(vmlal_u8(vmull_u8(vSrcColor, vAlpha), dstPixel.val[0], vInvAlpha), 8);
        dstPixel.val[1] = vshrn_n_u16(vmlal_u8(vmull_u8(vSrcColorG, vAlpha), dstPixel.val[1], vInvAlpha), 8);
        dstPixel.val[2] = vshrn_n_u16(vmlal_u8(vmull_u8(vSrcColorB, vAlpha), dstPixel.val[2], vInvAlpha), 8);

        vst3_u8(dstRow + i, dstPixel);
    }

    // Handle remaining pixels (scalar fallback)
    for (; i < rowLength * 3; i += 3)
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
                                BlendMode selectedBlendMode)
{

    // Conversion function for the source format
    PixelConverter::ConvertFunc convertToRGB24 = PixelConverter::GetConversionFunction(sourceInfo.format, targetInfo.format);
    if(convertToRGB24 == nullptr) return;
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




void BlendFunctions::BlendRGBA32ToRGB24(uint8_t *dstRow,
                               const uint8_t *srcRow,
                               size_t rowLength,
                               const PixelFormatInfo &targetInfo,
                               const PixelFormatInfo &sourceInfo,
                               Coloring coloring,
                               bool useSolidColor,
                               BlendMode selectedBlendMode)
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
        uint8_t alpha = (selectedBlendMode == BlendMode::COLORINGONLY) * 255 + (selectedBlendMode != BlendMode::COLORINGONLY) * srcPixel[3];

        uint8_t mask = -(alpha != 0);
        alpha &= mask;

        uint8_t *srcColor = &srcRGB24[i * 3];

        srcColor[0] = ((srcColor[0] * inverseColorFactor) + (colorR * colorFactor)) >> 8;
        srcColor[1] = ((srcColor[1] * inverseColorFactor) + (colorG * colorFactor)) >> 8;
        srcColor[2] = ((srcColor[2] * inverseColorFactor) + (colorB * colorFactor)) >> 8;

        uint8_t invAlpha = 255 - alpha;

        dstPixel[0] = ((srcColor[0] * alpha) + (dstPixel[0] * invAlpha)) >> 8;
        dstPixel[1] = ((srcColor[1] * alpha) + (dstPixel[1] * invAlpha)) >> 8;
        dstPixel[2] = ((srcColor[2] * alpha) + (dstPixel[2] * invAlpha)) >> 8;
    }
}
