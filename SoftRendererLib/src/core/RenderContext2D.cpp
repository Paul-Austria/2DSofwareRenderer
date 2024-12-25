#include "RenderContext2D.h"
#include "../data/PixelFormat/PixelConverter.h"
#include "../data/PixelFormat/PixelFormatInfo.h"
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

using namespace Renderer2D;

#define MAXBYTESPERPIXEL 16
#define MAXROWLENGTH 2048

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
        return;
    }

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    // Get pointer to the texture data
    uint8_t *textureData = targetTexture->GetData();
    uint16_t width = targetTexture->GetWidth();
    uint16_t height = targetTexture->GetHeight();

    // Use a stack-allocated array for pixel data
    uint8_t pixelData[4];
    color.ConvertTo(format, pixelData);

    // Calculate the total number of pixels
    size_t totalPixels = width * height;

    // Clear the texture by filling it with the converted color
    for (size_t i = 0; i < totalPixels; ++i)
    {
        // Calculate the position in the texture data array
        size_t pixelIndex = i * info.bytesPerPixel;
        MemHandler::MemCopy(&textureData[pixelIndex], pixelData, info.bytesPerPixel);
    }
}
void RenderContext2D::DrawRect(Color color, uint16_t x, uint16_t y, uint16_t length, uint16_t height)
{
    if (!targetTexture)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch(); // Get the pitch (bytes per row)

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
    uint8_t *dest = textureData + (clipStartY * pitch) + (clipStartX * info.bytesPerPixel);

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {

        uint8_t pixelData[MAXBYTESPERPIXEL];
        color.ConvertTo(format, pixelData);

        uint8_t singlePixelData[MAXBYTESPERPIXEL];
        MemHandler::MemCopy(singlePixelData, pixelData, info.bytesPerPixel);

        uint8_t rowPixelData[MAXROWLENGTH * MAXBYTESPERPIXEL];

        for (size_t byteIndex = 0; byteIndex < bytesPerRow; byteIndex += info.bytesPerPixel)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, singlePixelData, info.bytesPerPixel);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            // Calculate the destination for the current row using pitch
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;

            // Use memcpy to copy the whole row of pixels at once
            MemHandler::MemCopy(rowDest, rowPixelData, bytesPerRow);
        }
        break;
    }
    case BlendMode::BLEND:
    {

        uint8_t singlePixelData[MAXBYTESPERPIXEL];

        uint8_t rowPixelData[MAXROWLENGTH * MAXBYTESPERPIXEL];

        for (size_t byteIndex = 0; byteIndex < (clipEndX - clipStartX) * 4; byteIndex += 4)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, color.data, 4);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;
            size_t rowLength = (clipEndX - clipStartX); // Number of pixels per row
            PixelFormatInfo infosrcColor = PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888);
            BlendFunctions::BlendRow(rowDest, rowPixelData, rowLength, info, infosrcColor);
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
    size_t targetPitch = targetTexture->GetPitch(); // Row stride for target texture

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch(); // Row stride for source texture

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

    // Determine blending mode
    BlendMode subBlend = (mode == BlendMode::NOBLEND || sourceInfo.hasAlpha) ? mode : BlendMode::NOBLEND;

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {
        // Conversion function for source to target format if necessary
        PixelConverter::ConvertFunc convertFunc = nullptr;

        convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);
        if (!convertFunc)
            return;

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *targetRow = targetData + j * targetPitch + clipStartX * targetInfo.bytesPerPixel;
            const uint8_t *sourceRow = sourceData + (j - y) * sourcePitch + (clipStartX - x) * sourceInfo.bytesPerPixel;

            convertFunc(sourceRow, targetRow, clipEndX - clipStartX);
        }
        break;
    }

    case BlendMode::BLEND:
    {
        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *targetRow = targetData + j * targetPitch + clipStartX * targetInfo.bytesPerPixel;
            const uint8_t *sourceRow = sourceData + (j - y) * sourcePitch + (clipStartX - x) * sourceInfo.bytesPerPixel;

            // Blend the entire row
            BlendFunctions::BlendRow(targetRow, sourceRow, clipEndX - clipStartX, targetInfo, sourceInfo);
        }
        break;
    }

    default:
        break;
    }
}

