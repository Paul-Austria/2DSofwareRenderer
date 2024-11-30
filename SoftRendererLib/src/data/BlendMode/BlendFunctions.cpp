#include "BlendMode.h"
#include "BlendFunctions.h"
#include "../PixelFormat/PixelConverter.h"
#include "../PixelFormat/PixelFormatInfo.h"

#include <algorithm>
#include <cmath>

using namespace Renderer2D;
void BlendFunctions::BlendSimpleSolidColor(const Color &srcColor,
                                         uint8_t *dstData,
                                         PixelFormat format,
                                         size_t pixelCount,
                                         SelectedBlendMode selectedBlendMode)
{
    if (srcColor.data[0] == 0)
        return; // No blending needed if source alpha is 0

    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Get conversion functions
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(format, PixelFormat::ARGB8888);
    PixelConverter::ConvertFunc convertFuncBack = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, format);
    if (!convertFunc || !convertFuncBack)
        return;

    // Extract source alpha and compute inverse alpha
    uint8_t srcAlpha = srcColor.data[0];
    uint8_t invAlpha = 255 - srcAlpha;

    // Prepare source color in ARGB8888 for blending
    alignas(16) uint8_t srcARGB8888[4];
    std::memcpy(srcARGB8888, srcColor.data, 4); // Copy ARGB8888 data from srcColor

    // Buffer for batch processing
    constexpr size_t batchSize = 16;         // Number of pixels to process per batch
    constexpr size_t colorSize = 4;          // Size of ARGB8888 (4 bytes per color)
    alignas(16) uint8_t dstBatch[batchSize * colorSize]; // Buffer for destination pixels in ARGB8888

    // Process in batches
    size_t fullBatches = pixelCount / batchSize;
    size_t remainder = pixelCount % batchSize;

    for (size_t batch = 0; batch < fullBatches; ++batch)
    {
        size_t baseIndex = batch * batchSize * info.bytesPerPixel;

        // Convert destination pixels to ARGB8888 format
        convertFunc(dstData + baseIndex, dstBatch, batchSize);

        // Blend each pixel in the batch
        for (size_t i = 0; i < batchSize; ++i)
        {
            uint8_t *dstPixel = dstBatch + i * colorSize;

            // Blend RGB channels using integer arithmetic
            for (int c = 1; c <= 3; ++c) // Red, Green, Blue channels
            {
                dstPixel[c] = (srcARGB8888[c] * srcAlpha + dstPixel[c] * invAlpha) >> 8;
            }

            // Update alpha channel (max of source and destination alpha)
            dstPixel[0] = std::max(srcARGB8888[0], dstPixel[0]);
        }

        // Convert back to the destination format
        convertFuncBack(dstBatch, dstData + baseIndex, batchSize);
    }

    // Handle remaining pixels
    if (remainder > 0)
    {
        size_t baseIndex = fullBatches * batchSize * info.bytesPerPixel;

        convertFunc(dstData + baseIndex, dstBatch, remainder);

        for (size_t i = 0; i < remainder; ++i)
        {
            uint8_t *dstPixel = dstBatch + i * colorSize;

            // Blend RGB channels using integer arithmetic
            for (int c = 1; c <= 3; ++c) // Red, Green, Blue channels
            {
                dstPixel[c] = (srcARGB8888[c] * srcAlpha + dstPixel[c] * invAlpha) >> 8;
            }

            // Update alpha channel (max of source and destination alpha)
            dstPixel[0] = std::max(srcARGB8888[0], dstPixel[0]);
        }

        convertFuncBack(dstBatch, dstData + baseIndex, remainder);
    }
}

void BlendFunctions::BlendRow(uint8_t *dstRow, 
const uint8_t *srcRow, 
size_t rowLength,
const PixelFormatInfo &targetInfo, 
const PixelFormatInfo &sourceInfo,
SelectedBlendMode selectedBlendMode)
{
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

