#include "BlendMode.h"
#include "BlendFunctions.h"
#include "../PixelFormat/PixelFormatInfo.h"

#include <algorithm>
#include <cmath>

using namespace Renderer2D;

void BlendFunctions::BlendSimpleSolidColor(const Color &srcColor, uint8_t *dstData, PixelFormat format, size_t pixelCount)
{
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Determine how many full pixels we can process in a loop
    size_t batchSize = 4;                        // Process 4 pixels at once
    size_t fullBatches = pixelCount / batchSize; // Number of full batches
    size_t remainder = pixelCount % batchSize;   // Remaining pixels to process separately

    // Get source alpha and convert it to a normalized float [0, 1]
    uint8_t srcAlpha = srcColor.data[0]; // Get alpha from source color (ARGB8888 format)
    float alpha = srcAlpha / 255.0f;

    // Get offsets for each channel, or -1 if not present
    int rOffset = info.redOffset;
    int gOffset = info.greenOffset;
    int bOffset = info.blueOffset;
    int aOffset = info.alphaOffset;

    // Blend full batches of pixels
    for (size_t batch = 0; batch < fullBatches; ++batch)
    {
        size_t baseIndex = batch * batchSize * info.bytesPerPixel;

        for (size_t j = 0; j < batchSize; ++j)
        {
            size_t index = baseIndex + j * info.bytesPerPixel;

            // Extract destination pixel values based on format
            uint8_t dstR = (rOffset >= 0) ? dstData[index + rOffset] : 0;
            uint8_t dstG = (gOffset >= 0) ? dstData[index + gOffset] : 0;
            uint8_t dstB = (bOffset >= 0) ? dstData[index + bOffset] : 0;
            uint8_t dstA = (aOffset >= 0) ? dstData[index + aOffset] : 255;

            // Blend each channel
            uint8_t blendedR = (rOffset >= 0) ? static_cast<uint8_t>(srcColor.data[1] * alpha + dstR * (1 - alpha)) : dstR;
            uint8_t blendedG = (gOffset >= 0) ? static_cast<uint8_t>(srcColor.data[2] * alpha + dstG * (1 - alpha)) : dstG;
            uint8_t blendedB = (bOffset >= 0) ? static_cast<uint8_t>(srcColor.data[3] * alpha + dstB * (1 - alpha)) : dstB;
            uint8_t blendedA = (aOffset >= 0) ? std::max(srcAlpha, dstA) : dstA;

            // Write blended color back to memory
            if (rOffset >= 0)
                dstData[index + rOffset] = blendedR;
            if (gOffset >= 0)
                dstData[index + gOffset] = blendedG;
            if (bOffset >= 0)
                dstData[index + bOffset] = blendedB;
            if (aOffset >= 0)
                dstData[index + aOffset] = blendedA;
        }
    }

    // Process any remaining pixels individually
    for (size_t i = 0; i < remainder; ++i)
    {
        size_t index = (fullBatches * batchSize + i) * info.bytesPerPixel;

        uint8_t dstR = (rOffset >= 0) ? dstData[index + rOffset] : 0;
        uint8_t dstG = (gOffset >= 0) ? dstData[index + gOffset] : 0;
        uint8_t dstB = (bOffset >= 0) ? dstData[index + bOffset] : 0;
        uint8_t dstA = (aOffset >= 0) ? dstData[index + aOffset] : 255;

        uint8_t blendedR = (rOffset >= 0) ? static_cast<uint8_t>(srcColor.data[1] * alpha + dstR * (1 - alpha)) : dstR;
        uint8_t blendedG = (gOffset >= 0) ? static_cast<uint8_t>(srcColor.data[2] * alpha + dstG * (1 - alpha)) : dstG;
        uint8_t blendedB = (bOffset >= 0) ? static_cast<uint8_t>(srcColor.data[3] * alpha + dstB * (1 - alpha)) : dstB;
        uint8_t blendedA = (aOffset >= 0) ? std::max(srcAlpha, dstA) : dstA;

        if (rOffset >= 0)
            dstData[index + rOffset] = blendedR;
        if (gOffset >= 0)
            dstData[index + gOffset] = blendedG;
        if (bOffset >= 0)
            dstData[index + bOffset] = blendedB;
        if (aOffset >= 0)
            dstData[index + aOffset] = blendedA;
    }
}
void BlendFunctions::BlendRow(uint8_t *dstRow, const uint8_t *srcRow, size_t rowLength, const PixelFormatInfo &targetInfo, const PixelFormatInfo &sourceInfo)
{
    for (size_t i = 0; i < rowLength; ++i)
    {
        // Calculate pointers for each pixel in the row
        uint8_t *dstPixel = dstRow + i * targetInfo.bytesPerPixel;
        const uint8_t *srcPixel = srcRow + i * sourceInfo.bytesPerPixel;

        // Extract RGBA components for both source and destination pixels
        uint8_t dstR = dstPixel[targetInfo.redOffset];
        uint8_t dstG = dstPixel[targetInfo.greenOffset];
        uint8_t dstB = dstPixel[targetInfo.blueOffset];
        uint8_t dstA = dstPixel[targetInfo.alphaOffset];

        uint8_t srcR = srcPixel[sourceInfo.redOffset];
        uint8_t srcG = srcPixel[sourceInfo.greenOffset];
        uint8_t srcB = srcPixel[sourceInfo.blueOffset];
        uint8_t srcA = srcPixel[sourceInfo.alphaOffset];

        // Normalize the source alpha (fast float-to-int conversion)
        float alpha = srcA * (1.0f / 255.0f);

        // Calculate blended channels (alpha blending formula: src * alpha + dst * (1 - alpha))
        uint8_t blendedR = static_cast<uint8_t>(srcR * alpha + dstR * (1 - alpha));
        uint8_t blendedG = static_cast<uint8_t>(srcG * alpha + dstG * (1 - alpha));
        uint8_t blendedB = static_cast<uint8_t>(srcB * alpha + dstB * (1 - alpha));

        // Max of alpha channels to combine (no division needed)
        uint8_t blendedA = std::max(srcA, dstA);

        // Write back to destination pixel
        dstPixel[targetInfo.redOffset] = blendedR;
        dstPixel[targetInfo.greenOffset] = blendedG;
        dstPixel[targetInfo.blueOffset] = blendedB;
        dstPixel[targetInfo.alphaOffset] = blendedA;
    }
}
