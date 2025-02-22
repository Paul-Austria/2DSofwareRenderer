#include "PrimitivesRenderer.h"
#include <algorithm>
#include "../util/MemHandler.h"
#include "../data/BlendMode/BlendFunctions.h"

#include "../RenderContext2D.h"
#include <float.h>
#include <math.h>

using namespace Renderer2D;

PrimitivesRenderer::PrimitivesRenderer(RenderContext2D &context) : RendererBase(context)
{
}
void PrimitivesRenderer::DrawRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch(); // Get the pitch (bytes per row)

    if (x < 0)
    {
        length = length + x;
        x = 0;
    }
    if (y < 0)
    {
        height = height + y;
        y = 0;
    }

    auto clippingArea = context.GetClippingArea();

    uint16_t clipStartX = context.IsClippingEnabled() ? std::max(x, clippingArea.startX) : x;
    uint16_t clipStartY = context.IsClippingEnabled() ? std::max(y, clippingArea.startY) : y;
    uint16_t clipEndX = context.IsClippingEnabled() ? std::min(static_cast<int>(x + length), static_cast<int>(clippingArea.endX)) : x + length;
    uint16_t clipEndY = context.IsClippingEnabled() ? std::min(static_cast<int>(y + height), static_cast<int>(clippingArea.endY)) : y + height;

    // Restrict drawing within the texture bounds
    clipEndX = std::min(clipEndX, textureWidth);
    clipEndY = std::min(clipEndY, textureHeight);

    // If nothing to draw, return
    if (clipStartX >= clipEndX || clipStartY >= clipEndY)
        return;

    // Calculate the number of bytes in a row
    size_t bytesPerRow = (clipEndX - clipStartX) * info.bytesPerPixel;

    BlendMode subBlend = context.GetBlendMode();
    if (color.GetAlpha() == 255)
        subBlend = BlendMode::NOBLEND;
    uint8_t *dest = textureData + (clipStartY * pitch) + (clipStartX * info.bytesPerPixel);

    uint8_t pixelData[MAXBYTESPERPIXEL];
    uint8_t rowPixelData[MAXROWLENGTH * MAXBYTESPERPIXEL];

    switch (subBlend)
    {
    case BlendMode::NOBLEND:
    {

        color.ConvertTo(format, pixelData);

        uint8_t singlePixelData[MAXBYTESPERPIXEL];
        MemHandler::MemCopy(singlePixelData, pixelData, info.bytesPerPixel);

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
    default:
    {

        for (size_t byteIndex = 0; byteIndex < (clipEndX - clipStartX) * 4; byteIndex += 4)
        {
            MemHandler::MemCopy(rowPixelData + byteIndex, color.data, 4);
        }

        for (uint16_t j = clipStartY; j < clipEndY; ++j)
        {
            uint8_t *rowDest = dest + (j - clipStartY) * pitch;
            size_t rowLength = (clipEndX - clipStartX); // Number of pixels per row
            PixelFormatInfo infosrcColor = PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888);
            BlendFunctions::BlendRow(rowDest, rowPixelData, rowLength, info, infosrcColor, context.GetColoring(),true, context.GetBlendMode());
        }
        break;
    }
    }
}


















void PrimitivesRenderer::DrawRotatedRect(Color color, int16_t x, int16_t y, uint16_t length, uint16_t height, float angle, int16_t offsetX, int16_t offsetY)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch(); // Get the pitch (bytes per row)

    // Validate angle
    int normalizedAngle = static_cast<int>(angle) % 360;
    if (normalizedAngle < 0)
        normalizedAngle += 360;

    if (normalizedAngle == 0)
    {
        DrawRect(color, x, y, length, height);
        return;
    }

    // Allocate a buffer for one row of converted pixels
    size_t rowLength = length * info.bytesPerPixel;
    alignas(16) uint8_t buffer[MAXROWLENGTH * MAXBYTESPERPIXEL];

    // Calculate the center of the rectangle
    int centerX = length / 2;
    int centerY = height / 2;

    // Adjust offset
    offsetX = centerX + offsetX;
    offsetY = centerY + offsetY;

    float pivotX = x + offsetX;
    float pivotY = y + offsetY;

    ClippingArea clippingArea = context.GetClippingArea();

    float radians = normalizedAngle * 3.14159265358979f / 180.0f;
    float cosAngle = cos(radians);
    float sinAngle = sin(radians);

    // Calculate the bounds of the rotated rectangle by transforming all corners
    float corners[4][2] = {
        {-offsetX, -offsetY},                // Top-left
        {length - offsetX, -offsetY},        // Top-right
        {-offsetX, height - offsetY},        // Bottom-left
        {length - offsetX, height - offsetY} // Bottom-right
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
    color.ConvertTo(format, buffer);

    BlendMode subBlend = context.GetBlendMode();
    if (color.GetAlpha() == 255)
        subBlend = BlendMode::NOBLEND;

    for (int destY = boundMinY; destY <= boundMaxY; destY++)
    {
        for (int destX = boundMinX; destX <= boundMaxX; destX++)
        {
            if (destX < 0 || destX >= textureWidth || destY < 0 || destY >= textureHeight)
            {
                continue;
            }

            if (context.IsClippingEnabled())
            {
                if (destX < clippingArea.startX || destX >= clippingArea.endX || destY < clippingArea.startY || destY >= clippingArea.endY)
                {
                    continue;
                }
            }

            float dx = destX - pivotX;
            float dy = destY - pivotY;

            float srcXf = dx * cosAngle + dy * sinAngle + centerX;
            float srcYf = -dx * sinAngle + dy * cosAngle + centerY;

            int srcX = static_cast<int>(srcXf);
            int srcY = static_cast<int>(srcYf);

            if (srcX >= 0 && srcX < length && srcY >= 0 && srcY < height)
            {
                uint8_t *targetPixel = textureData + (destY * pitch) + (destX * info.bytesPerPixel);
                if (subBlend != BlendMode::NOBLEND)
                {
                    BlendFunctions::BlendRow(targetPixel, buffer, 1, info, PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888), context.GetColoring(),false, context.GetBlendMode());
                }
                else
                {

                    MemHandler::MemCopy(targetPixel, buffer, info.bytesPerPixel);
                }
            }
        }
    }
}

