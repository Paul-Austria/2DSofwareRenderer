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

void DrawTextureRotatedBlending(Texture &texture, uint16_t x, uint16_t y, float angle, int32_t offsetX, int32_t offsetY)
{
}
void RenderContext2D::DrawTexture(Texture &texture, uint16_t x, uint16_t y, float angle, int32_t offsetX, int32_t offsetY)
{
    if (!targetTexture)
        return;

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

    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Allocate a buffer for one row of converted pixels
    size_t rowLength = sourceWidth * targetInfo.bytesPerPixel;
    alignas(16) uint8_t buffer[MAXROWLENGTH * MAXBYTESPERPIXEL];
    PixelConverter::ConvertFunc convertFunc = PixelConverter::GetConversionFunction(sourceFormat, targetFormat);

    // Calculate the center of the source image
    int centerX = sourceWidth / 2;
    int centerY = sourceHeight / 2;

    // Calculate the offset to maintain the center point
    int offX = (x + centerX - sourceHeight / 2);
    int offY = (y + centerY - sourceWidth / 2);

    if (offsetX == 0 && offsetY == 0 && normalizedAngle % 90 == 0)
    {
        switch (normalizedAngle)
        {
        case 90:

            for (uint16_t j = 0; j < sourceHeight; ++j)
            {
                uint8_t *sourceRow = sourceData + (j * sourcePitch);
                uint8_t *bufferPointer = nullptr;
                switch (subBlend)
                {
                case BlendMode::NOBLEND:
                    convertFunc(sourceRow, buffer, sourceWidth);
                    bufferPointer = buffer;
                    break;

                default:
                    bufferPointer = sourceRow;
                    break;
                }

                for (uint16_t i = 0; i < sourceWidth; ++i)
                {
                    int targetX = offX + sourceHeight - 1 - j;
                    int targetY = offY + i;

                    if (enableClipping)
                    {
                        if (targetX < startX || targetX >= endX || targetY < startY || targetY >= endY)
                            continue;
                    }

                    if (targetX < 0 || targetX >= targetWidth || targetY < 0 || targetY >= targetHeight)
                        continue;

                    uint8_t *targetPixel = targetData + (targetY * targetPitch) + (targetX * targetInfo.bytesPerPixel);
                    switch (subBlend)
                    {
                    case BlendMode::NOBLEND:
                        memcpy(targetPixel, bufferPointer + (i * targetInfo.bytesPerPixel), targetInfo.bytesPerPixel);
                        break;
                    default:
                        BlendFunctions::BlendRow(targetPixel, bufferPointer + (i * sourceInfo.bytesPerPixel), 1, targetInfo, sourceInfo);
                        break;
                    }
                }
            }
            break;

        case 180:
            for (uint16_t j = 0; j < sourceHeight; ++j)
            {
                uint8_t *sourceRow = sourceData + (j * sourcePitch);
                uint8_t *bufferPointer = nullptr;
                switch (subBlend)
                {
                case BlendMode::NOBLEND:
                    convertFunc(sourceRow, buffer, sourceWidth);
                    bufferPointer = buffer;
                    break;

                default:
                    bufferPointer = sourceRow;
                    break;
                }

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
                    switch (subBlend)
                    {
                    case BlendMode::NOBLEND:
                        memcpy(targetPixel, bufferPointer + (i * targetInfo.bytesPerPixel), targetInfo.bytesPerPixel);
                        break;
                    default:
                        BlendFunctions::BlendRow(targetPixel, bufferPointer + (i * sourceInfo.bytesPerPixel), 1, targetInfo, sourceInfo);
                        break;
                    }
                }
            }
            break;

        case 270:
            for (uint16_t j = 0; j < sourceHeight; ++j)
            {
                uint8_t *sourceRow = sourceData + (j * sourcePitch);
                uint8_t *bufferPointer = nullptr;
                switch (subBlend)
                {
                case BlendMode::NOBLEND:
                    convertFunc(sourceRow, buffer, sourceWidth);
                    bufferPointer = buffer;
                    break;

                default:
                    bufferPointer = sourceRow;
                    break;
                }
                for (uint16_t i = 0; i < sourceWidth; ++i)
                {
                    int targetX = offX + j;
                    int targetY = offY + sourceWidth - 1 - i;

                    if (enableClipping)
                    {
                        if (targetX < startX || targetX >= endX || targetY < startY || targetY >= endY)
                            continue;
                    }

                    if (targetX < 0 || targetX >= targetWidth || targetY < 0 || targetY >= targetHeight)
                        continue;

                    uint8_t *targetPixel = targetData + (targetY * targetPitch) + (targetX * targetInfo.bytesPerPixel);
                    switch (subBlend)
                    {
                    case BlendMode::NOBLEND:
                        memcpy(targetPixel, bufferPointer + (i * targetInfo.bytesPerPixel), targetInfo.bytesPerPixel);
                        break;
                    default:
                        BlendFunctions::BlendRow(targetPixel, bufferPointer + (i * sourceInfo.bytesPerPixel), 1, targetInfo, sourceInfo);
                        break;
                    }
                }
            }
            break;
        }
    }
    else
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
                    uint8_t *sourcePixel = nullptr;
                    uint8_t *targetPixel = nullptr;
                    uint8_t *sourceRow = sourceData + (srcY * sourcePitch);
                    switch (subBlend)
                    {
                    case BlendMode::NOBLEND:
                        if (srcY != lastConvertedRow)
                        {
                            convertFunc(sourceRow, buffer, sourceWidth);
                            lastConvertedRow = srcY;
                        }
                        sourcePixel = buffer + (srcX * targetInfo.bytesPerPixel);
                        targetPixel = targetData + (destY * targetPitch) + (destX * targetInfo.bytesPerPixel);
                        memcpy(targetPixel, sourcePixel, targetInfo.bytesPerPixel);
                        break;

                    default:
                    {
                        sourcePixel = sourceRow + (srcX * sourceInfo.bytesPerPixel);
                        targetPixel = targetData + (destY * targetPitch) + (destX * targetInfo.bytesPerPixel);
                        BlendFunctions::BlendRow(targetPixel, sourcePixel, 1, targetInfo, sourceInfo);
                    }
                    break;
                    }
                }
            }
        }
    }
    // Rotation-specific loops
}

