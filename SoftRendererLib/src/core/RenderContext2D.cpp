#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"
#include <cmath>
#include <algorithm>
#include <stdio.h>

using namespace Renderer2D;

#define M_PI 3.14159265358979323846f /* pi */

RenderContext2D::RenderContext2D()
{
    SetCustomBlendFunction(BlendSimple);
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
        std::memcpy(singlePixelData.data(), pixelData.data(), info.bytesPerPixel);

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
            std::memcpy(rowDest, rowPixelData.data(), bytesPerRow);
        }
        break;
    }
    case BlendMode::BLEND:
    {
        if (blendFunction)
        {
            // Get the source row
            uint8_t *dest = textureData + (clipStartY * textureWidth + clipStartX) * info.bytesPerPixel;

            // Instead of creating a new buffer for every pixel, reuse a single buffer for conversion
            std::vector<uint8_t> convertedPixel(info.bytesPerPixel);

            for (uint16_t j = clipStartY; j < clipEndY; ++j)
            {
                uint8_t *rowDest = dest + j * textureWidth * info.bytesPerPixel;
                size_t rowLength = (clipEndX - clipStartX); // Number of pixels per row

                BlendSimpleSIMD(color, rowDest, format, rowLength);
            }
        }
        break;
    }
    default:
        break;
    }
}

void RenderContext2D::DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height, float angle)
{
    if (targetTexture == nullptr)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Get pointer to the texture data
    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();

    // Allocate memory for storing the converted pixel color
    uint8_t *pixelData = new uint8_t[info.bytesPerPixel];

    // Convert the input color to the format of the texture
    color.ConvertTo(format, pixelData);

    // Precompute sine and cosine of the rotation angle
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    // Center of the rectangle
    float centerX = x + length / 2.0f;
    float centerY = y + height / 2.0f;

    // Loop over every pixel in the texture space
    for (uint16_t i = 0; i < textureWidth; i++)
    {
        for (uint16_t j = 0; j < textureHeight; j++)
        {
            // Calculate the coordinates relative to the rectangle center
            float localX = i - centerX;
            float localY = j - centerY;

            // Apply reverse rotation (unrotate the pixel)
            float originalX = localX * cosAngle + localY * sinAngle;
            float originalY = -localX * sinAngle + localY * cosAngle;

            // Shift the coordinates back to the original rectangle's space
            float rectX = originalX + length / 2.0f;
            float rectY = originalY + height / 2.0f;

            // Check if the pixel falls within the original rectangle bounds
            if (rectX >= 0 && rectX < length && rectY >= 0 && rectY < height)
            {
                // Calculate the offset in the texture data based on the pixel position
                size_t offset = (j * textureWidth + i) * info.bytesPerPixel;

                // Copy the converted color data into the texture at the correct position
                memcpy(&textureData[offset], pixelData, info.bytesPerPixel);
            }
        }
    }

    // Free allocated memory for pixel data
    delete[] pixelData;
}

void RenderContext2D::DrawArray(uint8_t *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat)
{
    if (targetTexture == nullptr)
        return;

    // Get target texture properties
    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();

    // Clip the drawing area to avoid going out of bounds
    uint16_t clippedWidth = std::min(width, (uint16_t)(textureWidth - x));
    uint16_t clippedHeight = std::min(height, (uint16_t)(textureHeight - y));

    // Ensure the width and height are not negative
    if (clippedWidth <= 0 || clippedHeight <= 0)
        return;

    // Assuming source and target formats are the same
    for (uint16_t posY = 0; posY < clippedHeight; ++posY)
    {
        for (uint16_t posX = 0; posX < clippedWidth; ++posX)
        {
            // Source pixel index from `data` (assuming 3 bytes per pixel for RGB)
            uint32_t sourcePixelIndex = (posY * width + posX) * 3; // Assuming RGB24 (3 bytes per pixel)
            // Target pixel index in the texture data
            uint32_t targetPixelIndex = ((y + posY) * textureWidth + (x + posX)) * 3;

            // Copy RGB data from source to target
            textureData[targetPixelIndex] = data[sourcePixelIndex];         // Red
            textureData[targetPixelIndex + 1] = data[sourcePixelIndex + 1]; // Green
            textureData[targetPixelIndex + 2] = data[sourcePixelIndex + 2]; // Blue
        }
    }
}

