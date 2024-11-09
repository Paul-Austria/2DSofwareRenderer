#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

using namespace Renderer2D;

#define M_PI 3.14159265358979323846f /* pi */

RenderContext2D::RenderContext2D()
{
}
void RenderContext2D::SetTargetTexture(Texture *targettexture)
{
    this->targetTexture = targettexture;
}

void RenderContext2D::SetBlendMode(BlendMode mode)
{
    this->mode = mode;
}

void RenderContext2D::ClearTarget(Color color)
{
    if (targetTexture == nullptr)
    {
        return; // Early exit if no target texture
    }

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Get pointer to the texture data
    uint8_t *textureData = targetTexture->GetData();
    uint16_t width = targetTexture->GetWidth();
    uint16_t height = targetTexture->GetHeight();

    // Use a stack-allocated array for pixel data
    uint8_t pixelData[4]; // Assuming maximum bytes per pixel is 4 (RGBA)
    color.ConvertTo(format, pixelData);

    // Calculate the total number of pixels
    size_t totalPixels = width * height;

    // Clear the texture by filling it with the converted color
    for (size_t i = 0; i < totalPixels; ++i)
    {
        // Calculate the position in the texture data array
        size_t pixelIndex = i * info.bytesPerPixel;
        std::memcpy(&textureData[pixelIndex], pixelData, info.bytesPerPixel);
    }
}

void RenderContext2D::DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height)
{
    if (!targetTexture)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Access the texture data and dimensions
    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();

    // Set clipping boundaries (with respect to the rectangle's position)
    uint16_t clipStartX = enableClipping ? std::max(x, startX) : x;
    uint16_t clipStartY = enableClipping ? std::max(y, startY) : y;
    uint16_t clipEndX = enableClipping ? std::min(static_cast<int>(x + length), static_cast<int>(endX)) : x + length;
    uint16_t clipEndY = enableClipping ? std::min(static_cast<int>(y + height), static_cast<int>(endY)) : y + height;

    // Restrict drawing within the texture bounds
    clipEndX = std::min(clipEndX, textureWidth);
    clipEndY = std::min(clipEndY, textureHeight);

    // If nothing to draw, return
    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Calculate the number of bytes in a row
    size_t bytesPerRow = (clipEndX - clipStartX) * info.bytesPerPixel;

    BlendMode subBlend = mode;
    if (color.GetAlpha() == 255)
        subBlend = BlendMode::NOBLEND;

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {
        // Fast, unblended rectangle drawing within the specified bounds
        uint8_t *dest = textureData + (clipStartY * textureWidth + clipStartX) * info.bytesPerPixel;

        // Convert the color to the format of the texture for NOBLEND (no need for alpha)
        std::vector<uint8_t> pixelData(info.bytesPerPixel);
        color.ConvertTo(format, pixelData.data());

        // Prepare a full row of pixel data (the color to be drawn)
        std::vector<uint8_t> rowPixelData(bytesPerRow);
        // Create the single pixel data
        std::vector<uint8_t> singlePixelData(info.bytesPerPixel);
        MemHandler::MemCopy(singlePixelData.data(), pixelData.data(), info.bytesPerPixel);

        // Fill the entire row by repeating the single pixel data
        for (size_t byteIndex = 0; byteIndex < bytesPerRow; byteIndex += info.bytesPerPixel)
        {
            std::copy(singlePixelData.begin(), singlePixelData.end(), rowPixelData.begin() + byteIndex);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            // Calculate the destination for the current row
            uint8_t *rowDest = dest + j * textureWidth * info.bytesPerPixel;

            // Use memcpy to copy the whole row of pixels at once
            MemHandler::MemCopy(rowDest, rowPixelData.data(), bytesPerRow);
        }
        break;
    }
    case BlendMode::BLEND:
    {
        // Get the source row
        uint8_t *dest = textureData + (clipStartY * textureWidth + clipStartX) * info.bytesPerPixel;

        // Instead of creating a new buffer for every pixel, reuse a single buffer for conversion
        std::vector<uint8_t> convertedPixel(info.bytesPerPixel);

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *rowDest = dest + j * textureWidth * info.bytesPerPixel;
            size_t rowLength = (clipEndX - clipStartX); // Number of pixels per row

            BlendFunctions::BlendSimpleSolidColor(color, rowDest, format, rowLength);
        }
        break;
    }
    default:
        break;
    }
}

void RenderContext2D::DrawTexture(Texture &texture, uint16_t x, uint16_t y)
{
    if (!targetTexture)
        return;

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();

    // Set clipping boundaries within the source and target textures
    uint16_t clipStartX = enableClipping ? std::max(x, startX) : x;
    uint16_t clipStartY = enableClipping ? std::max(y, startY) : y;
    uint16_t clipEndX = enableClipping ? std::min(static_cast<int>(x + sourceWidth), static_cast<int>(endX)) : x + sourceWidth;
    uint16_t clipEndY = enableClipping ? std::min(static_cast<int>(y + sourceHeight), static_cast<int>(endY)) : y + sourceHeight;

    // Restrict drawing to the target texture’s bounds
    clipEndX = std::min(clipEndX, targetWidth);
    clipEndY = std::min(clipEndY, targetHeight);

    // Check if there is anything to draw
    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Get the blending mode to use
    BlendMode subBlend = (mode == BlendMode::NOBLEND || sourceInfo.hasAlpha) ? mode : BlendMode::NOBLEND;

    // Conversion function for source to target format if necessary
    PixelConverter::ConvertFunc convertFunc = nullptr;
    if (sourceFormat != targetFormat)
    {
        convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
        if (!convertFunc)
        {
            return;
        }
    }

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {
        std::vector<uint8_t> convertedRow((clipEndX - clipStartX) * targetInfo.bytesPerPixel);
        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *targetRow = targetData + (j * targetWidth + clipStartX) * targetInfo.bytesPerPixel;
            const uint8_t *sourceRow = sourceData + ((j - y) * sourceWidth + (clipStartX - x)) * sourceInfo.bytesPerPixel;

            if (convertFunc)
            {
                // Convert each pixel in the row from source to target format and copy directly
                convertFunc(sourceRow, convertedRow.data(), clipEndX - clipStartX);
                MemHandler::MemCopy(targetRow, convertedRow.data(), convertedRow.size());
            }
            else
            {
                // Direct copy if formats match
                MemHandler::MemCopy(targetRow, sourceRow, (clipEndX - clipStartX) * targetInfo.bytesPerPixel);
            }
        }
        break;
    }

    case BlendMode::BLEND:
    {
        const uint8_t *srcPixelBase = sourceData;
        uint8_t *dstPixelBase = targetData;

        // Directly calculate row strides for source and target
        size_t targetRowStride = targetWidth * targetInfo.bytesPerPixel;
        size_t sourceRowStride = sourceWidth * sourceInfo.bytesPerPixel;

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            // Calculate the start of the row for both source and target
            uint8_t *targetRow = dstPixelBase + (j * targetWidth + clipStartX) * targetInfo.bytesPerPixel;
            const uint8_t *sourceRow = srcPixelBase + ((j - y) * sourceWidth + (clipStartX - x)) * sourceInfo.bytesPerPixel;

            // Blend the entire row at once
            BlendFunctions::BlendRow(targetRow, sourceRow, clipEndX - clipStartX, targetInfo, sourceInfo);
        }

        break;
    }

    default:
        break;
    }
}

void RenderContext2D::EnableClipping(bool clipping)
{
    this->enableClipping = clipping;
}
void RenderContext2D::SetClipping(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    this->startX = startX;
    this->startY = startY;
    this->endX = endX;
    this->endY = endY;
}