void RenderContext2D::DrawTexture(Texture &texture, uint16_t x, uint16_t y,
                                  float scaleX, float scaleY, SamplingMethod method)
{
    if (!targetTexture || scaleX <= 0 || scaleY <= 0)
        return;

    // Get format information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);

    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Get source texture information
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Calculate scaled dimensions
    uint16_t dstWidth = static_cast<uint16_t>(sourceWidth * scaleX);
    uint16_t dstHeight = static_cast<uint16_t>(sourceHeight * scaleY);

    // Set clipping boundaries based on SCALED size
    uint16_t clipStartX = enableClipping ? std::max(x, startX) : x;
    uint16_t clipStartY = enableClipping ? std::max(y, startY) : y;
    uint16_t clipEndX = enableClipping
                            ? std::min(static_cast<int>(x + dstWidth), static_cast<int>(endX))
                            : x + dstWidth;
    uint16_t clipEndY = enableClipping
                            ? std::min(static_cast<int>(y + dstHeight), static_cast<int>(endY))
                            : y + dstHeight;

    // Clamp to target texture bounds
    clipEndX = std::min(clipEndX, targetWidth);
    clipEndY = std::min(clipEndY, targetHeight);

    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Prepare blending mode
    BlendMode subBlend = (mode == BlendMode::NOBLEND || sourceInfo.hasAlpha) ? mode : BlendMode::NOBLEND;
    bool useBlending = (subBlend == BlendMode::BLEND);

    uint8_t dstBuffer[MAXBYTESPERPIXEL];

    for (uint16_t dy = clipStartY; dy < clipEndY; dy++)
    {
        for (uint16_t dx = clipStartX; dx < clipEndX; dx++)
        {
            // Calculate normalized texture coordinates with inverse scaling
            float tx = (dx - x) * (static_cast<float>(sourceWidth) / dstWidth);
            float ty = (dy - y) * (static_cast<float>(sourceHeight) / dstHeight);

            // Clamp coordinates to texture size
            tx = std::max(0.0f, std::min(tx, static_cast<float>(sourceWidth - 1)));
            ty = std::max(0.0f, std::min(ty, static_cast<float>(sourceHeight - 1)));

            // Sample texture
            switch (method)
            {
            case SamplingMethod::NEAREST:
            {
                // Nearest neighbor sampling
                uint16_t sx = static_cast<uint16_t>(tx + 0.5f);
                uint16_t sy = static_cast<uint16_t>(ty + 0.5f);
                const uint8_t *srcPixel = sourceData +
                                          sy * sourcePitch +
                                          sx * sourceInfo.bytesPerPixel;

                PixelConverter::Convert(
                    sourceFormat,
                    targetFormat,
                    srcPixel,
                    dstBuffer,
                    1);
                break;
            }

            case SamplingMethod::LINEAR:
            {
                // Bilinear interpolation
                int x0 = static_cast<int>(tx);
                int y0 = static_cast<int>(ty);
                int x1 = std::min(x0 + 1, static_cast<int>(sourceWidth - 1));
                int y1 = std::min(y0 + 1, static_cast<int>(sourceHeight - 1));

                float fx = tx - x0;
                float fy = ty - y0;

                // Get four neighboring pixels
                const uint8_t *pixels[4] = {
                    sourceData + y0 * sourcePitch + x0 * sourceInfo.bytesPerPixel, // (x0,y0)
                    sourceData + y0 * sourcePitch + x1 * sourceInfo.bytesPerPixel, // (x1,y0)
                    sourceData + y1 * sourcePitch + x0 * sourceInfo.bytesPerPixel, // (x0,y1)
                    sourceData + y1 * sourcePitch + x1 * sourceInfo.bytesPerPixel  // (x1,y1)
                };

                // Convert all four pixels to ARGB8888 color format
                Color colors[4];
                for (int i = 0; i < 4; i++)
                {
                    colors[i] = Color(pixels[i],sourceFormat);
                }

                // Horizontal interpolation
                Color top = Color::Lerp(colors[0], colors[1], fx);
                Color bottom = Color::Lerp(colors[2], colors[3], fx);

                // Vertical interpolation
                Color finalColor = Color::Lerp(top, bottom, fy);

                finalColor.ConvertTo(targetFormat, dstBuffer);
                break;
            }
            }

            // Get destination pixel location
            uint8_t *dstPixel = targetData +
                                dy * targetPitch +
                                dx * targetInfo.bytesPerPixel;

            // Handle blending
            if (useBlending)
            {
                    BlendFunctions::BlendRow(dstPixel, dstBuffer, 1, targetInfo, sourceInfo);
            }
            else
            {
                MemHandler::MemCopy(dstPixel, dstBuffer, targetInfo.bytesPerPixel);
            }
        }
    }
}




