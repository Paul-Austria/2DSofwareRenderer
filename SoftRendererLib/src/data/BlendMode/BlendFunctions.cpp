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
        return;
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);
    PixelFormatInfo infoSrc = PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888);

    size_t batchSize = 1;                        // Process 4 pixels at once
    size_t fullBatches = pixelCount / batchSize; // Number of full batches
    size_t remainder = pixelCount % batchSize;   // Remaining pixels to process separately

    float alpha = srcColor.data[0] / 255.0f;

    // Masks and shifts for each channel
    uint16_t redMask = info.redMask, greenMask = info.greenMask, blueMask = info.blueMask, alphaMask = info.alphaMask;
    uint8_t redShift = info.redShift, greenShift = info.greenShift, blueShift = info.blueShift, alphaShift = info.alphaShift;

    PixelConverter::ConvertFunc convertFunc = nullptr;
    PixelConverter::ConvertFunc convertFuncBack = nullptr;

    convertFunc = PixelConverter::GetConversionFunction(format, PixelFormat::ARGB8888);
    convertFuncBack = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, format);

    // Blend full batches
    for (size_t batch = 0; batch < fullBatches; ++batch)
    {
        size_t baseIndex = batch * batchSize * info.bytesPerPixel;

        for (size_t j = 0; j < batchSize; ++j)
        {
            uint8_t *dstDataPointer = dstData + baseIndex + j * info.bytesPerPixel;

            Color dstColor;

            convertFunc(dstDataPointer, dstColor.data, 1);

            // Blend channels
            dstColor.data[1] = static_cast<uint8_t>(srcColor.data[1] * alpha + dstColor.data[1] * (1 - alpha));
            dstColor.data[2] = static_cast<uint8_t>(srcColor.data[2] * alpha + dstColor.data[2] * (1 - alpha));
            dstColor.data[3] = static_cast<uint8_t>(srcColor.data[3] * alpha + dstColor.data[3] * (1 - alpha));
            dstColor.data[0] = (alphaMask > 0) ? std::max(srcColor.data[0], dstColor.data[0]) : dstColor.data[0];

            convertFuncBack(dstColor.data, dstDataPointer, 1);
        }
    }
}

void BlendFunctions::BlendRow(uint8_t *dstRow, const uint8_t *srcRow, size_t rowLength, const PixelFormatInfo &targetInfo, const PixelFormatInfo &sourceInfo)
{
    // Temporary buffers for source and destination colors
    Color srcColor, dstColor;

    // Conversion functions for target and source formats
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceInfo.format, PixelFormat::ARGB8888);
    PixelConverter::ConvertFunc convertFuncTarget = PixelConverter::GetConversionFunction(targetInfo.format, PixelFormat::ARGB8888);

    PixelConverter::ConvertFunc convertFuncBack = PixelConverter::GetConversionFunction(PixelFormat::ARGB8888, targetInfo.format);

    uint8_t data[4];
    // Loop over each pixel in the row
    for (size_t i = 0; i < rowLength; ++i)
    {
        uint8_t *dstDataPointer = dstRow + i * targetInfo.bytesPerPixel;
        const uint8_t *srcDataPointer = srcRow + i * sourceInfo.bytesPerPixel;

        // Convert the source pixel to ARGB8888 format
        convertFunc(srcDataPointer, srcColor.data, 1);

        // Convert the destination pixel to ARGB8888 format
        convertFuncTarget(dstDataPointer, dstColor.data, 1);

        // Perform the blend (simple alpha blending in this case)
        float alpha = srcColor.data[0] / 255.0f; // Source alpha (assuming srcColor uses ARGB8888)
        if (alpha != 0)
        {

            dstColor.data[1] = static_cast<uint8_t>(srcColor.data[1] * alpha + dstColor.data[1] * (1 - alpha));
            dstColor.data[2] = static_cast<uint8_t>(srcColor.data[2] * alpha + dstColor.data[2] * (1 - alpha));
            dstColor.data[3] = static_cast<uint8_t>(srcColor.data[3] * alpha + dstColor.data[3] * (1 - alpha));
            dstColor.data[0] = std::max(srcColor.data[0], dstColor.data[0]);
        }

        // Create the blended color

        // Convert the blended color back to the target format and store it in the destination row
        convertFuncBack(dstColor.data, dstDataPointer, 1);
    }
}