void RenderContext2D::DrawArray(uint8_t *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat, float scaleX, float scaleY, float angleDegrees)
{
    if (targetTexture == nullptr)
        return;

    // Convert degrees to radians
    float angle = angleDegrees * M_PI / 180.0f;

    // Get target texture properties
    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();

    // Calculate the center of the source image
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;

    // Precompute sine and cosine of the angle for rotation
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    // Loop over each pixel in the target texture
    for (uint16_t i = 0; i < textureWidth; i++)
    {
        for (uint16_t j = 0; j < textureHeight; j++)
        {
            // Calculate the position relative to the center of the destination
            float destX = i - (x + width / 2.0f);
            float destY = j - (y + height / 2.0f);

            // Calculate the corresponding source pixel coordinates
            float scaledX = destX / scaleX;
            float scaledY = destY / scaleY;

            // Apply rotation to the scaled coordinates
            float srcX = scaledX * cosAngle - scaledY * sinAngle + centerX;
            float srcY = scaledX * sinAngle + scaledY * cosAngle + centerY;

            // Check if the source coordinates fall within the bounds of the source image
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height)
            {
                // Source pixel index from `data` (assuming 3 bytes per pixel for RGB)
                uint32_t sourcePixelIndex = ((int)srcY * width + (int)srcX) * 3; // Assuming RGB24 (3 bytes per pixel)

                // Target pixel index in the texture data
                uint32_t targetPixelIndex = (j * textureWidth + i) * 3;

                // Ensure the source pixel index is within bounds
                if (sourcePixelIndex < width * height * 3)
                {
                    // Copy RGB data from source to target
                    textureData[targetPixelIndex] = data[sourcePixelIndex];         // Red
                    textureData[targetPixelIndex + 1] = data[sourcePixelIndex + 1]; // Green
                    textureData[targetPixelIndex + 2] = data[sourcePixelIndex + 2]; // Blue
                }
            }
        }
    }
}

void RenderContext2D::DrawArray(uint8_t *data, uint16_t x, uint16_t y, uint16_t width, uint16_t height, PixelFormat sourceFormat, float scaleX, float scaleY, float angleDegrees, float pivotX, float pivotY)
{
    if (targetTexture == nullptr)
        return;

    // Convert degrees to radians
    float angle = angleDegrees * M_PI / 180.0f;

    // Get target texture properties
    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();

    // Precompute sine and cosine of the angle for rotation
    float cosAngle = cos(angle);
    float sinAngle = sin(angle);

    // Loop over each pixel in the target texture
    for (uint16_t i = 0; i < textureWidth; i++)
    {
        for (uint16_t j = 0; j < textureHeight; j++)
        {
            // Calculate the position relative to the pivot point of the destination
            float destX = i - (x + pivotX); // Change this to use pivotX
            float destY = j - (y + pivotY); // Change this to use pivotY

            // Calculate the corresponding source pixel coordinates
            float scaledX = destX / scaleX;
            float scaledY = destY / scaleY;

            // Apply rotation to the scaled coordinates
            float srcX = scaledX * cosAngle - scaledY * sinAngle + (width / 2.0f) + pivotX;  // Adjust for pivotX
            float srcY = scaledX * sinAngle + scaledY * cosAngle + (height / 2.0f) + pivotY; // Adjust for pivotY

            // Check if the source coordinates fall within the bounds of the source image
            if (srcX >= 0 && srcX < width && srcY >= 0 && srcY < height)
            {
                // Source pixel index from `data` (assuming 3 bytes per pixel for RGB)
                uint32_t sourcePixelIndex = ((int)srcY * width + (int)srcX) * 3; // Assuming RGB24 (3 bytes per pixel)

                // Target pixel index in the texture data
                uint32_t targetPixelIndex = (j * textureWidth + i) * 3;

                // Ensure the source pixel index is within bounds
                if (sourcePixelIndex < width * height * 3)
                {
                    // Copy RGB data from source to target
                    textureData[targetPixelIndex] = data[sourcePixelIndex];         // Red
                    textureData[targetPixelIndex + 1] = data[sourcePixelIndex + 1]; // Green
                    textureData[targetPixelIndex + 2] = data[sourcePixelIndex + 2]; // Blue
                }
            }
        }
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