void RenderContext2D::DrawTexture(Texture &texture, uint16_t x, uint16_t y, float angle, int32_t offsetX, int32_t offsetY)
{
    if (!targetTexture)
        return;

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Validate angle
    int normalizedAngle = static_cast<int>(angle) % 360;
    if (normalizedAngle < 0)
        normalizedAngle += 360;

    if (normalizedAngle == 0)
    {

        DrawTexture(texture, x, y);
        return;
    }

    // Determine blending mode
    BlendMode subBlend = (mode == BlendMode::NOBLEND || sourceInfo.hasAlpha) ? mode : BlendMode::NOBLEND;

    // Allocate a buffer for one row of converted pixels
    size_t rowLength = sourceWidth * targetInfo.bytesPerPixel;
    alignas(16) uint8_t buffer[MAXROWLENGTH * MAXBYTESPERPIXEL];
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);

    // Rotation-specific loops
    switch (normalizedAngle)
    {
    case 90:
        for (uint16_t j = 0; j < sourceHeight; ++j)
        {
            const uint8_t *sourceRow = sourceData + (j * sourcePitch);
            convertFunc(sourceRow, buffer, sourceWidth);

            for (uint16_t i = 0; i < sourceWidth; ++i)
            {
                int targetX = x + sourceHeight - 1 - j;
                int targetY = y + i;

                if (enableClipping)
                {
                    if (targetX < startX || targetX >= endX || targetY < startY || targetY >= endY)
                        continue;
                }

                if (targetX < 0 || targetX >= targetWidth || targetY < 0 || targetY >= targetHeight)
                    continue;

                uint8_t *targetPixel = targetData + (targetY * targetPitch) + (targetX * targetInfo.bytesPerPixel);
                memcpy(targetPixel, buffer + (i * targetInfo.bytesPerPixel), targetInfo.bytesPerPixel);
            }
        }
        break;

    case 180:
        for (uint16_t j = 0; j < sourceHeight; ++j)
        {
            const uint8_t *sourceRow = sourceData + (j * sourcePitch);
            convertFunc(sourceRow, buffer, sourceWidth);

            for (uint16_t i = 0; i < sourceWidth; ++i)
            {
                int targetX = x + sourceWidth - 1 - i;
                int targetY = y + sourceHeight - 1 - j;

                if (enableClipping)
                {
                    if (targetX < startX || targetX >= endX || targetY < startY || targetY >= endY)
                        continue;
                }

                if (targetX < 0 || targetX >= targetWidth || targetY < 0 || targetY >= targetHeight)
                    continue;

                uint8_t *targetPixel = targetData + (targetY * targetPitch) + (targetX * targetInfo.bytesPerPixel);
                memcpy(targetPixel, buffer + (i * targetInfo.bytesPerPixel), targetInfo.bytesPerPixel);
            }
        }
        break;

    case 270:
        for (uint16_t j = 0; j < sourceHeight; ++j)
        {
            const uint8_t *sourceRow = sourceData + (j * sourcePitch);
            convertFunc(sourceRow, buffer, sourceWidth);

            for (uint16_t i = 0; i < sourceWidth; ++i)
            {
                int targetX = x + j;
                int targetY = y + sourceWidth - 1 - i;

                if (enableClipping)
                {
                    if (targetX < startX || targetX >= endX || targetY < startY || targetY >= endY)
                        continue;
                }

                if (targetX < 0 || targetX >= targetWidth || targetY < 0 || targetY >= targetHeight)
                    continue;

                uint8_t *targetPixel = targetData + (targetY * targetPitch) + (targetX * targetInfo.bytesPerPixel);
                memcpy(targetPixel, buffer + (i * targetInfo.bytesPerPixel), targetInfo.bytesPerPixel);
            }
        }
        break;

    default: // rotate for all remaining degrees
    {
        float radians = normalizedAngle * 3.14159265358979f / 180.0f;
        float cosAngle = cos(radians);
        float sinAngle = sin(radians);

        offsetX = sourceWidth / 2 + offsetX;
        offsetY = sourceHeight / 2 + offsetY;

        float pivotX = x + offsetX;
        float pivotY = y + offsetY;

        // Calculate the bounds of the rotated image by transforming all corners
        float corners[4][2] = {
            {-offsetX, -offsetY},                           // Top-left
            {sourceWidth - offsetX, -offsetY},              // Top-right
            {-offsetX, sourceHeight - offsetY},             // Bottom-left
            {sourceWidth - offsetX, sourceHeight - offsetY} // Bottom-right
        };

        // Find the min/max coordinates after rotation
        float minX = FLT_MAX, minY = FLT_MAX;
        float maxX = -FLT_MAX, maxY = -FLT_MAX;

        for (int i = 0; i < 4; i++)
        {
            float rotX = corners[i][0] * cosAngle - corners[i][1] * sinAngle + pivotX;
            float rotY = corners[i][0] * sinAngle + corners[i][1] * cosAngle + pivotY;

            minX = std::min(minX, rotX);
            minY = std::min(minY, rotY);
            maxX = std::max(maxX, rotX);
            maxY = std::max(maxY, rotY);
        }

        int boundMinX = static_cast<int>(floor(minX)) - 1;
        int boundMinY = static_cast<int>(floor(minY)) - 1;
        int boundMaxX = static_cast<int>(ceil(maxX)) + 1;
        int boundMaxY = static_cast<int>(ceil(maxY)) + 1;

        int lastConvertedRow = -1; 

        
        for (int destY = boundMinY; destY <= boundMaxY; destY++)
        {
            for (int destX = boundMinX; destX <= boundMaxX; destX++)
            {
                if (destX < 0 || destX >= targetWidth || destY < 0 || destY >= targetHeight)
                {
                    continue;
                }

                if (enableClipping)
                {
                    if (destX < startX || destX >= endX || destY < startY || destY >= endY)
                    {
                        continue;
                    }
                }

                float dx = destX - pivotX;
                float dy = destY - pivotY;

                float srcXf = dx * cosAngle + dy * sinAngle + offsetX;
                float srcYf = -dx * sinAngle + dy * cosAngle + offsetY;

                int srcX = static_cast<int>(srcXf);
                int srcY = static_cast<int>(srcYf);

                if (srcX >= 0 && srcX < sourceWidth && srcY >= 0 && srcY < sourceHeight)
                {
                    if (srcY != lastConvertedRow)
                    {
                        const uint8_t *sourceRow = sourceData + (srcY * sourcePitch);
                        convertFunc(sourceRow, buffer, sourceWidth);
                        lastConvertedRow = srcY;
                    }

                    // Use the converted row from buffer
                    const uint8_t *sourcePixel = buffer + (srcX * targetInfo.bytesPerPixel);
                    uint8_t *targetPixel = targetData + (destY * targetPitch) + (destX * targetInfo.bytesPerPixel);
                    memcpy(targetPixel, sourcePixel, targetInfo.bytesPerPixel);
                }
            }
        }
        break;
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