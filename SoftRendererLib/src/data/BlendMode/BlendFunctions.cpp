#include "BlendMode.h"
#include "BlendFunctions.h"
#include "../PixelFormat/PixelConverter.h"
#include "../PixelFormat/PixelFormatInfo.h"

#include <algorithm>
#include <cmath>

using namespace Renderer2D;
void BlendFunctions::BlendSimpleSolidColor(const Color &srcColor, uint8_t *dstData, PixelFormat format, size_t pixelCount)
{
    if (srcColor.data[0] == 0)
        return; // No blending needed if source alpha is 0

    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Precompute alpha values
    float alpha = srcColor.data[0] / 255.0f;
    float invAlpha = 1.0f - alpha;

    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(format, PixelFormat::ARGB8888);
    PixelConverter::ConvertFunc convertFuncBack = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, format);
    if (!convertFunc || !convertFuncBack)
        return;

    // Buffer for batch processing
    constexpr size_t batchSize = 16; // Number of pixels to process per batch
    constexpr size_t colorSize = 4;  // Size of ARGB8888 (4 bytes per color)
    uint8_t srcBatch[batchSize * colorSize]; // Buffer for source color in ARGB8888
    uint8_t dstBatch[batchSize * colorSize]; // Buffer for destination pixels in ARGB8888

    // Precompute source color in ARGB8888 for batch
    for (size_t i = 0; i < batchSize; ++i)
    {
        std::memcpy(srcBatch + i * colorSize, srcColor.data, colorSize);
    }

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
            const uint8_t *srcPixel = srcBatch + i * colorSize;

            dstPixel[1] = static_cast<uint8_t>(srcPixel[1] * alpha + dstPixel[1] * invAlpha); // Red
            dstPixel[2] = static_cast<uint8_t>(srcPixel[2] * alpha + dstPixel[2] * invAlpha); // Green
            dstPixel[3] = static_cast<uint8_t>(srcPixel[3] * alpha + dstPixel[3] * invAlpha); // Blue
            dstPixel[0] = std::max(srcPixel[0], dstPixel[0]); // Alpha
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
            const uint8_t *srcPixel = srcBatch + i * colorSize;

            dstPixel[1] = static_cast<uint8_t>(srcPixel[1] * alpha + dstPixel[1] * invAlpha); // Red
            dstPixel[2] = static_cast<uint8_t>(srcPixel[2] * alpha + dstPixel[2] * invAlpha); // Green
            dstPixel[3] = static_cast<uint8_t>(srcPixel[3] * alpha + dstPixel[3] * invAlpha); // Blue
            dstPixel[0] = std::max(srcPixel[0], dstPixel[0]); // Alpha
        }

        convertFuncBack(dstBatch, dstData + baseIndex, remainder);
    }
}


void BlendFunctions::BlendRow(uint8_t *dstRow, const uint8_t *srcRow, size_t rowLength, const PixelFormatInfo &targetInfo, const PixelFormatInfo &sourceInfo)
{
    // Conversion functions for target and source formats
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::ARGB8888);
    PixelConverter::ConvertFunc convertFuncTarget = PixelConverter::GetConversionFunction(targetInfo.format, PixelFormat::ARGB8888);
    PixelConverter::ConvertFunc convertFuncBack = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    // Loop over each pixel in the row
    for (size_t i = 0; i < rowLength; ++i)
    {
        uint8_t *dstDataPointer = dstRow + i * targetInfo.bytesPerPixel;
        const uint8_t *srcDataPointer = srcRow + i * sourceInfo.bytesPerPixel;

        // Convert the source pixel to ARGB8888 format
        Color srcColor;
        convertFunc(srcDataPointer, srcColor.data, 1);

        // Get the source alpha once for the current pixel
        uint8_t srcAlpha = srcColor.data[0];
        if (srcAlpha == 0) {
            continue;  // No need to blend if the source alpha is 0
        }

        // Convert the destination pixel to ARGB8888 format
        Color dstColor;
        convertFuncTarget(dstDataPointer, dstColor.data, 1);

        // Invert the source alpha once
        uint8_t invAlpha = 255 - srcAlpha;

        // Perform the blend using the alpha values directly, optimizing for all channels at once
        for (int c = 1; c <= 3; ++c) {
            dstColor.data[c] = (srcColor.data[c] * srcAlpha + dstColor.data[c] * invAlpha) >> 8;
        }

        // The maximum of the source and destination alpha is used
        dstColor.data[0] = std::max(srcAlpha, dstColor.data[0]);

        // Convert the blended color back to the target format and store it in the destination row
        convertFuncBack(dstColor.data, dstDataPointer, 1);
    }
}