void RenderContext2D::DrawTexture(Texture &texture, uint16_t x, uint16_t y, 
                                                 float scaleX, float scaleY, float angle,
                                                 int32_t offsetX, int32_t offsetY, 
                                                 SamplingMethod method)
{
    if (!targetTexture || scaleX <= 0 || scaleY <= 0)
        return;

    // Get source texture information
    PixelFormat sourceFormat = texture.GetFormat();
    PixelFormatInfo sourceInfo = PixelFormatRegistry::GetInfo(sourceFormat);
    uint8_t *sourceData = texture.GetData();
    uint16_t sourceWidth = texture.GetWidth();
    uint16_t sourceHeight = texture.GetHeight();
    size_t sourcePitch = texture.GetPitch();

    // Calculate scaled dimensions
    uint16_t scaledWidth = static_cast<uint16_t>(sourceWidth * scaleX);
    uint16_t scaledHeight = static_cast<uint16_t>(sourceHeight * scaleY);

    // Validate angle
    int normalizedAngle = static_cast<int>(angle) % 360;
    if (normalizedAngle < 0)
        normalizedAngle += 360;

    // If no rotation, use the scaling-only function
    if (normalizedAngle == 0)
    {
        DrawTexture(texture, x, y, scaleX, scaleY, method);
        return;
    }

    // Get target texture information
    PixelFormat targetFormat = targetTexture->GetFormat();
    PixelFormatInfo targetInfo = PixelFormatRegistry::GetInfo(targetFormat);
    uint8_t *targetData = targetTexture->GetData();
    uint16_t targetWidth = targetTexture->GetWidth();
    uint16_t targetHeight = targetTexture->GetHeight();
    size_t targetPitch = targetTexture->GetPitch();

    // Determine blending mode
    BlendMode subBlend = (mode == BlendMode::NOBLEND || sourceInfo.hasAlpha) ? mode : BlendMode::NOBLEND;

    // Calculate rotation center with scaling
    float centerX = scaledWidth / 2.0f;
    float centerY = scaledHeight / 2.0f;

    // Calculate the pivot point including offset
    float pivotX = x + centerX + offsetX;
    float pivotY = y + centerY + offsetY;

    float radians = normalizedAngle * 3.14159265358979f / 180.0f;
    float cosAngle = cos(radians);
    float sinAngle = sin(radians);

    // Calculate the bounds of the rotated and scaled image
    float corners[4][2] = {
        {-centerX, -centerY},             // Top-left
        {scaledWidth - centerX, -centerY}, // Top-right
        {-centerX, scaledHeight - centerY}, // Bottom-left
        {scaledWidth - centerX, scaledHeight - centerY} // Bottom-right
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

    // Calculate bounds with padding
    int boundMinX = static_cast<int>(floor(minX)) - 1;
    int boundMinY = static_cast<int>(floor(minY)) - 1;
    int boundMaxX = static_cast<int>(ceil(maxX)) + 1;
    int boundMaxY = static_cast<int>(ceil(maxY)) + 1;

    // Clamp bounds to target texture and clipping region
    if (enableClipping)
    {
        boundMinX = std::max(boundMinX, static_cast<int>(startX));
        boundMinY = std::max(boundMinY, static_cast<int>(startY));
        boundMaxX = std::min(boundMaxX, static_cast<int>(endX));
        boundMaxY = std::min(boundMaxY, static_cast<int>(endY));
    }

    boundMinX = std::max(boundMinX, 0);
    boundMinY = std::max(boundMinY, 0);
    boundMaxX = std::min(boundMaxX, static_cast<int>(targetWidth));
    boundMaxY = std::min(boundMaxY, static_cast<int>(targetHeight));

    uint8_t dstBuffer[MAXBYTESPERPIXEL];

    // Inverse transformation matrix
    float invCosAngle = cosAngle;
    float invSinAngle = -sinAngle;

    for (int destY = boundMinY; destY < boundMaxY; destY++)
    {
        for (int destX = boundMinX; destX < boundMaxX; destX++)
        {
            // Calculate source position with rotation and scaling
            float dx = destX - pivotX;
            float dy = destY - pivotY;

            // Apply inverse rotation
            float rotX = dx * invCosAngle - dy * invSinAngle;
            float rotY = dx * invSinAngle + dy * invCosAngle;

            // Transform to source space (accounting for scale)
            float srcXf = (rotX + centerX) / scaleX;
            float srcYf = (rotY + centerY) / scaleY;

            // Check if the source pixel is within bounds
            if (srcXf >= 0 && srcXf < sourceWidth && srcYf >= 0 && srcYf < sourceHeight)
            {
                switch (method)
                {
                    case SamplingMethod::NEAREST:
                    {
                        uint16_t sx = static_cast<uint16_t>(srcXf + 0.5f);
                        uint16_t sy = static_cast<uint16_t>(srcYf + 0.5f);
                        const uint8_t *srcPixel = sourceData + sy * sourcePitch + sx * sourceInfo.bytesPerPixel;
                        PixelConverter::Convert(sourceFormat, targetFormat, srcPixel, dstBuffer, 1);
                        break;
                    }

                    case SamplingMethod::LINEAR:
                    {
                        int x0 = static_cast<int>(srcXf);
                        int y0 = static_cast<int>(srcYf);
                        int x1 = std::min(x0 + 1, static_cast<int>(sourceWidth - 1));
                        int y1 = std::min(y0 + 1, static_cast<int>(sourceHeight - 1));

                        float fx = srcXf - x0;
                        float fy = srcYf - y0;

                        const uint8_t *pixels[4] = {
                            sourceData + y0 * sourcePitch + x0 * sourceInfo.bytesPerPixel,
                            sourceData + y0 * sourcePitch + x1 * sourceInfo.bytesPerPixel,
                            sourceData + y1 * sourcePitch + x0 * sourceInfo.bytesPerPixel,
                            sourceData + y1 * sourcePitch + x1 * sourceInfo.bytesPerPixel
                        };

                        Color colors[4];
                        for (int i = 0; i < 4; i++)
                        {
                            colors[i] = Color(pixels[i], sourceFormat);
                        }

                        Color top = Color::Lerp(colors[0], colors[1], fx);
                        Color bottom = Color::Lerp(colors[2], colors[3], fx);
                        Color finalColor = Color::Lerp(top, bottom, fy);
                        finalColor.ConvertTo(targetFormat, dstBuffer);
                        break;
                    }
                }

                uint8_t *targetPixel = targetData + destY * targetPitch + destX * targetInfo.bytesPerPixel;
                
                if (subBlend == BlendMode::BLEND)
                {
                    BlendFunctions::BlendRow(targetPixel, dstBuffer, 1, targetInfo, sourceInfo);
                }
                else
                {
                    MemHandler::MemCopy(targetPixel, dstBuffer, targetInfo.bytesPerPixel);
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