void PrimitivesRenderer::DrawLine(Color color, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    auto targetTexture = context.GetTargetTexture();
    if (!targetTexture)
        return;

    if (x0 == x1)
    {
        DrawRect(color, x0, y0, 1, y1 - y0);
        return;
    }
    if (y0 == y1)
    {
        DrawRect(color, x0, y0, x1 - x0, 1);
        return;
    }
    auto clippingArea = context.GetClippingArea();

    // Clip the line to the clipping area
    if (context.IsClippingEnabled())
    {
        // Cohen-Sutherland clipping algorithm
        auto clipCode = [](int16_t x, int16_t y, const ClippingArea &clip)
        {
            int code = 0;
            if (x < clip.startX)
                code |= 1;
            if (x > clip.endX)
                code |= 2;
            if (y < clip.startY)
                code |= 4;
            if (y > clip.endY)
                code |= 8;
            return code;
        };

        int code0 = clipCode(x0, y0, clippingArea);
        int code1 = clipCode(x1, y1, clippingArea);
        bool accept = false;

        while (true)
        {
            if (!(code0 | code1))
            {
                accept = true;
                break;
            }
            else if (code0 & code1)
            {
                return;
            }
            else
            {
                int codeOut = code0 ? code0 : code1;
                int16_t x, y;

                if (codeOut & 8)
                {
                    x = x0 + (x1 - x0) * (clippingArea.endY - y0) / (y1 - y0);
                    y = clippingArea.endY;
                }
                else if (codeOut & 4)
                {
                    x = x0 + (x1 - x0) * (clippingArea.startY - y0) / (y1 - y0);
                    y = clippingArea.startY;
                }
                else if (codeOut & 2)
                {
                    y = y0 + (y1 - y0) * (clippingArea.endX - x0) / (x1 - x0);
                    x = clippingArea.endX;
                }
                else
                {
                    y = y0 + (y1 - y0) * (clippingArea.startX - x0) / (x1 - x0);
                    x = clippingArea.startX;
                }

                if (codeOut == code0)
                {
                    x0 = x;
                    y0 = y;
                    code0 = clipCode(x0, y0, clippingArea);
                }
                else
                {
                    x1 = x;
                    y1 = y;
                    code1 = clipCode(x1, y1, clippingArea);
                }
            }
        }

        if (!accept)
        {
            return;
        }
    }

    PixelFormat format = targetTexture->GetFormat();
    PixelFormatInfo info = PixelFormatRegistry::GetInfo(format);

    uint8_t *textureData = targetTexture->GetData();
    uint16_t textureWidth = targetTexture->GetWidth();
    uint16_t textureHeight = targetTexture->GetHeight();
    uint32_t pitch = targetTexture->GetPitch(); 

    int16_t dx = std::abs(x1 - x0);
    int16_t dy = std::abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;

    BlendMode subBlend = context.GetBlendMode();
    uint8_t pixelData[MAXBYTESPERPIXEL];
    color.ConvertTo(format, pixelData);

    while (true)
    {
        if (x0 >= 0 && x0 < textureWidth && y0 >= 0 && y0 < textureHeight)
        {
            uint8_t *targetPixel = textureData + (y0 * pitch) + (x0 * info.bytesPerPixel);
            switch (subBlend)
            {
            case BlendMode::NOBLEND:
                MemHandler::MemCopy(targetPixel, pixelData, info.bytesPerPixel);
                break;
            default:
                BlendFunctions::BlendRow(targetPixel, pixelData, 1, info, PixelFormatRegistry::GetInfo(PixelFormat::ARGB8888), context.GetColoring(),false, context.GetBlendMode());
                break;
            }
        }

        if (x0 == x1 && y0 == y1)
            break;

        int16_t e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}