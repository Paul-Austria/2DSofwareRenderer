#ifndef BLENDMODE
#define BLENDMODE

#include "Color.h"
#include <immintrin.h> // For SIMD intrinsics
#include "PixelFormat/PixelFormat.h"
#include <algorithm>
#include "PixelFormat/PixelFormatInfo.h"

namespace Renderer2D
{
    enum class BlendMode
    {
        NOBLEND,
        BLEND,
    };

    enum class SelectedBlendMode
    {
        SIMPLE,
        MULTIPLY

    };
    static inline Color BlendSimple(const Color &src, const Color &dst)
    {
        // Simple alpha blending: result = src * srcAlpha + dst * (1 - srcAlpha)
        uint8_t srcAlpha = src.data[0];  // Get alpha from source color
        float alpha = srcAlpha / 255.0f; // Convert alpha to a float [0, 1]

        // Blend each color channel
        uint8_t blendedR = static_cast<uint8_t>(src.data[1] * alpha + dst.data[1] * (1 - alpha));
        uint8_t blendedG = static_cast<uint8_t>(src.data[2] * alpha + dst.data[2] * (1 - alpha));
        uint8_t blendedB = static_cast<uint8_t>(src.data[3] * alpha + dst.data[3] * (1 - alpha));

        // Preserve maximum alpha between src and dst
        uint8_t blendedA = std::max(src.data[0], dst.data[0]);

        return Color(blendedR, blendedG, blendedB, blendedA);
    }

#include <algorithm>
#include <cmath>

static inline void BlendSimpleSIMD(const Color &srcColor, uint8_t *dstData, PixelFormat format, size_t pixelCount)
{
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Determine how many full pixels we can process in a loop
    size_t batchSize = 4; // Process 4 pixels at once
    size_t fullBatches = pixelCount / batchSize; // Number of full batches
    size_t remainder = pixelCount % batchSize; // Remaining pixels to process separately

    // Get source alpha once for all pixels in this batch
    uint8_t srcAlpha = srcColor.data[0];  // Get alpha from source color (ARGB8888 format)
    float alpha = srcAlpha / 255.0f; // Convert alpha to a float [0, 1]

    // Get offsets for the channels based on the target pixel format
    int rOffset = info.redOffset;
    int gOffset = info.greenOffset;
    int bOffset = info.blueOffset;
    int aOffset = info.alphaOffset;

    // Process full batches
    for (size_t batch = 0; batch < fullBatches; ++batch)
    {
        // Calculate the base index for the current batch
        size_t baseIndex = batch * batchSize * info.bytesPerPixel;

        // Blend four pixels at once
        for (size_t j = 0; j < batchSize; ++j)
        {
            // Calculate the index for the current pixel
            size_t index = baseIndex + j * info.bytesPerPixel;

            // Extract destination pixel values directly from memory
            uint8_t dstR = dstData[index + rOffset];
            uint8_t dstG = dstData[index + gOffset];
            uint8_t dstB = dstData[index + bOffset];
            uint8_t dstA = dstData[index + aOffset];

            // Blend each channel
            uint8_t blendedR = static_cast<uint8_t>(srcColor.data[1] * alpha + dstR * (1 - alpha));
            uint8_t blendedG = static_cast<uint8_t>(srcColor.data[2] * alpha + dstG * (1 - alpha));
            uint8_t blendedB = static_cast<uint8_t>(srcColor.data[3] * alpha + dstB * (1 - alpha));
            uint8_t blendedA = std::max(srcAlpha, dstA); // Preserve max alpha

            // Store blended pixel back into memory
            dstData[index + rOffset] = blendedR;
            dstData[index + gOffset] = blendedG;
            dstData[index + bOffset] = blendedB;
            dstData[index + aOffset] = blendedA;
        }
    }

    // Process any remaining pixels
    for (size_t i = 0; i < remainder; ++i)
    {
        // Calculate the index for the remaining pixel
        size_t index = (fullBatches * batchSize + i) * info.bytesPerPixel;

        // Extract destination pixel values directly from memory
        uint8_t dstR = dstData[index + rOffset];
        uint8_t dstG = dstData[index + gOffset];
        uint8_t dstB = dstData[index + bOffset];
        uint8_t dstA = dstData[index + aOffset];

        // Blend each channel for the remaining pixel
        uint8_t blendedR = static_cast<uint8_t>(srcColor.data[1] * alpha + dstR * (1 - alpha));
        uint8_t blendedG = static_cast<uint8_t>(srcColor.data[2] * alpha + dstG * (1 - alpha));
        uint8_t blendedB = static_cast<uint8_t>(srcColor.data[3] * alpha + dstB * (1 - alpha));
        uint8_t blendedA = std::max(srcAlpha, dstA); // Preserve max alpha

        // Store blended pixel back into memory
        dstData[index + rOffset] = blendedR;
        dstData[index + gOffset] = blendedG;
        dstData[index + bOffset] = blendedB;
        dstData[index + aOffset] = blendedA;
    }
}



}

#endif